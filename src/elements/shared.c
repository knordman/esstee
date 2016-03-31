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

#include <elements/shared.h>
#include <util/macros.h>

#include <utlist.h>

/**************************************************************************/
/* Array index                                                            */
/**************************************************************************/
void st_destroy_array_index(
    struct array_index_t *ai)
{
    /* TODO: destructor array index */
}

/**************************************************************************/
/* Qualified identifier                                                   */
/**************************************************************************/
int st_qualified_identifier_resolve_chain(
    struct qualified_identifier_t *qi,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *start_from = qi;
    
    if(qi->program)
    {
	if(qi->next)
	{
	    if(!(qi->program->header && qi->program->header->variables))
	    {
		issues->new_issue_at(issues,
				     "program has no variables",
				     ISSUE_ERROR_CLASS,
				     1,
				     qi->location);

		return ESSTEE_ERROR;
	    }
	    
	    struct variable_t *found = NULL;
	    HASH_FIND_STR(qi->program->header->variables, qi->next->identifier,	found);
	    if(!found)
	    {
 		issues->new_issue_at(issues,
				     "program has no such variable",
				     ISSUE_ERROR_CLASS,
				     1,
				     qi->next->location);

		return ESSTEE_ERROR;
	    }

	    qi->next->variable = found;

	    if(qi->next->next || qi->next->array_index)
	    {
		start_from = qi->next;
	    }
	    else
	    {
		qi->target = qi->next->variable->value;
		return ESSTEE_OK;
	    }
	}
	else
	{
	    /* Only referring to a program, then no value target */
	    qi->target = NULL;
	    return ESSTEE_OK;
	}
    }
    
    struct qualified_identifier_t *itr = NULL;
    DL_FOREACH(start_from, itr)
    {
	if(itr->array_index != NULL)
	{
	    return ESSTEE_OK;
	}
	else if(itr->next != NULL)
	{
	    if(!itr->variable->value->sub_variable)
	    {
		issues->new_issue_at(
		    issues,
		    "variable does not have any sub-variables",
		    ISSUE_ERROR_CLASS,
		    1,
		    itr->location);
		
		return ESSTEE_ERROR;
	    }
	    
	    struct variable_t *subvar = itr->variable->value->sub_variable(
		itr->variable->value,
		itr->next->identifier,
		config,
		issues);

	    if(subvar == NULL)
	    {
		issues->new_issue_at(
		    issues,
		    "reference to undefined variable",
		    ISSUE_ERROR_CLASS,
		    1,
		    itr->next->location);
		    
		return ESSTEE_ERROR;
	    }
	    else
	    {
		itr->next->variable = subvar;
	    }
	}
	else
	{
	    break;
	}
    }

    qi->target = itr->variable->value;
    return ESSTEE_OK;
}

int st_qualified_identifier_resolve_array_index(
    struct qualified_identifier_t *qi,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *last = qi->last;
    
    if(last->array_index != NULL)
    {
	if(!last->variable->value->index)
	{
	    issues->new_issue_at(
		issues,
		"variable is not indexable",
		ISSUE_ERROR_CLASS,
		1,
		last->location);
		
	    return ESSTEE_ERROR;
	}

	struct value_iface_t *array_value = last->variable->value->index(
	    last->variable->value,
	    last->array_index,
	    config,
	    issues);
	    
	if(array_value == NULL)
	{		
	    return ESSTEE_ERROR;
	}

	qi->target = array_value;
    }

    return ESSTEE_OK;
}

int st_qualified_identifier_verify(
    struct qualified_identifier_t *qi,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    int chain_resolve = st_qualified_identifier_resolve_chain(qi,
							      config,
							      issues);
    if(chain_resolve != ESSTEE_OK)
    {
	return chain_resolve;
    }

    if(qi->runtime_constant_reference)
    {
	int array_index_resolve = st_qualified_identifier_resolve_array_index(
	    qi,
	    config,
	    issues);

	if(array_index_resolve != ESSTEE_OK)
	{
	    return array_index_resolve;
	}
    }

    return ESSTEE_OK;
}

int st_qualified_identifier_step(
    struct qualified_identifier_t *qi,
    struct cursor_t *cursor,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!qi->runtime_constant_reference)
    {
	/* Skip already stepped indices */
	struct array_index_t *itr = NULL;
	int skipped = 0;
	DL_FOREACH(qi->last->array_index, itr)
	{
	    if(skipped >= qi->invoke_state)
	    {
		break;
	    }

	    skipped++;
	}

	/* Check if the ones left need to be stepped */
	for(; itr != NULL; itr = itr->next)
	{
	    if(itr->index_expression->invoke.step)
	    {
		/* Push to call stack (to be stepped), when stepping is
		 * complete, index has been evaluated (=increase state) */
		qi->invoke_state++;
		st_switch_current(cursor, &(itr->index_expression->invoke), config, issues);
		return INVOKE_RESULT_IN_PROGRESS;
	    }
	    else
	    {
		qi->invoke_state++;
	    }
	}

	int array_index_resolve =
	    st_qualified_identifier_resolve_array_index(qi,
							config,
							issues);

	if(array_index_resolve != ESSTEE_OK)
	{
	    return INVOKE_RESULT_ERROR;
	}
    }

    return INVOKE_RESULT_FINISHED;
}

int st_qualified_identifier_reset(
    struct qualified_identifier_t *qi,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    qi->invoke_state = 0;

    struct array_index_t *itr = NULL;
    DL_FOREACH(qi->last->array_index, itr)
    {
	if(itr->index_expression->invoke.step)
	{
	    int reset_result = itr->index_expression->invoke.reset(
		&(itr->index_expression->invoke),
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

int st_qualified_identifier_allocate(
    struct qualified_identifier_t *qi,
    struct issues_iface_t *issues)
{
    struct array_index_t *itr = NULL;
    DL_FOREACH(qi->last->array_index, itr)
    {
	if(itr->index_expression->invoke.allocate)
	{
	    int allocate_result = itr->index_expression->invoke.allocate(
		&(itr->index_expression->invoke),
		issues);

	    if(allocate_result != ESSTEE_OK)
	    {
		return allocate_result;
	    }
	}
    }

    return ESSTEE_OK;
}


struct qualified_identifier_t * st_clone_qualified_identifier(
    struct qualified_identifier_t *qi,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_t *copy = NULL;
    struct array_index_t *array_index_copy = NULL;
    struct array_index_t *array_index_element_copy = NULL;
    struct expression_iface_t *index_expression_copy = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct qualified_identifier_t,
	issues,
	error_free_resources);

    memcpy(copy, qi, sizeof(struct qualified_identifier_t));

    if(!qi->runtime_constant_reference)
    {
	struct array_index_t *itr = NULL;
	DL_FOREACH(qi->array_index, itr)
	{
	    ALLOC_OR_JUMP(
		array_index_element_copy,
		struct array_index_t,
		error_free_resources);

	    memcpy(array_index_element_copy, itr, sizeof(struct array_index_t));
	    
	    if(itr->index_expression->clone)
	    {
		index_expression_copy = itr->index_expression->clone(
		    itr->index_expression,
		    issues);

		if(!index_expression_copy)
		{
		    goto error_free_resources;
		}
	    }

	    DL_APPEND(array_index_copy, array_index_element_copy);
	}

	copy->array_index = array_index_copy;
    }

    return copy;
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

void st_destroy_qualified_identifier(
    struct qualified_identifier_t *qi)
{
    /* TODO: destructor for qualified identifier */
}

void st_destroy_qualified_identifier_clone(
    struct qualified_identifier_t *qi)
{
    if(!qi->runtime_constant_reference)
    {
	/* TODO: destructor */	
    }
}

/**************************************************************************/
/* Invoke parameters                                                      */
/**************************************************************************/
int st_verify_invoke_parameters(
    struct invoke_parameter_t *parameters,
    const struct variable_t *variables,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    int verified = ESSTEE_OK;
    struct variable_t *found = NULL;
    struct invoke_parameter_t *itr = NULL;
    DL_FOREACH(parameters, itr)
    {
	if(!itr->identifier)	/* TODO: process parameters without identifier */
	{
	    verified = ESSTEE_ERROR;
	    issues->new_issue_at(
		issues,
		"input variables must be referred to by name",
		ISSUE_ERROR_CLASS,
		1,
		itr->location);

	    continue;
	}
	
	HASH_FIND_STR(variables, itr->identifier, found);
	if(!found)
	{
	    verified = ESSTEE_ERROR;
	    const char *message = issues->build_message(issues,
							"no variable by name '%s' defined",
							itr->identifier);
	    
	    issues->new_issue_at(
		issues,
		message,
		ISSUE_ERROR_CLASS,
		1,
		itr->location);

	    continue;
	}

	if(!ST_FLAG_IS_SET(found->class, INPUT_VAR_CLASS))
	{
	    verified = ESSTEE_ERROR;
	    const char *message = issues->build_message(issues,
							"variable '%s' is not an input variable",
							itr->identifier);

	    issues->new_issue_at(
		issues,
		message,
		ISSUE_ERROR_CLASS,
		1,
		itr->location);

	    continue;
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
	int variable_assignable = found->value->assignable_from(found->value,
								assign_value,
								config,
								issues);
	
	if(variable_assignable != ESSTEE_TRUE)
	{
	    verified = ESSTEE_ERROR;

	    issues->new_issue(
		issues,
		"variable '%s' cannot be assigned from given value",
		ESSTEE_ARGUMENT_ERROR,
		found->identifier);

	    issues->set_group_location(issues,
				       1,
				       itr->location);				       
	}
	issues->end_group(issues);
    }

    return verified;
}

int st_step_invoke_parameters(
    struct invoke_parameter_t *parameters,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_parameter_t *itr = NULL;
    int skipped = 0;
    DL_FOREACH(parameters, itr)
    {
	if(skipped >= parameters->invoke_state)
	{
	    break;
	}

	skipped++;
    }

    /* Check if the ones left need to be stepped */
    for(; itr != NULL; itr = itr->next)
    {
	if(itr->expression->invoke.step)
	{
	    parameters->invoke_state++;
	    st_switch_current(cursor, &(itr->expression->invoke), config, issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	else
	{
	    parameters->invoke_state++;
	}
    }

    return INVOKE_RESULT_FINISHED;
}

int st_assign_from_invoke_parameters(
    struct invoke_parameter_t *parameters,
    struct variable_t *variables,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *found = NULL;
    struct invoke_parameter_t *itr = NULL;
    DL_FOREACH(parameters, itr)
    {
	/* TODO: handle assignemnts without identifier */
	HASH_FIND_STR(variables, itr->identifier, found);
	if(found)
	{
	    const struct value_iface_t *parameter_value =
		itr->expression->return_value(itr->expression);
	    
	    int assign_result = found->value->assign(found->value,
						     parameter_value,
						     config,
						     issues);

	    if(assign_result != ESSTEE_OK)
	    {
		return ESSTEE_ERROR;
	    }

	}
    }

    return ESSTEE_OK;
}

int st_reset_invoke_parameters(    
    struct invoke_parameter_t *parameters,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_parameter_t *itr = NULL;
    DL_FOREACH(parameters, itr)
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

int st_allocate_invoke_parameters(    
    struct invoke_parameter_t *parameters,
    struct issues_iface_t *issues)
{
    struct invoke_parameter_t *itr = NULL;
    DL_FOREACH(parameters, itr)
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

struct invoke_parameter_t * st_clone_invoke_parameters(    
    struct invoke_parameter_t *parameters,
    struct issues_iface_t *issues)
{
    struct invoke_parameter_t *parameters_copy = NULL;
    struct invoke_parameter_t *parameter_copy = NULL;
    struct expression_iface_t *parameter_expression_copy = NULL;

    struct invoke_parameter_t *itr = NULL;
    DL_FOREACH(parameters, itr)
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

	DL_APPEND(parameters_copy, parameter_copy);
    }

    return parameters_copy;
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

void st_destroy_invoke_parameters(
    struct invoke_parameter_t *parameters)
{
    /* TODO: destructor */
}

void st_destroy_invoke_parameters_clone(
    struct invoke_parameter_t *parameters)
{
    /* TODO: destructor */
}
