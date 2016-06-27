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

#include <statements/simple_assignment.h>
#include <elements/ivariable.h>
#include <util/macros.h>

/**************************************************************************/
/* Invoke interface                                                       */
/**************************************************************************/
struct simple_assignment_statement_t {
    struct invoke_iface_t invoke;
    struct variable_iface_t *lhs;
    struct st_location_t *lhs_location;
    struct st_location_t *location;
    struct expression_iface_t *rhs;
    int invoke_state;
};

static int assignment_statement_simple_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct simple_assignment_statement_t *sa =
	CONTAINER_OF(self, struct simple_assignment_statement_t, invoke);

    int rhs_verify_result = ESSTEE_OK;
    if(sa->rhs->invoke.verify)
    {
	rhs_verify_result = sa->rhs->invoke.verify(&(sa->rhs->invoke),
						   config,
						   issues);
    }

    if(rhs_verify_result != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    const struct value_iface_t *rhs_value = sa->rhs->return_value(sa->rhs);
    
    issues->begin_group(issues);
    int assignable_result = sa->lhs->assignable_from(sa->lhs,
						     rhs_value,
						     config,
						     issues);
    if(assignable_result != ESSTEE_TRUE)
    {
	issues->new_issue(issues,
			  "assignment of variable '%s' impossible",
			  ESSTEE_CONTEXT_ERROR,
			  sa->lhs->identifier);

	issues->set_group_location(issues,
				   2,
				   sa->lhs_location,
				   sa->rhs->invoke.location);
    }
    issues->end_group(issues);
    
    if(assignable_result != ESSTEE_TRUE)
    {
	return ESSTEE_ERROR;
    }
    
    return ESSTEE_OK;
}

static int assignment_statement_simple_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct simple_assignment_statement_t *sa =
	CONTAINER_OF(self, struct simple_assignment_statement_t, invoke);

    switch(sa->invoke_state)
    {
    case 0:
	if(sa->rhs->invoke.step)
	{
	    sa->invoke_state = 1;
	    cursor->switch_current(cursor,
				   &(sa->rhs->invoke),
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 1: {
	const struct value_iface_t *rhs_value = sa->rhs->return_value(sa->rhs);
	
	issues->begin_group(issues);
	int assign_result = sa->lhs->assign(sa->lhs,
					    rhs_value,
					    config,
					    issues);

	if(assign_result != ESSTEE_OK)
	{
	    issues->new_issue(issues,
			      "assignment of variable '%s' failed",
			      ESSTEE_CONTEXT_ERROR,
			      sa->lhs->identifier);
	
	    issues->set_group_location(issues,
				       2,
				       sa->lhs_location,
				       sa->rhs->invoke.location);
	}
	issues->end_group(issues);

	if(assign_result != ESSTEE_OK)
	{
	    return INVOKE_RESULT_ERROR;
	}
    }
    }

    return INVOKE_RESULT_FINISHED;
}

static int assignment_statement_simple_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct simple_assignment_statement_t *sa =
	CONTAINER_OF(self, struct simple_assignment_statement_t, invoke);

    if(sa->rhs->invoke.allocate)
    {
	return sa->rhs->invoke.allocate(
	    &(sa->rhs->invoke),
	    issues);
    }
    
    return ESSTEE_OK;
}

static void assignment_statement_simple_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: destructor */
}

static void assignment_statement_simple_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: destructor */
}

static struct invoke_iface_t * assignment_statement_simple_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct simple_assignment_statement_t *sa =
	CONTAINER_OF(self, struct simple_assignment_statement_t, invoke);

    struct simple_assignment_statement_t *copy = NULL;
    struct expression_iface_t *rhs_copy = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct simple_assignment_statement_t,
	issues,
	error_free_resources);
    
    memcpy(copy, sa, sizeof(struct simple_assignment_statement_t));

    if(sa->rhs->clone)
    {
	ALLOC_OR_ERROR_JUMP(
	    rhs_copy,
	    struct expression_iface_t,
	    issues,
	    error_free_resources);

	rhs_copy = sa->rhs->clone(sa->rhs, issues);
	if(!rhs_copy)
	{
	    goto error_free_resources;
	}

	copy->rhs = rhs_copy;
    }
    
    copy->invoke.destroy = assignment_statement_simple_clone_destroy;

    return &(copy->invoke);

error_free_resources:
    return NULL;
}

static int assignment_statement_simple_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct simple_assignment_statement_t *sa =
	CONTAINER_OF(self, struct simple_assignment_statement_t, invoke);

    sa->invoke_state = 0;

    if(sa->rhs->invoke.reset)
    {
	int reset_result = sa->rhs->invoke.reset(&(sa->rhs->invoke),
						 config,
						 issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

/**************************************************************************/
/* Linker callbacks                                                       */
/**************************************************************************/
static int simple_assignment_variable_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(target == NULL)
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
    
    struct simple_assignment_statement_t *sa =
	(struct simple_assignment_statement_t *)referrer;

    sa->lhs = (struct variable_iface_t *)target;

    return ESSTEE_OK;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct invoke_iface_t * st_create_assignment_statement_simple(
    char *identifier,
    const struct st_location_t *identifier_location,
    struct expression_iface_t *assignment,
    const struct st_location_t *location,
    struct named_ref_pool_iface_t *var_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct simple_assignment_statement_t *sa = NULL;
    struct st_location_t *sa_location = NULL;
    struct st_location_t *lhs_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	sa,
	struct simple_assignment_statement_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	sa_location,
	location,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	lhs_location,
	identifier_location,
	issues,
	error_free_resources);

    int ref_result = var_refs->add(
	var_refs,
	identifier,
	sa,
	identifier_location,
	simple_assignment_variable_resolved,
	issues);

    if(ref_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    sa->location = sa_location;
    sa->lhs_location = lhs_location;
    sa->lhs = NULL;
    sa->rhs = assignment;
    
    memset(&(sa->invoke), 0, sizeof(struct invoke_iface_t));
    sa->invoke.location = sa->location;
    sa->invoke.step = assignment_statement_simple_step;
    sa->invoke.verify = assignment_statement_simple_verify;
    sa->invoke.allocate = assignment_statement_simple_allocate;
    sa->invoke.clone = assignment_statement_simple_clone;
    sa->invoke.reset = assignment_statement_simple_reset;
    sa->invoke.destroy = assignment_statement_simple_destroy;
    
    return &(sa->invoke);
    
error_free_resources:
    free(sa);
    free(sa_location);
    free(lhs_location);
    return NULL;
}
