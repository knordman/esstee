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

#include <statements/qualified_assignment.h>
#include <util/macros.h>

/**************************************************************************/
/* Invoke interface                                                       */
/**************************************************************************/
struct qualified_assignment_statement_t {
    struct invoke_iface_t invoke;
    struct st_location_t *location;
    struct qualified_identifier_iface_t *lhs;
    struct expression_iface_t *rhs;
    int invoke_state;
};

static int assignment_statement_qualified_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    int identifier_verify_result = qis->lhs->verify(qis->lhs,
						    config,
						    issues);
    if(identifier_verify_result != ESSTEE_OK)
    {
	return identifier_verify_result;
    }
    
    int rhs_verify_result = ESSTEE_OK;
    if(qis->rhs->invoke.verify)
    {
	rhs_verify_result = qis->rhs->invoke.verify(&(qis->rhs->invoke),
						    config,
						    issues);
    }
    if(rhs_verify_result != ESSTEE_OK)
    {
	return rhs_verify_result;
    }

    if(qis->lhs->constant_reference == ESSTEE_TRUE)
    {
	const struct value_iface_t *rhs_value = qis->rhs->return_value(qis->rhs);
	struct value_iface_t *lhs_target = qis->lhs->target(qis->lhs);
	
	if(!lhs_target->assignable_from)
	{
	    const char *message = issues->build_message(
		issues,
		"element '%s' cannot be assigned a new value",
		qis->lhs->target_name(qis->lhs));

	    issues->new_issue_at(
		issues,
		message,
		ESSTEE_CONTEXT_ERROR,
		1,
		qis->lhs->location);

	    return ESSTEE_ERROR;
	}

	issues->begin_group(issues);
	
	int assignable_result = lhs_target->assignable_from(lhs_target,
							    rhs_value,
							    config,
							    issues);
	if(assignable_result != ESSTEE_TRUE)
	{
	    issues->new_issue(issues,
			      "assignment of element '%s' was not successful",
			      ESSTEE_CONTEXT_ERROR,
			      qis->lhs->target_name(qis->lhs));

	    issues->set_group_location(issues,
				       2,
				       qis->lhs->location,
				       qis->rhs->invoke.location);
	}
	issues->end_group(issues);

	if(assignable_result != ESSTEE_TRUE)
	{
	    return ESSTEE_ERROR;
	}
    }

    return ESSTEE_OK;
}

static int assignment_statement_qualified_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    const struct value_iface_t *rhs_value = NULL;
    struct value_iface_t *lhs_target = NULL;
    
    switch(qis->invoke_state)
    {
    case 0:
	if(qis->lhs->constant_reference == ESSTEE_FALSE)
	{
	    int identifier_step_result = qis->lhs->step(qis->lhs,
							cursor,
							time,
							config,
							issues);
	
	    if(identifier_step_result != INVOKE_RESULT_FINISHED)
	    {
		return identifier_step_result;
	    }
	}
	qis->invoke_state = 1;

    case 1:
	if(qis->rhs->invoke.step)
	{
	    qis->invoke_state = 2;
	    cursor->switch_current(cursor,
				   &(qis->rhs->invoke),
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 2:
	lhs_target = qis->lhs->target(qis->lhs);
	
	if(qis->lhs->constant_reference == ESSTEE_FALSE)
	{   
	    if(!lhs_target->assignable_from)
	    {
		const char *message = issues->build_message(
		    issues,
		    "element '%s' cannot be assigned a new value",
		    qis->lhs->target_name(qis->lhs));

		issues->new_issue_at(
		    issues,
		    message,
		    ESSTEE_CONTEXT_ERROR,
		    1,
		    qis->lhs->location);

		return INVOKE_RESULT_ERROR;
	    }
	}

	rhs_value = qis->rhs->return_value(qis->rhs);

	if(qis->lhs->constant_reference == ESSTEE_FALSE)
	{
	    issues->begin_group(issues);
	    int assignable_result = lhs_target->assignable_from(lhs_target,
								rhs_value,
								config,
								issues);
	    if(assignable_result != ESSTEE_TRUE)
	    {
		issues->new_issue(issues,
				  "assignment of element '%s' impossible",
				  ESSTEE_CONTEXT_ERROR,
				  qis->lhs->target_name(qis->lhs));

		issues->set_group_location(issues,
					   2,
					   qis->lhs->location,
					   qis->rhs->invoke.location);
	    }
	    issues->end_group(issues);

	    if(assignable_result != ESSTEE_TRUE)
	    {
		return INVOKE_RESULT_ERROR;
	    }
	}

	issues->begin_group(issues);
	int assign_result = lhs_target->assign(lhs_target,
					       rhs_value,
					       config,
					       issues);

	if(assign_result != ESSTEE_OK)
	{
	    issues->new_issue(issues,
			      "assignment of element '%s' failed",
			      ESSTEE_CONTEXT_ERROR,
			      qis->lhs->target_name(qis->lhs));
	
	    issues->set_group_location(issues,
				       2,
				       qis->lhs->location,
				       qis->rhs->invoke.location);
	}
	issues->end_group(issues);

	if(assign_result != ESSTEE_OK)
	{
	    return INVOKE_RESULT_ERROR;
	}
    }

    return INVOKE_RESULT_FINISHED;
}

static int assignment_statement_qualified_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    int id_reset_result = qis->lhs->reset(qis->lhs,
					  config,
					  issues);
    if(id_reset_result != ESSTEE_OK)
    {
	return id_reset_result;
    }
    
    if(qis->rhs->invoke.reset)
    {
	int reset_result = qis->rhs->invoke.reset(&(qis->rhs->invoke),
						  config,
						  issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

static void assignment_statement_qualified_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: destructor */
}

static void assignment_statement_qualified_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: destructor */
}

static struct invoke_iface_t * assignment_statement_qualified_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    struct qualified_assignment_statement_t *copy = NULL;
    struct qualified_identifier_iface_t *lhs_copy = NULL;
    struct expression_iface_t *rhs_copy = NULL;

    ALLOC_OR_ERROR_JUMP(
	copy,
	struct qualified_assignment_statement_t,
	issues,
	error_free_resources);
    
    memcpy(copy, qis, sizeof(struct qualified_assignment_statement_t));

    lhs_copy = qis->lhs->clone(qis->lhs, issues);

    if(!lhs_copy)
    {
	goto error_free_resources;
    }

    copy->lhs = lhs_copy;
    
    if(qis->rhs->clone)
    {
	ALLOC_OR_JUMP(
	    rhs_copy,
	    struct expression_iface_t,
	    error_free_resources);

	rhs_copy = qis->rhs->clone(qis->rhs, issues);
	if(!rhs_copy)
	{
	    goto error_free_resources;
	}

	copy->rhs = rhs_copy;
    }
    
    copy->invoke.destroy = assignment_statement_qualified_clone_destroy;

    return &(copy->invoke);

error_free_resources:
    free(copy);
    free(lhs_copy);
    free(rhs_copy);
    return NULL;
}

static int assignment_statement_qualified_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    int lhs_allocate_result = qis->lhs->allocate(qis->lhs, issues);

    if(lhs_allocate_result != ESSTEE_OK)
    {
	return lhs_allocate_result;
    }

    if(qis->rhs->invoke.allocate)
    {
	int rhs_allocate_result = qis->rhs->invoke.allocate(
	    &(qis->rhs->invoke),
	    issues);

	if(rhs_allocate_result != ESSTEE_OK)
	{
	    return rhs_allocate_result;
	}
    }

    return ESSTEE_OK;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct invoke_iface_t * st_create_assignment_statement_qualified(
    struct qualified_identifier_iface_t *qualified_identifier,
    struct expression_iface_t *assignment,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_assignment_statement_t *qis = NULL;
    struct st_location_t *qis_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	qis,
	struct qualified_assignment_statement_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	qis_location,
	location,
	issues,
	error_free_resources);

    qis->location = qis->location;
    qis->lhs = qualified_identifier;
    qis->rhs = assignment;
    
    memset(&(qis->invoke), 0, sizeof(struct invoke_iface_t));
    qis->invoke.location = qis->location;
    qis->invoke.step = assignment_statement_qualified_step;
    qis->invoke.verify = assignment_statement_qualified_verify;
    qis->invoke.clone = assignment_statement_qualified_clone;
    qis->invoke.reset = assignment_statement_qualified_reset;
    qis->invoke.allocate = assignment_statement_qualified_allocate;
    qis->invoke.destroy = assignment_statement_qualified_destroy;

    return &(qis->invoke);
    
error_free_resources:
    free(qis);
    free(qis_location);
    return NULL;
}

