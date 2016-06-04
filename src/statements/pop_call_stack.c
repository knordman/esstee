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

#include <statements/pop_call_stack.h>
#include <util/macros.h>

/**************************************************************************/
/* Invoke interface                                                       */
/**************************************************************************/
struct pop_call_stack_statement_t {
    struct invoke_iface_t invoke;
    struct st_location_t *location;
    int has_loop_parent;
};

int return_statement_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return cursor->jump_return(cursor);
}

int exit_statement_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return cursor->jump_exit(cursor);
}

int pop_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}

int pop_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}

struct invoke_iface_t * pop_statement_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct pop_call_stack_statement_t *ps = 
	CONTAINER_OF(self, struct pop_call_stack_statement_t, invoke);

    struct pop_call_stack_statement_t *copy = NULL;
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct pop_call_stack_statement_t,
	issues,
	error_free_resources);

    memcpy(copy, ps, sizeof(struct pop_call_stack_statement_t));

    return &(copy->invoke);
    
error_free_resources:
    return NULL;
}

void pop_statement_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: pop callstack statement destroy */
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct invoke_iface_t * st_create_exit_statement(
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct pop_call_stack_statement_t *ps = NULL;
    struct st_location_t *ps_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	ps,
	struct pop_call_stack_statement_t,
	issues,
	error_free_resources);
    
    LOCDUP_OR_ERROR_JUMP(
	ps_location,
	location,
	issues,
	error_free_resources);

    ps->location = ps_location;

    memset(&(ps->invoke), 0, sizeof(struct invoke_iface_t));
    ps->invoke.location = ps->location;
    ps->invoke.step = exit_statement_step;
    ps->invoke.verify = pop_statement_verify;
    ps->invoke.reset = pop_statement_reset;
    ps->invoke.clone = pop_statement_clone;
    ps->invoke.destroy = pop_statement_destroy;

    return &(ps->invoke);

error_free_resources:
    free(ps);
    free(ps_location);
    return NULL;
}

struct invoke_iface_t * st_create_return_statement(
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct invoke_iface_t *rs = st_create_exit_statement(location,
							 config,
							 issues);

    struct pop_call_stack_statement_t *ps =
	CONTAINER_OF(rs, struct pop_call_stack_statement_t, invoke);

    if(ps)
    {
	ps->invoke.step = return_statement_step;
    }
    
    return rs;
}
