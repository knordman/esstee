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
#include <statements/conditionals.h>
#include <statements/statements.h>


struct if_statement_t * st_new_elsif_clause(
    struct expression_iface_t *condition,
    const struct st_location_t *condition_location,
    struct invoke_iface_t *true_statements,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct if_statement_t *clause = st_create_elsif_clause(
	condition,
	condition_location,
	true_statements,
	location,
	parser->config,
	parser->errors);

    if(!clause)
    {
	condition->destroy(condition);
	st_destroy_statements(true_statements);
    }

    return clause;
}

struct if_statement_t * st_append_elsif_clause(
    struct if_statement_t *elsif_clauses,
    struct if_statement_t *elsif_clause,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct if_statement_t *new_clauses = st_extend_elsif_clauses(
	elsif_clauses,
	elsif_clause,
	location,
	parser->config,
	parser->errors);

    if(!new_clauses)
    {
	st_destroy_elsif_clauses(elsif_clauses);
	st_destroy_elsif_clauses(elsif_clause);
    }

    return new_clauses;
}
