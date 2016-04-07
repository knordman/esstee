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

#pragma once

#include <elements/iexpression.h>
#include <elements/variables.h>
#include <elements/pous.h>
#include <elements/shared.h>
#include <elements/types.h>
#include <elements/values.h>
#include <rt/isystime.h>
#include <rt/cursor.h>
#include <util/bitflag.h>
#include <util/iconfig.h>
#include <esstee/locations.h>

/**************************************************************************/
/* Wrapped inline value expression (e.g. literal)                         */
/**************************************************************************/
struct value_expression_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct value_iface_t *value;
};

const struct st_location_t * st_value_expression_location(
    const struct invoke_iface_t *self);
    
const struct value_iface_t * st_value_expression_return_value(
    struct expression_iface_t *self);

void st_value_expression_destroy(
    struct expression_iface_t *self);

/**************************************************************************/
/* Single identifier term, either an enum, or a variable reference        */
/**************************************************************************/
struct single_identifier_term_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct inline_enum_value_t inline_enum;
    struct variable_t *variable;
};

const struct st_location_t * st_single_identifier_term_location(
    const struct invoke_iface_t *self);

struct expression_iface_t * st_single_identifier_term_clone(
    struct expression_iface_t *self,
    struct issues_iface_t *issues);

const struct value_iface_t * st_single_identifier_term_var_return_value(
    struct expression_iface_t *self);

const struct value_iface_t * st_single_identifier_term_enum_return_value(
    struct expression_iface_t *self);

void st_single_identifier_term_destroy(
    struct expression_iface_t *self);

void st_single_identfier_term_clone_destroy(
    struct expression_iface_t *self);

/**************************************************************************/
/* Qualified identifier term                                              */
/**************************************************************************/
struct qualified_identifier_term_t {
    struct expression_iface_t expression;
    struct qualified_identifier_t *identifier;
    int invoke_state;
    struct st_location_t *location;
};

const struct st_location_t * st_qualified_identifier_term_location(
    const struct invoke_iface_t *self);

int st_qualified_identifier_term_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_qualified_identifier_term_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_qualified_identifier_term_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct value_iface_t * st_qualified_identifier_term_return_value(
    struct expression_iface_t *self);

struct expression_iface_t * st_qualified_identifier_term_clone(
    struct expression_iface_t *self,
    struct issues_iface_t *issues);

void st_qualified_identifier_term_destroy(
    struct expression_iface_t *self);

/**************************************************************************/
/* Direct address term                                                    */
/**************************************************************************/
struct direct_address_term_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct direct_address_term_value_t content;
};

const struct st_location_t * st_direct_address_term_location(
    const struct invoke_iface_t *self);

const struct value_iface_t * st_direct_address_term_return_value(
    struct expression_iface_t *self);
    
void st_direct_address_term_destroy(
    struct expression_iface_t *self);

/**************************************************************************/
/* Negative prefix term                                                   */
/**************************************************************************/
struct negative_prefix_term_t {
    struct expression_iface_t expression;
    struct value_iface_t *temporary;
    struct expression_iface_t *to_negate;
    struct st_location_t *location;
    int invoke_state;
};

const struct st_location_t * st_negative_prefix_term_location(
    const struct invoke_iface_t *self);

int st_negative_prefix_term_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_negative_prefix_term_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_negative_prefix_term_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_negative_prefix_term_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues);

int st_negative_prefix_term_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct value_iface_t * st_negative_prefix_term_return_value(
    struct expression_iface_t *self);
    
struct expression_iface_t * st_negative_prefix_term_clone(
    struct expression_iface_t *self,
    struct issues_iface_t *issues);

void st_negative_prefix_term_destroy(
    struct expression_iface_t *self);

void st_negative_prefix_term_clone_destroy(
    struct expression_iface_t *self);

/**************************************************************************/
/* Function invocation term                                               */
/**************************************************************************/
struct function_invocation_term_t {
    struct expression_iface_t expression;
    struct function_t *function;
    struct invoke_parameter_t *parameters;
    struct st_location_t *location;
    int invoke_state;
};

const struct st_location_t * st_function_invocation_term_location(
    const struct invoke_iface_t *self);

int st_function_invocation_term_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_function_invocation_term_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_function_invocation_term_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct value_iface_t * st_function_invocation_term_return_value(
    struct expression_iface_t *self);

struct expression_iface_t * st_function_invocation_term_clone(
    struct expression_iface_t *self,
    struct issues_iface_t *issues);

void st_function_invocation_term_destroy(
    struct expression_iface_t *self);

void st_function_invocation_clone_destroy(
    struct expression_iface_t *self);

/**************************************************************************/
/* Not prefix term                                                        */
/**************************************************************************/
struct not_prefix_term_t {
    struct expression_iface_t expression;
    struct value_iface_t *temporary;
    struct expression_iface_t *to_not;
    struct st_location_t *location;
    int invoke_state;
};

const struct st_location_t * st_not_prefix_term_location(
    const struct invoke_iface_t *self);

int st_not_prefix_term_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_not_prefix_term_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_not_prefix_term_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_not_prefix_term_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues);

int st_not_prefix_term_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct value_iface_t * st_not_prefix_term_return_value(
    struct expression_iface_t *self);    

struct expression_iface_t * st_not_prefix_term_clone(
    struct expression_iface_t *self,
    struct issues_iface_t *issues);

void st_not_prefix_term_destroy(
    struct expression_iface_t *self);

void st_not_prefix_term_clone_destroy(
    struct expression_iface_t *self);

/**************************************************************************/
/* Binary expressions                                                     */
/**************************************************************************/
struct binary_expression_t {
    struct expression_iface_t expression;
    struct expression_iface_t *left_operand;
    struct expression_iface_t *right_operand;
    int invoke_state;
    struct st_location_t *location;
    struct value_iface_t *temporary;
};

const struct st_location_t * st_binary_expression_location(
    const struct invoke_iface_t *self);

int st_binary_expression_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_binary_expression_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues);

int st_binary_expression_allocate_bool(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues);

const struct value_iface_t * st_binary_expression_return_value(
    struct expression_iface_t *self);

struct expression_iface_t * st_binary_expression_clone(
    struct expression_iface_t *self,
    struct issues_iface_t *issues);

void st_binary_expression_destroy(
    struct expression_iface_t *self);

void st_binary_expression_clone_destroy(
    struct expression_iface_t *self);

/* xor */
int st_xor_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_xor_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_xor_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* and */
int st_and_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_and_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_and_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* or */
int st_or_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_or_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_or_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* plus */
int st_plus_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_plus_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_plus_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* minus */
int st_minus_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_minus_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_minus_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* multiply */
int st_multiply_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_multiply_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_multiply_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* division */
int st_division_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_division_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_division_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* greater */
int st_greater_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_greater_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_greater_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* lesser */
int st_lesser_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_lesser_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_lesser_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* equals */
int st_equals_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_equals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_equals_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* greater or equals */
int st_gequals_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_gequals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_gequals_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* lesser or equals */
int st_lequals_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_lequals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_lequals_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* not equals */
int st_nequals_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_nequals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_nequals_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* mod */
int st_mod_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_mod_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_mod_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* power */
int st_power_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_power_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_power_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
