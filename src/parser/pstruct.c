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


struct struct_elements_iface_t * st_add_new_struct_element(
    struct struct_elements_iface_t *elements,
    char *identifier,
    const struct st_location_t *identifier_location,
    struct type_iface_t *type,
    struct parser_t *parser)
{
    if(!elements)
    {
	elements = st_create_struct_elements(parser->config,
					     parser->errors);

	if(!elements)
	{
	    goto error_free_resources;
	}
    }

    int extend_result = elements->extend_by_type(elements,
						 identifier,
						 identifier_location,
						 type,
						 parser->config,
						 parser->errors);

    if(extend_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    return elements;

error_free_resources:
    if(elements)
    {
	elements->destroy(elements);
    }
    type->destroy(type);
    free(identifier);
    return NULL;
}

struct struct_elements_iface_t * st_add_new_struct_element_by_name(
    struct struct_elements_iface_t *elements,
    char *identifier,
    const struct st_location_t *identifier_location,
    char *type_name,
    const struct st_location_t *type_name_location,
    struct parser_t *parser)
{
    if(!elements)
    {
	elements = st_create_struct_elements(parser->config,
					     parser->errors);

	if(!elements)
	{
	    goto error_free_resources;
	}
    }

    int extend_result = elements->extend_by_type_name(elements,
						      identifier,
						      identifier_location,
						      type_name,
						      type_name_location,
						      parser->pou_type_ref_pool,
						      parser->config,
						      parser->errors);

    if(extend_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    return elements;

error_free_resources:
    if(elements)
    {
	elements->destroy(elements);
    }
    free(type_name);
    free(identifier);
    return NULL;
}

struct value_iface_t * st_struct_initializer_value(
    struct struct_initializer_iface_t *initializer,
    struct parser_t *parser)
{
    return initializer->value(initializer);
}

struct struct_initializer_iface_t * st_add_new_struct_element_initializer(
    struct struct_initializer_iface_t *initializer,
    char *identifier,
    const struct st_location_t *identifier_location,
    struct value_iface_t *value,
    struct parser_t *parser)
{
    if(!initializer)
    {
	initializer = st_create_struct_initializer(parser->config,
						   parser->errors);

	if(!initializer)
	{
	    goto error_free_resources;
	}
    }

    int extend_result = initializer->extend(initializer,
					    identifier,
					    identifier_location,
					    value,
					    parser->config,
					    parser->errors);

    if(extend_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    return initializer;

error_free_resources:
    if(initializer)
    {
	initializer->destroy(initializer);
    }
    value->destroy(value);
    free(identifier);
    return NULL;
}
