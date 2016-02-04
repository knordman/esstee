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

#include <esstee/esstee.h>
#include <esstee/flags.h>
#include <parser/parser.h>
#include <linker/linker.h>
#include <elements/directmemory.h> /* Implementation of dmem_iface */
#include <util/macros.h>
#include <util/errorcontext.h>	/* Implementation of errors_iface */
#include <util/config.h>	/* Implementation of config_iface */
#include <rt/cursor.h>
#include <rt/systime.h>
#include <rt/runtime.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <utlist.h>
#include <uthash.h>


struct st_t {
    struct compilation_unit_t *compilation_units;

    struct type_iface_t *elementary_types; /* List of elementary types */
    struct type_iface_t *global_types;	   /* Table of global types */
    struct variable_t *global_variables;   /* Table of global variables */
    struct function_t *functions;	   /* Table of functions */
    struct program_t *programs;		   /* Table of programs */

    struct program_t *main;
    struct cursor_t cursor;

    struct errors_iface_t *errors;

    struct config_iface_t *config;
    struct dmem_iface_t *direct_memory;
    struct systime_iface_t *systime;
    
    struct parser_t parser;

    int needs_linking;
};

static void reset_linking(
    struct st_t *st);

struct st_t * st_new_instance(
    unsigned direct_memory_bytes)
{
    struct errors_iface_t *e = NULL, *pe = NULL;
    struct type_iface_t *et = NULL;
    struct config_iface_t *c = NULL;
    struct dmem_iface_t *dm = NULL;
    struct st_t *st = NULL;
    struct systime_iface_t *s = NULL;
    
    ALLOC_OR_JUMP(
	st,
	struct st_t,
	error_free_resources);

    e     = st_new_error_context();
    et    = st_new_elementary_types();
    c     = st_new_config();
    dm    = st_new_direct_memory(direct_memory_bytes);
    s     = st_new_systime();
    
    pe    = st_new_error_context();
    
    if(!(e && et && c && dm && s && pe))
    {
	goto error_free_resources;
    }

    yylex_init_extra(&(st->parser.scanner_options), &(st->parser.yyscanner));
    
    st->parser.errors = pe;
    st->parser.config = c;
    st->parser.direct_memory = dm;
    st->parser.global_type_ref_pool = NULL;
    st->parser.global_var_ref_pool = NULL;
    st->parser.function_ref_pool = NULL;
    st->parser.pou_type_ref_pool = NULL;
    st->parser.pou_var_ref_pool = NULL;
    
    st->elementary_types = et;
    st->global_types = NULL;
    st->global_variables = NULL;
    st->functions = NULL;
    st->programs = NULL;
    st->main = NULL;
    st->cursor.current = NULL;
    st->cursor.call_stack = NULL;
    st->errors = e;
    st->compilation_units = NULL;
    st->config = c;
    st->direct_memory = dm;
    st->systime = s;

    return st;

error_free_resources:
    st_destroy_error_context(e);
    st_destroy_error_context(pe);
    st_destroy_types_in_list(et);
    st_destroy_config(c);
    st_destroy_direct_memory(dm);

    return NULL;
}

int st_set_config(
    const char *option,
    int value,
    struct st_t *st)
{
    /* TODO: implement set config */
    return ESSTEE_FALSE;
}

int st_get_config(
    const char *option,
    struct st_t *st)
{
    /* TODO: implement get config */
    return ESSTEE_FALSE;
}

int st_load_file(struct st_t *st, const char *path)
{
    st_parser_reset(&(st->parser));
    struct compilation_unit_t *cu = st_parse_file(path, &(st->parser));

    if(!cu)
    {
	st->errors->merge(st->errors, st->parser.errors);
	return ESSTEE_ERROR;
    }

    struct compilation_unit_t *found = NULL;
    HASH_FIND_STR(st->compilation_units, path, found);

    if(found)
    {
	HASH_DEL(st->compilation_units, found);
	st_destroy_compilation_unit(found);
    }
    
    HASH_ADD_KEYPTR(
	hh, 
	st->compilation_units, 
	cu->source, 
	strlen(cu->source), 
	cu);

    st->needs_linking = 1;
    return ESSTEE_OK;
}

int st_load_buffer(
    const char *identifier, 
    const char *bytes, 
    size_t len, 
    struct st_t *st)
{
    /* TODO: implement loading of buffer */
    return ESSTEE_OK;
}

int st_link(struct st_t *st)
{
    if(!st->needs_linking)
    {
	return ESSTEE_OK;
    }

    int resolve_on_link_error =
	st->config->get(st->config, "resolve_links_on_parse_error") == ESSTEE_TRUE;
    
    /* Reset all link information */
    reset_linking(st);
    
    /* Link elementary types */
    st->global_types = st_link_types(st->elementary_types, st->global_types, st->errors);
    
    /* For each compilation unit ... */
    struct compilation_unit_t *cuitr = NULL;
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
	st->global_types = st_link_types(cuitr->global_types, st->global_types, st->errors);
	st->global_variables = st_link_variables(cuitr->global_variables, st->global_variables, st->errors);
	st->functions = st_link_functions(cuitr->functions, st->functions, st->errors);
	st->programs = st_link_programs(cuitr->programs, st->programs, st->errors);
	st_link_function_blocks(cuitr->function_blocks, st->errors);
    }

    if((st->errors->new_error_occured(st->errors) == ESSTEE_TRUE) && !resolve_on_link_error)
    {
	goto error_free_resources;
    }

    /* Resolve references ... */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
	cuitr->global_type_ref_pool->reset_resolved(cuitr->global_type_ref_pool);
	cuitr->global_var_ref_pool->reset_resolved(cuitr->global_var_ref_pool);
	cuitr->function_ref_pool->reset_resolved(cuitr->function_ref_pool);
	
	st_resolve_type_refs(cuitr->global_type_ref_pool, st->global_types);
	st_resolve_var_refs(cuitr->global_var_ref_pool, st->global_variables);
	st_resolve_function_refs(cuitr->function_ref_pool, st->functions);

	struct function_t *fitr = NULL;
	DL_FOREACH(cuitr->functions, fitr)
	{
	    fitr->type_ref_pool->reset_resolved(fitr->type_ref_pool);
	    fitr->var_ref_pool->reset_resolved(fitr->var_ref_pool);
	    
	    st_resolve_type_refs(fitr->type_ref_pool, st->global_types);
	    st_resolve_type_refs(fitr->type_ref_pool, fitr->header->types);
	    st_resolve_var_refs(fitr->var_ref_pool, fitr->header->variables);
	}

	struct function_block_t *fbitr = NULL;
	DL_FOREACH(cuitr->function_blocks, fbitr)
	{
	    fbitr->type_ref_pool->reset_resolved(fbitr->type_ref_pool);
	    fbitr->var_ref_pool->reset_resolved(fbitr->var_ref_pool);
	    
	    st_resolve_type_refs(fbitr->type_ref_pool, st->global_types);
	    st_resolve_type_refs(fbitr->type_ref_pool, fbitr->header->types);
	    st_resolve_var_refs(fbitr->var_ref_pool, fbitr->header->variables);
	}

	struct program_t *pitr = NULL;
	DL_FOREACH(cuitr->programs, pitr)
	{
	    pitr->type_ref_pool->reset_resolved(pitr->type_ref_pool);
	    pitr->var_ref_pool->reset_resolved(pitr->var_ref_pool);
	    
	    st_resolve_type_refs(pitr->type_ref_pool, st->global_types);
	    st_resolve_type_refs(pitr->type_ref_pool, pitr->header->types);
	    st_resolve_var_refs(pitr->var_ref_pool, pitr->header->variables);
	}
    }
    
    /* Call resolve callbacks ... */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
	cuitr->global_type_ref_pool->trigger_resolve_callbacks(
	    cuitr->global_type_ref_pool,
	    st->errors,
	    st->config);

	cuitr->global_var_ref_pool->trigger_resolve_callbacks(
	    cuitr->global_var_ref_pool,
	    st->errors,
	    st->config);

	cuitr->function_ref_pool->trigger_resolve_callbacks(
	    cuitr->function_ref_pool,
	    st->errors,
	    st->config);

	struct function_t *fitr = NULL;
	DL_FOREACH(cuitr->functions, fitr)
	{
	    fitr->type_ref_pool->trigger_resolve_callbacks(
		fitr->type_ref_pool,
		st->errors,
		st->config);

	    fitr->var_ref_pool->trigger_resolve_callbacks(
		fitr->var_ref_pool,
		st->errors,
		st->config);
	}

	struct function_block_t *fbitr = NULL;
	DL_FOREACH(cuitr->function_blocks, fbitr)
	{
	    fbitr->type_ref_pool->trigger_resolve_callbacks(
		fbitr->type_ref_pool,
		st->errors,
		st->config);

	    fbitr->var_ref_pool->trigger_resolve_callbacks(
		fbitr->var_ref_pool,
		st->errors,
		st->config);
	}

	struct program_t *pitr = NULL;
	DL_FOREACH(cuitr->programs, pitr)
	{
	    pitr->type_ref_pool->trigger_resolve_callbacks(
		pitr->type_ref_pool,
		st->errors,
		st->config);

	    pitr->var_ref_pool->trigger_resolve_callbacks(
		pitr->var_ref_pool,
		st->errors,
		st->config);
	}
    }

    if(st->errors->new_error_occured(st->errors) != ESSTEE_FALSE)
    {
	goto error_free_resources;
    }

    /* Create variable values */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
	struct variable_t *vitr = NULL;
	DL_FOREACH(cuitr->global_variables, vitr)
	{
	    if((vitr->value = vitr->type->create_value_of(vitr->type, st->config)) == NULL)
	    {
		goto error_free_resources;
	    }
	}

	struct function_t *fitr = NULL;
	DL_FOREACH(cuitr->functions, fitr)
	{
	    for(struct variable_t *vitr = fitr->header->variables;
		vitr != NULL;
		vitr = vitr->hh.next)
	    {
		if((vitr->value = vitr->type->create_value_of(vitr->type, st->config)) == NULL)
		{
		    goto error_free_resources;
		}
	    }
	}

	struct program_t *pitr = NULL;
	DL_FOREACH(cuitr->programs, pitr)
	{
	    struct variable_t *vitr = NULL;
	    DL_FOREACH(pitr->header->variables, vitr)
	    {
		if((vitr->value = vitr->type->create_value_of(vitr->type, st->config)) == NULL)
		{
		    goto error_free_resources;
		}
	    }
	}
    }
    
    /* Verify statements for functions, and programs (implicitly function blocks through variables) */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
	struct function_t *fitr = NULL;
	DL_FOREACH(cuitr->functions, fitr)
	{
	    st_verify_statements(fitr->statements, st->config, st->errors);
	}

	struct program_t *pitr = NULL;
	DL_FOREACH(cuitr->programs, pitr)
	{
	    st_verify_statements(pitr->statements, st->config, st->errors);
	}
    }

    if(st->errors->new_error_occured(st->errors) != ESSTEE_FALSE)
    {
	goto error_free_resources;
    }

    st->needs_linking = 0;
    return ESSTEE_OK;

error_free_resources:
    return ESSTEE_ERROR;
}

const struct st_location_t * st_start(
    struct st_t *st,
    const char *program)
{
    struct program_t *found = NULL;
    HASH_FIND_STR(st->programs, program, found);

    if(!found)
    {
	return NULL;
    }

    struct variable_t *vitr = NULL;
    for(vitr = st->global_variables; vitr != NULL; vitr = vitr->hh.next)
    {
	vitr->value->reset(vitr->value, st->config);
    }

    for(vitr = found->header->variables; vitr != NULL; vitr = vitr->hh.next)
    {
	vitr->value->reset(vitr->value, st->config);
    }
    
    st->main = found;

    st->cursor.current = st->main->statements;
    st->cursor.call_stack = NULL;
    
    return st->cursor.current->location(st->cursor.current);
}

const struct st_issue_t * st_next_issue(
    struct st_t *st,
    st_bitflag_t filter)
{
    return st->errors->next_issue(st->errors, filter);
}

static int display_main_program(
    struct program_t *main,
    char *output,
    size_t output_max_len,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    if(!(main->header && main->header->variables))
    {
	return 0;
    }

    struct variable_t *itr = NULL;
    size_t writable_bytes_left = output_max_len;
    size_t written_bytes = 0;

    const char *format = "%s.%s";
    int insert_separator = 0;
    
    DL_FOREACH(main->header->variables, itr)
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
	    main->identifier,
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

static int display_queries(
    struct query_t *queries,
    struct program_t *main,
    char *output,
    size_t output_max_len,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    size_t written_bytes = 0;
    int insert_separator = 0;
    
    struct query_t *itr = NULL;
    DL_FOREACH(queries, itr)
    {
	if(insert_separator)
	{
	    if(written_bytes+1 > output_max_len)
	    {
		errors->new_issue(
		    errors,
		    "output buffer full, output truncated",
		    ISSUE_WARNING_CLASS);

		return ESSTEE_ERROR;
	    }
	    else
	    {
		output[written_bytes] = ';';
		written_bytes++;
	    }
	}
	else
	{
	    insert_separator = 1;
	}

	int query_written_bytes = 0;
	if(!itr->qi->target)
	{
	    query_written_bytes = display_main_program(
		main,
		output+written_bytes,
		output_max_len-written_bytes,
		config,
		errors);
	}
	else
	{
	    query_written_bytes = itr->qi->target->display(
		itr->qi->target,
		output+written_bytes,
		output_max_len-written_bytes,
		config);
	}
	
	if(query_written_bytes == ESSTEE_ERROR)
	{
	    errors->new_issue(
		errors,
		"output buffer write error occurred",
		ISSUE_WARNING_CLASS);

	    return ESSTEE_ERROR;
	}
	else if(query_written_bytes == ESSTEE_FALSE)
	{
	    errors->new_issue(
		errors,
		"output buffer full, output truncated",
		ISSUE_WARNING_CLASS);

	    return ESSTEE_ERROR;
	}

	written_bytes += query_written_bytes;
    }

    if(written_bytes+1 > output_max_len)
    {
    	errors->new_issue(
    	    errors,
    	    "output buffer full, output truncated",
    	    ISSUE_WARNING_CLASS);

    	return ESSTEE_ERROR;
    }
    else
    {
    	output[written_bytes] = '\0';
    }
    
    return ESSTEE_OK;
}

int st_query(
    struct st_t *st,
    char *output,
    size_t output_max_len,
    const char *query_string)
{
    if(output_max_len > 0)
    {
	output[0] = '\0';
    }
    else
    {
	return ESSTEE_ERROR;
    }

    if(!st->main)
    {
	st->errors->new_issue(
	    st->errors,
	    "no program started",
	    ISSUE_WARNING_CLASS);

	return ESSTEE_ERROR;
    }
    
    /* Parse the queries */
    if(st_parser_reset(&(st->parser)) == ESSTEE_ERROR)
    {
	return ESSTEE_ERROR;
    }

    st->parser.scanner_options.query_mode_start = 1;
    YY_BUFFER_STATE yy_buffer = yy_scan_string(query_string, st->parser.yyscanner);
    yy_switch_to_buffer(yy_buffer, st->parser.yyscanner);
    yyparse(st->parser.yyscanner, &(st->parser));

    if(st->parser.errors->new_error_occured(st->parser.errors) == ESSTEE_TRUE)
    {
	st->errors->merge(st->errors, st->parser.errors);
	goto error_free_resources;
    }

    /* Link the querues */
    int link_result = st_link_queries(
	st->parser.queries,
	st->global_variables,
	st->functions,
	st->parser.pou_var_ref_pool,
	st->parser.function_ref_pool,
	st->main,
	st->config,
	st->errors);

    if(link_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    /* Evaluate the query */
    int evaluate_result = st_evaluate_queries(
	st->parser.queries,
	st->config,
	st->errors);

    if(evaluate_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    int display_result = display_queries(
	st->parser.queries,
	st->main,
	output,
	output_max_len,
	st->config,
	st->errors);

    if(display_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    st_destroy_queries(st->parser.queries);
    st->parser.queries = NULL;
    
    return ESSTEE_OK;
    
error_free_resources:
    st_destroy_queries(st->parser.queries);
    st->parser.queries = NULL;
    return ESSTEE_ERROR;
}

int st_run_cycle(
    struct st_t *st,
    uint64_t ms)
{
    struct invoke_iface_t *first = st->main->statements;

    int result = ESSTEE_OK;
    do
    {
	if(st_step(st) == NULL)
	{
	    result = ESSTEE_ERROR;
	    break;
	}
    }
    while(st->cursor.current != first);

    st->systime->add_time_ms(st->systime, ms);
    
    return result;
}

static void set_cursor_to_next(
    struct invoke_iface_t *current,
    struct st_t *st)
{
    if(current->next != NULL)
    {
	st->cursor.current = current->next;
    }
    else
    {
	if(st->cursor.call_stack == NULL)
	{
	    /* Cycle complete */
	    st->cursor.current = st->main->statements;
	    st->cursor.call_stack = NULL;
	}
	else
	{
	    /* Step out */
	    st->cursor.current = st->cursor.call_stack;
	    /* Pop the call stack */
	    DL_DELETE2(
		st->cursor.call_stack,
		st->cursor.call_stack,
		call_stack_prev,
		call_stack_next);
	}
    }
}

const struct st_location_t * st_step(
    struct st_t *st)
{
    struct invoke_iface_t *start = st->cursor.current;
    
    int invoke_result = INVOKE_RESULT_IN_PROGRESS;
    do
    {
	invoke_result = st->cursor.current->step(
	    st->cursor.current,
	    &(st->cursor),
	    st->systime,
	    st->config,
	    st->errors);

	if(invoke_result == INVOKE_RESULT_ERROR)
	{
	    return NULL;
	}
    }
    while(invoke_result != INVOKE_RESULT_FINISHED);
    
    set_cursor_to_next(start, st);

    return st->cursor.current->location(st->cursor.current);
}

const struct st_location_t * st_step_in(
    struct st_t *st)
{
    struct invoke_iface_t *start = st->cursor.current;
    
    int invoke_result = invoke_result = st->cursor.current->step(
	st->cursor.current,
	&(st->cursor),
	st->systime,
	st->config,
	st->errors);

    if(invoke_result == INVOKE_RESULT_ERROR)
    {
	return NULL;
    }
    else if(invoke_result == INVOKE_RESULT_FINISHED)
    {
	set_cursor_to_next(start, st);
    }
    
    return st->cursor.current->location(st->cursor.current);
}

const struct st_location_t * st_step_out(
    struct st_t *st)
{
    struct invoke_iface_t *start = st->cursor.current;
    struct invoke_iface_t *call_stack = st->cursor.call_stack;
    
    if(call_stack == NULL)
    {
	return st_step(st);
    }
    else
    {
	do
	{
	    int invoke_result = st->cursor.current->step(
		st->cursor.current,
		&(st->cursor),
		st->systime,
		st->config,
		st->errors);

	    if(invoke_result == INVOKE_RESULT_ERROR)
	    {
		return NULL;
	    }
	    else if(invoke_result == INVOKE_RESULT_FINISHED)
	    {
		set_cursor_to_next(start, st);
		start = start->next;
	    }
	}
	while(st->cursor.current != call_stack);
    }

    return st->cursor.current->location(st->cursor.current);
}

int st_step_time(
    struct st_t *st,
    uint64_t ms)
{
    st->systime->add_time_ms(st->systime, ms);
    return ESSTEE_OK;
}


void st_destroy(struct st_t *st)
{
    /* TODO: Destructor for st instance */
}

static void reset_linking(
    struct st_t *st)
{
    /* TODO: Functionality, resetting linking prior to link operation */
}
