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

#include <elements/queries.h>
#include <util/macros.h>
#include <linker/linker.h>

#include <utlist.h>

/**************************************************************************/
/* Query interface                                                        */
/**************************************************************************/
struct query_t {
    char *program_identifier;
    char *identifier;

    struct program_iface_t *program;

    struct variable_iface_t *variable;
    struct qualified_identifier_iface_t *qid;
    struct expression_iface_t *assignment;
    
    struct query_t *prev;
    struct query_t *next;
};

struct queries_t {
    struct queries_iface_t queries;

    struct named_ref_pool_iface_t *var_refs;
    struct named_ref_pool_iface_t *function_refs;
    struct named_ref_pool_iface_t *program_refs;
    
    struct query_t *entries;
};

static int queries_append(
    struct queries_iface_t *self,
    struct query_t *query,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct queries_t *query_list =
	CONTAINER_OF(self, struct queries_t, queries);

    DL_APPEND(query_list->entries, query);

    return ESSTEE_OK;
}

static int queries_finish(
    	struct queries_iface_t *self,
	struct named_ref_pool_iface_t *var_refs,
	struct named_ref_pool_iface_t *function_refs,
	struct named_ref_pool_iface_t *program_refs,
	const struct config_iface_t *config,
	struct issues_iface_t *issues)
{
    struct queries_t *query_list =
	CONTAINER_OF(self, struct queries_t, queries);

    query_list->var_refs = var_refs;
    query_list->function_refs = function_refs;
    query_list->program_refs = program_refs;

    return ESSTEE_OK;
}

static int queries_link(
    struct queries_iface_t *self,
    struct variable_iface_t *global_variables,
    struct function_iface_t *functions,
    struct program_iface_t *programs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct queries_t *query_list =
	CONTAINER_OF(self, struct queries_t, queries);
    
    st_resolve_var_refs(query_list->var_refs, global_variables);
    st_resolve_function_refs(query_list->function_refs, functions);
    st_resolve_program_refs(query_list->program_refs, programs);

    /* Resolve programs first, to make it possibly to override the base
     * of qualified identifiers */
    int callback_result = query_list->program_refs->trigger_resolve_callbacks(
	query_list->program_refs,
	config,
	issues);

    if(callback_result != ESSTEE_OK)
    {
	return callback_result;
    }
    
    callback_result = query_list->var_refs->trigger_resolve_callbacks(
	query_list->var_refs,
	config,
	issues);

    if(callback_result != ESSTEE_OK)
    {
	return callback_result;
    }
    
    callback_result = query_list->function_refs->trigger_resolve_callbacks(
	query_list->function_refs,
	config,
	issues);

    if(callback_result != ESSTEE_OK)
    {
	return callback_result;
    }

    struct query_t *qitr = NULL;
    DL_FOREACH(query_list->entries, qitr)
    {
	if(qitr->assignment && qitr->assignment->invoke.allocate)
	{
	    int allocate_result = qitr->assignment->invoke.allocate(
		&(qitr->assignment->invoke),
		issues);

	    if(allocate_result != ESSTEE_OK)
	    {
		return allocate_result;
	    }

	    int verify_result = qitr->assignment->invoke.verify(
		&(qitr->assignment->invoke),
		config,
		issues);

	    if(verify_result != ESSTEE_OK)
	    {
		return verify_result;
	    }
	}
    }

    
    return ESSTEE_OK;
}

static int queries_evaluate(
    struct queries_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct queries_t *query_list =
	CONTAINER_OF(self, struct queries_t, queries);

    /* Verify and step queries */
    struct query_t *itr = NULL;
    DL_FOREACH(query_list->entries, itr)
    {
	if(itr->qid)
	{
	    if(itr->qid->constant_reference == ESSTEE_FALSE)
	    {
		const char *message = issues->build_message(
		    issues,
		    "reference of '%s' is not constant which is required in a query",
		    itr->qid->target_name(itr->qid));
	    
		issues->new_issue_at(
		    issues,
		    message,
		    ESSTEE_CONTEXT_ERROR,
		    1,
		    itr->qid->location);

		return ESSTEE_ERROR;
	    }

	    int verify_result = itr->qid->verify(itr->qid,
						 config,
						 issues);

	    if(verify_result != ESSTEE_OK)
	    {
		return ESSTEE_ERROR;
	    }
	}

	if(itr->assignment)
	{
	    if(itr->assignment->invoke.step)
	    {
		issues->new_issue_at(
		    issues,
		    "query assignment expression must be compile time constant",
		    ESSTEE_CONTEXT_ERROR,
		    1,
		    itr->assignment->invoke.location);
		
		return ESSTEE_ERROR;
	    }
	    
	    if(itr->assignment->invoke.verify)
	    {
		int verify_result = itr->assignment->invoke.verify(&(itr->assignment->invoke),
								   config,
								   issues);
		if(verify_result != ESSTEE_OK)
		{
		    return ESSTEE_ERROR;
		}
	    }

	    const struct value_iface_t *assign_value =
		itr->assignment->return_value(itr->assignment);

	    int assignable_from_result = ESSTEE_TRUE;
	    if(itr->variable)
	    {
		assignable_from_result = itr->variable->assignable_from(
		    itr->variable,
		    NULL,
		    assign_value,
		    config,
		    issues);
	    }
	    else if(itr->qid)
	    {
		assignable_from_result = itr->qid->target_assignable_from(
		    itr->qid,
		    assign_value,
		    config,
		    issues);
	    }

	    if(assignable_from_result != ESSTEE_TRUE)
	    {
		return ESSTEE_ERROR;
	    }

	    int assignment_status = ESSTEE_OK;
	    if(itr->variable)
	    {
		assignment_status = itr->variable->assign(itr->variable,
							  NULL,
							  assign_value,
							  config,
							  issues);
	    }
	    else if(itr->qid)
	    {
		assignment_status = itr->qid->target_assign(itr->qid,
							    assign_value,
							    config,
							    issues);
	    }

	    if(assignment_status != ESSTEE_OK)
	    {
		return ESSTEE_ERROR;
	    }
	}
    }

    return ESSTEE_OK;
}

static int queries_display(
    struct queries_iface_t *self,
    char *output,
    size_t output_max_len,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct queries_t *query_list =
	CONTAINER_OF(self, struct queries_t, queries);
    
    size_t written_bytes = 0;
    int insert_separator = 0;
    
    struct query_t *itr = NULL;
    DL_FOREACH(query_list->entries, itr)
    {
	if(insert_separator)
	{
	    if(written_bytes+1 > output_max_len)
	    {
		issues->new_issue(
		    issues,
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
	if(!itr->variable && !itr->qid)
	{
	    query_written_bytes = itr->program->display(itr->program,
							output+written_bytes,
							output_max_len-written_bytes,
							config);
	}
	else
	{
	    const struct value_iface_t *target = NULL;

	    if(itr->variable)
	    {
		target = itr->variable->value(itr->variable);
	    }
	    else
	    {
		target = itr->qid->target_value(itr->qid);
	    }
	    	    
	    query_written_bytes = target->display(
		target,
		output+written_bytes,
		output_max_len-written_bytes,
		config);
	}
	
	if(query_written_bytes == ESSTEE_ERROR)
	{
	    issues->new_issue(
		issues,
		"output buffer write error occurred",
		ESSTEE_BUFFER_WARNING);

	    return ESSTEE_ERROR;
	}
	else if(query_written_bytes == ESSTEE_FALSE)
	{
	    issues->new_issue(
		issues,
		"output buffer full, output truncated",
		ESSTEE_BUFFER_WARNING);

	    return ESSTEE_ERROR;
	}

	written_bytes += query_written_bytes;
    }

    if(written_bytes+1 > output_max_len)
    {
    	issues->new_issue(
    	    issues,
    	    "output buffer full, output truncated",
    	    ESSTEE_BUFFER_WARNING);

    	return ESSTEE_ERROR;
    }
    else
    {
    	output[written_bytes] = '\0';
    }
    
    return ESSTEE_OK;
}

static void queries_destroy(
    struct queries_iface_t *self)
{
    /* TODO: query destroy */
}

/**************************************************************************/
/* Linker callback                                                        */
/**************************************************************************/
static int program_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!target)
    {
	const char *message = issues->build_message(
	    issues,
	    "reference to unknown program '%s'",
	    identifier);

	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_LINK_ERROR,
	    1,
	    location);
	    
	return ESSTEE_ERROR;
    }

    struct query_t *query = (struct query_t *)referrer;

    query->program = (struct program_iface_t *)target;

    if(query->identifier)
    {
	struct variable_iface_t *prgm_var = query->program->variable(query->program,
								     query->identifier,
								     config,
								     issues);
	if(!prgm_var)
	{
	    return ESSTEE_ERROR;
	}

	query->variable = prgm_var;
    }
    else if(query->qid)
    {
	const char *base_identifier = query->qid->base_identifier(query->qid,
								  config,
								  issues);

	struct variable_iface_t *prgm_var = query->program->variable(query->program,
							       base_identifier,
							       config,
							       issues);
	if(!prgm_var)
	{
	    return ESSTEE_ERROR;
	}
	
	query->qid->set_base(query->qid,
			     prgm_var,
			     config,
			     issues);
    }

    return ESSTEE_OK;
}

static int global_var_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!target)
    {
	const char *message = issues->build_message(
	    issues,
	    "reference to unknown global variable '%s'",
	    identifier);

	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_LINK_ERROR,
	    1,
	    location);
	    
	return ESSTEE_ERROR;
    }

    struct query_t *query = (struct query_t *)referrer;

    query->variable = (struct variable_iface_t *)target;

    return ESSTEE_OK;
}


/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct query_t * st_create_program_query(
    char *prgm_identifier,
    const struct st_location_t *prgm_identifier_location,
    struct named_ref_pool_iface_t *program_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct query_t *query = NULL;

    ALLOC_OR_ERROR_JUMP(
	query,
	struct query_t,
	issues,
	error_free_resources);

    query->program_identifier = prgm_identifier;
    query->identifier = NULL;
    query->program = NULL;
    query->variable = NULL;
    query->qid = NULL;
    query->assignment = NULL;
    
    int ref_result = program_refs->add(program_refs,
				       prgm_identifier,
				       query,
				       prgm_identifier_location,
				       program_resolved,
				       issues);

    if(ref_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    return query;
    
error_free_resources:
    free(query);
    return NULL;
}

struct query_t * st_create_identifier_query(
    char *prgm_identifier,
    const struct st_location_t *prgm_location,    
    char *identifier,
    const struct st_location_t *identifier_location,
    struct expression_iface_t *assigned,
    struct named_ref_pool_iface_t *program_refs,
    struct named_ref_pool_iface_t *var_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct query_t *query = NULL;

    ALLOC_OR_ERROR_JUMP(
	query,
	struct query_t,
	issues,
	error_free_resources);

    query->program_identifier = prgm_identifier;
    query->identifier = identifier;
    query->program = NULL;
    query->variable = NULL;
    query->qid = NULL;
    query->assignment = assigned;
    
    if(prgm_identifier)
    {
	int ref_result = program_refs->add(program_refs,
					   prgm_identifier,
					   query,
					   prgm_location,
					   program_resolved,
					   issues);

	if(ref_result != ESSTEE_OK)
	{
	    goto error_free_resources;
	}
    }
    else
    {
	int ref_result = var_refs->add(var_refs,
				       identifier,
				       query,
				       identifier_location,
				       global_var_resolved,
				       issues);

	if(ref_result != ESSTEE_OK)
	{
	    goto error_free_resources;
	}
    }

    return query;
    
error_free_resources:
    free(query);
    return NULL;
}

struct query_t * st_create_qualified_identifier_query(
    char *prgm_identifier,
    const struct st_location_t *prgm_location,
    struct qualified_identifier_iface_t *qid,
    struct expression_iface_t *assigned,
    struct named_ref_pool_iface_t *program_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct query_t *query = NULL;

    ALLOC_OR_ERROR_JUMP(
	query,
	struct query_t,
	issues,
	error_free_resources);

    query->program_identifier = prgm_identifier;
    query->identifier = NULL;
    query->program = NULL;
    query->variable = NULL;
    query->qid = qid;
    query->assignment = assigned;
    
    if(prgm_identifier)
    {
	int ref_result = program_refs->add(program_refs,
					   prgm_identifier,
					   query,
					   prgm_location,
					   program_resolved,
					   issues);

	if(ref_result != ESSTEE_OK)
	{
	    goto error_free_resources;
	}
    }

    return query;
    
error_free_resources:
    free(query);
    return NULL;
}

void st_destroy_query(
    struct query_t *query)
{
    /* TODO: query destroy */
}

struct queries_iface_t * st_create_queries(
    struct query_t *first_query,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct queries_t *query_list = NULL;

    ALLOC_OR_ERROR_JUMP(
	query_list,
	struct queries_t,
	issues,
	error_free_resources);

    query_list->entries = NULL;
    DL_APPEND(query_list->entries, first_query);

    memset(&(query_list->queries), 0, sizeof(struct queries_iface_t));
    query_list->queries.append = queries_append;
    query_list->queries.finish = queries_finish;
    query_list->queries.link = queries_link;
    query_list->queries.evaluate = queries_evaluate;
    query_list->queries.display = queries_display;
    query_list->queries.destroy = queries_destroy;
    
    return &(query_list->queries);

error_free_resources:
    return NULL;
}
