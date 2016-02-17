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
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <elements/values.h>
#include <util/macros.h>
#include <elements/shared.h>

#include <utlist.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>


#define CHECK_WRITTEN_BYTES(X)			\
    do {					\
	if(X == 0)				\
	{					\
	    return ESSTEE_FALSE;		\
	}					\
	else if(X < 0)				\
	{					\
	    return ESSTEE_ERROR;		\
	}					\
    } while(0)

/**************************************************************************/
/* Integer values                                                         */
/**************************************************************************/
int st_integer_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int written_bytes = snprintf(buffer, buffer_size, "%" PRId64, iv->num);
    if(written_bytes == 0)
    {
	return ESSTEE_FALSE;
    }
    else if(written_bytes < 0)
    {
	return ESSTEE_ERROR;
    }

    return written_bytes;
}

int st_integer_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(iv->explicit_type)
    {
	int can_hold = iv->explicit_type->can_hold(
	       iv->explicit_type,
	       new_value,
	       config);
	
	if(can_hold != ESSTEE_TRUE)
	{
	    return can_hold;
	}
    }

    iv->num = new_value->integer(new_value, config);

    return ESSTEE_OK;
}

const struct type_iface_t * st_integer_value_explicit_type(
    const struct value_iface_t *self)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    return iv->explicit_type;
}

int st_integer_value_compatible(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv 
	= CONTAINER_OF(self, struct integer_value_t, value);

    if(!other_value->integer)
    {
	return ESSTEE_FALSE;
    }
    else if(iv->explicit_type)
    {
	if(iv->explicit_type->can_hold(
	       iv->explicit_type,
	       other_value,
	       config) != ESSTEE_TRUE)
	{
	    return ESSTEE_FALSE;
	}

	const struct type_iface_t *other_value_type = NULL;
	if(other_value->explicit_type)
	{
	    other_value_type
		= other_value->explicit_type(other_value);
	}

	if(other_value_type)
	{
	    if(iv->explicit_type->compatible(
		   iv->explicit_type,
		   other_value_type,
		   config) != ESSTEE_TRUE)
	    {
		return ESSTEE_FALSE;
	    }
	}
    }
    
    return ESSTEE_TRUE;
}

struct value_iface_t * st_integer_value_create_temp_from(
    const struct value_iface_t *self)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    struct integer_value_t *clone = NULL;
    ALLOC_OR_JUMP(
	clone,
	struct integer_value_t,
	error_free_resources);

    memcpy(clone, iv, sizeof(struct integer_value_t));

    /* Temporary value has no initial value */
    clone->num = 0;
    
    /* Temporary has no type */
    clone->value.explicit_type = NULL;

    /* Temporary is assignable */
    clone->value.assign = st_integer_value_assign;
    
    return &(clone->value);

error_free_resources:
    return NULL;
}

void st_integer_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: integer value destroy */
}

int st_integer_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(iv->num > other_value->integer(other_value, config))
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

int st_integer_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(iv->num < other_value->integer(other_value, config))
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

int st_integer_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(iv->num == other_value->integer(other_value, config))
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

static int binary_expression_result_after_type_check(
    struct integer_value_t *iv,
    int64_t start_num,
    const struct config_iface_t *config)
{
    if(iv->explicit_type)
    {
	int fit_into_type =
	    iv->explicit_type->can_hold(
		iv->explicit_type,
		&(iv->value),
		config);

	if(fit_into_type != ESSTEE_TRUE)
	{
	    iv->num = start_num;
	    return fit_into_type;
	}
    }

    return ESSTEE_OK;
}

int st_integer_value_plus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int64_t start_num = iv->num;
    iv->num += other_value->integer(other_value, config);

    return binary_expression_result_after_type_check(iv, start_num, config);
}

int st_integer_value_minus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int64_t start_num = iv->num;
    iv->num -= other_value->integer(other_value, config);

    return binary_expression_result_after_type_check(iv, start_num, config);
}

int st_integer_value_multiply(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int64_t start_num = iv->num;
    iv->num *= other_value->integer(other_value, config);

    return binary_expression_result_after_type_check(iv, start_num, config);
}

int st_integer_value_divide(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int64_t start_num = iv->num;
    iv->num /= other_value->integer(other_value, config);

    return binary_expression_result_after_type_check(iv, start_num, config);
}

int st_integer_value_modulus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    iv->num %= other_value->integer(other_value, config);

    return ESSTEE_OK;
}

int st_integer_value_to_power(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int64_t start_num = iv->num;

    long double dnum = (long double)iv->num;
    long double exp = (long double)other_value->integer(other_value, config);

    errno = 0;
    long double result = powl(dnum, exp);
    if(errno != 0)
    {
	return ESSTEE_ERROR;
    }
    
    iv->num = (int64_t)result;

    return binary_expression_result_after_type_check(iv, start_num, config);
}

int64_t st_integer_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    return iv->num;
}

int st_bool_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int written_bytes = 0;
    int should_be_written = 0;
    if(iv->num != 0)
    {
	written_bytes = snprintf(buffer, buffer_size, "true");
	should_be_written = 4;
    }
    else
    {
	written_bytes = snprintf(buffer, buffer_size, "false");
	should_be_written = 5;
    }

    if(written_bytes < 0)
    {
	return ESSTEE_ERROR;
    }
    else if(written_bytes != should_be_written)
    {
	return ESSTEE_FALSE;
    }

    return written_bytes;
}

int st_bool_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(new_value->bool(new_value, config) == ESSTEE_TRUE)
    {
	iv->num = 1;
    }
    else
    {
	iv->num = 0;
    }

    return ESSTEE_OK;
}

int st_bool_value_compatible(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    if(!other_value->bool)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

struct value_iface_t * st_bool_value_create_temp_from(
    const struct value_iface_t *self)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    struct integer_value_t *clone = NULL;
    ALLOC_OR_JUMP(
	clone,
	struct integer_value_t,
	error_free_resources);

    memcpy(clone, iv, sizeof(struct integer_value_t));

    /* Temporary value has no initial value */
    clone->num = 0;
    
    /* Temporary has no type */
    clone->value.explicit_type = NULL;

    /* Temporary is assignable */
    clone->value.assign = st_bool_value_assign;
    
    return &(clone->value);

error_free_resources:
    return NULL;
}

int st_bool_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    if(self->bool(self, config) == other_value->bool(other_value, config))
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

int st_bool_value_bool(
    const struct value_iface_t *self,
    const struct config_iface_t *conf)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(iv->num == 0)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

int st_bool_value_xor(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int64_t ov = (other_value->bool(other_value, config) == ESSTEE_TRUE) ? 1 : 0;

    iv->num ^= ov;
    
    return ESSTEE_OK;
}

int st_bool_value_and(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int64_t ov = (other_value->bool(other_value, config) == ESSTEE_TRUE) ? 1 : 0;
    
    iv->num &= ov;
    
    return ESSTEE_OK;
}
    
int st_bool_value_or(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int64_t ov = (other_value->bool(other_value, config) == ESSTEE_TRUE) ? 1 : 0;

    iv->num |= ov;
    
    return ESSTEE_OK;
}

/**************************************************************************/
/* Enumerated value                                                       */
/**************************************************************************/
int st_enum_value_display(
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
    if(written_bytes == 0)
    {
	return ESSTEE_FALSE;
    }
    else if(written_bytes < 0)
    {
	return ESSTEE_ERROR;
    }

    return written_bytes;
}

int st_enum_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    int can_hold = ev->explicit_type->can_hold(
	ev->explicit_type,
	new_value,
	config);
    if(can_hold != ESSTEE_TRUE)
    {
	return can_hold;
    }
    
    ev->constant = new_value->enumeration(new_value, config);

    return ESSTEE_OK;
}

const struct type_iface_t * st_enum_value_explicit_type(
    const struct value_iface_t *self)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    return ev->explicit_type;
}

int st_enum_value_compatible(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    if(!other_value->enumeration)
    {
	return ESSTEE_FALSE;
    }

    /* Check that the enumeration of the other value is present by the
     * values defined by the type */
    int can_hold = ev->explicit_type->can_hold(
	ev->explicit_type,
	other_value,
	config);
    if(can_hold != ESSTEE_TRUE)
    {
	return can_hold;
    }

    /* If the other value has a reference to its values group, check
     * that the group is the same as the self group. Inline enum
     * values do not have any values group, since they lack a
     * reference to their type. */
    const struct enum_item_t *other_value_enum = other_value->enumeration(other_value, config);

    if(other_value_enum->group)
    {
	if(ev->constant->group != other_value_enum->group)
	{
	    return ESSTEE_FALSE;
	}
    }

    return ESSTEE_TRUE;
}

void st_enum_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: enum value destructor */
}

int st_enum_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    const struct enum_item_t *other_value_enum = other_value->enumeration(other_value, config);
    
    if(strcmp(ev->constant->identifier, other_value_enum->identifier) == 0)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

const struct enum_item_t * st_enum_value_enumeration(
    const struct value_iface_t *self,
    const struct config_iface_t *conf)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    return ev->constant;
}

/**************************************************************************/
/* Subrange value                                                         */
/**************************************************************************/
int st_subrange_value_display(
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

int st_subrange_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    int can_hold = sv->explicit_type->can_hold(
	sv->explicit_type,
	new_value,
	config);
	
    if(can_hold != ESSTEE_TRUE)
    {
	return can_hold;
    }

    return sv->current->assign(sv->current, new_value, config);
}

const struct type_iface_t * st_subrange_value_explicit_type(
    const struct value_iface_t *self)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->explicit_type;
}

int st_subrange_value_compatible(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    /* Cannot check if type can hold other_value, since other_value
     * will be 0 when it is a temporary (in verification), which may
     * fall outside of the subrange, so these errors, even for
     * constant literals, will take place at runtime */
    
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    if(!other_value->integer)
    {
	return ESSTEE_FALSE;
    }

    if(other_value->explicit_type)
    {
	const struct type_iface_t *other_value_type =
	    other_value->explicit_type(other_value);
	
	return sv->explicit_type->compatible(sv->explicit_type,
					     other_value_type,
					     config);
    }

    return ESSTEE_TRUE;
}

struct value_iface_t * st_subrange_value_create_temp_from(
    const struct value_iface_t *self)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);
    
    return sv->current->create_temp_from(sv->current);
}

int st_subrange_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->current->greater(sv->current, other_value, config);
}

int st_subrange_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->current->lesser(sv->current, other_value, config);
}

int st_subrange_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->current->equals(sv->current, other_value, config);
}

void st_subrange_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: subrange value destructor */
}

int64_t st_subrange_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->current->integer(sv->current, config);
}

const struct array_init_value_t * st_array_init_value(
    const struct value_iface_t *self,
    const struct config_iface_t *conf)
{
    struct array_init_value_t *av =
	CONTAINER_OF(self, struct array_init_value_t, value);

    return av;
}

/**************************************************************************/
/* Array value                                                            */
/**************************************************************************/
int st_array_value_compatible(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    const struct array_value_t *av =
	CONTAINER_OF(self, struct array_value_t, value);

    int type_can_hold_result = av->type->can_hold(av->type,
						  other_value,
						  config);
    if(type_can_hold_result != ESSTEE_TRUE)
    {
	return type_can_hold_result;
    }
    
    return ESSTEE_TRUE;
}

int st_array_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config)
{
    const struct array_value_t *av =
	CONTAINER_OF(self, struct array_value_t, value);

    int elements_assigned =
	st_array_type_assign_default_value(av->elements, new_value, config);

    if(elements_assigned <= 0)
    {
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

int st_array_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct array_value_t *av =
	CONTAINER_OF(self, struct array_value_t, value);

    int written_bytes = snprintf(buffer,
				 buffer_size,
				 "[");

    size_t buffer_size_start = buffer_size;

    CHECK_WRITTEN_BYTES(written_bytes);
    buffer += written_bytes;
    buffer_size -= written_bytes;
    
    for(size_t i = 0; i < av->total_elements; i++)
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

	int element_displayed_bytes  =
	    av->elements[i]->display(av->elements[i], buffer, buffer_size, config);

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

const struct type_iface_t * st_array_value_explicit_type(
    const struct value_iface_t *self)
{
    const struct array_value_t *av =
    	CONTAINER_OF(self, struct array_value_t, value);

    return av->type;
}

struct value_iface_t * st_array_value_index(
    struct value_iface_t *self,
    struct array_index_t *array_index,
    const struct config_iface_t *config)
{
    const struct array_value_t *av =
    	CONTAINER_OF(self, struct array_value_t, value);

    const struct array_range_t *range_itr = av->ranges;
    const struct array_index_t *index_itr = NULL;

    size_t elements_offset = 0;
    
    DL_FOREACH(array_index, index_itr)
    {
	const struct value_iface_t *index_value =
	    index_itr->index_expression->return_value(index_itr->index_expression);

	if(!range_itr)
	{
	    return NULL;
	}
	
	if(!index_value->integer)
	{
	    return NULL;
	}

	int index_too_small =
	    range_itr->subrange->min->greater(range_itr->subrange->min,
							   index_value,
							   config);
	if(index_too_small != ESSTEE_FALSE)
	{
	    return NULL;
	}

	int index_too_large =
	    range_itr->subrange->max->lesser(range_itr->subrange->max,
							       index_value,
							       config);
	if(index_too_large != ESSTEE_FALSE)
	{
	    return NULL;
	}

	int64_t index_num = index_value->integer(index_value, config);
	int64_t min_index_num = range_itr->subrange->min->integer(
	    range_itr->subrange->min,
	    config);
	
	size_t multiplier = 1;
	if(range_itr->next)
	{
	    multiplier = 0;
	    const struct array_range_t *ritr = NULL;
	    DL_FOREACH(range_itr->next, ritr)
	    {
		multiplier += ritr->entries;
	    }
	}
	
	elements_offset += (index_num-min_index_num) * multiplier;
	
	range_itr = range_itr->next;
    }

    return av->elements[elements_offset];
}    

void st_array_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: array value destructor */
}

void st_array_init_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: array init value destructor */
}

/**************************************************************************/
/* Structure value                                                        */
/**************************************************************************/
int st_struct_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    size_t buffer_size_start = buffer_size;

    int start_written_bytes = snprintf(buffer,
				       buffer_size,
				       "(");
    CHECK_WRITTEN_BYTES(start_written_bytes);
    buffer += start_written_bytes;
    buffer_size -= start_written_bytes;

    struct variable_t *itr = NULL;
    int first_element = 1;
    for(itr = sv->elements; itr != NULL; itr = itr->hh.next)
    {
	int written_bytes = 0;
	
	if(first_element)
	{
	    first_element = 0;
	}
	else
	{
	    written_bytes = snprintf(buffer,
				     buffer_size,
				     ",");

	    CHECK_WRITTEN_BYTES(written_bytes);
	    buffer += written_bytes;
	    buffer_size -= written_bytes;
	}
	
	written_bytes = snprintf(buffer,
				 buffer_size,
				 "%s:",
				 itr->identifier);

	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;

	written_bytes = itr->value->display(itr->value,
					    buffer,
					    buffer_size,
					    config);
	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;
    }
    
    int end_written_bytes = snprintf(buffer,
				     buffer_size,
				     ")");
    CHECK_WRITTEN_BYTES(end_written_bytes);
    buffer += end_written_bytes;
    buffer_size -= end_written_bytes;

    return buffer_size_start - buffer_size;
}

const struct type_iface_t * st_struct_value_explicit_type(
    const struct value_iface_t *self)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    return sv->explicit_type;
}

int st_struct_value_compatible(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    int type_can_hold_result = sv->explicit_type->can_hold(sv->explicit_type,
							   other_value,
							   config);
    if(type_can_hold_result != ESSTEE_TRUE)
    {
	return type_can_hold_result;
    }
    
    return ESSTEE_TRUE;
}

int st_struct_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    const struct struct_init_value_t *isv =
	new_value->struct_init_value(new_value, config);

    struct struct_element_init_t *itr = NULL;
    for(itr = isv->init_table; itr != NULL; itr = itr->hh.next)
    {
	struct variable_t *found = NULL;
	HASH_FIND_STR(sv->elements, itr->element_identifier, found);
	if(!found)
	{
	    return ESSTEE_ERROR;
	}

	int assign_result = found->value->assign(found->value, itr->element_default_value, config);
	if(assign_result != ESSTEE_OK)
	{
	    return assign_result;
	}
    }

    return ESSTEE_OK;
}

void st_struct_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: struct value destructor */
}

void st_struct_init_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: struct init value destructor */
}

struct variable_t * st_struct_value_sub_variable(
    struct value_iface_t *self,
    const char *identifier,
    const struct config_iface_t *config)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    struct variable_t *found = NULL;
    HASH_FIND_STR(sv->elements, identifier, found);

    return found;
}

const struct struct_init_value_t * st_struct_init_value(
    const struct value_iface_t *self,
    const struct config_iface_t *conf)
{
    const struct struct_init_value_t *isv =
	CONTAINER_OF(self, struct struct_init_value_t, value);

    return isv;
}
