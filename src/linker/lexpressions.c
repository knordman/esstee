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

#include <linker/linker.h>
#include <elements/expressions.h>


int st_single_identifier_variable_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct single_identifier_term_t *sit
	= (struct single_identifier_term_t *)referrer;
    
    if(target != NULL)
    {
	sit->variable = (struct variable_t *)target;

	sit->expression.return_value = st_single_identifier_term_var_return_value;
    }
    else
    {
	/* Interpret as enum */
	sit->variable = NULL;
	
	sit->expression.return_value = st_single_identifier_term_enum_return_value;

	sit->inline_enum.data.group = NULL;
	sit->inline_enum.data.location = sit->location;
	
	memset(&(sit->inline_enum.value), 0, sizeof(struct value_iface_t));

	sit->inline_enum.value.display = st_inline_enum_value_display;
	sit->inline_enum.value.comparable_to = st_inline_enum_value_comparable_to;
	sit->inline_enum.value.equals = st_inline_enum_value_equals;
	sit->inline_enum.value.enumeration = st_inline_enum_value_enumeration;
    }

    return ESSTEE_OK;
}

int st_function_invocation_term_function_resolved(
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
	    ISSUE_ERROR_CLASS,
	    1,
	    location);
	
	return ESSTEE_ERROR;
    }
    
    struct function_invocation_term_t *ft =
	(struct function_invocation_term_t *)referrer;

    ft->function = (struct function_t *)target;

    return ESSTEE_OK;
}
