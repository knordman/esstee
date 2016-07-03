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

#include <statements/conditionals.h>
#include <statements/statements.h>
#include <util/macros.h>

#include <utlist.h>

/**************************************************************************/
/* Invoke interface                                                       */
/**************************************************************************/
struct if_statement_t {
    struct invoke_iface_t invoke;
    struct expression_iface_t *condition;
    struct st_location_t *location;
    struct invoke_iface_t *true_statements;
    struct if_statement_t *elsif;
    struct invoke_iface_t *else_statements;
    int invoke_state;
};

static int if_statement_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    switch(ifs->invoke_state)
    {
    case 0:
	if(ifs->condition->invoke.step)
	{
	    ifs->invoke_state = 1;
	    cursor->switch_current(cursor,
				   &(ifs->condition->invoke),
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 1: {
	const struct value_iface_t *condition_value =
	    ifs->condition->return_value(ifs->condition);

	int current_condition = condition_value->bool(condition_value,
						      config,
						      issues);

	ifs->invoke_state = 2;
	
	if(current_condition == ESSTEE_TRUE)
	{
	    cursor->switch_current(cursor,
				   ifs->true_statements,
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	else if(ifs->elsif)
	{
	    cursor->switch_current(cursor,
				   &(ifs->elsif->invoke),
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	else if(ifs->else_statements)
	{
	    cursor->switch_current(cursor,
				   ifs->else_statements,
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
    }

    case 2:
    default:
	break;
    }

    return INVOKE_RESULT_FINISHED;
}

static int if_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    if(ifs->condition->invoke.verify)
    {
	int condition_verify =
	    ifs->condition->invoke.verify(&(ifs->condition->invoke),
					  config,
					  issues);
	if(condition_verify != ESSTEE_OK)
	{
	    return condition_verify;
	}
    }

    const struct value_iface_t *condition_value =
	ifs->condition->return_value(ifs->condition);

    if(!condition_value->bool)
    {
	issues->new_issue_at(issues,
			     "condition cannot be interpreted as true or false",
			     ISSUE_ERROR_CLASS,
			     1,
			     ifs->location);
	return ESSTEE_ERROR;
    }
    
    struct invoke_iface_t *true_itr;
    DL_FOREACH(ifs->true_statements, true_itr)
    {
	if(true_itr->verify(true_itr, config, issues) != ESSTEE_OK)
	{
	    return ESSTEE_ERROR;
	}
    }

    if(ifs->elsif)
    {
	return if_statement_verify(&(ifs->elsif->invoke),
				   config,
				   issues);
    }
    else if(ifs->else_statements)
    {
	struct invoke_iface_t *else_itr;
	DL_FOREACH(ifs->else_statements, else_itr)
	{
	    if(else_itr->verify(else_itr, config, issues) != ESSTEE_OK)
	    {
		return ESSTEE_ERROR;
	    }
	}
    }

    return ESSTEE_OK;
}

static int if_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    ifs->invoke_state = 0;

    if(ifs->condition->invoke.reset)
    {
	int reset_result = ifs->condition->invoke.reset(&(ifs->condition->invoke),
							config,
							issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    struct invoke_iface_t *true_itr = NULL;
    DL_FOREACH(ifs->true_statements, true_itr)
    {
	int reset_result = true_itr->reset(true_itr, config, issues);

	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    if(ifs->elsif)
    {
	int reset_result = ifs->elsif->invoke.reset(&(ifs->elsif->invoke),
						    config,
						    issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }
    else if(ifs->else_statements)
    {
	struct invoke_iface_t *else_itr;
	DL_FOREACH(ifs->else_statements, else_itr)
	{
	    int reset_result = else_itr->reset(else_itr,
					       config,
					       issues);
	    if(reset_result != ESSTEE_OK)
	    {
		return reset_result;
	    }
	}
    }

    return ESSTEE_OK;
}

static int if_statement_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    if(ifs->condition->invoke.allocate)
    {
	int allocate_result = ifs->condition->invoke.allocate(
	    &(ifs->condition->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    int true_allocate = st_allocate_statements(ifs->true_statements,
					       issues);
    if(true_allocate != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    if(ifs->elsif)
    {
	int allocate_result = ifs->elsif->invoke.allocate(
	    &(ifs->elsif->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }
    else if(ifs->else_statements)
    {
	return st_allocate_statements(ifs->else_statements, issues);
    }

    return ESSTEE_OK;
}

static void if_statement_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: if statement destructor */
}

static void if_statement_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: if statement clone destructor */
}

static struct invoke_iface_t * if_statement_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct if_statement_t *ifs =
	CONTAINER_OF(self, struct if_statement_t, invoke);

    struct if_statement_t *copy = NULL;
    struct expression_iface_t *condition_copy = NULL;
    struct invoke_iface_t *true_statements_copy = NULL;
    struct invoke_iface_t *else_statements_copy = NULL;
    struct invoke_iface_t *destroy_itr = NULL;
    struct invoke_iface_t *tmp = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct if_statement_t,
	issues,
	error_free_resources);

    memcpy(copy, ifs, sizeof(struct if_statement_t));

    if(ifs->condition->clone)
    {
	condition_copy = ifs->condition->clone(ifs->condition, issues);

	if(!condition_copy)
	{
	    goto error_free_resources;
	}

	copy->condition = condition_copy;
    }

    struct invoke_iface_t *true_itr = NULL;
    DL_FOREACH(ifs->true_statements, true_itr)
    {
	struct invoke_iface_t *statement_copy = true_itr->clone(true_itr, issues);

	if(!statement_copy)
	{
	    goto error_free_resources;
	}

	DL_APPEND(true_statements_copy, statement_copy);
    }

    if(ifs->elsif)
    {
	struct invoke_iface_t *elsif_copy_invoke =
	    ifs->elsif->invoke.clone(&(ifs->elsif->invoke), issues);

	if(!elsif_copy_invoke)
	{
	    goto error_free_resources;
	}

	struct if_statement_t *elsif_copy =
	    CONTAINER_OF(elsif_copy_invoke, struct if_statement_t, invoke);

	copy->elsif = elsif_copy;
    }
    else if(ifs->else_statements)
    {
	struct invoke_iface_t *else_itr;
	DL_FOREACH(ifs->else_statements, else_itr)
	{
	    struct invoke_iface_t *statement_copy = else_itr->clone(else_itr, issues);

	    if(!statement_copy)
	    {
		goto error_free_resources;
	    }

	    DL_APPEND(else_statements_copy, statement_copy);
	}
    }
    
    copy->true_statements = true_statements_copy;
    copy->else_statements = else_statements_copy;

    copy->invoke.destroy = if_statement_clone_destroy;
    
    return &(copy->invoke);
    
error_free_resources:
    DL_FOREACH_SAFE(true_statements_copy, destroy_itr, tmp)
    {
	free(destroy_itr);
    }
    DL_FOREACH_SAFE(else_statements_copy, destroy_itr, tmp)
    {
	free(destroy_itr);
    }
    free(copy);
    free(condition_copy);
    return NULL;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct if_statement_t * st_create_elsif_clause(
    struct expression_iface_t *condition,
    const struct st_location_t *condition_location,
    struct invoke_iface_t *true_statements,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct if_statement_t *ifs = NULL;
    struct st_location_t *ifs_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	ifs,
	struct if_statement_t,
	issues,
	error_free_resources);
    
    LOCDUP_OR_ERROR_JUMP(
	ifs_location,
	location,
	issues,
	error_free_resources);

    ifs->condition = condition;
    ifs->location = ifs_location;
    ifs->true_statements = true_statements;
    ifs->else_statements = NULL;
    ifs->elsif = NULL;

    memset(&(ifs->invoke), 0, sizeof(struct invoke_iface_t));
    ifs->invoke.location = ifs->location;
    ifs->invoke.step = if_statement_step;
    ifs->invoke.verify = if_statement_verify;
    ifs->invoke.reset = if_statement_reset;
    ifs->invoke.allocate = if_statement_allocate;
    ifs->invoke.clone = if_statement_clone;
	
    return ifs;
    
error_free_resources:
    free(ifs);
    free(ifs_location);
    return NULL;
}

struct if_statement_t * st_extend_elsif_clauses(
    struct if_statement_t *elsif_clauses,
    struct if_statement_t *elsif_clause,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!elsif_clauses)
    {
	return elsif_clause;
    }
    else
    {
	struct if_statement_t *itr = elsif_clauses;
	for(; itr->elsif != NULL; itr = itr->elsif){}

	itr->elsif = elsif_clause;
    }

    return elsif_clauses;
}

void st_destroy_elsif_clauses(
    struct if_statement_t *clauses)
{
    /* TODO: elsif clauses destroy */
}

struct invoke_iface_t * st_create_if_statement(
    struct expression_iface_t *condition,
    const struct st_location_t *condition_location,
    struct invoke_iface_t *true_statements,
    struct if_statement_t *elsif_clauses,
    struct invoke_iface_t *else_statements,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct if_statement_t *ifs = NULL;
    struct st_location_t *ifs_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	ifs,
	struct if_statement_t,
	issues,
	error_free_resources);
    
    LOCDUP_OR_ERROR_JUMP(
	ifs_location,
	location,
	issues,
	error_free_resources);

    ifs->condition = condition;
    ifs->location = ifs_location;
    ifs->true_statements = true_statements;
    ifs->elsif = elsif_clauses;
    struct if_statement_t *itr = ifs;
    for(; itr->elsif != NULL; itr = itr->elsif){}
    itr->else_statements = else_statements;

    memset(&(ifs->invoke), 0, sizeof(struct invoke_iface_t));
    ifs->invoke.location = ifs->location;
    ifs->invoke.step = if_statement_step;
    ifs->invoke.verify = if_statement_verify;
    ifs->invoke.reset = if_statement_reset;
    ifs->invoke.allocate = if_statement_allocate;
    ifs->invoke.clone = if_statement_clone;
    ifs->invoke.destroy = if_statement_destroy;
	
    return &(ifs->invoke);
    
error_free_resources:
    free(ifs);
    free(ifs_location);
    return NULL;
}
