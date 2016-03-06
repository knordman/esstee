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
#include <util/namedrefpool.h>	/* Implementation of namedreference_iface */
#include <util/macros.h>
    

int st_parser_reset(struct parser_t *parser)
{
    parser->global_types = NULL;
    parser->global_variables = NULL;
    parser->functions = NULL;
    parser->function_blocks = NULL;
    parser->programs = NULL;

    parser->direct_memory->reset(parser->direct_memory);
    
    parser->scanner_options.query_mode_start = 0;
    parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;

    parser->global_type_ref_pool = st_new_named_ref_pool();
    parser->global_var_ref_pool = st_new_named_ref_pool();
    parser->function_ref_pool = st_new_named_ref_pool();
    parser->pou_type_ref_pool = st_new_named_ref_pool();
    parser->pou_var_ref_pool = st_new_named_ref_pool();

    parser->loop_level = 0;
    
    if(!(parser->global_type_ref_pool
	 && parser->global_var_ref_pool
	 && parser->function_ref_pool
	 && parser->pou_type_ref_pool
	 && parser->pou_var_ref_pool))
    {
	return ESSTEE_ERROR;
    }
    
    return ESSTEE_OK;
}

int st_reset_parser_pou_refs(
    struct parser_t *parser)
{
    parser->pou_type_ref_pool = st_new_named_ref_pool();
    parser->pou_var_ref_pool = st_new_named_ref_pool();

    if(!(parser->pou_type_ref_pool && parser->pou_var_ref_pool))
    {
	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

void st_destroy_compilation_unit(
    struct compilation_unit_t *cu)
{
    /* TODO: destructor for compilation unit */
}

struct compilation_unit_t * st_parse_file(
    const char *path,
    struct parser_t *parser)
{
    FILE *fp = NULL;
    YY_BUFFER_STATE yy_buffer = NULL;
    struct compilation_unit_t *cu = NULL;
    char *source_path = NULL;
    
    fp = fopen(path, "r");
    if(!fp)
    {
	goto error_free_resources;
    }

    parser->scanner_options.query_mode_start = 0;
    yy_buffer = yy_create_buffer(fp, YY_BUF_SIZE, parser->yyscanner);
    if(!yy_buffer)
    {
	goto error_free_resources;
    }

    ALLOC_OR_ERROR_JUMP(
	cu,
	struct compilation_unit_t,
	parser->errors,
	error_free_resources);

    STRDUP_OR_ERROR_JUMP(
	source_path,
	path,
	parser->errors,
	error_free_resources);

    yy_switch_to_buffer(yy_buffer, parser->yyscanner);
    yyparse(parser->yyscanner, parser);

    if(parser->errors->new_error_occured(parser->errors) == ESSTEE_TRUE)
    {
	goto error_free_resources;
    }

    cu->source = source_path;
    cu->global_types = parser->global_types;
    cu->global_variables = parser->global_variables;
    cu->functions = parser->functions;
    cu->function_blocks = parser->function_blocks;
    cu->programs = parser->programs;

    cu->global_type_ref_pool = parser->global_type_ref_pool;
    cu->global_var_ref_pool = parser->global_var_ref_pool;
    cu->function_ref_pool = parser->function_ref_pool;

    parser->global_type_ref_pool = NULL;
    parser->global_var_ref_pool = NULL;
    parser->function_ref_pool = NULL;
    
    fclose(fp);
    yy_delete_buffer(yy_buffer, parser->yyscanner);

    return cu;
    
error_free_resources:
    if(fp)
    {
	fclose(fp);
    }
    yy_delete_buffer(yy_buffer, parser->yyscanner);
    free(cu);
    free(source_path);

    return NULL;
}
