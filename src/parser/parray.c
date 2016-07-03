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


struct array_range_t * st_add_sub_to_new_array_range(
    struct array_range_t *array_ranges,
    struct subrange_t *subrange,
    struct parser_t *parser)
{
    struct array_range_t *ar =
        st_extend_array_range(
	    array_ranges,
	    subrange,
	    parser->config,
	    parser->errors);

    if(!ar)
    {
	st_destroy_array_ranges(array_ranges);
	st_destroy_subrange(subrange);
    }

    return ar;
}

struct listed_value_t * st_append_initial_element(
    struct listed_value_t *values,
    struct value_iface_t *new_value,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct listed_value_t *lv =
	st_extend_array_initializer(values,
				    NULL,
				    new_value,
				    location,
				    parser->config,
				    parser->errors);
    if(!lv)
    {
	st_destroy_listed_values(values);
	new_value->destroy(new_value);
    }

    return lv;
}

struct listed_value_t * st_append_initial_elements(
    struct listed_value_t *values,
    struct value_iface_t *multiplier,
    struct value_iface_t *new_value,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct listed_value_t *lv =
	st_extend_array_initializer(values,
				    multiplier,
				    new_value,
				    location,
				    parser->config,
				    parser->errors);
    if(!lv)
    {
	st_destroy_listed_values(values);
	multiplier->destroy(multiplier);
	new_value->destroy(new_value);
    }

    return lv;
}

struct value_iface_t * st_new_array_init_value(
    struct listed_value_t *values,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct value_iface_t *ai =
	st_create_array_init_value(
	    values,
	    location,
	    parser->config,
	    parser->errors);

    if(!ai)
    {
	st_destroy_listed_values(values);
    }

    return ai;
}

struct array_index_t * st_append_new_array_index(
    struct array_index_t *index_list,
    struct expression_iface_t *index_expression,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct array_index_t *ai = NULL;
    struct st_location_t *ai_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	ai,
	struct array_index_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	ai_location,
	location,
	parser->errors,
	error_free_resources);

    ai->location = ai_location;
    ai->index_expression = index_expression;

    DL_APPEND(index_list, ai);

    return index_list;
    
error_free_resources:
    free(ai);
    /* TODO: destroy array indices */
    return NULL;
}
