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
#include <elements/struct.h>


struct struct_element_t * st_add_new_struct_element(
    struct struct_element_t *element_group,
    char *element_identifier,
    const struct st_location_t *identifier_location,
    struct type_iface_t *element_type,
    struct parser_t *parser)
{
    struct struct_element_t *se = st_extend_element_group(
	element_group,
	element_identifier,
	identifier_location,
	element_type,
	parser->config,
	parser->errors);

    if(!se)
    {
	st_destroy_struct_element_group(element_group);
	free(element_identifier);
	element_type->destroy(element_type);
    }

    return se;
}

struct struct_element_t * st_add_new_struct_element_by_name(
    struct struct_element_t *element_group,
    char *element_identifier,
    const struct st_location_t *identifier_location,
    char *element_type_name,
    const struct st_location_t *element_type_name_location,
    struct parser_t *parser)
{
    struct struct_element_t *se = st_extend_element_group_type_name(
	element_group,
	element_identifier,
	identifier_location,
	element_type_name,
	element_type_name_location,
	parser->pou_type_ref_pool,
	parser->config,
	parser->errors);

    if(!se)
    {
	st_destroy_struct_element_group(element_group);
	free(element_identifier);
	free(element_type_name);
    }

    return se;
}

struct struct_element_init_t * st_new_struct_element_initializer(
    char *element_identifier,
    const struct st_location_t *identifier_location,
    struct value_iface_t *value,
    struct parser_t *parser)
{
    struct struct_element_init_t *ei =
	st_create_element_initializer(
	    element_identifier,
	    identifier_location,
	    value,
	    parser->config,
	    parser->errors);

    if(!ei)
    {
	free(element_identifier);
	value->destroy(value);
    }

    return ei;
}

struct struct_element_init_t * st_add_initial_struct_element(
    struct struct_element_init_t *initializer_group,
    struct struct_element_init_t *initializer,
    struct parser_t *parser)
{
    struct struct_element_init_t *sei = 
	st_extend_element_initializer_group(
	    initializer_group,
	    initializer,
	    parser->config,
	    parser->errors);

    if(!sei)
    {
	st_destroy_initializer_group(initializer_group);
	st_destroy_element_initializer(initializer);
    }

    return sei;
}

struct value_iface_t * st_new_struct_init_value(
    struct struct_element_init_t *initializer_group,
    struct parser_t *parser)
{
    struct value_iface_t *iv = st_create_struct_initializer_value(
	initializer_group,
	parser->config,
	parser->errors);

    if(!iv)
    {
	st_destroy_initializer_group(initializer_group);
    }

    return iv;
}
