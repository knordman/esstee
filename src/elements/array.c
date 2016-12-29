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

/**************************************************************************/
/* Array index interface                                                  */
/**************************************************************************/
struct index_node_t {
    struct array_index_element_t index_element;
    struct expression_iface_t *expression;
    struct index_node_t *prev;
    struct index_node_t *next;
};

struct array_index_t {
    struct array_index_iface_t array_index;
    struct index_node_t *nodes;
    struct index_node_t *invoke_state_node;
    int constant_reference;
    struct st_location_t location;
};

static int array_index_step(
    struct array_index_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_index_t *ai =
	CONTAINER_OF(self, struct array_index_t, array_index);

    for(;
	ai->invoke_state_node != NULL;
	ai->invoke_state_node = ai->invoke_state_node->next)
    {
	if(ai->invoke_state_node->expression->invoke.step)
	{
	    struct index_node_t *current_node = ai->invoke_state_node;
	    ai->invoke_state_node = ai->invoke_state_node->next;

	    cursor->switch_current(
		cursor,
		&(current_node->expression->invoke),
		config,
		issues);

	    return INVOKE_RESULT_IN_PROGRESS;
	}
    }

    return INVOKE_RESULT_FINISHED;
}

static int array_index_reset(
    struct array_index_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_index_t *ai =
	CONTAINER_OF(self, struct array_index_t, array_index);

    struct index_node_t *itr = NULL;
    DL_FOREACH(ai->nodes, itr)
    {
	if(itr->expression->invoke.reset)
	{
	    int reset_result = itr->expression->invoke.reset(
		&(itr->expression->invoke),
		config,
		issues);

	    if(reset_result != ESSTEE_OK)
	    {
		return reset_result;
	    }	    
	}
    }

    ai->invoke_state_node = ai->nodes;

    return ESSTEE_OK;
}

static int array_index_allocate(
    struct array_index_iface_t *self,
    struct issues_iface_t *issues)
{
    struct array_index_t *ai =
	CONTAINER_OF(self, struct array_index_t, array_index);

    struct index_node_t *itr = NULL;
    DL_FOREACH(ai->nodes, itr)
    {
	if(itr->expression->invoke.allocate)
	{
	    int allocate_result = itr->expression->invoke.allocate(
		&(itr->expression->invoke),
		issues);

	    if(allocate_result != ESSTEE_OK)
	    {
		return allocate_result;
	    }
	}
    }

    return ESSTEE_OK;
}

static void array_index_destroy(
    struct array_index_iface_t *self)
{
    /* TODO: array index destructor */
}

static void array_index_destroy_clone(
    struct array_index_iface_t *self)
{
    /* TODO: array index clone destructor */
}

static struct array_index_iface_t * array_index_clone(
    struct array_index_iface_t *self,
    struct issues_iface_t *issues)
{
    struct array_index_t *ai =
	CONTAINER_OF(self, struct array_index_t, array_index);

    struct array_index_t *copy = NULL;
    struct index_node_t *copy_node = NULL;
    struct index_node_t *copy_nodes = NULL;
    struct index_node_t *itr = NULL;
    struct index_node_t *tmp = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct array_index_t,
	issues,
	error_free_resources);
    memcpy(copy, ai, sizeof(struct array_index_t));

    DL_FOREACH(ai->nodes, itr)
    {
	ALLOC_OR_ERROR_JUMP(
	    copy_node,
	    struct index_node_t,
	    issues,
	    error_free_resources);

	memset(copy_node, 0, sizeof(struct index_node_t));
	
	struct expression_iface_t *copy_expr = itr->expression;
	if(itr->expression->clone)
	{
	    copy_expr = itr->expression->clone(itr->expression, issues);

	    if(!copy_expr)
	    {
		goto error_free_resources;
	    }
	}

	copy_node->expression = copy_expr;
	copy_node->index_element.expression = copy_node->expression;
	
	DL_APPEND(copy_nodes, copy_node);

	if(copy_node->prev == copy_node) /* First index check */
	{
	    copy->array_index.first_node = &(copy_node->index_element);
	}
	else
	{
	    copy_node->prev->index_element.next = &(copy_node->index_element);
	}
    }

    copy->nodes = copy_nodes;
    copy->array_index.destroy = array_index_destroy_clone;

    return &(copy->array_index);
error_free_resources:
    DL_FOREACH_SAFE(copy_nodes, itr, tmp)
    {
	free(itr);
    }
    free(copy_node);
    free(copy);
    return NULL;
}

static int array_index_extend(
    struct array_index_iface_t *self,
    struct expression_iface_t *expression,
    const struct st_location_t *expression_location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_index_t *ai =
	CONTAINER_OF(self, struct array_index_t, array_index);

    struct index_node_t *node = NULL;
    ALLOC_OR_ERROR_JUMP(
	node,
	struct index_node_t,
	issues,
	error_free_resources);

    node->expression = expression;
    node->index_element.expression = node->expression;
    node->index_element.next = NULL;

    DL_APPEND(ai->nodes, node);
    if(node->prev == node)	/* First index check */
    {
	ai->array_index.first_node = &(node->index_element);
    }
    else
    {
	node->prev->index_element.next = &(node->index_element);
    }

    if(!ai->location.source)
    {
	memcpy(&(ai->location), expression_location, sizeof(struct st_location_t));
	ai->location.prev = NULL;
	ai->location.next = NULL;
    }
    else
    {
	ai->location.last_line = expression_location->last_line;
	ai->location.last_column = expression_location->last_column;
    }

    if(node->expression->invoke.step || node->expression->clone)
    {
	ai->constant_reference = ESSTEE_FALSE;
    }

    return ESSTEE_OK;
    
error_free_resources:
    return ESSTEE_ERROR;
}

int array_index_constant_reference(
    struct array_index_iface_t *self)
{
    struct array_index_t *ai =
	CONTAINER_OF(self, struct array_index_t, array_index);

    return ai->constant_reference;
}

struct array_index_iface_t * st_create_array_index(
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_index_t *ai = NULL;
    ALLOC_OR_ERROR_JUMP(
	ai,
	struct array_index_t,
	issues,
	error_free_resources);
    memset(ai, 0, sizeof(struct array_index_t));

    ai->array_index.location = &(ai->location);

    ai->constant_reference = ESSTEE_TRUE;
    
    ai->array_index.step = array_index_step;
    ai->array_index.reset = array_index_reset;
    ai->array_index.allocate = array_index_allocate;
    ai->array_index.clone = array_index_clone;
    ai->array_index.extend = array_index_extend;
    ai->array_index.constant_reference = array_index_constant_reference;
    ai->array_index.destroy = array_index_destroy;
    ai->array_index.first_node = NULL;

    return &(ai->array_index);

error_free_resources:
    return NULL;
}

/**************************************************************************/
/* Array range                                                            */
/**************************************************************************/
struct range_node_t {
    struct subrange_iface_t *subrange;
    int64_t entries;
    struct range_node_t *prev;
    struct range_node_t *next;
};

struct array_range_t {
    struct array_range_iface_t array_range;
    struct range_node_t *nodes;
};

static int array_range_extend(
    struct array_range_iface_t *self,
    struct subrange_iface_t *subrange,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_range_t *ar =
	CONTAINER_OF(self, struct array_range_t, array_range);

    struct range_node_t *node = NULL;
    ALLOC_OR_ERROR_JUMP(
	node,
	struct range_node_t,
	issues,
	error_free_resources);

    node->subrange = subrange;

    if(!subrange->min->integer)
    {
	issues->new_issue_at(
	    issues,
	    "only integer array ranges are supported",
	    ESSTEE_CONTEXT_ERROR,
	    1,
	    subrange->min_location);

	goto error_free_resources;
    }

    if(!subrange->max->integer)
    {
	issues->new_issue_at(
	    issues,
	    "only integer array ranges are supported",
	    ESSTEE_CONTEXT_ERROR,
	    1,
	    subrange->min_location);
	
	goto error_free_resources;
    }

    int64_t min_integer = subrange->min->integer(subrange->min,
						 config,
						 issues);
    int64_t max_integer = subrange->max->integer(subrange->max,
						 config,
						 issues);
    
    node->entries = max_integer - min_integer + 1;
    DL_APPEND(ar->nodes, node);

    return ESSTEE_OK;

error_free_resources:
    free(node);
    return ESSTEE_ERROR;
}

static void array_range_destroy(
    struct array_range_iface_t *self)
{
    /* TODO: array range destructor */
}

struct array_range_iface_t * st_create_array_range(
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_range_t *ar = NULL;
    ALLOC_OR_ERROR_JUMP(
	ar,
	struct array_range_t,
	issues,
	error_free_resources);
    memset(ar, 0, sizeof(struct array_range_t));

    ar->array_range.extend = array_range_extend;
    ar->array_range.destroy = array_range_destroy;

    return &(ar->array_range);
    
error_free_resources:
    return NULL;
}

/**************************************************************************/
/* Array initializer                                                      */
/**************************************************************************/
struct init_node_t {
    struct value_iface_t *value;
    struct value_iface_t *multiplier;
    struct st_location_t *location;
    struct init_node_t *prev;
    struct init_node_t *next;
};

struct array_init_t {
    struct array_initializer_iface_t array_init;
    struct value_iface_t value;
    struct init_node_t *nodes;
};

static int extend_array_init(
    struct array_initializer_iface_t *self,
    struct value_iface_t *multiplier,
    struct value_iface_t *value,
    const struct st_location_t *value_location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_init_t *ai =
	CONTAINER_OF(self, struct array_init_t, array_init);

    struct init_node_t *node = NULL;
    struct st_location_t *node_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	node,
	struct init_node_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	node_location,
	value_location,
	issues,
	error_free_resources);

    node->value = value;
    node->location = node_location;
    node->multiplier = multiplier;
    DL_APPEND(ai->nodes, node);

    return ESSTEE_OK;
    
error_free_resources:
    free(node);
    free(node_location);
    return ESSTEE_ERROR;
}
    
static int array_init_extend_by_value(
    struct array_initializer_iface_t *self,
    struct value_iface_t *value,
    const struct st_location_t *value_location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return extend_array_init(self,
			     NULL,
			     value,
			     value_location,
			     config,
			     issues);
}

static int array_init_extend_by_multiplied_value(
    struct array_initializer_iface_t *self,
    struct value_iface_t *multiplier,
    struct value_iface_t *value,
    const struct st_location_t *value_location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return extend_array_init(self,
			     NULL,
			     multiplier,
			     value_location,
			     config,
			     issues);

}

static struct value_iface_t * array_init_value(
    struct array_initializer_iface_t *self)
{
    struct array_init_t *ai =
	CONTAINER_OF(self, struct array_init_t, array_init);

    return &(ai->value);
}

static void array_init_destroy(
    struct array_initializer_iface_t *self)
{
    /* TODO: array init destructor */
}

/* Value methods */
static const struct array_initializer_iface_t * array_init_value_array_initializer(
    const struct value_iface_t *self)
{
    struct array_init_t *ai =
	CONTAINER_OF(self, struct array_init_t, value);

    return &(ai->array_init);
}

static void array_init_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: array init value destructor */
}

struct array_initializer_iface_t * st_create_array_initializer(
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct array_init_t *ai = NULL;
    ALLOC_OR_ERROR_JUMP(
	ai,
	struct array_init_t,
	issues,
	error_free_resources);
    memset(ai, 0, sizeof(struct array_init_t));
    
    ai->array_init.extend_by_value = array_init_extend_by_value;
    ai->array_init.extend_by_multiplied_value = array_init_extend_by_multiplied_value;
    ai->array_init.value = array_init_value;
    ai->array_init.destroy = array_init_destroy;

    ai->value.array_initializer = array_init_value_array_initializer;
    ai->value.destroy = array_init_value_destroy;
    
    return &(ai->array_init);

error_free_resources:
    return NULL;
}

/**************************************************************************/
/* Array type                                                             */
/**************************************************************************/
struct array_type_t {
    struct type_iface_t type;
    struct type_iface_t *arrayed_type;
    struct array_range_t *ranges;
    struct array_init_t *initializer;
    size_t total_elements;
};

struct array_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    struct value_iface_t **elements;
    const struct type_iface_t *arrayed_type;
};

static int check_array_initializer(
    const struct range_node_t *range,
    const struct array_init_t *initializer,
    struct type_iface_t *arrayed_type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    size_t checked_entries = 0;

    const struct init_node_t *itr = NULL;

    DL_FOREACH(initializer->nodes, itr)
    {
	if(itr->value->array_initializer)
	{
	    if(!range->next)
	    {
		issues->new_issue_at(
		    issues,
		    "the initializer addresses a new array level, but that does not match the array specification.", 
		    ESSTEE_TYPE_ERROR,
		    1,
		    itr->location);
		
		return ESSTEE_ERROR;
	    }
	    else
	    {
		const struct array_initializer_iface_t *inner_initializer =
		    itr->value->array_initializer(itr->value);

		const struct array_init_t *inner =
		    CONTAINER_OF(inner_initializer, struct array_init_t, array_init);
		
		if(check_array_initializer(
		       range->next,
		       inner,
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
	    if(!range->next)
	    {
		if(arrayed_type->can_hold(arrayed_type,
					  itr->value,
					  config,
					  issues) != ESSTEE_OK)
		{
		    return ESSTEE_ERROR;
		}
	    }
	    else
	    {
		issues->new_issue_at(
		    issues,
		    "according to the array specification the initializer should address a new level.", 
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
		    "only integer multipliers are supported",
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

    if(checked_entries < range->entries)
    {
	issues->new_issue_at(
	    issues,
	    "too few values in array initializer.",
	    ESSTEE_TYPE_ERROR,
	    1,
	    initializer->nodes->location);
	
	return ESSTEE_ERROR;
    }
    else if(checked_entries > range->entries)
    {
	issues->new_issue_at(
	    issues,
	    "too many values in array initializer.", 
	    ESSTEE_TYPE_ERROR,
	    1,
	    initializer->nodes->location);

	return ESSTEE_ERROR;
    }
	
    return ESSTEE_OK;
}

static int assign_array_initializer(
    struct value_iface_t **elements,
    const struct array_init_t *initializer,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    int elements_assigned = 0;

    const struct init_node_t *itr = NULL;
    DL_FOREACH(initializer->nodes, itr)
    {
	if(itr->value->array_initializer)
	{
	    const struct array_initializer_iface_t *inner_initializer =
		itr->value->array_initializer(itr->value);

	    const struct array_init_t *inner =
		CONTAINER_OF(inner_initializer, struct array_init_t, array_init);
	    
	    int inner_array_elements_assigned = assign_array_initializer(
		elements,
		inner,
		config,
		issues);

	    if(inner_array_elements_assigned <= 0)
	    {
		return ESSTEE_ERROR;
	    }

	    elements += inner_array_elements_assigned;
	    elements_assigned += inner_array_elements_assigned;
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
static int array_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct array_value_t *av =
	CONTAINER_OF(self, struct array_value_t, value);

    if(!other_value->array_initializer)
    {
	issues->new_issue(
	    issues,
	    "the complete array can only be assigned an initializer in initialization",
	    ESSTEE_CONTEXT_ERROR);

	return ESSTEE_FALSE;
    }
    
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

    const struct array_initializer_iface_t *initializer =
	new_value->array_initializer(new_value);

    const struct array_init_t *ai =
	CONTAINER_OF(initializer, struct array_init_t, array_init);
    
    int elements_assigned = assign_array_initializer(av->elements,
						     ai,
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

    const struct type_iface_t *vt =
	TYPE_ANCESTOR(av->type);

    struct array_type_t *at =
	CONTAINER_OF(vt, struct array_type_t, type);
    
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
    const struct array_index_iface_t *array_index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct array_value_t *av =
    	CONTAINER_OF(self, struct array_value_t, value);

    const struct type_iface_t *vt =
	TYPE_ANCESTOR(av->type);
    
    struct array_type_t *at =
	CONTAINER_OF(vt, struct array_type_t, type);

    const struct range_node_t *range_itr = at->ranges->nodes;
    const struct array_index_element_t *index_itr = NULL;

    size_t elements_offset = 0;

    for(index_itr = array_index->first_node; index_itr != NULL; index_itr = index_itr->next)
    {
	const struct value_iface_t *index_value =
	    index_itr->expression->return_value(index_itr->expression);

	if(!range_itr)
	{
	    issues->new_issue_at(
		issues,
		"array index addresses a non existing range",
		ESSTEE_ARGUMENT_ERROR,
		1,
		array_index->location);

	    return NULL;
	}
	
	if(!index_value->integer)
	{
	    issues->new_issue_at(
		issues,
		"only integer array indices are supported",
		ESSTEE_ARGUMENT_ERROR,
		1,
		index_itr->expression->invoke.location);

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
		"array index out of range (smaller than minimum)",
		ESSTEE_ARGUMENT_ERROR,
		1,
		index_itr->expression->invoke.location);

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
		"array index out of range (larger than maximum)",
		ESSTEE_ARGUMENT_ERROR,
		1,
		index_itr->expression->invoke.location);

	    return NULL;
	}

	int64_t index_num = index_value->integer(index_value, config, issues);
	int64_t min_index_num = range_itr->subrange->min->integer(range_itr->subrange->min,
								  config,
								  issues);
	
	size_t multiplier = 1;
	const struct range_node_t *r_itr = NULL;
	for(r_itr = range_itr->next; r_itr != NULL; r_itr = r_itr->next)
	{
	    multiplier *= r_itr->entries;
	}
	
	elements_offset += (index_num-min_index_num) * multiplier;
	
	range_itr = range_itr->next;
    }

    if(range_itr)
    {
	issues->new_issue_at(
	    issues,
	    "array index does not address all ranges in array",
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

    if(at->initializer)
    {
	int elements_assigned = assign_array_initializer(av->elements,
							 at->initializer,
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

    if(!value->array_initializer)
    {
	issues->new_issue(
	    issues,
	    "an array type variable can only be initialized when declared by an initializer",
	    ESSTEE_CONTEXT_ERROR);

	return ESSTEE_FALSE;
    }
    
    const struct array_initializer_iface_t *initializer =
	value->array_initializer(value);

    const struct array_init_t *ai = 
	CONTAINER_OF(initializer, struct array_init_t, array_init);
    
    return check_array_initializer(at->ranges->nodes,
				   ai,
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
    
    if(at->initializer)
    {    
	return check_array_initializer(at->ranges->nodes,
				       at->initializer,
				       at->arrayed_type,
				       config,
				       issues);
    }
    
    return ESSTEE_OK;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct type_iface_t * st_create_array_type(
    struct array_range_iface_t *array_range,
    char *arrayed_type_identifier,
    const struct st_location_t *arrayed_type_identifier_location,
    struct value_iface_t *default_value,
    const struct st_location_t *default_value_location,
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

    struct array_range_t *ranges =
	CONTAINER_OF(array_range, struct array_range_t, array_range);
    
    at->ranges = ranges;
    at->initializer = NULL;

    if(default_value)
    {
	if(!default_value->array_initializer)
	{
	    issues->new_issue_at(
		issues,
		"an array can only be initialized from an array initializer",
		ESSTEE_CONTEXT_ERROR,
		1,
		default_value_location);

	    goto error_free_resources;
	}

	const struct array_initializer_iface_t *initializer =
	    default_value->array_initializer(default_value);

	/* The only one creating array_initializer_iface instances is
	 * the array code, so it is safe to assume that it is actually
	 * an array_initializer_t type. */
	struct array_init_t *ai =
	    CONTAINER_OF(initializer, struct array_init_t, array_init);
	
	at->initializer = ai;
    }
    
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
    const struct range_node_t *itr = NULL;
    DL_FOREACH(at->ranges->nodes, itr)
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
    free(at);
    return NULL;
}
