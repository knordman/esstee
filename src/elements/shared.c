/*
eCopyright (C) 2015 Kristian Nordman

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
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    struct qualified_identifier_t *start_from = qi;
    
    if(qi->program)
    {
	if(qi->next)
	{
	    if(!(qi->program->header && qi->program->header->variables))
	    {
		errors->new_issue_at(errors,
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
 		errors->new_issue_at(errors,
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
		errors->new_issue_at(
		    errors,
		    "variable does not have any sub-variables",
		    ISSUE_ERROR_CLASS,
		    1,
		    itr->location);
		
		return ESSTEE_ERROR;
	    }
	    
	    struct variable_t *subvar = itr->variable->value->sub_variable(
		itr->variable->value,
		itr->next->identifier,
		config);

	    if(subvar == NULL)
	    {
		errors->new_issue_at(
		    errors,
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
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    struct qualified_identifier_t *last = qi->last;
    
    if(last->array_index != NULL)
    {
	if(!last->variable->value->index)
	{
	    errors->new_issue_at(
		errors,
		"variable is not indexable",
		ISSUE_ERROR_CLASS,
		1,
		last->location);
		
	    return ESSTEE_ERROR;
	}

	struct value_iface_t *array_value = last->variable->value->index(
	    last->variable->value,
	    last->array_index,
	    config);
	    
	if(array_value == NULL)
	{
	    errors->new_issue_at(
		errors,
		"index out of range for array variable",
		ISSUE_ERROR_CLASS,
		2,
		last->location,
		last->array_index->location);
		
	    return ESSTEE_ERROR;
	}

	qi->target = array_value;
    }

    return ESSTEE_OK;
}

int st_qualified_identifier_verify(
    struct qualified_identifier_t *qi,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    int chain_resolve = st_qualified_identifier_resolve_chain(qi,
							      errors,
							      config);
    if(chain_resolve != ESSTEE_OK)
    {
	return chain_resolve;
    }

    if(qi->runtime_constant_reference)
    {
	int array_index_resolve = st_qualified_identifier_resolve_array_index(
	    qi,
	    errors,
	    config);

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
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
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

		DL_APPEND(cursor->call_stack, &(itr->index_expression->invoke));
		cursor->current = &(itr->index_expression->invoke);

		return INVOKE_RESULT_IN_PROGRESS;
	    }
	    else
	    {
		qi->invoke_state++;
	    }
	}

	int array_index_resolve = st_qualified_identifier_resolve_array_index(
	    qi,
	    errors,
	    config);

	if(array_index_resolve != ESSTEE_OK)
	{
	    return INVOKE_RESULT_ERROR;
	}
    }

    return INVOKE_RESULT_FINISHED;
}


int st_qualified_identifier_reset(
    struct qualified_identifier_t *qi)
{
    qi->invoke_state = 0;

    struct array_index_t *itr = NULL;
    DL_FOREACH(qi->last->array_index, itr)
    {
	if(itr->index_expression->invoke.step)
	{
	    int reset = itr->index_expression->invoke.reset(
		&(itr->index_expression->invoke));

	    if(reset != ESSTEE_OK)
	    {
		return reset;
	    }
	}
    }

    return ESSTEE_OK;
}

void st_destroy_qualified_identifier(
    struct qualified_identifier_t *qi)
{
    /* TODO: destructor for qualified identifier */
}
