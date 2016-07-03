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

#include <statements/loops.h>
#include <util/macros.h>
#include <elements/ivariable.h>
#include <elements/integers.h>
#include <statements/statements.h>

#include <utlist.h>

/**************************************************************************/
/* Invoke interface                                                       */
/**************************************************************************/
/* For */
struct for_statement_t {
    struct invoke_iface_t invoke;
    struct variable_iface_t *variable;
    struct expression_iface_t *from;
    struct expression_iface_t *to;
    struct expression_iface_t *increment;
    struct invoke_iface_t *statements;
    struct st_location_t *location;
    struct st_location_t *identifier_location;
    int invoke_state;
    struct value_iface_t *implicit_increment;
};

static int for_statement_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    st_integer_value_set(fs->implicit_increment, 1);
    const struct value_iface_t *increment_value = fs->implicit_increment;
    if(fs->increment)
    {
	increment_value = fs->increment->return_value(fs->increment);
    }
    
    const struct value_iface_t *from_value = fs->from->return_value(fs->from);
    const struct value_iface_t *to_value = fs->to->return_value(fs->to);
    int variable_assign_result = ESSTEE_ERROR;
    int add_result = ESSTEE_ERROR;
    const struct value_iface_t *var_value = NULL;
    
    switch(fs->invoke_state)
    {
    case 0:
	cursor->push_exit_context(cursor, self);
	
	if(fs->from->invoke.step)
	{
	    fs->invoke_state = 1;
	    cursor->switch_current(cursor,
				   &(fs->from->invoke),
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 1:
	variable_assign_result = fs->variable->assign(fs->variable,
						      NULL,
						      from_value,
						      config,
						      issues);
	if(variable_assign_result != ESSTEE_OK)
	{
	    return INVOKE_RESULT_ERROR;
	}

    case 2:
	if(fs->to->invoke.step)
	{
	    fs->invoke_state = 3;
	    cursor->switch_current(cursor,
				   &(fs->to->invoke),
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 3:
	var_value = fs->variable->value(fs->variable);
	
	if(var_value->greater(var_value, to_value, config, issues) == ESSTEE_TRUE)
	{
	    break;
	}
	
    case 4: 
	fs->invoke_state = 5;
	cursor->switch_current(cursor,
			       fs->statements,
			       config,
			       issues);
	return INVOKE_RESULT_IN_PROGRESS;

    case 5:
	if(fs->increment && fs->increment->invoke.step)
	{
	    fs->invoke_state = 6;
	    cursor->switch_current(cursor,
				   &(fs->increment->invoke),
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	
    case 6:
	add_result = fs->variable->plus(fs->variable,
					NULL,
					increment_value,
					config,
					issues);
	
	if(add_result != ESSTEE_OK)
	{
	    return INVOKE_RESULT_ERROR;
	}

	fs->invoke_state = 2;
	return INVOKE_RESULT_IN_PROGRESS;
    }

    cursor->pop_exit_context(cursor);
    
    return INVOKE_RESULT_FINISHED;
}

static int for_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    if(fs->from->invoke.verify)
    {
	int verify_result = fs->from->invoke.verify(&(fs->from->invoke),
						    config,
						    issues);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }

    if(fs->to->invoke.verify)
    {
	int verify_result = fs->to->invoke.verify(&(fs->to->invoke),
						  config,
						  issues);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }
    
    if(fs->increment && fs->increment->invoke.verify)
    {
	int verify_result = fs->increment->invoke.verify(&(fs->increment->invoke),
							 config,
							 issues);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }

    const struct value_iface_t *from_value = fs->from->return_value(fs->from);
    const struct value_iface_t *to_value = fs->to->return_value(fs->to);
    const struct value_iface_t *var_value = fs->variable->value(fs->variable);

    st_integer_value_set(fs->implicit_increment, 1);
    const struct value_iface_t *increment_value = fs->implicit_increment;
    if(fs->increment)
    {
	increment_value = fs->increment->return_value(fs->increment);
    }
    
    issues->begin_group(issues);
    int var_from_assignable = fs->variable->assignable_from(fs->variable,
							    NULL,
							    from_value,
							    config,
							    issues);
    if(var_from_assignable != ESSTEE_TRUE)
    {
	issues->new_issue(issues,
			  "iteration variable '%s' cannot be assigned the from value",
			  ESSTEE_CONTEXT_ERROR,
			  fs->variable->identifier);

	issues->set_group_location(issues, 
				   2,
				   fs->identifier_location,
				   fs->from->invoke.location);
    }
    issues->end_group(issues);

    if(var_from_assignable != ESSTEE_TRUE)
    {
	return ESSTEE_ERROR;
    }

    const struct type_iface_t *var_type = var_value->type_of(var_value);

    issues->begin_group(issues);
    int type_can_hold_to_value = var_type->can_hold(var_type,
						    to_value,
						    config,
						    issues);
    if(type_can_hold_to_value != ESSTEE_TRUE)
    {
	issues->new_issue(issues,
			  "iteration variable '%s' cannot hold the end value",
			  ESSTEE_TYPE_ERROR,
			  fs->variable->identifier);

	issues->set_group_location(issues,
				   2,
				   fs->identifier_location,
				   fs->to->invoke.location);
    }
    issues->end_group(issues);

    if(type_can_hold_to_value != ESSTEE_TRUE)
    {
	return ESSTEE_ERROR;
    }

    issues->begin_group(issues);
    int var_increment_operates = var_value->operates_with(var_value,
							  increment_value,
							  config,
							  issues);

    if(var_increment_operates != ESSTEE_TRUE)
    {
	issues->new_issue(issues,
			  "iteration variable '%s' cannot be incremented as specified",
			  ESSTEE_CONTEXT_ERROR,
			  fs->variable->identifier);

	issues->set_group_location(issues,
				   2,
				   fs->identifier_location,
				   fs->increment->invoke.location);
    }
    issues->end_group(issues);
    
    if(var_increment_operates != ESSTEE_TRUE)
    {
	return ESSTEE_ERROR;
    }

    issues->begin_group(issues);
    int var_to_comparable = var_value->comparable_to(var_value,
						     to_value,
						     config,
						     issues);

    if(var_to_comparable != ESSTEE_TRUE)
    {
	issues->new_issue(issues,
			  "iteration variable '%s' cannot be compared to end value",
			  ESSTEE_CONTEXT_ERROR,
			  fs->variable->identifier);

	issues->set_group_location(issues,
				   2,
				   fs->identifier_location,
				   fs->to->invoke.location);
    }
    issues->end_group(issues);
    
    if(var_to_comparable != ESSTEE_TRUE)
    {
	return ESSTEE_ERROR;
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(fs->statements, itr)
    {
	int verify_result = itr->verify(itr, config, issues);

	if(verify_result != ESSTEE_OK)
	{
	    return ESSTEE_ERROR;
	}
    }

    return ESSTEE_OK;
}

static int for_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    fs->invoke_state = 0;
    
    if(fs->from->invoke.reset)
    {
	int reset_result = fs->from->invoke.reset(&(fs->from->invoke),
						  config,
						  issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }
    
    if(fs->to->invoke.reset)
    {
	int reset_result = fs->to->invoke.reset(&(fs->to->invoke),
						config,
						issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    if(fs->increment && fs->increment->invoke.reset)
    {
	int reset_result = fs->increment->invoke.reset(&(fs->increment->invoke),
						       config,
						       issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(fs->statements, itr)
    {
	int reset_result = itr->reset(itr,
				      config,
				      issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

static int for_statement_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    if(fs->from->invoke.allocate)
    {
	int allocate_result = fs->from->invoke.allocate(
	    &(fs->from->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }
    
    if(fs->to->invoke.allocate)
    {
	int allocate_result = fs->to->invoke.allocate(&(fs->to->invoke),
						      issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    if(fs->increment && fs->increment->invoke.allocate)
    {
	int allocate_result = fs->increment->invoke.allocate(&(fs->increment->invoke),
							     issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    return st_allocate_statements(fs->statements, issues);
}

static void for_statement_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: for statement destructor */
}

static void for_statement_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: cloned for statement destructor */
}

static struct invoke_iface_t * for_statement_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct for_statement_t *fs =
	CONTAINER_OF(self, struct for_statement_t, invoke);

    struct for_statement_t *copy = NULL;
    struct expression_iface_t *from_copy = NULL;
    struct expression_iface_t *to_copy = NULL;
    struct expression_iface_t *increment_copy = NULL;
    struct invoke_iface_t *statements_copy = NULL;
    struct invoke_iface_t *statement_copy = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct for_statement_t,
	issues,
	error_free_resources);

    memcpy(copy, fs, sizeof(struct for_statement_t));
    copy->statements = NULL;
    
    if(fs->from->clone)
    {
	from_copy = fs->from->clone(fs->from, issues);

	if(!from_copy)
	{
	    goto error_free_resources;
	}

	copy->from = from_copy;
    }
    
    if(fs->to->clone)
    {
	to_copy = fs->to->clone(fs->to, issues);

	if(!to_copy)
	{
	    goto error_free_resources;
	}

	copy->to = to_copy;
    }

    if(fs->increment && fs->increment->clone)
    {
	increment_copy = fs->increment->clone(fs->increment, issues);

	if(!increment_copy)
	{
	    goto error_free_resources;
	}

	copy->increment = increment_copy;
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(fs->statements, itr)
    {
	statement_copy = itr->clone(itr, issues);

	if(!statement_copy)
	{
	    goto error_free_resources;
	}

	DL_APPEND(statements_copy, statement_copy);
    }

    copy->statements = statements_copy;
    copy->invoke.destroy = for_statement_clone_destroy;
    
    return &(copy->invoke);

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

/* While/repeat */
struct while_statement_t {
    struct invoke_iface_t invoke;
    struct expression_iface_t *while_expression;
    struct invoke_iface_t *statements;
    struct st_location_t *location;
    int invoke_state;
};

static int while_statement_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);

    const struct value_iface_t *while_value = NULL;
    
    switch(ws->invoke_state)
    {
    case 0:
	cursor->push_exit_context(cursor, self);
	
    case 1:
	if(ws->while_expression->invoke.step)
	{
	    ws->invoke_state = 2;
	    cursor->switch_current(cursor,
				   &(ws->while_expression->invoke),
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 2:
    default:
	while_value = ws->while_expression->return_value(ws->while_expression);

	if(while_value->bool(while_value, config, issues) == ESSTEE_TRUE)
	{
	    ws->invoke_state = 1;
	    cursor->switch_current(cursor,
				   ws->statements,
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}
    }

    cursor->pop_exit_context(cursor);
    
    return INVOKE_RESULT_FINISHED;
}

static int while_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);
    
    if(ws->while_expression->invoke.verify)
    {
	int verify_result =
	    ws->while_expression->invoke.verify(&(ws->while_expression->invoke),
						config,
						issues);
	if(verify_result != ESSTEE_OK)
	{
	    return verify_result;
	}
    }

    const struct value_iface_t *while_value =
	ws->while_expression->return_value(ws->while_expression);

    if(!while_value->bool)
    {
	issues->new_issue_at(issues,
			     "condition cannot be interpreted as true or false",
			     ESSTEE_CONTEXT_ERROR,
			     1,
			     ws->while_expression->invoke.location);
	return ESSTEE_ERROR;
    }

    return st_verify_statements(ws->statements, config, issues);
}

static int while_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);

    ws->invoke_state = 0;

    if(ws->while_expression->invoke.reset)
    {
	int reset_result =
	    ws->while_expression->invoke.reset(&(ws->while_expression->invoke),
					       config,
					       issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(ws->statements, itr)
    {
	int reset_result = itr->reset(itr, config, issues);

	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }
    
    return ESSTEE_OK;
}

static int while_statement_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);

    if(ws->while_expression->invoke.allocate)
    {
	int allocate_result = ws->while_expression->invoke.allocate(
	    &(ws->while_expression->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    return st_allocate_statements(ws->statements, issues);
}

static void while_statement_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: while statement destructor */
}

static void while_statement_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: while statement clone destructor */
}

static struct invoke_iface_t * while_statement_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);

    struct while_statement_t *copy = NULL;
    struct invoke_iface_t *statements_copy = NULL;
    struct expression_iface_t *while_expression_copy = NULL;

    ALLOC_OR_ERROR_JUMP(
	copy,
	struct while_statement_t,
	issues,
	error_free_resources);

    memcpy(copy, ws, sizeof(struct while_statement_t));

    if(ws->while_expression->clone)
    {
	while_expression_copy = ws->while_expression->clone(ws->while_expression,
							    issues);

	if(!while_expression_copy)
	{
	    goto error_free_resources;
	}

	copy->while_expression = while_expression_copy;
    }

    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(ws->statements, itr)
    {
	struct invoke_iface_t *statement_copy = itr->clone(itr, issues);

	if(!statement_copy)
	{
	    goto error_free_resources;
	}

	DL_APPEND(statements_copy, statement_copy);
    }
    
    copy->statements = statements_copy;

    copy->invoke.destroy = while_statement_clone_destroy;

    return &(copy->invoke);

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

static int repeat_statement_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct while_statement_t *ws =
	CONTAINER_OF(self, struct while_statement_t, invoke);

    const struct value_iface_t *while_value = NULL;
    
    switch(ws->invoke_state)
    {
    case 0:
	cursor->push_exit_context(cursor, self);
	
    case 1:
	ws->invoke_state = 2;
	cursor->switch_current(cursor,
			       ws->statements,
			       config,
			       issues);
	return INVOKE_RESULT_IN_PROGRESS;

    case 2:
	if(ws->while_expression->invoke.step)
	{
	    ws->invoke_state = 3;
	    cursor->switch_current(cursor,
				   &(ws->while_expression->invoke),
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 3:
	while_value = ws->while_expression->return_value(ws->while_expression);

	if(while_value->bool(while_value, config, issues) == ESSTEE_FALSE)
	{
	    ws->invoke_state = 1;
	    return INVOKE_RESULT_IN_PROGRESS;
	}
	else
	{
	    break;
	}
    }

    cursor->pop_exit_context(cursor);
    
    return INVOKE_RESULT_FINISHED;
}

/**************************************************************************/
/* Linker callbacks                                                       */
/**************************************************************************/
static int for_statement_variable_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(target == NULL)
    {
	const char *message = issues->build_message(
	    issues,
	    "reference to undefined variable '%s'",
	    identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_LINK_ERROR,
	    1,
	    location);

	return ESSTEE_ERROR;
    }
    
    struct for_statement_t *fs =
	(struct for_statement_t *)referrer;

    fs->variable = (struct variable_iface_t *)target;

    return ESSTEE_OK;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct invoke_iface_t * st_create_for_statement(
    char *variable_identifier,
    const struct st_location_t *identifier_location,
    struct expression_iface_t *from,
    struct expression_iface_t *to,
    struct expression_iface_t *increment,
    struct invoke_iface_t *statements,
    const struct st_location_t *location,
    struct named_ref_pool_iface_t *var_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct for_statement_t *fs = NULL;
    struct st_location_t *fs_location = NULL;
    struct st_location_t *fs_identifier_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	fs,
	struct for_statement_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	fs_location,
	location,
	issues,
	error_free_resources);
    
    LOCDUP_OR_ERROR_JUMP(
	fs_identifier_location,
	identifier_location,
	issues,
	error_free_resources);

    fs->implicit_increment = st_new_typeless_integer_value(0,
							   0,
							   config,
							   issues);

    if(!fs->implicit_increment)
    {
	goto error_free_resources;
    }
    
    int ref_add_result = var_refs->add(var_refs,
				       variable_identifier,
				       fs,
				       identifier_location,
				       for_statement_variable_resolved,
				       issues);

    if(ref_add_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    fs->location = fs_location;
    fs->identifier_location = fs_identifier_location;
    fs->from = from;
    fs->to = to;
    fs->increment = increment;
    fs->statements = statements;

    memset(&(fs->invoke), 0, sizeof(struct invoke_iface_t));
    fs->invoke.location = fs->location;
    fs->invoke.step = for_statement_step;
    fs->invoke.verify = for_statement_verify;
    fs->invoke.reset = for_statement_reset;
    fs->invoke.allocate = for_statement_allocate;
    fs->invoke.clone = for_statement_clone;
    fs->invoke.destroy = for_statement_destroy;

    return &(fs->invoke);
    
error_free_resources:
    free(fs);
    free(fs_location);
    free(fs_identifier_location);
    return NULL;
}

struct invoke_iface_t * st_create_while_statement(
    struct expression_iface_t *while_expression,
    struct invoke_iface_t *statements,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct while_statement_t *ws = NULL;
    struct st_location_t *ws_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	ws,
	struct while_statement_t,
	issues,
	error_free_resources);
    
    LOCDUP_OR_ERROR_JUMP(
	ws_location,
	location,
	issues,
	error_free_resources);

    ws->location = ws_location;
    ws->while_expression = while_expression;
    ws->statements = statements;

    memset(&(ws->invoke), 0, sizeof(struct invoke_iface_t));
    ws->invoke.location = ws->location;
    ws->invoke.step = while_statement_step;
    ws->invoke.verify = while_statement_verify;
    ws->invoke.reset = while_statement_reset;
    ws->invoke.allocate = while_statement_allocate;
    ws->invoke.clone = while_statement_clone;
    ws->invoke.destroy = while_statement_destroy;
      
    return &(ws->invoke);
    
error_free_resources:
    free(ws);
    free(ws_location);
    return NULL;
}

struct invoke_iface_t * st_create_repeat_statement(
    struct expression_iface_t *repeat_expression,
    struct invoke_iface_t *statements,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_iface_t *loop = st_create_while_statement(repeat_expression,
							  statements,
							  location,
							  config,
							  issues);
    if(loop)
    {
	struct while_statement_t *ws =
	    CONTAINER_OF(loop, struct while_statement_t, invoke);

	ws->invoke.step = repeat_statement_step;
    }
      
    return loop;
}
