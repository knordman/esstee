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
#include <statements/empty.h>
#include <statements/case.h>
#include <statements/conditionals.h>
#include <statements/invoke_statement.h>
#include <statements/loops.h>
#include <statements/pop_call_stack.h>
#include <statements/qualified_assignment.h>
#include <statements/simple_assignment.h>
#include <statements/statements.h>

#include <utlist.h>


struct invoke_iface_t * st_append_to_statement_list(
    struct invoke_iface_t *statement_list,
    struct invoke_iface_t *statement,
    struct parser_t *parser)
{
    DL_APPEND(statement_list, statement);
    return statement_list;
}

/**************************************************************************/
/* Empty statement                                                        */
/**************************************************************************/
struct invoke_iface_t * st_new_empty_statement(
    struct invoke_iface_t *statement_list,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct invoke_iface_t *new_statement_list = st_append_empty_statement(
	statement_list,
	location,
	parser->config,
	parser->errors);

    if(!new_statement_list)
    {
	st_destroy_statements(statement_list);
    }

    return new_statement_list;
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
    struct invoke_iface_t *statement = st_create_assignment_statement_simple(
	var_identifier,
	var_location,
	assignment,
	location,
	parser->pou_var_ref_pool,
	parser->config,
	parser->errors);

    if(!statement)
    {
	free(var_identifier);
	assignment->destroy(assignment);
    }

    return statement;
}

/**************************************************************************/
/* Qualified assignment                                                   */
/**************************************************************************/
struct invoke_iface_t * st_new_assignment_statement_qualified(
    struct qualified_identifier_iface_t *qualified_identifier,
    struct expression_iface_t *assignment,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct invoke_iface_t *statement = st_create_assignment_statement_qualified(
	qualified_identifier,
	assignment,
	location,
	parser->config,
	parser->errors);

    if(!statement)
    {
	qualified_identifier->destroy(qualified_identifier);
	assignment->destroy(assignment);
    }

    return statement;
}

/**************************************************************************/
/* Invoke statement                                                       */
/**************************************************************************/
struct invoke_iface_t * st_new_invoke_statement(
    char *identifier,
    const struct st_location_t *identifier_location,
    struct invoke_parameters_iface_t *invoke_parameters,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct invoke_iface_t *statement = st_create_invoke_statement(
	identifier,
	identifier_location,
	invoke_parameters,
	location,
	parser->pou_var_ref_pool,
	parser->function_ref_pool,
	parser->config,
	parser->errors);

    if(!statement)
    {
	free(identifier);
	invoke_parameters->destroy(invoke_parameters);
    }

    return statement;
}

/**************************************************************************/
/* If statements                                                          */
/**************************************************************************/
struct invoke_iface_t * st_new_if_statement(
    struct expression_iface_t *condition,
    const struct st_location_t *condition_location,
    struct invoke_iface_t *true_statements,
    struct if_statement_t *elsif_clauses,
    struct invoke_iface_t *else_statements,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct invoke_iface_t *statement = st_create_if_statement(
	condition,
	condition_location,
	true_statements,
	elsif_clauses,
	else_statements,
	location,
	parser->config,
	parser->errors);

    if(!statement)
    {
	condition->destroy(condition);
	st_destroy_statements(true_statements);
	st_destroy_elsif_clauses(elsif_clauses);
	st_destroy_statements(else_statements);
    }

    return statement;
}

/**************************************************************************/
/* Case statement                                                         */
/**************************************************************************/
struct invoke_iface_t * st_new_case_statement(
    struct expression_iface_t *selector,
    struct case_t *cases,
    struct invoke_iface_t *else_statements,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct invoke_iface_t *statement = st_create_case_statement(
	selector,
	cases,
	else_statements,
	location,
	parser->config,
	parser->errors);

    if(!statement)
    {
	selector->destroy(selector);
	st_destroy_case(cases);
	st_destroy_statements(else_statements);
    }

    return statement;
}

/**************************************************************************/
/* For statement                                                          */
/**************************************************************************/
struct invoke_iface_t * st_new_for_statement(
    char *variable_identifier,
    const struct st_location_t *identifier_location,
    struct expression_iface_t *from,
    struct expression_iface_t *to,
    struct expression_iface_t *increment,
    struct invoke_iface_t *statements,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct invoke_iface_t *statement = st_create_for_statement(
	variable_identifier,
	identifier_location,
	from,
	to,
	increment,
	statements,
	location,
	parser->pou_var_ref_pool,
	parser->config,
	parser->errors);

    if(!statement)
    {
	free(variable_identifier);
	from->destroy(from);
	to->destroy(to);
	increment->destroy(increment);
	st_destroy_statements(statements);
    }

    return statement;
}

/**************************************************************************/
/* While statement                                                        */
/**************************************************************************/
struct invoke_iface_t * st_new_while_statement(
    struct expression_iface_t *while_expression,
    struct invoke_iface_t *statements,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct invoke_iface_t *statement = st_create_while_statement(
	while_expression,
	statements,
	location,
	parser->config,
	parser->errors);

    if(!statement)
    {
	while_expression->destroy(while_expression);
	st_destroy_statements(statements);
    }

    return statement;
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
    struct invoke_iface_t *statement = st_create_repeat_statement(
	repeat_expression,
	statements,
	location,
	parser->config,
	parser->errors);

    if(!statement)
    {
	repeat_expression->destroy(repeat_expression);
	st_destroy_statements(statements);
    }

    return statement;
}

/**************************************************************************/
/* Exit statement                                                         */
/**************************************************************************/
struct invoke_iface_t * st_new_exit_statement(
    const struct st_location_t *location,
    struct parser_t *parser)
{
    if(parser->loop_level < 1)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "exit outside of loop",
	    ESSTEE_CONTEXT_ERROR,
	    1,
	    location);

	return NULL;
    }

    return st_create_exit_statement(location,
				    parser->config,
				    parser->errors);
}

/**************************************************************************/
/* Return statement                                                       */
/**************************************************************************/
struct invoke_iface_t * st_new_return_statement(
    const struct st_location_t *location,
    struct parser_t *parser)
{
    return st_create_return_statement(location,
				      parser->config,
				      parser->errors);
}
