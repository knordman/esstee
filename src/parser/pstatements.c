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

/**************************************************************************/
/* Empty statement                                                        */
/**************************************************************************/

struct invoke_iface_t * st_new_empty_statement(
    struct invoke_iface_t *statement_list,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct empty_statement_t *es = NULL;
    struct st_location_t *loc = NULL;

    ALLOC_OR_ERROR_JUMP(
	es,
	struct empty_statement_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	parser->errors,
	error_free_resources);

    es->location = loc;
    es->invoke.verify = st_empty_statement_verify;
    es->invoke.step = st_empty_statement_step;
    es->invoke.location = st_empty_statement_location;
    es->invoke.clone = st_empty_statement_clone;
    es->invoke.reset = st_empty_statement_reset;
    es->invoke.destroy = st_empty_statement_destroy;

    DL_APPEND(statement_list, &(es->invoke));
    return statement_list;

error_free_resources:
    free(loc);
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
    sa->invoke.clone = st_assignment_statement_simple_clone;
    sa->invoke.reset = st_assignment_statement_simple_reset;
    
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
    qis->invoke.clone = st_assignment_statement_qualified_clone;
    qis->invoke.reset = st_assignment_statement_qualified_reset;

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
    const struct st_location_t *identifier_location,
    struct invoke_parameter_t *invoke_parameters,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct invoke_statement_t *is = NULL;
    struct st_location_t *loc = NULL;
    ALLOC_OR_ERROR_JUMP(
	is,
	struct invoke_statement_t,
	parser->errors,
	error_free_resources);
    LOCDUP_OR_ERROR_JUMP(
	loc,
	identifier_location,
	parser->errors,
	error_free_resources);

    if(parser->pou_var_ref_pool->add(
	   parser->pou_var_ref_pool,
	   identifier,
	   is,
	   NULL,
	   identifier_location,
	   st_invoke_statement_as_variable_resolved) != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    if(parser->function_ref_pool->add(
	   parser->function_ref_pool,
	   identifier,
	   is,
	   NULL,
	   identifier_location,
	   st_invoke_statement_as_func_resolved) != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    is->location = loc;
    is->parameters = invoke_parameters;
    is->invoke.location = st_invoke_statement_location;
    is->invoke.step = st_invoke_statement_step;
    is->invoke.verify = st_invoke_statement_verify;
    is->invoke.reset = st_invoke_statement_reset;
    is->invoke.clone = st_invoke_statement_clone;
    is->invoke.destroy = st_invoke_statement_destroy;

    return &(is->invoke);
    
error_free_resources:
    free(is);
    free(loc);
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
    struct if_statement_t *ifs = NULL;
    struct st_location_t *loc = NULL;
    ALLOC_OR_ERROR_JUMP(
	ifs,
	struct if_statement_t,
	parser->errors,
	error_free_resources);
    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	parser->errors,
	error_free_resources);

    ifs->condition = condition;
    ifs->location = loc;
    ifs->true_statements = true_statements;
    ifs->else_statements = NULL;
    ifs->elsif = NULL;

    ifs->invoke.location = st_if_statement_location;
    ifs->invoke.step = st_if_statement_step;
    ifs->invoke.verify = st_if_statement_verify;
    ifs->invoke.reset = st_if_statement_reset;
    ifs->invoke.clone = st_if_statement_clone;
    ifs->invoke.destroy = st_if_statement_destroy;
	
    return ifs;
    
error_free_resources:
    free(ifs);
    free(loc);
    return NULL;
}

struct if_statement_t * st_append_elsif_clause(
    struct if_statement_t *elsif_clauses,
    struct if_statement_t *elsif_clause,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    if(!elsif_clauses)
    {
	return elsif_clause;
    }
    else
    {
	struct if_statement_t *itr = elsif_clauses;
	for(; itr->elsif != NULL; itr = itr->elsif){}

	itr->elsif = elsif_clause;
    }

    return elsif_clauses;
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
    struct if_statement_t *ifs = NULL;
    struct st_location_t *loc = NULL;
    ALLOC_OR_ERROR_JUMP(
	ifs,
	struct if_statement_t,
	parser->errors,
	error_free_resources);
    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	parser->errors,
	error_free_resources);

    ifs->condition = condition;
    ifs->location = loc;
    ifs->true_statements = true_statements;
    ifs->elsif = elsif_clauses;
    struct if_statement_t *itr = ifs;
    for(; itr->elsif != NULL; itr = itr->elsif){}
    itr->else_statements = else_statements;

    ifs->invoke.location = st_if_statement_location;
    ifs->invoke.step = st_if_statement_step;
    ifs->invoke.verify = st_if_statement_verify;
    ifs->invoke.reset = st_if_statement_reset;
    ifs->invoke.clone = st_if_statement_clone;
    ifs->invoke.destroy = st_if_statement_destroy;
	
    return &(ifs->invoke);
    
error_free_resources:
    /* TODO: determine what to destroy  */
    return NULL;
}

/**************************************************************************/
/* Case statement                                                         */
/**************************************************************************/
struct case_list_element_t * st_append_case_value(
    struct case_list_element_t *case_list,
    struct value_iface_t *case_value,
    const struct st_location_t *case_value_location,
    struct parser_t *parser)
{
    struct case_list_element_t *ce = NULL;
    struct st_location_t *loc = NULL;
    ALLOC_OR_ERROR_JUMP(
	ce,
	struct case_list_element_t,
	parser->errors,
	error_free_resources);
    LOCDUP_OR_ERROR_JUMP(
	loc,
	case_value_location,
	parser->errors,
	error_free_resources);

    ce->location = loc;
    ce->value = case_value;

    DL_APPEND(case_list, ce);

    return case_list;

error_free_resources:
    free(ce);
    free(loc);
    return NULL;
}

struct case_t * st_new_case(
    struct case_list_element_t *case_value_list,
    const struct st_location_t *location,
    struct invoke_iface_t *statements,
    struct parser_t *parser)
{
    struct case_t *c = NULL;
    struct st_location_t *loc = NULL;
    ALLOC_OR_ERROR_JUMP(
	c,
	struct case_t,
	parser->errors,
	error_free_resources);
    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	parser->errors,
	error_free_resources);

    c->location = loc;
    c->case_list = case_value_list;
    c->statements = statements;

    return c;
    
error_free_resources:
    free(c);
    free(loc);
    return NULL;
}

struct case_t * st_append_case(
    struct case_t *case_list,
    struct case_t *new_case,
    struct parser_t *parser)
{
    DL_APPEND(case_list, new_case);

    return case_list;
}

struct invoke_iface_t * st_new_case_statement(
    struct expression_iface_t *selector,
    struct case_t *case_list,
    struct invoke_iface_t *else_statements,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct case_statement_t *cs = NULL;
    struct st_location_t *loc = NULL;
    ALLOC_OR_ERROR_JUMP(
	cs,
	struct case_statement_t,
	parser->errors,
	error_free_resources);
    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	parser->errors,
	error_free_resources);

    cs->location = loc;
    cs->selector = selector;
    cs->cases = case_list;
    cs->else_statements = else_statements;

    cs->invoke.location = st_case_statement_location;
    cs->invoke.step = st_case_statement_step;
    cs->invoke.verify = st_case_statement_verify;
    cs->invoke.reset = st_case_statement_reset;
    cs->invoke.clone = st_case_statement_clone;
    cs->invoke.destroy = st_case_statement_destroy;

    return &(cs->invoke);
    
error_free_resources:
    /* TODO: determine what to destroy */
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
