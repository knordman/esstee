/*
Copyright (C) 2016 Kristian Nordman

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

#include <esstee/elements.h>
#include <api/elementnode.h>
#include <elements/itype.h>


static st_bitflag_t element_type_class(
    const struct st_element_t *self)
{


    return 0;
}

static const char * element_type_identifier(
    const struct st_element_t *self)
{


    return NULL;
}

static int element_display(
    const struct st_element_t *self,
    char *buffer,
    size_t buffer_size)
{
    struct element_node_t *en =
	CONTAINER_OF(self, struct element_node_t, element);

    return en->value->display(en->value, buffer, buffer_size, en->context->config);
}

static struct st_element_t * element_sub_element(
    struct st_element_t *self,
    const char *identifier)
{
    return NULL;
}

/* static struct st_element_t * element_sub_item( */
/*     struct st_element_t *self, */
/*     const char *identifier) */
/* { */
/*     return NULL; */
/* } */

static int64_t element_as_integer(
    struct st_element_t *self)
{
    struct element_node_t *en =
	CONTAINER_OF(self, struct element_node_t, element);

    if(!en->value->integer)
    {
	en->context->issues->new_issue(en->context->issues,
				       "cannot be interpreted as an integer",
				       ESSTEE_TYPE_ERROR);
	return 0;
    }

    return en->value->integer(en->value, en->context->config, en->context->issues);
}

static int element_as_bool(
    struct st_element_t *self)
{
    return 0;
}

static double element_as_real(
    struct st_element_t *self)
{
    return 0.0;
}

static const char * element_as_text(
    struct st_element_t *self)
{
    return "";
}

static int element_as_duration(
    struct st_element_t *self,
    struct st_duration_t *output)
{
    return 0;
}
    
static int element_as_date_tod(
    struct st_element_t *self,
    struct st_date_tod_t *output)
{
    return 0;
}

static int element_set_by_integer(
    struct st_element_t *self,
    int64_t integer)
{
    return 0;
}

static int element_set_by_bool(
    struct st_element_t *self,
    int bool)
{
    return 0;
}
    
static int element_set_by_real(
    struct st_element_t *self,
    double real)
{
    return 0;
}

static int element_set_by_text(
    struct st_element_t *self,
    const char *text)
{
    return 0;
}

static int element_set_by_duration(
    struct st_element_t *self,
    const struct st_duration_t *duration)
{
    return 0;
}

static int element_set_by_date_tod(
    struct st_element_t *self,
    const struct st_date_tod_t *date_tod)
{
    return 0;
}

struct element_node_t * st_new_element_node(
    const char *identifier,
    struct value_iface_t *value,
    const struct element_node_context_t *context)
{
    struct element_node_t *en = NULL;
    char *identifier_copy = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	en,
	struct element_node_t,
	context->issues,
	error_free_resources);

    STRDUP_OR_ERROR_JUMP(
	identifier_copy,
	identifier,
	context->issues,
	error_free_resources);
    
    en->identifier = identifier_copy;
    en->value = value;
    en->sub_nodes = NULL;
    en->context = context;

    en->element.type_class = element_type_class;
    en->element.type_identifier = element_type_identifier;
    en->element.display = element_display;
    en->element.sub_element = element_sub_element;
    en->element.as_integer = element_as_integer;
    en->element.as_bool = element_as_bool;
    en->element.as_real = element_as_real;
    en->element.as_text = element_as_text;
    en->element.as_duration = element_as_duration;
    en->element.as_date_tod = element_as_date_tod;
    en->element.set_by_integer = element_set_by_integer;
    en->element.set_by_bool = element_set_by_bool;
    en->element.set_by_real = element_set_by_real;
    en->element.set_by_text = element_set_by_text;
    en->element.set_by_duration = element_set_by_duration;
    en->element.set_by_date_tod = element_set_by_date_tod;

    return en;
    
error_free_resources:
    free(en);
    free(identifier_copy);
    return NULL;
}

