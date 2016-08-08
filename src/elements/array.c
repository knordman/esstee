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

#include <elements/array.h>
#include <util/macros.h>
#include <elements/values.h>

#include <utlist.h>
#include <stdio.h>

struct listed_value_t {
    struct value_iface_t *value;
    struct value_iface_t *multiplier;
    struct st_location_t *location;
    struct listed_value_t *prev;
    struct listed_value_t *next;
};

struct array_init_value_t {
    struct value_iface_t value;
    size_t entries;
    struct st_location_t *location;
    struct listed_value_t *values;
};

struct array_range_t {
    struct subrange_t *subrange;
    size_t entries;
    struct array_range_t *prev;
    struct array_range_t *next;
};

struct array_type_t {
    struct type_iface_t type;
    struct type_iface_t *arrayed_type;
    struct array_range_t *ranges;
    size_t total_elements;
    struct value_iface_t *default_value;
};

struct array_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    struct value_iface_t **elements;
    const struct type_iface_t *arrayed_type;
};

static int array_type_check_array_initializer(
    struct array_range_t *ranges,
    const struct value_iface_t *default_value,
    struct type_iface_t *arrayed_type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct array_init_value_t *initializer =
	default_value->array_init_value(default_value);
    
    size_t checked_entries = 0;
    struct array_range_t *current_range = ranges;
    struct listed_value_t *itr = NULL;
	
    for(itr = initializer->values; itr != NULL; itr = itr->next)
    {
	if(itr->value->array_init_value)
	{
	    if(!current_range->next)
	    {
		issues->new_issue_at(
		    issues,
		    "value addresses a new array level, but that does not match the array specification.", 
		    ESSTEE_TYPE_ERROR,
		    1,
		    itr->location);
		
		return ESSTEE_ERROR;
	    }
	    else
	    {
		if(array_type_check_array_initializer(
		       current_range->next,
		       itr->value,
		       arrayed_type,
		       config,
		       issues) == ESSTEE_ERROR)
		{
		    return ESSTEE_ERROR;
		}
	    }
	}
	else
	{
	    if(!current_range->next)
	    {
		if(arrayed_type->can_hold(arrayed_type, itr->value, config, issues) != ESSTEE_OK)
		{
		    return ESSTEE_ERROR;
		}
	    }
	    else
	    {
		issues->new_issue_at(
		    issues,
		    "according to the array specification the value should address a new level.", 
		    ESSTEE_TYPE_ERROR,
		    1,
		    itr->location);
		
		return ESSTEE_ERROR;
	    }
	}

	if(itr->multiplier)
	{
	    if(!itr->multiplier->integer)
	    {
		issues->new_issue_at(
		    issues,
		    "bad multiplier in array initializer.",
		    ESSTEE_TYPE_ERROR,
		    1,
		    itr->location);
		
		return ESSTEE_ERROR;
	    }

	    int64_t multiplier = itr->multiplier->integer(itr->multiplier, config, issues);
	    checked_entries += multiplier;
	}
	else
	{
	    checked_entries++;
	}
    }

    if(checked_entries < current_range->entries)
    {
	issues->new_issue_at(
	    issues,
	    "too few values in array initializer.",
	    ESSTEE_TYPE_ERROR,
	    1,
	    initializer->location);
	
	return ESSTEE_ERROR;
    }
    else if(checked_entries > current_range->entries)
    {
	issues->new_issue_at(
	    issues,
	    "too many values in array initializer.", 
	    ESSTEE_TYPE_ERROR,
	    1,
	    initializer->location);

	return ESSTEE_ERROR;
    }
	
    return ESSTEE_OK;
}

static int array_type_assign_default_value(
    struct value_iface_t **elements,
    const struct value_iface_t *default_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct array_init_value_t *iav =
	default_value->array_init_value(default_value);

    int elements_assigned = 0;
    
    struct listed_value_t *itr = NULL;
    DL_FOREACH(iav->values, itr)
    {
	if(itr->value->array_init_value)
	{
	    int sub_array_elements_assigned
		= array_type_assign_default_value(elements,
						  itr->value,
						  config,
						  issues);

	    if(sub_array_elements_assigned <= 0)
	    {
		return ESSTEE_ERROR;
	    }

	    elements += sub_array_elements_assigned;
	    elements_assigned += sub_array_elements_assigned;
	}
	else
	{
	    size_t elements_to_assign = 1;
	    if(itr->multiplier)
	    {
		elements_to_assign = itr->multiplier->integer(itr->multiplier, config, issues);
	    }

	    for(size_t i = 0; i < elements_to_assign; i++)
	    {
		int assign_result = (*elements)->assign((*elements),
							itr->value,
							config,
							issues);

		if(assign_result != ESSTEE_OK)
		{
		    return assign_result;
		}

		elements++;
	    }

	    elements_assigned += elements_to_assign;
	}
    }

    return elements_assigned;
}

/**************************************************************************/
/* Value interface                                                        */
/**************************************************************************/
/* Initializer value */
static const struct array_init_value_t * array_init_value(
    const struct value_iface_t *self)
{
    struct array_init_value_t *av =
	CONTAINER_OF(self, struct array_init_value_t, value);

    return av;
}

static void array_init_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: array init value destructor */
}

/* Array variable value */
static int array_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct array_value_t *av =
	CONTAINER_OF(self, struct array_value_t, value);
    
    int type_can_hold = av->type->can_hold(av->type,
					   other_value,
					   config,
					   issues);

    if(type_can_hold != ESSTEE_TRUE)
    {
	return type_can_hold;
    }

    return ESSTEE_TRUE;
}

static int array_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct array_value_t *av =
	CONTAINER_OF(self, struct array_value_t, value);

    int elements_assigned = array_type_assign_default_value(av->elements,
							    new_value,
							    config,
							    issues);

    if(elements_assigned <= 0)
    {
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int array_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct array_value_t *av =
	CONTAINER_OF(self, struct array_value_t, value);

    const struct type_iface_t *avt =
	TYPE_ANCESTOR(av->type);

    struct array_type_t *at =
	CONTAINER_OF(avt, struct array_type_t, type);
    
    int written_bytes = snprintf(buffer,
				 buffer_size,
				 "[");

    size_t buffer_size_start = buffer_size;

    CHECK_WRITTEN_BYTES(written_bytes);
    buffer += written_bytes;
    buffer_size -= written_bytes;
    
    for(size_t i = 0; i < at->total_elements; i++)
    {
	if(i != 0)
	{
	    written_bytes = snprintf(buffer,
				     buffer_size,
				     ",");
	    CHECK_WRITTEN_BYTES(written_bytes);
	    buffer += written_bytes;
	    buffer_size -= written_bytes;	    
	}

	int element_displayed_bytes  = av->elements[i]->display(av->elements[i],
								buffer,
								buffer_size,
								config);

	CHECK_WRITTEN_BYTES(element_displayed_bytes);
	
	buffer += element_displayed_bytes;
	buffer_size -= element_displayed_bytes;  
    }

    written_bytes = snprintf(buffer,
			     buffer_size,
			     "]");
    CHECK_WRITTEN_BYTES(written_bytes);
    buffer += written_bytes;
    buffer_size -= written_bytes;	    
    
    return buffer_size_start - buffer_size;
}

static const struct type_iface_t * array_value_type_of(
    const struct value_iface_t *self)
{
    const struct array_value_t *av =
    	CONTAINER_OF(self, struct array_value_t, value);

    return av->type;
}

static struct value_iface_t * array_value_index(
    struct value_iface_t *self,
    const struct array_index_t *array_index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct array_value_t *av =
    	CONTAINER_OF(self, struct array_value_t, value);

    const struct type_iface_t *avt =
	TYPE_ANCESTOR(av->type);
    struct array_type_t *at =
	CONTAINER_OF(avt, struct array_type_t, type);

    const struct array_range_t *range_itr = at->ranges;
    const struct array_index_t *index_itr = NULL;

    size_t elements_offset = 0;
    
    DL_FOREACH(array_index, index_itr)
    {
	const struct value_iface_t *index_value =
	    index_itr->index_expression->return_value(index_itr->index_expression);

	if(!range_itr)
	{
	    issues->new_issue_at(
		issues,
		"index out of range",
		ESSTEE_ARGUMENT_ERROR,
		1,
		array_index->location);

	    return NULL;
	}
	
	if(!index_value->integer)
	{
	    issues->new_issue_at(
		issues,
		"only integer indices are supported",
		ESSTEE_ARGUMENT_ERROR,
		1,
		array_index->location);

	    return NULL;
	}

	int index_too_small =
	    range_itr->subrange->min->greater(range_itr->subrange->min,
					      index_value,
					      config,
					      issues);
	if(index_too_small != ESSTEE_FALSE)
	{
	    issues->new_issue_at(
		issues,
		"index out of range (smaller than minimum)",
		ESSTEE_ARGUMENT_ERROR,
		1,
		array_index->location);

	    return NULL;
	}

	int index_too_large =
	    range_itr->subrange->max->lesser(range_itr->subrange->max,
					     index_value,
					     config,
					     issues);
	if(index_too_large != ESSTEE_FALSE)
	{
	    issues->new_issue_at(
		issues,
		"index out of range (larger than maximum)",
		ESSTEE_ARGUMENT_ERROR,
		1,
		array_index->location);

	    return NULL;
	}

	int64_t index_num = index_value->integer(index_value, config, issues);
	int64_t min_index_num = range_itr->subrange->min->integer(range_itr->subrange->min,
								  config,
								  issues);
	
	size_t multiplier = 1;
	if(range_itr->next)
	{
	    const struct array_range_t *ritr = NULL;
	    DL_FOREACH(range_itr->next, ritr)
	    {
		multiplier *= ritr->entries;
	    }
	}
	
	elements_offset += (index_num-min_index_num) * multiplier;
	
	range_itr = range_itr->next;
    }

    if(range_itr)
    {
	issues->new_issue_at(
	    issues,
	    "missing array indices",
	    ESSTEE_ARGUMENT_ERROR,
	    1,
	    array_index->location);

	return NULL;
    }
    
    return av->elements[elements_offset];
}    

static int array_value_override_type(
    const struct value_iface_t *self,
    const struct type_iface_t *type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_value_t *av =
    	CONTAINER_OF(self, struct array_value_t, value);

    av->type = type;

    return ESSTEE_OK;
}


static void array_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: array value destructor */
}

/**************************************************************************/
/* Type interface                                                         */
/**************************************************************************/
static struct value_iface_t * array_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct array_type_t *at =
	CONTAINER_OF(self, struct array_type_t, type);

    struct array_value_t *av = NULL;
    struct value_iface_t **elements = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	av,
	struct array_value_t,
	issues,
	error_free_resources);

    ALLOC_ARRAY_OR_ERROR_JUMP(
	elements,
	struct value_iface_t *,
	at->total_elements,
	issues,
	error_free_resources);

    memset(elements, 0, sizeof(struct value_iface_t *)*at->total_elements);

    for(size_t i = 0; i < at->total_elements; i++)
    {
	elements[i] = at->arrayed_type->create_value_of(at->arrayed_type,
							config,
							issues);

	if(!elements[i])
	{
	    goto error_free_resources;
	}
    }

    if(at->default_value)
    {
	int elements_assigned = array_type_assign_default_value(elements,
								at->default_value,
								config,
								issues);

	if(elements_assigned <= 0)
	{
	    goto error_free_resources;
	}
    }
	
    av->type = self;
    av->arrayed_type = at->arrayed_type;
    av->elements = elements;

    memset(&(av->value), 0, sizeof(struct value_iface_t));
    
    av->value.display = array_value_display;
    av->value.assignable_from = array_value_assignable_from;
    av->value.assign = array_value_assign;
    av->value.type_of = array_value_type_of;
    av->value.index = array_value_index;
    av->value.destroy = array_value_destroy;
    av->value.class = st_general_value_empty_class;
    av->value.override_type = array_value_override_type;

    return &(av->value);

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

static int array_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct array_type_t *at =
	CONTAINER_OF(self, struct array_type_t, type);

    struct array_value_t *av =
	CONTAINER_OF(value_of, struct array_value_t, value);

    if(at->default_value)
    {
	int elements_assigned = array_type_assign_default_value(av->elements,
								at->default_value,
								config,
								issues);

	if(elements_assigned != at->total_elements)
	{
	    return ESSTEE_ERROR;
	}
    }
    else
    {
	for(size_t i = 0; i < at->total_elements; i++)
	{
	    int element_reset_result =
		at->arrayed_type->reset_value_of(at->arrayed_type,
						 av->elements[i],
						 config,
						 issues);
	
	    if(element_reset_result != ESSTEE_OK)
	    {
		return element_reset_result;
	    }
	}
    }

    return ESSTEE_OK;
}

static int array_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct array_type_t *at =
	CONTAINER_OF(self, struct array_type_t, type);

    return array_type_check_array_initializer(at->ranges,
					      value,
					      at->arrayed_type,
					      config,
					      issues);
}

static st_bitflag_t array_type_class(
    const struct type_iface_t *self)
{
    return ARRAY_TYPE;
}

static void array_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: array type destructor */
}

/**************************************************************************/
/* Linker callback functions                                              */
/**************************************************************************/
static int array_type_arrayed_type_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_type_t *st =
	(struct array_type_t *)referrer;

    st->arrayed_type = (struct type_iface_t *)target;

    return ESSTEE_OK;
}

static int array_type_arrayed_type_check(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_type_t *at =
	(struct array_type_t *)referrer;

    if(!at->arrayed_type)
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
    
    if(at->default_value)
    {
	return array_type_check_array_initializer(at->ranges,
						  at->default_value,
						  at->arrayed_type,
						  config,
						  issues);
    }
    
    return ESSTEE_OK;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct array_range_t * st_extend_array_range(
    struct array_range_t *array_ranges,
    struct subrange_t *subrange,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_range_t *ar = NULL;

    ALLOC_OR_ERROR_JUMP(
	ar,
	struct array_range_t,
	issues,
	error_free_resources);
    
    ar->subrange = subrange;

    int64_t max_num = subrange->max->integer(subrange->max,
					     config,
					     issues);

    int64_t min_num = subrange->min->integer(subrange->min,
					     config,
					     issues);
    ar->entries = max_num - min_num + 1;

    DL_APPEND(array_ranges, ar);

    return array_ranges;

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

void st_destroy_listed_values(
    struct listed_value_t *values)
{
    /* TODO: listed values destroy */
}

void st_destroy_array_ranges(
    struct array_range_t *array_ranges)
{
    /* TODO: array ranges destroy */
}

struct listed_value_t * st_extend_array_initializer(
    struct listed_value_t *values,
    struct value_iface_t *multiplier,
    struct value_iface_t *new_value,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct listed_value_t *listed_value = NULL;
    struct st_location_t *value_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	listed_value,
	struct listed_value_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	value_location,
	location,
	issues,
	error_free_resources);

    listed_value->value = new_value;
    listed_value->multiplier = multiplier;
    listed_value->location = value_location;

    DL_APPEND(values, listed_value);

    return values;

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

struct value_iface_t * st_create_array_init_value(
    struct listed_value_t *values,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_init_value_t *av = NULL;
    struct st_location_t *loc = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	av,
	struct array_init_value_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	loc,
	location,
	issues,
	error_free_resources);

    size_t entries = 0;
    struct listed_value_t *itr = NULL;
    DL_COUNT(values, itr, entries);

    av->location = loc;
    av->values = values;
    av->entries = entries;

    memset(&(av->value), 0, sizeof(struct value_iface_t));
    av->value.array_init_value = array_init_value;
    av->value.destroy = array_init_value_destroy;
    
    return &(av->value);
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

struct type_iface_t * st_create_array_type(
    struct array_range_t *array_ranges,
    char *arrayed_type_identifier,
    const struct st_location_t *arrayed_type_identifier_location,
    struct value_iface_t *default_value,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_type_t *at = NULL;

    ALLOC_OR_ERROR_JUMP(
	at,
	struct array_type_t,
	issues,
	error_free_resources);

    at->ranges = array_ranges;
    at->default_value = default_value;

    int ref_add_result = type_refs->add_two_step(
	type_refs,
	arrayed_type_identifier,
	at,
	arrayed_type_identifier_location,
	array_type_arrayed_type_resolved,
	array_type_arrayed_type_check,
	issues);

    if(ref_add_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    /* Determine the array size */
    at->total_elements = 0;
    struct array_range_t *itr = NULL;
    DL_FOREACH(at->ranges, itr)
    {
	if(at->total_elements == 0)
	{
	    at->total_elements = itr->entries;
	}
	else
	{
	    at->total_elements *= itr->entries;
	}
    }
    
    memset(&(at->type), 0, sizeof(struct type_iface_t));
    at->type.create_value_of = array_type_create_value_of;
    at->type.reset_value_of = array_type_reset_value_of;
    at->type.can_hold = array_type_can_hold;
    at->type.class = array_type_class;
    at->type.destroy = array_type_destroy;

    return &(at->type);
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}
