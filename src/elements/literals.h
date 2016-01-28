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

#pragma once

#include <elements/iexpression.h>
#include <elements/values.h>

/**************************************************************************/
/* Integer literals                                                       */
/**************************************************************************/
struct integer_literal_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct integer_value_t data;
};

const struct st_location_t * st_integer_literal_location(
    const struct invoke_iface_t *self);
    
const struct value_iface_t * st_integer_literal_expression_return_value(
    struct expression_iface_t *self);

void st_integer_literal_expression_destroy(
    struct expression_iface_t *self);

void st_integer_literal_value_destroy(
    struct value_iface_t *self);

/**************************************************************************/
/* Real literals                                                          */
/**************************************************************************/
struct real_literal_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct real_value_t data;
};

/**************************************************************************/
/* String literals                                                        */
/**************************************************************************/
struct string_literal_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct string_value_t data;
};

/**************************************************************************/
/* Duration literal                                                       */
/**************************************************************************/
struct duration_literal_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct duration_value_t data;
};

/**************************************************************************/
/* Date literal                                                           */
/**************************************************************************/
struct date_literal_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct date_value_t data;
};

/**************************************************************************/
/* Tod literal                                                            */
/**************************************************************************/
struct tod_literal_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct tod_value_t data;
};

/**************************************************************************/
/* Date tod literal                                                       */
/**************************************************************************/
struct date_tod_literal_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct date_tod_value_t data;
};
