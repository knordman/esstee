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
struct value_iface_t * st_new_enum_inline_value(
    char *identifier,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct inline_enum_value_t *ie = NULL;
    struct st_location_t *loc = NULL;
    ALLOC_OR_ERROR_JUMP(
	ie,
	struct inline_enum_value_t,
	parser->errors,
	error_free_resources);
    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	parser->errors,
	error_free_resources);

    ie->data.identifier = identifier;
    ie->data.location = loc;
    ie->data.group = NULL;

    memset(&(ie->value), 0, sizeof(struct value_iface_t));

    ie->value.comparable_to = st_inline_enum_value_comparable_to;
    ie->value.equals = st_inline_enum_value_equals;
    ie->value.enumeration = st_inline_enum_value_enumeration;
    ie->value.destroy = st_inline_enum_value_destroy;
    
    return &(ie->value);
	
error_free_resources:
    free(ie);
    free(loc);
    return NULL;
}

struct value_iface_t * st_new_subrange_case_value(
    struct subrange_t *subrange,
    struct parser_t *parser)
{
    struct subrange_case_value_t *sv = NULL;
    ALLOC_OR_ERROR_JUMP(
	sv,
	struct subrange_case_value_t,
	parser->errors,
	error_free_resources);

    sv->subrange = subrange;

    memset(&(sv->value), 0, sizeof(struct value_iface_t));

    sv->value.comparable_to = st_subrange_case_value_comparable_to;
    sv->value.equals = st_subrange_case_value_equals;
    sv->value.destroy = st_subrange_case_value_destroy;

    return &(sv->value);

error_free_resources:
    return NULL;
}
