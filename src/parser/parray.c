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
#include <util/macros.h>

#include <utlist.h>


struct array_range_iface_t * st_add_sub_to_new_array_range(
    struct array_range_iface_t *array_ranges,
    struct subrange_iface_t *subrange,
    struct parser_t *parser)
{
    if(!array_ranges)
    {
	array_ranges = st_create_array_range(parser->config,
					     parser->errors);

	if(!array_ranges)
	{
	    goto error_free_resources;
	}
    }

    int extend_result = array_ranges->extend(array_ranges,
					     subrange,
					     parser->config,
					     parser->errors);

    if(extend_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
	
    return array_ranges;

error_free_resources:
    subrange->destroy(subrange);
    array_ranges->destroy(array_ranges);
    return NULL;
}

struct array_initializer_iface_t * st_append_initial_element(
    struct array_initializer_iface_t *initializer,
    struct value_iface_t *new_value,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    if(!initializer)
    {
	initializer = st_create_array_initializer(parser->config,
						  parser->errors);
	if(!initializer)
	{
	    goto error_free_resources;
	}
    }

    int extend_result = initializer->extend_by_value(initializer,
						     new_value,
						     location,
						     parser->config,
						     parser->errors);
    if(extend_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    return initializer;

error_free_resources:
    initializer->destroy(initializer);
    new_value->destroy(new_value);
    return NULL;
}

struct array_initializer_iface_t * st_append_initial_elements(
    struct array_initializer_iface_t *initializer,
    struct value_iface_t *multiplier,
    struct value_iface_t *new_value,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    if(!initializer)
    {
	initializer = st_create_array_initializer(parser->config,
						  parser->errors);
	if(!initializer)
	{
	    goto error_free_resources;
	}
    }

    int extend_result = initializer->extend_by_multiplied_value(initializer,
								multiplier,
								new_value,
								location,
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
    multiplier->destroy(multiplier);
    new_value->destroy(new_value);
    return NULL;
}

struct value_iface_t * st_new_array_init_value(
    struct array_initializer_iface_t *initializer,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return initializer->value(initializer);
}

struct array_index_iface_t * st_new_array_index(
    struct expression_iface_t *expression,
    const struct st_location_t *expression_location,
    struct parser_t *parser)
{
    struct array_index_iface_t * index =
	st_create_array_index(parser->config, parser->errors);
    
    if(!index)
    {
	goto error_free_resources;
    }

    int extend_result = index->extend(index,
				      expression,
				      expression_location,
				      parser->config,
				      parser->errors);

    if(extend_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    return index;
    
error_free_resources:
    if(index)
    {
	index->destroy(index);
    }
    expression->destroy(expression);
    return NULL;
}

struct array_index_iface_t * st_extend_array_index(
    struct array_index_iface_t *index,
    struct expression_iface_t *expression,
    const struct st_location_t *expression_location,
    struct parser_t *parser)
{
    int extend_result = index->extend(index,
				      expression,
				      expression_location,
				      parser->config,
				      parser->errors);

    if(extend_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    return index;
    
error_free_resources:
    index->destroy(index);
    expression->destroy(expression);
    return NULL;
}
