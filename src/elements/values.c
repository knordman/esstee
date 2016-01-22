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

    return ESSTEE_TRUE;
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
    /* TODO: integer value modulus */
    return ESSTEE_OK;
}

int st_integer_value_to_power(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    /* TODO: integer value to power */
    return ESSTEE_OK;
}

int64_t st_integer_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv =
	CONTAINER_OF(self, struct integer_value_t, value);

    return iv->num;
}