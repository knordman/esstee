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
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
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
	sit->expression.return_value = st_single_identifier_term_enum_return_value;

	sit->inline_enum.group = NULL;
	sit->inline_enum.location = NULL;
	sit->inline_enum.identifier = sit->identifier;
	
	memset(&(sit->value), 0, sizeof(struct value_iface_t));

	sit->value.display = st_inline_enum_value_display;
	sit->value.comparable_to = st_inline_enum_value_comparable_to;
	sit->value.equals = st_inline_enum_value_equals;
	sit->value.enumeration = st_inline_enum_value_enumeration;
    }

    return ESSTEE_OK;
}
