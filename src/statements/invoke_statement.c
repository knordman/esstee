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

#include <statements/invoke_statement.h>
#include <util/macros.h>
#include <elements/ifunction.h>

/**************************************************************************/
/* Invoke interface                                                       */
/**************************************************************************/
struct invoke_statement_t {
    struct invoke_iface_t invoke;
    struct st_location_t *location;
    struct invoke_parameters_iface_t *parameters;
    struct variable_iface_t *variable;
    struct function_iface_t *function;
    int invoke_state;
};

static int invoke_statement_verify(
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

	issues->begin_group(issues);

	int var_invoke = is->variable->invoke_verify(is->variable,
						     NULL,
						     is->parameters,
						     config,
						     issues);

	if(var_invoke != ESSTEE_OK)
	{
	    issues->set_group_location(issues,
				       1,
				       is->location);
	    issues->end_group(issues);
	    
	    return var_invoke;
	}

	issues->end_group(issues);
    }
    else if(is->function)
    {
	int verify_result = is->function->verify_invoke(
	    is->function,
	    is->parameters,
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

static int invoke_statement_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_statement_t *is =
	CONTAINER_OF(self, struct invoke_statement_t, invoke);

    switch(is->invoke_state)
    {
    case 0:
	cursor->push_return_context(cursor, self);
	
	if(is->parameters)
	{
	    is->invoke_state = 1;
	    	    
	    int step_result = is->parameters->step(is->parameters,
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
	    is->invoke_state = 2;
	    int step_result = is->variable->invoke_step(
		is->variable,
		NULL,
		is->parameters,
		cursor,
		time,
		config,
		issues);

	    if(step_result != INVOKE_RESULT_FINISHED)
	    {
		return step_result;
	    }
	}
	else if(is->function)
	{
	    int step_result = is->function->step(
		is->function,
		is->parameters,
		cursor,
		time,
		config,
		issues);

	    if(step_result != INVOKE_RESULT_FINISHED)
	    {
		return step_result;
	    }
	}
	
    case 2:
    default:
	break;
    }

    cursor->pop_return_context(cursor);
    
    return INVOKE_RESULT_FINISHED;
}

static void invoke_statement_destroy(
    struct invoke_iface_t *self)
{
    /* struct invoke_statement_t *is = */
    /* 	CONTAINER_OF(self, struct invoke_statement_t, invoke); */

    /* TODO: invoke destructor */
}

static void invoke_statement_clone_destroy(
    struct invoke_iface_t *self)
{
    /* struct invoke_statement_t *is = */
    /* 	CONTAINER_OF(self, struct invoke_statement_t, invoke); */

    /* TODO: clone destructor invoke statement */
}

static int invoke_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_statement_t *is =
	CONTAINER_OF(self, struct invoke_statement_t, invoke);

    if(is->variable)
    {
	int reset_result = is->variable->invoke_reset(is->variable,
						      NULL,
						      config,
						      issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }
    else if(is->function)
    {
	int reset_result = is->function->reset(is->function,
					       config,
					       issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    is->invoke_state = 0;
    if(is->parameters)
    {
	int reset_result = is->parameters->reset(is->parameters,
						 config,
						 issues);

	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

static int invoke_statement_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct invoke_statement_t *is =
	CONTAINER_OF(self, struct invoke_statement_t, invoke);

    return is->parameters->allocate(is->parameters, issues);
}

struct invoke_iface_t * invoke_statement_clone(
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

    if(is->parameters)
    {
	struct invoke_parameters_iface_t *parameters_copy =
	    is->parameters->clone(is->parameters, issues);

	if(!parameters_copy)
	{
	    goto error_free_resources;
	}

	copy->parameters = parameters_copy;
    }    

    copy->invoke.destroy = invoke_statement_clone_destroy;

    return &(copy->invoke);

error_free_resources:
    free(copy);
    return NULL;
}

/**************************************************************************/
/* Linker callbacks                                                       */
/**************************************************************************/
static int invoke_statement_as_variable_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_statement_t *is =
	(struct invoke_statement_t *)referrer;

    is->variable = (struct variable_iface_t *)target;

    return ESSTEE_OK;
}

static int invoke_statement_as_function_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_statement_t *is =
	(struct invoke_statement_t *)referrer;

    is->function = (struct function_iface_t *)target;

    return ESSTEE_OK;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct invoke_iface_t * st_create_invoke_statement(
    char *identifier,
    const struct st_location_t *identifier_location,
    struct invoke_parameters_iface_t *invoke_parameters,
    const struct st_location_t *location,
    struct named_ref_pool_iface_t *var_refs,
    struct named_ref_pool_iface_t *function_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_statement_t *is = NULL;
    struct st_location_t *is_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	is,
	struct invoke_statement_t,
	issues,
	error_free_resources);
    
    LOCDUP_OR_ERROR_JUMP(
	is_location,
	identifier_location,
	issues,
	error_free_resources);

    int var_ref_result = var_refs->add(
	var_refs, 
	identifier,
	is,
	identifier_location,
	invoke_statement_as_variable_resolved,
	issues);

    if(var_ref_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    int function_ref_result = function_refs->add(
	function_refs, 
	identifier,
	is,
	identifier_location,
	invoke_statement_as_function_resolved,
	issues);

    if(function_ref_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    is->location = is_location;
    is->parameters = invoke_parameters;

    memset(&(is->invoke), 0, sizeof(struct invoke_iface_t));
    is->invoke.location = is->location;
    is->invoke.step = invoke_statement_step;
    is->invoke.verify = invoke_statement_verify;
    is->invoke.allocate = invoke_statement_allocate;
    is->invoke.reset = invoke_statement_reset;
    is->invoke.clone = invoke_statement_clone;
    is->invoke.destroy = invoke_statement_destroy;
    
    return &(is->invoke);
    
error_free_resources:
    free(is);
    free(is_location);
    return NULL;
}
