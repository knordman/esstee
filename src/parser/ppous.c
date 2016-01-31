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


int st_new_function_pou(
    char *identifier,
    const struct st_location_t *location,
    char *return_type_identifier,
    const struct st_location_t *type_identifier_location,
    struct header_t *header,
    struct invoke_iface_t *statements,
    struct parser_t *parser)
{
    struct function_t *f = NULL;
    ALLOC_OR_ERROR_JUMP(f, struct function_t, parser->errors, error_free_resources);

    f->type_ref_pool = parser->pou_type_ref_pool;
    f->var_ref_pool = parser->pou_var_ref_pool;
    parser->pou_type_ref_pool = NULL;
    parser->pou_var_ref_pool = NULL;

    f->header = header;
    f->statements = statements;

    /* TODO: error handling if type ref fails */
    parser->global_type_ref_pool->add(
	parser->global_type_ref_pool,
	return_type_identifier,
	f,
	NULL,
	type_identifier_location,
	st_function_return_type_resolved);

    memcpy(&(f->location), location, sizeof(struct st_location_t));
    
    DL_APPEND(parser->functions, f);
    return ESSTEE_OK;

error_free_resources:
    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
    return ESSTEE_ERROR;
}

int st_new_program_pou(
    char *identifier,
    const struct st_location_t *location,
    struct header_t *header,
    struct invoke_iface_t *statements,
    struct parser_t *parser)
{
    struct program_t *p = NULL;
    ALLOC_OR_ERROR_JUMP(
	p,
	struct program_t,
	parser->errors,
	error_free_resources);

    STRDUP_OR_ERROR_JUMP(
	p->identifier,
	identifier,
	parser->errors,
	error_free_resources);
    
    p->type_ref_pool = parser->pou_type_ref_pool;
    p->var_ref_pool = parser->pou_var_ref_pool;
    parser->pou_type_ref_pool = NULL;
    parser->pou_var_ref_pool = NULL;

    p->header = header;
    p->statements = statements;
    
    memcpy(&(p->location), location, sizeof(struct st_location_t));
    
    DL_APPEND(parser->programs, p);
    return ESSTEE_OK;

error_free_resources:
    free(p);
    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
    return ESSTEE_ERROR;
}

int st_new_function_block_pou(
    char *identifier,
    const struct st_location_t *location,
    struct header_t *header,
    struct invoke_iface_t *statements,
    struct parser_t *parser)
{
    /* TODO: new function block */
    return ESSTEE_ERROR;;
}

int st_new_type_block_pou(
    struct type_iface_t *types,
    struct parser_t *parser)
{
    DL_CONCAT(parser->global_types, types);
    return ESSTEE_OK;
}

int st_new_var_block_pou(
    struct variable_t *variables,
    struct parser_t *parser)
{
    DL_CONCAT(parser->global_variables, variables);
    return ESSTEE_OK;
}

struct header_t * st_append_types_to_header(
    struct header_t *header, 
    struct type_iface_t *type_block, 
    struct parser_t *parser)
{
    if(!header)
    {
	ALLOCATE_OR_ERROR_JUMP(
	    header,
	    struct header_t,
	    parser->errors,
	    error_free_resources);

	header->types = NULL;
	header->variables = NULL;
    }

    DL_CONCAT(header->types, type_block);
    return header;

error_free_resources:
    /* TODO: clear named references in header */
    /* TODO: destroy types block */
    return NULL;
}

struct header_t * st_append_vars_to_header(
    struct header_t *header,
    struct variable_t *var_block,
    struct parser_t *parser)
{
    if(!header)
    {
	ALLOC_OR_ERROR_JUMP(
	    header,
	    struct header_t,
	    parser->errors,
	    error_free_resources);

	header->types = NULL;
	header->variables = NULL;
    }

    DL_CONCAT(header->variables, var_block);

    return header;

error_free_resources:
    /* TODO: clear named references in header */    
    // TODO: destroy vars block
    return NULL;
}
