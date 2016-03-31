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

#include <utlist.h>
#include <stdlib.h>


void st_switch_current(
    struct cursor_t *cursor,
    struct invoke_iface_t *switch_to,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    /* Prepare a new run of the invoke switched to */
    switch_to->reset(switch_to, config, issues);
    
    DL_PREPEND2(cursor->call_stack,
		cursor->current,
		call_stack_prev,
		call_stack_next);
    cursor->current = switch_to;
}

void st_push_return_context(
    struct cursor_t *cursor,
    struct invoke_iface_t *context)
{
    DL_PREPEND2(
	cursor->return_context,
	context,
	return_context_prev,
	return_context_next);
}

void st_pop_return_context(
    struct cursor_t *cursor)
{
    if(cursor->return_context)
    {
	DL_DELETE2(
	    cursor->return_context,
	    cursor->return_context,
	    return_context_prev,
	    return_context_next);
    }
}

int st_jump_return(
    struct cursor_t *cursor)
{
    if(cursor->return_context)
    {
	cursor->current = cursor->return_context;
	st_pop_return_context(cursor);

	if(cursor->call_stack)
	{
	    while(cursor->call_stack != cursor->current)
	    {
		DL_DELETE2(cursor->call_stack,
			   cursor->call_stack,
			   call_stack_prev,
			   call_stack_next);
	    }

	    DL_DELETE2(cursor->call_stack,
		       cursor->call_stack,
		       call_stack_prev,
		       call_stack_next);
	}

	return INVOKE_RESULT_FINISHED;
    }

    return INVOKE_RESULT_ALL_FINISHED;
}

void st_push_exit_context(
    struct cursor_t *cursor,
    struct invoke_iface_t *context)
{
    DL_PREPEND2(
	cursor->exit_context,
	context,
	exit_context_prev,
	exit_context_next);
}

void st_pop_exit_context(
    struct cursor_t *cursor)
{
    if(cursor->exit_context)
    {
	DL_DELETE2(
	    cursor->exit_context,
	    cursor->exit_context,
	    exit_context_prev,
	    exit_context_next);
    }
}

void st_jump_exit(
    struct cursor_t *cursor)
{
    cursor->current = cursor->exit_context;
    st_pop_return_context(cursor);

    if(cursor->call_stack)
    {
	while(cursor->call_stack != cursor->current)
	{
	    DL_DELETE2(cursor->call_stack,
		       cursor->call_stack,
		       call_stack_prev,
		       call_stack_next);
	}

	DL_DELETE2(cursor->call_stack,
		   cursor->call_stack,
		   call_stack_prev,
		   call_stack_next);
    }
}
