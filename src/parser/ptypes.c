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
#include <util/macros.h>
#include <linker/linker.h>

#include <utlist.h>

struct type_iface_t * st_append_type_declaration(
    struct type_iface_t *type_block,
    struct type_iface_t *type,
    struct parser_t *parser)
{
    DL_APPEND(type_block, type);
    return type_block;
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
    struct derived_type_t *dt = NULL;
    struct st_location_t *loc = NULL;
	
    ALLOC_OR_ERROR_JUMP(
	dt,
	struct derived_type_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	parser->errors,
	error_free_resources);

    dt->ancestor = parent_type;
    dt->parent = parent_type;
    dt->location = loc;
    dt->default_value = default_value;

    dt->type.class = DERIVED_TYPE;    
    dt->type.identifier = type_name;
    dt->type.location = st_derived_type_location;
    dt->type.create_value_of = st_derived_type_create_value_of;
    dt->type.reset_value_of = st_derived_type_reset_value_of;
    dt->type.can_hold = st_derived_type_can_hold;
    dt->type.compatible = st_derived_type_compatible;
    dt->type.destroy = st_derived_type_destroy;

    return &(dt->type);

error_free_resources:
    free(loc);
    return NULL;
}

struct type_iface_t * st_new_derived_type_by_name(
    char *type_name,
    char *parent_type_name,
    const struct st_location_t *parent_type_name_location,
    const struct st_location_t *location,
    struct value_iface_t *default_value,
    struct parser_t *parser)
{
    struct derived_type_t *dt = NULL;
    struct st_location_t *loc = NULL;
	
    ALLOC_OR_ERROR_JUMP(
	dt,
	struct derived_type_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	parser->errors,
	error_free_resources);

    int ref_add_result = parser->pou_type_ref_pool->add_two_step(
	parser->pou_type_ref_pool,
	parent_type_name,
	dt,
	NULL,
	parent_type_name_location,
	st_derived_type_parent_name_resolved,
	st_derived_type_resolve_ancestor);

    if(ref_add_result != ESSTEE_OK)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	goto error_free_resources;
    }
    
    dt->ancestor = NULL;
    dt->parent = NULL;
    dt->location = loc;
    dt->default_value = default_value;

    dt->type.class = DERIVED_TYPE;
    dt->type.identifier = type_name;
    dt->type.location = st_derived_type_location;
    dt->type.create_value_of = st_derived_type_create_value_of;
    dt->type.reset_value_of = st_derived_type_reset_value_of;
    dt->type.can_hold = st_derived_type_can_hold;
    dt->type.compatible = st_derived_type_compatible;
    dt->type.destroy = st_derived_type_destroy;
    
    return &(dt->type);

error_free_resources:
    free(loc);
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
    struct enum_item_t *enum_value_group,
    char *identifier,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct enum_item_t *ei = NULL;
    struct st_location_t *loc = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	ei,
	struct enum_item_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	parser->errors,
	error_free_resources);

    ei->identifier = identifier;
    ei->location = loc;
    if(!enum_value_group)
    {
	ei->group = ei;
    }
    else
    {
	ei->group = enum_value_group;
    }

    struct enum_item_t *found = NULL;
    HASH_FIND_STR(enum_value_group, ei->identifier, found);
    if(found)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "non unique enumeration",
	    ISSUE_ERROR_CLASS,
	    1,
	    ei->location);
	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	
	goto error_free_resources;
    }
    
    HASH_ADD_KEYPTR(hh, 
		    enum_value_group, 
		    ei->identifier, 
		    strlen(ei->identifier), 
		    ei);

    return enum_value_group;
    
error_free_resources:
    free(ei);
    free(loc);
    return NULL;
}

struct type_iface_t * st_new_enum_type(
    struct enum_item_t *value_group, 
    const struct st_location_t *location,
    char *initial_value_identifier, 
    const struct st_location_t *initial_value_location,
    struct parser_t *parser)
{
    struct enum_type_t *et = NULL;
    struct st_location_t *loc = NULL;
    ALLOC_OR_ERROR_JUMP(
	et,
	struct enum_type_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	parser->errors,
	error_free_resources);

    struct enum_item_t *initial_value = value_group;
    if(initial_value_identifier != NULL)
    {
	HASH_FIND_STR(value_group, initial_value_identifier, initial_value);
	if(initial_value == NULL)
	{
	    parser->errors->new_issue_at(
		parser->errors,
		"enumeration value not found",
		ISSUE_ERROR_CLASS,
		1,
		initial_value_location);

	    parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	    goto error_free_resources;
	}
    }

    et->default_item = initial_value;
    et->values = value_group;
    et->location = loc;
    
    et->type.class = ENUM_TYPE;
    et->type.location = st_enum_type_location;
    et->type.create_value_of = st_enum_type_create_value_of;
    et->type.reset_value_of = st_enum_type_reset_value_of;
    et->type.can_hold = st_enum_type_can_hold;
    et->type.compatible = st_type_general_compatible;
    et->type.destroy = st_enum_type_destroy;

    return &(et->type);
    
error_free_resources:
    free(loc);
    free(et);
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
