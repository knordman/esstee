/*
Copyright (C) 2016 Kristian Nordman

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

#include <elements/qualified_identifier.h>
#include <elements/ivariable.h>
#include <util/macros.h>

#include <utlist.h>

struct qualified_part_t {
    char *identifier;
    struct st_location_t *location;
    struct array_index_iface_t *index;
    struct variable_iface_t *variable;
    struct qualified_part_t *prev;
    struct qualified_part_t *next;
};

struct qualified_identifier_t {
    struct qualified_identifier_iface_t qid;
    struct variable_iface_t *target_variable;
    struct array_index_iface_t *target_index;
    const struct value_iface_t *target;
    const char *target_name;
    struct qualified_part_t *path;
    struct named_ref_pool_iface_t *base_context;
    int explicit_base;
    struct qualified_part_t *invoke_state_part;
    int constant_reference;
    struct st_location_t location;
};

/**************************************************************************/
/* Linker callbacks                                                       */
/**************************************************************************/
static int qualified_identifier_base_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	(struct qualified_identifier_t *)referrer;
    
    if(qi->explicit_base == ESSTEE_TRUE)
    {
	/* If base has been set explicitly, don't use link result */
	return ESSTEE_OK;
    }
    else if(!target)
    {
	const char *message = issues->build_message(
	    issues,
	    "reference to undefined variable '%s'",
	    identifier);

	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_LINK_ERROR,
	    1,
	    location);

	return ESSTEE_ERROR;
    }

    qi->path->variable = (struct variable_iface_t *)target;

    return ESSTEE_OK;
}

/**************************************************************************/
/* Helper functions                                                       */
/**************************************************************************/
static int qualified_identifier_resolve_chain(
    struct qualified_identifier_t *qi,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_part_t *itr = NULL;
    struct issue_group_iface_t *ig = NULL;
    
    DL_FOREACH(qi->path, itr)
    {
	if(itr->index != NULL)
	{
	    if(itr->next)
	    {
		ig = issues->open_group(issues);
		
		struct variable_iface_t *subvar = itr->variable->sub_variable(
		    itr->variable,
		    itr->index,
		    itr->next->identifier,
		    config,
		    issues);

		ig->close(ig);

		if(!subvar)
		{
		    const char *message = issues->build_message(
			issues,
			"invalid reference to subvariable in variable '%s'",
			itr->variable->identifier);
		    
		    ig->main_issue(ig,
				   message,
				   ESSTEE_LINK_ERROR,
				   1,
				   itr->location);
		    
		    return ESSTEE_ERROR;
		}
		
		itr->next->variable = subvar;
	    }
	    else
	    {
		ig = issues->open_group(issues);

		const struct value_iface_t *index_value = itr->variable->index_value(
		    itr->variable,
		    itr->index,
		    config,
		    issues);

		ig->close(ig);

		if(!index_value)
		{
		    const char *message = issues->build_message(
			issues,
			"invalid array index for variable '%s'",
			itr->variable->identifier);

		    ig->main_issue(ig,
				   message,
				   ESSTEE_LINK_ERROR,
				   1,
				   itr->location);

		    return ESSTEE_ERROR;
		}

		qi->target_variable = itr->variable;
		qi->target_index = itr->index;
		qi->target = index_value;
		qi->target_name = itr->variable->identifier;
	    }
	}
	else if(itr->next)
	{
	    ig = issues->open_group(issues);
	    
	    struct variable_iface_t *subvar = itr->variable->sub_variable(
		itr->variable,
		NULL,
		itr->next->identifier,
		config,
		issues);

	    ig->close(ig);

	    if(!subvar)
	    {
		ig->main_issue(ig,
			       "invalid reference",
			       ESSTEE_LINK_ERROR,
			       1,
			       itr->location);

		return ESSTEE_ERROR;
	    }

	    itr->next->variable = subvar;
	}
	else
	{
	    qi->target_variable = itr->variable;
	    qi->target_index = NULL;
	    qi->target = itr->variable->value(itr->variable);
	    qi->target_name = itr->variable->identifier;
	}
    }
    
    return ESSTEE_OK;
}

/**************************************************************************/
/* Qualified identifier interface                                         */
/**************************************************************************/
static void qualified_identifier_destroy(
    struct qualified_identifier_iface_t *self)
{
    /* TODO: qualified identifier destructor */
}

static void qualified_identifier_destroy_clone(
    struct qualified_identifier_iface_t *self)
{
    /* TODO: qualified identifier destructor */
}

static int qualified_identifier_extend_by_index(
    struct qualified_identifier_iface_t *self,
    char *identifier,
    const struct st_location_t *identifier_location,
    struct array_index_iface_t *index,
    const struct st_location_t *qid_location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    struct qualified_part_t *part = NULL;
    struct st_location_t *part_location = NULL;
    ALLOC_OR_ERROR_JUMP(
	part,
	struct qualified_part_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	part_location,
	identifier_location,
	issues,
	error_free_resources);

    memcpy(&(qi->location), qid_location, sizeof(struct st_location_t));
    
    part->identifier = identifier;
    part->location = part_location;
    part->index = index;
    part->variable = NULL;

    if(index->constant_reference(index) != ESSTEE_TRUE)
    {
	qi->constant_reference = ESSTEE_FALSE;
    }

    if(!qi->path)
    {
	qi->qid.location = part->location;
	
	int add_base_ref_result = qi->base_context->add(
	    qi->base_context,
	    part->identifier,
	    qi,
	    part->location,
	    qualified_identifier_base_resolved,
	    issues);

	if(add_base_ref_result != ESSTEE_OK)
	{
	    goto error_free_resources;
	}
    }
    
    DL_APPEND(qi->path, part);

    return ESSTEE_OK;
    
error_free_resources:
    return ESSTEE_ERROR;
}

static int qualified_identifier_extend(
    struct qualified_identifier_iface_t *self,
    char *identifier,
    const struct st_location_t *identifier_location,
    const struct st_location_t *qid_location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    struct qualified_part_t *part = NULL;
    struct st_location_t *part_location = NULL;
    ALLOC_OR_ERROR_JUMP(
	part,
	struct qualified_part_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	part_location,
	identifier_location,
	issues,
	error_free_resources);

    memcpy(&(qi->location), qid_location, sizeof(struct st_location_t));
    
    part->identifier = identifier;
    part->location = part_location;
    part->index = NULL;
    part->variable = NULL;

    if(!qi->path)
    {
	qi->qid.location = part->location;
	
	int add_base_ref_result = qi->base_context->add(
	    qi->base_context,
	    part->identifier,
	    qi,
	    part->location,
	    qualified_identifier_base_resolved,
	    issues);

	if(add_base_ref_result != ESSTEE_OK)
	{
	    goto error_free_resources;
	}
    }

    DL_APPEND(qi->path, part);

    return ESSTEE_OK;
    
error_free_resources:
    free(part);
    return ESSTEE_ERROR;
}

static int qualified_identifier_verify(
    const struct qualified_identifier_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);
		     
    if(qi->constant_reference == ESSTEE_TRUE)
    {
	int resolve_result = qualified_identifier_resolve_chain(qi,
								config,
								issues);
	return resolve_result;
    }

    return ESSTEE_OK;
}

static int qualified_identifier_step(
    struct qualified_identifier_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);
    
    if(qi->constant_reference == ESSTEE_FALSE)
    {
	while(qi->invoke_state_part)
	{
	    struct qualified_part_t *current_part = qi->invoke_state_part;
	    qi->invoke_state_part = qi->invoke_state_part->next;	    

	    if(current_part->index)
	    {
		int index_step_result = current_part->index->step(current_part->index,
								  cursor,
								  time,
								  config,
								  issues);
		
		if(index_step_result != INVOKE_RESULT_FINISHED)
		{
		    return index_step_result;
		}
	    }
	}

	int resolve_result = qualified_identifier_resolve_chain(qi,
								config,
								issues);
	if(resolve_result != ESSTEE_OK)
	{
	    return INVOKE_RESULT_ERROR;
        }
    }
    
    return INVOKE_RESULT_FINISHED;
}

static int qualified_identifier_reset(
    struct qualified_identifier_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    struct qualified_part_t *itr = NULL;
    DL_FOREACH(qi->path, itr)
    {
	if(itr->index)
	{
	    int reset_result = itr->index->reset(itr->index,
						 config,
						 issues);

	    if(reset_result != ESSTEE_OK)
	    {
		return reset_result;
	    }
	}
    }

    qi->invoke_state_part = qi->path;
	
    return ESSTEE_OK;
}

static int qualified_identifier_allocate(
    struct qualified_identifier_iface_t *self,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    struct qualified_part_t *itr = NULL;
    DL_FOREACH(qi->path, itr)
    {
	if(itr->index)
	{
	    int allocate_result = itr->index->allocate(itr->index,
						       issues);
	    if(allocate_result != ESSTEE_OK)
	    {
		return allocate_result;
	    }
	}
    }
    
    return ESSTEE_OK;
}
    
static struct qualified_identifier_iface_t * qualified_identifier_clone(
    struct qualified_identifier_iface_t *self,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    struct qualified_identifier_t *clone = NULL;
    struct qualified_part_t *path_clone = NULL;
    struct qualified_part_t *part_clone = NULL;
    struct array_index_iface_t *index_clone = NULL;

    struct qualified_part_t *itr = NULL;
    DL_FOREACH(qi->path, itr)
    {
	ALLOC_OR_ERROR_JUMP(
	    part_clone,
	    struct qualified_part_t,
	    issues,
	    error_free_resources);

	memcpy(part_clone, itr, sizeof(struct qualified_part_t));

	if(itr->index)
	{
	    index_clone = itr->index->clone(itr->index, issues);

	    if(!index_clone)
	    {
		goto error_free_resources;
	    }

	    part_clone->index = index_clone;
	}

	DL_APPEND(path_clone, part_clone);
    }

    /* Allocate space for the copy */
    ALLOC_OR_ERROR_JUMP(
	clone,
	struct qualified_identifier_t,
	issues,
	error_free_resources);

    memcpy(clone, qi, sizeof(struct qualified_identifier_t));

    clone->path = path_clone;
    clone->qid.destroy = qualified_identifier_destroy_clone;

    return &(clone->qid);
    
error_free_resources:
    DL_FOREACH_SAFE(path_clone, itr, part_clone)
    {
	if(itr->index)
	{
	    itr->index->destroy(itr->index);
	}
	free(itr);
    }
    free(clone);
    return NULL;
}

static int qualified_identifier_constant_reference(
    struct qualified_identifier_iface_t *self)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->constant_reference;
}

/* Path base interaction */
static int qualified_identifier_set_base(
    struct qualified_identifier_iface_t *self,
    struct variable_iface_t *base,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    qi->path->variable = base;
    qi->explicit_base = ESSTEE_TRUE;

    return ESSTEE_OK;
}

static const char * qualified_identifier_base_identifier(
    struct qualified_identifier_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->path->identifier;
}

/* Target invoke */
static int qualified_identifier_target_invoke_verify(
    const struct qualified_identifier_iface_t *self,
    const struct invoke_parameters_iface_t *parameters,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->invoke_verify(qi->target_variable,
					      qi->target_index,
					      parameters,
					      config,
					      issues);
}
    
static int qualified_identifier_target_invoke_step(
    struct qualified_identifier_iface_t *self,
    const struct invoke_parameters_iface_t *parameters,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->invoke_step(qi->target_variable,
					    qi->target_index,
					    parameters,
					    cursor,
					    time,
					    config,
					    issues);
}

static int qualified_identifier_target_invoke_reset(
    struct qualified_identifier_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->invoke_reset(qi->target_variable,
					     qi->target_index,
					     config,
					     issues);
}

/* Target access */
static const struct value_iface_t * qualified_identifier_target_value(
    const struct qualified_identifier_iface_t *self)
{
    const struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);
    
    return qi->target;
}

static const char * qualified_identifier_target_name(
    struct qualified_identifier_iface_t *self)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_name;
}

/* Target modification check */
static int qualified_identifier_target_assignable_from(
    const struct qualified_identifier_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->assignable_from(qi->target_variable,
						qi->target_index,
						new_value,
						config,
						issues);
}

/* Target modification */
static int qualified_identifier_target_assign(
    struct qualified_identifier_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->assign(qi->target_variable,
				       qi->target_index,
				       new_value,
				       config,
				       issues);
}

static int qualified_identifier_target_not(
    struct qualified_identifier_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->not(qi->target_variable,
				    qi->target_index,
				    config,
				    issues);
}

static int qualified_identifier_target_negate(
    struct qualified_identifier_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->negate(qi->target_variable,
				       qi->target_index,
				       config,
				       issues);
}

static int qualified_identifier_target_xor(
    struct qualified_identifier_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->xor(qi->target_variable,
				    qi->target_index,
				    other_value,
				    config,
				    issues);
}

static int qualified_identifier_target_and(
    struct qualified_identifier_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->and(qi->target_variable,
				    qi->target_index,
				    other_value,
				    config,
				    issues);
}

static int qualified_identifier_target_or(
    struct qualified_identifier_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->or(qi->target_variable,
				   qi->target_index,
				   other_value,
				   config,
				   issues);
}

static int qualified_identifier_target_plus(
    struct qualified_identifier_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->plus(qi->target_variable,
				     qi->target_index,
				     other_value,
				     config,
				     issues);
}

static int qualified_identifier_target_minus(
    struct qualified_identifier_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->minus(qi->target_variable,
				      qi->target_index,
				      other_value,
				      config,
				      issues);
}

static int qualified_identifier_target_multiply(
    struct qualified_identifier_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->multiply(qi->target_variable,
					 qi->target_index,
					 other_value,
					 config,
					 issues);
}

static int qualified_identifier_target_divide(
    struct qualified_identifier_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->divide(qi->target_variable,
				       qi->target_index,
				       other_value,
				       config,
				       issues);
}

static int qualified_identifier_target_modulus(
    struct qualified_identifier_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->modulus(qi->target_variable,
					qi->target_index,
					other_value,
					config,
					issues);
}

static int qualified_identifier_target_to_power(
    struct qualified_identifier_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    return qi->target_variable->to_power(qi->target_variable,
					 qi->target_index,
					 other_value,
					 config,
					 issues);
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct qualified_identifier_iface_t * st_create_qualified_identifier(
    struct named_ref_pool_iface_t *variable_context,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi;

    ALLOC_OR_ERROR_JUMP(
	qi,
	struct qualified_identifier_t,
	issues,
	error_free_resources);

    qi->path = NULL;
    qi->target_name = NULL;
    qi->explicit_base = ESSTEE_FALSE;
    qi->constant_reference = ESSTEE_TRUE;
    qi->base_context = variable_context;

    memset(&(qi->location), 0, sizeof(struct st_location_t));
    
    /* Set up interface functions */
    memset(&(qi->qid), 0, sizeof(struct qualified_identifier_iface_t));
    qi->qid.extend_by_index = qualified_identifier_extend_by_index;
    qi->qid.extend = qualified_identifier_extend;
    
    qi->qid.constant_reference = qualified_identifier_constant_reference;
    
    qi->qid.verify = qualified_identifier_verify;
    qi->qid.step = qualified_identifier_step;
    qi->qid.reset = qualified_identifier_reset;
    qi->qid.allocate = qualified_identifier_allocate;
    qi->qid.clone = qualified_identifier_clone;
    qi->qid.constant_reference = qualified_identifier_constant_reference;

    qi->qid.set_base = qualified_identifier_set_base;
    qi->qid.base_identifier = qualified_identifier_base_identifier;

    qi->qid.target_invoke_verify = qualified_identifier_target_invoke_verify;
    qi->qid.target_invoke_step = qualified_identifier_target_invoke_step;
    qi->qid.target_invoke_reset = qualified_identifier_target_invoke_reset;
    
    qi->qid.target_value = qualified_identifier_target_value;
    qi->qid.target_name = qualified_identifier_target_name;
    qi->qid.target_assignable_from = qualified_identifier_target_assignable_from;
    
    qi->qid.target_assign = qualified_identifier_target_assign;
    qi->qid.target_not = qualified_identifier_target_not;
    qi->qid.target_negate = qualified_identifier_target_negate;
    qi->qid.target_xor = qualified_identifier_target_xor;
    qi->qid.target_and = qualified_identifier_target_and;
    qi->qid.target_or = qualified_identifier_target_or;
    qi->qid.target_plus = qualified_identifier_target_plus;
    qi->qid.target_minus = qualified_identifier_target_minus;
    qi->qid.target_multiply = qualified_identifier_target_multiply;
    qi->qid.target_divide = qualified_identifier_target_divide;
    qi->qid.target_modulus = qualified_identifier_target_modulus;
    qi->qid.target_to_power = qualified_identifier_target_to_power;

    qi->qid.destroy = qualified_identifier_destroy;

    qi->qid.location = &(qi->location);
    
    return &(qi->qid);

error_free_resources:
    free(qi);
    return NULL;
}
