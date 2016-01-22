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
#include <rt/isystime.h>
#include <rt/cursor.h>
#include <util/bitflag.h>
#include <util/iconfig.h>
#include <esstee/locations.h>

/**************************************************************************/
/* Single identifier term, either an enum, or a variable reference        */
/**************************************************************************/
struct single_identifier_term_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct value_iface_t value;	/* identifier value */
    char *identifier;
    struct variable_t *variable;
};

const struct value_iface_t * st_single_identifier_term_var_return_value(
    struct expression_iface_t *self);

const struct value_iface_t * st_single_identifier_term_enum_return_value(
    struct expression_iface_t *self);

const struct st_location_t * st_single_identifier_term_location(
    const struct invoke_iface_t *self);

void st_single_identifier_term_destroy(
    struct expression_iface_t *self);

/**************************************************************************/
/* Qualified identifier term                                              */
/**************************************************************************/
struct qualified_identifier_term_t {
    struct expression_iface_t expression;
    struct qualified_identifier_t *identifier;
    struct st_location_t *location;
};

int st_qualified_identifier_term_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);
    
const struct value_iface_t * st_qualified_identifier_term_return_value(
    struct expression_iface_t *self);
    
const struct st_location_t * st_qualified_identifier_term_location(
    const struct invoke_iface_t *self);

void st_qualified_identifier_term_destroy(
    struct expression_iface_t *self);

/**************************************************************************/
/* Direct address term                                                    */
/**************************************************************************/
struct direct_address_term_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct direct_address_t *direct_address;
};

/* TODO: direct address term expression functions */

/**************************************************************************/
/* Negative prefix term                                                   */
/**************************************************************************/
struct negative_prefix_term_t {
    struct expression_iface_t expression;
    struct value_iface_t evaluated_value;
    struct expression_iface_t *to_negate;
};

/* TODO: negative prefix term expression functions */

/**************************************************************************/
/* Function invocation term                                               */
/**************************************************************************/
struct function_invocation_term_t {
    struct expression_iface_t expression;
    struct function_t *function;
    struct invoke_parameter_t *parameters;
};

/* TODO: function invocation term expression functions */

/**************************************************************************/
/* Not prefix term                                                        */
/**************************************************************************/
struct not_prefix_term_t {
    struct expression_iface_t expression;
    struct value_iface_t evaluated_value;
    struct expression_iface_t *to_not;
};

/* TODO: not prefix term expression functions */

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

int st_binary_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

const struct st_location_t * st_binary_expression_location(
    const struct invoke_iface_t *self);

const struct value_iface_t * st_binary_expression_return_value(
    struct expression_iface_t *self);

int st_xor_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_and_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_or_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_plus_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_minus_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_multiply_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_division_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);