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
#include <linker/linker.h>

#include <utlist.h>

/**************************************************************************/
/* Empty statement                                                        */
/**************************************************************************/

int st_empty_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}

int st_empty_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return INVOKE_RESULT_FINISHED;
}

int st_empty_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}

struct invoke_iface_t * st_empty_statement_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct empty_statement_t *es =
	CONTAINER_OF(self, struct empty_statement_t, invoke);

    struct empty_statement_t *copy = NULL;
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct empty_statement_t,
	issues,
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

    if(!sa->lhs->value->assignable_from)
    {
	const char *message = issues->build_message(issues,
						    "variable '%s' cannot be assigned a new value",
						    ESSTEE_ARGUMENT_ERROR,
						    sa->lhs->identifier);
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_CONTEXT_ERROR,
	    1,
	    sa->lhs_location);

	return ESSTEE_ERROR;
    }
    
    issues->begin_group(issues);
    int assignable_result = sa->lhs->value->assignable_from(sa->lhs->value,
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
				   sa->rhs->invoke.location(&(sa->rhs->invoke)));
    }
    issues->end_group(issues);
    
    if(assignable_result != ESSTEE_TRUE)
    {
	return ESSTEE_ERROR;
    }
    
    return ESSTEE_OK;
}

int st_assignment_statement_simple_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
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
	    st_switch_current(cursor, &(sa->rhs->invoke), config, issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 1: {
	const struct value_iface_t *rhs_value = sa->rhs->return_value(sa->rhs);
	
	issues->begin_group(issues);
	int assign_result = sa->lhs->value->assign(sa->lhs->value,
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
				       sa->rhs->invoke.location(&(sa->rhs->invoke)));
	}
	issues->end_group(issues);

	if(assign_result != ESSTEE_OK)
	{
	    return INVOKE_RESULT_ERROR;
	}

	if(sa->lhs->address)
	{
	    sa->lhs->type->sync_direct_memory(sa->lhs->type,
					      sa->lhs->value,
					      sa->lhs->address,
					      1);
	}
    }
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

int st_assignment_statement_simple_allocate(
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

struct invoke_iface_t * st_assignment_statement_simple_clone(
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
    
    copy->invoke.destroy = st_assignment_statement_simple_clone_destroy;

    return &(copy->invoke);

error_free_resources:
    return NULL;
}

int st_assignment_statement_simple_reset(
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
    struct issues_iface_t *issues)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    int identifier_verify_result = st_qualified_identifier_verify(qis->lhs,
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

    if(qis->lhs->runtime_constant_reference)
    {
	const struct value_iface_t *rhs_value = qis->rhs->return_value(qis->rhs);

	if(!qis->lhs->target->assignable_from)
	{
	    const char *format = (qis->lhs->last->array_index) ?
		"element '%s' (with array index) cannot be assigned a new value" :
		"element '%s' cannot be assigned a new value";

	    const char *message = issues->build_message(
		issues,
		format,
		qis->lhs->last->identifier);

	    issues->new_issue_at(
		issues,
		message,
		ESSTEE_CONTEXT_ERROR,
		1,
		qis->lhs->location);

	    return ESSTEE_ERROR;
	}

	issues->begin_group(issues);
	int assignable_result = qis->lhs->target->assignable_from(qis->lhs->target,
								  rhs_value,
								  config,
								  issues);
	if(assignable_result != ESSTEE_TRUE)
	{
	    const char *format = (qis->lhs->last->array_index) ?
		"assignment of element '%s' (with array index) impossible" :
		"assignment of element '%s' impossible";

	    issues->new_issue(issues,
			      format,
			      ESSTEE_CONTEXT_ERROR,
			      qis->lhs->last->identifier);

	    issues->set_group_location(issues,
				       2,
				       qis->lhs->location,
				       qis->rhs->invoke.location(&(qis->rhs->invoke)));
	}
	issues->end_group(issues);

	if(assignable_result != ESSTEE_TRUE)
	{
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
    struct issues_iface_t *issues)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    const struct value_iface_t *rhs_value = NULL;
    
    switch(qis->invoke_state)
    {
    case 0: {
	int identifier_step_result = st_qualified_identifier_step(qis->lhs,
								  cursor,
								  config,
								  issues);
	if(identifier_step_result != INVOKE_RESULT_FINISHED)
	{
	    return identifier_step_result;
	}

	qis->invoke_state = 1;
    }

    case 1:
	if(qis->rhs->invoke.step)
	{
	    qis->invoke_state = 2;
	    st_switch_current(cursor, &(qis->rhs->invoke), config, issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 2:
	if(!qis->lhs->runtime_constant_reference)
	{
	    if(!qis->lhs->target->assignable_from)
	    {
		const char *format = (qis->lhs->last->array_index) ?
		    "element '%s' (with non-constant array index) cannot be assigned a new value" :
		    "element '%s' cannot be assigned a new value";

		const char *message = issues->build_message(
		    issues,
		    format,
		    qis->lhs->last->identifier);

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

	if(!qis->lhs->runtime_constant_reference)
	{
	    issues->begin_group(issues);
	    int assignable_result = qis->lhs->target->assignable_from(qis->lhs->target,
								     rhs_value,
								     config,
								     issues);
	    if(assignable_result != ESSTEE_TRUE)
	    {
		const char *format = (qis->lhs->last->array_index) ?
		    "assignment of element '%s' (with non-constant array index) impossible" :
		    "assignment of element '%s' impossible";

		issues->new_issue(issues,
				  format,
				  ESSTEE_CONTEXT_ERROR,
				  qis->lhs->last->identifier);

		issues->set_group_location(issues,
					   2,
					   qis->lhs->location,
					   qis->rhs->invoke.location(&(qis->rhs->invoke)));
	    }
	    issues->end_group(issues);

	    if(assignable_result != ESSTEE_TRUE)
	    {
		return INVOKE_RESULT_ERROR;
	    }
	}

	issues->begin_group(issues);
	int assign_result = qis->lhs->target->assign(qis->lhs->target,
						     rhs_value,
						     config,
						     issues);

	if(assign_result != ESSTEE_OK)
	{
	    const char *format = (qis->lhs->last->array_index) ?
		"assignment of element '%s' (with array index) failed" :
		"assignment of element '%s' failed";

	    issues->new_issue(issues,
			      format,
			      ESSTEE_CONTEXT_ERROR,
			      qis->lhs->last->identifier);
	
	    issues->set_group_location(issues,
				       2,
				       qis->lhs->location,
				       qis->rhs->invoke.location(&(qis->rhs->invoke)));
	}
	issues->end_group(issues);

	if(assign_result != ESSTEE_OK)
	{
	    return INVOKE_RESULT_ERROR;
	}
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
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    struct qualified_assignment_statement_t *copy = NULL;
    struct qualified_identifier_t *lhs_copy = NULL;
    struct expression_iface_t *rhs_copy = NULL;

    ALLOC_OR_ERROR_JUMP(
	copy,
	struct qualified_assignment_statement_t,
	issues,
	error_free_resources);
    
    memcpy(copy, qis, sizeof(struct qualified_assignment_statement_t));

    lhs_copy = st_clone_qualified_identifier(qis->lhs, issues);

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
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    int id_reset_result = st_qualified_identifier_reset(qis->lhs,
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

int st_assignment_statement_qualified_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct qualified_assignment_statement_t *qis =
	CONTAINER_OF(self, struct qualified_assignment_statement_t, invoke);

    int lhs_allocate_result = st_qualified_identifier_allocate(qis->lhs, issues);

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
    struct issues_iface_t *issues)
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
							issues);
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
						    issues);
	}
	else if(is->function)
	{
	    is->invoke_state = 2;
	    int assign_result = st_assign_from_invoke_parameters(is->parameters,
								 is->function->header->variables,
								 config,
								 issues);
	    if(assign_result != ESSTEE_OK)
	    {
		return INVOKE_RESULT_ERROR;
	    }
	}
	
    case 2:
	is->invoke_state = 3;
	st_switch_current(cursor, is->function->statements, config, issues);
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
    struct issues_iface_t *issues)
{
    struct invoke_statement_t *is =
	CONTAINER_OF(self, struct invoke_statement_t, invoke);
    
    if(is->variable)
    {
	if(is->function)
	{
	    const char *message = issues->build_message(
		issues,
		"variable '%s' shadows function with same name",
		is->variable->identifier);

	    issues->new_issue_at(
		issues,
		message,
		ESSTEE_GENERAL_WARNING_ISSUE,
		1,
		is->location);
	}

	if(!is->variable->value->invoke_step)
	{
	    const char *message = issues->build_message(issues,
							"variable '%s' cannot be invoked",
							is->variable->identifier);	    
	    issues->new_issue_at(
		issues,
		message,
		ESSTEE_CONTEXT_ERROR,
		1,
		is->location);

	    return ESSTEE_ERROR;
	}

	int verify_result = is->variable->value->invoke_verify(is->variable->value,
							       is->parameters,
							       config,
							       issues);

	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}	
    }
    else if(is->function)
    {
	int verify_result = st_verify_invoke_parameters(is->parameters,
							is->function->header->variables,	
							config,
							issues);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }
    else
    {
	issues->new_issue_at(
	    issues,
	    "no known variable or function referenced",
	    ISSUE_ERROR_CLASS,
	    1,
	    is->location);

	return ESSTEE_ERROR;
    }
    
    return ESSTEE_OK;
}

int st_invoke_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_statement_t *is =
	CONTAINER_OF(self, struct invoke_statement_t, invoke);

    if(is->variable && is->variable->value->invoke_reset)
    {
        int reset_result = is->variable->value->invoke_reset(is->variable->value,
							     config,
							     issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }
    else if(is->function)
    {
	struct variable_t *itr = NULL;
	DL_FOREACH(is->function->header->variables, itr)
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

    is->invoke_state = 0;
    if(is->parameters)
    {
	int reset_result = st_reset_invoke_parameters(is->parameters,
						      config,
						      issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

int st_invoke_statement_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct invoke_statement_t *is =
	CONTAINER_OF(self, struct invoke_statement_t, invoke);

    return st_allocate_invoke_parameters(is->parameters, issues);
}

struct invoke_iface_t * st_invoke_statement_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct invoke_statement_t *is =
	CONTAINER_OF(self, struct invoke_statement_t, invoke);

    struct invoke_statement_t *copy = NULL;
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct invoke_statement_t,
	issues,
	error_free_resources);

    memcpy(copy, is, sizeof(struct invoke_statement_t));

    struct invoke_parameter_t *parameters_copy =
	st_clone_invoke_parameters(is->parameters, issues);
    if(!parameters_copy)
    {
	goto error_free_resources;
    }

    copy->parameters = parameters_copy;
    copy->invoke.destroy = st_invoke_statement_clone_destroy;

    return &(copy->invoke);

error_free_resources:
    free(copy);
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
    struct issues_iface_t *issues)
{
    struct case_list_element_t *itr = NULL;
    DL_FOREACH(case_list, itr)
    {
	int equals = itr->value->equals(itr->value,
					selector,
					config,
					issues);

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
    struct issues_iface_t *issues)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);

    switch(cs->invoke_state)
    {
    case 0:
	if(cs->selector->invoke.step)
	{
	    cs->invoke_state = 1;
	    st_switch_current(cursor, &(cs->selector->invoke), config, issues);
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
								      issues);
	    
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
	    st_switch_current(cursor, case_itr->statements, config, issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	else if(cs->else_statements)
	{
	    st_switch_current(cursor, cs->else_statements, config, issues);
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
    struct issues_iface_t *issues)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);
    
    if(cs->selector->invoke.verify)
    {
	int verify_result =
	    cs->selector->invoke.verify(&(cs->selector->invoke),
					config,
					issues);
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
		issues->begin_group(issues);
		int comparable_result = 
		    case_list_itr->value->comparable_to(case_list_itr->value,
							selector_value,
							config,
							issues);		
		if(comparable_result != ESSTEE_TRUE)
		{
		    issues->new_issue(issues,
				      "case selector not comparable to case value",
				      ESSTEE_CONTEXT_ERROR);
				      
		    issues->set_group_location(issues,
					       2,
					       case_list_itr->location,
					       cs->selector->invoke.location(&(cs->selector->invoke)));
		}
		issues->end_group(issues);

		if(comparable_result != ESSTEE_TRUE)
		{
		    return ESSTEE_ERROR;
		}
	    }
	}
	
	struct invoke_iface_t *statement_itr = NULL;
	DL_FOREACH(case_itr->statements, statement_itr)
	{
	    int verify_result = statement_itr->verify(statement_itr,
						      config,
						      issues);
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
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);

    cs->invoke_state = 0;

    if(cs->selector->invoke.reset)
    {
	int reset_result = cs->selector->invoke.reset(&(cs->selector->invoke),
						      config,
						      issues);
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
						    config,
						    issues);
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
						config,
						issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

int st_case_statement_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);

    if(cs->selector->invoke.allocate)
    {
	int allocate_result = cs->selector->invoke.allocate(
	    &(cs->selector->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    struct case_t *case_itr = NULL;
    DL_FOREACH(cs->cases, case_itr)
    {
	int allocate_result = st_allocate_statements(case_itr->statements,
						     issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    int else_allocate_result = st_allocate_statements(cs->else_statements,
						      issues);

    if(else_allocate_result != ESSTEE_OK)
    {
	return else_allocate_result;
    }

    return ESSTEE_OK;
}

struct invoke_iface_t * st_case_statement_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);

    struct case_statement_t *copy = NULL;
    struct expression_iface_t *selector_copy = NULL;
    struct invoke_iface_t *case_statements_copy = NULL;
    struct invoke_iface_t *else_statements_copy = NULL;
    struct case_t *case_itr = NULL;
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct case_statement_t,
	issues,
	error_free_resources);

    memcpy(copy, cs, sizeof(struct case_statement_t));

    if(cs->selector->clone)
    {
	selector_copy = cs->selector->clone(cs->selector, issues);

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
	ALLOC_OR_ERROR_JUMP(
	    case_copy,
	    struct case_t,
	    issues,
	    error_free_resources);

	memcpy(case_copy, case_itr, sizeof(struct case_t));
	case_copy->statements = NULL;

	case_statements_copy = NULL;
	struct invoke_iface_t *statement_itr = NULL;
	DL_FOREACH(case_itr->statements, statement_itr)
	{
	    struct invoke_iface_t *statement_copy =
		statement_itr->clone(statement_itr, issues);

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
	    = statement_itr->clone(statement_itr, issues);

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
    struct issues_iface_t *issues)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    switch(ifs->invoke_state)
    {
    case 0:
	if(ifs->condition->invoke.step)
	{
	    ifs->invoke_state = 1;
	    st_switch_current(cursor, &(ifs->condition->invoke), config, issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 1: {
	const struct value_iface_t *condition_value =
	    ifs->condition->return_value(ifs->condition);

	int current_condition = condition_value->bool(condition_value,
						      config,
						      issues);

	ifs->invoke_state = 2;
	
	if(current_condition == ESSTEE_TRUE)
	{
	    st_switch_current(cursor, ifs->true_statements, config, issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	else if(ifs->elsif)
	{
	    st_switch_current(cursor, &(ifs->elsif->invoke), config, issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	else if(ifs->else_statements)
	{
	    st_switch_current(cursor, ifs->else_statements, config, issues);
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
    struct issues_iface_t *issues)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    if(ifs->condition->invoke.verify)
    {
	int condition_verify =
	    ifs->condition->invoke.verify(&(ifs->condition->invoke),
					  config,
					  issues);
	if(condition_verify != ESSTEE_OK)
	{
	    return condition_verify;
	}
    }

    const struct value_iface_t *condition_value =
	ifs->condition->return_value(ifs->condition);

    if(!condition_value->bool)
    {
	issues->new_issue_at(issues,
			     "condition cannot be interpreted as true or false",
			     ISSUE_ERROR_CLASS,
			     1,
			     ifs->location);
	return ESSTEE_ERROR;
    }
    
    struct invoke_iface_t *true_itr;
    DL_FOREACH(ifs->true_statements, true_itr)
    {
	if(true_itr->verify(true_itr, config, issues) != ESSTEE_OK)
	{
	    return ESSTEE_ERROR;
	}
    }

    if(ifs->elsif)
    {
	return st_if_statement_verify(&(ifs->elsif->invoke),
				      config,
				      issues);
    }
    else if(ifs->else_statements)
    {
	struct invoke_iface_t *else_itr;
	DL_FOREACH(ifs->else_statements, else_itr)
	{
	    if(else_itr->verify(else_itr, config, issues) != ESSTEE_OK)
	    {
		return ESSTEE_ERROR;
	    }
	}
    }

    return ESSTEE_OK;
}

int st_if_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    ifs->invoke_state = 0;

    if(ifs->condition->invoke.reset)
    {
	int reset_result = ifs->condition->invoke.reset(&(ifs->condition->invoke),
							config,
							issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    struct invoke_iface_t *true_itr = NULL;
    DL_FOREACH(ifs->true_statements, true_itr)
    {
	int reset_result = true_itr->reset(true_itr, config, issues);

	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    if(ifs->elsif)
    {
	int reset_result = ifs->elsif->invoke.reset(&(ifs->elsif->invoke),
						    config,
						    issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }
    else if(ifs->else_statements)
    {
	struct invoke_iface_t *else_itr;
	DL_FOREACH(ifs->else_statements, else_itr)
	{
	    int reset_result = else_itr->reset(else_itr,
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

int st_if_statement_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    if(ifs->condition->invoke.allocate)
    {
	int allocate_result = ifs->condition->invoke.allocate(
	    &(ifs->condition->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    int true_allocate = st_allocate_statements(ifs->true_statements,
					       issues);
    if(true_allocate != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    if(ifs->elsif)
    {
	int allocate_result = ifs->elsif->invoke.allocate(
	    &(ifs->elsif->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }
    else if(ifs->else_statements)
    {
	return st_allocate_statements(ifs->else_statements, issues);
    }

    return ESSTEE_OK;
}

struct invoke_iface_t * st_if_statement_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    struct if_statement_t *copy = NULL;
    struct expression_iface_t *condition_copy = NULL;
    struct invoke_iface_t *true_statements_copy = NULL;
    struct invoke_iface_t *else_statements_copy = NULL;
    struct invoke_iface_t *destroy_itr = NULL;
    struct invoke_iface_t *tmp = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct if_statement_t,
	issues,
	error_free_resources);

    memcpy(copy, ifs, sizeof(struct if_statement_t));

    if(ifs->condition->clone)
    {
	condition_copy = ifs->condition->clone(ifs->condition, issues);

	if(!condition_copy)
	{
	    goto error_free_resources;
	}

	copy->condition = condition_copy;
    }

    struct invoke_iface_t *true_itr = NULL;
    DL_FOREACH(ifs->true_statements, true_itr)
    {
	struct invoke_iface_t *statement_copy = true_itr->clone(true_itr, issues);

	if(!statement_copy)
	{
	    goto error_free_resources;
	}

	DL_APPEND(true_statements_copy, statement_copy);
    }

    if(ifs->elsif)
    {
	struct invoke_iface_t *elsif_copy_invoke =
	    ifs->elsif->invoke.clone(&(ifs->elsif->invoke), issues);

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
	    struct invoke_iface_t *statement_copy = else_itr->clone(else_itr, issues);

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
    struct issues_iface_t *issues)
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
	    st_switch_current(cursor, &(ws->while_expression->invoke), config, issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 2:
    default:
	while_value = ws->while_expression->return_value(ws->while_expression);

	if(while_value->bool(while_value, config, issues) == ESSTEE_TRUE)
	{
	    ws->invoke_state = 1;
	    st_switch_current(cursor, ws->statements, config, issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
    }

    st_pop_exit_context(cursor);
    
    return INVOKE_RESULT_FINISHED;
}

int st_while_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);
    
    if(ws->while_expression->invoke.verify)
    {
	int verify_result =
	    ws->while_expression->invoke.verify(&(ws->while_expression->invoke),
						config,
						issues);
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
	
	issues->new_issue_at(issues,
			     "condition cannot be interpreted as true or false",
			     ISSUE_ERROR_CLASS,
			     1,
			     while_expression_location);
	return ESSTEE_ERROR;
    }

    return st_verify_statements(ws->statements, config, issues);
}

int st_while_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);

    ws->invoke_state = 0;

    if(ws->while_expression->invoke.reset)
    {
	int reset_result =
	    ws->while_expression->invoke.reset(&(ws->while_expression->invoke),
					       config,
					       issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(ws->statements, itr)
    {
	int reset_result = itr->reset(itr, config, issues);

	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }
    
    return ESSTEE_OK;
}

int st_while_statement_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);

    if(ws->while_expression->invoke.allocate)
    {
	int allocate_result = ws->while_expression->invoke.allocate(
	    &(ws->while_expression->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    return st_allocate_statements(ws->statements, issues);
}


struct invoke_iface_t * st_while_statement_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);

    struct while_statement_t *copy = NULL;
    struct invoke_iface_t *statements_copy = NULL;
    struct expression_iface_t *while_expression_copy = NULL;

    ALLOC_OR_ERROR_JUMP(
	copy,
	struct while_statement_t,
	issues,
	error_free_resources);

    memcpy(copy, ws, sizeof(struct while_statement_t));

    if(ws->while_expression->clone)
    {
	while_expression_copy = ws->while_expression->clone(ws->while_expression,
							    issues);

	if(!while_expression_copy)
	{
	    goto error_free_resources;
	}

	copy->while_expression = while_expression_copy;
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(ws->statements, itr)
    {
	struct invoke_iface_t *statement_copy = itr->clone(itr, issues);

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
    struct issues_iface_t *issues)
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
	    st_switch_current(cursor, &(fs->from->invoke), config, issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 1:
	variable_assign_result = fs->variable->value->assign(fs->variable->value,
							     from_value,
							     config,
							     issues);
	if(variable_assign_result != ESSTEE_OK)
	{
	    return INVOKE_RESULT_ERROR;
	}

    case 2:
	if(fs->to->invoke.step)
	{
	    fs->invoke_state = 3;
	    st_switch_current(cursor, &(fs->to->invoke), config, issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 3:
	if(fs->variable->value->greater(fs->variable->value, to_value, config, issues) == ESSTEE_TRUE)
	{
	    break;
	}
	
    case 4: 
	fs->invoke_state = 5;
	st_switch_current(cursor, fs->statements, config, issues);
	return INVOKE_RESULT_IN_PROGRESS;

    case 5:
	if(fs->increment && fs->increment->invoke.step)
	{
	    fs->invoke_state = 6;
	    st_switch_current(cursor, &(fs->increment->invoke), config, issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	
    case 6:
	if(fs->variable->value->plus(fs->variable->value, increment_value, config, issues) != ESSTEE_OK)
	{
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
    struct issues_iface_t *issues)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    if(fs->from->invoke.verify)
    {
	int verify_result = fs->from->invoke.verify(&(fs->from->invoke),
						    config,
						    issues);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }

    if(fs->to->invoke.verify)
    {
	int verify_result = fs->to->invoke.verify(&(fs->to->invoke),
						  config,
						  issues);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }
    
    if(fs->increment && fs->increment->invoke.verify)
    {
	int verify_result = fs->increment->invoke.verify(&(fs->increment->invoke),
							 config,
							 issues);
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


    if(!fs->variable->value->assignable_from)
    {
	const char *message = issues->build_message(
	    issues,
	    "iteration variable '%s' cannot be assigned a value",
	    fs->variable->identifier);
	
	issues->new_issue_at(issues,
			     message,
			     ESSTEE_TYPE_ERROR,
			     1,
			     fs->identifier_location);

	return ESSTEE_ERROR;
    }
    
    issues->begin_group(issues);
    int var_from_assignable = fs->variable->value->assignable_from(fs->variable->value,
								   from_value,
								   config,
								   issues);
    if(var_from_assignable != ESSTEE_TRUE)
    {
	issues->new_issue(issues,
			  "iteration variable '%s' cannot be assigned the from value",
			  ESSTEE_CONTEXT_ERROR,
			  fs->variable->identifier);

	issues->set_group_location(issues, 
				   2,
				   fs->identifier_location,
				   fs->from->invoke.location(&(fs->from->invoke)));
    }
    issues->end_group(issues);

    if(var_from_assignable != ESSTEE_TRUE)
    {
	return ESSTEE_ERROR;
    }
    
    const struct type_iface_t *var_type =
	fs->variable->value->type_of(fs->variable->value);

    issues->begin_group(issues);
    int type_can_hold_to_value = var_type->can_hold(var_type,
						    to_value,
						    config,
						    issues);
    if(type_can_hold_to_value != ESSTEE_TRUE)
    {
	issues->new_issue(issues,
			  "iteration variable '%s' cannot hold the end value",
			  ESSTEE_TYPE_ERROR,
			  fs->variable->identifier);

	issues->set_group_location(issues,
				   2,
				   fs->identifier_location,
				   fs->to->invoke.location(&(fs->to->invoke)));
    }
    issues->end_group(issues);

    if(type_can_hold_to_value != ESSTEE_TRUE)
    {
	return ESSTEE_ERROR;
    }

    issues->begin_group(issues);
    int var_increment_operates = fs->variable->value->operates_with(fs->variable->value,
								    increment_value,
								    config,
								    issues);

    if(var_increment_operates != ESSTEE_TRUE)
    {
	issues->new_issue(issues,
			  "iteration variable '%s' cannot be incremented as specified",
			  ESSTEE_CONTEXT_ERROR,
			  fs->variable->identifier);

	issues->set_group_location(issues,
				   2,
				   fs->identifier_location,
				   fs->increment->invoke.location(&(fs->increment->invoke)));
    }
    issues->end_group(issues);
    
    if(var_increment_operates != ESSTEE_TRUE)
    {
	return ESSTEE_ERROR;
    }

    issues->begin_group(issues);
    int var_to_comparable = fs->variable->value->comparable_to(fs->variable->value,
							       to_value,
							       config,
							       issues);

    if(var_to_comparable != ESSTEE_TRUE)
    {
	issues->new_issue(issues,
			  "iteration variable '%s' cannot be compared to end value",
			  ESSTEE_CONTEXT_ERROR,
			  fs->variable->identifier);

	issues->set_group_location(issues,
				   2,
				   fs->identifier_location,
				   fs->to->invoke.location(&(fs->to->invoke)));
    }
    issues->end_group(issues);
    
    if(var_to_comparable != ESSTEE_TRUE)
    {
	return ESSTEE_ERROR;
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(fs->statements, itr)
    {
	int verify_result = itr->verify(itr, config, issues);

	if(verify_result != ESSTEE_OK)
	{
	    return ESSTEE_ERROR;
	}
    }

    return ESSTEE_OK;
}

int st_for_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    fs->invoke_state = 0;
    
    if(fs->from->invoke.reset)
    {
	int reset_result = fs->from->invoke.reset(&(fs->from->invoke),
						  config,
						  issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }
    
    if(fs->to->invoke.reset)
    {
	int reset_result = fs->to->invoke.reset(&(fs->to->invoke),
						config,
						issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    if(fs->increment && fs->increment->invoke.reset)
    {
	int reset_result = fs->increment->invoke.reset(&(fs->increment->invoke),
						       config,
						       issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(fs->statements, itr)
    {
	int reset_result = itr->reset(itr,
				      config,
				      issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

int st_for_statement_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    if(fs->from->invoke.allocate)
    {
	int allocate_result = fs->from->invoke.allocate(
	    &(fs->from->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }
    
    if(fs->to->invoke.allocate)
    {
	int allocate_result = fs->to->invoke.allocate(&(fs->to->invoke),
						      issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    if(fs->increment && fs->increment->invoke.allocate)
    {
	int allocate_result = fs->increment->invoke.allocate(&(fs->increment->invoke),
							     issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    return st_allocate_statements(fs->statements, issues);
}

struct invoke_iface_t * st_for_statement_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    struct for_statement_t *copy = NULL;
    struct expression_iface_t *from_copy = NULL;
    struct expression_iface_t *to_copy = NULL;
    struct expression_iface_t *increment_copy = NULL;
    struct invoke_iface_t *statements_copy = NULL;
    struct invoke_iface_t *statement_copy = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct for_statement_t,
	issues,
	error_free_resources);

    memcpy(copy, fs, sizeof(struct for_statement_t));
    copy->statements = NULL;
    
    if(fs->from->clone)
    {
	from_copy = fs->from->clone(fs->from, issues);

	if(!from_copy)
	{
	    goto error_free_resources;
	}

	copy->from = from_copy;
    }
    
    if(fs->to->clone)
    {
	to_copy = fs->to->clone(fs->to, issues);

	if(!to_copy)
	{
	    goto error_free_resources;
	}

	copy->to = to_copy;
    }

    if(fs->increment && fs->increment->clone)
    {
	increment_copy = fs->increment->clone(fs->increment, issues);

	if(!increment_copy)
	{
	    goto error_free_resources;
	}

	copy->increment = increment_copy;
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(fs->statements, itr)
    {
	statement_copy = itr->clone(itr, issues);

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
    struct issues_iface_t *issues)
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
	st_switch_current(cursor, ws->statements, config, issues);
	return INVOKE_RESULT_IN_PROGRESS;

    case 2:
	if(ws->while_expression->invoke.step)
	{
	    ws->invoke_state = 3;
	    st_switch_current(cursor, &(ws->while_expression->invoke), config, issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 3:
	while_value = ws->while_expression->return_value(ws->while_expression);

	if(while_value->bool(while_value, config, issues) == ESSTEE_FALSE)
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
    struct issues_iface_t *issues)
{
    return st_jump_return(cursor);
}

int st_exit_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    st_jump_exit(cursor);
    return INVOKE_RESULT_FINISHED;
}

int st_pop_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}

int st_pop_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}

struct invoke_iface_t * st_pop_statement_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct pop_call_stack_statement_t *ps = 
	CONTAINER_OF(self, struct pop_call_stack_statement_t, invoke);

    struct pop_call_stack_statement_t *copy = NULL;
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct pop_call_stack_statement_t,
	issues,
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
