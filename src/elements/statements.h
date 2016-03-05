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

#include <elements/iinvoke.h>
#include <elements/iexpression.h>
#include <elements/shared.h>
#include <elements/variables.h>
#include <elements/pous.h>

/**************************************************************************/
/* Empty statement                                                        */
/**************************************************************************/
struct empty_statement_t {
    struct invoke_iface_t invoke;
    struct st_location_t *location;
};

int st_empty_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_empty_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

const struct st_location_t * st_empty_statement_location(
    const struct invoke_iface_t *self);

struct invoke_iface_t * st_empty_statement_clone(
    struct invoke_iface_t *self);

int st_empty_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config);

void st_empty_statement_destroy(
    struct invoke_iface_t *self);

void st_empty_statement_clone_destroy(
    struct invoke_iface_t *self);

/**************************************************************************/
/* Simple assignment                                                      */
/**************************************************************************/
struct simple_assignment_statement_t {
    struct invoke_iface_t invoke;
    struct variable_t *lhs;
    struct st_location_t *lhs_location;
    struct st_location_t *location;
    struct expression_iface_t *rhs;
    int invoke_state;
};

int st_assignment_statement_simple_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_assignment_statement_simple_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

const struct st_location_t * st_assignment_statement_simple_location(
    const struct invoke_iface_t *self);

struct invoke_iface_t * st_assignment_statement_simple_clone(
    struct invoke_iface_t *self);

int st_assignment_statement_simple_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config);

void st_assignment_statement_simple_destroy(
    struct invoke_iface_t *self);

void st_assignment_statement_simple_clone_destroy(
    struct invoke_iface_t *self);

/**************************************************************************/
/* Qualified identifier assignment                                        */
/**************************************************************************/
struct qualified_assignment_statement_t {
    struct invoke_iface_t invoke;
    struct st_location_t *location;
    struct qualified_identifier_t *lhs;
    struct expression_iface_t *rhs;
    int lhs_invoke_state;
    int rhs_invoke_state;
};

int st_assignment_statement_qualified_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_assignment_statement_qualified_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

const struct st_location_t * st_assignment_statement_qualified_location(
    const struct invoke_iface_t *self);

struct invoke_iface_t * st_assignment_statement_qualified_clone(
    struct invoke_iface_t *self);

int st_assignment_statement_qualified_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config);
   
void st_assignment_statement_qualified_destroy(
    struct invoke_iface_t *self);

void st_assignment_statement_qualified_clone_destroy(
    struct invoke_iface_t *self);

/**************************************************************************/
/* Invoke statement                                                       */
/**************************************************************************/
struct invoke_statement_t {
    struct invoke_iface_t invoke;
    struct st_location_t *location;
    struct variable_t *variable;
    struct function_t *function;
    struct invoke_parameter_t *parameters;
    
    int invoke_state;
};

const struct st_location_t * st_invoke_statement_location(
    const struct invoke_iface_t *self);
    
int st_invoke_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_invoke_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_invoke_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config);

struct invoke_iface_t * st_invoke_statement_clone(
    struct invoke_iface_t *self);

void st_invoke_statement_destroy(
    struct invoke_iface_t *self);

void st_invoke_statement_clone_destroy(
    struct invoke_iface_t *self);

/**************************************************************************/
/* Case statement                                                         */
/**************************************************************************/
struct case_list_element_t {
    struct value_iface_t *value;
    struct st_location_t *location;
    struct case_list_element_t *prev;
    struct case_list_element_t *next;
};

struct case_t {
    struct case_list_element_t *case_list;
    struct invoke_iface_t *statements;
    struct st_location_t *location;
    struct case_t *prev;
    struct case_t *next;
};

struct case_statement_t {
    struct invoke_iface_t invoke;
    struct expression_iface_t *selector;
    struct case_t *cases;
    struct st_location_t *location;
    struct invoke_iface_t *else_statements;
    int invoke_state;
};

const struct st_location_t * st_case_statement_location(
    const struct invoke_iface_t *self);

int st_case_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_case_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_case_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config);

struct invoke_iface_t * st_case_statement_clone(
    struct invoke_iface_t *self);

void st_case_statement_destroy(
    struct invoke_iface_t *self);

void st_case_statement_clone_destroy(
    struct invoke_iface_t *self);

/**************************************************************************/
/* If statements                                                          */
/**************************************************************************/
struct if_statement_t {
    struct invoke_iface_t invoke;
    struct expression_iface_t *condition;
    struct st_location_t *location;
    struct invoke_iface_t *true_statements;
    struct if_statement_t *elsif;
    struct invoke_iface_t *else_statements;
    int invoke_state;
};

const struct st_location_t * st_if_statement_location(
    const struct invoke_iface_t *self);
    
int st_if_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_if_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_if_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config);

struct invoke_iface_t * st_if_statement_clone(
    struct invoke_iface_t *self);

void st_if_statement_destroy(
    struct invoke_iface_t *self);

void st_if_statement_clone_destroy(
    struct invoke_iface_t *self);

/**************************************************************************/
/* While statement                                                        */
/**************************************************************************/
struct while_statement_t {
    struct invoke_iface_t invoke;
    struct expression_iface_t *while_expression;
    struct invoke_iface_t *statements;
    struct st_location_t *location;
    int invoke_state;
};

const struct st_location_t * st_while_statement_location(
    const struct invoke_iface_t *self);

int st_while_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_while_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_while_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config);

struct invoke_iface_t * st_while_statement_clone(
    struct invoke_iface_t *self);

void st_while_statement_destroy(
    struct invoke_iface_t *self);

void st_while_statement_clone_destroy(
    struct invoke_iface_t *self);

/**************************************************************************/
/* For statement                                                          */
/**************************************************************************/
struct for_statement_t {
    struct invoke_iface_t invoke;
    struct variable_t *variable;
    struct expression_iface_t *from;
    struct expression_iface_t *to;
    struct expression_iface_t *increment;
    struct invoke_iface_t *statements;
    struct st_location_t *location;
    struct st_location_t *identifier_location;
    int invoke_state;
};

const struct st_location_t * st_for_statement_location(
    const struct invoke_iface_t *self);

int st_for_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_for_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

int st_for_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config);

struct invoke_iface_t * st_for_statement_clone(
    struct invoke_iface_t *self);

void st_for_statement_destroy(
    struct invoke_iface_t *self);

void st_for_statement_clone_destroy(
    struct invoke_iface_t *self);

/**************************************************************************/
/* Repeat statement                                                       */
/**************************************************************************/
int st_repeat_statement_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

/**************************************************************************/
/* Exit statement                                                         */
/**************************************************************************/

/* TODO: exit statement, struct and invoke functions */

/**************************************************************************/
/* Return statement                                                       */
/**************************************************************************/

/* TODO: return statement, struct and invoke functions */

