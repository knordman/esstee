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
#include <elements/ivalue.h>
#include <elements/variables.h>
#include <elements/pous.h>
#include <rt/cursor.h>

/**************************************************************************/
/* Array index                                                            */
/**************************************************************************/
struct array_index_t {
    struct expression_iface_t *index_expression;
    struct st_location_t *location;
    struct array_index_t *prev;
    struct array_index_t *next;
};

void st_destroy_array_index(
    struct array_index_t *ai);

/**************************************************************************/
/* Qualified identifier                                                   */
/**************************************************************************/
struct qualified_identifier_t {
    char *identifier;
    struct array_index_t *array_index;
    int runtime_constant_reference;
    struct variable_t *variable;
    struct value_iface_t *target;
    int invoke_state;
    struct program_t *program;
    struct st_location_t *location;
    struct qualified_identifier_t *last;
    struct qualified_identifier_t *prev;
    struct qualified_identifier_t *next;
};

int st_qualified_identifier_resolve_chain(
    struct qualified_identifier_t *qi,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_qualified_identifier_resolve_array_index(
    struct qualified_identifier_t *qi,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_qualified_identifier_verify(
    struct qualified_identifier_t *qi,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_qualified_identifier_step(
    struct qualified_identifier_t *qi,
    struct cursor_t *cursor,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_qualified_identifier_reset(
    struct qualified_identifier_t *qi,
    const struct config_iface_t *config);

struct qualified_identifier_t * st_clone_qualified_identifier(
    struct qualified_identifier_t *qi);

void st_destroy_qualified_identifier(
    struct qualified_identifier_t *qi);

void st_destroy_qualified_identifier_clone(
    struct qualified_identifier_t *qi);

/**************************************************************************/
/* Invoke parameters                                                      */
/**************************************************************************/
struct invoke_parameter_t {
    char *identifier;
    struct expression_iface_t *expression;
    struct st_location_t *location;
    int invoke_state;
    struct invoke_parameter_t *prev;
    struct invoke_parameter_t *next;
};

int st_verify_invoke_parameters(
    struct invoke_parameter_t *parameters,
    const struct variable_t *variables,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_step_invoke_parameters(
    struct invoke_parameter_t *parameters,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_assign_from_invoke_parameters(
    struct invoke_parameter_t *parameters,
    struct variable_t *variables,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);
    
int st_reset_invoke_parameters(    
    struct invoke_parameter_t *parameters,
    const struct config_iface_t *config);

struct invoke_parameter_t * st_clone_invoke_parameters(    
    struct invoke_parameter_t *parameters);

void st_destroy_invoke_parameters(
    struct invoke_parameter_t *parameters);

void st_destroy_invoke_parameters_clone(
    struct invoke_parameter_t *parameters);
