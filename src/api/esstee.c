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
#include <elements/directmemory.h>
#include <util/macros.h>
#include <util/issue_context.h>
#include <util/config.h>
#include <api/elementnode.h>
#include <rt/cursor.h>
#include <rt/systime.h>
#include <elements/ifunction_block.h>
#include <elements/types.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <utlist.h>
#include <uthash.h>


struct st_t {
    struct compilation_unit_t *compilation_units;

    struct type_iface_t *elementary_types; /* List of elementary types */
    struct type_iface_t *global_types;	   /* Table of global types */
    struct variable_iface_t *global_variables; /* Table of global variables */
    struct function_iface_t *functions;	   /* Table of functions */
    struct program_iface_t *programs;	   /* Table of programs */
    struct program_iface_t *main;

    struct cursor_iface_t *cursor;

    struct issues_iface_t *errors;

    struct config_iface_t *config;
    struct dmem_iface_t *direct_memory;
    struct systime_iface_t *systime;

    struct element_node_context_t element_node_context;
    struct element_node_t *element_nodes;
    
    struct parser_t parser;

    int needs_linking;
};

static void reset_linking(
    struct st_t *st);

struct st_t * st_new_instance(
    size_t direct_memory_bytes)
{
    struct issues_iface_t *e = NULL;
    struct issues_iface_t *pe = NULL;
    struct cursor_iface_t *cur = NULL;
    struct type_iface_t *et = NULL;
    struct config_iface_t *c = NULL;
    struct dmem_iface_t *dm = NULL;
    struct st_t *st = NULL;
    struct systime_iface_t *s = NULL;
    
    ALLOC_OR_JUMP(
	st,
	struct st_t,
	error_free_resources);

    e     = st_new_issue_context();
    et    = st_new_elementary_types();
    c     = st_new_config();
    dm    = st_new_direct_memory(direct_memory_bytes);
    s     = st_new_systime();
    cur   = st_new_cursor();
    
    pe    = st_new_issue_context();
    
    if(!(e && et && c && dm && s && pe && cur))
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
    st->cursor = cur;
    st->errors = e;
    st->compilation_units = NULL;
    st->config = c;
    st->direct_memory = dm;
    st->systime = s;
    st->element_nodes = NULL;

    return st;

error_free_resources:
    /* TODO: determine what to destroy */
    st_destroy_types_in_list(et);

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

    /* int resolve_on_link_error = */
    /* 	st->config->get(st->config, "resolve_links_on_parse_error") == ESSTEE_TRUE; */
    
    /* Reset all link information */
    reset_linking(st);
    
    /* Link elementary and built-in types */
    st->global_types = st_link_types(st->elementary_types, st->global_types, st->errors);
    
    /* Link all resources */
    struct compilation_unit_t *cuitr = NULL;
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
	st->global_types = st_link_types(cuitr->global_types, st->global_types, st->errors);
	st->global_variables = st_link_variables(cuitr->global_variables, st->global_variables, st->errors);
	st->functions = st_link_functions(cuitr->functions, st->functions, st->errors);
	st->programs = st_link_programs(cuitr->programs, st->programs, st->errors);
    }

    if(st->errors->count(st->errors, ESSTEE_FILTER_ANY_ERROR) > 0)
    {
	return ESSTEE_ERROR;
    }
    
    /* Resolve global type references */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
	cuitr->global_type_ref_pool->reset_resolved(cuitr->global_type_ref_pool);
	st_resolve_type_refs(cuitr->global_type_ref_pool, st->global_types);
	cuitr->global_type_ref_pool->trigger_resolve_callbacks(cuitr->global_type_ref_pool,
							       st->config,
							       st->errors);
    }

    if(st->errors->count(st->errors, ESSTEE_FILTER_ANY_ERROR) > 0)
    {
	return ESSTEE_ERROR;
    }
    
    /* Resolve function block type references. References are resolved
     * separately, so that a function block's variables that have a
     * function block as type, may create the values of the variables
     * in the finalize header function. */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
        struct function_block_iface_t *fbitr = NULL;
        DL_FOREACH(cuitr->function_blocks, fbitr)
        {
            fbitr->resolve_header_type_references(fbitr,
						  st->global_types,
						  st->config,
						  st->errors);
        }
    }

    if(st->errors->count(st->errors, ESSTEE_FILTER_ANY_ERROR) > 0)
    {
	return ESSTEE_ERROR;
    }
    
    /* Create values of global variables */
    struct variable_iface_t *vitr = NULL;
    DL_FOREACH(st->global_variables, vitr)
    {
	vitr->create(vitr, st->config, st->errors);
    }

    if(st->errors->count(st->errors, ESSTEE_FILTER_ANY_ERROR) > 0)
    {
	return ESSTEE_ERROR;
    }
    
    /* Resolve global variable references */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
	cuitr->global_var_ref_pool->reset_resolved(cuitr->global_var_ref_pool);
	st_resolve_var_refs(cuitr->global_var_ref_pool, st->global_variables);
	cuitr->global_var_ref_pool->trigger_resolve_callbacks(cuitr->global_var_ref_pool,
							      st->config,
							      st->errors);
    }

    if(st->errors->count(st->errors, ESSTEE_FILTER_ANY_ERROR) > 0)
    {
	return ESSTEE_ERROR;
    }
    
    /* Finalize function block headers */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
        struct function_block_iface_t *fbitr = NULL;
        DL_FOREACH(cuitr->function_blocks, fbitr)
        {
            fbitr->finalize_header(fbitr,
                      st->global_variables,
                      st->config,
                      st->errors);
        }
    }

    /* Finalize function headers */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
        struct function_iface_t *fitr = NULL;
        DL_FOREACH(cuitr->functions, fitr)
        {
            fitr->finalize_header(fitr,
                      st->global_types,
                      st->global_variables,
                      st->config,
                      st->errors);
        }
    }

    /* Finalize program headers */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
        struct program_iface_t *pitr = NULL;
        DL_FOREACH(cuitr->programs, pitr)
        {
            pitr->finalize_header(pitr,
                      st->global_types,
                      st->global_variables,
                      st->config,
                      st->errors);
        }
    }

    if(st->errors->count(st->errors, ESSTEE_FILTER_ANY_ERROR) > 0)
    {
	return ESSTEE_ERROR;
    }
    
    /* Resolve function references (global) */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
	cuitr->function_ref_pool->reset_resolved(cuitr->function_ref_pool);
	st_resolve_function_refs(cuitr->function_ref_pool, st->functions);
	cuitr->function_ref_pool->trigger_resolve_callbacks(cuitr->function_ref_pool,
							    st->config,
							    st->errors);
    }

    if(st->errors->count(st->errors, ESSTEE_FILTER_ANY_ERROR) > 0)
    {
	return ESSTEE_ERROR;
    }
    
    /* Finalize function block statements */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
        struct function_block_iface_t *fbitr = NULL;
        DL_FOREACH(cuitr->function_blocks, fbitr)
        {
            fbitr->finalize_statements(fbitr,
				       st->config,
				       st->errors);
        }
    }

    /* Finalize function statements */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
        struct function_iface_t *fitr = NULL;
        DL_FOREACH(cuitr->functions, fitr)
        {
            fitr->finalize_statements(fitr,
				      st->config,
				      st->errors);
        }
    }

    /* Finalize program statements */
    for(cuitr = st->compilation_units; cuitr != NULL; cuitr = cuitr->hh.next)
    {
        struct program_iface_t *pitr = NULL;
        DL_FOREACH(cuitr->programs, pitr)
        {
            pitr->finalize_statements(pitr,
				      st->config,
				      st->errors);
        }
    }

    if(st->errors->count(st->errors, ESSTEE_FILTER_ANY_ERROR) > 0)
    {
	return ESSTEE_ERROR;
    }
    
    st->needs_linking = 0;
    return ESSTEE_OK;
}

const struct st_location_t * st_start(
    struct st_t *st,
    const char *program)
{
    struct program_iface_t *found = NULL;
    HASH_FIND_STR(st->programs, program, found);

    if(!found)
    {
	st->errors->new_issue(
	    st->errors,
	    "no program named '%s' defined",
	    ESSTEE_CONTEXT_ERROR,
	    program);

	return NULL;
    }

    /* Reset global variables */
    struct variable_iface_t *itr = NULL;
    DL_FOREACH(st->global_variables, itr)
    {
	int reset_result = itr->reset(itr,
				      st->config,
				      st->errors);
	
	if(reset_result != ESSTEE_OK)
	{
	    st->errors->internal_error(st->errors,
				       __FILE__,
				       __FUNCTION__,
				       __LINE__);

	    return NULL;
	}
    }

    /* Start program */
    int start_result = found->start(found,
				    st->cursor,
				    st->config,
				    st->errors);

    if(start_result != ESSTEE_OK)
    {
	return NULL;
    }

    st->main = found;
    
    return st->cursor->current_location(st->cursor);
}

const struct st_issue_t * st_fetch_issue(
    struct st_t *st,
    st_bitflag_t filter)
{
    return st->errors->fetch(st->errors, filter);
}

const struct st_issue_t * st_fetch_sub_issue(
    struct st_t *st,
    const struct st_issue_t *issue,
    st_bitflag_t filter)
{
    return st->errors->fetch_sub_issue(st->errors, issue, filter);
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

    struct queries_iface_t *queries = st_parse_query_string(query_string,
							    &(st->parser));
    if(!queries)
    {
	st->errors->merge(st->errors, st->parser.errors);
	return ESSTEE_ERROR;
    }
    
    int link_result = queries->link(queries,
				    st->global_variables,
				    st->functions,
				    st->programs,
				    st->config,
				    st->errors);

    if(link_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    int evaluate_result = queries->evaluate(queries,
					    st->config,
					    st->errors);

    if(evaluate_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    int display_result = queries->display(queries,
					  output,
					  output_max_len,
					  st->config,
					  st->errors);

    queries->destroy(queries);
    return display_result;
    
error_free_resources:
    queries->destroy(queries);
    return ESSTEE_ERROR;    
}

struct st_element_t * st_get_element(
    struct st_t *st,
    const char *identifier)
{
    return NULL;
}

int st_run_cycle(
    struct st_t *st,
    uint64_t ms)
{
    if(!st->main)
    {
	st->errors->new_issue(st->errors,
			      "no program started",
			      ESSTEE_CONTEXT_ERROR);

	return ESSTEE_ERROR;
    }

    int cycle_result = st->main->run_cycle(st->main,
					   st->cursor,
					   st->systime,
					   st->config,
					   st->errors);

    if(cycle_result != ESSTEE_OK)
    {
	return cycle_result;
    }
    
    st->systime->add_time_ms(st->systime, ms);
    
    return ESSTEE_OK;
}


const struct st_location_t * st_step(
    struct st_t *st)
{
    return NULL;
}

const struct st_location_t * st_step_in(
    struct st_t *st)
{
    return NULL;
}

const struct st_location_t * st_step_out(
    struct st_t *st)
{
    return NULL;
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
