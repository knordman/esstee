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

#include <statements/empty.h>
#include <util/macros.h>

#include <utlist.h>

/**************************************************************************/
/* Invoke interface                                                       */
/**************************************************************************/
struct empty_statement_t {
    struct invoke_iface_t invoke;
    struct st_location_t *location;
};

static int empty_statement_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}

static int empty_statement_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return INVOKE_RESULT_FINISHED;
}

static int empty_statement_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}

static void empty_statement_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: destructor */
}

static void empty_statement_clone_destroy(
    struct invoke_iface_t *self)
{
    /* TODO: destructor */
}

static struct invoke_iface_t * empty_statement_clone(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct empty_statement_t *es =
	CONTAINER_OF(self, struct empty_statement_t, invoke);

    struct empty_statement_t *copy = NULL;
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct empty_statement_t,
	issues,
	error_free_resources);

    memcpy(copy, es, sizeof(struct empty_statement_t));
    copy->invoke.destroy = empty_statement_clone_destroy;

    return &(copy->invoke);
    
error_free_resources:
    return NULL;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct invoke_iface_t * st_append_empty_statement(
    struct invoke_iface_t *statement_list,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct empty_statement_t *es = NULL;
    struct st_location_t *es_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	es,
	struct empty_statement_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	es_location,
	location,
	issues,
	error_free_resources);

    es->location = es_location;
    es->invoke.verify = empty_statement_verify;
    es->invoke.step = empty_statement_step;
    es->invoke.location = es->location;
    es->invoke.clone = empty_statement_clone;
    es->invoke.reset = empty_statement_reset;
    es->invoke.destroy = empty_statement_destroy;

    DL_APPEND(statement_list, &(es->invoke));
    return statement_list;

error_free_resources:
    free(es);
    free(es_location);
    return NULL;
}
