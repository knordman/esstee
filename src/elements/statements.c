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

const struct st_location_t * st_empty_statement_location(
    const struct invoke_iface_t *self)
{
    struct empty_statement_t *es =
	CONTAINER_OF(self, struct empty_statement_t, invoke);

    return es->location;
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
    if(sa->rhs->invoke.verify != NULL)
    {
	rhs_verified = sa->rhs->invoke.verify(&(sa->rhs->invoke), config, errors);
    }

    if(rhs_verified != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    const struct value_iface_t *rhs_value = sa->rhs->return_value(sa->rhs);

    int lhs_compatible_with_rhs = ESSTEE_TRUE;
    if(sa->lhs->value->compatible != NULL)
    {
	lhs_compatible_with_rhs = sa->lhs->value->compatible(sa->lhs->value, rhs_value, config);
    }

    if(lhs_compatible_with_rhs != ESSTEE_TRUE)
    {
	errors->new_issue_at(
	    errors,
	    "values not compatible",
	    ISSUE_ERROR_CLASS,
	    2,
	    sa->lhs_location,
	    sa->rhs->invoke.location(&(sa->rhs->invoke)));

	return ESSTEE_ERROR;
    }

    if(!sa->lhs->value->assign)
    {
	errors->new_issue_at(
	    errors,
	    "cannot assign value",
	    ISSUE_ERROR_CLASS,
	    1,
	    sa->lhs_location);

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

    if(sa->invoke_state == 0)
    {
	int rhs_invoke_status = INVOKE_RESULT_FINISHED;
	if(sa->rhs->invoke.step)
	{
	    rhs_invoke_status = sa->rhs->invoke.step(
		&(sa->rhs->invoke),
		cursor,
		time,
		config,
		errors);
	}

	switch(rhs_invoke_status)
	{
	case INVOKE_RESULT_FINISHED:
	    break;

	case INVOKE_RESULT_IN_PROGRESS:
	    sa->invoke_state = 1;
	    return INVOKE_RESULT_IN_PROGRESS;

	case INVOKE_RESULT_ERROR:
	default:
	    sa->invoke_state = 0;
	    return INVOKE_RESULT_ERROR;
	}
    }
    else
    {
	sa->invoke_state = 0;
    }

    const struct value_iface_t *rhs_value = sa->rhs->return_value(sa->rhs);

    int assignment_status = sa->lhs->value->assign(
	sa->lhs->value,
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

    int identifier_verified = st_inner_resolve_qualified_identifier(qis->lhs, errors);

    int rhs_verified = ESSTEE_OK;
    if(qis->rhs->invoke.verify != NULL)
    {
	rhs_verified = qis->rhs->invoke.verify(&(qis->rhs->invoke), config, errors);
    }

    if(identifier_verified != ESSTEE_OK || rhs_verified != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }
    
    const struct value_iface_t *rhs_value = qis->rhs->return_value(qis->rhs);

    int lhs_compatible_with_rhs = ESSTEE_TRUE;
    if(qis->lhs->target->compatible != NULL)
    {
	lhs_compatible_with_rhs = qis->lhs->target->compatible(qis->lhs->target, rhs_value, config);
    }

    if(lhs_compatible_with_rhs != ESSTEE_TRUE)
    {
	errors->new_issue_at(
	    errors,
	    "values not compatible",
	    ISSUE_ERROR_CLASS,
	    2,
	    qis->lhs->location,
	    qis->rhs->invoke.location(&(qis->rhs->invoke)));

	return ESSTEE_ERROR;
    }

    if(!qis->lhs->target->assign)
    {
	errors->new_issue_at(
	    errors,
	    "cannot assign value",
	    ISSUE_ERROR_CLASS,
	    1,
	    qis->lhs->location);

	return ESSTEE_ERROR;
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

    if(qis->invoke_state == 0)
    {
	int rhs_invoke_status = INVOKE_RESULT_FINISHED;
	if(qis->rhs->invoke.step)
	{
	    rhs_invoke_status = qis->rhs->invoke.step(
		&(qis->rhs->invoke),
		cursor,
		time,
		config,
		errors);
	}

	switch(rhs_invoke_status)
	{
	case INVOKE_RESULT_FINISHED:
	    break;

	case INVOKE_RESULT_IN_PROGRESS:
	    qis->invoke_state = 1;
	    return INVOKE_RESULT_IN_PROGRESS;

	case INVOKE_RESULT_ERROR:
	default:
	    qis->invoke_state = 0;	    
	    return INVOKE_RESULT_ERROR;
	}
    }
    else
    {
	qis->invoke_state = 0;
    }
	
    const struct value_iface_t *rhs_value = qis->rhs->return_value(qis->rhs);

    int assignment_status = qis->lhs->target->assign(
	qis->lhs->target,
	rhs_value,
	config);

    if(assignment_status != ESSTEE_OK)
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
