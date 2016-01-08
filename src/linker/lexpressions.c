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
#include <elements/values.h>


int st_single_identifier_variable_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors)
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
	
	sit->value.display = NULL; /* TODO: set to correct function */
	sit->value.compatible = NULL; /* TODO: set to correct function */
	sit->value.equals = NULL;     /* TODO: set to correct function */
	sit->value.enumeration = NULL; /* TODO: set correct function */
    }

    return ESSTEE_OK;
}
