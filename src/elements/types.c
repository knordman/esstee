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

#include <elements/types.h>
#include <elements/values.h>
#include <util/macros.h>
#include <util/bitflag.h>

#include <utlist.h>


/**************************************************************************/
/* Elementary types                                                       */
/**************************************************************************/
static struct integer_type_t integer_type_templates[] = {
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = NULL,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .class = INTEGER_BOOL_TYPE,
	    .identifier = "BOOL",
	},
	.size = 1,
	.default_value = 0,
	.min = 0,
	.max = 1
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .class = INTEGER_SINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_SIGNED,
	    .identifier = "SINT",
	},
	.size = 1,
	.default_value = 0,
	.min = -127,
	.max = 127
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .class = INTEGER_INT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_SIGNED,
	    .identifier = "INT",
	},
	.size = 2,
	.default_value = 0,
	.min = -32767,
	.max = 32767,
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .class = INTEGER_DINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_SIGNED,
	    .identifier = "DINT",
	},
	.size = 4,
	.default_value = 0,
	.min = -2147483647,
	.max = 2147483647
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .class = INTEGER_LINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_SIGNED,
	    .identifier = "LINT",
	},
	.size = 8,
	.default_value = 0,
	.min = -9223372036854775806,
	.max = 9223372036854775806
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .class = INTEGER_USINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_UNSIGNED,
	    .identifier = "USINT",
	},
	.size = 1,
	.default_value = 0,
	.min = 0,
	.max = 255
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .class = INTEGER_UINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_UNSIGNED,
	    .identifier = "UINT",
	},
	.size = 2,
	.default_value = 0,
	.min = 0,
	.max = 65535 
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .class = INTEGER_UDINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_UNSIGNED,
	    .identifier = "UDINT",
	},
	.size = 4,
	.default_value = 0,
	.min = 0,
	.max = 4294967295
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .class = INTEGER_ULINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_UNSIGNED,
	    .identifier = "ULINT",
	},
	.size = 8,
	.default_value = 0,
	.min = 0,
	.max = 9223372036854775806 /* Full range not supported */
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .class = INTEGER_BYTE_TYPE|INTEGER_BITDATA_CLASS,
	    .identifier = "BYTE",
	},
	.size = 1,
	.default_value = 0x00,
	.min = 0x00,
	.max = 0xff,
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .class = INTEGER_WORD_TYPE|INTEGER_BITDATA_CLASS,
	    .identifier = "WORD",
	},
	.size = 2,
	.default_value = 0x0000,
	.min = 0x0000,
	.max = 0xffff,
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .class = INTEGER_DWORD_TYPE|INTEGER_BITDATA_CLASS,
	    .identifier = "DWORD",
	},
	.size = 4,
	.default_value = 0x00000000,
	.min = 0x00000000,
	.max = 0xffffffff,
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .class = INTEGER_LWORD_TYPE|INTEGER_BITDATA_CLASS,
	    .identifier = "LWORD",
	},
	.size = 8,
	.default_value = 0x0000000000000000,
	.min = 0x0000000000000000,
	.max = 0xffffffffffffffff,
    },
};

static struct real_type_t real_type_templates[] = {
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_real_type_create_value_of,
	    .can_hold = NULL,
	    .compatible = st_type_general_compatible,
	    .destroy = st_real_type_destroy,
	    .class = REAL_TYPE,
	    .identifier = "REAL",
	},
	.size = 4,
	.default_value = 0.0,
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_real_type_create_value_of,
	    .can_hold = NULL,
	    .compatible = st_type_general_compatible,
	    .destroy = st_real_type_destroy,
	    .class = LREAL_TYPE,
	    .identifier = "LREAL",
	},
	.size = 8,
	.default_value = 0.0,
    },
};

static struct string_type_t string_type_templates[] = {
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_string_type_create_value_of,
	    .can_hold = NULL,
	    .compatible = st_type_general_compatible,
	    .destroy = st_string_type_destroy,
	    .class = STRING_TYPE,
	    .identifier = "STRING",
	},
	.default_value = "",
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_string_type_create_value_of,
	    .can_hold = NULL,
	    .compatible = st_type_general_compatible,
	    .destroy = st_string_type_destroy,
	    .class = WSTRING_TYPE,
	    .identifier = "WSTRING",
	},
	.default_value = "",
    },
};

static struct duration_type_t duration_type_template = {
    .type = {
	.location = st_built_in_type_location_get,
	.create_value_of = st_duration_type_create_value_of,
	.can_hold = NULL,
	.compatible = st_type_general_compatible,
	.destroy = st_duration_type_destroy,
	.class = TIME_TYPE,
	.identifier = "TIME",
    },
    .default_d = 0.0,
    .default_h = 0.0,
    .default_m = 0.0,
    .default_s = 0.0,
    .default_ms = 0.0,
};

static struct date_type_t date_type_template = {
    .type = {
	.location = st_built_in_type_location_get,
	.create_value_of = st_date_type_create_value_of,
	.can_hold = NULL,
	.compatible = st_type_general_compatible,
	.destroy = st_date_type_destroy,
	.class = DATE_TYPE,
	.identifier = "DATE",
    },
    .default_year = 1,
    .default_month = 1,
    .default_day = 1
};

static struct tod_type_t tod_type_template = {
    .type = {
	.location = st_built_in_type_location_get,
	.create_value_of = st_tod_type_create_value_of,
	.can_hold = NULL,
	.compatible = st_type_general_compatible,
	.destroy = st_tod_type_destroy,
	.class = TOD_TYPE,
	.identifier = "TIME_OF_DAY",
    },
    .default_hour = 0,
    .default_minute = 0,
    .default_second = 0,
};

static struct date_tod_type_t date_tod_type_template = {
    .type = {
	.location = st_built_in_type_location_get,
	.create_value_of = st_date_tod_type_create_value_of,
	.can_hold = NULL,
	.compatible = st_type_general_compatible,
	.destroy = st_date_tod_type_destroy,
	.class = DATE_TOD_TYPE,
	.identifier = "DATE_AND_TIME",
    },
    .default_year = 1,
    .default_month = 1,
    .default_day = 1,
    .default_hour = 0,
    .default_minute = 0,
    .default_second = 0,
};

const struct st_location_t * st_built_in_type_location_get(
    const struct type_iface_t *self)
{
    return NULL;
}

int st_type_general_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config)
{
    if(self->class == other_type->class)
    {
	return ESSTEE_TRUE;
    }

    /* Clear derived flag from both types */
    st_bitflag_t self_class_cmp = self->class & (~DERIVED_TYPE);
    st_bitflag_t other_type_class_cmp = other_type->class & (~DERIVED_TYPE);

    if(self_class_cmp == other_type_class_cmp)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}


struct type_iface_t * st_new_elementary_types(void) 
{
    struct type_iface_t *elementary_types = NULL; 
    struct integer_type_t *integer_types = NULL;
    struct real_type_t *real_types = NULL;
    struct string_type_t *string_types = NULL;
    struct duration_type_t *duration_type = NULL;
    struct date_type_t *date_type = NULL;
    struct tod_type_t *tod_type = NULL;
    struct date_tod_type_t *date_tod_type = NULL;
    
    /* Integer types */
    size_t num_integer_types = sizeof(integer_type_templates)/sizeof(struct integer_type_t);
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

	DL_APPEND(elementary_types, &(integer_types[i].type));
    } 

    /* Real types */
    size_t num_real_types = sizeof(real_type_templates)/sizeof(struct real_type_t);
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

	DL_APPEND(elementary_types, &(real_types[i].type));
    } 

    /* String types */
    size_t num_string_types = sizeof(string_type_templates)/sizeof(struct string_type_t);
    ALLOC_ARRAY_OR_JUMP(
	string_types,
	struct string_type_t,
	num_string_types,
	error_free_resources);

    for(int i=0; i < num_string_types; i++)  
    { 
	memcpy(
	    &(string_types[i]),
	    &(string_type_templates[i]),
	    sizeof(struct string_type_t));

	DL_APPEND(elementary_types, &(string_types[i].type));
    }

    /* Duration type */
    ALLOC_OR_JUMP(
	duration_type,
	struct duration_type_t,
	error_free_resources);

    memcpy(
	duration_type,
	&(duration_type_template),
	sizeof(struct duration_type_t));

    DL_APPEND(elementary_types, &(duration_type->type));
    
    /* Date type */
    ALLOC_OR_JUMP(
	date_type,
	struct date_type_t,
	error_free_resources);

    memcpy(
	date_type,
	&(date_type_template),
	sizeof(struct date_type_t));

    DL_APPEND(elementary_types, &(date_type->type));
    
    /* Tod type */
    ALLOC_OR_JUMP(
	tod_type,
	struct tod_type_t,
	error_free_resources);

    memcpy(
	tod_type,
	&(tod_type_template),
	sizeof(struct tod_type_t));

    DL_APPEND(elementary_types, &(tod_type->type));

    /* Date and tod type */
    ALLOC_OR_JUMP(
	date_tod_type,
	struct date_tod_type_t,
	error_free_resources);

    memcpy(
	date_tod_type,
	&(date_tod_type_template),
	sizeof(struct date_tod_type_t));

    DL_APPEND(elementary_types, &(date_tod_type->type));

    return elementary_types;
    
error_free_resources:
    free(integer_types);
    free(real_types);
    free(string_types);
    free(duration_type);
    free(date_type);
    free(tod_type);
    free(date_tod_type);
    return NULL;
}

void st_destroy_types_in_list(
    struct type_iface_t *types)
{
    /* TODO: destroy types in list */
}

/**************************************************************************/
/* Integer types                                                          */
/**************************************************************************/
struct value_iface_t * st_integer_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv = NULL;
    ALLOC_OR_JUMP(
	iv,
	struct integer_value_t,
	error_free_resources);
    
    iv->explicit_type = self;
    memset(&(iv->value), 0, sizeof(struct value_iface_t));
    
    iv->value.display = st_integer_value_display;
    iv->value.assign = st_integer_value_assign;
    iv->value.reset = st_integer_value_reset;
    iv->value.explicit_type = st_integer_value_explicit_type;
    iv->value.compatible = st_integer_value_compatible;
    iv->value.create_temp_from = st_integer_value_create_temp_from;
    iv->value.destroy = st_integer_value_destroy;

    iv->value.greater = st_integer_value_greater;
    iv->value.lesser = st_integer_value_lesser;
    iv->value.equals = st_integer_value_equals;

    iv->value.plus = st_integer_value_plus;
    iv->value.minus = st_integer_value_minus;
    iv->value.multiply = st_integer_value_multiply;
    iv->value.divide = st_integer_value_divide;
    iv->value.modulus = st_integer_value_modulus;
    iv->value.to_power = st_integer_value_to_power;
    
    iv->value.integer = st_integer_value_integer;

    return &(iv->value);
    
error_free_resources:
    return NULL;
}

int st_integer_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    struct integer_type_t *it =
	CONTAINER_OF(self, struct integer_type_t, type);

    struct integer_value_t *iv
	= CONTAINER_OF(value_of, struct integer_value_t, value);

    iv->num = it->default_value;
    
    return ESSTEE_OK;
}

int st_integer_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    struct integer_type_t *it =
	CONTAINER_OF(self, struct integer_type_t, type);

    if(!value->integer)
    {
	return ESSTEE_FALSE;
    }

    int64_t intval = value->integer(value, config);

    if(intval > it->max)
    {
	return ESSTEE_RT_TYPE_OVERFLOW;
    }
    
    if(intval < it->min)
    {
	return ESSTEE_RT_TYPE_UNDERFLOW;
    }

    return ESSTEE_TRUE;
}

void st_integer_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: integer type destructor */
}

/**************************************************************************/
/* Real types                                                             */
/**************************************************************************/
struct value_iface_t * st_real_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    /* TODO: create real type value */
    return NULL;
}

int st_real_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    /* TODO: real type reset value of */
    return ESSTEE_FALSE;
}

int st_real_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    /* TODO: real type can hold evaluation */
    return ESSTEE_FALSE;
}

void st_real_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: real type destructor */
}

/**************************************************************************/
/* String types                                                           */
/**************************************************************************/
struct value_iface_t * st_string_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    /* TODO: string type create value of */
    return NULL;
}

int st_string_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    /* TODO: string type reset value of */
    return ESSTEE_FALSE;
}

int st_string_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    /* TODO: string type can hold */
    return ESSTEE_FALSE;
}

void st_string_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: string type destructor */
}

/**************************************************************************/
/* Duration type                                                          */
/**************************************************************************/
struct value_iface_t * st_duration_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    /* TODO: duration type create value of */
    return NULL;
}

int st_duration_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    /* TODO: duration type reset value of */
    return ESSTEE_FALSE;
}

int st_duration_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    /* TODO: duration type can hold */
    return ESSTEE_FALSE;
}

void st_duration_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: duration type destructor */
}

/**************************************************************************/
/* Date type                                                              */
/**************************************************************************/
struct value_iface_t * st_date_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    /* TODO: date type create value of */
    return NULL;
}

int st_date_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    /* TODO: date type reset value of */
    return ESSTEE_FALSE;
}

int st_date_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    /* TODO: date type can hold */
    return ESSTEE_FALSE;
}

void st_date_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: date type destructor */
}

/**************************************************************************/
/* Tod type                                                               */
/**************************************************************************/
struct value_iface_t * st_tod_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    /* TODO: tod type create value of */
    return NULL;
}

int st_tod_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    /* TODO: tod type reset value of */
    return ESSTEE_FALSE;
}

int st_tod_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    /* TODO: tod type can hold */
    return ESSTEE_FALSE;
}

void st_tod_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: tod type destructor */
}

/**************************************************************************/
/* Date tod type                                                          */
/**************************************************************************/
struct value_iface_t * st_date_tod_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    /* TODO: date tod type create value */
    return NULL;
}

int st_date_tod_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    /* TODO: date tod type reset value of */
    return ESSTEE_FALSE;
}

int st_date_tod_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    /* TODO: date tod type can hold */
    return ESSTEE_FALSE;
}

void st_date_tod_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: date tod type destructor */
}

/**************************************************************************/
/* Derived type                                                           */
/**************************************************************************/
const struct st_location_t * st_derived_type_location(
    const struct type_iface_t *self)
{
    /* TODO: derived type location */
    return NULL;
}

struct value_iface_t * st_derived_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    /* TODO: derived type create value of */
    return NULL;
}

int st_derived_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    /* TODO: derived type reset value of */
    return ESSTEE_FALSE;
}

int st_derived_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    /* TODO: derived type can hold */
    return ESSTEE_FALSE;
}

void st_derived_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: derived type destructor */
}

/**************************************************************************/
/* Enumerated type                                                        */
/**************************************************************************/
const struct st_location_t * st_enum_type_location(
    const struct type_iface_t *self)
{
    /* TODO: enum type location */
    return NULL;
}

struct value_iface_t * st_enum_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    /* TODO: enum type create value of */
    return NULL;
}

int st_enum_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    /* TODO: enum type reset value of */
    return ESSTEE_FALSE;
}

int st_enum_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    /* TODO: enum type can hold */
    return ESSTEE_FALSE;
}

int st_enum_type_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config)
{
    /* TODO: enum type compatible, true when; enum of same values, or
     * derived type of such type */
    return ESSTEE_FALSE;
}

void st_enum_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: enum type destructor */
}

/**************************************************************************/
/* Subrange type                                                          */
/**************************************************************************/
const struct st_location_t * st_subrange_type_location(
    const struct type_iface_t *self)
{
    /* TODO: subrange type location */
    return NULL;
}

struct value_iface_t * st_subrange_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    /* TODO: subrange type create value of */
    return NULL;
}

int st_subrange_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    /* TODO: subrange type reset value of */
    return ESSTEE_FALSE;
}

int st_subrange_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    /* TODO: subrange type can hold */
    return ESSTEE_FALSE;
}

int st_subrange_type_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config)
{
    /* TODO: subrange type compatible */
    return ESSTEE_FALSE;
}

void st_subrange_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: subrange type destructor */
}

/**************************************************************************/
/* Array type                                                             */
/**************************************************************************/
const struct st_location_t * st_array_type_location(
    const struct type_iface_t *self)
{
    /* TODO: array type location */
    return NULL;
}

struct value_iface_t * st_array_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    /* TODO: array type create value of */
    return NULL;
}

int st_array_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    /* TODO: array type reset value of */
    return ESSTEE_FALSE;
}

void st_array_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: array type destructor */
}

/**************************************************************************/
/* Structure type                                                         */
/**************************************************************************/
const struct st_location_t * st_struct_type_location(
    const struct type_iface_t *self)
{
    /* TODO: struct type location */
    return NULL;
}

struct value_iface_t * st_struct_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    /* TODO: struct type create value of */
    return NULL;
}

int st_struct_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    /* TODO: struct type reset value of */
    return ESSTEE_FALSE;
}

void st_struct_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: struct type destructor */
}

/**************************************************************************/
/* Function block type                                                    */
/**************************************************************************/
const struct st_location_t * st_function_block_type_location(
    const struct type_iface_t *self)
{
    /* TODO: function block type location */
    return NULL;
}

struct value_iface_t * st_function_block_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    /* TODO: function block type create value of */
    return NULL;
}

int st_function_block_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    /* TODO: function block type reset value of */
    return ESSTEE_FALSE;
}

void st_function_block_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: function block type destructor */
}

/**************************************************************************/
/* String type with defined length                                        */
/**************************************************************************/
/* TODO: functions for string type with defined length */
