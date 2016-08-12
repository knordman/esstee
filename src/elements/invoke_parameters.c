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

#include <elements/invoke_parameters.h>
#include <util/macros.h>

#include <utlist.h>

/**************************************************************************/
/* Invoke parameters interface                                            */
/**************************************************************************/
struct invoke_parameter_t {
    char *identifier;
    struct expression_iface_t *expression;
    struct st_location_t *location;
    int invoke_state;
    struct invoke_parameter_t *prev;
    struct invoke_parameter_t *next;
};

struct invoke_parameters_t {
    struct invoke_parameters_iface_t params;
    struct invoke_parameter_t *list;
};

static int invoke_parameters_append(
    struct invoke_parameters_iface_t *self,
    struct invoke_parameter_t *parameter,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_parameters_t *params =
	CONTAINER_OF(self, struct invoke_parameters_t, params);

    DL_APPEND(params->list, parameter);

    return ESSTEE_OK;
}

static const struct variable_iface_t * next_input_variable(
    const struct variable_iface_t *variable_list,
    int accept_start)
{
    const struct variable_iface_t *itr = NULL;
    const struct variable_iface_t *next = NULL;
    
    DL_FOREACH(variable_list, itr)
    {
	if(ST_FLAG_IS_SET(itr->class, INPUT_VAR_CLASS))
	{
	    if(itr == variable_list && !accept_start)
	    {
		continue;
	    }
	    
	    next = itr;
	    break;
	}
    }
    
    return next;
}
    
static int invoke_parameters_verify(
    const struct invoke_parameters_iface_t *self,
    const struct variable_iface_t *variables,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct invoke_parameters_t *params =
	CONTAINER_OF(self, struct invoke_parameters_t, params);
    
    int verified = ESSTEE_OK;
    struct invoke_parameter_t *itr = NULL;
    const struct variable_iface_t *check_var = NULL;
    const struct variable_iface_t *last_var = NULL;
    
    DL_FOREACH(params->list, itr)
    {
	if(itr->identifier)
	{
	    HASH_FIND_STR(variables, itr->identifier, check_var);
	    
	    if(!check_var)
	    {
		verified = ESSTEE_ERROR;
		const char *message = issues->build_message(issues,
							    "no parameter by name '%s' defined",
							    itr->identifier);
		issues->new_issue_at(
		    issues,
		    message,
		    ESSTEE_CONTEXT_ERROR,
		    1,
		    itr->location);

		break;
	    }

	    if(!ST_FLAG_IS_SET(check_var->class, INPUT_VAR_CLASS))
	    {
		verified = ESSTEE_ERROR;
		const char *message = issues->build_message(issues,
							    "parameter '%s' is not an input variable",
							    itr->identifier);

		issues->new_issue_at(
		    issues,
		    message,
		    ESSTEE_CONTEXT_ERROR,
		    1,
		    itr->location);

		last_var = check_var;
		continue;
	    }
	}
	else
	{
	    check_var = (last_var) ?
		next_input_variable(last_var, 0) :
		next_input_variable(variables, 1);
	}

	if(!check_var)
	{
	    verified = ESSTEE_ERROR;

	    issues->new_issue_at(
		issues,
		"too many parameters given",
		ESSTEE_CONTEXT_ERROR,
		1,
		itr->location);

	    break;
	}

	if(itr->expression->invoke.verify)
	{
	    int expression_verified = itr->expression->invoke.verify(
		&(itr->expression->invoke),
		config,
		issues);
	    
	    if(expression_verified != ESSTEE_OK)
	    {
		verified = ESSTEE_ERROR;
	    }

	    continue;
	}

	const struct value_iface_t *assign_value =
	    itr->expression->return_value(itr->expression);

	issues->begin_group(issues);
	int variable_assignable = check_var->assignable_from(check_var,
							     NULL,
							     assign_value,
							     config,
							     issues);
	
	if(variable_assignable != ESSTEE_TRUE)
	{
	    verified = ESSTEE_ERROR;

	    if(itr->identifier)
	    {
		issues->new_issue(
		    issues,
		    "parameter '%s' cannot be assigned from the given value",
		    ESSTEE_ARGUMENT_ERROR,
		    check_var->identifier);
	    }
	    else
	    {
		issues->new_issue(
		    issues,
		    "one unnamed parameter cannot be assigned from the given value",
		    ESSTEE_ARGUMENT_ERROR,
		    check_var->identifier);
	    }
	    
	    issues->set_group_location(issues,
				       1,
				       itr->location);
	}
	issues->end_group(issues);

	last_var = check_var;
    }

    return verified;
}

static int invoke_parameters_step(
    struct invoke_parameters_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_parameters_t *params =
	CONTAINER_OF(self, struct invoke_parameters_t, params);

    struct invoke_parameter_t *itr = NULL;
    DL_FOREACH(params->list, itr)
    {
	if(itr->invoke_state == 0)
	{
	    break;
	}
    }

    /* Check if the ones left need to be stepped */
    for(; itr != NULL; itr = itr->next)
    {
	if(itr->expression->invoke.step)
	{
	    itr->invoke_state = 1;
	    cursor->switch_current(cursor,
				   &(itr->expression->invoke),
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	else
	{
	    itr->invoke_state = 1;
	}
    }

    return INVOKE_RESULT_FINISHED;
}

static int invoke_parameters_reset(
    struct invoke_parameters_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_parameters_t *params =
	CONTAINER_OF(self, struct invoke_parameters_t, params);

    struct invoke_parameter_t *itr = NULL;
    DL_FOREACH(params->list, itr)
    {
	if(itr->expression->invoke.reset)
	{
	    int reset_result = itr->expression->invoke.reset(
		&(itr->expression->invoke),
		config,
		issues);

	    if(reset_result != ESSTEE_OK)
	    {
		return reset_result;
	    }
	}

	itr->invoke_state = 0;
    }

    return ESSTEE_OK;
}

int invoke_parameters_allocate(
    struct invoke_parameters_iface_t *self,
    struct issues_iface_t *issues)
{
    struct invoke_parameters_t *params =
	CONTAINER_OF(self, struct invoke_parameters_t, params);

    struct invoke_parameter_t *itr = NULL;
    DL_FOREACH(params->list, itr)
    {
	if(itr->expression->invoke.allocate)
	{
	    int allocate_result = itr->expression->invoke.allocate(
		&(itr->expression->invoke),
		issues);

	    if(allocate_result != ESSTEE_OK)
	    {
		return allocate_result;
	    }
	}
    }

    return ESSTEE_OK;
}

void invoke_parameters_destroy(
    struct invoke_parameters_iface_t *self)
{
    /* TODO: invoke parameters destructor */
}

void invoke_parameters_clone_destroy(
    struct invoke_parameters_iface_t *self)
{
    /* TODO: invoke parameters clone destructor */
}

struct invoke_parameters_iface_t * invoke_parameters_clone(
    struct invoke_parameters_iface_t *self,
    struct issues_iface_t *issues)
{
    struct invoke_parameters_t *params =
	CONTAINER_OF(self, struct invoke_parameters_t, params);

    struct invoke_parameters_t *parameters_copy = NULL;
    struct invoke_parameter_t *list_copy = NULL;
    struct invoke_parameter_t *parameter_copy = NULL;
    struct expression_iface_t *parameter_expression_copy = NULL;

    ALLOC_OR_ERROR_JUMP(
	parameters_copy,
	struct invoke_parameters_t,
	issues,
	error_free_resources);

    memcpy(parameters_copy, params, sizeof(struct invoke_parameters_t));
    
    struct invoke_parameter_t *itr = NULL;
    DL_FOREACH(params->list, itr)
    {
	parameter_expression_copy = NULL;
	
	ALLOC_OR_ERROR_JUMP(
	    parameter_copy,
	    struct invoke_parameter_t,
	    issues,
	    error_free_resources);

	memcpy(parameter_copy, itr, sizeof(struct invoke_parameter_t));

	if(itr->expression->clone)
	{
	    parameter_expression_copy =
		itr->expression->clone(itr->expression, issues);

	    if(!parameter_expression_copy)
	    {
		goto error_free_resources;
	    }

	    parameter_copy->expression = parameter_expression_copy;
	}

	DL_APPEND(list_copy, parameter_copy);
    }

    parameters_copy->params.destroy = invoke_parameters_clone_destroy;
    parameters_copy->list = list_copy;

    return &(parameters_copy->params);
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}
    
int invoke_parameters_assign_from(
    const struct invoke_parameters_iface_t *self,
    struct variable_iface_t *variables,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct invoke_parameters_t *params =
	CONTAINER_OF(self, struct invoke_parameters_t, params);

    struct variable_iface_t *assign_var = NULL;
    struct variable_iface_t *last_var = NULL;
    struct invoke_parameter_t *itr = NULL;
    
    DL_FOREACH(params->list, itr)
    {
	if(itr->identifier)
	{
	    HASH_FIND_STR(variables, itr->identifier, assign_var);
	}
	else
	{
	    assign_var = (last_var) ?
		(struct variable_iface_t *)next_input_variable(last_var, 0) :
		(struct variable_iface_t *)next_input_variable(variables, 1);
	}

	if(assign_var)
	{
	    const struct value_iface_t *parameter_value =
		itr->expression->return_value(itr->expression);
	    
	    int assign_result = assign_var->assign(assign_var,
						   NULL,
						   parameter_value,
						   config,
						   issues);

	    if(assign_result != ESSTEE_OK)
	    {
		return ESSTEE_ERROR;
	    }
	}

	last_var = assign_var;
    }

    return ESSTEE_OK;   
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct invoke_parameters_iface_t * st_create_invoke_parameters(
    struct invoke_parameter_t *first_parameter,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_parameters_t *params = NULL;

    ALLOC_OR_ERROR_JUMP(
	params,
	struct invoke_parameters_t,
	issues,
	error_free_resources);

    params->list = NULL;
    DL_APPEND(params->list, first_parameter);

    memset(&(params->params), 0, sizeof(struct invoke_parameters_iface_t));
    params->params.append = invoke_parameters_append;
    params->params.verify = invoke_parameters_verify;
    params->params.step = invoke_parameters_step;
    params->params.reset = invoke_parameters_reset;
    params->params.allocate = invoke_parameters_allocate;
    params->params.clone = invoke_parameters_clone;
    params->params.assign_from = invoke_parameters_assign_from;
    params->params.destroy = invoke_parameters_destroy;

    return &(params->params);

error_free_resources:
    return NULL;
}
    
struct invoke_parameter_t * st_create_invoke_parameter(
    char *identifier,
    const struct st_location_t *location,
    struct expression_iface_t *assigned,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_parameter_t *ip = NULL;
    struct st_location_t *ip_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	ip,
	struct invoke_parameter_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	ip_location,
	location,
	issues,
	error_free_resources);    

    ip->location = ip_location;
    ip->identifier = identifier;
    ip->expression = assigned;

    return ip;
    
error_free_resources:
    free(ip);
    free(ip_location);
    return NULL;
}

void st_destroy_invoke_parameter(
    struct invoke_parameter_t *parameter)
{
    /* TODO: invoke parameter destroy */
}
