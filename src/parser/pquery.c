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
#include <elements/query.h>

#include <utlist.h>


int st_new_query_by_identifier(
    char *identifier,
    const struct st_location_t *identifier_location,
    struct expression_iface_t *assign_value,
    struct parser_t *parser)
{
    struct query_t *q = NULL;
    ALLOC_OR_ERROR_JUMP(
	q,
	struct query_t,
	parser->errors,
	error_free_resources);

    struct qualified_identifier_t *qi = NULL;
    ALLOC_OR_ERROR_JUMP(
	qi,
	struct qualified_identifier_t,
	parser->errors,
	error_free_resources);

    struct st_location_t *l = NULL;
    LOCDUP_OR_ERROR_JUMP(
	l,
    	identifier_location,
    	parser->errors,
    	error_free_resources);

    q->qi = qi;
    q->new_value = assign_value;
    
    qi->identifier = identifier;
    qi->location = l;
    qi->array_index = NULL;
    qi->variable = NULL;
    qi->target = NULL;
    qi->program = NULL;

    parser->pou_var_ref_pool->add(
	parser->pou_var_ref_pool,
	qi->identifier,
	qi,
	NULL,
	identifier_location,
	st_qualified_identifier_base_resolved);

    DL_APPEND(parser->queries, q);
    
    return ESSTEE_OK;
    
error_free_resources:
    free(q);
    free(qi);
    free(l);
    parser->queries = NULL;
    return ESSTEE_ERROR;
}

int st_new_query_by_qualified_identifier(
    struct qualified_identifier_t *qualified_identifier,
    struct expression_iface_t *assign_value,
    struct parser_t *parser)
{
    struct query_t *q = NULL;
    ALLOC_OR_ERROR_JUMP(
	q,
	struct query_t,
	parser->errors,
	error_free_resources);

    q->qi = qualified_identifier;
    q->new_value = assign_value;

    DL_APPEND(parser->queries, q);
    
    return ESSTEE_OK;
    
error_free_resources:
    return ESSTEE_ERROR;
}
