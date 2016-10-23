/*
  Copyright (C) 2015 Kristian Nordman

  This file is part of esstee.

  esstee is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  esstee is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with esstee.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <elements/user_function_blocks.h>
#include <elements/ifunction_block.h>
#include <statements/statements.h>
#include <util/macros.h>
#include <linker/linker.h>

#include <utlist.h>

#include <stdio.h>

struct user_function_block_t {
    struct function_block_iface_t function_block;
    struct type_iface_t type;

    struct header_t *header;
    struct invoke_iface_t *statements;

    struct named_ref_pool_iface_t *type_refs;
    struct named_ref_pool_iface_t *var_refs;

    char *identifier;
    struct st_location_t *location;
};

/**************************************************************************/
/* Function block interface                                               */
/**************************************************************************/
static int user_fb_resolve_header_type_references(
    struct function_block_iface_t *self,
    struct type_iface_t *global_type_table,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_function_block_t *ufb =
        CONTAINER_OF(self, struct user_function_block_t, function_block);

    ufb->type_refs->reset_resolved(ufb->type_refs);
    ufb->var_refs->reset_resolved(ufb->var_refs);

    /* Resolve type references */
    if(ufb->header && ufb->header->types)
    {
        st_resolve_type_refs(ufb->type_refs, ufb->header->types);
    }

    /* Any remaining type references by global types */
    st_resolve_type_refs(ufb->type_refs, global_type_table);

    /* Trigger callbacks for type references (e.g. those registered by
     * variables) */
    return ufb->type_refs->trigger_resolve_callbacks(ufb->type_refs,
						     config,
						     issues);
}

static int depends_on(
    const struct function_block_iface_t *self,
    const struct type_iface_t *type)
{
    const struct user_function_block_t *ufb =
        CONTAINER_OF(self, struct user_function_block_t, function_block);

    if(type == &(ufb->type))
    {
	/* A function block depends on itself. */
	return ESSTEE_TRUE;
    }
    
    if(ufb->header && ufb->header->variables)
    {
	/* There is a dependency on a type if any of the variables in
	 * a function block lead to that type. */
	const struct variable_iface_t *itr = NULL;
	DL_FOREACH(ufb->header->variables, itr)
	{
	    const struct type_iface_t *var_type = itr->type(itr);

	    /* If we are dealing with a derived type, the checks
	     * should be made on its ancestor */
	    var_type = TYPE_ANCESTOR(var_type);

	    if(var_type == type)
	    {
		/* Clear case of dependency. */
		return ESSTEE_TRUE;
	    }
	    else if(var_type == &(ufb->type))
	    {
		/* No dependency, but a circular reference while
		 * checking for dependency. */
		return ESSTEE_FALSE;
	    }	
	    else if(var_type->const_function_block_handle)
	    {
		/* If the variable type is a function block, and not
		 * the same function block we are investigating, we
		 * should go a head and check whether any of the
		 * variables in this function block depends on the
		 * type. */
		const struct function_block_iface_t *fb =
		    var_type->const_function_block_handle(var_type);

		return depends_on(fb, type);
	    }
	}
    }

    return ESSTEE_FALSE;
}


static int user_fb_check_dependencies(
    	struct function_block_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues)
{
    struct user_function_block_t *ufb =
        CONTAINER_OF(self, struct user_function_block_t, function_block);

    /* At this point, all types for variables have been resolved. The
     * next step is to check for any circular references in the types
     * the variables are of. Check that the function block does not
     * depend on itself.*/
    if(ufb->header && ufb->header->variables)
    {
        struct variable_iface_t *itr = NULL;
        DL_FOREACH(ufb->header->variables, itr)
        {
	    const struct type_iface_t *var_type = itr->type(itr);

	    /* In case the variable has the type of a derived type, do
	     * all checks on the ancestor */
	    var_type = TYPE_ANCESTOR(var_type);

	    /* If the type is a function block, check for circular
	     * references */
	    if(var_type->const_function_block_handle)
	    {
		const struct function_block_iface_t *fb
		    = var_type->const_function_block_handle(var_type);

		if(depends_on(fb, &(ufb->type)) == ESSTEE_TRUE)
		{
		    const char *message = issues->build_message(
			issues,
			"circular reference, type of variable '%s' depends on its container",
			itr->identifier);

		    issues->new_issue_at(
			issues,
			message,
			ESSTEE_TYPE_ERROR,
			1,
			itr->location);

		    return ESSTEE_ERROR;
		}
	    }
	}
    }

    return ESSTEE_OK;
}

static int user_fb_finalize_header(
    struct function_block_iface_t *self,
    struct variable_iface_t *global_var_table,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_function_block_t *ufb =
        CONTAINER_OF(self, struct user_function_block_t, function_block);

    int header_valid = st_create_header_tables(ufb->header, issues);
    if(header_valid != ESSTEE_OK)
    {
	return header_valid;
    }
    
    /* Create values of header variables */
    struct variable_iface_t *itr = NULL;
    DL_FOREACH(ufb->header->variables, itr)
    {
	int create_result = itr->create(itr, config, issues);

	if(create_result != ESSTEE_OK)
	{
	    return create_result;
	}
    }

    /* Resolve variable references */
    st_resolve_var_refs(ufb->var_refs, ufb->header->variables);

    return ufb->var_refs->trigger_resolve_callbacks(ufb->var_refs,
						    config,
						    issues);
}

static int user_fb_finalize_statements(
    struct function_block_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_function_block_t *ufb =
        CONTAINER_OF(self, struct user_function_block_t, function_block);

    /* Allocate working memory for statements */
    int allocate_result = st_allocate_statements(ufb->statements,
						 issues);
    if(allocate_result != ESSTEE_OK)
    {
	return allocate_result;
    }

    return st_verify_statements(ufb->statements,
				config,
				issues);
}

static void user_fb_destroy(
    struct function_block_iface_t *self)
{
    /* TODO: user function block destructor */
}

/**************************************************************************/
/* Value interface                                                        */
/**************************************************************************/
struct user_fb_instance_t {
    struct value_iface_t value;
    struct user_function_block_t *fb;
    
    struct variable_iface_t *variables;
    struct invoke_iface_t *statements;
    int invoke_state;
};

static int user_fb_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    struct user_fb_instance_t *fv =
	CONTAINER_OF(self, struct user_fb_instance_t, value);

    int start_buffer_size = buffer_size;
    int start_written_bytes =  snprintf(buffer,
					buffer_size,
					"%s:(",
					fv->fb->identifier);
    CHECK_WRITTEN_BYTES(start_written_bytes);
    buffer += start_written_bytes;
    buffer_size -= start_written_bytes;

    struct variable_iface_t *itr = NULL;
    DL_FOREACH(fv->variables, itr)
    {
	int var_name_bytes = snprintf(buffer,
				      buffer_size,
				      "%s:",
				      itr->identifier);
	CHECK_WRITTEN_BYTES(var_name_bytes);
	buffer += var_name_bytes;
	buffer_size -= var_name_bytes;

	const struct value_iface_t *var_value = itr->value(itr);
	
	int var_written_bytes = var_value->display(var_value,
						   buffer,
						   buffer_size,
						   config);
	CHECK_WRITTEN_BYTES(var_written_bytes);
	buffer += var_written_bytes;
	buffer_size -= var_written_bytes;

	if(itr->next)
	{
	    int comma_written_bytes = snprintf(buffer,
					       buffer_size,
					       ",");
	    
	    CHECK_WRITTEN_BYTES(comma_written_bytes);
	    buffer += comma_written_bytes;
	    buffer_size -= comma_written_bytes;
	}
    }

    int end_written_bytes =  snprintf(buffer,
				      buffer_size,
				      ")");
    CHECK_WRITTEN_BYTES(end_written_bytes);
    buffer_size -= end_written_bytes;

    return start_buffer_size-buffer_size;
}

static const struct type_iface_t * user_fb_value_type_of(
    const struct value_iface_t *self)
{
    struct user_fb_instance_t *fv =
	CONTAINER_OF(self, struct user_fb_instance_t, value);

    return &(fv->fb->type);
}

static void user_fb_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: function block value destructor */
}

static struct variable_iface_t * user_fb_value_sub_variable(
    struct value_iface_t *self,
    const char *identifier,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_fb_instance_t *fv =
	CONTAINER_OF(self, struct user_fb_instance_t, value);

    struct variable_iface_t *found = NULL;
    HASH_FIND_STR(fv->variables, identifier, found);
    if(!found)
    {
	issues->new_issue(
	    issues,
	    "function block has no variable '%s'",
	    ESSTEE_CONTEXT_ERROR,
	    identifier);
    }

    return found;
}

static int user_fb_value_invoke_step(
    struct value_iface_t *self,
    const struct invoke_parameters_iface_t *parameters,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_fb_instance_t *fv =
	CONTAINER_OF(self, struct user_fb_instance_t, value);

    if(fv->invoke_state > 0)
    {
	return INVOKE_RESULT_FINISHED;
    }

    if(parameters)
    {
	int input_assign = parameters->assign_from(parameters,
						   fv->variables,
						   config,
						   issues);
	if(input_assign != ESSTEE_OK)
	{
	    return INVOKE_RESULT_ERROR;
	}
    }

    fv->invoke_state = 1;

    cursor->switch_current(cursor,
			   fv->statements,
			   config,
			   issues);

    return INVOKE_RESULT_IN_PROGRESS;
}

static int user_fb_value_invoke_verify(
    const struct value_iface_t *self,
    const struct invoke_parameters_iface_t *parameters,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct user_fb_instance_t *fv =
	CONTAINER_OF(self, struct user_fb_instance_t, value);

    if(parameters)
    {
	return parameters->verify(parameters,
				  fv->variables,
				  config,
				  issues);
    }

    return ESSTEE_OK;
}

static int user_fb_value_invoke_reset(
    struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_fb_instance_t *fv =
	CONTAINER_OF(self, struct user_fb_instance_t, value);

    fv->invoke_state = 0;

    return ESSTEE_OK;
}

/**************************************************************************/
/* Type interface                                                         */
/**************************************************************************/
static struct value_iface_t * user_fb_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_function_block_t *fb =
	CONTAINER_OF(self, struct user_function_block_t, type);

    /* Memory handles that need destroying in case of an error */
    struct user_fb_instance_t *fv = NULL;    
    struct variable_iface_t *variable_clones = NULL;
    struct invoke_iface_t *statement_clones = NULL;
    
    /* Allocate the space for an fb instance */
    ALLOC_OR_ERROR_JUMP(
	fv,
	struct user_fb_instance_t,
	issues,
	error_free_resources);

    /* Create clones of the variables for the fb instance */
    if(fb->header)
    {
	struct variable_iface_t *v_itr = NULL;
	for(v_itr = fb->header->variables; v_itr != NULL; v_itr = v_itr->hh.next)
	{
	    struct variable_iface_t *clone = v_itr->clone(v_itr, issues);

	    if(!clone)
	    {
		goto error_free_resources;
	    }
	
	    DL_APPEND(variable_clones, clone);
	}
    }

    /* Make a variable table out of the cloned variables */
    struct variable_iface_t *variable_table_clone =
	st_link_variables(variable_clones, NULL, issues);

    /* (Re)link the variable references in the fb type to the newly
     * cloned variables. No need to check the result of the resolve
     * callbacks, since any possible error is found while verifying
     * the fb type */
    fb->var_refs->reset_resolved(fb->var_refs);
    st_resolve_var_refs(fb->var_refs, variable_table_clone);
    fb->var_refs->trigger_resolve_callbacks(fb->var_refs, config, issues);

    /* Create the values of the cloned variables */
    struct variable_iface_t *v_itr = NULL;
    DL_FOREACH(variable_clones, v_itr)
    {
	int create_result = v_itr->create(v_itr, config, issues);
	
	if(create_result != ESSTEE_OK)
	{
	    goto error_free_resources;
	}
    }

    /* Create clones of the statements in the fb type */
    struct invoke_iface_t *s_itr = NULL;
    DL_FOREACH(fb->statements, s_itr)
    {
	struct invoke_iface_t *clone = s_itr->clone(s_itr, issues);

	if(!clone)
	{
	    goto error_free_resources;
	}
	
	DL_APPEND(statement_clones, clone);
    }

    /* Allocate working memory for the cloned statements */
    if(st_allocate_statements(statement_clones, issues) != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    /* Set up fb instance members */
    fv->variables = variable_table_clone;
    fv->statements = statement_clones;

    memset(&(fv->value), 0, sizeof(struct value_iface_t));
    fv->value.display = user_fb_value_display;
    fv->value.type_of = user_fb_value_type_of;
    fv->value.destroy = user_fb_value_destroy;
    fv->value.sub_variable = user_fb_value_sub_variable;
    fv->value.invoke_verify = user_fb_value_invoke_verify;
    fv->value.invoke_step = user_fb_value_invoke_step;
    fv->value.invoke_reset = user_fb_value_invoke_reset;

    return &(fv->value);
    
error_free_resources:
    /* free(fv); */
    /* struct variable_iface_t *variable_clones = NULL; */
    /* struct invoke_iface_t *statement_clones = NULL; */
    return NULL;
}

static int user_fb_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_fb_instance_t *fv =
	CONTAINER_OF(value_of, struct user_fb_instance_t, value);

    struct variable_iface_t *vitr = NULL;
    DL_FOREACH(fv->variables, vitr)
    {
	int reset_result = vitr->reset(vitr,
				       config,
				       issues);
	
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    struct invoke_iface_t *sitr = NULL;
    DL_FOREACH(fv->statements, sitr)
    {
	int reset_result = sitr->reset(sitr,
				       config,
				       issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

const static struct function_block_iface_t * user_fb_type_const_function_block_handle(
    const struct type_iface_t *self)
{
    struct user_function_block_t *fb =
	CONTAINER_OF(self, struct user_function_block_t, type);

    return &(fb->function_block);
}

static struct function_block_iface_t * user_fb_type_function_block_handle(
    struct type_iface_t *self)
{
    struct user_function_block_t *fb =
	CONTAINER_OF(self, struct user_function_block_t, type);

    return &(fb->function_block);
}

static st_bitflag_t user_fb_type_class(
    const struct type_iface_t *self)
{
    return FB_TYPE;
}

static void user_fb_type_destroy(
    struct type_iface_t *self)
{
    /* Function block not destroyed through type */
}

struct type_iface_t * st_new_user_function_block(
    char *identifier,
    const struct st_location_t *location,
    struct header_t *header,
    struct invoke_iface_t *statements,
    struct named_ref_pool_iface_t *type_refs,
    struct named_ref_pool_iface_t *var_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_function_block_t *ufb = NULL;
    struct st_location_t *ufb_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	ufb,
	struct user_function_block_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	ufb_location,
	location,
	issues,
	error_free_resources);

    ufb->identifier = identifier;
    ufb->location = ufb_location;
    ufb->type_refs = type_refs;
    ufb->var_refs = var_refs;
    ufb->header = header;
    ufb->statements = statements;
    
    memset(&(ufb->type), 0, sizeof(struct type_iface_t));
    memset(&(ufb->function_block), 0, sizeof(struct function_block_iface_t));

    ufb->type.identifier = ufb->identifier;
    ufb->type.location = ufb->location;
    ufb->type.create_value_of = user_fb_type_create_value_of;
    ufb->type.reset_value_of = user_fb_type_reset_value_of;
    ufb->type.ancestor = NULL;
    ufb->type.const_function_block_handle = user_fb_type_const_function_block_handle;
    ufb->type.function_block_handle = user_fb_type_function_block_handle;
    ufb->type.class = user_fb_type_class;
    ufb->type.destroy = user_fb_type_destroy;

    ufb->identifier = ufb->identifier;
    ufb->location = ufb->location;
    ufb->function_block.resolve_header_type_references = user_fb_resolve_header_type_references;
    ufb->function_block.check_dependencies = user_fb_check_dependencies;
    ufb->function_block.finalize_header = user_fb_finalize_header;
    ufb->function_block.finalize_statements = user_fb_finalize_statements;
    ufb->function_block.destroy = user_fb_destroy;
    
    return &(ufb->type);
    
error_free_resources:

    return NULL;
}
