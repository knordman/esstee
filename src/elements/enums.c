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

#include <elements/enums.h>
#include <elements/values.h>
#include <util/macros.h>

#include <stdio.h>

/**************************************************************************/
/* Enum group interface                                                   */
/**************************************************************************/
struct enum_group_item_t {
    char *identifier;
    struct st_location_t *location;
    struct enum_item_t item;
    UT_hash_handle hh;
};

struct enum_group_t {
    struct enum_group_iface_t group;
    struct enum_group_item_t *items;
};

static int enum_group_extend(
    struct enum_group_iface_t *self,
    char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct enum_group_t *eg =
	CONTAINER_OF(self, struct enum_group_t, group);
    
    struct enum_group_item_t *ei = NULL;
    struct st_location_t *ei_location = NULL;

    struct enum_group_item_t *found = NULL;
    HASH_FIND_STR(eg->items, identifier, found);
    if(found)
    {
	const char *message = issues->build_message(
	    issues,
	    "multiple definition of enumerated value '%s'",
	    identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_CONTEXT_ERROR,
	    2,
	    location,
	    found->location);
	
	goto error_free_resources;
    }

    ALLOC_OR_ERROR_JUMP(
	ei,
	struct enum_group_item_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	ei_location,
	location,
	issues,
	error_free_resources);

    ei->identifier = identifier;
    ei->location = ei_location;
    ei->item.identifier = identifier;
    ei->item.location = location;
    
    HASH_ADD_KEYPTR(hh, 
		    eg->items, 
		    ei->identifier, 
		    strlen(ei->identifier), 
		    ei);

    return ESSTEE_OK;
    
error_free_resources:
    free(ei);
    free(ei_location);
    return ESSTEE_ERROR;
}

static void enum_group_destroy(
    struct enum_group_iface_t *self)
{
    /* TODO: enum group destroy */
}

struct enum_group_iface_t * st_create_enum_group(
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct enum_group_t *eg = NULL;

    ALLOC_OR_ERROR_JUMP(
	eg,
	struct enum_group_t,
	issues,
	error_free_resources);

    memset(&(eg->group), 0, sizeof(struct enum_group_iface_t));
    eg->group.extend = enum_group_extend;
    eg->group.destroy = enum_group_destroy;

    return &(eg->group);

error_free_resources:
    return NULL;
}


/**************************************************************************/
/* Value interface                                                        */
/**************************************************************************/
struct enum_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    const struct enum_item_t *constant;
};

static int enum_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    int written_bytes = snprintf(buffer,
				 buffer_size,
				 "%s",
				 ev->constant->identifier);

    CHECK_WRITTEN_BYTES(written_bytes);

    return written_bytes;
}

static int enum_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);
    
    ev->constant = new_value->enumeration(new_value,
					  config,
					  issues);

    return ESSTEE_OK;
}

static const struct type_iface_t * enum_value_type_of(
    const struct value_iface_t *self)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    return ev->type;
}

static int enum_value_assigns_and_compares(
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

    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    /* Check that the enumeration of the other value is present by the
     * values defined by the type */
    int type_can_hold = ev->type->can_hold(ev->type,
					   other_value,
					   config,
					   issues);

    if(type_can_hold != ESSTEE_TRUE)
    {
	return type_can_hold;
    }

    /* Check that the type of the other value is compatible with the
     * value type */
    if(other_value->type_of)
    {
	const struct type_iface_t *other_type =
	    other_value->type_of(other_value);

	return ev->type->compatible(ev->type,
				    other_type,
				    config,
				    issues);
    }

    return ESSTEE_TRUE;
}

static void enum_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: enum value destructor */
}

static int enum_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    const struct enum_item_t *other_value_enum =
	other_value->enumeration(other_value,
				 config,
				 issues);
    
    if(strcmp(ev->constant->identifier, other_value_enum->identifier) == 0)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

static const struct enum_item_t * enum_value_enumeration(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    return ev->constant;
}

static int enum_value_override_type(
    const struct value_iface_t *self,
    const struct type_iface_t *type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    ev->type = type;
    ev->value.type_of = enum_value_type_of;

    return ESSTEE_OK;
}

/**************************************************************************/
/* Type interface                                                         */
/**************************************************************************/
struct enum_type_t {
    struct type_iface_t type;
    struct enum_group_t *values;
    const struct enum_group_item_t *default_item;
};

static struct value_iface_t * enum_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct enum_value_t *ev = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	ev,
	struct enum_value_t,
	issues,
	error_free_resources);
    
    struct enum_type_t *et =
	CONTAINER_OF(self, struct enum_type_t, type);

    ev->type = self;
    ev->constant = &(et->default_item->item);

    memset(&(ev->value), 0, sizeof(struct value_iface_t));

    ev->value.display = enum_value_display;
    ev->value.assign = enum_value_assign;
    ev->value.type_of = enum_value_type_of;
    ev->value.assignable_from = enum_value_assigns_and_compares;
    ev->value.comparable_to = enum_value_assigns_and_compares;
    ev->value.destroy = enum_value_destroy;
    ev->value.equals = enum_value_equals;
    ev->value.enumeration = enum_value_enumeration;
    ev->value.class = st_general_value_empty_class;
    ev->value.override_type = enum_value_override_type;

    return &(ev->value);
    
error_free_resources:
    return NULL;
}

static int enum_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct enum_type_t *et =
	CONTAINER_OF(self, struct enum_type_t, type);

    struct enum_value_t *ev =
	CONTAINER_OF(value_of, struct enum_value_t, value);

    ev->constant = &(et->default_item->item);

    return ESSTEE_OK;
}

static int enum_type_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(self == other_type)
    {
	return ESSTEE_TRUE;
    }

    struct enum_type_t *et =
	CONTAINER_OF(self, struct enum_type_t, type);

    const struct type_iface_t *otype =
	(other_type->ancestor) ? other_type->ancestor(other_type) : other_type;

    if(otype->class(otype) != ENUM_TYPE)
    {
	issues->new_issue(
	    issues,
	    "other type is not an enumerated type",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }

    struct enum_type_t *oet =
	CONTAINER_OF(otype, struct enum_type_t, type);
    
    struct enum_group_item_t *o_itr = oet->values->items;
    struct enum_group_item_t *e_itr = et->values->items;
    for(; o_itr != NULL; o_itr = o_itr->hh.next, e_itr = e_itr->hh.next)
    {
	if(strcmp(o_itr->identifier, e_itr->identifier) != 0)
	{
	    break;
	}
    }

    if(o_itr != NULL || e_itr != NULL)
    {
	issues->new_issue(
	    issues,
	    "compatible enumerated types require that all enumerated values match",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }
	    
    return ESSTEE_TRUE;
}

static int enum_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!value->enumeration)
    {
	issues->new_issue(
	    issues,
	    "enumerated type can only hold enumerated values",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }

    struct enum_type_t *et =
	CONTAINER_OF(self, struct enum_type_t, type);

    const struct enum_item_t *vei =
	value->enumeration(value, config, issues);
    
    struct enum_group_item_t *found = NULL;
    HASH_FIND_STR(et->values->items, vei->identifier, found);
    if(!found)
    {
	issues->new_issue(
	    issues,
	    "enumerated value '%s' is not part of the enumerated type",
	    ESSTEE_TYPE_ERROR,
	    vei->identifier);

	return ESSTEE_FALSE;
    }

    if(value->type_of)
    {
	const struct type_iface_t *value_type = value->type_of(value);

	return self->compatible(self,
				value_type,
				config,
				issues);
    }
    
    return ESSTEE_TRUE;
}

static st_bitflag_t enum_type_class(
    const struct type_iface_t *self)
{
    return ENUM_TYPE;
}

static void enum_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: enum type destructor */
}

/**************************************************************************/
/* Public functions                                                       */
/**************************************************************************/
struct type_iface_t * st_create_enum_type(
    struct enum_group_iface_t *value_group, 
    const char *default_item,
    const struct st_location_t *default_item_location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct enum_type_t *et = NULL;

    ALLOC_OR_ERROR_JUMP(
	et,
	struct enum_type_t,
	issues,
	error_free_resources);

    struct enum_group_t *eg =
	CONTAINER_OF(value_group, struct enum_group_t, group);
    
    struct enum_group_item_t *default_value = eg->items;
    if(default_item != NULL)
    {
	HASH_FIND_STR(eg->items, default_item, default_value);
	if(default_value == NULL)
	{
	    issues->new_issue_at(
		issues,
		"enumeration value not found",
		ISSUE_ERROR_CLASS,
		1,
		default_item_location);

	    goto error_free_resources;
	}
    }     

    et->default_item = default_value;
    et->values = eg;

    et->type.identifier = NULL;
    et->type.location = NULL;
    et->type.create_value_of = enum_type_create_value_of;
    et->type.reset_value_of = enum_type_reset_value_of;
    et->type.sync_direct_memory = NULL;
    et->type.validate_direct_address = NULL;
    et->type.can_hold = enum_type_can_hold;
    et->type.compatible = enum_type_compatible;
    et->type.class = enum_type_class;
    et->type.destroy = enum_type_destroy;

    return &(et->type);
    
error_free_resources:
    free(et);
    return NULL;
}

struct value_iface_t * st_create_enum_value(
    char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return enum_type_create_value_of(NULL,
				     config,
				     issues);
}
