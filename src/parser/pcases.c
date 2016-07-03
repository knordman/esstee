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
#include <statements/case.h>
#include <statements/statements.h>
#include <elements/subrange_case.h>


struct case_list_element_t * st_append_case_value(
    struct case_list_element_t *case_list,
    struct value_iface_t *case_value,
    const struct st_location_t *case_value_location,
    struct parser_t *parser)
{
    struct case_list_element_t *list = st_extend_case_list(
	case_list,
	case_value,
	case_value_location,
	parser->config,
	parser->errors);

    if(!list)
    {
	st_destroy_case_list(case_list);
	case_value->destroy(case_value);
    }

    return list;
}

struct case_t * st_new_case(
    struct case_list_element_t *case_value_list,
    const struct st_location_t *location,
    struct invoke_iface_t *statements,
    struct parser_t *parser)
{
    struct case_t *c = st_create_case(case_value_list,
				      location,
				      statements,
				      parser->config,
				      parser->errors);

    if(!c)
    {
	st_destroy_case_list(case_value_list);
	st_destroy_statements(statements);
    }

    return c;
}

struct case_t * st_append_case(
    struct case_t *cases,
    struct case_t *new_case,
    struct parser_t *parser)
{
    struct case_t *extended_cases = st_extend_cases(cases,
						    new_case,
						    parser->config,
						    parser->errors);
    if(!extended_cases)
    {
	st_destroy_case(cases);
	st_destroy_case(new_case);
    }

    return extended_cases;
}

struct value_iface_t * st_new_subrange_case_value(
    struct subrange_t *subrange,
    struct parser_t *parser)
{
    struct value_iface_t *scv = st_create_subrange_case_selector(
	subrange,
	parser->config,
	parser->errors);

    if(scv)
    {
	st_destroy_subrange(subrange);
    }

    return scv;
}
