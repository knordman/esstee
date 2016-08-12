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

#include <expressions/function_invocation.h>
#include <elements/ifunction.h>
#include <util/macros.h>

/**************************************************************************/
/* Expression interface                                                   */
/**************************************************************************/
struct function_invocation_term_t {
    struct expression_iface_t expression;
    struct function_iface_t *function;
    struct invoke_parameters_iface_t *parameters;
    struct st_location_t *location;
    int invoke_state;
};

static int function_invocation_term_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct function_invocation_term_t *ft =
	CONTAINER_OF(expr, struct function_invocation_term_t, expression);

    return ft->function->verify_invoke(ft->function,
				       ft->parameters,
				       config,
				       issues);
}

static int function_invocation_term_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct function_invocation_term_t *ft =
	CONTAINER_OF(expr, struct function_invocation_term_t, expression);

    switch(ft->invoke_state)
    {
    case 0:
	cursor->push_return_context(cursor, self);
	
	if(ft->parameters)
	{
	    int step_result = ft->parameters->step(ft->parameters,
						   cursor,
						   time,
						   config,
						   issues);
	    
	    if(step_result != INVOKE_RESULT_FINISHED)
	    {
		return step_result;
	    }

	    ft->invoke_state = 1;
	}
	
    case 1: {
	int step_result = ft->function->step(
	    ft->function,
	    ft->parameters,
	    cursor,
	    time,
	    config,
	    issues);

	if(step_result != INVOKE_RESULT_FINISHED)
	{
	    return step_result;
	}

	ft->invoke_state = 2;
    }
	
    case 2:
    default:
	break;
    }

    cursor->pop_return_context(cursor);
    
    return INVOKE_RESULT_FINISHED;
}

static int function_invocation_term_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct function_invocation_term_t *ft =
	CONTAINER_OF(expr, struct function_invocation_term_t, expression);
    
    int reset_result = ft->function->reset(ft->function,
					   config,
					   issues);
    if(reset_result != ESSTEE_OK)
    {
	return reset_result;
    }

    if(ft->parameters)
    {
	reset_result = ft->parameters->reset(ft->parameters,
					     config,
					     issues);

	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

static int function_invocation_term_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct function_invocation_term_t *ft =
	CONTAINER_OF(expr, struct function_invocation_term_t, expression);

    if(ft->parameters)
    {
	return ft->parameters->allocate(ft->parameters,
					issues);
    }

    return ESSTEE_OK;
}

static const struct value_iface_t * function_invocation_term_return_value(
    struct expression_iface_t *self)
{
    struct function_invocation_term_t *ft =
	CONTAINER_OF(self, struct function_invocation_term_t, expression);
    
    return ft->function->result_value(ft->function);
}

static void function_invocation_term_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

static void function_invocation_clone_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

static struct expression_iface_t * function_invocation_term_clone(
    struct expression_iface_t *self,
    struct issues_iface_t *issues)
{
    struct function_invocation_term_t *ft =
	CONTAINER_OF(self, struct function_invocation_term_t, expression);

    struct function_invocation_term_t *copy = NULL;
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct function_invocation_term_t,
	issues,
	error_free_resources);

    memcpy(copy, ft, sizeof(struct function_invocation_term_t));

    struct invoke_parameters_iface_t *parameters_copy =
        ft->parameters->clone(ft->parameters, issues);
    
    if(!parameters_copy)
    {
	goto error_free_resources;
    }

    copy->parameters = parameters_copy;
    copy->expression.destroy = function_invocation_clone_destroy;

    return &(copy->expression);
    
error_free_resources:
    return NULL;
}

/**************************************************************************/
/* Linker callbacks                                                       */
/**************************************************************************/
static int function_invocation_term_function_resolved(
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
	    "reference to undefined function '%s'",
	    identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_LINK_ERROR,
	    1,
	    location);
	
	return ESSTEE_ERROR;
    }
    
    struct function_invocation_term_t *ft =
	(struct function_invocation_term_t *)referrer;

    ft->function = (struct function_iface_t *)target;

    return ESSTEE_OK;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct expression_iface_t * st_create_function_invocation_term(
    char *function_identifier,
    const struct st_location_t *location,
    struct named_ref_pool_iface_t *function_refs,
    struct invoke_parameters_iface_t *invoke_parameters,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct function_invocation_term_t *ft = NULL;
    struct st_location_t *ft_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	ft,
	struct function_invocation_term_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	ft_location,
	location,
	issues,
	error_free_resources);

    int ref_add_result = function_refs->add(
	function_refs,
	function_identifier,
	ft,
	location,
	function_invocation_term_function_resolved,
	issues);

    if(ref_add_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    ft->location = ft_location;
    ft->function = NULL;
    ft->parameters = invoke_parameters;

    memset(&(ft->expression), 0, sizeof(struct expression_iface_t));
    
    ft->expression.invoke.location = ft->location;
    ft->expression.invoke.step = function_invocation_term_step;
    ft->expression.invoke.verify = function_invocation_term_verify;
    ft->expression.invoke.reset = function_invocation_term_reset;
    ft->expression.invoke.allocate = function_invocation_term_allocate;
    ft->expression.return_value = function_invocation_term_return_value;
    ft->expression.clone = function_invocation_term_clone;
    ft->expression.destroy = function_invocation_term_destroy;

    return &(ft->expression);
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}
