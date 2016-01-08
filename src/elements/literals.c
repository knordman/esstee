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

#include <elements/literals.h>
#include <util/macros.h>


/**************************************************************************/
/* Integer literals                                                       */
/**************************************************************************/
const struct st_location_t * st_integer_literal_location(
    const struct invoke_iface_t *self)
{
    struct expression_iface_t *ile =
	CONTAINER_OF(self, struct expression_iface_t, invoke);

    struct integer_literal_t *il =
	CONTAINER_OF(ile, struct integer_literal_t, expression);

    return il->location;    
}
    
const struct value_iface_t * st_integer_literal_expression_return_value(
    struct expression_iface_t *self)
{
    struct integer_literal_t *il =
	CONTAINER_OF(self, struct integer_literal_t, expression);

    return &(il->data.value);
}

void st_integer_literal_expression_destroy(
    struct expression_iface_t *self)
{
    /* TODO: literal destructor from expression */
}

void st_integer_literal_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: literal destructor from value */
}
