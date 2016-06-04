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

#include <elements/reals.h>
#include <elements/types.h>
#include <util/macros.h>

#include <utlist.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

/**************************************************************************/
/* Value interface                                                        */
/**************************************************************************/
struct real_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    st_bitflag_t class;
    double num;
};

static int real_value_display(
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

static int real_value_assign(
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

static int real_value_assignable_from(
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

static int real_value_compares_and_operates(
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

static const struct type_iface_t * real_value_type_of(
    const struct value_iface_t *self)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    return rv->type;
}

static struct value_iface_t * real_value_create_temp_from(
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

    clone->num = 0.0;
    clone->type = NULL;
    clone->value.type_of = NULL;
    clone->value.assign = real_value_assign;
    ST_SET_FLAGS(clone->class, TEMPORARY_VALUE);
    
    return &(clone->value);

error_free_resources:
    return NULL;
}

static void real_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: real value destroy */
}

static int real_value_greater(
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

static int real_value_lesser(
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

static int real_value_equals(
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

static int real_value_negate(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    rv->num *= -1;

    return ESSTEE_OK;
}

static int real_value_plus(
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

static int real_value_minus(
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

static int real_value_multiply(
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

static int real_value_divide(
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

static int real_value_to_power(
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

static double real_value_real(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    return rv->num;
}

static st_bitflag_t real_value_class(
    const struct value_iface_t *self)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    return rv->class;
}

static int real_value_override_type(
    const struct value_iface_t *self,
    const struct type_iface_t *type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv =
	CONTAINER_OF(self, struct real_value_t, value);

    rv->type = type;
    rv->value.type_of = real_value_type_of;

    return ESSTEE_OK;
}

/**************************************************************************/
/* Real types                                                             */
/**************************************************************************/
struct real_type_t {
    struct type_iface_t type;
    unsigned size;
    st_bitflag_t class;
    double default_value;
};

static struct value_iface_t * real_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_value_t *rv = NULL;
    ALLOC_OR_ERROR_JUMP(
	rv,
	struct real_value_t,
	issues,
	error_free_resources);

    rv->type = self;
    memset(&(rv->value), 0, sizeof(struct value_iface_t));

    rv->value.display = real_value_display;
    rv->value.assign = real_value_assign;
    rv->value.type_of = real_value_type_of;
    rv->value.assignable_from = real_value_assignable_from;
    rv->value.comparable_to = real_value_compares_and_operates;
    rv->value.operates_with = real_value_compares_and_operates;
    rv->value.create_temp_from = real_value_create_temp_from;
    rv->value.destroy = real_value_destroy;
    rv->value.class = real_value_class;
    rv->value.override_type = real_value_override_type;

    rv->value.greater = real_value_greater;
    rv->value.lesser = real_value_lesser;
    rv->value.equals = real_value_equals;

    rv->value.negate = real_value_negate;
    rv->value.plus = real_value_plus;
    rv->value.minus = real_value_minus;
    rv->value.multiply = real_value_multiply;
    rv->value.divide = real_value_divide;
    rv->value.to_power = real_value_to_power;
    
    rv->value.real = real_value_real;

    return &(rv->value);
    
error_free_resources:
    return NULL;
}

static int real_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct real_type_t *rt =
	CONTAINER_OF(self, struct real_type_t, type);

    struct real_value_t *rv
	= CONTAINER_OF(value_of, struct real_value_t, value);

    rv->num = rt->default_value;
    
    return ESSTEE_OK;
}

static int real_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!value->real)
    {
	issues->new_issue(
	    issues,
	    "type '%s' can only hold float values",
	    ESSTEE_TYPE_ERROR,
	    self->identifier);

	return ESSTEE_FALSE;
    }
    
    return ESSTEE_TRUE;
}

static st_bitflag_t real_type_class(
    const struct type_iface_t *self)
{
    struct real_type_t *rt =
	CONTAINER_OF(self, struct real_type_t, type);

    return rt->class;
}

static void real_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: real type destructor */
}

/**************************************************************************/
/* Public functions                                                       */
/**************************************************************************/
static struct real_type_t real_type_templates[] = {
    {	.type = {
	    .create_value_of = real_type_create_value_of,
	    .reset_value_of = real_type_reset_value_of,
	    .can_hold = real_type_can_hold,
	    .class = real_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = real_type_destroy,
	    .identifier = "REAL",
	},
	.size = 4,
	.class = REAL_TYPE,
	.default_value = 0.0,
    },
    {	.type = {
	    .create_value_of = real_type_create_value_of,
	    .reset_value_of = real_type_reset_value_of,
	    .can_hold = real_type_can_hold,
	    .class = real_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = real_type_destroy,
	    .identifier = "LREAL",
	},
	.size = 8,
	.class = LREAL_TYPE,
	.default_value = 0.0,
    },
};

struct type_iface_t * st_new_elementary_real_types()
{
    struct real_type_t *real_types = NULL;
    struct type_iface_t *real_type_list = NULL;
    
    size_t num_real_types =
	sizeof(real_type_templates)/sizeof(struct real_type_t);

    ALLOC_ARRAY_OR_JUMP(
	real_types,
	struct real_type_t,
	num_real_types,
	error_free_resources);
    
    for(int i=0; i < num_real_types; i++)  
    { 
	memcpy(
	    &(real_types[i]),
	    &(real_type_templates[i]),
	    sizeof(struct real_type_t));

	DL_APPEND(real_type_list, &(real_types[i].type));
    } 

    return real_type_list;
    
error_free_resources:
    return NULL;
}


struct value_iface_t * st_new_typeless_real_value(
    double num,
    st_bitflag_t value_class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct value_iface_t *v =
	real_type_create_value_of(NULL, config, issues);

    if(!v)
    {
	return NULL;
    }

    v->type_of = NULL;
    
    struct real_value_t *rv =
	CONTAINER_OF(v, struct real_value_t, value);

    rv->num = num;
    rv->class = value_class;

    return v;
}
