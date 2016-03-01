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

#include <elements/statements.h>
#include <elements/shared.h>
#include <util/macros.h>

#include <utlist.h>

/**************************************************************************/
/* Empty statement                                                        */
/**************************************************************************/

int st_empty_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return ESSTEE_OK;
}

int st_empty_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return INVOKE_RESULT_FINISHED;
}

int st_empty_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    return ESSTEE_OK;
}

struct invoke_iface_t * st_empty_statement_clone(
    struct invoke_iface_t *self)
{
    struct empty_statement_t *es =
	CONTAINER_OF(self, struct empty_statement_t, invoke);

    struct empty_statement_t *copy = NULL;
    ALLOC_OR_JUMP(
	copy,
	struct empty_statement_t,
	error_free_resources);

    memcpy(copy, es, sizeof(struct empty_statement_t));
    copy->invoke.destroy = st_empty_statement_clone_destroy;

    return &(copy->invoke);
    
error_free_resources:
    return NULL;
}

const struct st_location_t * st_empty_statement_location(
    const struct invoke_iface_t *self)
{
    struct empty_statement_t *es =
	CONTAINER_OF(self, struct empty_statement_t, invoke);

    return es->location;
}

void st_empty_statement_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: destructor */
}

void st_empty_statement_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: destructor */
}

/**************************************************************************/
/* Simple assignment                                                      */
/**************************************************************************/
int st_assignment_statement_simple_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct simple_assignment_statement_t *sa =
	CONTAINER_OF(self, struct simple_assignment_statement_t, invoke);

    int rhs_verified = ESSTEE_OK;
    if(sa->rhs->invoke.verify)
    {
	rhs_verified = sa->rhs->invoke.verify(&(sa->rhs->invoke), config, errors);
    }

    if(rhs_verified != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    const struct value_iface_t *rhs_value = sa->rhs->return_value(sa->rhs);

    if(!sa->lhs->value->assignable_from)
    {
	errors->new_issue_at(
	    errors,
	    "value is not assignable",
	    ISSUE_ERROR_CLASS,
	    1,
	    sa->lhs_location);

	return ESSTEE_ERROR;
    }
    
    int lhs_assignable_from_rhs
	= sa->lhs->value->assignable_from(sa->lhs->value, rhs_value, config);

    if(lhs_assignable_from_rhs != ESSTEE_TRUE)
    {
	errors->new_issue_at(
	    errors,
	    "left value cannot be assigned from right value",
	    ISSUE_ERROR_CLASS,
	    2,
	    sa->lhs_location,
	    sa->rhs->invoke.location(&(sa->rhs->invoke)));

	return ESSTEE_ERROR;
    }
    
    return ESSTEE_OK;
}

int st_assignment_statement_simple_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct simple_assignment_statement_t *sa =
	CONTAINER_OF(self, struct simple_assignment_statement_t, invoke);

    if(sa->invoke_state == 0 && sa->rhs->invoke.step)
    {
	sa->invoke_state = 1;
	st_switch_current(cursor, &(sa->rhs->invoke), config);
	return INVOKE_RESULT_IN_PROGRESS;
    }

    const struct value_iface_t *rhs_value = sa->rhs->return_value(sa->rhs);

    int assignment_status = sa->lhs->value->assign(sa->lhs->value,
						   rhs_value,
						   config);
    
    if(assignment_status == ESSTEE_RT_TYPE_UNDERFLOW)
    {
	errors->new_issue_at(
	    errors,
	    "assignment destination type underflow",
	    ISSUE_ERROR_CLASS,
	    1,
	    sa->lhs_location);

	return INVOKE_RESULT_ERROR;
    }
    else if(assignment_status == ESSTEE_RT_TYPE_OVERFLOW)
    {
	errors->new_issue_at(
	    errors,
	    "assignment destination type overflow",
	    ISSUE_ERROR_CLASS,
	    1,
	    sa->lhs_location);

	return INVOKE_RESULT_ERROR;
    }
    else if(assignment_status != ESSTEE_OK)
    {
	errors->new_issue_at(
	    errors,
	    "assignment failed",
	    ISSUE_ERROR_CLASS,
	    2,
	    sa->lhs_location,
	    sa->rhs->invoke.location(&(sa->rhs->invoke)));

	return INVOKE_RESULT_ERROR;
    }
    
    return INVOKE_RESULT_FINISHED;
}

const struct st_location_t * st_assignment_statement_simple_location(
    const struct invoke_iface_t *self)
{
    struct simple_assignment_statement_t *sa =
	CONTAINER_OF(self, struct simple_assignment_statement_t, invoke);

    return sa->location;
}

struct invoke_iface_t * st_assignment_statement_simple_clone(
    struct invoke_iface_t *self)
{
    struct simple_assignment_statement_t *sa =
	CONTAINER_OF(self, struct simple_assignment_statement_t, invoke);

    struct simple_assignment_statement_t *copy = NULL;
    struct expression_iface_t *rhs_copy = NULL;
    
    ALLOC_OR_JUMP(
	copy,
	struct simple_assignment_statement_t,
	error_free_resources);
    
    memcpy(copy, sa, sizeof(struct simple_assignment_statement_t));

    if(sa->rhs->clone)
    {
	ALLOC_OR_JUMP(
	    rhs_copy,
	    struct expression_iface_t,
	    error_free_resources);

	rhs_copy = sa->rhs->clone(sa->rhs);
	if(!rhs_copy)
	{
	    goto error_free_resources;
	}

	copy->rhs = rhs_copy;
    }
    
    copy->invoke.destroy = st_assignment_statement_simple_clone_destroy;

    return &(copy->invoke);

error_free_resources:
    return NULL;
}

int st_assignment_statement_simple_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    struct simple_assignment_statement_t *sa =
	CONTAINER_OF(self, struct simple_assignment_statement_t, invoke);

    sa->invoke_state = 0;

    if(sa->rhs->invoke.reset)
    {
	int rhs_reset = sa->rhs->invoke.reset(&(sa->rhs->invoke), config);

	if(rhs_reset != ESSTEE_OK)
	{
	    return rhs_reset;
	}
    }

    return ESSTEE_OK;
}

void st_assignment_statement_simple_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: destructor */
}

void st_assignment_statement_simple_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: destructor */
}

/**************************************************************************/
/* Qualified identifier assignment                                        */
/**************************************************************************/
int st_assignment_statement_qualified_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    int identifier_verified = st_qualified_identifier_verify(qis->lhs,
							     errors,
							     config);

    if(identifier_verified != ESSTEE_OK)
    {
	return identifier_verified;
    }
    
    int rhs_verified = ESSTEE_OK;
    if(qis->rhs->invoke.verify)
    {
	rhs_verified = qis->rhs->invoke.verify(&(qis->rhs->invoke), config, errors);
    }

    if(rhs_verified != ESSTEE_OK)
    {
	return rhs_verified;
    }

    if(qis->lhs->runtime_constant_reference)
    {
	const struct value_iface_t *rhs_value = qis->rhs->return_value(qis->rhs);

	if(!qis->lhs->target->assignable_from)
	{
	    errors->new_issue_at(
		errors,
		"value is not assignable",
		ISSUE_ERROR_CLASS,
		1,
		qis->lhs->location);

	    return ESSTEE_ERROR;
	}
    
	int lhs_assignable_from_rhs
	    = qis->lhs->target->assignable_from(qis->lhs->target, rhs_value, config);

	if(lhs_assignable_from_rhs != ESSTEE_TRUE)
	{
	    errors->new_issue_at(
		errors,
		"left value cannot be assigned from right value",
		ISSUE_ERROR_CLASS,
		2,
		qis->lhs->location,
		qis->rhs->invoke.location(&(qis->rhs->invoke)));

	    return ESSTEE_ERROR;
	}
    }

    return ESSTEE_OK;
}

int st_assignment_statement_qualified_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    if(qis->lhs_invoke_state != INVOKE_RESULT_FINISHED)
    {
	qis->lhs_invoke_state = st_qualified_identifier_step(qis->lhs,
							     cursor,
							     errors,
							     config);
    }

    if(qis->lhs_invoke_state != INVOKE_RESULT_FINISHED)
    {
	return qis->lhs_invoke_state;
    }

    if(qis->rhs_invoke_state == 0 && qis->rhs->invoke.step)
    {
	qis->rhs_invoke_state = 1;
	st_switch_current(cursor, &(qis->rhs->invoke), config);
	return INVOKE_RESULT_IN_PROGRESS;
    }
    	
    const struct value_iface_t *rhs_value = qis->rhs->return_value(qis->rhs);

    if(!qis->lhs->runtime_constant_reference)
    {
	int lhs_assignable_from_rhs
	    = qis->lhs->target->assignable_from(qis->lhs->target, rhs_value, config);

	if(lhs_assignable_from_rhs != ESSTEE_TRUE)
	{
	    errors->new_issue_at(
		errors,
		"left value cannot be assigned from right value",
		ISSUE_ERROR_CLASS,
		2,
		qis->lhs->location,
		qis->rhs->invoke.location(&(qis->rhs->invoke)));

	    return INVOKE_RESULT_ERROR;
	}
    }
    
    int assignment_status = qis->lhs->target->assign(qis->lhs->target,
						     rhs_value,
						     config);

    if(assignment_status == ESSTEE_RT_TYPE_UNDERFLOW)
    {
	errors->new_issue_at(
	    errors,
	    "assignment destination type underflow",
	    ISSUE_ERROR_CLASS,
	    1,
	    qis->lhs->location);

	return INVOKE_RESULT_ERROR;
    }
    else if(assignment_status == ESSTEE_RT_TYPE_OVERFLOW)
    {
	errors->new_issue_at(
	    errors,
	    "assignment destination type overflow",
	    ISSUE_ERROR_CLASS,
	    1,
	    qis->lhs->location);

	return INVOKE_RESULT_ERROR;
    }
    else if(assignment_status != ESSTEE_OK)
    {
	errors->new_issue_at(
	    errors,
	    "assignment failed",
	    ISSUE_ERROR_CLASS,
	    2,
	    qis->lhs->location,
	    qis->rhs->invoke.location(&(qis->rhs->invoke)));

	return INVOKE_RESULT_ERROR;
    }
    
    return INVOKE_RESULT_FINISHED;
}

const struct st_location_t * st_assignment_statement_qualified_location(
    const struct invoke_iface_t *self)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    return qis->location;
}

struct invoke_iface_t * st_assignment_statement_qualified_clone(
    struct invoke_iface_t *self)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    struct qualified_assignment_statement_t *copy = NULL;
    struct qualified_identifier_t *lhs_copy = NULL;
    struct expression_iface_t *rhs_copy = NULL;

    ALLOC_OR_JUMP(
	copy,
	struct qualified_assignment_statement_t,
	error_free_resources);

    ALLOC_OR_JUMP(
	lhs_copy,
	struct qualified_identifier_t,
	error_free_resources);
    
    memcpy(copy, qis, sizeof(struct qualified_assignment_statement_t));
    memcpy(lhs_copy, qis->lhs, sizeof(struct qualified_identifier_t));
    
    if(qis->rhs->clone)
    {
	ALLOC_OR_JUMP(
	    rhs_copy,
	    struct expression_iface_t,
	    error_free_resources);

	rhs_copy = qis->rhs->clone(qis->rhs);
	if(!rhs_copy)
	{
	    goto error_free_resources;
	}

	copy->rhs = rhs_copy;
    }
    
    copy->invoke.destroy = st_assignment_statement_qualified_clone_destroy;

    return &(copy->invoke);

error_free_resources:
    free(copy);
    free(lhs_copy);
    free(rhs_copy);
    return NULL;
}

int st_assignment_statement_qualified_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    int lhs_reset = st_qualified_identifier_reset(qis->lhs, config);
    if(lhs_reset != ESSTEE_OK)
    {
	return lhs_reset;
    }
    
    if(qis->rhs->invoke.reset)
    {
	int rhs_reset = qis->rhs->invoke.reset(&(qis->rhs->invoke), config);

	if(rhs_reset != ESSTEE_OK)
	{
	    return rhs_reset;
	}
    }
    
    return ESSTEE_OK;
}

void st_assignment_statement_qualified_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: destructor */
}

void st_assignment_statement_qualified_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: destructor */
}

/**************************************************************************/
/* Invoke statement                                                       */
/**************************************************************************/

const struct st_location_t * st_invoke_statement_location(
    const struct invoke_iface_t *self)
{
    struct invoke_statement_t *is =
	CONTAINER_OF(self, struct invoke_statement_t, invoke);

    return is->location;
}
    
int st_invoke_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct invoke_statement_t *is =
	CONTAINER_OF(self, struct invoke_statement_t, invoke);

    if(is->invoke_state == 2)
    {
	return INVOKE_RESULT_FINISHED;
    }
    
    if(is->parameters && is->invoke_state == 0)
    {
	int step_result = st_step_invoke_parameters(is->parameters,
						    cursor,
						    time,
						    config,
						    errors);
	if(step_result != INVOKE_RESULT_FINISHED)
	{
	    return step_result;
	}
	else
	{
	    is->invoke_state = 1;
	}
    }
    else
    {
	is->invoke_state = 1;
    }

    if(is->variable)
    {
	is->invoke_state = 2;
	return is->variable->value->invoke_step(is->variable->value,
						is->parameters,
						cursor,
						time,
						config,
						errors);
    }
    
    return INVOKE_RESULT_FINISHED;
}

int st_invoke_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct invoke_statement_t *is =
	CONTAINER_OF(self, struct invoke_statement_t, invoke);

    if(is->variable)
    {
	if(is->function)
	{
	    errors->new_issue_at(
		errors,
		"variable shadows function",
		ISSUE_WARNING_CLASS,
		1,
		is->location);
	}

	if(!is->variable->value->invoke_step)
	{
	    errors->new_issue_at(
		errors,
		"variable cannot be invoked",
		ISSUE_ERROR_CLASS,
		1,
		is->location);

	    return ESSTEE_ERROR;
	}

	int verify = is->variable->value->invoke_verify(
	    is->variable->value,
	    is->parameters,
	    config,
	    errors);

	if(verify != ESSTEE_OK)
	{
	    return verify;
	}

    }
    else if(!is->function)
    {
	errors->new_issue_at(
	    errors,
	    "no variable or function referenced",
	    ISSUE_ERROR_CLASS,
	    1,
	    is->location);

	return ESSTEE_ERROR;
    }
    
    return ESSTEE_OK;
}

int st_invoke_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    struct invoke_statement_t *is =
	CONTAINER_OF(self, struct invoke_statement_t, invoke);

    if(is->variable && is->variable->value->invoke_reset)
    {
	int reset = 
	    is->variable->value->invoke_reset(is->variable->value);

	if(reset != ESSTEE_OK)
	{
	    return reset;
	}
    }
    
    is->invoke_state = 0;
    if(is->parameters)
    {
	is->parameters->invoke_state = 0;
    }
    
    return ESSTEE_OK;
}

struct invoke_iface_t * st_invoke_statement_clone(
    struct invoke_iface_t *self)
{
    struct invoke_statement_t *is =
	CONTAINER_OF(self, struct invoke_statement_t, invoke);

    struct invoke_statement_t *copy = NULL;
    ALLOC_OR_JUMP(
	copy,
	struct invoke_statement_t,
	error_free_resources);

    memcpy(copy, is, sizeof(struct invoke_statement_t));
    
    copy->invoke.destroy = st_invoke_statement_clone_destroy;

    return &(copy->invoke);

error_free_resources:
    return NULL;
}

void st_invoke_statement_destroy(
    struct invoke_iface_t *self)
{
    /* struct invoke_statement_t *is = */
    /* 	CONTAINER_OF(self, struct invoke_statement_t, invoke); */

    /* TODO: invoke destructor */
}

void st_invoke_statement_clone_destroy(
    struct invoke_iface_t *self)
{
    /* struct invoke_statement_t *is = */
    /* 	CONTAINER_OF(self, struct invoke_statement_t, invoke); */

    /* TODO: clone destructor invoke statement */
}
