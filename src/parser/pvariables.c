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
#include <util/bitflag.h>

#include <utlist.h>


struct variable_t * st_new_var_declaration_block(
    st_bitflag_t block_class,
    st_bitflag_t retain_flag,
    st_bitflag_t constant_flag,
    struct variable_t *variables,
    struct parser_t *parser)
{
    struct variable_t *itr = NULL;
    if(variables)
    {	
	DL_FOREACH(variables, itr)
	{
	    itr->class |= (block_class|retain_flag|constant_flag);
	}
    }

    return variables;
}

struct variable_t * st_append_var_declarations(
    struct variable_t *variable_group,
    struct variable_t *new_variables,
    struct parser_t *parser)
{
    DL_CONCAT(variable_group, new_variables);
    return variable_group;
}

struct variable_t * st_append_new_var(
    struct variable_t *variable_group,
    char *variable_name,
    const struct st_location_t *name_location,
    struct parser_t *parser)
{
    struct variable_t *v = NULL;
    ALLOC_OR_ERROR_JUMP(
	v,
	struct variable_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	v->identifier_location,
	name_location,
	parser->errors,
	error_free_resources);

    v->identifier = variable_name;
    v->type = NULL;
    v->value = NULL;
    v->address = NULL;

    DL_APPEND(variable_group, v);
    return variable_group;
    
error_free_resources:
    free(v);
    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
    return NULL;
}

struct variable_t * st_finalize_var_list(
    struct variable_t *var_list,
    struct type_iface_t *var_type,
    struct parser_t *parser)
{
    struct variable_t *itr = NULL;
    DL_FOREACH(var_list, itr)
    {
	itr->type = var_type;
    }

    return var_list;
}

struct variable_t * st_finalize_var_list_by_name(
    struct variable_t *var_list,
    char *type_name,
    const struct st_location_t *type_name_location,
    struct parser_t *parser)
{
    struct variable_t *itr = NULL;
    DL_FOREACH(var_list, itr)
    {
	if(parser->pou_type_ref_pool->add(
	       parser->pou_type_ref_pool,
	       type_name,
	       itr,
	       type_name_location,
	       st_variable_type_resolved,
	       parser->errors) != ESSTEE_OK)
	{
	    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	    return NULL;
	}
    }

    return var_list;
}

struct variable_t * st_finalize_var_list_by_edge(
    struct variable_t *var_list,
    char *type_name,
    const struct st_location_t *name_location,
    int rising_edge,
    struct parser_t *parser)
{
    parser->errors->new_issue_at(
	parser->errors,
	"edge variable specifiers not (yet) supported",
	ESSTEE_GENERAL_ERROR_ISSUE,
	1,
	name_location);

    parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
    return NULL;
}

int st_initialize_direct_memory(
    struct direct_address_t *address,
    char *init_type_name,
    const struct st_location_t *name_location,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    parser->errors->new_issue_at(
	parser->errors,
	"initialization of direct memory through VAR block not (yet) supported",
	ESSTEE_GENERAL_ERROR_ISSUE,
	1,
	location);

    parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
    return ESSTEE_ERROR;
}

int st_initialize_direct_memory_explicit(
    struct direct_address_t *address,
    char *init_type_name,
    const struct st_location_t *name_location,
    struct value_iface_t *value,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    parser->errors->new_issue_at(
	parser->errors,
	"initialization of direct memory through VAR block not (yet) supported",
	ESSTEE_GENERAL_ERROR_ISSUE,
	1,
	location);

    parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
    return ESSTEE_ERROR;
}

struct variable_t * st_new_direct_var(
    char *name,
    const struct st_location_t *name_location,
    const struct st_location_t *declaration_location,
    char *type_name,
    const struct st_location_t *type_name_location,
    struct direct_address_t *address,
    struct parser_t *parser)
{
    struct variable_t *v = NULL;
    struct st_location_t *loc = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	v,
	struct variable_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc,
	name_location,
	parser->errors,
	error_free_resources);

    v->identifier_location = loc;
    v->identifier = name;
    v->type = NULL;
    v->value = NULL;
    v->address = address;

    if(parser->pou_type_ref_pool->add(
	   parser->pou_type_ref_pool,
	   type_name,
	   v,
	   type_name_location,
	   st_variable_type_resolved,
	   parser->errors) != ESSTEE_OK)
    {
	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }

    if(parser->pou_type_ref_pool->add_post_resolve(
	   parser->pou_type_ref_pool,
	   v,
	   st_direct_variable_type_post_resolve,
	   parser->errors) != ESSTEE_OK)
    {
	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }
    
    /* Return a list to fit var_list format */
    struct variable_t *list = NULL;
    DL_APPEND(list, v);
    return list;
    
error_free_resources:
    free(v);
    free(loc);
    return NULL;
}

struct variable_t * st_new_direct_var_explicit(
    char *name,
    const struct st_location_t *name_location,
    const struct st_location_t *declaration_location,
    char *type_name,
    const struct st_location_t *type_name_location,
    struct direct_address_t *address,
    struct value_iface_t *default_value,
    const struct st_location_t *default_value_location,
    struct parser_t *parser)
{
    struct variable_t *v = NULL;
    struct st_location_t *loc = NULL;
    
    struct type_iface_t *dt = st_new_derived_type_by_name(NULL,
							  type_name,
							  type_name_location,
							  declaration_location,
							  default_value,
							  default_value_location,
							  parser);
    
    if(!dt)
    {
	goto error_free_resources;
    }

    ALLOC_OR_ERROR_JUMP(
	v,
	struct variable_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc,
	name_location,
	parser->errors,
	error_free_resources);

    v->identifier_location = loc;
    v->identifier = name;
    v->type = dt;
    v->value = NULL;
    v->address = address;

    if(parser->pou_type_ref_pool->add_post_resolve(
	   parser->pou_type_ref_pool,
	   v,
	   st_direct_variable_type_post_resolve,
	   parser->errors) != ESSTEE_OK)
    {
	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }
    
    struct variable_t *list = NULL;
    DL_APPEND(list, v);
    return list;
    
error_free_resources:
    /* TODO: check what to destroy */
    free(v);
    free(loc);
    return NULL;					  
}
