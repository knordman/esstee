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
#include <elements/types.h>


struct type_iface_t * st_append_type_declaration(
    struct type_iface_t *type_block,
    struct type_iface_t *type,
    struct parser_t *parser)
{
    /* TODO: append new type declaration to type block */
    return NULL;
}

/**************************************************************************/
/* Derived type                                                           */
/**************************************************************************/
struct type_iface_t * st_new_derived_type(
    char *type_name,
    struct type_iface_t *parent_type,
    const struct st_location_t *location,
    struct value_iface_t *default_value,
    struct parser_t *parser)
{
    /* TODO: new derived type by inline defined type */
    return NULL;
}

struct type_iface_t * st_new_derived_type_by_name(
    char *type_name,
    char *parent_type_name,
    const struct st_location_t *location,
    struct value_iface_t *default_value,
    struct parser_t *parser)
{
    /* TODO: new derived type from other type name */
    return NULL;
}

/**************************************************************************/
/* Subrange type                                                          */
/**************************************************************************/
struct subrange_t * st_new_subrange(
    struct expression_iface_t *min, 
    struct expression_iface_t *max, 
    const struct st_location_t *location,
    struct parser_t *parser)
{
    /* TODO: new subrange */
    return NULL;
}

struct type_iface_t * st_new_subrange_type(
    char *storage_type_identifier,
    const struct st_location_t *storage_type_identifier_location,
    struct subrange_t *subrange,
    struct expression_iface_t *initial_value_literal,
    struct parser_t *parser)
{
    /* TODO: new subrange type */
    return NULL;
}

/**************************************************************************/
/* Enum type                                                              */
/**************************************************************************/
struct enum_item_t * st_append_new_enum_item(
    struct enum_item_t *enumerated_value_group,
    char *identifier,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    /* TODO: add new enum item to enum item group */
    return NULL;
}

struct type_iface_t * st_new_enum_type(
    struct enum_item_t *value_group, 
    char *initial_value_identifier, 
    const struct st_location_t *initial_value_location,
    struct parser_t *parser)
{
    /* TODO: new enum type */
    return NULL;
}

/**************************************************************************/
/* Array type                                                             */
/**************************************************************************/
struct array_range_t * st_add_sub_to_new_array_range(
    struct array_range_t *array_ranges,
    struct subrange_t *subrange,
    struct parser_t *parser)
{
    /* TODO: extend array range by subrange */
    return NULL;
}

struct value_iface_t * st_append_initial_element(
    struct value_iface_t *values,
    struct value_iface_t *new_value,
    struct parser_t *parser)
{
    /* TODO: append new initial element to array */
    return NULL;
}

struct value_iface_t * st_append_initial_elements(
    struct value_iface_t *values,
    struct expression_iface_t *multiplier,
    struct value_iface_t *new_value,
    struct parser_t *parser)
{
    /* TODO: append initial elements to array by multiplier */
    return NULL;
}

struct type_iface_t * st_new_array_type(
    struct array_range_t *array_ranges,
    char *arrayed_type_identifier,
    const struct st_location_t *arrayed_type_identifier_location,
    struct value_iface_t *initial_value,
    struct parser_t *parser)
{
    /* TODO: new array type */
    return NULL;
}

/**************************************************************************/
/* Struct type                                                            */
/**************************************************************************/
struct struct_element_t * st_add_new_struct_element(
    struct struct_element_t *element_group,
    char *element_identifier,
    const struct st_location_t *identifier_location,
    struct type_iface_t *element_type,
    struct parser_t *parser)
{
    /* TODO: add new struct element to struct element group */
    return NULL;
}

struct type_iface_t * st_new_struct_type(
    struct struct_element_t *elements,
    struct parser_t *parser)
{
    /* TODO: new struct type */
    return NULL;
}

struct struct_element_init_t * st_add_initial_struct_element(
    struct struct_element_init_t *element_group,
    struct struct_element_init_t *new_element,
    struct parser_t *parser)
{
    /* TODO: add new struct element initializer to initializer group */
    return NULL;
}

struct struct_element_init_t * st_new_struct_element_initializer(
    char *element_identifier,
    const struct st_location_t *identifier_location,
    struct value_iface_t *value,
    struct parser_t *parser)
{
    /* TODO: new struct element initializer from value */
    return NULL;
}

/**************************************************************************/
/* String type with defined length                                        */
/**************************************************************************/
struct type_iface_t * st_new_string_type(
    char *string_type_identifier,
    struct expression_iface_t *length,
    struct expression_iface_t *default_value,
    struct parser_t *parser)
{
    /* TODO: new string type with defined value */
    return NULL;
}