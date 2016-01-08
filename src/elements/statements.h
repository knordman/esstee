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
};

/* TODO: empty statement */

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

/**************************************************************************/
/* Qualified identifier assignment                                        */
/**************************************************************************/
struct qualified_assignment_statement_t {
    struct invoke_iface_t invoke;
    struct st_location_t *location;
    struct qualified_identifier_t *lhs;
    struct expression_iface_t *rhs;
    int invoke_state;
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

/**************************************************************************/
/* Invoke statement                                                       */
/**************************************************************************/
struct invoke_statement_t {
    struct invoke_iface_t invoke;

    struct variable_t *var;
    struct function_t *function;
};

/* TODO: invoke statement invoke functions */

/**************************************************************************/
/* If statements                                                          */
/**************************************************************************/
struct if_statement_t {
    struct invoke_iface_t invoke;

    struct expression_iface_t *condition;

    struct invoke_iface_t *true_statements;
    struct if_statement_t *elsif;
    struct invoke_iface_t *else_statements;
};

/* TODO: if statements invoke functions */

/**************************************************************************/
/* While statement                                                        */
/**************************************************************************/
struct while_statement_t {
    struct invoke_iface_t invoke;

    struct expression_iface_t *while_expression;

    struct invoke_iface_t *true_statements;
};

/* TODO: while statement invoke functions */

/**************************************************************************/
/* Repeat statement                                                       */
/**************************************************************************/
struct repeat_statement_t {
    struct invoke_iface_t invoke;

    struct expression_iface_t *until_condition;

    struct invoke_iface_t *statements;
};

/* TODO: repeat statement invoke functions */

/**************************************************************************/
/* Exit statement                                                         */
/**************************************************************************/

/* TODO: exit statement, struct and invoke functions */

/**************************************************************************/
/* Return statement                                                       */
/**************************************************************************/

/* TODO: return statement, struct and invoke functions */
