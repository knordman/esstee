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

#include <statements/case.h>
#include <statements/statements.h>
#include <util/macros.h>

#include <utlist.h>

struct case_list_element_t {
    struct value_iface_t *value;
    struct st_location_t *location;
    struct case_list_element_t *prev;
    struct case_list_element_t *next;
};

struct case_t {
    struct case_list_element_t *case_list;
    struct invoke_iface_t *statements;
    struct st_location_t *location;
    struct case_t *prev;
    struct case_t *next;
};

/**************************************************************************/
/* Invoke interface                                                       */
/**************************************************************************/
struct case_statement_t {
    struct invoke_iface_t invoke;
    struct expression_iface_t *selector;
    struct case_t *cases;
    struct st_location_t *location;
    struct invoke_iface_t *else_statements;
    int invoke_state;
};

static int selector_value_in_case_list(
    const struct value_iface_t *selector,
    struct case_list_element_t *case_list,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct case_list_element_t *itr = NULL;
    DL_FOREACH(case_list, itr)
    {
	int equals = itr->value->equals(itr->value,
					selector,
					config,
					issues);

	if(equals != ESSTEE_FALSE)
	{
	    return equals;
	}
    }

    return ESSTEE_FALSE;
}

static int case_statement_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);

    switch(cs->invoke_state)
    {
    case 0:
	if(cs->selector->invoke.step)
	{
	    cs->invoke_state = 1;
	    cursor->switch_current(cursor,
				   &(cs->selector->invoke),
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	
    case 1: {
	const struct value_iface_t *selector_value =
	    cs->selector->return_value(cs->selector);

	struct case_t *case_itr = NULL;
	DL_FOREACH(cs->cases, case_itr)
	{
	    int selector_in_case_result = selector_value_in_case_list(selector_value,
								      case_itr->case_list,
								      config,
								      issues);
	    
	    if(selector_in_case_result == ESSTEE_TRUE)
	    {
		break;
	    }
	    else if(selector_in_case_result == ESSTEE_ERROR)
	    {
		return INVOKE_RESULT_ERROR;
	    }
	}

	cs->invoke_state = 2;
	if(case_itr)
	{
	    cursor->switch_current(cursor,
				   case_itr->statements,
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	else if(cs->else_statements)
	{
	    cursor->switch_current(cursor,
				   cs->else_statements,
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

static int case_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);
    
    if(cs->selector->invoke.verify)
    {
	int verify_result =
	    cs->selector->invoke.verify(&(cs->selector->invoke),
					config,
					issues);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }

    const struct value_iface_t *selector_value =
	cs->selector->return_value(cs->selector);
    
    struct case_t *case_itr = NULL;
    DL_FOREACH(cs->cases, case_itr)
    {
	if(selector_value)
	{
	    struct case_list_element_t *case_list_itr = NULL;
	    DL_FOREACH(case_itr->case_list, case_list_itr)
	    {
		issues->begin_group(issues);
		int comparable_result = 
		    case_list_itr->value->comparable_to(case_list_itr->value,
							selector_value,
							config,
							issues);		
		if(comparable_result != ESSTEE_TRUE)
		{
		    issues->new_issue(issues,
				      "case selector not comparable to case value",
				      ESSTEE_CONTEXT_ERROR);
				      
		    issues->set_group_location(issues,
					       2,
					       case_list_itr->location,
					       cs->selector->invoke.location);
		}
		issues->end_group(issues);

		if(comparable_result != ESSTEE_TRUE)
		{
		    return ESSTEE_ERROR;
		}
	    }
	}
	
	struct invoke_iface_t *statement_itr = NULL;
	DL_FOREACH(case_itr->statements, statement_itr)
	{
	    int verify_result = statement_itr->verify(statement_itr,
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

static int case_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);

    cs->invoke_state = 0;

    if(cs->selector->invoke.reset)
    {
	int reset_result = cs->selector->invoke.reset(&(cs->selector->invoke),
						      config,
						      issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    struct case_t *case_itr = NULL;
    DL_FOREACH(cs->cases, case_itr)
    {
	struct invoke_iface_t *statement_itr = NULL;
	DL_FOREACH(case_itr->statements, statement_itr)
	{
	    int reset_result = statement_itr->reset(statement_itr,
						    config,
						    issues);
	    if(reset_result != ESSTEE_OK)
	    {
		return reset_result;
	    }
	}
    }

    struct invoke_iface_t *statement_itr = NULL;
    DL_FOREACH(cs->else_statements, statement_itr)
    {
	int reset_result = statement_itr->reset(statement_itr,
						config,
						issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

static int case_statement_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);

    if(cs->selector->invoke.allocate)
    {
	int allocate_result = cs->selector->invoke.allocate(
	    &(cs->selector->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    struct case_t *case_itr = NULL;
    DL_FOREACH(cs->cases, case_itr)
    {
	int allocate_result = st_allocate_statements(case_itr->statements,
						     issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    int else_allocate_result = st_allocate_statements(cs->else_statements,
						      issues);

    if(else_allocate_result != ESSTEE_OK)
    {
	return else_allocate_result;
    }

    return ESSTEE_OK;
}

static void case_statement_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: case statement destructor */
}

static void case_statement_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: case statement clone destructor */
}

static struct invoke_iface_t * case_statement_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct case_statement_t *cs =
	CONTAINER_OF(self, struct case_statement_t, invoke);

    struct case_statement_t *copy = NULL;
    struct expression_iface_t *selector_copy = NULL;
    struct invoke_iface_t *case_statements_copy = NULL;
    struct invoke_iface_t *else_statements_copy = NULL;
    struct case_t *case_itr = NULL;
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct case_statement_t,
	issues,
	error_free_resources);

    memcpy(copy, cs, sizeof(struct case_statement_t));

    if(cs->selector->clone)
    {
	selector_copy = cs->selector->clone(cs->selector, issues);

	if(!selector_copy)
	{
	    goto error_free_resources;
	}

	copy->selector = selector_copy;
    }

    struct case_t *cases_copy = NULL;
    struct case_t *case_copy = NULL;
    for(case_itr = cs->cases; case_itr != NULL; case_itr = case_itr->next)
    {
	ALLOC_OR_ERROR_JUMP(
	    case_copy,
	    struct case_t,
	    issues,
	    error_free_resources);

	memcpy(case_copy, case_itr, sizeof(struct case_t));
	case_copy->statements = NULL;

	case_statements_copy = NULL;
	struct invoke_iface_t *statement_itr = NULL;
	DL_FOREACH(case_itr->statements, statement_itr)
	{
	    struct invoke_iface_t *statement_copy =
		statement_itr->clone(statement_itr, issues);

	    if(!statement_copy)
	    {
		goto error_free_resources;
	    }

	    DL_APPEND(case_statements_copy, statement_copy);
	}

	case_copy->statements = case_statements_copy;
	DL_APPEND(cases_copy, case_copy);
    }
    copy->cases = cases_copy;
    
    struct invoke_iface_t *statement_itr = NULL;
    DL_FOREACH(cs->else_statements, statement_itr)
    {
	struct invoke_iface_t *statement_copy
	    = statement_itr->clone(statement_itr, issues);

	if(!statement_copy)
	{
	    goto error_free_resources;
	}

	DL_APPEND(else_statements_copy, statement_copy);
    }

    copy->else_statements = else_statements_copy;
    
    copy->invoke.destroy = case_statement_clone_destroy;

    return &(copy->invoke);

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct case_list_element_t * st_extend_case_list(
    struct case_list_element_t *case_list,
    struct value_iface_t *case_value,
    const struct st_location_t *case_value_location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct case_list_element_t *ce = NULL;
    struct st_location_t *ce_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	ce,
	struct case_list_element_t,
	issues,
	error_free_resources);
    
    LOCDUP_OR_ERROR_JUMP(
	ce_location,
	case_value_location,
	issues,
	error_free_resources);

    ce->location = ce_location;
    ce->value = case_value;

    DL_APPEND(case_list, ce);

    return case_list;

error_free_resources:
    free(ce);
    free(ce_location);
    return NULL;
}

struct case_t * st_create_case(
    struct case_list_element_t *case_list,
    const struct st_location_t *location,
    struct invoke_iface_t *statements,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct case_t *c = NULL;
    struct st_location_t *c_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	c,
	struct case_t,
	issues,
	error_free_resources);
    
    LOCDUP_OR_ERROR_JUMP(
	c_location,
	location,
	issues,
	error_free_resources);

    c->location = c_location;
    c->case_list = case_list;
    c->statements = statements;

    return c;
    
error_free_resources:
    free(c);
    free(c_location);
    return NULL;
}

struct case_t * st_extend_cases(
    struct case_t *cases,
    struct case_t *new_case,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    DL_APPEND(cases, new_case);

    return cases;
}

struct invoke_iface_t * st_create_case_statement(
    struct expression_iface_t *selector,
    struct case_t *cases,
    struct invoke_iface_t *else_statements,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct case_statement_t *cs = NULL;
    struct st_location_t *cs_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	cs,
	struct case_statement_t,
	issues,
	error_free_resources);
    
    LOCDUP_OR_ERROR_JUMP(
	cs_location,
	location,
	issues,
	error_free_resources);

    cs->location = cs_location;
    cs->selector = selector;
    cs->cases = cases;
    cs->else_statements = else_statements;

    memset(&(cs->invoke), 0, sizeof(struct invoke_iface_t));
    cs->invoke.location = cs->location;
    cs->invoke.step = case_statement_step;
    cs->invoke.verify = case_statement_verify;
    cs->invoke.reset = case_statement_reset;
    cs->invoke.allocate = case_statement_allocate;
    cs->invoke.clone = case_statement_clone;
    cs->invoke.destroy = case_statement_destroy;

    return &(cs->invoke);
    
error_free_resources:
    free(cs);
    free(cs_location);
    return NULL;
}

void st_destroy_case_list(
    struct case_list_element_t *case_list)
{
    /* TODO: case list destroy */
}

void st_destroy_case(
    struct case_t *c)
{
    /* TODO: case destroy */
}
