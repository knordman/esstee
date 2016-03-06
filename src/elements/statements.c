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
#include <elements/values.h>
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

    switch(is->invoke_state)
    {
    case 0:
	st_push_return_context(cursor, self);
	
	if(is->parameters)
	{
	    is->invoke_state = 1;
	    int step_result = st_step_invoke_parameters(is->parameters,
							cursor,
							time,
							config,
							errors);
	    if(step_result != INVOKE_RESULT_FINISHED)
	    {
		return step_result;
	    }
	}

    case 1:
	if(is->variable)
	{
	    is->invoke_state = 3;
	    return is->variable->value->invoke_step(is->variable->value,
						    is->parameters,
						    cursor,
						    time,
						    config,
						    errors);
	}
	else if(is->function)
	{
	    is->invoke_state = 2;
	    int assign_result = st_assign_from_invoke_parameters(is->parameters,
								 is->function->header->variables,
								 config,
								 errors);
	    if(assign_result != ESSTEE_OK)
	    {
		return INVOKE_RESULT_ERROR;
	    }
	}
	
    case 2:
	is->invoke_state = 3;
	st_switch_current(cursor, is->function->statements, config);
	return INVOKE_RESULT_IN_PROGRESS;

    case 3:
    default:
	break;
    }

    st_pop_return_context(cursor);
    
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
    else if(is->function)
    {
	int verify_result = st_verify_invoke_parameters(is->parameters,
							is->function->header->variables,
							errors,
							config);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }
    else
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
    else if(is->function)
    {
	struct variable_t *itr = NULL;
	DL_FOREACH(is->function->header->variables, itr)
	{
	    int reset_result = itr->type->reset_value_of(itr->type,
							 itr->value,
							 config);

	    if(reset_result != ESSTEE_OK)
	    {
		return reset_result;
	    }
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

/**************************************************************************/
/* Case statement                                                         */
/**************************************************************************/
const struct st_location_t * st_case_statement_location(
    const struct invoke_iface_t *self)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);

    return cs->location;
}

static int selector_value_in_case_list(
    const struct value_iface_t *selector,
    struct case_list_element_t *case_list,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct case_list_element_t *itr = NULL;
    DL_FOREACH(case_list, itr)
    {
	int equals = itr->value->equals(itr->value,
					selector,
					config);

	if(equals != ESSTEE_FALSE)
	{
	    return equals;
	}
    }

    return ESSTEE_FALSE;
}

int st_case_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);

    switch(cs->invoke_state)
    {
    case 0:
	if(cs->selector->invoke.step)
	{
	    cs->invoke_state = 1;
	    st_switch_current(cursor, &(cs->selector->invoke), config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	
    case 1: {
	const struct value_iface_t *selector_value =
	    cs->selector->return_value(cs->selector);

	struct case_t *case_itr = NULL;
	DL_FOREACH(cs->cases, case_itr)
	{
	    int selector_in_case_result = selector_value_in_case_list(selector_value,
								      case_itr->case_list,
								      config,
								      errors);
	    
	    if(selector_in_case_result == ESSTEE_TRUE)
	    {
		break;
	    }
	    else if(selector_in_case_result == ESSTEE_ERROR)
	    {
		return INVOKE_RESULT_ERROR;
	    }
	}

	cs->invoke_state = 2;
	if(case_itr)
	{
	    st_switch_current(cursor, case_itr->statements, config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	else if(cs->else_statements)
	{
	    st_switch_current(cursor, cs->else_statements, config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
    }

    case 2:
    default:
	break;
    }

    return INVOKE_RESULT_FINISHED;
}

int st_case_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);
    
    if(cs->selector->invoke.verify)
    {
	int verify_result =
	    cs->selector->invoke.verify(&(cs->selector->invoke),
					  config,
					  errors);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }

    const struct value_iface_t *selector_value =
	cs->selector->return_value(cs->selector);
    
    struct case_t *case_itr = NULL;
    DL_FOREACH(cs->cases, case_itr)
    {
	if(selector_value)
	{
	    struct case_list_element_t *case_list_itr = NULL;
	    DL_FOREACH(case_itr->case_list, case_list_itr)
	    {
		int comparable_result
		    = case_list_itr->value->comparable_to(case_list_itr->value,
							  selector_value,
							  config);
		
		if(comparable_result != ESSTEE_TRUE)
		{
		    errors->new_issue_at(
			errors,
			"values cannot be compared",
			ISSUE_ERROR_CLASS,
			2,
			cs->selector->invoke.location(&(cs->selector->invoke)),
			case_list_itr->location);
		    return ESSTEE_ERROR;
		}
	    }
	}
	
	struct invoke_iface_t *statement_itr = NULL;
	DL_FOREACH(case_itr->statements, statement_itr)
	{
	    int verify_result = statement_itr->verify(statement_itr,
						      config,
						      errors);
	    if(verify_result != ESSTEE_OK)
	    {
		return verify_result;
	    }
	}
    }

    return ESSTEE_OK;
}

int st_case_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);

    cs->invoke_state = 0;

    if(cs->selector->invoke.reset)
    {
	int reset_result
	    = cs->selector->invoke.reset(&(cs->selector->invoke),
					   config);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    struct case_t *case_itr = NULL;
    DL_FOREACH(cs->cases, case_itr)
    {
	struct invoke_iface_t *statement_itr = NULL;
	DL_FOREACH(case_itr->statements, statement_itr)
	{
	    int reset_result = statement_itr->reset(statement_itr,
						    config);
	    if(reset_result != ESSTEE_OK)
	    {
		return reset_result;
	    }
	}
    }

    struct invoke_iface_t *statement_itr = NULL;
    DL_FOREACH(cs->else_statements, statement_itr)
    {
	int reset_result = statement_itr->reset(statement_itr,
						config);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

struct invoke_iface_t * st_case_statement_clone(
    struct invoke_iface_t *self)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);

    struct case_statement_t *copy = NULL;
    struct expression_iface_t *selector_copy = NULL;
    struct invoke_iface_t *case_statements_copy = NULL;
    struct invoke_iface_t *else_statements_copy = NULL;
    struct case_t *case_itr = NULL;
    ALLOC_OR_JUMP(
	copy,
	struct case_statement_t,
	error_free_resources);

    memcpy(copy, cs, sizeof(struct case_statement_t));

    if(cs->selector->clone)
    {
	selector_copy = cs->selector->clone(cs->selector);

	if(!selector_copy)
	{
	    goto error_free_resources;
	}

	copy->selector = selector_copy;
    }

    struct case_t *cases_copy = NULL;
    struct case_t *case_copy = NULL;
    for(case_itr = cs->cases; case_itr != NULL; case_itr = case_itr->next)
    {
	ALLOC_OR_JUMP(
	    case_copy,
	    struct case_t,
	    error_free_resources);

	memcpy(case_copy, case_itr, sizeof(struct case_t));
	case_copy->statements = NULL;

	case_statements_copy = NULL;
	struct invoke_iface_t *statement_itr = NULL;
	DL_FOREACH(case_itr->statements, statement_itr)
	{
	    struct invoke_iface_t *statement_copy = statement_itr->clone(statement_itr);

	    if(!statement_copy)
	    {
		goto error_free_resources;
	    }

	    DL_APPEND(case_statements_copy, statement_copy);
	}

	case_copy->statements = case_statements_copy;
	DL_APPEND(cases_copy, case_copy);
    }
    copy->cases = cases_copy;
    
    struct invoke_iface_t *statement_itr = NULL;
    DL_FOREACH(cs->else_statements, statement_itr)
    {
	struct invoke_iface_t *statement_copy
	    = statement_itr->clone(statement_itr);

	if(!statement_copy)
	{
	    goto error_free_resources;
	}

	DL_APPEND(else_statements_copy, statement_copy);
    }

    copy->else_statements = else_statements_copy;
    
    copy->invoke.destroy = st_case_statement_clone_destroy;

    return &(copy->invoke);

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

void st_case_statement_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: case statement destructor */
}


void st_case_statement_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: case statement clone destructor */
}

/**************************************************************************/
/* If statements                                                          */
/**************************************************************************/
const struct st_location_t * st_if_statement_location(
    const struct invoke_iface_t *self)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    return ifs->location;
}
    
int st_if_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    switch(ifs->invoke_state)
    {
    case 0:
	if(ifs->condition->invoke.step)
	{
	    ifs->invoke_state = 1;
	    st_switch_current(cursor, &(ifs->condition->invoke), config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 1: {
	const struct value_iface_t *condition_value =
	    ifs->condition->return_value(ifs->condition);

	int current_condition = condition_value->bool(condition_value, config);

	ifs->invoke_state = 2;
	
	if(current_condition == ESSTEE_TRUE)
	{
	    st_switch_current(cursor, ifs->true_statements, config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	else if(ifs->elsif)
	{
	    st_switch_current(cursor, &(ifs->elsif->invoke), config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	else if(ifs->else_statements)
	{
	    st_switch_current(cursor, ifs->else_statements, config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
    }

    case 2:
    default:
	break;
    }

    return INVOKE_RESULT_FINISHED;
}

int st_if_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    if(ifs->condition->invoke.verify)
    {
	int condition_verify =
	    ifs->condition->invoke.verify(&(ifs->condition->invoke),
					  config,
					  errors);
	if(condition_verify != ESSTEE_OK)
	{
	    return condition_verify;
	}
    }

    const struct value_iface_t *condition_value =
	ifs->condition->return_value(ifs->condition);

    if(!condition_value->bool)
    {
	errors->new_issue_at(errors,
			     "condition cannot be interpreted as true or false",
			     ISSUE_ERROR_CLASS,
			     1,
			     ifs->location);
	return ESSTEE_ERROR;
    }
    
    struct invoke_iface_t *true_itr;
    DL_FOREACH(ifs->true_statements, true_itr)
    {
	if(true_itr->verify(true_itr, config, errors) != ESSTEE_OK)
	{
	    return ESSTEE_ERROR;
	}
    }

    if(ifs->elsif)
    {
	return st_if_statement_verify(&(ifs->elsif->invoke),
				      config,
				      errors);
    }
    else if(ifs->else_statements)
    {
	struct invoke_iface_t *else_itr;
	DL_FOREACH(ifs->else_statements, else_itr)
	{
	    if(else_itr->verify(else_itr, config, errors) != ESSTEE_OK)
	    {
		return ESSTEE_ERROR;
	    }
	}
    }

    return ESSTEE_OK;
}

int st_if_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    ifs->invoke_state = 0;

    if(ifs->condition->invoke.reset)
    {
	int reset_result
	    = ifs->condition->invoke.reset(&(ifs->condition->invoke),
					   config);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    struct invoke_iface_t *true_itr = NULL;
    DL_FOREACH(ifs->true_statements, true_itr)
    {
	int reset_result = true_itr->reset(true_itr, config);

	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    if(ifs->elsif)
    {
	return ifs->elsif->invoke.reset(&(ifs->elsif->invoke),
					config);
    }
    else if(ifs->else_statements)
    {
	struct invoke_iface_t *else_itr;
	DL_FOREACH(ifs->else_statements, else_itr)
	{
	    if(else_itr->reset(else_itr, config) != ESSTEE_OK)
	    {
		return ESSTEE_ERROR;
	    }
	}
    }

    return ESSTEE_OK;
}

struct invoke_iface_t * st_if_statement_clone(
    struct invoke_iface_t *self)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    struct if_statement_t *copy = NULL;
    struct expression_iface_t *condition_copy = NULL;
    struct invoke_iface_t *true_statements_copy = NULL;
    struct invoke_iface_t *else_statements_copy = NULL;
    struct invoke_iface_t *destroy_itr = NULL;
    struct invoke_iface_t *tmp = NULL;
    
    ALLOC_OR_JUMP(
	copy,
	struct if_statement_t,
	error_free_resources);

    memcpy(copy, ifs, sizeof(struct if_statement_t));

    if(ifs->condition->clone)
    {
	condition_copy = ifs->condition->clone(ifs->condition);

	if(!condition_copy)
	{
	    goto error_free_resources;
	}

	copy->condition = condition_copy;
    }

    struct invoke_iface_t *true_itr = NULL;
    DL_FOREACH(ifs->true_statements, true_itr)
    {
	struct invoke_iface_t *statement_copy = true_itr->clone(true_itr);

	if(!statement_copy)
	{
	    goto error_free_resources;
	}

	DL_APPEND(true_statements_copy, statement_copy);
    }

    if(ifs->elsif)
    {
	struct invoke_iface_t *elsif_copy_invoke =
	    ifs->elsif->invoke.clone(&(ifs->elsif->invoke));

	if(!elsif_copy_invoke)
	{
	    goto error_free_resources;
	}

	struct if_statement_t *elsif_copy =
	    CONTAINER_OF(elsif_copy_invoke, struct if_statement_t, invoke);

	copy->elsif = elsif_copy;
    }
    else if(ifs->else_statements)
    {
	struct invoke_iface_t *else_itr;
	DL_FOREACH(ifs->else_statements, else_itr)
	{
	    struct invoke_iface_t *statement_copy = else_itr->clone(else_itr);

	    if(!statement_copy)
	    {
		goto error_free_resources;
	    }

	    DL_APPEND(else_statements_copy, statement_copy);
	}
    }
    
    copy->true_statements = true_statements_copy;
    copy->else_statements = else_statements_copy;

    copy->invoke.destroy = st_if_statement_clone_destroy;
    
    return &(copy->invoke);
    
error_free_resources:
    DL_FOREACH_SAFE(true_statements_copy, destroy_itr, tmp)
    {
	free(destroy_itr);
    }
    DL_FOREACH_SAFE(else_statements_copy, destroy_itr, tmp)
    {
	free(destroy_itr);
    }
    free(copy);
    free(condition_copy);
    return NULL;
}

void st_if_statement_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: if statement destructor */
}

void st_if_statement_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: if statement clone destructor */
}

/**************************************************************************/
/* While statement                                                        */
/**************************************************************************/
const struct st_location_t * st_while_statement_location(
    const struct invoke_iface_t *self)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);

    return ws->location;
}
    
int st_while_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);

    const struct value_iface_t *while_value = NULL;
    
    switch(ws->invoke_state)
    {
    case 0:
	st_push_exit_context(cursor, self);
	
    case 1:
	if(ws->while_expression->invoke.step)
	{
	    ws->invoke_state = 2;
	    st_switch_current(cursor, &(ws->while_expression->invoke), config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 2:
    default:
	while_value = ws->while_expression->return_value(ws->while_expression);

	if(while_value->bool(while_value, config) == ESSTEE_TRUE)
	{
	    ws->invoke_state = 1;
	    st_switch_current(cursor, ws->statements, config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
    }

    st_pop_exit_context(cursor);
    
    return INVOKE_RESULT_FINISHED;
}

int st_while_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);
    
    if(ws->while_expression->invoke.verify)
    {
	int verify_result =
	    ws->while_expression->invoke.verify(&(ws->while_expression->invoke),
						config,
						errors);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }

    const struct value_iface_t *while_value =
	ws->while_expression->return_value(ws->while_expression);

    if(!while_value->bool)
    {
	const struct st_location_t *while_expression_location =
	    ws->while_expression->invoke.location(&(ws->while_expression->invoke));	    
	
	errors->new_issue_at(errors,
			     "condition cannot be interpreted as true or false",
			     ISSUE_ERROR_CLASS,
			     1,
			     while_expression_location);
	return ESSTEE_ERROR;
    }
   
    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(ws->statements, itr)
    {
	int verify_result = itr->verify(itr, config, errors);

	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }

    return ESSTEE_OK;
}

int st_while_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);

    ws->invoke_state = 0;

    if(ws->while_expression->invoke.reset)
    {
	int reset_result
	    = ws->while_expression->invoke.reset(&(ws->while_expression->invoke),
						 config);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(ws->statements, itr)
    {
	int reset_result = itr->reset(itr, config);

	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }
    
    return ESSTEE_OK;
}

struct invoke_iface_t * st_while_statement_clone(
    struct invoke_iface_t *self)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);

    struct while_statement_t *copy = NULL;
    struct invoke_iface_t *statements_copy = NULL;
    struct expression_iface_t *while_expression_copy = NULL;

    ALLOC_OR_JUMP(
	copy,
	struct while_statement_t,
	error_free_resources);

    memcpy(copy, ws, sizeof(struct while_statement_t));

    if(ws->while_expression->clone)
    {
	while_expression_copy = ws->while_expression->clone(ws->while_expression);

	if(!while_expression_copy)
	{
	    goto error_free_resources;
	}

	copy->while_expression = while_expression_copy;
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(ws->statements, itr)
    {
	struct invoke_iface_t *statement_copy = itr->clone(itr);

	if(!statement_copy)
	{
	    goto error_free_resources;
	}

	DL_APPEND(statements_copy, statement_copy);
    }
    
    copy->statements = statements_copy;

    copy->invoke.destroy = st_while_statement_clone_destroy;

    return &(copy->invoke);

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

void st_while_statement_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: while statement destructor */
}

void st_while_statement_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: while statement clone destructor */
}

/**************************************************************************/
/* For statement                                                          */
/**************************************************************************/
const struct st_location_t * st_for_statement_location(
    const struct invoke_iface_t *self)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    return fs->location;
}

int st_for_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    struct integer_value_t implicit_increment = {
	.value = {
	    .integer = st_integer_value_integer
	},
	.num = 1
    };

    const struct value_iface_t *increment_value = &(implicit_increment.value);
    if(fs->increment)
    {
	increment_value = fs->increment->return_value(fs->increment);
    }
    
    const struct value_iface_t *from_value = fs->from->return_value(fs->from);
    const struct value_iface_t *to_value = fs->to->return_value(fs->to);
    int variable_assign_result = ESSTEE_ERROR;
    
    switch(fs->invoke_state)
    {
    case 0:
	st_push_exit_context(cursor, self);
	
	if(fs->from->invoke.step)
	{
	    fs->invoke_state = 1;
	    st_switch_current(cursor, &(fs->from->invoke), config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 1:
	variable_assign_result = fs->variable->value->assign(fs->variable->value,
							     from_value,
							     config);
	if(variable_assign_result != ESSTEE_OK)
	{
	    errors->new_issue_at(
		errors,
		"assignment of variable failed",
		ISSUE_ERROR_CLASS,
		1,
		fs->identifier_location);
	    return INVOKE_RESULT_ERROR;
	}

    case 2:
	if(fs->to->invoke.step)
	{
	    fs->invoke_state = 3;
	    st_switch_current(cursor, &(fs->to->invoke), config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 3:
	if(fs->variable->value->greater(fs->variable->value, to_value, config) == ESSTEE_TRUE)
	{
	    break;
	}
	
    case 4: 
	fs->invoke_state = 5;
	st_switch_current(cursor, fs->statements, config);
	return INVOKE_RESULT_IN_PROGRESS;

    case 5:
	if(fs->increment && fs->increment->invoke.step)
	{
	    fs->invoke_state = 6;
	    st_switch_current(cursor, &(fs->increment->invoke), config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	
    case 6:
	if(fs->variable->value->plus(fs->variable->value, increment_value, config) != ESSTEE_OK)
	{
	    errors->new_issue_at(
		errors,
		"incrementing variable failed",
		ISSUE_ERROR_CLASS,
		1,
		fs->identifier_location);
	    return INVOKE_RESULT_ERROR;
	}

	fs->invoke_state = 2;
	return INVOKE_RESULT_IN_PROGRESS;
    }

    st_pop_exit_context(cursor);
    
    return INVOKE_RESULT_FINISHED;
}

int st_for_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    if(fs->from->invoke.verify)
    {
	int verify_result = fs->from->invoke.verify(&(fs->from->invoke),
						    config,
						    errors);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }

    if(fs->to->invoke.verify)
    {
	int verify_result = fs->to->invoke.verify(&(fs->to->invoke),
						    config,
						    errors);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }
    
    if(fs->increment && fs->increment->invoke.verify)
    {
	int verify_result = fs->increment->invoke.verify(&(fs->increment->invoke),
							 config,
							 errors);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }

    const struct value_iface_t *from_value = fs->from->return_value(fs->from);

    const struct value_iface_t *to_value = fs->to->return_value(fs->to);

    struct integer_value_t implicit_increment = {
	.value = {
	    .integer = st_integer_value_integer,
	    .class = st_general_value_empty_class,
	},
	.num = 1,
    };

    const struct value_iface_t *increment_value = &(implicit_increment.value);
    if(fs->increment)
    {
	increment_value = fs->increment->return_value(fs->increment);
    }
    
    int var_from_assignable = fs->variable->value->assignable_from(fs->variable->value,
								   from_value,
								   config);
    if(var_from_assignable != ESSTEE_TRUE)
    {
	errors->new_issue_at(
	    errors,
	    "variable cannot be assigned the value of the from expression",
	    ISSUE_ERROR_CLASS,
	    2,
	    fs->identifier_location,
	    fs->from->invoke.location(&(fs->from->invoke)));
	return ESSTEE_ERROR;
    }

    const struct type_iface_t *var_type =
	fs->variable->value->type_of(fs->variable->value);

    int type_can_hold_to_value = var_type->can_hold(var_type, to_value, config);
    if(type_can_hold_to_value != ESSTEE_TRUE)
    {	
	errors->new_issue_at(
	    errors,
	    "variable cannot hold the to value",
	    ISSUE_ERROR_CLASS,
	    2,
	    fs->identifier_location,
	    fs->to->invoke.location(&(fs->to->invoke)));
	return ESSTEE_ERROR;
    }
    
    int var_increment_operates =
	fs->variable->value->operates_with(fs->variable->value, increment_value, config);

    if(var_increment_operates != ESSTEE_TRUE)
    {
	errors->new_issue_at(
	    errors,
	    "variable value cannot be incremented by value",
	    ISSUE_ERROR_CLASS,
	    1,
	    fs->identifier_location);
	return ESSTEE_ERROR;
    }
    
    int var_to_comparable =
	fs->variable->value->comparable_to(fs->variable->value,
					   to_value,
					   config);
    if(var_to_comparable != ESSTEE_TRUE)
    {
	errors->new_issue_at(
	    errors,
	    "variable cannot be compared to the value of the to expression",
	    ISSUE_ERROR_CLASS,
	    2,
	    fs->identifier_location,
	    fs->to->invoke.location(&(fs->to->invoke)));
	return ESSTEE_ERROR;
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(fs->statements, itr)
    {
	int verify_result = itr->verify(itr, config, errors);

	if(verify_result != ESSTEE_OK)
	{
	    return ESSTEE_ERROR;
	}
    }

    return ESSTEE_OK;
}

int st_for_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    fs->invoke_state = 0;
    
    if(fs->from->invoke.reset)
    {
	int reset_result = fs->from->invoke.reset(&(fs->from->invoke),
						  config);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }
    
    if(fs->to->invoke.reset)
    {
	int reset_result = fs->to->invoke.reset(&(fs->to->invoke),
						  config);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    if(fs->increment && fs->increment->invoke.reset)
    {
	int reset_result = fs->increment->invoke.reset(&(fs->increment->invoke),
						       config);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(fs->statements, itr)
    {
	int reset_result = itr->reset(itr, config);
	
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

struct invoke_iface_t * st_for_statement_clone(
    struct invoke_iface_t *self)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    struct for_statement_t *copy = NULL;
    struct expression_iface_t *from_copy = NULL;
    struct expression_iface_t *to_copy = NULL;
    struct expression_iface_t *increment_copy = NULL;
    struct invoke_iface_t *statements_copy = NULL;
    struct invoke_iface_t *statement_copy = NULL;
    
    ALLOC_OR_JUMP(
	copy,
	struct for_statement_t,
	error_free_resources);

    memcpy(copy, fs, sizeof(struct for_statement_t));
    copy->statements = NULL;
    
    if(fs->from->clone)
    {
	from_copy = fs->from->clone(fs->from);

	if(!from_copy)
	{
	    goto error_free_resources;
	}

	copy->from = from_copy;
    }
    
    if(fs->to->clone)
    {
	to_copy = fs->to->clone(fs->to);

	if(!to_copy)
	{
	    goto error_free_resources;
	}

	copy->to = to_copy;
    }

    if(fs->increment && fs->increment->clone)
    {
	increment_copy = fs->increment->clone(fs->increment);

	if(!increment_copy)
	{
	    goto error_free_resources;
	}

	copy->increment = increment_copy;
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(fs->statements, itr)
    {
	statement_copy = itr->clone(itr);

	if(!statement_copy)
	{
	    goto error_free_resources;
	}

	DL_APPEND(statements_copy, statement_copy);
    }

    copy->statements = statements_copy;
    copy->invoke.destroy = st_for_statement_clone_destroy;
    
    return &(copy->invoke);

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

void st_for_statement_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: for statement destructor */
}

void st_for_statement_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: cloned for statement destructor */
}

/**************************************************************************/
/* Repeat statement                                                       */
/**************************************************************************/
int st_repeat_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);

    const struct value_iface_t *while_value = NULL;
    
    switch(ws->invoke_state)
    {
    case 0:
	st_push_exit_context(cursor, self);
	
    case 1:
	ws->invoke_state = 2;
	st_switch_current(cursor, ws->statements, config);
	return INVOKE_RESULT_IN_PROGRESS;

    case 2:
	if(ws->while_expression->invoke.step)
	{
	    ws->invoke_state = 3;
	    st_switch_current(cursor, &(ws->while_expression->invoke), config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 3:
	while_value = ws->while_expression->return_value(ws->while_expression);

	if(while_value->bool(while_value, config) == ESSTEE_FALSE)
	{
	    ws->invoke_state = 1;
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	else
	{
	    break;
	}
    }

    st_pop_exit_context(cursor);
    
    return INVOKE_RESULT_FINISHED;
}

/**************************************************************************/
/* Exit/return statement                                                  */
/**************************************************************************/
const struct st_location_t * st_pop_statement_location(
    const struct invoke_iface_t *self)
{
    struct pop_call_stack_statement_t *ps = 
	CONTAINER_OF(self, struct pop_call_stack_statement_t, invoke);

    return ps->location;
}

int st_return_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return st_jump_return(cursor);
}

int st_exit_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    st_jump_exit(cursor);
    return INVOKE_RESULT_FINISHED;
}

int st_pop_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return ESSTEE_OK;
}

int st_pop_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    return ESSTEE_OK;
}

struct invoke_iface_t * st_pop_statement_clone(
    struct invoke_iface_t *self)
{
    struct pop_call_stack_statement_t *ps = 
	CONTAINER_OF(self, struct pop_call_stack_statement_t, invoke);

    struct pop_call_stack_statement_t *copy = NULL;
    ALLOC_OR_JUMP(
	copy,
	struct pop_call_stack_statement_t,
	error_free_resources);

    memcpy(copy, ps, sizeof(struct pop_call_stack_statement_t));

    return &(copy->invoke);
    
error_free_resources:
    return NULL;
}

void st_pop_statement_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: pop callstack statement destroy */
}
