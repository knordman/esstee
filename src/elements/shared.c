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
int st_inner_resolve_qualified_identifier(
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
	    if(!itr->variable->value->index)
	    {
		errors->new_issue_at(
		    errors,
		    "variable is not indexable",
		    ISSUE_ERROR_CLASS,
		    1,
		    itr->location);
		
		return ESSTEE_ERROR;
	    }

	    struct value_iface_t *array_value = itr->variable->value->index(
		itr->variable->value,
		itr->array_index,
		config);
	    
	    if(array_value == NULL)
	    {
		errors->new_issue_at(
		    errors,
		    "index out of range for array variable",
		    ISSUE_ERROR_CLASS,
		    2,
		    itr->location,
		    itr->array_index->location);
		
		return ESSTEE_ERROR;
	    }

	    qi->target = array_value;
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
		itr->next->identifier);

	    if(subvar == NULL)
	    {
		errors->new_issue_at(
		    errors,
		    "undefined variable",
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
    }

    qi->target = itr->variable->value;
    return ESSTEE_OK;
}

void st_destroy_qualified_identifier(
    struct qualified_identifier_t *qi)
{
    /* TODO: destructor for qualified identifier */
}
