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

#include <linker/linker.h>
#include <elements/types.h>
#include <elements/values.h>
#include <util/bitflag.h>
#include <util/macros.h>


struct type_chain_entry_t {
    struct type_iface_t *type;
    struct type_chain_entry_t *prev;
    struct type_chain_entry_t *next;
};

static struct type_iface_t * resolve_ancestor(
    struct type_chain_entry_t *children,
    void *parent,
    const struct st_location_t *error_location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    /* Check if parent is defined */
    if(!parent)
    {
	errors->new_issue_at(
	    errors,
	    "reference to undefined type",
	    ISSUE_ERROR_CLASS,
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
	    errors->new_issue_at(
		errors,
		"circular reference",
		ISSUE_ERROR_CLASS,
		1,
		error_location);

	    return NULL;
	}
    }

    st_bitflag_t parent_type_class = parent_type->class(parent_type,
						   config);
    
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
				errors,
				config);
    }

    return parent_type;
}
    
int st_derived_type_parent_name_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    struct derived_type_t *dt =
	(struct derived_type_t *)referrer;

    dt->parent = (struct type_iface_t *)target;
    dt->ancestor = NULL;
    
    return ESSTEE_OK;
}

int st_derived_type_resolve_ancestor(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
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
						     errors,
						     config);

    if(!ancestor)
    {
	return ESSTEE_ERROR;
    }

    dt->ancestor = ancestor;

    if(dt->default_value)
    {
	if(!ancestor->can_hold)
	{
	    errors->new_issue_at(
		errors,
		"type cannot be assigned a new default value",
		ISSUE_ERROR_CLASS,
		1,
		dt->default_value_location);

	    return ESSTEE_ERROR;
	}
	
	int ancestor_can_hold_default_value =
	    ancestor->can_hold(ancestor, dt->default_value, config);

	if(ancestor_can_hold_default_value != ESSTEE_TRUE)
	{
	    errors->new_issue_at(
		errors,
		"incompatible default value",
		ISSUE_ERROR_CLASS,
		1,
		dt->default_value_location);
	    
	    return ancestor_can_hold_default_value;
	}
    }
    
    return ESSTEE_OK;
}

int st_subrange_type_storage_type_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    struct subrange_type_t *st =
	(struct subrange_type_t *)referrer;

    st->subranged_type = (struct type_iface_t *)target;

    return ESSTEE_OK;
}

int st_subrange_type_storage_type_check(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    struct subrange_type_t *st =
	(struct subrange_type_t *)referrer;

    if(!st->subranged_type)
    {
	errors->new_issue_at(
	    errors,
	    "reference to undefined type",
	    ISSUE_ERROR_CLASS,
	    1,
	    location);
	return ESSTEE_ERROR;
    }
    
    int subranged_type_can_hold_min =
	st->subranged_type->can_hold(st->subranged_type,
				     st->subrange->min,
				     config);
    if(subranged_type_can_hold_min != ESSTEE_TRUE)
    {
	errors->new_issue_at(
	    errors,
	    "the subranged type cannot hold the minimum value",
	    ISSUE_ERROR_CLASS,
	    2,
	    location,
	    st->subrange->min_location);
	return ESSTEE_ERROR;
    }    

    int subranged_type_can_hold_max =
	st->subranged_type->can_hold(st->subranged_type,
				     st->subrange->max,
				     config);
    if(subranged_type_can_hold_max != ESSTEE_TRUE)
    {
	errors->new_issue_at(
	    errors,
	    "the subranged type cannot hold the maximum value",
	    ISSUE_ERROR_CLASS,
	    2,
	    location,
	    st->subrange->max_location);
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

int st_array_type_arrayed_type_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    struct array_type_t *st =
	(struct array_type_t *)referrer;

    st->arrayed_type = (struct type_iface_t *)target;

    return ESSTEE_OK;
}

int st_array_type_arrayed_type_check(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    struct array_type_t *at =
	(struct array_type_t *)referrer;

    if(!at->arrayed_type)
    {
	errors->new_issue_at(
	    errors,
	    "reference to undefined type",
	    ISSUE_ERROR_CLASS,
	    1,
	    location);
	return ESSTEE_ERROR;
    }
    
    if(at->default_value)
    {
	return st_array_type_check_array_initializer(at->ranges,
						     at->default_value,
						     at->arrayed_type,
						     errors,
						     config);
    }
    
    return ESSTEE_OK;
}

int st_struct_element_type_name_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    if(!target)
    {
	errors->new_issue_at(
	    errors,
	    "reference to undefined type",
	    ISSUE_ERROR_CLASS,
	    1,
	    location);
	return ESSTEE_ERROR;
    }

    struct struct_element_t *se =
	(struct struct_element_t *)referrer;

    se->element_type = (struct type_iface_t *)target;
    
    return ESSTEE_OK;
}
