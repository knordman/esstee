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

#include <expressions/identifier.h>
#include <elements/enums.h>
#include <util/macros.h>
#include <elements/ivariable.h>

struct single_identifier_term_t {
    struct expression_iface_t expression;
    char *identifier;
    struct st_location_t *location;
    
    /* Identifier term refers to an enum value */
    struct value_iface_t value;
    struct enum_item_t item;

    /* Identifier term refers to a variable */
    struct variable_iface_t *variable;
};

/**************************************************************************/
/* Expression interface                                                   */
/**************************************************************************/
static const struct value_iface_t * identifier_term_variable_return_value(
    struct expression_iface_t *self)
{
    struct single_identifier_term_t *sit =
	CONTAINER_OF(self, struct single_identifier_term_t, expression);

    return sit->variable->value(sit->variable);
}

static void identifier_term_destroy(
    struct expression_iface_t *self)
{
    /* TODO: single identifier term destructor */
}

static void identfier_term_clone_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

static struct expression_iface_t * identifier_term_variable_clone(
    struct expression_iface_t *self,
    struct issues_iface_t *issues)
{
    struct single_identifier_term_t *sit =
	CONTAINER_OF(self, struct single_identifier_term_t, expression);

    struct single_identifier_term_t *copy = NULL;
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct single_identifier_term_t,
	issues,
	error_free_resources);

    memcpy(copy, sit, sizeof(struct single_identifier_term_t));
    copy->expression.destroy = identfier_term_clone_destroy;

    return &(copy->expression);

error_free_resources:
    return NULL;
}

static const struct value_iface_t * identifier_term_enum_return_value(
    struct expression_iface_t *self)
{
    struct single_identifier_term_t *sit = 
	CONTAINER_OF(self, struct single_identifier_term_t, expression);

    return &(sit->value);
}

/**************************************************************************/
/* Value interface                                                        */
/**************************************************************************/
static int enum_identifier_value_comparable_to(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->enumeration)
    {
	issues->new_issue(
	    issues,
	    "the other value cannot be interpreted as an enumerated value",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

static int enum_identifier_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct single_identifier_term_t *sit =
	CONTAINER_OF(self, struct single_identifier_term_t, value);

    const struct enum_item_t *other_value_enum =
	other_value->enumeration(other_value, config, issues);
    
    if(strcmp(sit->identifier, other_value_enum->identifier) == 0)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

static const struct enum_item_t * enum_identifier_value_enumeration(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct single_identifier_term_t *sit =
	CONTAINER_OF(self, struct single_identifier_term_t, value);

    return &(sit->item);
}

static void enum_identifier_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: inline enum value destructor */
}

/**************************************************************************/
/* Linker callbacks                                                       */
/**************************************************************************/
static int identifier_variable_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct single_identifier_term_t *sit
	= (struct single_identifier_term_t *)referrer;
    
    if(target != NULL)
    {
	sit->variable = (struct variable_iface_t *)target;
    }
    else
    {
	/* Interpret as an inline enum value */
	sit->variable = NULL;
	
	sit->expression.return_value = identifier_term_enum_return_value;
	sit->expression.clone = NULL;

	sit->item.identifier = sit->identifier;
	sit->item.location = sit->location;
	
	memset(&(sit->value), 0, sizeof(struct value_iface_t));
	sit->value.comparable_to = enum_identifier_value_comparable_to;
	sit->value.equals = enum_identifier_value_equals;
	sit->value.enumeration = enum_identifier_value_enumeration;
	sit->value.destroy = enum_identifier_value_destroy;
    }

    return ESSTEE_OK;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct expression_iface_t * st_create_identifier_term(
    char *identifier,
    const struct st_location_t *location,
    struct named_ref_pool_iface_t *var_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct single_identifier_term_t *sit = NULL;
    ALLOC_OR_ERROR_JUMP(
	sit,
	struct single_identifier_term_t,
	issues,
	error_free_resources);

    struct st_location_t *sit_location = NULL;
    LOCDUP_OR_ERROR_JUMP(
	sit_location,
	location,
	issues,
	error_free_resources);

    int var_ref_add_result = var_refs->add(
	var_refs,
	identifier,
	sit,
	location,
        identifier_variable_resolved,
	issues);

    if(var_ref_add_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    sit->variable = NULL;
    sit->location = sit_location;
    sit->identifier = identifier;

    memset(&(sit->expression), 0, sizeof(struct expression_iface_t));

    sit->expression.invoke.location = sit->location;
    sit->expression.destroy = identifier_term_destroy;
    sit->expression.return_value = identifier_term_variable_return_value;
    sit->expression.clone = identifier_term_variable_clone;

    return &(sit->expression);
    
error_free_resources:
    free(sit);
    free(sit_location);
    return NULL;
}
