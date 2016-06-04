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

#include <utlist.h>


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
    return NULL;
}

struct header_t * st_append_vars_to_header(
    struct header_t *header,
    struct variable_iface_t *var_block,
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
	if(ST_FLAG_IS_SET(var_block->class, GLOBAL_VAR_CLASS))
	{
	    DL_CONCAT(parser->global_variables, var_block);
	}
	else
	{
	    DL_CONCAT(header->variables, var_block);
	}
    }

    return header;

error_free_resources:
    /* TODO: clear named references in header */    
    // TODO: destroy vars block
    return NULL;
}
