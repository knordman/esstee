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
#include <elements/user_functions.h>

#include <utlist.h>


int st_new_function_pou(
    char *identifier,
    const struct st_location_t *location,
    char *return_type_identifier,
    const struct st_location_t *return_type_identifier_location,
    struct header_t *header,
    struct invoke_iface_t *statements,
    struct parser_t *parser)
{
    struct function_iface_t *function = st_new_user_function(
	identifier,
	location,
	return_type_identifier,
	return_type_identifier_location,
	header,
	parser->global_type_ref_pool,
	parser->pou_type_ref_pool,
	parser->global_var_ref_pool,
	parser->pou_var_ref_pool,
	statements,
	parser->config,
	parser->errors);

    if(!function)
    {
	/* TODO: determine what to destroy */
	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }

    parser->pou_type_ref_pool = NULL;
    parser->pou_var_ref_pool = NULL;

    DL_APPEND(parser->functions, function);
    return ESSTEE_OK;
}

int st_new_program_pou(
    char *identifier,
    const struct st_location_t *location,
    struct header_t *header,
    struct invoke_iface_t *statements,
    struct parser_t *parser)
{
    struct program_t *p = NULL;
    struct st_location_t *loc = NULL;
    ALLOC_OR_ERROR_JUMP(
	p,
	struct program_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	parser->errors,
	error_free_resources);

    p->identifier = identifier;
    p->type_ref_pool = parser->pou_type_ref_pool;
    p->var_ref_pool = parser->pou_var_ref_pool;
    parser->pou_type_ref_pool = NULL;
    parser->pou_var_ref_pool = NULL;

    p->header = header;
    p->statements = statements;    
    p->location = loc;
    
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
    struct function_block_t *fb = NULL;
    struct st_location_t *loc = NULL;
    ALLOC_OR_ERROR_JUMP(
	fb,
	struct function_block_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	parser->errors,
	error_free_resources);

    memset(&(fb->type), 0, sizeof(struct type_iface_t));
    fb->type.identifier = identifier;
    fb->type.location = st_function_block_type_location;
    fb->type.create_value_of = st_function_block_type_create_value_of;
    fb->type.reset_value_of = st_function_block_type_reset_value_of;
    fb->type.class = st_function_block_type_class;
    
    fb->header = header;
    fb->statements = statements;
    fb->location = loc;
    fb->type_ref_pool = parser->pou_type_ref_pool;
    fb->var_ref_pool = parser->pou_var_ref_pool;
    parser->pou_type_ref_pool = NULL;
    parser->pou_var_ref_pool = NULL;
    
    DL_APPEND(parser->function_blocks, fb);
    DL_APPEND(parser->global_types, &(fb->type));

    return ESSTEE_OK;
    
error_free_resources:
    /* TODO: determine what to destroy */
    return ESSTEE_ERROR;
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
	ALLOC_OR_ERROR_JUMP(
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

    if(var_block)
    {	
	if(ST_FLAG_IS_SET(var_block->class, EXTERNAL_VAR_CLASS))
	{
	    struct variable_t *itr = NULL;
	    DL_FOREACH(var_block, itr)
	    {
		int ref_result = parser->global_var_ref_pool->add(
		    parser->global_var_ref_pool,
		    itr->identifier,
		    itr,
		    itr->identifier_location,
		    st_external_variable_resolved,
		    parser->errors);

		if(ref_result != ESSTEE_OK)
		{
		    goto error_free_resources;
		}
	    }
	}
	
	DL_CONCAT(header->variables, var_block);	
    }

    return header;

error_free_resources:
    /* TODO: clear named references in header */    
    // TODO: destroy vars block
    return NULL;
}
