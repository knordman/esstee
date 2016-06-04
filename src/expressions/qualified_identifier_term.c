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

#include <expressions/qualified_identifier_term.h>
#include <util/macros.h>

/**************************************************************************/
/* Expression interface                                                   */
/**************************************************************************/
struct qualified_identifier_term_t {
    struct expression_iface_t expression;
    struct qualified_identifier_iface_t *qid;
    struct st_location_t *location;
};

static int qualified_identifier_term_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct qualified_identifier_term_t *qit
	= CONTAINER_OF(expr, struct qualified_identifier_term_t, expression);

    return qit->qid->verify(qit->qid, config, issues);
}

static int qualified_identifier_term_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct qualified_identifier_term_t *qit
	= CONTAINER_OF(expr, struct qualified_identifier_term_t, expression);

    return qit->qid->allocate(qit->qid, issues);
}


static int qualified_identifier_term_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct qualified_identifier_term_t *qit
	= CONTAINER_OF(expr, struct qualified_identifier_term_t, expression);

    return qit->qid->step(qit->qid,
			  cursor,
			  time,
			  config,
			  issues);
}

static int qualified_identifier_term_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct qualified_identifier_term_t *qit
	= CONTAINER_OF(expr, struct qualified_identifier_term_t, expression);

    return qit->qid->reset(qit->qid, config, issues);
}

static const struct value_iface_t * qualified_identifier_term_return_value(
    struct expression_iface_t *self)
{
    struct qualified_identifier_term_t *qit =
	CONTAINER_OF(self, struct qualified_identifier_term_t, expression);

    return qit->qid->target(qit->qid);
}

static struct expression_iface_t * qualified_identifier_term_clone(
    struct expression_iface_t *self,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_term_t *qit
	= CONTAINER_OF(self, struct qualified_identifier_term_t, expression);

    struct qualified_identifier_term_t *copy = NULL;
    struct qualified_identifier_iface_t *qid_copy = NULL;

    ALLOC_OR_ERROR_JUMP(
	copy,
	struct qualified_identifier_term_t,
	issues,
	error_free_resources);

    memcpy(copy, qit, sizeof(struct qualified_identifier_term_t));

    qid_copy = qit->qid->clone(qit->qid, issues);

    if(!qid_copy)
    {
	goto error_free_resources;
    }

    copy->qid = qid_copy;
    
    return &(copy->expression);

error_free_resources:
    free(copy);
    return NULL;
}

static void qualified_identifier_term_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor qualified identifier term */
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct expression_iface_t * st_create_qualified_identifier_term(
    struct qualified_identifier_iface_t *qualified_identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct qualified_identifier_term_t *qt = NULL;
    struct st_location_t *qt_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	qt,
	struct qualified_identifier_term_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	qt_location,
	location,
	issues,
	error_free_resources);

    qt->location = qt_location;
    qt->qid = qualified_identifier;

    memset(&(qt->expression), 0, sizeof(struct expression_iface_t));
    qt->expression.invoke.verify = qualified_identifier_term_verify;
    qt->expression.invoke.allocate = qualified_identifier_term_allocate;
    qt->expression.invoke.step = qualified_identifier_term_step;
    qt->expression.invoke.reset = qualified_identifier_term_reset;

    qt->expression.invoke.location = qt->location;
    qt->expression.return_value = qualified_identifier_term_return_value;
    qt->expression.destroy = qualified_identifier_term_destroy;
    qt->expression.clone = qualified_identifier_term_clone;

    return &(qt->expression);
    
error_free_resources:
    free(qt);
    free(qt_location);
    return NULL;
}
