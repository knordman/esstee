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
#include <elements/queries.h>

#include <utlist.h>


struct query_t * st_new_query_by_program(
    char *prgm_identifier,
    const struct st_location_t *prgm_location,
    struct parser_t *parser)
{
    struct query_t *query = st_create_program_query(
	prgm_identifier,
	prgm_location,
	parser->program_ref_pool,
	parser->config,
	parser->errors);

    if(!query)
    {
	free(prgm_identifier);
    }

    return query;
}

struct query_t * st_new_query_by_identifier(
    char *prgm_identifier,
    const struct st_location_t *prgm_location,
    char *identifier,
    const struct st_location_t *identifier_location,
    struct expression_iface_t *assigned,
    struct parser_t *parser)
{
    struct query_t *query = st_create_identifier_query(
	prgm_identifier,
	prgm_location,
	identifier,
	identifier_location,
	assigned,
	parser->program_ref_pool,
	parser->pou_var_ref_pool,
	parser->config,
	parser->errors);

    if(!query)
    {
	free(prgm_identifier);
	free(identifier);
	assigned->destroy(assigned);
    }

    return query;
}

struct query_t * st_new_query_by_qualified_identifier(
    char *prgm_identifier,
    const struct st_location_t *prgm_location,
    struct qualified_identifier_iface_t *qid,
    struct expression_iface_t *assigned,
    struct parser_t *parser)
{
    struct query_t *query = st_create_qualified_identifier_query(
	prgm_identifier,
	prgm_location,
	qid,
	assigned,
	parser->program_ref_pool,
	parser->config,
	parser->errors);

    if(!query)
    {
	free(prgm_identifier);
	qid->destroy(qid);
	assigned->destroy(assigned);
    }

    return query;
}

int st_append_query(
    struct query_t *query,
    struct parser_t *parser)
{
    if(parser->queries)
    {
	int append_result = parser->queries->append(parser->queries,
						    query,
						    parser->config,
						    parser->errors);
	if(append_result != ESSTEE_OK)
	{
	    goto error_free_resources;
	}
    }
    else
    {
	parser->queries = st_create_queries(query,
					    parser->config,
					    parser->errors);

	if(!parser->queries)
	{
	    goto error_free_resources;
	}
    }

    return ESSTEE_OK;

error_free_resources:
    st_destroy_query(query);
    return ESSTEE_ERROR;
}

int st_finish_queries(
    struct parser_t *parser)
{
    int finish_result = parser->queries->finish(parser->queries,
						parser->pou_var_ref_pool,
						parser->function_ref_pool,
						parser->program_ref_pool,
						parser->config,
						parser->errors);

    if(finish_result == ESSTEE_OK)
    {
	parser->pou_var_ref_pool = NULL;
	parser->function_ref_pool = NULL;
	parser->program_ref_pool = NULL;
    }
	   
    return finish_result;
}
