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

#include <elements/derived.h>
#include <util/macros.h>

#include <utlist.h>

/**************************************************************************/
/* Type interface                                                         */
/**************************************************************************/
struct derived_type_t {
    struct type_iface_t type;
    char *identifier;
    struct type_iface_t *ancestor;
    struct type_iface_t *parent;
    struct st_location_t *location;
    struct value_iface_t *default_value;
    struct st_location_t *default_value_location;
};

static struct value_iface_t * derived_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct derived_type_t *dt =
	CONTAINER_OF(self, struct derived_type_t, type);

    if(!dt->ancestor)
    {
	return NULL;
    }
    
    struct value_iface_t *new_value =
	dt->ancestor->create_value_of(dt->ancestor, config, issues);

    if(!new_value)
    {
	return NULL;
    }
    
    if(new_value->override_type)
    {
	new_value->override_type(new_value,
				 &(dt->type),
				 config,
				 issues);
    }

    return new_value;
}

static int derived_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct derived_type_t *dt =
	CONTAINER_OF(self, struct derived_type_t, type);

    if(!dt->ancestor)
    {
	return ESSTEE_ERROR;
    }
    
    int reset_result = dt->ancestor->reset_value_of(dt->ancestor,
						    value_of,
						    config,
						    issues);
    if(reset_result != ESSTEE_OK)
    {
	return reset_result;
    }

    if(dt->default_value)
    {
	reset_result = value_of->assign(value_of,
					dt->default_value,
					config,
					issues);
    }

    if(reset_result != ESSTEE_OK)
    {
	return reset_result;
    }
    
    return ESSTEE_OK;
}

static void derived_type_sync_direct_memory(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct direct_address_t *address,
    int write)
{
    struct derived_type_t *dt =
	CONTAINER_OF(self, struct derived_type_t, type);

    dt->ancestor->sync_direct_memory(dt->ancestor,
				     value_of,
				     address,
				     write);
}

static int derived_type_validate_direct_address(
    const struct type_iface_t *self,
    struct direct_address_t *address,
    struct issues_iface_t *issues)
{
    struct derived_type_t *dt =
	CONTAINER_OF(self, struct derived_type_t, type);

    if(!dt->ancestor->validate_direct_address)
    {
	const char *type_name = (dt->ancestor->identifier) ?
	    dt->ancestor->identifier : "(no explicit type name)";

	const char *message;
	if(dt->type.identifier)
	{
	    message = issues->build_message(
		issues,
		"derived type '%s' does not support direct memory storage, its ancestor type '%s' lacks support",
		dt->type.identifier,
		type_name);
	}
	else
	{
	    message = issues->build_message(
		issues,
		"type '%s' does not support direct memory storage",
		dt->ancestor->identifier,
		type_name);
	}
	
	issues->new_issue(issues,
			  message,
			  ESSTEE_TYPE_ERROR);

	return ESSTEE_ERROR;
    }
    
    return dt->ancestor->validate_direct_address(
	dt->ancestor,
	address,
	issues);
}

static int derived_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct derived_type_t *dt =
	CONTAINER_OF(self, struct derived_type_t, type);

    if(!dt->ancestor)
    {
	return ESSTEE_FALSE;
    }
    
    int can_hold_result = dt->ancestor->can_hold(dt->ancestor,
						 value,
						 config,
						 issues);

    if(can_hold_result != ESSTEE_TRUE)
    {
	issues->new_issue(
	    issues,
	    "derived type '%s' cannot hold value due to ancestor limitations",
	    ESSTEE_TYPE_ERROR,
	    self->identifier);

	return can_hold_result;
    }

    return ESSTEE_TRUE;
}

static int derived_type_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct derived_type_t *dt =
	CONTAINER_OF(self, struct derived_type_t, type);

    if(!dt->ancestor)
    {
	return ESSTEE_FALSE;
    }
    
    return dt->ancestor->compatible(dt->ancestor,
				    other_type,
				    config,
				    issues);
}

static st_bitflag_t derived_type_class(
    const struct type_iface_t *self)
{
    struct derived_type_t *dt =
	CONTAINER_OF(self, struct derived_type_t, type);

    st_bitflag_t ancestor_class = 0;
    
    if(dt->ancestor)
    {
	ancestor_class = dt->ancestor->class(dt->ancestor);
    }
    
    return ancestor_class|DERIVED_TYPE;    
}

static void derived_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: derived type destructor */
}

/**************************************************************************/
/* Linker callbacks                                                       */
/**************************************************************************/
struct type_chain_entry_t {
    struct type_iface_t *type;
    struct type_chain_entry_t *prev;
    struct type_chain_entry_t *next;
};

static struct type_iface_t * resolve_ancestor(
    struct type_chain_entry_t *children,
    void *parent,
    const struct st_location_t *error_location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    /* Check if parent is defined */
    if(!parent)
    {
	issues->new_issue_at(
	    issues,
	    "reference to undefined type",
	    ESSTEE_TYPE_ERROR,
	    1,
	    error_location);

	return NULL;
    }

    struct type_iface_t *parent_type =
	(struct type_iface_t *)parent;
    
    /* Check if parent is among children -> circular ref. */
    struct type_chain_entry_t *child = NULL;
    DL_FOREACH(children, child)
    {
	if(child->type == parent_type)
	{
	    const char *message = issues->build_message(
		issues,
		"circular reference found while resolving type '%s'",
		parent_type->identifier);
	    
	    issues->new_issue_at(
		issues,
		message,
		ESSTEE_CONTEXT_ERROR,
		1,
		error_location);

	    return NULL;
	}
    }

    st_bitflag_t parent_type_class = parent_type->class(parent_type);
    
    if(ST_FLAG_IS_SET(parent_type_class, DERIVED_TYPE))
    {
	/* Stays on the stack for the whole recursive call, no need to
	 * malloc */
	struct type_chain_entry_t parent_entry = {
	    .type = parent_type,
	};

	DL_APPEND(children, &parent_entry);

	struct derived_type_t *pdt =
	    CONTAINER_OF(parent_type, struct derived_type_t, type);

	struct type_iface_t *next_parent = NULL;

	if(pdt->ancestor)
	{
	    next_parent = pdt->ancestor;
	}
	else
	{
	    next_parent = pdt->parent;
	}
	
	return resolve_ancestor(children,
				next_parent,
				error_location,
				config,
				issues);
    }

    return parent_type;
}
    
static int derived_type_parent_name_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct derived_type_t *dt =
	(struct derived_type_t *)referrer;

    dt->parent = (struct type_iface_t *)target;
    dt->ancestor = NULL;
    
    return ESSTEE_OK;
}

static int derived_type_resolve_ancestor(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct derived_type_t *dt =
	(struct derived_type_t *)referrer;

    struct type_chain_entry_t start;
    start.type = &(dt->type);

    struct type_chain_entry_t *children = NULL;
    DL_APPEND(children, &start);
    
    struct type_iface_t *ancestor = resolve_ancestor(children,
						     target,
						     location,
						     config,
						     issues);

    if(!ancestor)
    {
	return ESSTEE_ERROR;
    }

    dt->ancestor = ancestor;

    if(dt->default_value)
    {
	if(!ancestor->can_hold)
	{
	    const char *message = issues->build_message(
		issues,
		"type '%s' cannot have a default value",
		dt->type.identifier);
	    
	    issues->new_issue_at(
		issues,
		message,
		ESSTEE_CONTEXT_ERROR,
		1,
		dt->default_value_location);

	    return ESSTEE_ERROR;
	}

	issues->begin_group(issues);
	int ancestor_can_hold_default_value =
	    ancestor->can_hold(ancestor, dt->default_value, config, issues);
	issues->set_group_location(issues,
				   1,
				   dt->default_value_location);
	
	if(ancestor_can_hold_default_value != ESSTEE_TRUE)
	{
	    return ESSTEE_ERROR;
	}
    }
    
    return ESSTEE_OK;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct type_iface_t * st_create_derived_type(
    char *type_name,
    struct type_iface_t *parent_type,
    const struct st_location_t *location,
    struct value_iface_t *default_value,
    const struct st_location_t *default_value_location,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct derived_type_t *dt = NULL;
    struct st_location_t *dt_location = NULL;
    struct st_location_t *dv_location = NULL;
	
    ALLOC_OR_ERROR_JUMP(
	dt,
	struct derived_type_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	dt_location,
	location,
	issues,
	error_free_resources);

    if(default_value_location)
    {
	LOCDUP_OR_ERROR_JUMP(
	    dv_location,
	    default_value_location,
	    issues,
	    error_free_resources);
    }

    dt->identifier = type_name;
    dt->location = dt_location;
    dt->ancestor = parent_type;
    dt->parent = parent_type;
    
    dt->default_value = default_value;
    dt->default_value_location = dv_location;

    memset(&(dt->type), 0, sizeof(struct type_iface_t));
    dt->type.identifier = dt->identifier;
    dt->type.location = dt->location;
    dt->type.create_value_of = derived_type_create_value_of;
    dt->type.reset_value_of = derived_type_reset_value_of;
    dt->type.can_hold = derived_type_can_hold;
    dt->type.validate_direct_address = derived_type_validate_direct_address;
    dt->type.sync_direct_memory = derived_type_sync_direct_memory;
    dt->type.compatible = derived_type_compatible;
    dt->type.class = derived_type_class;
    dt->type.destroy = derived_type_destroy;

    return &(dt->type);

error_free_resources:
    free(dt_location);
    free(dv_location);
    return NULL;
}

struct type_iface_t * st_create_derived_type_by_name(
    char *type_name,
    char *parent_type_name,
    const struct st_location_t *parent_type_name_location,
    const struct st_location_t *location,
    struct value_iface_t *default_value,
    const struct st_location_t *default_value_location,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct derived_type_t *dt = NULL;
    struct st_location_t *dt_location = NULL;
    struct st_location_t *dv_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	dt,
	struct derived_type_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	dt_location,
	location,
	issues,
	error_free_resources);

    if(default_value_location)
    {
	LOCDUP_OR_ERROR_JUMP(
	    dv_location,
	    default_value_location,
	    issues,
	    error_free_resources);
    }
    
    int ref_add_result = type_refs->add_two_step(
	type_refs,
	parent_type_name,
	dt,
	parent_type_name_location,
	derived_type_parent_name_resolved,
	derived_type_resolve_ancestor,
	issues);

    if(ref_add_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    dt->identifier = type_name;
    dt->location = dt_location;
    dt->ancestor = NULL;
    dt->parent = NULL;
    dt->default_value = default_value;
    dt->default_value_location = dv_location;

    memset(&(dt->type), 0, sizeof(struct type_iface_t));    
    dt->type.identifier = dt->identifier;
    dt->type.location = dt->location;
    dt->type.create_value_of = derived_type_create_value_of;
    dt->type.reset_value_of = derived_type_reset_value_of;
    dt->type.can_hold = derived_type_can_hold;
    dt->type.compatible = derived_type_compatible;
    dt->type.class = derived_type_class;
    dt->type.destroy = derived_type_destroy;
    
    return &(dt->type);

error_free_resources:
    free(dt_location);
    free(dv_location);
    return NULL;
}
