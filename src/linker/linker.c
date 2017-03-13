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


struct type_iface_t * st_link_types(
    struct type_iface_t *type_list,
    struct type_iface_t *type_table,
    struct issues_iface_t *issues)
{
    struct type_iface_t *itr = NULL, *found = NULL;
    DL_FOREACH(type_list, itr)
    {
	HASH_FIND_STR(type_table, itr->identifier, found);
	if(found != NULL && issues)
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
		itr->location,
		found->location);
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

struct variable_iface_t * st_link_variables(
    struct variable_iface_t *variable_list,
    struct variable_iface_t *variable_table,
    struct issues_iface_t *issues)
{
    struct variable_iface_t *itr = NULL, *found = NULL;
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
		ESSTEE_LINK_ERROR,
		2,
		itr->location,
		found->location);
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
	}
	else
	{
	    HASH_ADD_KEYPTR(
		hh, 
		function_table, 
		itr->identifier, 
		strlen(itr->identifier), 
		itr);
	}

    }

    return function_table;
}

struct program_iface_t * st_link_programs(
    struct program_iface_t *program_list,
    struct program_iface_t *program_table,
    struct issues_iface_t *issues)
{
    struct program_iface_t *itr = NULL, *found = NULL;
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

void st_resolve_var_refs(
    struct named_ref_pool_iface_t *var_ref_pool,
    struct variable_iface_t *var_table)
{
    const char *resolve = NULL;
    while((resolve = var_ref_pool->next_unresolved(var_ref_pool)) != NULL)
    {
	struct variable_iface_t *found = NULL;
	HASH_FIND_STR(var_table, resolve, found);
	if(found)
	{
	    var_ref_pool->resolve(var_ref_pool, resolve, found);
	}
    }
}

void st_resolve_program_refs(
    struct named_ref_pool_iface_t *prgm_ref_pool,
    struct program_iface_t *prgm_table)
{
    const char *resolve = NULL;
    while((resolve = prgm_ref_pool->next_unresolved(prgm_ref_pool)) != NULL)
    {
	struct program_iface_t *found = NULL;
	HASH_FIND_STR(prgm_table, resolve, found);
	if(found)
	{
	    prgm_ref_pool->resolve(prgm_ref_pool, resolve, found);
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
	if(found)
	{
	    function_ref_pool->resolve(function_ref_pool, resolve, found);
	}
    }

    return result;
}

