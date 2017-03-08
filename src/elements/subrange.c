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

#include <elements/subrange.h>
#include <util/macros.h>
#include <elements/values.h>

/**************************************************************************/
/* Subrange interface                                                     */
/**************************************************************************/
struct subrange_t {
    struct subrange_iface_t subrange;
    struct value_iface_t *min;
    struct st_location_t *min_location;
    struct value_iface_t *max;
    struct st_location_t *max_location;
    struct st_location_t *location;
};

void subrange_destroy(
    struct subrange_iface_t *self)
{
    /* TODO: subrange destroy */
}

/**************************************************************************/
/* Value interface                                                        */
/**************************************************************************/
struct subrange_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    struct value_iface_t *current;
};

static int subrange_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->current->display(sv->current,
				buffer,
				buffer_size,
				config);
}

static int subrange_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    int type_can_hold = sv->type->can_hold(sv->type,
					   new_value,
					   config,
					   issues);
	
    if(type_can_hold != ESSTEE_TRUE)
    {
	return type_can_hold;
    }

    return sv->current->assign(sv->current,
			       new_value,
			       config,
			       issues);
}

static const struct type_iface_t * subrange_value_type_of(
    const struct value_iface_t *self)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->type;
}

static int subrange_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->integer)
    {
	issues->new_issue(
	    issues,
	    "the new value cannot be interpreted as an integer",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }
    
    st_bitflag_t other_value_class = other_value->class(other_value);
    
    if(!ST_FLAG_IS_SET(other_value_class, TEMPORARY_VALUE))
    {
	struct subrange_value_t *sv =
	    CONTAINER_OF(self, struct subrange_value_t, value);
	
	int type_can_hold = sv->type->can_hold(sv->type,
					       other_value,
					       config,
					       issues);

	if(type_can_hold != ESSTEE_TRUE)
	{
	    return type_can_hold;
	}

	if(other_value->type_of)
	{
	    const struct type_iface_t *other_value_type =
		other_value->type_of(other_value);

	    int types_compatible =
		sv->type->compatible(sv->type,
				     other_value_type,
				     config,
				     issues);

	    if(types_compatible != ESSTEE_TRUE)
	    {
		return types_compatible;
	    }
	}
    }
    
    return ESSTEE_TRUE;
}

static int subrange_value_compares_and_operates(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->integer)
    {
	issues->new_issue(
	    issues,
	    "the other value cannot be interpreted as an integer",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }
    
    st_bitflag_t other_value_class = other_value->class(other_value);
    
    if(!ST_FLAG_IS_SET(other_value_class, TEMPORARY_VALUE))
    {
	struct subrange_value_t *sv =
	    CONTAINER_OF(self, struct subrange_value_t, value);
	
	if(other_value->type_of)
	{
	    const struct type_iface_t *other_value_type =
		other_value->type_of(other_value);

	    int types_compatible =
		sv->type->compatible(sv->type, other_value_type, config, issues);

	    if(types_compatible != ESSTEE_TRUE)
	    {
		return types_compatible;
	    }
	}
    }
    
    return ESSTEE_TRUE;
}

static int subrange_value_override_type(
	const struct value_iface_t *self,
	const struct type_iface_t *type,
	const struct config_iface_t *config,
	struct issues_iface_t *issues)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    sv->type = type;

    return ESSTEE_OK;
}

static struct value_iface_t * subrange_value_create_temp_from(
    const struct value_iface_t *self,
    struct issues_iface_t *issues)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);
    
    return sv->current->create_temp_from(sv->current, issues);
}

static int subrange_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->current->greater(sv->current, other_value, config, issues);
}

static int subrange_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->current->lesser(sv->current, other_value, config, issues);
}

static int subrange_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->current->equals(sv->current, other_value, config, issues);
}

static void subrange_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: subrange value destructor */
}

static int64_t subrange_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->current->integer(sv->current, config, issues);
}

/**************************************************************************/
/* Type interface                                                         */
/**************************************************************************/
struct subrange_type_t {
    struct type_iface_t type;
    struct type_iface_t *subranged_type;
    struct value_iface_t *default_value;
    struct st_location_t *default_value_location;
    struct subrange_iface_t *subrange;
};

static struct value_iface_t * subrange_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_value_t *sv = NULL;

    ALLOC_OR_ERROR_JUMP(
	sv,
	struct subrange_value_t,
	issues,
	error_free_resources);

    struct subrange_type_t *st =
	CONTAINER_OF(self, struct subrange_type_t, type);

    sv->type = self;

    sv->current = st->subranged_type->create_value_of(st->subranged_type,
						      config,
						      issues);
    if(!sv->current)
    {
	goto error_free_resources;
    }

    int assign_result = ESSTEE_ERROR;
    if(st->default_value)
    {
	assign_result = sv->current->assign(sv->current,
					    st->default_value,
					    config,
					    issues);
    }
    else
    {
	assign_result = sv->current->assign(sv->current,
					    st->subrange->min,
					    config,
					    issues);
    }
    
    if(assign_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    memset(&(sv->value), 0, sizeof(struct value_iface_t));
    
    sv->value.display = subrange_value_display;
    sv->value.assign = subrange_value_assign;
    sv->value.type_of = subrange_value_type_of;
    sv->value.assignable_from = subrange_value_assignable_from;
    sv->value.comparable_to = subrange_value_compares_and_operates;
    sv->value.operates_with = subrange_value_compares_and_operates;
    sv->value.override_type = subrange_value_override_type;
    sv->value.create_temp_from = subrange_value_create_temp_from;
    sv->value.destroy = subrange_value_destroy;
    sv->value.integer = subrange_value_integer;
    sv->value.class = st_general_value_empty_class;
    sv->value.greater = subrange_value_greater;
    sv->value.lesser = subrange_value_lesser;
    sv->value.equals = subrange_value_equals;

    return &(sv->value);
    
error_free_resources:
    free(sv);
    return NULL;
}

static int subrange_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_type_t *st =
	CONTAINER_OF(self, struct subrange_type_t, type);

    struct subrange_value_t *sv =
	CONTAINER_OF(value_of, struct subrange_value_t, value);

    int assign_result = ESSTEE_ERROR;
    if(st->default_value)
    {
	assign_result = sv->current->assign(sv->current,
					    st->default_value,
					    config,
					    issues);
    }
    else
    {
	assign_result = sv->current->assign(sv->current,
					    st->subrange->min,
					    config,
					    issues);
    }

    return assign_result;
}

static int subrange_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_type_t *st =
	CONTAINER_OF(self, struct subrange_type_t, type);

    int subranged_type_can_hold = st->subranged_type->can_hold(st->subranged_type,
							       value,
							       config,
							       issues);

    if(subranged_type_can_hold != ESSTEE_TRUE)
    {
	return subranged_type_can_hold;
    }

    if(!value->lesser || !value->greater)
    {
	issues->new_issue(
	    issues,
	    "value stored in a subranged type must support > and < operators",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }
    
    int min_check_result = ESSTEE_ERROR;
    min_check_result = st->subrange->min->greater(st->subrange->min,
						  value,
						  config,
						  issues);
    if(min_check_result != ESSTEE_FALSE)
    {
	issues->new_issue(
	    issues,
	    "value cannot be smaller than subrange minimum",
	    ESSTEE_TYPE_ERROR);
	
	return ESSTEE_FALSE;
    }

    int max_check_result = ESSTEE_ERROR;
    max_check_result = st->subrange->max->lesser(st->subrange->max,
						 value,
						 config,
						 issues);
    if(max_check_result != ESSTEE_FALSE)
    {
	issues->new_issue(
	    issues,
	    "value cannot be larger than subrange maximum",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

static st_bitflag_t subrange_type_class(
    const struct type_iface_t *self)
{
    struct subrange_type_t *st =
	CONTAINER_OF(self, struct subrange_type_t, type);

    return st->subranged_type->class(st->subranged_type)|SUBRANGE_TYPE;
}

static int subrange_type_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_type_t *st =
	CONTAINER_OF(self, struct subrange_type_t, type);

    return st->subranged_type->compatible(st->subranged_type,
					  other_type,
					  config,
					  issues);
}

static void subrange_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: subrange type destructor */
}

/**************************************************************************/
/* Linker callback functions                                              */
/**************************************************************************/
static int subrange_type_storage_type_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_type_t *st =
	(struct subrange_type_t *)referrer;

    st->subranged_type = (struct type_iface_t *)target;

    return ESSTEE_OK;
}

static int subrange_type_storage_type_check(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_type_t *st =
	(struct subrange_type_t *)referrer;

    if(!st->subranged_type)
    {
	const char *message = issues->build_message(
	    issues,
	    "reference to undefined type '%s'",
	    identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_TYPE_ERROR,
	    1,
	    location);

	return ESSTEE_ERROR;
    }
    
    int subranged_type_can_hold_min =
	st->subranged_type->can_hold(st->subranged_type,
				     st->subrange->min,
				     config,
				     issues);
    
    if(subranged_type_can_hold_min != ESSTEE_TRUE)
    {
	issues->new_issue_at(
	    issues,
	    "the subranged type cannot hold the minimum value",
	    ESSTEE_TYPE_ERROR,
	    2,
	    location,
	    st->subrange->min_location);
	return ESSTEE_ERROR;
    }    

    int subranged_type_can_hold_max =
	st->subranged_type->can_hold(st->subranged_type,
				     st->subrange->max,
				     config,
				     issues);
    if(subranged_type_can_hold_max != ESSTEE_TRUE)
    {
	issues->new_issue_at(
	    issues,
	    "the subranged type cannot hold the maximum value",
	    ESSTEE_TYPE_ERROR,
	    2,
	    location,
	    st->subrange->max_location);
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct subrange_iface_t * st_create_subrange(
    struct value_iface_t *min,
    const struct st_location_t *min_location,
    struct value_iface_t *max,
    const struct st_location_t *max_location,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_t *sr = NULL;
    struct st_location_t *loc = NULL;
    struct st_location_t *min_loc = NULL;
    struct st_location_t *max_loc = NULL;
    
    int min_max_accepted = 1;

    if(!min->integer)
    {
	issues->new_issue_at(
	    issues,
	    "wrong minimum value, must be of integer type",
	    ESSTEE_TYPE_ERROR,
	    1,
	    min_location);
	
	min_max_accepted = 0;
    }
    else if(!min->greater || !min->lesser)
    {
	issues->new_issue_at(
	    issues,
	    "wrong minimum value, must support > and < operators",
	    ESSTEE_TYPE_ERROR,
	    1,
	    min_location);
	
	min_max_accepted = 0;
    }

    if(!max->integer)
    {
	issues->new_issue_at(
	    issues,
	    "wrong maximum value, must be of integer type",
	    ESSTEE_TYPE_ERROR,
	    1,
	    max_location);
	
	min_max_accepted = 0;
    }
    else if(!min->greater || !min->lesser)
    {
	issues->new_issue_at(
	    issues,
	    "wrong maximum value, must support > and < operators",
	    ESSTEE_TYPE_ERROR,
	    1,
	    min_location);
	
	min_max_accepted = 0;
    }
    
    if(!min_max_accepted)
    {
	goto error_free_resources;
    }

    int min_greater_than_max = min->greater(min, max, config, issues);	
    if(min_greater_than_max != ESSTEE_FALSE)
    {
	issues->new_issue_at(
	    issues,
	    "maximum must be larger or equal to the minumum value",
	    ESSTEE_ARGUMENT_ERROR,
	    1,
	    min_location);
	
	min_max_accepted = 0;	
    }

    if(!min_max_accepted)
    {
	goto error_free_resources;
    }
        
    ALLOC_OR_ERROR_JUMP(
	sr,
	struct subrange_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	issues,
	error_free_resources);
    
    LOCDUP_OR_ERROR_JUMP(
	min_loc,
	min_location,
	issues,
	error_free_resources);
    
    LOCDUP_OR_ERROR_JUMP(
	max_loc,
	max_location,
	issues,
	error_free_resources);

    sr->min = min;
    sr->min_location = min_loc;
    sr->max = max;
    sr->max_location = max_loc;

    sr->subrange.min = sr->min;
    sr->subrange.min_location = sr->min_location;
    sr->subrange.max = sr->max;
    sr->subrange.max_location = sr->max_location;
    sr->subrange.destroy = subrange_destroy;
    
    return &(sr->subrange);
    
error_free_resources:
    free(sr);
    free(loc);
    free(min_loc);
    free(max_loc);
    min->destroy(min);
    max->destroy(max);
    return NULL;
}

struct type_iface_t * st_create_subrange_type(
    char *storage_type_identifier,
    const struct st_location_t *storage_type_identifier_location,
    struct subrange_iface_t *subrange,
    struct value_iface_t *default_value,
    const struct st_location_t *default_value_location,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(default_value)
    {
	if(!default_value->integer)
	{
	    issues->new_issue_at(
		issues,
		"wrong default value, must be of integer type",
		ESSTEE_ARGUMENT_ERROR,
		1,
		default_value_location);
	    
	    goto error_free_resources;
	}

	if(!default_value->greater || !default_value->lesser)
	{
	    issues->new_issue_at(
		issues,
		"wrong default value, must support > and < operations",
		ESSTEE_TYPE_ERROR,
		1,
		default_value_location);
	    
	    goto error_free_resources;
	}

	int default_greater_than_max = default_value->greater(default_value,
							      subrange->max,
							      config,
							      issues);
	if(default_greater_than_max != ESSTEE_FALSE)
	{
	    issues->new_issue_at(
		issues,
		"default value cannot be larger than max",
		ESSTEE_ARGUMENT_ERROR,
		2,
		default_value_location,
		subrange->max_location);
	    
	    goto error_free_resources;
	}
	
	int default_lesser_than_min = default_value->lesser(default_value,
							    subrange->min,
							    config,
							    issues);
	if(default_lesser_than_min != ESSTEE_FALSE)
	{
	    issues->new_issue_at(
		issues,
		"default value cannot be smaller than min",
		ESSTEE_ARGUMENT_ERROR,
		2,
		default_value_location,
		subrange->min_location);
	    
	    goto error_free_resources;
	}
    }

    struct subrange_type_t *st = NULL;
    struct st_location_t *default_value_loc = NULL;

    ALLOC_OR_ERROR_JUMP(
	st,
	struct subrange_type_t,
	issues,
	error_free_resources);

    if(default_value)
    {
	LOCDUP_OR_ERROR_JUMP(
	    default_value_loc,
	    default_value_location,
	    issues,
	    error_free_resources);

	st->default_value = default_value;
	st->default_value_location = default_value_loc;
    }
    else
    {
	st->default_value = NULL;
	st->default_value_location = NULL;
    }

    int ref_add_result = type_refs->add_two_step(
	type_refs,
	storage_type_identifier,
	st,
	storage_type_identifier_location,
	subrange_type_storage_type_resolved,
	subrange_type_storage_type_check,
	issues);

    if(ref_add_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    st->subranged_type = NULL;
    st->subrange = subrange;
    
    memset(&(st->type), 0, sizeof(struct type_iface_t));
    st->type.create_value_of = subrange_type_create_value_of;
    st->type.reset_value_of = subrange_type_reset_value_of;
    st->type.can_hold = subrange_type_can_hold;
    st->type.class = subrange_type_class;
    st->type.compatible = subrange_type_compatible;
    st->type.destroy = subrange_type_destroy;

    return &(st->type);
    
error_free_resources:
    /* TODO: destroy stuff */
    return NULL;
}
