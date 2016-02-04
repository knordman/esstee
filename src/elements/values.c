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

#include <stdio.h>
#include <math.h>
#include <errno.h>

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

int st_integer_value_reset(
    struct value_iface_t *self,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int reset_result = iv->explicit_type->reset_value_of(
	iv->explicit_type,
	self,
	config);
    
    if(reset_result != ESSTEE_OK)
    {
	return reset_result;
    }

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

int st_enum_value_reset(
    struct value_iface_t *self,
    const struct config_iface_t *config)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    return ev->explicit_type->reset_value_of(ev->explicit_type,
					     self,
					     config);
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

int st_subrange_value_reset(
    struct value_iface_t *self,
    const struct config_iface_t *config)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->explicit_type->reset_value_of(sv->explicit_type,
					     self,
					     config);
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
