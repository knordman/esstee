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
#include <elements/ivalue.h>
#include <elements/variables.h>
#include <elements/pous.h>

/**************************************************************************/
/* Array index                                                            */
/**************************************************************************/
struct array_index_t {
    struct expression_iface_t *index_expression;
    struct st_location_t *location;
    struct array_index_t *prev;
    struct array_index_t *next;
};

void st_destroy_array_index(
    struct array_index_t *ai);

/**************************************************************************/
/* Qualified identifier                                                   */
/**************************************************************************/
struct qualified_identifier_t {
    char *identifier;
    struct array_index_t *array_index;
    struct variable_t *variable;
    struct value_iface_t *target;
    struct program_t *program;
    struct st_location_t *location;
    struct qualified_identifier_t *prev;
    struct qualified_identifier_t *next;
};

int st_inner_resolve_qualified_identifier(
    struct qualified_identifier_t *qi,
    struct errors_iface_t *errors);

void st_destroy_qualified_identifier(
    struct qualified_identifier_t *qi);

/**************************************************************************/
/* Invoke parameters                                                      */
/**************************************************************************/
struct invoke_parameter_t {
    char *identifier;
    struct expression_iface_t *expression;
    struct st_location_t *location;
    struct invoke_parameter_t *prev;
    struct invoke_parameter_t *next;
};
