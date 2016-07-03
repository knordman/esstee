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

#include <elements/integers.h>
#include <elements/types.h>
#include <util/macros.h>

#include <utlist.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

/**************************************************************************/
/* Integer values                                                         */
/**************************************************************************/
struct integer_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    st_bitflag_t class;
    int64_t num;
};

static int integer_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int written_bytes = snprintf(buffer, buffer_size, "%" PRId64, iv->num);
    CHECK_WRITTEN_BYTES(written_bytes);

    return written_bytes;
}

static int integer_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(!ST_FLAG_IS_SET(iv->class, TEMPORARY_VALUE))
    {
	int type_can_hold = iv->type->can_hold(iv->type, new_value, config, issues);

	if(type_can_hold != ESSTEE_TRUE)
	{
	    return type_can_hold;
	}
    }

    iv->num = new_value->integer(new_value, config, issues);

    return ESSTEE_OK;
}

static int integer_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->integer)
    {
	issues->new_issue(
	    issues,
	    "new value cannot be interpreted as an integer",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }

    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(ST_FLAG_IS_SET(iv->class, TEMPORARY_VALUE))
    {
	return ESSTEE_TRUE;
    }
    
    st_bitflag_t other_value_class = other_value->class(other_value);

    if(ST_FLAG_IS_SET(other_value_class, CONSTANT_VALUE))
    {
	int type_can_hold = iv->type->can_hold(iv->type,
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
		iv->type->compatible(iv->type,
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

static int integer_value_compares_and_operates(
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

    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(ST_FLAG_IS_SET(iv->class, TEMPORARY_VALUE))
    {
	return ESSTEE_TRUE;
    }

    st_bitflag_t other_value_class = other_value->class(other_value);
    
    if(!ST_FLAG_IS_SET(other_value_class, TEMPORARY_VALUE))
    {
	if(other_value->type_of && iv->type)
	{
	    const struct type_iface_t *other_value_type =
		other_value->type_of(other_value);

	    int types_compatible = iv->type->compatible(iv->type,
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

static const struct type_iface_t * integer_value_type_of(
    const struct value_iface_t *self)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    return iv->type;
}

static struct value_iface_t * integer_value_create_temp_from(
    const struct value_iface_t *self,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    struct integer_value_t *clone = NULL;
    ALLOC_OR_ERROR_JUMP(
	clone,
	struct integer_value_t,
	issues,
	error_free_resources);

    memcpy(clone, iv, sizeof(struct integer_value_t));

    /* Temporary value has no initial value */
    clone->num = 0;
    
    /* Temporary has no type */
    clone->type = NULL;

    /* Temporary is assignable */
    clone->value.assign = integer_value_assign;
    
    ST_SET_FLAGS(clone->class, TEMPORARY_VALUE);
    
    return &(clone->value);

error_free_resources:
    return NULL;
}

static void integer_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: integer value destroy */
}

static int integer_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(iv->num > other_value->integer(other_value, config, issues))
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

static int integer_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(iv->num < other_value->integer(other_value, config, issues))
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

static int integer_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(iv->num == other_value->integer(other_value, config, issues))
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

static int integer_value_negate(
    struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    iv->num *= -1;

    return ESSTEE_OK;
}

static int integer_value_plus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    iv->num += other_value->integer(other_value, config, issues);

    return ESSTEE_OK;
}

static int integer_value_minus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    iv->num -= other_value->integer(other_value, config, issues);

    return ESSTEE_OK;
}

static int integer_value_multiply(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    iv->num *= other_value->integer(other_value, config, issues);

    return ESSTEE_OK;
}

static int integer_value_divide(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int64_t other_value_num = other_value->integer(other_value, config, issues);

    if(other_value_num == 0)
    {
	issues->new_issue(
	    issues,
	    "division by zero",
	    ESSTEE_ARGUMENT_ERROR);

	return ESSTEE_ERROR;
    }
    
    iv->num /= other_value_num;

    return ESSTEE_OK;
}

static int integer_value_modulus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    iv->num %= other_value->integer(other_value, config, issues);

    return ESSTEE_OK;
}

static int integer_value_to_power(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    long double dnum = (long double)iv->num;
    long double exp = (long double)other_value->integer(other_value, config, issues);

    errno = 0;
    long double result = powl(dnum, exp);
    if(errno != 0)
    {
	issues->new_issue(issues,
			  "evaluation of power expression failed",
			  ESSTEE_ARGUMENT_ERROR);
	
	return ESSTEE_ERROR;
    }
    
    iv->num = (int64_t)result;

    return ESSTEE_OK;
}

static int64_t integer_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    return iv->num;
}

static st_bitflag_t integer_value_class(
    const struct value_iface_t *self)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    return iv->class;
}

static int bool_value_display(
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

static int bool_value_assigns_compares_operates(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->bool)
    {
	issues->new_issue(
	    issues,
	    "the other value cannot be interpreted as a boolean",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }
    
    return ESSTEE_TRUE;
}

static int bool_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(new_value->bool(new_value, config, issues) == ESSTEE_TRUE)
    {
	iv->num = 1;
    }
    else
    {
	iv->num = 0;
    }

    return ESSTEE_OK;
}

static struct value_iface_t * bool_value_create_temp_from(
    const struct value_iface_t *self,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    struct integer_value_t *clone = NULL;
    ALLOC_OR_ERROR_JUMP(
	clone,
	struct integer_value_t,
	issues,
	error_free_resources);

    memcpy(clone, iv, sizeof(struct integer_value_t));

    /* Temporary value has no initial value */
    clone->num = 0;
    
    /* Temporary has no type */
    clone->type = NULL;

    /* Temporary is assignable */
    clone->value.assign = bool_value_assign;

    ST_SET_FLAGS(clone->class, TEMPORARY_VALUE);
    
    return &(clone->value);

error_free_resources:
    return NULL;
}

static int bool_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(self->bool(self, config, issues) == other_value->bool(other_value, config, issues))
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

static int bool_value_bool(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    if(iv->num == 0)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

static int bool_value_not(
    struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    iv->num = (iv->num == 0) ? 1 : 0;

    return ESSTEE_OK;
}

static int bool_value_xor(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int64_t ov = (other_value->bool(other_value, config, issues) == ESSTEE_TRUE) ? 1 : 0;

    iv->num ^= ov;
    
    return ESSTEE_OK;
}

static int bool_value_and(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int64_t ov = (other_value->bool(other_value, config, issues) == ESSTEE_TRUE) ? 1 : 0;
    
    iv->num &= ov;
    
    return ESSTEE_OK;
}
    
static int bool_value_or(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    int64_t ov = (other_value->bool(other_value, config, issues) == ESSTEE_TRUE) ? 1 : 0;

    iv->num |= ov;
    
    return ESSTEE_OK;
}

static int integer_value_override_type(
    const struct value_iface_t *self,
    const struct type_iface_t *type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    iv->type = type;
    iv->value.type_of = integer_value_type_of;

    return ESSTEE_OK;
}

/**************************************************************************/
/* Integer types                                                          */
/**************************************************************************/
struct integer_type_t {
    struct type_iface_t type;
    st_bitflag_t class;
    size_t size;
    int64_t default_value;
    int64_t min;
    int64_t max;
};

static struct value_iface_t * integer_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv = NULL;
    ALLOC_OR_ERROR_JUMP(
	iv,
	struct integer_value_t,
	issues,
	error_free_resources);
    
    iv->type = self;
    memset(&(iv->value), 0, sizeof(struct value_iface_t));
    
    iv->value.display = integer_value_display;
    iv->value.assign = integer_value_assign;
    iv->value.type_of = integer_value_type_of;
    iv->value.assignable_from = integer_value_assignable_from;
    iv->value.comparable_to = integer_value_compares_and_operates;
    iv->value.operates_with = integer_value_compares_and_operates;
    iv->value.create_temp_from = integer_value_create_temp_from;
    iv->value.destroy = integer_value_destroy;
    iv->value.class = integer_value_class;
    iv->value.override_type = integer_value_override_type;

    iv->value.greater = integer_value_greater;
    iv->value.lesser = integer_value_lesser;
    iv->value.equals = integer_value_equals;

    iv->value.negate = integer_value_negate;
    iv->value.plus = integer_value_plus;
    iv->value.minus = integer_value_minus;
    iv->value.multiply = integer_value_multiply;
    iv->value.divide = integer_value_divide;
    iv->value.modulus = integer_value_modulus;
    iv->value.to_power = integer_value_to_power;
    
    iv->value.integer = integer_value_integer;

    return &(iv->value);
    
error_free_resources:
    return NULL;
}

static int integer_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_type_t *it =
	CONTAINER_OF(self, struct integer_type_t, type);

    struct integer_value_t *iv
	= CONTAINER_OF(value_of, struct integer_value_t, value);

    iv->num = it->default_value;
    
    return ESSTEE_OK;
}

static void integer_type_sync_direct_memory(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct direct_address_t *address,
    int write)
{
    struct integer_type_t *it =
	CONTAINER_OF(self, struct integer_type_t, type);

    struct integer_value_t *iv
	= CONTAINER_OF(value_of, struct integer_value_t, value);
    
    if(!address->storage)
    {
	return;
    }

    size_t data_size = address->field_size_bits / 8;
    
    if(write)
    {    
	memset(address->storage, 0, data_size);

	switch(data_size)
	{
	case 8:
	    memcpy(address->storage, &iv->num, 8);
	case 4: {
	    uint32_t data = (uint32_t)iv->num;
	    memcpy(address->storage, &data, 4);
	}
	case 2: {
	    uint16_t data = (uint16_t)iv->num;
	    memcpy(address->storage, &data, 2);
	}
	case 1:
	    *(address->storage) = (uint8_t)iv->num;
	}
    }
    else
    {
	switch(data_size)
	{
	case 8:
	    iv->num = *((int64_t *)address->storage);
	case 4:
	    if(ST_FLAG_IS_SET(it->class, INTEGER_DINT_TYPE))
	    {
		iv->num = *((int32_t *)address->storage);
	    }
	    else
	    {
		iv->num = *((uint32_t *)address->storage);
	    }
	case 2:
	    if(ST_FLAG_IS_SET(it->class, INTEGER_INT_TYPE))
	    {
		iv->num = *((int16_t *)address->storage);
	    }
	    else
	    {
		iv->num = *((uint16_t *)address->storage);
	    }
	case 1:
	    if(ST_FLAG_IS_SET(it->class, INTEGER_SINT_TYPE))
	    {
		iv->num = *((int8_t *)address->storage);
	    }
	    else
	    {
		iv->num = *(address->storage);
	    }
	}
    }
}

static int integer_type_validate_direct_address(
    const struct type_iface_t *self,
    struct direct_address_t *address,
    struct issues_iface_t *issues)
{
    struct integer_type_t *it =
	CONTAINER_OF(self, struct integer_type_t, type);

    size_t type_size = it->size * 8;
    
    if(address->field_size_bits != 0)
    {
	if(type_size > address->field_size_bits)
	{
	    const char *bits_text = (address->field_size_bits == 1) ? "bit" : "bits";
	    
	    issues->new_issue(issues,
			      "type '%s' needs '%lu' bits of storage and does not fit into explicilty given '%lu' %s",
			      ESSTEE_CONTEXT_ERROR,
			      self->identifier,
			      type_size,
			      address->field_size_bits,
			      bits_text);

	    return ESSTEE_ERROR;
	}
    }

    address->field_size_bits = type_size;

    return ESSTEE_OK;
}

static int integer_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_type_t *it =
	CONTAINER_OF(self, struct integer_type_t, type);

    if(!value->integer)
    {
	issues->new_issue(
	    issues,
	    "type '%s' can only hold integer values",
	    ESSTEE_TYPE_ERROR,
	    self->identifier);

	return ESSTEE_FALSE;
    }

    int64_t intval = value->integer(value, config, issues);

    if(intval > it->max)
    {
	issues->new_issue(
	    issues,
	    "type '%s' can at maximum hold a value of '%" PRId64 "', not '%" PRId64 "'",
	    ESSTEE_TYPE_ERROR,
	    self->identifier,
	    it->max,
	    intval);

	return ESSTEE_FALSE;	
    }
    
    if(intval < it->min)
    {
	issues->new_issue(
	    issues,
	    "type '%s' can at minimum hold a value of '%" PRId64 "', not '%" PRId64 "'",
	    ESSTEE_TYPE_ERROR,
	    self->identifier,
	    it->min,
	    intval);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

static st_bitflag_t integer_type_class(
    const struct type_iface_t *self)
{
    struct integer_type_t *it =
	CONTAINER_OF(self, struct integer_type_t, type);

    return it->class;
}


static void integer_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: integer type destructor */
}

/* Bool specializations */
static struct value_iface_t * bool_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv = NULL;
    ALLOC_OR_ERROR_JUMP(
	iv,
	struct integer_value_t,
	issues,
	error_free_resources);
    
    iv->type = self;
    memset(&(iv->value), 0, sizeof(struct value_iface_t));
    
    iv->value.display = bool_value_display;
    iv->value.assign = bool_value_assign;
    iv->value.type_of = integer_value_type_of;
    iv->value.assignable_from = bool_value_assigns_compares_operates;
    iv->value.comparable_to = bool_value_assigns_compares_operates;
    iv->value.operates_with = bool_value_assigns_compares_operates; 
    iv->value.create_temp_from = bool_value_create_temp_from;
    iv->value.destroy = integer_value_destroy;
    iv->value.class = integer_value_class;
    iv->value.override_type = integer_value_override_type;

    iv->value.equals = bool_value_equals;

    iv->value.not = bool_value_not;
    iv->value.xor = bool_value_xor;
    iv->value.and = bool_value_and;
    iv->value.or = bool_value_or;
    
    iv->value.bool = bool_value_bool;

    return &(iv->value);
    
error_free_resources:
    return NULL;
}

static void bool_type_sync_direct_memory(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct direct_address_t *address,
    int write)
{
    struct integer_value_t *iv
	= CONTAINER_OF(value_of, struct integer_value_t, value);
    
    if(!address->storage)
    {
	return;
    }

    size_t bit_offset = address->bit_offset % 8;
    uint8_t mask = (1 << bit_offset);
    
    if(write)
    {
	if(iv->num == 0)
	{
	    *(address->storage) &= ~mask;
	}
	else
	{
	    *(address->storage) |= mask;
	}
    }
    else
    {
	iv->num = mask & *(address->storage);
    }	
}

static int bool_type_validate_direct_address(
    const struct type_iface_t *self,
    struct direct_address_t *address,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}

static int bool_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!value->bool)
    {
	issues->new_issue(
	    issues,
	    "type '%s' can only hold boolean values",
	    ESSTEE_TYPE_ERROR,
	    self->identifier);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

/**************************************************************************/
/* Public functions                                                       */
/**************************************************************************/
static struct integer_type_t integer_type_templates[] = {
    {	.type = {
	    .create_value_of = bool_type_create_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .sync_direct_memory = bool_type_sync_direct_memory,
	    .validate_direct_address = bool_type_validate_direct_address,
	    .can_hold = bool_type_can_hold,
	    .class = integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = integer_type_destroy,
	    .identifier = "BOOL",
	},
	.size = 1,
	.default_value = 0,
	.class = INTEGER_BOOL_TYPE,
	.min = 0,
	.max = 1
    },
    {	.type = {
	    .create_value_of = integer_type_create_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .sync_direct_memory = integer_type_sync_direct_memory,
	    .validate_direct_address = integer_type_validate_direct_address,
	    .can_hold = integer_type_can_hold,
	    .class = integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = integer_type_destroy,
	    .identifier = "SINT",
	},
	.size = 1,
	.default_value = 0,
	.class = INTEGER_SINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_SIGNED,
	.min = -127,
	.max = 127
    },
    {	.type = {
	    .create_value_of = integer_type_create_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .sync_direct_memory = integer_type_sync_direct_memory,
	    .validate_direct_address = integer_type_validate_direct_address,
	    .can_hold = integer_type_can_hold,
	    .class = integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = integer_type_destroy,
	    .identifier = "INT",
	},
	.size = 2,
	.default_value = 0,
	.class = INTEGER_INT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_SIGNED,
	.min = -32767,
	.max = 32767,
    },
    {	.type = {
	    .create_value_of = integer_type_create_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .sync_direct_memory = integer_type_sync_direct_memory,
	    .validate_direct_address = integer_type_validate_direct_address,
	    .can_hold = integer_type_can_hold,
	    .class = integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = integer_type_destroy,
	    .identifier = "DINT",
	},
	.size = 4,
	.default_value = 0,
	.class = INTEGER_DINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_SIGNED,
	.min = -2147483647,
	.max = 2147483647
    },
    {	.type = {
	    .create_value_of = integer_type_create_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .sync_direct_memory = integer_type_sync_direct_memory,
	    .can_hold = integer_type_can_hold,
	    .class = integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = integer_type_destroy,
	    .identifier = "LINT",
	},
	.size = 8,
	.default_value = 0,
	.class = INTEGER_LINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_SIGNED,
	.min = -9223372036854775806,
	.max = 9223372036854775806
    },
    {	.type = {
	    .create_value_of = integer_type_create_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .sync_direct_memory = integer_type_sync_direct_memory,
	    .validate_direct_address = integer_type_validate_direct_address,
	    .can_hold = integer_type_can_hold,
	    .class = integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = integer_type_destroy,
	    .identifier = "USINT",
	},
	.size = 1,
	.default_value = 0,
	.class = INTEGER_USINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_UNSIGNED,
	.min = 0,
	.max = 255
    },
    {	.type = {
	    .create_value_of = integer_type_create_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .sync_direct_memory = integer_type_sync_direct_memory,
	    .validate_direct_address = integer_type_validate_direct_address,
	    .can_hold = integer_type_can_hold,
	    .class = integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = integer_type_destroy,
	    .identifier = "UINT",
	},
	.size = 2,
	.default_value = 0,
	.class = INTEGER_UINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_UNSIGNED,
	.min = 0,
	.max = 65535 
    },
    {	.type = {
	    .create_value_of = integer_type_create_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .sync_direct_memory = integer_type_sync_direct_memory,
	    .validate_direct_address = integer_type_validate_direct_address,
	    .can_hold = integer_type_can_hold,
	    .class = integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = integer_type_destroy,
	    .identifier = "UDINT",
	},
	.size = 4,
	.default_value = 0,
	.class = INTEGER_UDINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_UNSIGNED,
	.min = 0,
	.max = 4294967295
    },
    {	.type = {
	    .create_value_of = integer_type_create_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .sync_direct_memory = integer_type_sync_direct_memory,
	    .validate_direct_address = integer_type_validate_direct_address,
	    .can_hold = integer_type_can_hold,
	    .class = integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = integer_type_destroy,
	    .identifier = "ULINT",
	},
	.size = 8,
	.default_value = 0,
	.class = INTEGER_ULINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_UNSIGNED,
	.min = 0,
	.max = 9223372036854775806 /* Full range not supported */
    },
    {	.type = {
	    .create_value_of = integer_type_create_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .sync_direct_memory = integer_type_sync_direct_memory,
	    .validate_direct_address = integer_type_validate_direct_address,
	    .can_hold = integer_type_can_hold,
	    .class = integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = integer_type_destroy,
	    .identifier = "BYTE",
	},
	.size = 1,
	.default_value = 0x00,
	.class = INTEGER_BYTE_TYPE|INTEGER_BITDATA_CLASS,
	.min = 0x00,
	.max = 0xff,
    },
    {	.type = {
	    .create_value_of = integer_type_create_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .sync_direct_memory = integer_type_sync_direct_memory,
	    .validate_direct_address = integer_type_validate_direct_address,
	    .can_hold = integer_type_can_hold,
	    .class = integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = integer_type_destroy,
	    .identifier = "WORD",
	},
	.size = 2,
	.default_value = 0x0000,
	.class = INTEGER_WORD_TYPE|INTEGER_BITDATA_CLASS,
	.min = 0x0000,
	.max = 0xffff,
    },
    {	.type = {
	    .create_value_of = integer_type_create_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .sync_direct_memory = integer_type_sync_direct_memory,
	    .validate_direct_address = integer_type_validate_direct_address,
	    .can_hold = integer_type_can_hold,
	    .class = integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = integer_type_destroy,
	    .identifier = "DWORD",
	},
	.size = 4,
	.default_value = 0x00000000,
	.class = INTEGER_DWORD_TYPE|INTEGER_BITDATA_CLASS,
	.min = 0x00000000,
	.max = 0xffffffff,
    },
    {	.type = {
	    .create_value_of = integer_type_create_value_of,
	    .reset_value_of = integer_type_reset_value_of,
	    .sync_direct_memory = integer_type_sync_direct_memory,
	    .validate_direct_address = integer_type_validate_direct_address,
	    .can_hold = integer_type_can_hold,
	    .class = integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = integer_type_destroy,
	    .identifier = "LWORD",
	},
	.size = 8,
	.default_value = 0x0000000000000000,
	.class = INTEGER_LWORD_TYPE|INTEGER_BITDATA_CLASS,
	.min = 0x0000000000000000,
	.max = 0xffffffffffffffff,
    },
};

struct type_iface_t * st_new_elementary_integer_types()
{
    struct integer_type_t *integer_types = NULL;
    struct type_iface_t *integer_type_list = NULL;

    size_t num_integer_types =
	sizeof(integer_type_templates)/sizeof(struct integer_type_t);
    
    ALLOC_ARRAY_OR_JUMP(
	integer_types,
	struct integer_type_t,
	num_integer_types,
	error_free_resources);

    for(int i=0; i < num_integer_types; i++)  
    {
	memcpy(
	    &(integer_types[i]),
	    &(integer_type_templates[i]),
	    sizeof(struct integer_type_t));

	DL_APPEND(integer_type_list, &(integer_types[i].type));
    } 

    return integer_type_list;
    
error_free_resources:
    return NULL;
}

struct value_iface_t * st_new_typeless_integer_value(
    int64_t num,
    st_bitflag_t value_class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct value_iface_t *v =
	integer_type_create_value_of(NULL, config, issues);

    if(!v)
    {
	return NULL;
    }
    
    v->type_of = NULL;
    
    struct integer_value_t *iv =
	CONTAINER_OF(v, struct integer_value_t, value);

    iv->num = num;
    iv->class = value_class;

    return v;
}

int st_integer_value_set(
    struct value_iface_t *value,
    int64_t num)
{
    struct integer_value_t *iv
	= CONTAINER_OF(value, struct integer_value_t, value);

    iv->num = num;

    return ESSTEE_OK;
}

struct value_iface_t * st_new_bool_value(
    int state,
    st_bitflag_t value_class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct value_iface_t *v =
	bool_type_create_value_of(NULL, config, issues);

    if(!v)
    {
	return NULL;
    }
    
    v->type_of = NULL;
    
    struct integer_value_t *iv =
	CONTAINER_OF(v, struct integer_value_t, value);

    iv->num = (state == ESSTEE_TRUE) ? 1 : 0;
    iv->class = value_class;

    return v;
}

int st_set_bool_value_state(
    struct value_iface_t *value,
    int state)
{
    struct integer_value_t *iv =
	CONTAINER_OF(value, struct integer_value_t, value);

    iv->num = (state == ESSTEE_TRUE) ? 1 : 0;

    return ESSTEE_OK;
}
