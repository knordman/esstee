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
    ve->expression.invoke.reset = NULL;
    ve->expression.invoke.clone = NULL;
    
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
	location,
	st_single_identifier_variable_resolved,
	parser->errors);

    if(var_ref_add_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    sit->variable = NULL;
    sit->location = sitl;
    sit->inline_enum.data.identifier = identifier;
        
    sit->expression.invoke.verify = NULL;
    sit->expression.invoke.step = NULL;
    sit->expression.invoke.location = st_single_identifier_term_location;
    sit->expression.invoke.allocate = NULL;

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
    qt->expression.invoke.allocate = NULL;
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
    struct direct_address_t *address,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct direct_address_term_t *dt = NULL;
    struct st_location_t *term_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	dt,
	struct direct_address_term_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	term_location,
	location,
	parser->errors,
	error_free_resources);

    dt->expression.invoke.location = st_direct_address_term_location;
    dt->expression.invoke.verify = NULL;
    dt->expression.invoke.step = NULL;
    dt->expression.invoke.reset = NULL;
    dt->expression.invoke.allocate = NULL;
    dt->expression.invoke.clone = NULL;
    dt->expression.invoke.destroy = NULL;

    dt->expression.return_value = st_direct_address_term_return_value;
    dt->expression.clone = NULL;
    dt->expression.destroy = st_direct_address_term_destroy;

    memset(&(dt->content.value), 0, sizeof(struct value_iface_t));

    dt->content.value.comparable_to = st_direct_address_term_value_comparable_to;
    dt->content.value.create_temp_from = st_direct_address_term_value_create_temp_from; 
    dt->content.value.equals = st_direct_address_term_value_equals;
	
    if(address->field_size_bits > 1)
    {
	/* Integer */
	dt->content.value.operates_with = st_direct_address_term_value_operates_with;
	dt->content.value.greater = st_direct_address_term_value_greater;
	dt->content.value.lesser = st_direct_address_term_value_lesser;

	dt->content.value.integer = st_direct_address_term_value_integer;
    }
    else
    {
	/* Default to bit size if no size is given = boolean */
	address->field_size_bits = 1;
	ST_SET_FLAGS(address->class, BIT_UNIT_ADDRESS);

	dt->content.value.bool = st_direct_address_term_value_bool;
    }
    
    dt->location = term_location;
    dt->content.address = address;
    dt->content.data = 0;

    return &(dt->expression);

error_free_resources:
    free(dt);
    free(term_location);
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
    struct function_invocation_term_t *ft = NULL;
    struct st_location_t *ft_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	ft,
	struct function_invocation_term_t,
	parser->errors,
	error_free_resources);
    LOCDUP_OR_ERROR_JUMP(
	ft_location,
	location,
	parser->errors,
	error_free_resources);

    int ref_add_result = parser->function_ref_pool->add(
	parser->function_ref_pool,
	function_identifier,
	ft,
	location,
	st_function_invocation_term_function_resolved,
	parser->errors);

    if(ref_add_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    ft->location = ft_location;
    ft->function = NULL;
    ft->parameters = invoke_parameters;

    ft->expression.invoke.location = st_function_invocation_term_location;
    ft->expression.invoke.step = st_function_invocation_term_step;
    ft->expression.invoke.verify = st_function_invocation_term_verify;
    ft->expression.invoke.allocate = NULL;
    ft->expression.invoke.reset = st_function_invocation_term_reset;
    ft->expression.invoke.clone = NULL;
    ft->expression.invoke.destroy = NULL;

    ft->expression.return_value = st_function_invocation_term_return_value;
    ft->expression.clone = st_function_invocation_term_clone;
    ft->expression.destroy = st_function_invocation_term_destroy;

    return &(ft->expression);
    
error_free_resources:
    /* TODO: determine what to destroy */
    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
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
    struct negative_prefix_term_t *nt = NULL;
    struct st_location_t *nt_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	nt,
	struct negative_prefix_term_t,
	parser->errors,
	error_free_resources);
    LOCDUP_OR_ERROR_JUMP(
	nt_location,
	location,
	parser->errors,
	error_free_resources);

    nt->location = nt_location;
    nt->to_negate = term;

    if(nt->to_negate->invoke.step || nt->to_negate->clone)
    {
	nt->expression.invoke.step = st_negative_prefix_term_step;
	nt->expression.invoke.verify = st_negative_prefix_term_verify;
	nt->expression.invoke.reset = st_negative_prefix_term_reset;
    }
    else
    {
	nt->expression.invoke.step = NULL;
	nt->expression.invoke.verify = st_negative_prefix_term_constant_verify;
	nt->expression.invoke.reset = NULL;
    }
    
    nt->expression.invoke.location = st_negative_prefix_term_location;
    nt->expression.invoke.allocate = st_negative_prefix_term_allocate;
    nt->expression.invoke.clone = NULL;
    nt->expression.invoke.destroy = NULL;
    nt->expression.return_value = st_negative_prefix_term_return_value;
    nt->expression.destroy = st_negative_prefix_term_destroy;
    
    nt->expression.clone = NULL;
    if(nt->to_negate->clone)
    {
	nt->expression.clone = st_negative_prefix_term_clone;
    }

    return &(nt->expression);

error_free_resources:
    free(nt);
    free(nt_location);
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
    struct not_prefix_term_t *nt = NULL;
    struct st_location_t *nt_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	nt,
	struct not_prefix_term_t,
	parser->errors,
	error_free_resources);
    LOCDUP_OR_ERROR_JUMP(
	nt_location,
	location,
	parser->errors,
	error_free_resources);

    nt->location = nt_location;
    nt->to_not = term;

    if(nt->to_not->invoke.step || nt->to_not->clone)
    {
	nt->expression.invoke.step = st_not_prefix_term_step;
	nt->expression.invoke.verify = st_not_prefix_term_verify;
	nt->expression.invoke.reset = st_not_prefix_term_reset;
    }
    else
    {
	nt->expression.invoke.step = NULL;
	nt->expression.invoke.verify = st_not_prefix_term_constant_verify;
	nt->expression.invoke.reset = NULL;
    }
    
    nt->expression.invoke.location = st_not_prefix_term_location;
    nt->expression.invoke.allocate = st_not_prefix_term_allocate;
    nt->expression.invoke.clone = NULL;
    nt->expression.invoke.destroy = NULL;
    nt->expression.return_value = st_not_prefix_term_return_value;

    nt->expression.clone = NULL;
    if(nt->to_not->clone)
    {
	nt->expression.clone = st_not_prefix_term_clone;
    }

    nt->expression.destroy = st_not_prefix_term_destroy;

    return &(nt->expression);

error_free_resources:
    free(nt);
    free(nt_location);
    return NULL;
}

/**************************************************************************/
/* Binary expressions                                                     */
/**************************************************************************/
static struct expression_iface_t * new_binary_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    int (*verify_constant_function)(
	struct invoke_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *errors),
    int (*verify_function)(
	struct invoke_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *errors),
    const struct st_location_t *location,
    int (*step_function)(
	struct invoke_iface_t *,
	struct cursor_t *,
	const struct systime_iface_t *,
	const struct config_iface_t *,
	struct issues_iface_t *),
    int (*allocate_function)(
	struct invoke_iface_t *,
	struct issues_iface_t *),
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

    if((left_operand->invoke.step || left_operand->clone) || (right_operand->invoke.step || right_operand->clone))
    {
	be->expression.invoke.verify = verify_function;
	be->expression.invoke.step = step_function;
	be->expression.invoke.reset = st_binary_expression_reset;
    }
    else
    {
	be->expression.invoke.verify = verify_constant_function;
	be->expression.invoke.step = NULL;
	be->expression.invoke.reset = NULL;
    }

    if(left_operand->clone || right_operand->clone)
    {
	be->expression.clone = st_binary_expression_clone;
    }
    else
    {
	be->expression.clone = NULL;
    }
    
    be->expression.invoke.location = st_binary_expression_location;
    be->expression.invoke.clone = NULL;
    be->expression.invoke.allocate = allocate_function;
    be->expression.return_value = st_binary_expression_return_value;
    be->expression.destroy = st_binary_expression_destroy;
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
	st_xor_expression_constant_verify,
	st_xor_expression_verify,
	location,
	st_xor_expression_step,
	st_binary_expression_allocate,
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
	st_and_expression_constant_verify,
	st_and_expression_verify,       
	location,
	st_and_expression_step,
	st_binary_expression_allocate,
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
	st_or_expression_constant_verify,
	st_or_expression_verify,
	location,
	st_or_expression_step,
	st_binary_expression_allocate,
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
	st_greater_expression_constant_verify,
	st_greater_expression_verify,
	location,
	st_greater_expression_step,
	st_binary_expression_allocate_bool,
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
	st_lesser_expression_constant_verify,
	st_lesser_expression_verify,	
	location,
	st_lesser_expression_step,
	st_binary_expression_allocate_bool,
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
	st_equals_expression_constant_verify,
	st_equals_expression_verify,
	location,
	st_equals_expression_step,
	st_binary_expression_allocate_bool,
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
	st_gequals_expression_constant_verify,
	st_gequals_expression_verify,
	location,
	st_gequals_expression_step,
	st_binary_expression_allocate_bool,
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
	st_lequals_expression_constant_verify,
	st_lequals_expression_verify,
	location,
	st_lequals_expression_step,	
	st_binary_expression_allocate_bool,
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
	st_nequals_expression_constant_verify,
	st_nequals_expression_verify,	
	location,
	st_nequals_expression_step,
	st_binary_expression_allocate_bool,
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
	st_plus_expression_constant_verify,
	st_plus_expression_verify,	
	location,
	st_plus_expression_step,
	st_binary_expression_allocate,
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
	st_minus_expression_constant_verify,
	st_minus_expression_verify,	
	location,
	st_minus_expression_step,
	st_binary_expression_allocate,
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
	st_multiply_expression_constant_verify,
	st_multiply_expression_verify,
	location,
	st_multiply_expression_step,
	st_binary_expression_allocate,
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
	st_division_expression_constant_verify,
	st_division_expression_verify,
	location,
	st_division_expression_step,
	st_binary_expression_allocate,
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
	st_mod_expression_constant_verify,
	st_mod_expression_verify,
	location,
	st_mod_expression_step,
	st_binary_expression_allocate,
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
	st_power_expression_constant_verify,
	st_power_expression_verify,	
	location,
	st_power_expression_step,
	st_binary_expression_allocate,
	parser);
}
