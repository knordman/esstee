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

#include <elements/user_functions.h>
#include <util/macros.h>
#include <linker/linker.h>

struct user_function_t {
    struct function_iface_t function;

    struct header_t *header;    
    struct variable_t result;

    struct named_ref_pool_iface_t *type_refs;
    struct named_ref_pool_iface_t *var_refs;
    
    struct invoke_iface_t *statements;

    struct function_t *prev;
    struct function_t *next;
    char *identifier;

    UT_hash_handle hh;
};

static int user_function_finalize_header(
    struct function_iface_t *self,
    struct type_iface_t *global_type_table,
    struct variable_t *global_var_table,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_function_t *uf =
	CONTAINER_OF(self, struct user_function_t, function);

    uf->type_refs->reset_resolved(uf->type_refs);
    uf->var_refs->reset_resolved(uf->var_refs);

    /* Resolve type references in the function */
    /* First from types defined by the function */
    if(uf->header && uf->header->types)
    {
	st_resolve_type_refs(uf->type_refs, uf->header->types);
    }

    /* And then any remaining type references by global types */
    st_resolve_type_refs(uf->type_refs, global_type_table);

    /* Trigger callbacks for type references (e.g. those registered by
     * variables) */
    int callbacks_result = uf->type_refs->trigger_resolve_callbacks(uf->type_refs,
								    config,
								    issues);
    if(callbacks_result != ESSTEE_OK)
    {
	return callbacks_result;
    }

    /* At this point, all types for variables have been resolved. The
     * next step is to create the values of the variables. */
    if(uf->header && uf->header->variables)
    {
	/* Before createing the variable values check that variables are
	 * properly defined */
	int invalid_variables = 0;
	struct variable_t *itr = NULL;
	DL_FOREACH(uf->header->variables, itr)
	{
	    st_bitflag_t var_class = itr->class;
	    ST_CLEAR_FLAGS(var_class, INPUT_VAR_CLASS|CONSTANT_VAR_CLASS|EXTERNAL_VAR_CLASS);
	    if(var_class != 0)
	    {
		const char *message = issues->build_message(
		    issues,
		    "variable '%s' in function '%s' has an invalid specifier",
		    itr->identifier,
		    uf->function.identifier);
		
		issues->new_issue_at(
		    issues,
		    message,
		    ESSTEE_TYPE_ERROR,
		    1,
		    itr->identifier_location);
		
		invalid_variables = 1;
	    }
	}
 	if(invalid_variables)
	{
	    return ESSTEE_ERROR;
	}

	/* Check that there is no explicit variable by the same name
	 * as the implict result variable */
	struct variable_t *found = NULL;
	HASH_FIND_STR(uf->header->variables, uf->identifier, found);
	if(found)
	{
	    const char *message = issues->build_message(
		issues,
		"implicit result variable '%s' explicitly defined",
		itr->identifier);

	    issues->new_issue_at(
		issues,
		message,
		ESSTEE_LINK_ERROR,
		1,
		found->identifier_location);
	}

	/* Check restrictions on result variable? */
	
	int create_result = st_create_header_variable_values(uf->header->variables,
							     config,
							     issues);
	if(create_result != ESSTEE_OK)
	{
	    return create_result;
	}
    }

    /* Create result variable */
    uf->result.value = uf->result.type->create_value_of(uf->result.type,
							config,
							issues);
    if(!uf->result.value)
    {
	return ESSTEE_ERROR;
    }
    
    /* Resolve variable references */
    st_resolve_pou_var_refs(uf->var_refs, global_var_table, uf->header->variables);

    return uf->var_refs->trigger_resolve_callbacks(uf->var_refs,
						   config,
						   issues);
}

static int user_function_finalize_statements(
    struct function_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_function_t *uf =
	CONTAINER_OF(self, struct user_function_t, function);

    /* Allocate working memory for statements */
    int allocate_result = st_allocate_statements(uf->statements,
						 issues);
    if(allocate_result != ESSTEE_OK)
    {
	return allocate_result;
    }

    return st_verify_statements(uf->statements,
				config,
				issues);
}

static int user_function_verify_invoke(
    struct function_iface_t *self,
    struct invoke_parameter_t *parameters,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_function_t *uf =
	CONTAINER_OF(self, struct user_function_t, function);
    
    return st_verify_invoke_parameters(parameters,
				       uf->header->variables,
				       config,
				       issues);
}

static int user_function_step(
    struct function_iface_t *self,
    struct invoke_parameter_t *parameters,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_function_t *uf =
	CONTAINER_OF(self, struct user_function_t, function);
    
    st_switch_current(cursor, uf->statements, config, issues);
    return INVOKE_RESULT_IN_PROGRESS;
}

static struct value_iface_t * user_function_result_value(
    struct function_iface_t *self)
{
    struct user_function_t *uf =
	CONTAINER_OF(self, struct user_function_t, function);

    return uf->result.value;
}

static int user_function_reset(
    struct function_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_function_t *uf =
	CONTAINER_OF(self, struct user_function_t, function);

    if(uf->header && uf->header->variables)
    {
	struct variable_t *itr = NULL;
	DL_FOREACH(uf->header->variables, itr)
	{
	    int reset_result = itr->type->reset_value_of(itr->type,
							 itr->value,
							 config,
							 issues);
	    if(reset_result != ESSTEE_OK)
	    {
		return reset_result;
	    }
	}
    }

    return ESSTEE_OK;
}

static int user_function_return_type_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_function_t *uf =
	(struct user_function_t *)referrer;
    
    if(target == NULL)
    {
	const char *message = issues->build_message(
	    issues,
	    "return type '%s' undefined in function '%s'",
	    identifier,
	    uf->function.identifier);

	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_LINK_ERROR,
	    1,
	    location);
	
	return ESSTEE_ERROR;
    }

    uf->result.type = (struct type_iface_t *)target;

    return ESSTEE_OK;
}

struct function_iface_t * st_new_user_function(
    char *identifier,
    const struct st_location_t *location,
    char *return_type_identifier,
    const struct st_location_t *return_type_identifier_location,
    struct header_t *header,
    struct named_ref_pool_iface_t *global_type_refs,
    struct named_ref_pool_iface_t *type_refs,
    struct named_ref_pool_iface_t *global_var_refs,
    struct named_ref_pool_iface_t *var_refs,
    struct invoke_iface_t *statements,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_function_t *uf = NULL;
    struct st_location_t *uf_location = NULL;

    int header_valid = st_create_header_tables(header, issues);

    if(!header_valid)
    {
	goto error_free_resources;
    }
    
    ALLOC_OR_ERROR_JUMP(
	uf,
	struct user_function_t,
	issues,
	error_free_resources);
    
    LOCDUP_OR_ERROR_JUMP(
	uf_location,
	location,
	issues,
	error_free_resources);

    uf->header = header;
    uf->result.identifier = identifier;
    uf->result.class = OUTPUT_VAR_CLASS;
    uf->result.type = NULL;
    uf->result.identifier_location = uf_location;
    
    uf->type_refs = type_refs;
    uf->var_refs = var_refs;

    uf->statements = statements;
        
    int ref_result = global_type_refs->add(global_type_refs,
					   return_type_identifier,
					   uf,
					   return_type_identifier_location,
					   user_function_return_type_resolved,
					   issues);

    if(ref_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    memset(&(uf->function), 0, sizeof(struct function_iface_t));
    
    uf->function.location = uf_location;
    uf->function.identifier = identifier;
    
    uf->function.finalize_header = user_function_finalize_header;
    uf->function.finalize_statements = user_function_finalize_statements;
    uf->function.verify_invoke = user_function_verify_invoke;
    uf->function.step = user_function_step;
    uf->function.reset = user_function_reset;
    uf->function.result_value = user_function_result_value;

    return &(uf->function);
    
error_free_resources:
    /* TODO: determine what to destroy */
    free(uf);
    free(uf_location);
    return NULL;
}
