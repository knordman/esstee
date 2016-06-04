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

#include <rt/cursor.h>
#include <elements/iinvoke.h>
#include <util/iconfig.h>
#include <util/iissues.h>
#include <util/macros.h>

#include <utlist.h>
#include <stdlib.h>

struct cursor_t {
    struct cursor_iface_t cursor;
    struct invoke_iface_t *call_stack;
    struct invoke_iface_t *exit_context;
    struct invoke_iface_t *return_context;
    struct invoke_iface_t *current;
    struct invoke_iface_t *cycle_start;
};

static void set_current_to_next(
    struct cursor_t *cur,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(cur->current->next != NULL)
    {
	cur->current->next->reset(cur->current->next, config, issues);
	cur->current = cur->current->next;
    }
    else
    {
	if(cur->call_stack == NULL)
	{
	    /* Cycle complete */
	    cur->current = cur->cycle_start;
	}
	else
	{
	    /* Step out */
	    cur->current = cur->call_stack;

	    /* Pop the call stack */
	    DL_DELETE2(
		cur->call_stack,
		cur->call_stack,
		call_stack_prev,
		call_stack_next);
	}
    }
}

static struct invoke_iface_t * cursor_step(
    struct cursor_iface_t *self,
    struct systime_iface_t *systime,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct cursor_t *cur =
	CONTAINER_OF(self, struct cursor_t, cursor);

    struct invoke_iface_t *start = cur->current;
    int start_result = INVOKE_RESULT_IN_PROGRESS;
    
    do
    {
	int invoke_result = cur->current->step(cur->current,
					       self,
					       systime,
					       config,
					       issues);

	if(cur->current == start)
	{
	    start_result = invoke_result;
	}

	if(invoke_result == INVOKE_RESULT_ERROR)
	{
	    return NULL;
	}
	else if(invoke_result == INVOKE_RESULT_ALL_FINISHED)
	{
	    cur->current = cur->cycle_start;
	    break;
	}
	else if(invoke_result == INVOKE_RESULT_FINISHED)
	{
	    set_current_to_next(cur, config, issues);
	}
    }
    while(start_result != INVOKE_RESULT_FINISHED);

    return cur->current;
}

static struct invoke_iface_t * cursor_step_in(
    struct cursor_iface_t *self,
    struct systime_iface_t *systime,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct cursor_t *cur =
	CONTAINER_OF(self, struct cursor_t, cursor);

    int invoke_result = cur->current->step(cur->current,
					   self,
					   systime,
					   config,
					   issues);

    if(invoke_result == INVOKE_RESULT_ERROR)
    {
	return NULL;
    }
    else if(invoke_result == INVOKE_RESULT_ALL_FINISHED)
    {
	cur->current = cur->cycle_start;
    }
    else if(invoke_result == INVOKE_RESULT_FINISHED)
    {
	set_current_to_next(cur, config, issues);
    }

    return cur->current;
}

static struct invoke_iface_t * cursor_step_out(
    struct cursor_iface_t *self,
    struct systime_iface_t *systime,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct cursor_t *cur =
	CONTAINER_OF(self, struct cursor_t, cursor);
    
    struct invoke_iface_t *call_stack = cur->call_stack;
    
    if(call_stack == NULL)
    {
	return self->step(self, systime, config, issues);
    }
    else
    {
	struct invoke_iface_t *stop_at = cur->call_stack->next;
	
	do
	{
	    int invoke_result = cur->current->step(cur->current,
						   self,
						   systime,
						   config,
						   issues);

	    if(invoke_result == INVOKE_RESULT_ERROR)
	    {
		return NULL;
	    }
	    else if(invoke_result == INVOKE_RESULT_ALL_FINISHED)
	    {
		cur->current = cur->cycle_start;
		break;
	    }
	    else if(invoke_result == INVOKE_RESULT_FINISHED)
	    {
		set_current_to_next(cur, config, issues);
	    }
	}
	while(cur->current != stop_at);
    }

    return cur->current;
}

static int cursor_switch_current(
    struct cursor_iface_t *self,
    struct invoke_iface_t *switch_to,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct cursor_t *cur =
	CONTAINER_OF(self, struct cursor_t, cursor);

    int reset_result = switch_to->reset(switch_to, config, issues);
    if(reset_result != ESSTEE_OK)
    {
	return reset_result;
    }
    
    DL_PREPEND2(cur->call_stack,
		cur->current,
		call_stack_prev,
		call_stack_next);
    cur->current = switch_to;

    return ESSTEE_OK;
}

static void cursor_push_return_context(
    struct cursor_iface_t *self,
    struct invoke_iface_t *context)
{
    struct cursor_t *cur =
	CONTAINER_OF(self, struct cursor_t, cursor);

    DL_PREPEND2(
	cur->return_context,
	context,
	return_context_prev,
	return_context_next);
}

static void cursor_pop_return_context(
    struct cursor_iface_t *self)
{
    struct cursor_t *cur =
	CONTAINER_OF(self, struct cursor_t, cursor);

    if(cur->return_context)
    {
	DL_DELETE2(
	    cur->return_context,
	    cur->return_context,
	    return_context_prev,
	    return_context_next);
    }
}

static int cursor_jump_return(
    struct cursor_iface_t *self)
{
    struct cursor_t *cur =
	CONTAINER_OF(self, struct cursor_t, cursor);
    
    if(cur->return_context)
    {
	cur->current = cur->return_context;
	self->pop_return_context(self);

	if(cur->call_stack)
	{
	    while(cur->call_stack != cur->current)
	    {
		DL_DELETE2(cur->call_stack,
			   cur->call_stack,
			   call_stack_prev,
			   call_stack_next);
	    }

	    DL_DELETE2(cur->call_stack,
		       cur->call_stack,
		       call_stack_prev,
		       call_stack_next);
	}

	return INVOKE_RESULT_FINISHED;
    }

    return INVOKE_RESULT_ALL_FINISHED;
}

static void cursor_push_exit_context(
    struct cursor_iface_t *self,
    struct invoke_iface_t *context)
{
    struct cursor_t *cur =
	CONTAINER_OF(self, struct cursor_t, cursor);
    
    DL_PREPEND2(
	cur->exit_context,
	context,
	exit_context_prev,
	exit_context_next);
}

static void cursor_pop_exit_context(
    struct cursor_iface_t *self)
{
    struct cursor_t *cur =
	CONTAINER_OF(self, struct cursor_t, cursor);

    if(cur->exit_context)
    {
	DL_DELETE2(
	    cur->exit_context,
	    cur->exit_context,
	    exit_context_prev,
	    exit_context_next);
    }
}

static int cursor_jump_exit(
    struct cursor_iface_t *self)
{
    struct cursor_t *cur =
	CONTAINER_OF(self, struct cursor_t, cursor);
    
    cur->current = cur->exit_context;
    self->pop_exit_context(self);

    if(cur->call_stack)
    {
	while(cur->call_stack != cur->current)
	{
	    DL_DELETE2(cur->call_stack,
		       cur->call_stack,
		       call_stack_prev,
		       call_stack_next);
	}

	DL_DELETE2(cur->call_stack,
		   cur->call_stack,
		   call_stack_prev,
		   call_stack_next);
    }

    return INVOKE_RESULT_FINISHED;
}

static void cursor_reset(
    struct cursor_iface_t *self)
{
    struct cursor_t *cur =
	CONTAINER_OF(self, struct cursor_t, cursor);
    
    cur->current = NULL;
    cur->exit_context = NULL;
    cur->return_context = NULL;
    cur->call_stack = NULL;
    cur->cycle_start = NULL;    
}

static void cursor_destroy(
    struct cursor_iface_t *self)
{
    /* TODO: cursor destructor*/
}

struct cursor_iface_t * st_new_cursor()
{
    struct cursor_t *cur = NULL;

    ALLOC_OR_JUMP(
	cur,
	struct cursor_t,
	error_free_resources);

    cursor_reset(&(cur->cursor));

    cur->cursor.step = cursor_step;
    cur->cursor.step_in = cursor_step_in;
    cur->cursor.step_out = cursor_step_out;
    cur->cursor.reset = cursor_reset;
    cur->cursor.switch_current = cursor_switch_current;
    cur->cursor.push_return_context = cursor_push_return_context;
    cur->cursor.pop_return_context = cursor_pop_return_context;
    cur->cursor.jump_return = cursor_jump_return;
    cur->cursor.push_exit_context = cursor_push_exit_context;
    cur->cursor.pop_exit_context = cursor_pop_exit_context;
    cur->cursor.jump_exit = cursor_jump_exit;
    cur->cursor.destroy = cursor_destroy;
    
    return &(cur->cursor);

error_free_resources:
    return NULL;
}
