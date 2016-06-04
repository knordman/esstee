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
#include <expressions/inline_expression.h>
#include <expressions/binary_expressions.h>
#include <expressions/direct_address_term.h>
#include <expressions/function_invocation.h>
#include <expressions/identifier.h>
#include <expressions/negative_prefix.h>
#include <expressions/qualified_identifier_term.h>

#include <utlist.h>

/**************************************************************************/
/* Wrapped value expression (e.g. literal)                                */
/**************************************************************************/
struct expression_iface_t * st_new_expression_value(
    struct value_iface_t *value,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *ev =
	st_create_value_expression(
	    value,
	    location,
	    parser->config,
	    parser->errors);

    if(!ev)
    {
	value->destroy(value);
    }

    return ev;
}

/**************************************************************************/
/* Single identifier term, either an enum, or a variable reference        */
/**************************************************************************/
struct expression_iface_t * st_new_single_identifier_term(
    char *identifier,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_identifier_term(
	identifier,
	location,
	parser->pou_var_ref_pool,
	parser->config,
	parser->errors);

    if(!expr)
    {
	free(identifier);
    }

    return expr;
}

/**************************************************************************/
/* Qualified identifier term                                              */
/**************************************************************************/
struct expression_iface_t * st_new_qualified_identifier_term(
    struct qualified_identifier_iface_t *qualified_identifier,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_qualified_identifier_term(
	qualified_identifier,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	qualified_identifier->destroy(qualified_identifier);
    }

    return expr;
}

/**************************************************************************/
/* Direct address term                                                    */
/**************************************************************************/
struct expression_iface_t * st_new_direct_address_term(
    struct direct_address_t *address,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_direct_address_term(
	address,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	free(address);
    }

    return expr;
}

/**************************************************************************/
/* Function invocation term                                               */
/**************************************************************************/
struct expression_iface_t * st_new_function_invocation_term(
    char *function_identifier,
    const struct st_location_t *location,
    struct invoke_parameters_iface_t *invoke_parameters,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_function_invocation_term(
	function_identifier,
	location,
	parser->function_ref_pool,
	invoke_parameters,
	parser->config,
	parser->errors);

    if(!expr)
    {
	free(function_identifier);
	invoke_parameters->destroy(invoke_parameters);
    }

    return expr;
}

/**************************************************************************/
/* Negative prefix term                                                   */
/**************************************************************************/
struct expression_iface_t * st_new_negate_term(
    struct expression_iface_t *term,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_negative_prefix_term(
	term,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	term->destroy(term);
    }

    return expr;
}

/**************************************************************************/
/* Not prefix term                                                        */
/**************************************************************************/
struct expression_iface_t * st_new_not_term(
    struct expression_iface_t *term,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_not_prefix_term(
	term,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	term->destroy(term);
    }

    return expr;
}

/**************************************************************************/
/* Binary expressions                                                     */
/**************************************************************************/
struct expression_iface_t * st_new_xor_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_xor_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_and_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_and_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_or_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_or_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_greater_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_greater_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_lesser_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_lesser_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_equals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_equals_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_gequals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_gequals_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_lequals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_lequals_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_nequals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_nequals_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_plus_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_plus_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_minus_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_minus_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_multiply_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_multiply_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_division_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_division_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_mod_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_mod_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}

struct expression_iface_t * st_new_to_power_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct expression_iface_t *expr = st_create_to_power_expression(
	left_operand,
	right_operand,
	location,
	parser->config,
	parser->errors);

    if(!expr)
    {
	left_operand->destroy(left_operand);
	right_operand->destroy(right_operand);
    }

    return expr;
}
