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
#include <elements/variable.h>


struct variable_iface_t * st_new_var_declaration_block(
    st_bitflag_t block_class,
    st_bitflag_t retain_flag,
    st_bitflag_t constant_flag,
    struct variable_stub_t *stubs,
    struct parser_t *parser)
{
    struct variable_iface_t *vars = st_create_variable_block(
	block_class,
	retain_flag,
	constant_flag,
	stubs,
	parser->global_var_ref_pool,
	parser->pou_type_ref_pool,
	parser->config,
	parser->errors);

    if(!vars)
    {
	st_destroy_variable_stubs(stubs);
    }

    return vars;
}

struct variable_stub_t * st_append_var_declarations(
    struct variable_stub_t *variable_group,
    struct variable_stub_t *new_variables,
    struct parser_t *parser)
{
    struct variable_stub_t *merged = st_concatenate_variable_stubs(
	variable_group,
	new_variables,
	parser->config,
	parser->errors);

    if(!merged)
    {
	st_destroy_variable_stubs(variable_group);
	st_destroy_variable_stubs(new_variables);
    }

    return merged;
}

struct variable_stub_t * st_append_new_var(
    struct variable_stub_t *variable_group,
    char *variable_name,
    const struct st_location_t *name_location,
    struct parser_t *parser)
{
    struct variable_stub_t *extended = st_extend_variable_stubs(
	variable_group,
	variable_name,
	name_location,
	parser->config,
	parser->errors);

    if(!extended)
    {
	st_destroy_variable_stubs(variable_group);
    }

    return extended;
}

struct variable_stub_t * st_finalize_var_list(
    struct variable_stub_t *var_list,
    struct type_iface_t *var_type,
    struct parser_t *parser)
{
    struct variable_stub_t *vars = st_set_variable_stubs_type(
	var_list,
	var_type,
	parser->config,
	parser->errors);

    if(!vars)
    {
	st_destroy_variable_stubs(vars);
    }

    return vars;
}

struct variable_stub_t * st_finalize_var_list_by_name(
    struct variable_stub_t *var_list,
    char *type_name,
    const struct st_location_t *type_name_location,
    struct parser_t *parser)
{
    struct variable_stub_t *vars = st_set_variable_stubs_type_name(
	var_list,
	type_name,
	parser->config,
	parser->errors);

    if(!vars)
    {
	st_destroy_variable_stubs(vars);
    }

    return vars;
}

struct variable_stub_t * st_finalize_var_list_by_edge(
    struct variable_stub_t *var_list,
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

    return ESSTEE_ERROR;
}

struct variable_stub_t * st_new_direct_var(
    char *name,
    const struct st_location_t *name_location,
    const struct st_location_t *declaration_location,
    char *type_name,
    const struct st_location_t *type_name_location,
    struct direct_address_t *address,
    struct parser_t *parser)
{
    struct variable_stub_t *var = st_create_direct_variable_stub(
	name,
	address,
	type_name,
	type_name_location,
	name_location,
	NULL,
	NULL,
	parser->pou_type_ref_pool,
	parser->config,
	parser->errors);

    if(!var)
    {
	free(name);
	free(type_name);
	free(address);
    }

    return var;
}

struct variable_stub_t * st_new_direct_var_explicit(
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
    struct variable_stub_t *var = st_create_direct_variable_stub(
	name,
	address,
	type_name,
	type_name_location,
	name_location,
	NULL,
	NULL,
	parser->pou_type_ref_pool,
	parser->config,
	parser->errors);

    if(!var)
    {
	free(name);
	free(address);
	free(type_name);
	default_value->destroy(default_value);
    }
	
    return var;
}
