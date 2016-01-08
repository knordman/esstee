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
#include <linker/linker.h>
#include <util/macros.h>

#include <utlist.h>

struct invoke_iface_t * st_append_to_statement_list(
    struct invoke_iface_t *statement_list,
    struct invoke_iface_t *statement,
    struct parser_t *parser)
{
    DL_APPEND(statement_list, statement);
    return statement_list;
}

struct invoke_iface_t * st_new_empty_statement(
    struct invoke_iface_t *statement_list,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    
    return NULL;
}

/**************************************************************************/
/* Simple assignment                                                      */
/**************************************************************************/
struct invoke_iface_t * st_new_assignment_statement_simple(
    char *var_identifier,
    const struct st_location_t *var_location,
    struct expression_iface_t *assignment,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct simple_assignment_statement_t *sa = NULL;
    struct st_location_t *loc_sa = NULL, *loc_lhs = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	sa,
	struct simple_assignment_statement_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc_sa,
	location,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc_lhs,
	var_location,
	parser->errors,
	error_free_resources);

    if(parser->pou_var_ref_pool->add(
	parser->pou_var_ref_pool,
	var_identifier,
	sa,
	NULL,
	var_location,
	st_simple_assignment_variable_resolved) != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    sa->location = loc_sa;
    sa->lhs_location = loc_lhs;
    sa->lhs = NULL;
    
    sa->invoke.location = st_assignment_statement_simple_location;
    sa->invoke.step = st_assignment_statement_simple_step;
    sa->invoke.verify = st_assignment_statement_simple_verify;
    
    sa->rhs = assignment;

    free(var_identifier);
    
    return &(sa->invoke);
    
error_free_resources:
    free(var_identifier);
    free(sa);
    free(loc_sa);
    free(loc_lhs);
    assignment->destroy(assignment);
    return NULL;
}

/**************************************************************************/
/* Qualified assignment                                                   */
/**************************************************************************/
struct invoke_iface_t * st_new_assignment_statement_qualified(
    struct qualified_identifier_t *qualified_identifier,
    struct expression_iface_t *assignment,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct qualified_assignment_statement_t *qis = NULL;
    ALLOC_OR_ERROR_JUMP(
	qis,
	struct qualified_assignment_statement_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	qis->location,
	location,
	parser->errors,
	error_free_resources);
    
    qis->invoke.location = st_assignment_statement_qualified_location;
    qis->invoke.step = st_assignment_statement_qualified_step;
    qis->invoke.verify = st_assignment_statement_qualified_verify;

    qis->lhs = qualified_identifier;
    qis->rhs = assignment;
    
    return &(qis->invoke);
    
error_free_resources:
    free(qis);
    return NULL;
}

/**************************************************************************/
/* Invoke statement                                                       */
/**************************************************************************/
struct invoke_iface_t * st_new_invoke_statement(
    char *identifier,
    const struct st_location_t *identfier_location,
    struct invoke_parameter_t *invoke_parameters,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    /* TODO: new invoke statement */
    return NULL;
}

/**************************************************************************/
/* If statements                                                          */
/**************************************************************************/
struct if_statement_t * st_new_elsif_clause(
    struct expression_iface_t *condition,
    const struct st_location_t *condition_location,
    struct invoke_iface_t *true_statements,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    /* TODO: new if statement */
    return NULL;
}

struct if_statement_t * st_append_elsif_clause(
    struct if_statement_t *elsif_clauses,
    struct if_statement_t *elsif_clause,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    /* TODO: append else if to if */
    return NULL;
}

struct invoke_iface_t * st_new_if_statement(
    struct expression_iface_t *condition,
    const struct st_location_t *condition_location,
    struct invoke_iface_t *true_statements,
    struct if_statement_t *elsif_clauses,
    struct invoke_iface_t *else_statements,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    /* TODO: new if statement */
    return NULL;
}

/**************************************************************************/
/* Case statement                                                         */
/**************************************************************************/
struct value_iface_t * st_append_case_value(
    struct value_iface_t *case_list,
    struct value_iface_t *case_value,
    struct parser_t *parser)
{
    /* TODO: append case value to case */
    return NULL;
}

struct case_t * st_new_case(
    struct value_iface_t *case_value_list,
    struct invoke_iface_t *statements,
    struct parser_t *parser)
{
    /* TODO: new case */
    return NULL;
}

struct case_t * st_append_case(
    struct case_t *case_list,
    struct case_t *new_case,
    struct parser_t *parser)
{
    /* TODO: append case to case conditional */
    return NULL;
}

struct invoke_iface_t * st_new_case_statement(
    struct expression_iface_t *switcher,
    struct case_t *case_list,
    struct invoke_iface_t *else_statements,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    /* TODO: new case statement */
    return NULL;
}

/**************************************************************************/
/* For statement                                                          */
/**************************************************************************/
struct invoke_iface_t * st_new_for_statement(
    char *variable_identifier,
    const struct st_location_t *identifier_location,
    struct expression_iface_t *start,
    struct expression_iface_t *end,
    struct expression_iface_t *increment,
    struct invoke_iface_t *statements,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    /* TODO: new for statement */
    return NULL;
}

/**************************************************************************/
/* While statement                                                        */
/**************************************************************************/
struct invoke_iface_t * st_new_while_statement(
    struct expression_iface_t *while_expression,
    struct invoke_iface_t *true_statements,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    /* TODO: new while statement */
    return NULL;
}

/**************************************************************************/
/* Repeat statement                                                       */
/**************************************************************************/
struct invoke_iface_t * st_new_repeat_statement(
    struct expression_iface_t *repeat_expression,
    struct invoke_iface_t *statements,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    /* TODO: new repeat statement */
    return NULL;
}

/**************************************************************************/
/* Exit statement                                                         */
/**************************************************************************/
struct invoke_iface_t * st_new_exit_statement(
    const struct st_location_t *location,
    struct parser_t *parser)
{
    /* TODO: new exit statement */
    return NULL;
}

/**************************************************************************/
/* Return statement                                                       */
/**************************************************************************/
struct invoke_iface_t * st_new_return_statement(
    const struct st_location_t *location,
    struct parser_t *parser)
{
    /* TODO: new return statement */
    return NULL;
}
