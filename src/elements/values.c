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
#include <rt/cursor.h>
#include <linker/linker.h>

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

st_bitflag_t st_general_value_empty_class(
    const struct value_iface_t *self)
{
    return 0;
}

int st_general_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    int greater = self->greater(self, other_value, config, issues);
    if(greater == ESSTEE_TRUE)
    {
	return ESSTEE_FALSE;
    }

    int lesser = self->lesser(self, other_value, config, issues);
    if(lesser == ESSTEE_TRUE)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

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
    CHECK_WRITTEN_BYTES(written_bytes);

    return written_bytes;
}

int st_integer_value_assign(
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

int st_integer_value_assignable_from(
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

int st_integer_value_compares_and_operates(
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

const struct type_iface_t * st_integer_value_type_of(
    const struct value_iface_t *self)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    return iv->type;
}

struct value_iface_t * st_integer_value_create_temp_from(
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
    clone->value.assign = st_integer_value_assign;
    
    ST_SET_FLAGS(clone->class, TEMPORARY_VALUE);
    
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

int st_integer_value_lesser(
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

int st_integer_value_equals(
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

int st_integer_value_negate(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    iv->num *= -1;

    return ESSTEE_OK;
}

int st_integer_value_plus(
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

int st_integer_value_minus(
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

int st_integer_value_multiply(
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

int st_integer_value_divide(
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

int st_integer_value_modulus(
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

int st_integer_value_to_power(
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

int64_t st_integer_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    return iv->num;
}

st_bitflag_t st_integer_value_class(
    const struct value_iface_t *self)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    return iv->class;
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

int st_bool_value_assigns_compares_operates(
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

int st_bool_value_assign(
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

struct value_iface_t * st_bool_value_create_temp_from(
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
    clone->value.assign = st_bool_value_assign;

    ST_SET_FLAGS(clone->class, TEMPORARY_VALUE);
    
    return &(clone->value);

error_free_resources:
    return NULL;
}

int st_bool_value_equals(
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

int st_bool_value_bool(
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

int st_bool_value_not(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    iv->num = (iv->num == 0) ? 1 : 0;

    return ESSTEE_OK;
}

int st_bool_value_xor(
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

int st_bool_value_and(
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
    
int st_bool_value_or(
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

int st_integer_literal_override_type(
    const struct value_iface_t *self,
    const struct type_iface_t *type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    iv->type = type;
    iv->value.type_of = st_integer_value_type_of;

    return ESSTEE_OK;
}

/**************************************************************************/
/* Real values                                                            */
/**************************************************************************/
int st_real_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    int written_bytes = snprintf(buffer, buffer_size, "%.2f", rv->num);
    CHECK_WRITTEN_BYTES(written_bytes);

    return written_bytes;
}

int st_real_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    if(!ST_FLAG_IS_SET(rv->class, TEMPORARY_VALUE))
    {
	int type_can_hold = rv->type->can_hold(rv->type, new_value, config, issues);

	if(type_can_hold != ESSTEE_TRUE)
	{
	    return type_can_hold;
	}
    }

    rv->num = new_value->real(new_value, config, issues);

    return ESSTEE_OK;
}

int st_real_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->real)
    {
	issues->new_issue(
	    issues,
	    "new value cannot be interpreted as a float number",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }

    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    if(ST_FLAG_IS_SET(rv->class, TEMPORARY_VALUE))
    {
	return ESSTEE_TRUE;
    }
    
    st_bitflag_t other_value_class = other_value->class(other_value);
    
    if(!ST_FLAG_IS_SET(other_value_class, TEMPORARY_VALUE))
    {
	int type_can_hold = rv->type->can_hold(rv->type,
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

	    int types_compatible = rv->type->compatible(rv->type,
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

int st_real_value_compares_and_operates(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{ 
    if(!other_value->real)
    {
	issues->new_issue(
	    issues,
	    "the other value cannot be interpreted as a float number",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }

    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    if(ST_FLAG_IS_SET(rv->class, TEMPORARY_VALUE))
    {
	return ESSTEE_TRUE;
    }

    st_bitflag_t other_value_class = other_value->class(other_value);
    
    if(!ST_FLAG_IS_SET(other_value_class, TEMPORARY_VALUE))
    {
	if(other_value->type_of)
	{
	    const struct type_iface_t *other_value_type =
		other_value->type_of(other_value);

	    int types_compatible = rv->type->compatible(rv->type,
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

const struct type_iface_t * st_real_value_type_of(
    const struct value_iface_t *self)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    return rv->type;
}

struct value_iface_t * st_real_value_create_temp_from(
    const struct value_iface_t *self,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    struct real_value_t *clone = NULL;
    ALLOC_OR_ERROR_JUMP(
	clone,
	struct real_value_t,
	issues,
	error_free_resources);

    memcpy(clone, rv, sizeof(struct real_value_t));

    clone->num = 0;
    clone->type = NULL;
    clone->value.assign = st_real_value_assign;
    ST_SET_FLAGS(clone->class, TEMPORARY_VALUE);
    
    return &(clone->value);

error_free_resources:
    return NULL;
}

void st_real_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: real value destroy */
}

int st_real_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    if(rv->num > other_value->real(other_value, config, issues))
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

int st_real_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    if(rv->num < other_value->real(other_value, config, issues))
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

int st_real_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    double other_value_num = other_value->real(other_value, config, issues);

    if(!(rv->num > other_value_num + 1e-4) && !(rv->num < other_value_num - 1e-4))
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

int st_real_value_negate(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    rv->num *= -1;

    return ESSTEE_OK;
}

int st_real_value_plus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    rv->num += other_value->real(other_value, config, issues);

    return ESSTEE_OK;
}

int st_real_value_minus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    rv->num -= other_value->real(other_value, config, issues);

    return ESSTEE_OK;
}

int st_real_value_multiply(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    rv->num *= other_value->real(other_value, config, issues);

    return ESSTEE_OK;
}

int st_real_value_divide(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    rv->num /= other_value->real(other_value, config, issues);

    return ESSTEE_OK;
}

int st_real_value_to_power(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    double dnum = rv->num;
    double exp = other_value->real(other_value, config, issues);

    errno = 0;
    double result = pow(dnum, exp);
    if(errno != 0)
    {
	issues->new_issue(issues,
			  "evaluation of power expression failed",
			  ESSTEE_ARGUMENT_ERROR);
	
	return ESSTEE_ERROR;
    }
    
    rv->num = result;

    return ESSTEE_OK;
}

double st_real_value_real(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    return rv->num;
}

st_bitflag_t st_real_value_class(
    const struct value_iface_t *self)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    return rv->class;
}

int st_real_literal_override_type(
    const struct value_iface_t *self,
    const struct type_iface_t *type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    rv->type = type;
    rv->value.type_of = st_real_value_type_of;

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
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    int type_can_hold = ev->type->can_hold(ev->type,
					   new_value,
					   config,
					   issues);

    if(type_can_hold != ESSTEE_TRUE)
    {
	return type_can_hold;
    }
    
    ev->constant = new_value->enumeration(new_value,
					  config,
					  issues);

    return ESSTEE_OK;
}

const struct type_iface_t * st_enum_value_type_of(
    const struct value_iface_t *self)
{
    struct enum_value_t *ev =
	CONTAINER_OF(self, struct enum_value_t, value);

    return ev->type;
}

int st_enum_value_assigns_and_compares(
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

    /* If the other value has a reference to its values group, check
     * that the group is the same as the self group. Inline enum
     * values do not have any values group, since they lack a
     * reference to their type. */
    const struct enum_item_t *other_value_enum = other_value->enumeration(other_value,
									  config,
									  issues);

    if(other_value_enum->group)
    {
	if(ev->constant->group != other_value_enum->group)
	{
	    issues->new_issue(
		issues,
		"enums are not part of the same enum type",
		ESSTEE_TYPE_ERROR);
	    
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

const struct enum_item_t * st_enum_value_enumeration(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
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

    return sv->current->assign(sv->current, new_value, config, issues);
}

const struct type_iface_t * st_subrange_value_type_of(
    const struct value_iface_t *self)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->type;
}

int st_subrange_value_assignable_from(
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
	
	int type_can_hold = sv->type->can_hold(sv->type, other_value, config, issues);

	if(type_can_hold != ESSTEE_TRUE)
	{
	    return type_can_hold;
	}

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

int st_subrange_value_compares_and_operates(
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

struct value_iface_t * st_subrange_value_create_temp_from(
    const struct value_iface_t *self,
    struct issues_iface_t *issues)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);
    
    return sv->current->create_temp_from(sv->current, issues);
}

int st_subrange_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->current->greater(sv->current, other_value, config, issues);
}

int st_subrange_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->current->lesser(sv->current, other_value, config, issues);
}

int st_subrange_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->current->equals(sv->current, other_value, config, issues);
}

void st_subrange_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: subrange value destructor */
}

int64_t st_subrange_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_value_t *sv =
	CONTAINER_OF(self, struct subrange_value_t, value);

    return sv->current->integer(sv->current, config, issues);
}

/**************************************************************************/
/* Array value                                                            */
/**************************************************************************/
const struct array_init_value_t * st_array_init_value(
    const struct value_iface_t *self)
{
    struct array_init_value_t *av =
	CONTAINER_OF(self, struct array_init_value_t, value);

    return av;
}

int st_array_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct array_value_t *av =
	CONTAINER_OF(self, struct array_value_t, value);

    int type_can_hold = av->type->can_hold(av->type, other_value, config, issues);

    if(type_can_hold != ESSTEE_TRUE)
    {
	return type_can_hold;
    }

    return ESSTEE_TRUE;
}

int st_array_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct array_value_t *av =
	CONTAINER_OF(self, struct array_value_t, value);

    int elements_assigned = st_array_type_assign_default_value(av->elements,
							       new_value,
							       config,
							       issues);

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

const struct type_iface_t * st_array_value_type_of(
    const struct value_iface_t *self)
{
    const struct array_value_t *av =
    	CONTAINER_OF(self, struct array_value_t, value);

    return av->type;
}

struct value_iface_t * st_array_value_index(
    struct value_iface_t *self,
    struct array_index_t *array_index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
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
	int64_t min_index_num =
	    range_itr->subrange->min->integer(range_itr->subrange->min,
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

const struct type_iface_t * st_struct_value_type_of(
    const struct value_iface_t *self)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    return sv->type;
}

int st_struct_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    int type_can_hold = sv->type->can_hold(sv->type, other_value, config, issues);

    if(type_can_hold != ESSTEE_TRUE)
    {
	return type_can_hold;
    }
    
    return ESSTEE_TRUE;
}

int st_struct_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    const struct struct_init_value_t *isv =
	new_value->struct_init_value(new_value);

    struct struct_element_init_t *itr = NULL;
    for(itr = isv->init_table; itr != NULL; itr = itr->hh.next)
    {
	struct variable_t *found = NULL;
	HASH_FIND_STR(sv->elements, itr->element_identifier, found);
	if(!found)
	{
	    issues->internal_error(
		issues,
		__FILE__,
		__FUNCTION__,
		__LINE__);
	    
	    return ESSTEE_ERROR;
	}

	int assign_result = found->value->assign(found->value,
						 itr->element_default_value,
						 config,
						 issues);
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
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    struct variable_t *found = NULL;
    HASH_FIND_STR(sv->elements, identifier, found);
    if(!found)
    {
	issues->new_issue(
	    issues,
	    "struct has no member '%s'",
	    ESSTEE_ARGUMENT_ERROR,
	    identifier);

	return NULL;
    }
    
    return found;
}

const struct struct_init_value_t * st_struct_init_value(
    const struct value_iface_t *self)
{
    const struct struct_init_value_t *isv =
	CONTAINER_OF(self, struct struct_init_value_t, value);

    return isv;
}

/**************************************************************************/
/* Duration value                                                         */
/**************************************************************************/
int st_duration_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct duration_value_t *dv =
	CONTAINER_OF(self, struct duration_value_t, value);

    int total_written_bytes = 0;
    if(dv->duration.d > 0.0)
    {
	int written_bytes = snprintf(buffer,
				     buffer_size,
				     "%.2fd",
				     dv->duration.d);
	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;
	total_written_bytes += written_bytes;
    }
    
    if(dv->duration.h > 0.0)
    {
	int written_bytes = snprintf(buffer,
				     buffer_size,
				     "%.2fh",
				     dv->duration.h);
	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;
	total_written_bytes += written_bytes;
    }

    if(dv->duration.m > 0.0)
    {
	int written_bytes = snprintf(buffer,
				     buffer_size,
				     "%.2fm",
				     dv->duration.m);
	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;
	total_written_bytes += written_bytes;
    }

    if(dv->duration.s > 0.0)
    {
	int written_bytes = snprintf(buffer,
				     buffer_size,
				     "%.2fs",
				     dv->duration.s);
	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;
	total_written_bytes += written_bytes;
    }

    if(dv->duration.ms > 0.0)
    {
	int written_bytes = snprintf(buffer,
				     buffer_size,
				     "%.2fms",
				     dv->duration.ms);
	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;
	total_written_bytes += written_bytes;
    }

    if(total_written_bytes == 0)
    {
	int written_bytes = snprintf(buffer,
				     buffer_size,
				     "%.2fms",
				     0.0);
	CHECK_WRITTEN_BYTES(written_bytes);
	total_written_bytes += written_bytes;
    }

    return total_written_bytes;
}

int st_duration_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct duration_value_t *dv =
	CONTAINER_OF(self, struct duration_value_t, value);

    const struct duration_t *ov =
	new_value->duration(new_value, config, issues);

    dv->duration.d = ov->d;
    dv->duration.h = ov->h;
    dv->duration.m = ov->m;
    dv->duration.s = ov->s;
    dv->duration.ms = ov->ms;

    return ESSTEE_OK;
}

int st_duration_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->duration)
    {
	issues->new_issue(
	    issues,
	    "the other value cannot be interpreted as a duration",
	    ESSTEE_ARGUMENT_ERROR);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

const struct type_iface_t * st_duration_value_type_of(
    const struct value_iface_t *self)
{
    struct duration_value_t *dv =
	CONTAINER_OF(self, struct duration_value_t, value);

    return dv->type;
}

void st_duration_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: duration value destructor */
}

int st_duration_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct duration_value_t *dv =
	CONTAINER_OF(self, struct duration_value_t, value);

    const struct duration_t *ov =
	other_value->duration(other_value, config, issues);

    if(dv->duration.d > ov->d)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.h > ov->h)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.m > ov->m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.s > ov->s)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.ms > ov->ms)
    {
	return ESSTEE_TRUE;
    }
    
    return ESSTEE_FALSE;
}

int st_duration_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct duration_value_t *dv =
	CONTAINER_OF(self, struct duration_value_t, value);

    const struct duration_t *ov =
	other_value->duration(other_value, config, issues);

    if(dv->duration.d < ov->d)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.h < ov->h)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.m < ov->m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.s < ov->s)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.ms < ov->ms)
    {
	return ESSTEE_TRUE;
    }
    
    return ESSTEE_FALSE;
}

const struct duration_t * st_duration_value_duration(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct duration_value_t *dv =
	CONTAINER_OF(self, struct duration_value_t, value);

    return &(dv->duration);
}

/**************************************************************************/
/* Date value                                                             */
/**************************************************************************/
int st_date_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct date_value_t *dv =
	CONTAINER_OF(self, struct date_value_t, value);

    int written_bytes = snprintf(buffer,
				 buffer_size,
				 "%" PRIu64 "-%" PRIu8 "-%" PRIu8,
				 dv->date.y,
				 dv->date.m,
				 dv->date.d);
    CHECK_WRITTEN_BYTES(written_bytes);

    return written_bytes;
}

int st_date_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_value_t *dv =
	CONTAINER_OF(self, struct date_value_t, value);

    const struct date_t *ov =
	new_value->date(new_value, config, issues);

    dv->date.y = ov->y;
    dv->date.m = ov->m;
    dv->date.d = ov->d;

    return ESSTEE_OK;
}

int st_date_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->date)
    {
	issues->new_issue(
	    issues,
	    "the other value cannot be interpreted as a date",
	    ESSTEE_ARGUMENT_ERROR);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

const struct type_iface_t * st_date_value_type_of(
    const struct value_iface_t *self)
{
    struct date_value_t *dv =
	CONTAINER_OF(self, struct date_value_t, value);

    return dv->type;
}

void st_date_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: date destructor */
}

int st_date_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_value_t *dv =
	CONTAINER_OF(self, struct date_value_t, value);

    const struct date_t *ov =
	other_value->date(other_value, config, issues);

    if(dv->date.y > ov->y)
    {
	return ESSTEE_TRUE;
    }

    if(dv->date.m > ov->m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->date.d > ov->d)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

int st_date_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_value_t *dv =
	CONTAINER_OF(self, struct date_value_t, value);

    const struct date_t *ov =
	other_value->date(other_value, config, issues);

    if(dv->date.y < ov->y)
    {
	return ESSTEE_TRUE;
    }

    if(dv->date.m < ov->m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->date.d < ov->d)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

const struct date_t * st_date_value_date(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_value_t *dv =
	CONTAINER_OF(self, struct date_value_t, value);

    return &(dv->date);
}

/**************************************************************************/
/* Tod value                                                              */
/**************************************************************************/
int st_tod_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct tod_value_t *tv =
	CONTAINER_OF(self, struct tod_value_t, value);

    int written_bytes = snprintf(buffer,
				 buffer_size,
				 "%" PRIu8 "h%" PRIu8 "m%" PRIu8 ".%" PRIu8 "s",
				 tv->tod.h,
				 tv->tod.m,
				 tv->tod.s,
				 tv->tod.fs);
    CHECK_WRITTEN_BYTES(written_bytes);

    return written_bytes;
}

int st_tod_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct tod_value_t *tv =
	CONTAINER_OF(self, struct tod_value_t, value);

    const struct tod_t *ov =
	new_value->tod(new_value, config, issues);

    tv->tod.h = ov->h;
    tv->tod.m = ov->m;
    tv->tod.s = ov->s;
    tv->tod.fs = ov->fs;

    return ESSTEE_OK;
}

int st_tod_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->tod)
    {
	issues->new_issue(
	    issues,
	    "the other value cannot be interpreted as a tod",
	    ESSTEE_ARGUMENT_ERROR);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

const struct type_iface_t * st_tod_value_type_of(
    const struct value_iface_t *self)
{
    struct tod_value_t *tv =
	CONTAINER_OF(self, struct tod_value_t, value);

    return tv->type;
}

void st_tod_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: tod value destructor */
}

int st_tod_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct tod_value_t *tv =
	CONTAINER_OF(self, struct tod_value_t, value);

    const struct tod_t *ov =
	other_value->tod(other_value, config, issues);

    if(tv->tod.h > ov->h)
    {
	return ESSTEE_TRUE;
    }
    
    if(tv->tod.m > ov->m)
    {
	return ESSTEE_TRUE;
    }

    if(tv->tod.s > ov->s)
    {
	return ESSTEE_TRUE;
    }

    if(tv->tod.fs > ov->fs)
    {
	return ESSTEE_TRUE;
    }
    
    return ESSTEE_FALSE;
}

int st_tod_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct tod_value_t *tv =
	CONTAINER_OF(self, struct tod_value_t, value);

    const struct tod_t *ov =
	other_value->tod(other_value, config, issues);

    if(tv->tod.h < ov->h)
    {
	return ESSTEE_TRUE;
    }
    
    if(tv->tod.m < ov->m)
    {
	return ESSTEE_TRUE;
    }

    if(tv->tod.s < ov->s)
    {
	return ESSTEE_TRUE;
    }

    if(tv->tod.fs < ov->fs)
    {
	return ESSTEE_TRUE;
    }
    
    return ESSTEE_FALSE;
}

const struct tod_t * st_tod_value_tod(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct tod_value_t *tv =
	CONTAINER_OF(self, struct tod_value_t, value);

    return &(tv->tod);
}

/**************************************************************************/
/* Date tod value                                                         */
/**************************************************************************/
int st_date_tod_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct date_tod_value_t *dv =
	CONTAINER_OF(self, struct date_tod_value_t, value);

    int total_written_bytes = 0;
    int written_bytes = snprintf(buffer,
				 buffer_size,
				 "%" PRIu64 "-%" PRIu8 "-%" PRIu8,
				 dv->dt.date.y,
				 dv->dt.date.m,
				 dv->dt.date.d);
    CHECK_WRITTEN_BYTES(written_bytes);
    total_written_bytes += written_bytes;
    
    written_bytes = snprintf(buffer,
			     buffer_size,
			     "%" PRIu8 "h%" PRIu8 "m%" PRIu8 ".%" PRIu8 "s",
			     dv->dt.tod.h,
			     dv->dt.tod.m,
			     dv->dt.tod.s,
			     dv->dt.tod.fs);
    CHECK_WRITTEN_BYTES(written_bytes);
    total_written_bytes += written_bytes;

    return total_written_bytes;
}

int st_date_tod_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_tod_value_t *dv =
	CONTAINER_OF(self, struct date_tod_value_t, value);

    const struct date_tod_t *ov =
	new_value->date_tod(new_value, config, issues);

    dv->dt.date.y = ov->date.y;
    dv->dt.date.m = ov->date.m;
    dv->dt.date.d = ov->date.d;
    dv->dt.tod.h = ov->tod.h;
    dv->dt.tod.m = ov->tod.m;
    dv->dt.tod.s = ov->tod.s;
    dv->dt.tod.fs = ov->tod.fs;
    
    return ESSTEE_OK;
}

int st_date_tod_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->date_tod)
    {
	issues->new_issue(
	    issues,
	    "the other value cannot be interpreted as a date tod",
	    ESSTEE_ARGUMENT_ERROR);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

const struct type_iface_t * st_date_tod_value_type_of(
    const struct value_iface_t *self)
{
    struct date_tod_value_t *dv =
	CONTAINER_OF(self, struct date_tod_value_t, value);

    return dv->type;
}

void st_date_tod_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: date tod value destructor */
}

int st_date_tod_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_tod_value_t *dv =
	CONTAINER_OF(self, struct date_tod_value_t, value);

    const struct date_tod_t *ov =
	other_value->date_tod(other_value, config, issues);

    if(dv->dt.date.y > ov->date.y)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.date.m > ov->date.m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.date.d > ov->date.d)
    {
	return ESSTEE_TRUE;
    }
    
    if(dv->dt.tod.h > ov->tod.h)
    {
	return ESSTEE_TRUE;
    }
    
    if(dv->dt.tod.m > ov->tod.m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.tod.s > ov->tod.s)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.tod.fs > ov->tod.fs)
    {
	return ESSTEE_TRUE;
    }
    
    return ESSTEE_FALSE;
}

int st_date_tod_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_tod_value_t *dv =
	CONTAINER_OF(self, struct date_tod_value_t, value);

    const struct date_tod_t *ov =
	other_value->date_tod(other_value, config, issues);

    if(dv->dt.date.y < ov->date.y)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.date.m < ov->date.m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.date.d < ov->date.d)
    {
	return ESSTEE_TRUE;
    }
    
    if(dv->dt.tod.h < ov->tod.h)
    {
	return ESSTEE_TRUE;
    }
    
    if(dv->dt.tod.m < ov->tod.m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.tod.s < ov->tod.s)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.tod.fs < ov->tod.fs)
    {
	return ESSTEE_TRUE;
    }
    
    return ESSTEE_FALSE;
}

const struct date_tod_t * st_date_tod_value_tod(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_tod_value_t *dv =
	CONTAINER_OF(self, struct date_tod_value_t, value);

    return &(dv->dt);
}

/**************************************************************************/
/* String values                                                          */
/**************************************************************************/
int st_string_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    struct string_value_t *sv =
	CONTAINER_OF(self, struct string_value_t, value);

    int written_bytes = snprintf(buffer,
				 buffer_size,
				 "%s",
				 sv->str);
    CHECK_WRITTEN_BYTES(written_bytes);
    return written_bytes;
}

int st_string_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct string_value_t *sv =
	CONTAINER_OF(self, struct string_value_t, value);

    sv->str = new_value->string(new_value, config, issues);

    return ESSTEE_OK;
}

int st_string_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct string_value_t *sv =
	CONTAINER_OF(self, struct string_value_t, value);

    return sv->type->can_hold(sv->type, other_value, config, issues);
}

const struct type_iface_t * st_string_value_type_of(
    const struct value_iface_t *self)
{
    struct string_value_t *sv =
	CONTAINER_OF(self, struct string_value_t, value);

    return sv->type;
}

void st_string_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: string value destroy */
}

void st_string_literal_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: string literal value destructor */
}

int st_string_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct string_value_t *sv =
	CONTAINER_OF(self, struct string_value_t, value);

    const char *other_string = other_value->string(other_value, config, issues);

    if(strcmp(sv->str, other_string) == 0)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

const char * st_string_value_string(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct string_value_t *sv =
	CONTAINER_OF(self, struct string_value_t, value);

    return sv->str;
}

/**************************************************************************/
/* Function block value                                                   */
/**************************************************************************/
int st_function_block_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    struct function_block_value_t *fv =
	CONTAINER_OF(self, struct function_block_value_t, value);

    int start_buffer_size = buffer_size;
    int start_written_bytes =  snprintf(buffer,
					buffer_size,
					"%s:(",
					fv->type->identifier);
    CHECK_WRITTEN_BYTES(start_written_bytes);
    buffer += start_written_bytes;
    buffer_size -= start_written_bytes;

    struct variable_t *itr = NULL;
    DL_FOREACH(fv->variables, itr)
    {
	int var_name_bytes = snprintf(buffer,
				      buffer_size,
				      "%s:",
				      itr->identifier);
	CHECK_WRITTEN_BYTES(var_name_bytes);
	buffer += var_name_bytes;
	buffer_size -= var_name_bytes;
	
	int var_written_bytes = itr->value->display(itr->value,
						    buffer,
						    buffer_size,
						    config);
	CHECK_WRITTEN_BYTES(var_written_bytes);
	buffer += var_written_bytes;
	buffer_size -= var_written_bytes;

	if(itr->next)
	{
	    int comma_written_bytes = snprintf(buffer,
					       buffer_size,
					       ",");
	    
	    CHECK_WRITTEN_BYTES(comma_written_bytes);
	    buffer += comma_written_bytes;
	    buffer_size -= comma_written_bytes;
	}
    }

    int end_written_bytes =  snprintf(buffer,
				      buffer_size,
				      ")");
    CHECK_WRITTEN_BYTES(end_written_bytes);
    buffer_size -= end_written_bytes;

    return start_buffer_size-buffer_size;
}

const struct type_iface_t * st_function_block_value_type_of(
    const struct value_iface_t *self)
{
    struct function_block_value_t *fv =
	CONTAINER_OF(self, struct function_block_value_t, value);

    return fv->type;
}

void st_function_block_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: function block value destructor */
}

struct variable_t * st_function_block_value_sub_variable(
    struct value_iface_t *self,
    const char *identifier,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct function_block_value_t *fv =
	CONTAINER_OF(self, struct function_block_value_t, value);

    struct variable_t *found = NULL;
    HASH_FIND_STR(fv->variables, identifier, found);
    if(!found)
    {
	issues->new_issue(
	    issues,
	    "function block has no variable '%s'",
	    ESSTEE_ARGUMENT_ERROR,
	    identifier);
    }

    return found;
}

int st_function_block_value_invoke_step(
    struct value_iface_t *self,
    struct invoke_parameter_t *parameters,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct function_block_value_t *fbv =
	CONTAINER_OF(self, struct function_block_value_t, value);

    if(fbv->invoke_state > 0)
    {
	return INVOKE_RESULT_FINISHED;
    }
    
    int input_assign = st_assign_from_invoke_parameters(parameters,
							fbv->variables,
							config,
							issues);
    if(input_assign != ESSTEE_OK)
    {
	return INVOKE_RESULT_ERROR;
    }

    fbv->invoke_state = 1;
    
    st_switch_current(cursor, fbv->statements, config, issues);

    return INVOKE_RESULT_IN_PROGRESS;
}

int st_function_block_value_invoke_verify(
    struct value_iface_t *self,
    struct invoke_parameter_t *parameters,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct function_block_value_t *fbv =
	CONTAINER_OF(self, struct function_block_value_t, value);

    return st_verify_invoke_parameters(parameters,
				       fbv->variables,
				       config,
				       issues);
}

int st_function_block_value_invoke_reset(
    struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct function_block_value_t *fbv =
	CONTAINER_OF(self, struct function_block_value_t, value);

    fbv->invoke_state = 0;

    return ESSTEE_OK;
}

/**************************************************************************/
/* Inline values                                                          */
/**************************************************************************/
int st_inline_enum_value_comparable_to(
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

int st_inline_enum_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct inline_enum_value_t *ie
	= CONTAINER_OF(self, struct inline_enum_value_t, value);

    const struct enum_item_t *other_value_enum = other_value->enumeration(other_value, config, issues);
    
    if(strcmp(ie->data.identifier, other_value_enum->identifier) == 0)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

const struct enum_item_t * st_inline_enum_value_enumeration(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct inline_enum_value_t *ie
	= CONTAINER_OF(self, struct inline_enum_value_t, value);

    return &(ie->data);
}

void st_inline_enum_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: inline enum value destructor */
}

int st_subrange_case_value_comparable_to(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(other_value->integer)
    {
	issues->new_issue(
	    issues,
	    "the other value cannot be interpreted as an integer",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }

    return ESSTEE_FALSE;
}

int st_subrange_case_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_case_value_t *sv =
	CONTAINER_OF(self, struct subrange_case_value_t, value);


    int min_greater_than_other = sv->subrange->min->greater(sv->subrange->min,
							    other_value,
							    config,
							    issues);
    if(min_greater_than_other == ESSTEE_TRUE)
    {
	return ESSTEE_FALSE;
    }
    
    int max_lesser_than_other = sv->subrange->max->lesser(sv->subrange->max,
							  other_value,
							  config,
							  issues);
    if(max_lesser_than_other == ESSTEE_TRUE)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

void st_subrange_case_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: inline subrange case value destructor */
}

/**************************************************************************/
/* Direct address term value                                              */
/**************************************************************************/
int st_direct_address_term_value_comparable_to(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    if(ST_FLAG_IS_SET(dv->address->class, BIT_UNIT_ADDRESS))
    {
	if(!other_value->bool)
	{
	    issues->new_issue(issues,
			      "a direct address referring to a bit may only be compared booleans",
			      ESSTEE_CONTEXT_ERROR);

	    return ESSTEE_ERROR;
	}
    }
    else
    {
	if(!other_value->integer)
	{
	    issues->new_issue(issues,
			      "multi bit direct address can only be compared to integers",
			      ESSTEE_CONTEXT_ERROR);

	    return ESSTEE_ERROR;
	}
    }

    return ESSTEE_OK;
}

int st_direct_address_term_value_operates_with(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->integer)
    {
	issues->new_issue(issues,
			  "multi bit direct address can only be used in expression with integers",
			  ESSTEE_CONTEXT_ERROR);

	return ESSTEE_ERROR;
    }
    
    return ESSTEE_OK;
}

struct value_iface_t * st_direct_address_term_value_create_temp_from(
    const struct value_iface_t *self,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    struct value_iface_t *v = NULL;
    if(ST_FLAG_IS_SET(dv->address->class, BIT_UNIT_ADDRESS))
    {
	v = st_bool_type_create_value_of(NULL,
					 NULL, /* TODO: fix dependency on config */
					 issues);
    }
    else
    {
	v = st_integer_type_create_value_of(NULL,
					    NULL, /* TODO: fix dependency on config */
					    issues);
    }

    if(!v)
    {
	return NULL;
    }
    
    struct integer_value_t *iv =
	CONTAINER_OF(v, struct integer_value_t, value);

    ST_SET_FLAGS(iv->class, TEMPORARY_VALUE);

    return v;
}

int st_direct_address_term_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    int64_t other_value_num = other_value->integer(other_value,
						   config,
						   issues);

    if(dv->data > other_value_num)
    {
	return ESSTEE_TRUE;
    }
       
    return ESSTEE_FALSE;
}

int st_direct_address_term_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    int64_t other_value_num = other_value->integer(other_value,
						   config,
						   issues);

    if(dv->data < other_value_num)
    {
	return ESSTEE_TRUE;
    }
       
    return ESSTEE_FALSE;
}
    
int st_direct_address_term_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    int64_t other_value_num;
    
    if(ST_FLAG_IS_SET(dv->address->class, BIT_UNIT_ADDRESS))
    {
	other_value_num = other_value->bool(other_value,
					    config,
					    issues);
    }
    else
    {
	other_value_num = other_value->integer(other_value,
					    config,
					    issues);
    }

    if(dv->data == other_value_num)
    {
	return ESSTEE_TRUE;
    }
       
    return ESSTEE_FALSE;
}

int64_t st_direct_address_term_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    return dv->data;
}

int st_direct_address_term_value_bool(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    if(dv->data == 0)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}
