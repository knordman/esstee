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

#include <expressions/inline_expression.h>
#include <util/macros.h>

/**************************************************************************/
/* Expression interface                                                   */
/**************************************************************************/
struct value_expression_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct value_iface_t *value;
};

static const struct value_iface_t * value_expression_return_value(
    const struct expression_iface_t *self)
{
    const struct value_expression_t *ve
	= CONTAINER_OF(self, struct value_expression_t, expression);

    return ve->value;
}

static void value_expression_destroy(
    struct expression_iface_t *self)
{
    /* TODO: value expression destructor */
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct expression_iface_t * st_create_value_expression(
    struct value_iface_t *value,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct value_expression_t *ve = NULL;
    struct st_location_t *value_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	ve,
	struct value_expression_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	value_location,
	location,
	issues,
	error_free_resources);

    ve->location = value_location;
    ve->value = value;

    memset(&(ve->expression), 0, sizeof(struct expression_iface_t));
    
    ve->expression.invoke.location = ve->location;    
    ve->expression.return_value = value_expression_return_value;
    ve->expression.destroy = value_expression_destroy;

    return &(ve->expression);
    
error_free_resources:
    free(ve);
    return NULL;    
}
