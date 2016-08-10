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
#include <elements/array.h>
#include <elements/ivariable.h>
#include <util/macros.h>

#include <utlist.h>

struct qualified_part_t {
    char *identifier;
    struct st_location_t *location;
    struct array_index_t *index;
    struct variable_iface_t *variable;
    struct qualified_part_t *prev;
    struct qualified_part_t *next;
};

struct qualified_identifier_t {
    struct qualified_identifier_iface_t qid;
    struct variable_iface_t *target_variable;
    struct array_index_t *target_index;
    const struct value_iface_t *target;
    const char *target_name;
    struct qualified_part_t *path;
    int invoke_state;
    struct qualified_part_t *next_step_part;
    struct st_location_t *location;
};

/**************************************************************************/
/* Helper functions                                                       */
/**************************************************************************/
static int qualified_identifier_resolve_chain(
    struct qualified_identifier_t *qi,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_part_t *itr = NULL;
    DL_FOREACH(qi->path, itr)
    {
	if(itr->index != NULL)
	{
	    issues->begin_group(issues);
	    
	    if(itr->next)
	    {
		struct variable_iface_t *subvar = itr->variable->sub_variable(
		    itr->variable,
		    itr->index,
		    itr->next->identifier,
		    config,
		    issues);

		if(!subvar)
		{
		    issues->set_group_location(issues,
					       1,
					       itr->location);
		    issues->end_group(issues);
		    return ESSTEE_ERROR;
		}
		
		itr->next->variable = subvar;
	    }
	    else
	    {
		const struct value_iface_t *index_value = itr->variable->index_value(
		    itr->variable,
		    itr->index,
		    config,
		    issues);

		if(!index_value)
		{
		    issues->set_group_location(issues,
					       1,
					       itr->location);
		    issues->end_group(issues);
		    return ESSTEE_ERROR;
		}

		qi->target_variable = itr->variable;
		qi->target_index = itr->index;
		qi->target = index_value;
		qi->target_name = itr->variable->identifier;
	    }

	    issues->end_group(issues);
	}
	else if(itr->next)
	{
	    issues->begin_group(issues);
	    
	    struct variable_iface_t *subvar = itr->variable->sub_variable(
		itr->variable,
		NULL,
		itr->next->identifier,
		config,
		issues);

	    if(!subvar)
	    {
		issues->set_group_location(issues,
					   1,
					   itr->location);
		issues->end_group(issues);
		return ESSTEE_ERROR;
	    }

	    issues->end_group(issues);
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
static int qualified_identifier_verify(
    const struct qualified_identifier_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(self->constant_reference == ESSTEE_TRUE)
    {
	struct qualified_identifier_t *qi =
	    CONTAINER_OF(self, struct qualified_identifier_t, qid);

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
    if(self->constant_reference == ESSTEE_FALSE)
    {
	struct qualified_identifier_t *qi =
	    CONTAINER_OF(self, struct qualified_identifier_t, qid);

        struct qualified_part_t *itr = qi->next_step_part;
	struct array_index_t *index_itr = NULL;

	for(; itr != NULL; itr = itr->next)
	{
	    int skipped = 0;
	    DL_FOREACH(itr->index, index_itr)
	    {
		if(skipped >= qi->invoke_state)
		{
		    break;
		}

		skipped++;
	    }

	    for(; index_itr != NULL; index_itr = index_itr->next)
	    {
		if(index_itr->index_expression->invoke.step)
		{
		    /* Push to call stack (to be stepped), when stepping is
		     * complete, index has been evaluated (=increase state) */
		    qi->invoke_state++;
		    cursor->switch_current(cursor,
					   &(index_itr->index_expression->invoke),
					   config,
					   issues);
		    return INVOKE_RESULT_IN_PROGRESS;
		}
		else
		{
		    qi->invoke_state++;
		}
	    }

	    qi->next_step_part = itr->next;
	    qi->invoke_state = 0;
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
    for(itr = qi->path; itr != NULL; itr = itr->next)
    {
	if(itr->index && itr->index->index_expression->invoke.reset)
	{
	    int reset_result = itr->index->index_expression->invoke.reset(
		&(itr->index->index_expression->invoke),
		config,
		issues);

	    if(reset_result != ESSTEE_OK)
	    {
		return reset_result;
	    }
	}
    }

    qi->invoke_state = 0;
    qi->next_step_part = NULL;

    return ESSTEE_OK;
}

static int qualified_identifier_allocate(
    struct qualified_identifier_iface_t *self,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *qi =
	CONTAINER_OF(self, struct qualified_identifier_t, qid);

    struct qualified_part_t *itr = NULL;
    for(itr = qi->path; itr != NULL; itr = itr->next)
    {
	if(itr->index && itr->index->index_expression->invoke.allocate)
	{
	    int allocate_result = itr->index->index_expression->invoke.allocate(
		&(itr->index->index_expression->invoke),
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

    struct qualified_identifier_t *copy = NULL;

    ALLOC_OR_ERROR_JUMP(
	copy,
	struct qualified_identifier_t,
	issues,
	error_free_resources);

    struct qualified_part_t *path_copy = NULL;
    struct qualified_part_t *part_copy = NULL;
    struct qualified_part_t *part_itr = NULL;

    DL_FOREACH(qi->path, part_itr)
    {
	ALLOC_OR_ERROR_JUMP(
	    part_copy,
	    struct qualified_part_t,
	    issues,
	    error_free_resources);

	memcpy(part_copy, part_itr, sizeof(struct qualified_part_t));

	DL_APPEND(path_copy, part_copy);
    }

    copy->path = part_copy;
    
error_free_resources:
    DL_FOREACH_SAFE(path_copy, part_itr, part_copy)
    {
	free(part_itr);
    }
    free(copy);
    return NULL;
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
    struct qualified_identifier_iface_t *self)
{
    struct qualified_identifier_t *qi =
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

/* Target modificaton check */
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

/* Destructor */
static void qualified_identifier_destroy(
    struct qualified_identifier_iface_t *self)
{
    /* TODO: qualified identifier destructor */
}

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
    
    if(qi->path->variable)
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
/* Public interface                                                       */
/**************************************************************************/
struct qualified_part_t * st_extend_qualified_path(
    struct qualified_part_t *path,
    char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_part_t *part = NULL;
    struct st_location_t *part_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	part,
	struct qualified_part_t,
	issues,
	error_free_resources);
       
    LOCDUP_OR_ERROR_JUMP(
	part_location,
	location,
	issues,
	error_free_resources);

    part->identifier = identifier;
    part->location = part_location;
    part->index = NULL;

    DL_APPEND(path, part);
    
    return path;
    
error_free_resources:
    free(part);
    free(part_location);
    return NULL;
}

struct qualified_part_t * st_extend_qualified_path_by_index(
    struct qualified_part_t *path,
    struct array_index_t *array_index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_part_t *last_part = NULL;
    for(last_part = path; last_part->next != NULL; last_part = last_part->next);

    last_part->index = array_index;

    return path;
}

void st_destroy_qualified_path(
    struct qualified_part_t *path)
{
    /* TODO: qualified path destroy */
}

struct qualified_identifier_iface_t * st_create_qualified_identifier(
    char *base,
    const struct st_location_t *base_location,
    struct qualified_part_t *inner_path,
    struct named_ref_pool_iface_t *var_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_part_t *part = NULL;
    struct st_location_t *part_location = NULL;
    struct qualified_identifier_t *qi;

    /* Set up the base part of the path */
    ALLOC_OR_ERROR_JUMP(
	part,
	struct qualified_part_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	part_location,
	base_location,
	issues,
	error_free_resources);

    part->identifier = base;
    part->location = part_location;

    DL_PREPEND(inner_path, part);

    /* Set up the qualified identifier */
    ALLOC_OR_ERROR_JUMP(
	qi,
	struct qualified_identifier_t,
	issues,
	error_free_resources);

    int ref_result = var_refs->add(
	var_refs,
	base,
	qi,
	base_location,
	qualified_identifier_base_resolved,
	issues);

    if(ref_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    /* Check whether the reference is run-time constant */
    int constant_ref = ESSTEE_TRUE;
    struct qualified_part_t *itr = NULL;
    for(itr = inner_path; itr != NULL; itr = itr->next)
    {
	struct array_index_t *index_itr;
	for(index_itr = itr->index; index_itr != NULL; index_itr = index_itr->next)
	{
	    if(index_itr->index_expression->invoke.step || index_itr->index_expression->clone)
	    {
		constant_ref = ESSTEE_FALSE;
		break;
	    }
	}

	if(constant_ref != ESSTEE_TRUE)
	{
	    break;
	}
    }

    qi->path = inner_path;
    qi->target_name = NULL;
    
    /* Set up interface functions */
    memset(&(qi->qid), 0, sizeof(struct qualified_identifier_iface_t));
    qi->qid.constant_reference = constant_ref;
    qi->qid.location = qi->path->location;
    
    qi->qid.verify = qualified_identifier_verify;
    qi->qid.step = qualified_identifier_step;
    qi->qid.reset = qualified_identifier_reset;
    qi->qid.allocate = qualified_identifier_allocate;
    qi->qid.clone = qualified_identifier_clone;

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
    
    return &(qi->qid);

error_free_resources:
    free(part);
    free(part_location);
    free(qi);
    return NULL;
}

struct qualified_identifier_iface_t * st_create_qualified_identifier_by_index(
    char *base,
    const struct st_location_t *base_location,
    struct array_index_t *array_index,
    struct named_ref_pool_iface_t *var_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_iface_t *qid =
	st_create_qualified_identifier(base,
				       base_location,
				       NULL,
				       var_refs,
				       config,
				       issues);

    if(!qid)
    {
	return NULL;
    }

    struct qualified_identifier_t *qi =
	CONTAINER_OF(qid, struct qualified_identifier_t, qid);

    qi->path->index = array_index;
    if(array_index->index_expression->invoke.step || array_index->index_expression->clone)
    {
	qi->qid.constant_reference = ESSTEE_FALSE;
    }

    return &(qi->qid);
}
