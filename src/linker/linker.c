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

#include <linker/linker.h>
#include <esstee/flags.h>

#include <utlist.h>
#include <stdio.h>

static int create_header_tables(
    struct header_t *header,
    struct issues_iface_t *issues)
{
    int result = ESSTEE_OK;
    
    /* Make hash table from type list */
    int issues_at_type_start = issues->count(issues, ESSTEE_FILTER_ANY_ERROR);
    struct type_iface_t *type_table =
	st_link_types(header->types, NULL, issues);

    if(issues->count(issues, ESSTEE_FILTER_ANY_ERROR) == issues_at_type_start)
    {
	header->types = type_table;
    }
    else
    {
	result = ESSTEE_ERROR;
    }

    /* Make hash table from var list */
    int issues_at_var_start = issues->count(issues, ESSTEE_FILTER_ANY_ERROR);
    struct variable_t *variable_table =
	st_link_variables(header->variables, NULL, issues);

    if(issues->count(issues, ESSTEE_FILTER_ANY_ERROR) == issues_at_var_start)
    {
	header->variables = variable_table;
    }
    else
    {
	result = ESSTEE_ERROR;
    }

    return result;
}

int st_create_header_tables(
    struct header_t *header,
    struct issues_iface_t *issues)
{
    return create_header_tables(header, issues);
}

int st_link_queries(
    struct query_t *queries,
    struct variable_t *global_variables,
    struct function_iface_t *functions,
    struct named_ref_pool_iface_t *var_ref_pool,
    struct named_ref_pool_iface_t *func_ref_pool,
    struct program_t *main,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    /* Resolve references */
    const char *resolve = NULL;
    while((resolve = var_ref_pool->next_unresolved(var_ref_pool)) != NULL)
    {
	struct variable_t *var_found = NULL;
	HASH_FIND_STR(global_variables, resolve, var_found);
	if(var_found != NULL)
	{
	    var_ref_pool->resolve(var_ref_pool, resolve, var_found);
	}
	else
	{
	    if(strcmp(main->identifier, resolve) == 0)
	    {
		var_ref_pool->resolve_with_remark(
		    var_ref_pool,
		    resolve,
		    main,
		    1);
	    }
	}
    }

    /* Resolve function references, might be used in new value
     * expression */
    st_resolve_function_refs(func_ref_pool, functions);

    /* Do resolve callbacks */
    var_ref_pool->trigger_resolve_callbacks(var_ref_pool, config, issues);
    func_ref_pool->trigger_resolve_callbacks(func_ref_pool, config, issues);

    if(issues->count(issues, ESSTEE_FILTER_ANY_ERROR) > 0)
    {
	return ESSTEE_ERROR;
    }

    struct query_t *itr = NULL;
    DL_FOREACH(queries, itr)
    {
	if(!itr->qi->runtime_constant_reference)
	{
	    issues->new_issue_at(
		issues,
		"array indices must be runtime constant in query mode",
		1,
		ISSUE_ERROR_CLASS,
		itr->qi->last->location);

	    return ESSTEE_ERROR;
	}
	
	/* Verify the qualified identifer that is displayed/altered */
	int chain_resolve = st_qualified_identifier_resolve_chain(itr->qi,
								  config,
								  issues);
	if(chain_resolve != ESSTEE_OK)
	{
	    return chain_resolve;
	}

	int array_index_resolve = st_qualified_identifier_resolve_array_index(
	    itr->qi,
	    config,
	    issues);
	if(array_index_resolve != ESSTEE_OK)
	{
	    return array_index_resolve;
	}
	    
	/* Verify the new value expression and its compatibility */
	if(itr->new_value)
	{
	    /* Check that we have target */
	    if(!itr->qi->target)
	    {
		issues->new_issue_at(
		    issues,
		    "cannot assign a value to a program",
		    1,
		    ISSUE_ERROR_CLASS,
		    itr->qi->location);

		return ESSTEE_ERROR;
	    }
	
	    if(itr->new_value->invoke.verify)
	    {
		int verify_result =
		    itr->new_value->invoke.verify(&(itr->new_value->invoke),
						  config,
						  issues);
		if(verify_result != ESSTEE_OK)
		{
		    return verify_result;
		}
	    }
	    
	    if(!itr->qi->target->assignable_from)
	    {
		const char *message = issues->build_message(
		    issues,
		    "element '%s' cannot be assigned a new value",
		    itr->qi->last->identifier);
		
		issues->new_issue_at(
		    issues,
		    message,
		    1,
		    ISSUE_ERROR_CLASS,
		    itr->qi->location);

		return ESSTEE_ERROR;
	    }

	    const struct value_iface_t *assign_value =
		itr->new_value->return_value(itr->new_value);

	    int assignable_from = itr->qi->target->assignable_from(itr->qi->target,
								   assign_value,
								   config,
								   issues);
	    if(assignable_from != ESSTEE_TRUE)
	    {
		return ESSTEE_ERROR;
	    }
	}
    }
    
    return ESSTEE_OK;
}

struct type_iface_t * st_link_types(
    struct type_iface_t *type_list,
    struct type_iface_t *type_table,
    struct issues_iface_t *issues)
{
    struct type_iface_t *itr = NULL, *found = NULL;
    DL_FOREACH(type_list, itr)
    {
	HASH_FIND_STR(type_table, itr->identifier, found);
	if(found != NULL)
	{
	    const char *message = issues->build_message(
		issues,
		"duplicate definition of type '%s'",
		itr->identifier);
	    
	    issues->new_issue_at(
		issues,
		message,
		ESSTEE_LINK_ERROR,
		2,
		itr->location(itr),
		found->location(found));
	}
	else
	{
	    HASH_ADD_KEYPTR(
		hh, 
		type_table, 
		itr->identifier, 
		strlen(itr->identifier), 
		itr);
	}
    }

    return type_table;
}

struct variable_t * st_link_variables(
    struct variable_t *variable_list,
    struct variable_t *variable_table,
    struct issues_iface_t *issues)
{
    struct variable_t *itr = NULL, *found = NULL;
    DL_FOREACH(variable_list, itr)
    {
	HASH_FIND_STR(variable_table, itr->identifier, found);
	if(found != NULL)
	{
	    const char *message = issues->build_message(
		issues,
		"duplicate definition of variable '%s'",
		itr->identifier);

	    issues->new_issue_at(
		issues,
		message,
		ISSUE_ERROR_CLASS,
		2,
		itr->identifier_location,
		found->identifier_location);
	}
	else
	{
	    HASH_ADD_KEYPTR(
		hh, 
		variable_table, 
		itr->identifier, 
		strlen(itr->identifier), 
		itr);
	}
    }

    return variable_table;
}

struct function_iface_t * st_link_functions(
    struct function_iface_t *function_list,
    struct function_iface_t *function_table,
    struct issues_iface_t *issues)
{
    struct function_iface_t *itr = NULL, *found = NULL;
    DL_FOREACH(function_list, itr)
    {
	HASH_FIND_STR(function_table, itr->identifier, found);
	if(found != NULL)
	{
	    const char *message = issues->build_message(
		issues,
		"duplicate definition of function '%s'",
		itr->identifier);
	    
	    issues->new_issue_at(
		issues,
		message,
		ESSTEE_LINK_ERROR,
		2,
		itr->location,
		found->location);
	    
	    continue;
	}
    }

    return function_table;
}

struct program_t * st_link_programs(
    struct program_t *program_list,
    struct program_t *program_table,
    struct issues_iface_t *issues)
{
    struct program_t *itr = NULL, *found = NULL;
    DL_FOREACH(program_list, itr)
    {
	HASH_FIND_STR(program_table, itr->identifier, found);
	if(found != NULL)
	{
	    const char *message = issues->build_message(
		issues,
		"duplicate definition of program '%s'",
		itr->identifier);
	    
	    issues->new_issue_at(
		issues,
		message,
		ESSTEE_LINK_ERROR,
		2,
		itr->location,
		found->location);
	}
	else
	{
	    create_header_tables(itr->header, issues);
	    
	    HASH_ADD_KEYPTR(
		hh, 
		program_table, 
		itr->identifier, 
		strlen(itr->identifier), 
		itr);
	}
    }

    return program_table;
}

int st_link_function_blocks(
    struct function_block_t *function_blocks,
    struct issues_iface_t *issues)
{
    int result = ESSTEE_OK;
    struct function_block_t *itr = NULL;
    DL_FOREACH(function_blocks, itr)
    {
	if(create_header_tables(itr->header, issues) == ESSTEE_ERROR)
	{
	    result = ESSTEE_ERROR;
	}
    }

    return result;
}

void st_resolve_type_refs(
    struct named_ref_pool_iface_t *type_ref_pool,
    struct type_iface_t *type_table)
{
    const char *resolve = NULL;
    while((resolve = type_ref_pool->next_unresolved(type_ref_pool)) != NULL)
    {
	struct type_iface_t *found = NULL;
	HASH_FIND_STR(type_table, resolve, found);
	if(found != NULL)
	{
	    type_ref_pool->resolve(type_ref_pool, resolve, found);
	}
    }
}

int st_create_header_variable_values(
    struct variable_t *var_table,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
/* In the special case of an EXTERNAL variable, no value should be
 * created, since the variable is a link to a global variable. In that
 * case, check that that classes of the two variables match. This is
 * not a perfect way to catch type differences, e.g. two different
 * arrays will be accepted. */

    int result = ESSTEE_OK;
    struct variable_t *itr = NULL;
    DL_FOREACH(var_table, itr)
    {
	if(ST_FLAG_IS_SET(itr->class, EXTERNAL_VAR_CLASS))
	{
	    /* Link will be NULL if global reference failed  */
	    if(itr->external_alias)
	    {
		st_bitflag_t global_type_class = 
		    itr->external_alias->type->class(itr->external_alias->type);

		st_bitflag_t local_type_class = 
		    itr->type->class(itr->type);

		if(global_type_class != local_type_class)
		{
		    const char *message = issues->build_message(
			issues,
			"type of the external variable '%s' does not match the type of global variable '%s'",
			itr->identifier);
			
		    issues->new_issue_at(issues,
					 message,
					 ESSTEE_TYPE_ERROR,
					 2,
					 itr->identifier_location,
					 itr->external_alias->identifier_location);

		    result = ESSTEE_ERROR;
		}
	    }
	}
	else
	{
	    itr->value = itr->type->create_value_of(itr->type, config, issues);

	    if(!itr->value)
	    {
		return ESSTEE_ERROR;
	    }
	}
    }

    return result;
}

void st_resolve_pou_var_refs(
    struct named_ref_pool_iface_t *var_refs,
    struct variable_t *global_var_table,
    struct variable_t *var_table)
{
    const char *resolve = NULL;
    while((resolve = var_refs->next_unresolved(var_refs)) != NULL)
    {
	struct variable_t *found = NULL;
	HASH_FIND_STR(var_table, resolve, found);
	if(found)
	{
	    if(ST_FLAG_IS_SET(found->class, EXTERNAL_VAR_CLASS))
	    {
		var_refs->resolve(var_refs,
				  resolve,
				  found->external_alias);
	    }
	    else
	    {
		var_refs->resolve(var_refs,
				  resolve,
				  found);
	    }
	}
    }
}

void st_resolve_var_refs(
    struct named_ref_pool_iface_t *var_ref_pool,
    struct variable_t *var_table)
{
    const char *resolve = NULL;
    while((resolve = var_ref_pool->next_unresolved(var_ref_pool)) != NULL)
    {
	struct variable_t *found = NULL;
	HASH_FIND_STR(var_table, resolve, found);
	if(found != NULL)
	{
	    var_ref_pool->resolve(var_ref_pool, resolve, found);
	}
    }
}

int st_resolve_function_refs(
    struct named_ref_pool_iface_t *function_ref_pool,
    struct function_iface_t *function_table)
{
    int result = ESSTEE_OK;
    const char *resolve = NULL;
    while((resolve = function_ref_pool->next_unresolved(function_ref_pool)) != NULL)
    {
	struct function_iface_t *found = NULL;
	HASH_FIND_STR(function_table, resolve, found);
	if(found != NULL)
	{
	    function_ref_pool->resolve(function_ref_pool, resolve, found);
	}
    }

    return result;
}

int st_allocate_statements(
    struct invoke_iface_t *statements,
    struct issues_iface_t *issues)
{
    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(statements, itr)
    {
	if(!itr->allocate)
	{
	    continue;
	}
	
	if(itr->allocate(itr, issues) != ESSTEE_OK)
	{
	    return ESSTEE_ERROR;
	}
    }

    return ESSTEE_OK;
}


int st_verify_statements(
    struct invoke_iface_t *statements,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    int result = ESSTEE_OK;
    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(statements, itr)
    {
	if(itr->verify(itr, config, issues) != ESSTEE_OK)
	{
	    result = ESSTEE_ERROR;
	}
    }

    return result;
}
