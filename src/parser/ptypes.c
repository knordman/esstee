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
#include <elements/array.h>
#include <elements/derived.h>
#include <elements/subrange.h>
#include <elements/struct.h>
#include <elements/strings.h>
#include <elements/enums.h>

#include <utlist.h>


struct type_iface_t * st_append_type_declaration(
    struct type_iface_t *type_block,
    struct type_iface_t *type,
    struct parser_t *parser)
{
    DL_APPEND(type_block, type);
    return type_block;
}

struct type_iface_t * st_new_derived_type(
    char *type_name,
    struct type_iface_t *parent_type,
    const struct st_location_t *location,
    struct value_iface_t *default_value,
    const struct st_location_t *default_value_location,
    struct parser_t *parser)
{
    struct type_iface_t *dt =
	st_create_derived_type(
	    type_name,
	    parent_type,
	    location,
	    default_value,
	    default_value_location,
	    parser->pou_type_ref_pool,
	    parser->config,
	    parser->errors);

    if(!dt)
    {
	free(type_name);
	parent_type->destroy(parent_type);
	if(default_value)
	{
	    default_value->destroy(default_value);
	}
    }

    return dt;
}

struct type_iface_t * st_new_derived_type_by_name(
    char *type_name,
    char *parent_type_name,
    const struct st_location_t *parent_type_name_location,
    const struct st_location_t *location,
    struct value_iface_t *default_value,
    const struct st_location_t *default_value_location,
    struct parser_t *parser)
{
    struct type_iface_t *dt =
	st_create_derived_type_by_name(
	    type_name,
	    parent_type_name,
	    parent_type_name_location,
	    location,
	    default_value,
	    default_value_location,
	    parser->pou_type_ref_pool,
	    parser->config,
	    parser->errors);

    if(!dt)
    {
	free(type_name);
	free(parent_type_name);
	if(default_value)
	{
	    default_value->destroy(default_value);
	}
    }

    return dt;
}

struct type_iface_t * st_new_subrange_type(
    char *storage_type_identifier,
    const struct st_location_t *storage_type_identifier_location,
    struct subrange_t *subrange,
    struct value_iface_t *default_value,
    const struct st_location_t *default_value_location,
    struct parser_t *parser)
{
    struct type_iface_t *st =
	st_create_subrange_type(
	    storage_type_identifier,
	    storage_type_identifier_location,
	    subrange,
	    default_value,
	    default_value_location,
	    parser->pou_type_ref_pool,
	    parser->config,
	    parser->errors);

    if(!st)
    {
	free(storage_type_identifier);
	st_destroy_subrange(subrange);
	if(default_value)
	{
	    default_value->destroy(default_value);
	}
    }

    return st;
}

struct type_iface_t * st_new_enum_type(
    struct enum_group_item_t *value_group, 
    char *default_item, 
    const struct st_location_t *default_item_location,
    struct parser_t *parser)
{
    struct type_iface_t *et = st_create_enum_type(
	value_group,
	default_item,
	default_item_location,
	parser->config,
	parser->errors);

    if(!et)
    {
	st_destroy_enum_group(value_group);
    }

    free(default_item);
    
    return et;
}

struct type_iface_t * st_new_array_type(
    struct array_range_t *array_ranges,
    char *arrayed_type_identifier,
    const struct st_location_t *arrayed_type_identifier_location,
    struct value_iface_t *default_value,
    struct parser_t *parser)
{
    struct type_iface_t *at = st_create_array_type(
	array_ranges,
	arrayed_type_identifier,
	arrayed_type_identifier_location,
	default_value,
	parser->pou_type_ref_pool,
	parser->config,
	parser->errors);

    if(!at)
    {
	st_destroy_array_ranges(array_ranges);
	if(default_value)
	{
	    default_value->destroy(default_value);
	}
    }

    return at;
}

struct type_iface_t * st_new_struct_type(
    struct struct_element_t *element_group,
    struct parser_t *parser)
{
    struct type_iface_t *st = st_create_struct_type(element_group,
						    parser->config,
						    parser->errors);
    if(!st)
    {
	st_destroy_struct_element_group(element_group);
    }

    return st;
}

struct type_iface_t * st_new_string_type(
    char *string_type_identifier,
    struct value_iface_t *length,
    const struct st_location_t *length_location,
    struct value_iface_t *default_value,
    struct parser_t *parser)
{
    struct type_iface_t *ns = st_new_custom_length_string_type(
	string_type_identifier,
	length,
	length_location,
	default_value,
	parser->config,
	parser->errors);

    if(!ns)
    {
	length->destroy(length);
	default_value->destroy(default_value);
    }
	
    return ns;
}
