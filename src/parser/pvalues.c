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

#include <parser/parser.h>
#include <util/macros.h>

/**************************************************************************/
/* Inline values                                                          */
/**************************************************************************/
struct value_iface_t * st_new_array_init_value(
    struct value_iface_t *values,
    struct parser_t *parser)
{
    /* TODO: move to types, change name from value, not really a value */
    return NULL;
}

struct value_iface_t * st_new_struct_init_value(
    struct struct_element_init_t *element_group,
    struct parser_t *parser)
{
    /* TODO: move to types, change name from value, not really a value */
    return NULL;
}

struct value_iface_t * st_new_enum_inline_value(
    char *identifiepr,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    /* TODO: inline enum value, determine where use, perhaps not a value either */
    return NULL;
}

struct value_iface_t * st_new_subrange_case_value(
    struct subrange_t *subrange,
    struct parser_t *parser)
{
    /* TODO: only used for case structure, not a value */
    return NULL;
}

struct value_iface_t * st_new_bool_temporary_value(    
    struct errors_iface_t *errors)
{
    /* TODO: temporary boolean, can be defined somewhere else */
    return NULL;
}
