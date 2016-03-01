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
#include <linker/linker.h>

#include <utlist.h>

/**************************************************************************/
/* Wrapped value expression (e.g. literal)                                */
/**************************************************************************/
struct expression_iface_t * st_new_expression_value(
    struct value_iface_t *value,
    const struct st_location_t *value_location,
    struct parser_t *parser)
{
    struct value_expression_t *ve = NULL;
    struct st_location_t *loc = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	ve,
	struct value_expression_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc,
	value_location,
	parser->errors,
	error_free_resources);

    ve->location = loc;
    ve->value = value;

    memset(&(ve->expression), 0, sizeof(struct expression_iface_t));
    
    ve->expression.invoke.location = st_value_expression_location;
    ve->expression.invoke.step = NULL;
    ve->expression.invoke.verify = NULL;
    ve->expression.runtime_constant = st_value_expression_runtime_constant;
    
    ve->expression.return_value = st_value_expression_return_value;
    ve->expression.destroy = st_value_expression_destroy;

    return &(ve->expression);
    
error_free_resources:
    free(ve);
    return NULL;
}

/**************************************************************************/
/* Single identifier term, either an enum, or a variable reference        */
/**************************************************************************/
struct expression_iface_t * st_new_single_identifier_term(
    char *identifier,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct single_identifier_term_t *sit = NULL;
    ALLOC_OR_ERROR_JUMP(
	sit,
	struct single_identifier_term_t,
	parser->errors,
	error_free_resources);

    struct st_location_t *sitl = NULL;
    LOCDUP_OR_ERROR_JUMP(
	sitl,
	location,
	parser->errors,
	error_free_resources);

    int var_ref_add_result = parser->pou_var_ref_pool->add(
	parser->pou_var_ref_pool,
	identifier,
	sit,
	NULL,
	location,
	st_single_identifier_variable_resolved);

    if(var_ref_add_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    sit->variable = NULL;
    sit->location = sitl;
    sit->identifier = identifier;
    memset(&(sit->value), 0, sizeof(struct value_iface_t));
        
    sit->expression.invoke.verify = NULL;
    sit->expression.invoke.step = NULL;
    sit->expression.invoke.location = st_single_identifier_term_location;

    sit->expression.return_value = NULL;
    sit->expression.destroy = st_single_identifier_term_destroy;
    sit->expression.clone = st_single_identifier_term_clone;

    return &(sit->expression);
    
error_free_resources:
    free(sit);
    free(sitl);
    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
    return NULL;
}

/**************************************************************************/
/* Qualified identifier term                                              */
/**************************************************************************/
struct expression_iface_t * st_new_qualified_identifier_term(
    struct qualified_identifier_t *qualified_identifier,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct qualified_identifier_term_t *qt = NULL;
    struct st_location_t *loc = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	qt,
	struct qualified_identifier_term_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	parser->errors,
	error_free_resources);

    qt->location = loc;
    qt->identifier = qualified_identifier;

    qt->expression.invoke.verify = st_qualified_identifier_term_verify;
    qt->expression.invoke.step = st_qualified_identifier_term_step;
    qt->expression.invoke.reset = st_qualified_identifier_term_reset;

    qt->invoke_state = 0;
    qt->expression.invoke.location = st_qualified_identifier_term_location;
    qt->expression.return_value = st_qualified_identifier_term_return_value;
    qt->expression.destroy = st_qualified_identifier_term_destroy;
    qt->expression.clone = st_qualified_identifier_term_clone;

    return &(qt->expression);
    
error_free_resources:
    free(qt);
    free(loc);
    st_destroy_qualified_identifier(qualified_identifier);    
    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
    return NULL;
}

/**************************************************************************/
/* Direct address term                                                    */
/**************************************************************************/
struct expression_iface_t * st_new_direct_address_term(
    struct direct_address_t *direct_address,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return NULL;
}

/**************************************************************************/
/* Function invocation term                                               */
/**************************************************************************/
struct expression_iface_t * st_new_function_invocation_term(
    char *function_identifier,
    const struct st_location_t *location,
    struct invoke_parameter_t *invoke_parameters,
    struct parser_t *parser)
{
    return NULL;
}

/**************************************************************************/
/* Negative prefix term                                                   */
/**************************************************************************/
struct expression_iface_t * st_new_negate_term(
    struct expression_iface_t *term,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return NULL;
}

/**************************************************************************/
/* Not prefix term                                                        */
/**************************************************************************/
struct expression_iface_t * st_new_not_term(
    struct expression_iface_t *term,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return NULL;
}

/**************************************************************************/
/* Binary expressions                                                     */
/**************************************************************************/
static struct expression_iface_t * new_binary_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    int (*verify_function)(
	struct invoke_iface_t *self,
	const struct config_iface_t *config,
	struct errors_iface_t *errors),
    const struct st_location_t *location,
    int (*step_function)(
	struct invoke_iface_t *,
	struct cursor_t *,
	const struct systime_iface_t *,
	const struct config_iface_t *,
	struct errors_iface_t *),
    struct parser_t *parser)
{
    struct binary_expression_t *be = NULL;
    ALLOC_OR_ERROR_JUMP(
	be,
	struct binary_expression_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
    	be->location,
    	location,
    	parser->errors,
    	error_free_resources);

    be->left_operand = left_operand;
    be->right_operand = right_operand;
    be->invoke_state = 0;
    
    be->expression.invoke.step = step_function;
    be->expression.invoke.verify = verify_function;
    be->expression.invoke.location = st_binary_expression_location;
    be->expression.invoke.reset = st_binary_expression_reset;
    be->expression.return_value = st_binary_expression_return_value;
    be->expression.runtime_constant = st_binary_expression_runtime_constant;
    be->expression.clone = st_binary_expression_clone;
    
    be->temporary = NULL;
    
    return &(be->expression);
    
error_free_resources:
    free(be);
    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
    return NULL;
}

struct expression_iface_t * st_new_xor_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_xor_expression_verify,
	location,
	st_xor_expression_step,
	parser);
}

struct expression_iface_t * st_new_and_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_and_expression_verify,       
	location,
	st_and_expression_step,
	parser);
}

struct expression_iface_t * st_new_or_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_or_expression_verify,	
	location,
	st_or_expression_step,
	parser);
}

struct expression_iface_t * st_new_greater_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_greater_expression_verify,	
	location,
	st_greater_expression_step,
	parser);
}

struct expression_iface_t * st_new_lesser_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_lesser_expression_verify,	
	location,
	st_lesser_expression_step,
	parser);
}

struct expression_iface_t * st_new_equals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_equals_expression_verify,	
	location,
	st_equals_expression_step,
	parser);
}

struct expression_iface_t * st_new_gequals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_gequals_expression_verify,	
	location,
	st_gequals_expression_step,
	parser);
}

struct expression_iface_t * st_new_lequals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_lequals_expression_verify,	
	location,
	st_lequals_expression_step,
	parser);
}

struct expression_iface_t * st_new_nequals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_nequals_expression_verify,	
	location,
	st_nequals_expression_step,
	parser);
}

struct expression_iface_t * st_new_plus_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_plus_expression_verify,	
	location,
	st_plus_expression_step,
	parser);
}

struct expression_iface_t * st_new_minus_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_minus_expression_verify,	
	location,
	st_minus_expression_step,
	parser);
}

struct expression_iface_t * st_new_multiply_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_multiply_expression_verify,	
	location,
	st_multiply_expression_step,
	parser);
}

struct expression_iface_t * st_new_division_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_division_expression_verify,	
	location,
	st_division_expression_step,
	parser);
}

struct expression_iface_t * st_new_mod_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_mod_expression_verify,	
	location,
	st_mod_expression_step,
	parser);
}

struct expression_iface_t * st_new_to_power_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	st_power_expression_verify,	
	location,
	st_power_expression_step,
	parser);
}
