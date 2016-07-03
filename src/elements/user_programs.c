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

#include <elements/user_programs.h>
#include <linker/linker.h>
#include <statements/statements.h>
#include <util/macros.h>

#include <utlist.h>

#include <stdio.h>

/**************************************************************************/
/* Program interface                                                      */
/**************************************************************************/
struct user_program_t {
    struct program_iface_t program;

    struct header_t *header;    
    struct invoke_iface_t *statements;

    struct named_ref_pool_iface_t *type_refs;
    struct named_ref_pool_iface_t *var_refs;

    struct st_location_t *location;
    char *identifier;
};

static int user_program_finalize_header(
    struct program_iface_t *self,
    struct type_iface_t *global_type_table,
    struct variable_iface_t *global_var_table,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_program_t *p =
	CONTAINER_OF(self, struct user_program_t, program);

    int header_valid = st_create_header_tables(p->header, issues);
    if(header_valid != ESSTEE_OK)
    {
	return header_valid;
    }
    
    p->type_refs->reset_resolved(p->type_refs);
    p->var_refs->reset_resolved(p->var_refs);

    /* Resolve types */
    if(p->header && p->header->types)
    {
	st_resolve_type_refs(p->type_refs, p->header->types);
    }

    st_resolve_type_refs(p->type_refs, global_type_table);

    int callbacks_result = p->type_refs->trigger_resolve_callbacks(p->type_refs,
								   config,
								   issues);
    if(callbacks_result != ESSTEE_OK)
    {
	return callbacks_result;
    }

    /* Resolve variables */
    if(p->header && p->header->variables)
    {
	struct variable_iface_t *itr = NULL;
	DL_FOREACH(p->header->variables, itr)
	{
	    int create_result = itr->create(itr, config, issues);

	    if(create_result != ESSTEE_OK)
	    {
		return create_result;
	    }
	}
    }

    /* Resolve variable references */
    st_resolve_var_refs(p->var_refs, p->header->variables);

    return p->var_refs->trigger_resolve_callbacks(p->var_refs,
						   config,
						   issues);
}

static int user_program_finalize_statements(
    struct program_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_program_t *p =
	CONTAINER_OF(self, struct user_program_t, program);

    /* Allocate working memory for statements */
    int allocate_result = st_allocate_statements(p->statements,
						 issues);
    if(allocate_result != ESSTEE_OK)
    {
	return allocate_result;
    }

    return st_verify_statements(p->statements,
				config,
				issues);
}

static int user_program_start(
    struct program_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_program_t *p =
	CONTAINER_OF(self, struct user_program_t, program);

    struct variable_iface_t *itr = NULL;
    DL_FOREACH(p->header->variables, itr)
    {
	int reset_result = itr->reset(itr,
				      config,
				      issues);
	
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    cursor->switch_cycle_start(cursor,
			       p->statements,
			       config,
			       issues);
    
    return ESSTEE_OK;
}

static int user_program_run_cycle(
    struct program_iface_t *self,
    struct cursor_iface_t *cursor,
    struct systime_iface_t *systime,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_program_t *p =
	CONTAINER_OF(self, struct user_program_t, program);
    
    struct invoke_iface_t *first = p->statements;
    struct invoke_iface_t *current = NULL;
    
    do
    {
	current = cursor->step(cursor, systime, config, issues);

	if(!current)
	{
	    return ESSTEE_ERROR;
	}
    }
    while(current != first);

    return ESSTEE_OK;
}

static struct variable_iface_t * user_program_variable(
    struct program_iface_t *self,
    const char *variable_identifier,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_program_t *p =
	CONTAINER_OF(self, struct user_program_t, program);

    struct variable_iface_t *found = NULL;
    if(p->header && p->header->variables)
    {
	HASH_FIND_STR(p->header->variables, variable_identifier, found);
    }

    if(!found)
    {
	issues->new_issue(
	    issues,
	    "program has no variable named '%s'",
	    ESSTEE_CONTEXT_ERROR,
	    variable_identifier);
    }

    return found;
}

static int user_program_display(
    struct program_iface_t *self,
    char *output,
    size_t output_max_len,
    const struct config_iface_t *config)
{
    struct user_program_t *p =
	CONTAINER_OF(self, struct user_program_t, program);

    if(!(p->header && p->header->variables))
    {
	return 0;
    }

    struct variable_iface_t *itr = NULL;
    size_t writable_bytes_left = output_max_len;
    size_t written_bytes = 0;

    const char *format = "%s.%s";
    int insert_separator = 0;
    
    DL_FOREACH(p->header->variables, itr)
    {
	if(insert_separator)
	{
	    format = ";%s.%s";
	}
	else
	{
	    insert_separator = 1;
	}
	
	if(writable_bytes_left == 0)
	{
	    return ESSTEE_FALSE;
	}
	    
	int write_result = snprintf(
	    output+written_bytes,
	    writable_bytes_left,
	    format,
	    p->identifier,
	    itr->identifier);

	if(write_result < 0)
	{
	    return ESSTEE_ERROR;
	}

	written_bytes += write_result;
	writable_bytes_left = output_max_len - written_bytes;
    }

    return written_bytes;
}

static void user_program_destroy(
    struct program_iface_t *self)
{
    /* TODO: program destroy */
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct program_iface_t * st_new_user_program(
    char *identifier,
    const struct st_location_t *location,
    struct header_t *header,
    struct invoke_iface_t *statements,
    struct named_ref_pool_iface_t *type_refs,
    struct named_ref_pool_iface_t *var_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct user_program_t *p = NULL;
    struct st_location_t *p_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	p,
	struct user_program_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	p_location,
	location,
	issues,
	error_free_resources);

    p->identifier = identifier;
    p->location = p_location;
    p->header = header;
    p->statements = statements;
    p->type_refs = type_refs;
    p->var_refs = var_refs;

    memset(&(p->program), 0, sizeof(struct program_iface_t));
    p->program.identifier = p->identifier;
    p->program.location = p->location;
    
    p->program.finalize_header = user_program_finalize_header;
    p->program.finalize_statements = user_program_finalize_statements;
    p->program.start = user_program_start;
    p->program.run_cycle = user_program_run_cycle;
    p->program.variable = user_program_variable;
    p->program.display = user_program_display;
    p->program.destroy = user_program_destroy;
    
    return &(p->program);
    
error_free_resources:
    free(p);
    free(p_location);
    return NULL;
}
