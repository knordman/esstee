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
#include <elements/user_functions.h>
#include <elements/user_function_blocks.h>
#include <elements/user_programs.h>
#include <statements/statements.h>

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
	free(return_type_identifier);
	st_destroy_header(header);
	st_destroy_statements(statements);
    }
    else
    {
	parser->pou_type_ref_pool = NULL;
	parser->pou_var_ref_pool = NULL;
    }

    if(function)
    {
	/* TODO: commit of references */
	DL_APPEND(parser->functions, function);
	return ESSTEE_OK;
    }

    /* TODO: check clearing of references */
    return ESSTEE_ERROR;
}

int st_new_function_block_pou(
    char *identifier,
    const struct st_location_t *location,
    struct header_t *header,
    struct invoke_iface_t *statements,
    struct parser_t *parser)
{
    struct type_iface_t *fb = st_new_user_function_block(
	identifier,
	location,
	header,
	statements,
	parser->pou_type_ref_pool,
	parser->pou_var_ref_pool,
	parser->config,
	parser->errors);

    if(!fb)
    {
	free(identifier);
	st_destroy_header(header);
	st_destroy_statements(statements);
    }
    else
    {
	parser->pou_type_ref_pool = NULL;
	parser->pou_var_ref_pool = NULL;
    }

    if(fb)
    {
	DL_APPEND(parser->global_types, fb);
	DL_APPEND(parser->function_blocks, fb->function_block_handle(fb));
	return ESSTEE_OK;
    }

    return ESSTEE_ERROR;
}


int st_new_program_pou(
    char *identifier,
    const struct st_location_t *location,
    struct header_t *header,
    struct invoke_iface_t *statements,
    struct parser_t *parser)
{
    struct program_iface_t *program = st_new_user_program(
	identifier,
	location,
	header,
	statements,
	parser->pou_type_ref_pool,
	parser->pou_var_ref_pool,
	parser->config,
	parser->errors);

    if(!program)
    {
	free(identifier);
	st_destroy_header(header);
	st_destroy_statements(statements);
    }
    else
    {
	parser->pou_type_ref_pool = NULL;
	parser->pou_var_ref_pool = NULL;
    }

    if(st_reset_parser_next_pou(parser) == ESSTEE_OK)
    {
	if(program)
	{
	    DL_APPEND(parser->programs, program);
	    return ESSTEE_OK;
	}
    }

    return ESSTEE_ERROR;
}

int st_new_type_block_pou(
    struct type_iface_t *types,
    struct parser_t *parser)
{
    DL_CONCAT(parser->global_types, types);

    parser->global_type_ref_pool->merge(parser->global_type_ref_pool,
					parser->pou_type_ref_pool);
    
    return st_reset_parser_next_pou(parser);
}

int st_new_var_block_pou(
    struct variable_iface_t *variables,
    struct parser_t *parser)
{   
    DL_CONCAT(parser->global_variables, variables);

    parser->global_type_ref_pool->merge(parser->global_type_ref_pool,
					parser->pou_type_ref_pool);
    
    return st_reset_parser_next_pou(parser);
}
