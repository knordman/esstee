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
#include <elements/variables.h>
#include <util/macros.h>

#include <utlist.h>

struct qualified_part_t {
    char *identifier;
    struct st_location_t *location;
    struct array_index_t *index;
    struct variable_t *variable;
    struct qualified_part_t *prev;
    struct qualified_part_t *next;
};

struct qualified_identifier_t {
    struct qualified_identifier_iface_t qid;
    struct value_iface_t *target;
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
	    if(!itr->variable->value->index)
	    {
		const char *message = issues->build_message(
		    issues,
		    "variable '%s' is not indexable");
		
		issues->new_issue_at(
		    issues,
		    message,
		    ESSTEE_CONTEXT_ERROR,
		    1,
		    itr->location);

		return ESSTEE_ERROR;
	    }
	    
	    struct value_iface_t *array_target = itr->variable->value->index(
		itr->variable->value,
		itr->index,
		config,
		issues);

	    if(!array_target)
	    {
		return ESSTEE_ERROR;
	    }

	    if(itr->next)
	    {
		if(!array_target->sub_variable)
		{
		    const char *message = issues->build_message(
			issues,
			"an element of array variable '%s' does not have any sub-variables",
			itr->identifier);
		
		    issues->new_issue_at(
			issues,
			message,
			ESSTEE_CONTEXT_ERROR,
			1,
			itr->location);
		
		    return ESSTEE_ERROR;
		}

		issues->begin_group(issues);
		struct variable_t *subvar =
		    array_target->sub_variable(array_target,
					       itr->next->identifier,
					       config,
					       issues);
		if(!subvar)
		{
		    issues->set_group_location(issues,
					       1,
					       itr->location);
		}
		issues->end_group(issues);

		itr->next->variable = subvar;
	    }
	    else
	    {
		qi->target = array_target;
		qi->target_name = itr->variable->identifier;
	    }
	}
	else if(itr->next)
	{
	    if(!itr->variable->value->sub_variable)
	    {
		const char *message = issues->build_message(
		    issues,
		    "variable '%s' does not have any sub-variables",
		    itr->identifier);
		
		issues->new_issue_at(
		    issues,
		    message,
		    ESSTEE_CONTEXT_ERROR,
		    1,
		    itr->location);
		
		return ESSTEE_ERROR;
	    }

	    issues->begin_group(issues);
	    struct variable_t *subvar =
		itr->variable->value->sub_variable(itr->variable->value,
						   itr->next->identifier,
						   config,
						   issues);
	    if(!subvar)
	    {
		issues->set_group_location(issues,
					   1,
					   itr->next->location);
	    }
	    
	    issues->end_group(issues);
	    
	    if(!subvar)
	    {
		return ESSTEE_ERROR;
	    }
	    else
	    {
		itr->next->variable = subvar;
	    }
	}
	else
	{
	    qi->target = itr->variable->value;
	    qi->target_name = itr->variable->identifier;
	}
    }
    
    return ESSTEE_OK;
}

/**************************************************************************/
/* Qualified identifier interface                                         */
/**************************************************************************/
static int qualified_identifier_verify(
    struct qualified_identifier_iface_t *self,
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

static int qualified_identifier_set_base(
    struct qualified_identifier_iface_t *self,
    struct variable_t *base,
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

    return (qi->path->variable) ? qi->path->variable->identifier : NULL;
}

static struct value_iface_t * qualified_identifier_target(
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

    qi->path->variable = (struct variable_t *)target;

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
	if(itr->index)
	{
	    if(itr->index->index_expression->invoke.step || itr->index->index_expression->clone)
	    {
		constant_ref = ESSTEE_FALSE;
	    }
	}
    }
    qi->qid.constant_reference = constant_ref;

    qi->path = inner_path;
    qi->target_name = NULL;
    
    /* Set up interface functions */
    memset(&(qi->qid), 0, sizeof(struct qualified_identifier_iface_t));
    qi->qid.location = qi->path->location;
    qi->qid.reset = qualified_identifier_reset;
    qi->qid.verify = qualified_identifier_verify;
    qi->qid.allocate = qualified_identifier_allocate;
    qi->qid.clone = qualified_identifier_clone;
    qi->qid.step = qualified_identifier_step;
    qi->qid.set_base = qualified_identifier_set_base;
    qi->qid.base_identifier = qualified_identifier_base_identifier;
    qi->qid.target = qualified_identifier_target;
    qi->qid.target_name = qualified_identifier_target_name;

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
