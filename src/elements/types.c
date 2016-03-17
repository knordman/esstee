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
#include <elements/pous.h>
#include <elements/values.h>
#include <elements/variables.h>
#include <util/macros.h>
#include <util/bitflag.h>
#include <linker/linker.h>

#include <utlist.h>


/**************************************************************************/
/* Elementary types                                                       */
/**************************************************************************/
static struct integer_type_t integer_type_templates[] = {
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_bool_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_bool_type_can_hold,
	    .class = st_integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .identifier = "BOOL",
	},
	.size = 1,
	.default_value = 0,
	.class = INTEGER_BOOL_TYPE,
	.min = 0,
	.max = 1
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .class = st_integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .identifier = "SINT",
	},
	.size = 1,
	.default_value = 0,
	.class = INTEGER_SINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_SIGNED,
	.min = -127,
	.max = 127
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .class = st_integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .identifier = "INT",
	},
	.size = 2,
	.default_value = 0,
	.class = INTEGER_INT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_SIGNED,
	.min = -32767,
	.max = 32767,
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .class = st_integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .identifier = "DINT",
	},
	.size = 4,
	.default_value = 0,
	.class = INTEGER_DINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_SIGNED,
	.min = -2147483647,
	.max = 2147483647
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .class = st_integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .identifier = "LINT",
	},
	.size = 8,
	.default_value = 0,
	.class = INTEGER_LINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_SIGNED,
	.min = -9223372036854775806,
	.max = 9223372036854775806
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .class = st_integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .identifier = "USINT",
	},
	.size = 1,
	.default_value = 0,
	.class = INTEGER_USINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_UNSIGNED,
	.min = 0,
	.max = 255
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .class = st_integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .identifier = "UINT",
	},
	.size = 2,
	.default_value = 0,
	.class = INTEGER_UINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_UNSIGNED,
	.min = 0,
	.max = 65535 
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .class = st_integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .identifier = "UDINT",
	},
	.size = 4,
	.default_value = 0,
	.class = INTEGER_UDINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_UNSIGNED,
	.min = 0,
	.max = 4294967295
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .class = st_integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .identifier = "ULINT",
	},
	.size = 8,
	.default_value = 0,
	.class = INTEGER_ULINT_TYPE|INTEGER_NUMERIC_CLASS|INTEGER_UNSIGNED,
	.min = 0,
	.max = 9223372036854775806 /* Full range not supported */
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .class = st_integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .identifier = "BYTE",
	},
	.size = 1,
	.default_value = 0x00,
	.class = INTEGER_BYTE_TYPE|INTEGER_BITDATA_CLASS,
	.min = 0x00,
	.max = 0xff,
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .class = st_integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .identifier = "WORD",
	},
	.size = 2,
	.default_value = 0x0000,
	.class = INTEGER_WORD_TYPE|INTEGER_BITDATA_CLASS,
	.min = 0x0000,
	.max = 0xffff,
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .class = st_integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .identifier = "DWORD",
	},
	.size = 4,
	.default_value = 0x00000000,
	.class = INTEGER_DWORD_TYPE|INTEGER_BITDATA_CLASS,
	.min = 0x00000000,
	.max = 0xffffffff,
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_integer_type_create_value_of,
	    .reset_value_of = st_integer_type_reset_value_of,
	    .can_hold = st_integer_type_can_hold,
	    .class = st_integer_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_integer_type_destroy,
	    .identifier = "LWORD",
	},
	.size = 8,
	.default_value = 0x0000000000000000,
	.class = INTEGER_LWORD_TYPE|INTEGER_BITDATA_CLASS,
	.min = 0x0000000000000000,
	.max = 0xffffffffffffffff,
    },
};

static struct real_type_t real_type_templates[] = {
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_real_type_create_value_of,
	    .reset_value_of = st_real_type_reset_value_of,
	    .can_hold = st_real_type_can_hold,
	    .class = st_real_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_real_type_destroy,
	    .identifier = "REAL",
	},
	.size = 4,
	.class = REAL_TYPE,
	.default_value = 0.0,
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_real_type_create_value_of,
	    .reset_value_of = st_real_type_reset_value_of,
	    .can_hold = st_real_type_can_hold,
	    .class = st_real_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_real_type_destroy,
	    .identifier = "LREAL",
	},
	.size = 8,
	.class = LREAL_TYPE,
	.default_value = 0.0,
    },
};

static struct string_type_t string_type_templates[] = {
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_string_type_create_value_of,
	    .reset_value_of = st_string_type_reset_value_of,
	    .can_hold = st_string_type_can_hold,
	    .class = st_string_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_string_type_destroy,
	    .identifier = "STRING",
	},
	.class = STRING_TYPE,
	.default_value = "''",
    },
    {	.type = {
	    .location = st_built_in_type_location_get,
	    .create_value_of = st_string_type_create_value_of,
	    .reset_value_of = st_string_type_reset_value_of,
	    .can_hold = st_string_type_can_hold,
	    .class = st_string_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = st_string_type_destroy,
	    .identifier = "WSTRING",
	},
	.class = WSTRING_TYPE,
	.default_value = "\"\"",
    },
};

static struct duration_type_t duration_type_template = {
    .type = {
	.location = st_built_in_type_location_get,
	.create_value_of = st_duration_type_create_value_of,
	.reset_value_of = st_duration_type_reset_value_of,
	.can_hold = st_duration_type_can_hold,
	.compatible = st_type_general_compatible,
	.destroy = st_duration_type_destroy,
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
    st_bitflag_t self_class = self->class(self, config);
    st_bitflag_t other_type_class = other_type->class(other_type, config);
    
    if(self_class == other_type_class)
    {
	return ESSTEE_TRUE;
    }

    /* Clear derived and subrange flags from both types */
    st_bitflag_t self_class_cmp = self_class & (~(DERIVED_TYPE|SUBRANGE_TYPE));
    st_bitflag_t other_type_class_cmp = other_type_class & (~(DERIVED_TYPE|SUBRANGE_TYPE));

    if(self_class_cmp == other_type_class_cmp)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_TYPE_INCOMPATIBILITY;
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
    
    iv->type = self;
    memset(&(iv->value), 0, sizeof(struct value_iface_t));
    
    iv->value.display = st_integer_value_display;
    iv->value.assign = st_integer_value_assign;
    iv->value.type_of = st_integer_value_type_of;
    iv->value.assignable_from = st_integer_value_assignable_from;
    iv->value.comparable_to = st_integer_value_compares_and_operates;
    iv->value.operates_with = st_integer_value_compares_and_operates;
    iv->value.create_temp_from = st_integer_value_create_temp_from;
    iv->value.destroy = st_integer_value_destroy;
    iv->value.class = st_integer_value_class;

    iv->value.greater = st_integer_value_greater;
    iv->value.lesser = st_integer_value_lesser;
    iv->value.equals = st_integer_value_equals;

    iv->value.negate = st_integer_value_negate;
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
	return ESSTEE_TYPE_OVERFLOW;
    }
    
    if(intval < it->min)
    {
	return ESSTEE_TYPE_UNDERFLOW;
    }

    return ESSTEE_TRUE;
}

st_bitflag_t st_integer_type_class(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    struct integer_type_t *it =
	CONTAINER_OF(self, struct integer_type_t, type);

    return it->class;
}


void st_integer_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: integer type destructor */
}

/* Bool specializations */
struct value_iface_t * st_bool_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    struct integer_value_t *iv = NULL;
    ALLOC_OR_JUMP(
	iv,
	struct integer_value_t,
	error_free_resources);
    
    iv->type = self;
    memset(&(iv->value), 0, sizeof(struct value_iface_t));
    
    iv->value.display = st_bool_value_display;
    iv->value.assign = st_bool_value_assign;
    iv->value.type_of = st_integer_value_type_of;
    iv->value.assignable_from = st_bool_value_assigns_compares_operates;
    iv->value.comparable_to = st_bool_value_assigns_compares_operates;
    iv->value.operates_with = st_bool_value_assigns_compares_operates; 
    iv->value.create_temp_from = st_bool_value_create_temp_from;
    iv->value.destroy = st_integer_value_destroy;
    iv->value.class = st_integer_value_class;

    iv->value.equals = st_bool_value_equals;

    iv->value.not = st_bool_value_not;
    iv->value.xor = st_bool_value_xor;
    iv->value.and = st_bool_value_and;
    iv->value.or = st_bool_value_or;
    
    iv->value.bool = st_bool_value_bool;

    return &(iv->value);
    
error_free_resources:
    return NULL;
}

struct value_iface_t * st_bool_type_create_temp_value(
    const struct config_iface_t *config)
{
    struct value_iface_t *value =
	st_bool_type_create_value_of(NULL, config);

    if(!value)
    {
	return NULL;
    }

    struct integer_value_t *iv =
	CONTAINER_OF(value, struct integer_value_t, value);
    
    ST_SET_FLAGS(iv->class, TEMPORARY_VALUE);

    return value;
}

int st_bool_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    if(!value->bool)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

int st_bool_type_true(
    struct value_iface_t *value)
{
    struct integer_value_t *iv =
	CONTAINER_OF(value, struct integer_value_t, value);

    iv->num = 1;

    return ESSTEE_OK;
}

int st_bool_type_false(
    struct value_iface_t *value)
{
    struct integer_value_t *iv =
	CONTAINER_OF(value, struct integer_value_t, value);

    iv->num = 0;

    return ESSTEE_OK;
}

/**************************************************************************/
/* Real types                                                             */
/**************************************************************************/
struct value_iface_t * st_real_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    struct real_value_t *rv = NULL;
    ALLOC_OR_JUMP(
	rv,
	struct real_value_t,
	error_free_resources);

    rv->type = self;
    memset(&(rv->value), 0, sizeof(struct value_iface_t));

    rv->value.display = st_real_value_display;
    rv->value.assign = st_real_value_assign;
    rv->value.type_of = st_real_value_type_of;
    rv->value.assignable_from = st_real_value_assignable_from;
    rv->value.comparable_to = st_real_value_compares_and_operates;
    rv->value.operates_with = st_real_value_compares_and_operates;
    rv->value.create_temp_from = st_real_value_create_temp_from;
    rv->value.destroy = st_real_value_destroy;
    rv->value.class = st_real_value_class;

    rv->value.greater = st_real_value_greater;
    rv->value.lesser = st_real_value_lesser;
    rv->value.equals = st_real_value_equals;

    rv->value.negate = st_real_value_negate;
    rv->value.plus = st_real_value_plus;
    rv->value.minus = st_real_value_minus;
    rv->value.multiply = st_real_value_multiply;
    rv->value.divide = st_real_value_divide;
    rv->value.to_power = st_real_value_to_power;
    
    rv->value.real = st_real_value_real;

    return &(rv->value);
    
error_free_resources:
    return NULL;
}

int st_real_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    struct real_type_t *rt =
	CONTAINER_OF(self, struct real_type_t, type);

    struct real_value_t *rv
	= CONTAINER_OF(value_of, struct real_value_t, value);

    rv->num = rt->default_value;
    
    return ESSTEE_OK;
}

int st_real_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    if(!value->real)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

st_bitflag_t st_real_type_class(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    struct real_type_t *rt =
	CONTAINER_OF(self, struct real_type_t, type);

    return rt->class;
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
    struct string_value_t *sv = NULL;
    ALLOC_OR_JUMP(
	sv,
	struct string_value_t,
	error_free_resources);

    sv->type = self;

    memset(&(sv->value), 0, sizeof(struct value_iface_t));

    sv->value.display = st_string_value_display;
    sv->value.assign = st_string_value_assign;
    sv->value.assignable_from = st_string_value_assigns_and_compares;
    sv->value.comparable_to = st_string_value_assigns_and_compares;
    sv->value.class = st_general_value_empty_class;
    sv->value.type_of = st_string_value_type_of;
    sv->value.destroy = st_string_value_destroy;

    sv->value.equals = st_string_value_equals;
    sv->value.string = st_string_value_string;
	
    return &(sv->value);
    
error_free_resources:
    return NULL;
}

int st_string_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    const struct string_type_t *st =
	CONTAINER_OF(self, struct string_type_t, type);

    struct string_value_t *sv =
	CONTAINER_OF(value_of, struct string_value_t, value);

    sv->str = st->default_value;

    return ESSTEE_OK;
}

int st_string_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    if(!value->string)
    {
	return ESSTEE_FALSE;
    }

    if(!value->type_of)
    {
	return ESSTEE_FALSE;
    }
    
    const struct type_iface_t *value_type = value->type_of(value);

    return self->compatible(self, value_type, config);
}

st_bitflag_t st_string_type_class(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    const struct string_type_t *st =
	CONTAINER_OF(self, struct string_type_t, type);

    return st->class;
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
    struct duration_value_t *dt = NULL;
    ALLOC_OR_JUMP(
	dt,
	struct duration_value_t,
	error_free_resources);

    dt->type = self;
    
    memset(&(dt->value), 0, sizeof(struct value_iface_t));
    dt->value.display = st_duration_value_display;
    dt->value.assignable_from = st_duration_value_assigns_and_compares;
    dt->value.comparable_to = st_duration_value_assigns_and_compares;
    dt->value.assign = st_duration_value_assign;
    dt->value.destroy = st_duration_value_destroy;

    dt->value.greater = st_duration_value_greater;
    dt->value.lesser = st_duration_value_lesser;
    dt->value.equals = st_general_value_equals;
    dt->value.duration = st_duration_value_duration;
    dt->value.class = st_general_value_empty_class;

    return &(dt->value);
    
error_free_resources:
    return NULL;
}

int st_duration_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    const struct duration_type_t *dt =
	CONTAINER_OF(self, struct duration_type_t, type);

    struct duration_value_t *dv =
	CONTAINER_OF(value_of, struct duration_value_t, value);

    dv->duration.d = dt->default_d;
    dv->duration.h = dt->default_h;
    dv->duration.m = dt->default_m;
    dv->duration.s = dt->default_s;
    dv->duration.ms = dt->default_ms;

    return ESSTEE_OK;
}

int st_duration_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    if(!value->duration)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

st_bitflag_t st_duration_type_class(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    return DURATION_TYPE;
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
    struct date_value_t *dv = NULL;
    ALLOC_OR_JUMP(
	dv,
	struct date_value_t,
	error_free_resources);

    dv->type = self;
    
    memset(&(dv->value), 0, sizeof(struct value_iface_t));
    dv->value.display = st_date_value_display;
    dv->value.assignable_from = st_date_value_assigns_and_compares;
    dv->value.comparable_to = st_date_value_assigns_and_compares;
    dv->value.assign = st_date_value_assign;
    dv->value.destroy = st_date_value_destroy;

    dv->value.greater = st_date_value_greater;
    dv->value.lesser = st_date_value_lesser;
    dv->value.equals = st_general_value_equals;
    dv->value.date = st_date_value_date;
    dv->value.class = st_general_value_empty_class;

    return &(dv->value);
    
error_free_resources:
    return NULL;
}

int st_date_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    const struct date_type_t *dt =
	CONTAINER_OF(self, struct date_type_t, type);

    struct date_value_t *dv =
	CONTAINER_OF(value_of, struct date_value_t, value);

    dv->date.y = dt->default_year;
    dv->date.m = dt->default_month;
    dv->date.d = dt->default_day;

    return ESSTEE_OK;
}

int st_date_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    if(!value->date)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

st_bitflag_t st_date_type_class(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    return DATE_TYPE;
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
    struct tod_value_t *tv = NULL;
    ALLOC_OR_JUMP(
	tv,
	struct tod_value_t,
	error_free_resources);

    tv->type = self;
    
    memset(&(tv->value), 0, sizeof(struct value_iface_t));
    tv->value.display = st_tod_value_display;
    tv->value.assignable_from = st_tod_value_assigns_and_compares;
    tv->value.comparable_to = st_tod_value_assigns_and_compares;
    tv->value.assign = st_tod_value_assign;
    tv->value.destroy = st_tod_value_destroy;

    tv->value.greater = st_tod_value_greater;
    tv->value.lesser = st_tod_value_lesser;
    tv->value.equals = st_general_value_equals;
    tv->value.tod = st_tod_value_tod;
    tv->value.class = st_general_value_empty_class;

    return &(tv->value);
    
error_free_resources:
    return NULL;
}

int st_tod_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    const struct tod_type_t *tt =
	CONTAINER_OF(self, struct tod_type_t, type);

    struct tod_value_t *tv =
	CONTAINER_OF(value_of, struct tod_value_t, value);

    tv->tod.h = tt->default_hour;
    tv->tod.m = tt->default_minute;
    tv->tod.s = tt->default_second;
    tv->tod.fs = tt->default_fractional_second;

    return ESSTEE_OK;
}

int st_tod_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    if(!value->tod)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

st_bitflag_t st_tod_type_class(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    return TOD_TYPE;
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
    struct date_tod_value_t *dv = NULL;
    ALLOC_OR_JUMP(
	dv,
	struct date_tod_value_t,
	error_free_resources);

    dv->type = self;
    
    memset(&(dv->value), 0, sizeof(struct value_iface_t));
    dv->value.display = st_date_tod_value_display;
    dv->value.assignable_from = st_date_tod_value_assigns_and_compares;
    dv->value.comparable_to = st_date_tod_value_assigns_and_compares;
    dv->value.assign = st_date_tod_value_assign;
    dv->value.destroy = st_date_tod_value_destroy;

    dv->value.greater = st_date_tod_value_greater;
    dv->value.lesser = st_date_tod_value_lesser;
    dv->value.equals = st_general_value_equals;
    dv->value.date_tod = st_date_tod_value_tod;
    dv->value.class = st_general_value_empty_class;

    return &(dv->value);
    
error_free_resources:
    return NULL;
}

int st_date_tod_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    const struct date_tod_type_t *dt =
	CONTAINER_OF(self, struct date_tod_type_t, type);

    struct date_tod_value_t *dv =
	CONTAINER_OF(value_of, struct date_tod_value_t, value);

    dv->dt.date.y = dt->default_year;
    dv->dt.date.m = dt->default_month;
    dv->dt.date.d = dt->default_day;
    dv->dt.tod.h = dt->default_hour;
    dv->dt.tod.m = dt->default_minute;
    dv->dt.tod.s = dt->default_second;
    dv->dt.tod.fs = dt->default_fractional_second;
    
    return ESSTEE_OK;
}

int st_date_tod_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    if(!value->date_tod)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

st_bitflag_t st_date_tod_type_class(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    return DATE_TOD_TYPE;
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
    struct derived_type_t *dt =
	CONTAINER_OF(self, struct derived_type_t, type);

    return dt->location;
}

struct value_iface_t * st_derived_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    struct derived_type_t *dt =
	CONTAINER_OF(self, struct derived_type_t, type);

    struct value_iface_t *new_value =
	dt->ancestor->create_value_of(dt->ancestor, config);

    if(!new_value)
    {
	return NULL;
    }

    return new_value;
}

int st_derived_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    struct derived_type_t *dt =
	CONTAINER_OF(self, struct derived_type_t, type);

    int reset_result = dt->ancestor->reset_value_of(dt->ancestor,
						    value_of,
						    config);
    if(reset_result != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    if(dt->default_value)
    {
	reset_result = value_of->assign(value_of,
					dt->default_value,
					config);
    }

    if(reset_result != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }
    
    return ESSTEE_OK;
}

int st_derived_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    struct derived_type_t *dt =
	CONTAINER_OF(self, struct derived_type_t, type);

    return dt->ancestor->can_hold(dt->ancestor, value, config);
}

int st_derived_type_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config)
{
    struct derived_type_t *dt =
	CONTAINER_OF(self, struct derived_type_t, type);

    return dt->ancestor->compatible(dt->ancestor, other_type, config);
}

st_bitflag_t st_derived_type_class(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    return DERIVED_TYPE;
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
    struct enum_type_t *et =
	CONTAINER_OF(self, struct enum_type_t, type);

    return et->location;
}

struct value_iface_t * st_enum_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    struct enum_value_t *ev = NULL;
    ALLOC_OR_JUMP(
	ev,
	struct enum_value_t,
	error_free_resources);
    
    struct enum_type_t *et =
	CONTAINER_OF(self, struct enum_type_t, type);

    ev->type = self;
    ev->constant = et->default_item;

    memset(&(ev->value), 0, sizeof(struct value_iface_t));

    ev->value.display = st_enum_value_display;
    ev->value.assign = st_enum_value_assign;
    ev->value.type_of = st_enum_value_type_of;
    ev->value.assignable_from = st_enum_value_assigns_and_compares;
    ev->value.comparable_to = st_enum_value_assigns_and_compares;
    ev->value.destroy = st_enum_value_destroy;
    ev->value.equals = st_enum_value_equals;
    ev->value.enumeration = st_enum_value_enumeration;
    ev->value.class = st_general_value_empty_class;

    return &(ev->value);
    
error_free_resources:
    return NULL;
}

int st_enum_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    struct enum_type_t *et =
	CONTAINER_OF(self, struct enum_type_t, type);

    struct enum_value_t *ev =
	CONTAINER_OF(value_of, struct enum_value_t, value);

    ev->constant = et->default_item;

    return ESSTEE_OK;
}

int st_enum_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    if(!value->enumeration)
    {
	return ESSTEE_FALSE;
    }

    struct enum_type_t *et = CONTAINER_OF(self, struct enum_type_t, type);

    const char *value_constant = value->enumeration(value, config)->identifier;
    
    struct enum_item_t *found = NULL;
    HASH_FIND_STR(et->values, value_constant, found);
    if(!found)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

st_bitflag_t st_enum_type_class(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    return ENUM_TYPE;
}

void st_enum_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: enum type destructor */
}

/**************************************************************************/
/* Subrange type                                                          */
/**************************************************************************/
struct value_iface_t * st_subrange_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    struct subrange_value_t *sv = NULL;
    ALLOC_OR_JUMP(
	sv,
	struct subrange_value_t,
	error_free_resources);

    struct subrange_type_t *st =
	CONTAINER_OF(self, struct subrange_type_t, type);

    sv->type = self;

    sv->current = st_integer_type_create_value_of(NULL, config);
    if(!sv->current)
    {
	goto error_free_resources;
    }

    int assign_result = ESSTEE_ERROR;
    if(st->default_value)
    {
	assign_result = sv->current->assign(sv->current,
					    st->default_value,
					    config);
    }
    else
    {
	assign_result = sv->current->assign(sv->current,
					    st->subrange->min,
					    config);
    }
    
    if(assign_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    memset(&(sv->value), 0, sizeof(struct value_iface_t));
    
    sv->value.display = st_subrange_value_display;
    sv->value.assign = st_subrange_value_assign;
    sv->value.type_of = st_subrange_value_type_of;
    sv->value.assignable_from = st_subrange_value_assignable_from;
    sv->value.comparable_to = st_subrange_value_compares_and_operates;
    sv->value.operates_with = st_subrange_value_compares_and_operates;
    sv->value.create_temp_from = st_subrange_value_create_temp_from;
    sv->value.destroy = st_subrange_value_destroy;
    sv->value.integer = st_subrange_value_integer;
    sv->value.class = st_general_value_empty_class;

    return &(sv->value);
    
error_free_resources:
    free(sv);
    /* TODO: destroy subranged value */
    return NULL;
}

int st_subrange_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
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
					    config);
    }
    else
    {
	assign_result = sv->current->assign(sv->current,
					    st->subrange->min,
					    config);
    }

    return assign_result;
}

int st_subrange_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    struct subrange_type_t *st =
	CONTAINER_OF(self, struct subrange_type_t, type);

    int subranged_type_can_hold =
	st->subranged_type->can_hold(st->subranged_type, value, config);

    if(subranged_type_can_hold != ESSTEE_OK)
    {
	return subranged_type_can_hold;
    }

    if(!value->lesser || !value->greater)
    {
	return ESSTEE_FALSE;
    }
    
    int min_check_result = ESSTEE_ERROR;
    min_check_result = st->subrange->min->greater(st->subrange->min,
						  value,
						  config);
    if(min_check_result != ESSTEE_FALSE)
    {
	return ESSTEE_FALSE;
    }

    int max_check_result = ESSTEE_ERROR;
    max_check_result = st->subrange->max->lesser(st->subrange->max,
						 value,
						 config);
    if(max_check_result != ESSTEE_FALSE)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

st_bitflag_t st_subrange_type_class(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    struct subrange_type_t *st =
	CONTAINER_OF(self, struct subrange_type_t, type);

    int subranged_type_class = st->subranged_type->class(st->subranged_type,
							 config);

    return (subranged_type_class | SUBRANGE_TYPE);
}

int st_subrange_type_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config)
{
    struct subrange_type_t *st =
	CONTAINER_OF(self, struct subrange_type_t, type);

    return st->subranged_type->compatible(st->subranged_type,
					  other_type,
					  config);
}

void st_subrange_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: subrange type destructor */
}

/**************************************************************************/
/* Array type                                                             */
/**************************************************************************/
int st_array_type_check_array_initializer(
    struct array_range_t *ranges,
    const struct value_iface_t *default_value,
    struct type_iface_t *arrayed_type,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    const struct array_init_value_t *initializer =
	default_value->array_init_value(default_value, config);
    size_t checked_entries = 0;
    struct array_range_t *current_range = ranges;
    struct listed_value_t *itr = NULL;
	
    for(itr = initializer->values; itr != NULL; itr = itr->next)
    {
	if(itr->value->array_init_value)
	{
	    if(!current_range->next)
	    {
		if(errors)
		{
		    errors->new_issue_at(
			errors,
			"value addresses a new array level, but that does not match the array specification.", 
			ISSUE_ERROR_CLASS,
			1,
			itr->location);
		}
		
		return ESSTEE_ERROR;
	    }
	    else
	    {
		if(st_array_type_check_array_initializer(
		       current_range->next,
		       itr->value,
		       arrayed_type,
		       errors,
		       config) == ESSTEE_ERROR)
		{
		    return ESSTEE_ERROR;
		}
	    }
	}
	else
	{
	    if(!current_range->next)
	    {
		if(arrayed_type->can_hold(arrayed_type, itr->value, config) != ESSTEE_OK)
		{
		    if(errors)
		    {
			errors->new_issue_at(
			    errors,
			    "incompatible initialization value.",
			    ISSUE_ERROR_CLASS,
			    1,
			    itr->location);
		    }

		    return ESSTEE_ERROR;
		}
	    }
	    else
	    {
		if(errors)
		{
		    errors->new_issue_at(
			errors,
			"according to the array specification the value should address a new level.", 
			ISSUE_ERROR_CLASS,
			1,
			itr->location);
		}
		
		return ESSTEE_ERROR;
	    }
	}

	if(itr->multiplier)
	{
	    if(!itr->multiplier->integer)
	    {
		if(errors)
		{
		    errors->new_issue_at(
			errors,
			"bad multiplier in array initializer.",
			ISSUE_ERROR_CLASS,
			1,
			itr->location);
		}
		
		return ESSTEE_ERROR;
	    }

	    int64_t multiplier = itr->multiplier->integer(itr->multiplier, config);
	    checked_entries += multiplier;
	}
	else
	{
	    checked_entries++;
	}
    }

    if(checked_entries < current_range->entries)
    {
	if(errors)
	{
	    errors->new_issue_at(
		errors,
		"too few values in array initializer.",
		ISSUE_ERROR_CLASS,
		1,
		initializer->location);
	}
	
	return ESSTEE_ERROR;
    }
    else if(checked_entries > current_range->entries)
    {
	if(errors)
	{
	    errors->new_issue_at(
		errors,
		"too many values in array initializer.", 
		ISSUE_ERROR_CLASS,
		1,
		initializer->location);
	}

	return ESSTEE_ERROR;
    }
	
    return ESSTEE_OK;
}

int st_array_type_assign_default_value(
    struct value_iface_t **elements,
    const struct value_iface_t *default_value,
    const struct config_iface_t *config)
{
    const struct array_init_value_t *iav =
	default_value->array_init_value(default_value, config);

    int elements_assigned = 0;
    
    struct listed_value_t *itr = NULL;
    DL_FOREACH(iav->values, itr)
    {
	if(itr->value->array_init_value)
	{
	    int sub_array_elements_assigned =
		st_array_type_assign_default_value(elements, itr->value, config);

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
		elements_to_assign = itr->multiplier->integer(itr->multiplier, config);
	    }

	    for(size_t i = 0; i < elements_to_assign; i++)
	    {
		int assign_result =
		    (*elements)->assign((*elements), itr->value, config);

		if(assign_result != ESSTEE_OK)
		{
		    return ESSTEE_ERROR;
		}

		elements++;
	    }

	    elements_assigned += elements_to_assign;
	}
    }

    return elements_assigned;
}

struct value_iface_t * st_array_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    const struct array_type_t *at =
	CONTAINER_OF(self, struct array_type_t, type);

    struct array_value_t *av = NULL;
    struct value_iface_t **elements = NULL;
    
    ALLOC_OR_JUMP(
	av,
	struct array_value_t,
	error_free_resources);

    ALLOC_ARRAY_OR_JUMP(
	elements,
	struct value_iface_t *,
	at->total_elements,
	error_free_resources);

    memset(elements, 0, sizeof(struct value_iface_t *)*at->total_elements);

    for(size_t i = 0; i < at->total_elements; i++)
    {
	elements[i] = 
	    at->arrayed_type->create_value_of(at->arrayed_type, config);

	if(!elements[i])
	{
	    goto error_free_resources;
	}
    }

    if(at->default_value)
    {
	int elements_assigned =
	    st_array_type_assign_default_value(elements, at->default_value, config);

	if(elements_assigned <= 0)
	{
	    goto error_free_resources;
	}
    }
	
    av->type = self;
    av->total_elements = at->total_elements;
    av->ranges = at->ranges;
    av->arrayed_type = at->arrayed_type;
    av->elements = elements;

    memset(&(av->value), 0, sizeof(struct value_iface_t));
    
    av->value.display = st_array_value_display;
    av->value.assign = st_array_value_assign;
    av->value.assignable_from = st_array_value_assignable_from;
    av->value.type_of = st_array_value_type_of;
    av->value.index = st_array_value_index;
    av->value.destroy = st_array_value_destroy;
    av->value.class = st_general_value_empty_class;

    return &(av->value);

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

int st_array_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    const struct array_type_t *at =
	CONTAINER_OF(self, struct array_type_t, type);

    struct array_value_t *av =
	CONTAINER_OF(value_of, struct array_value_t, value);

    if(at->default_value)
    {
	int elements_assigned =
	    st_array_type_assign_default_value(av->elements, at->default_value, config);

	if(elements_assigned <= 0)
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
						 config);
	
	    if(element_reset_result != ESSTEE_OK)
	    {
		return element_reset_result;
	    }
	}
    }

    return ESSTEE_OK;
}

int st_array_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    const struct array_type_t *at =
	CONTAINER_OF(self, struct array_type_t, type);

    return st_array_type_check_array_initializer(at->ranges,
						 value,
						 at->arrayed_type,
						 NULL,
						 config);
}

st_bitflag_t st_array_type_class(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    return ARRAY_TYPE;
}

void st_array_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: array type destructor */
}

/**************************************************************************/
/* Structure type                                                         */
/**************************************************************************/
struct value_iface_t * st_struct_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    const struct struct_type_t *st =
	CONTAINER_OF(self, struct struct_type_t, type);

    struct struct_value_t *sv = NULL;
    ALLOC_OR_JUMP(
	sv,
	struct struct_value_t,
	error_free_resources);

    sv->elements = NULL;
    struct struct_element_t *itr = NULL;
    for(itr = st->elements; itr != NULL; itr = itr->hh.next)
    {
	struct variable_t *ev = NULL;
	ALLOC_OR_JUMP(
	    ev,
	    struct variable_t,
	    error_free_resources);

	ev->identifier = itr->element_identifier;
	ev->identifier_location = NULL;

	ev->next = NULL;
	ev->prev = NULL;
	ev->address = NULL;
	
	ev->value = itr->element_type->create_value_of(itr->element_type,
						       config);
	if(!ev->value)
	{
	    goto error_free_resources;
	}

	ev->type = itr->element_type;

	HASH_ADD_KEYPTR(hh, 
			sv->elements, 
			ev->identifier, 
			strlen(ev->identifier), 
			ev);
    }

    sv->type = self;
    
    memset(&(sv->value), 0, sizeof(struct value_iface_t));
    sv->value.display = st_struct_value_display;
    sv->value.assign = st_struct_value_assign;
    sv->value.assignable_from = st_struct_value_assignable_from;
    sv->value.destroy = st_struct_value_destroy;
    sv->value.sub_variable = st_struct_value_sub_variable;
    sv->value.class = st_general_value_empty_class;

    return &(sv->value);
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

int st_struct_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    struct struct_value_t *sv =
	CONTAINER_OF(value_of, struct struct_value_t, value);

    struct variable_t *itr = NULL;
    for(itr = sv->elements; itr != NULL; itr = itr->hh.next)
    {
	int reset_result = itr->type->reset_value_of(itr->type, itr->value, config);

	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

int st_struct_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config)
{
    const struct struct_type_t *st =
	CONTAINER_OF(self, struct struct_type_t, type);

    if(!value->struct_init_value)
    {
	return ESSTEE_FALSE;
    }

    const struct struct_init_value_t *isv =
	value->struct_init_value(value, config);
    
    struct struct_element_init_t *itr = NULL;
    for(itr = isv->init_table; itr != NULL; itr = itr->hh.next)
    {
	/* Check that identifier is one of the members */
	struct struct_element_t *found = NULL;
	HASH_FIND_STR(st->elements, itr->element_identifier, found);
	if(!found)
	{
	    return ESSTEE_FALSE;
	}

	/* Check that the member type can hold the value */
	int member_type_can_hold = found->element_type->can_hold(found->element_type,
								 itr->element_default_value,
								 config);
	if(member_type_can_hold != ESSTEE_TRUE)
	{
	    return member_type_can_hold;
	}
    }
    
    return ESSTEE_TRUE;
}

st_bitflag_t st_struct_type_class(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    return STRUCT_TYPE;
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
    struct function_block_t *fb =
	CONTAINER_OF(self, struct function_block_t, type);

    return fb->location;
}

struct value_iface_t * st_function_block_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    struct function_block_t *fb =
	CONTAINER_OF(self, struct function_block_t, type);

    struct function_block_value_t *fbv = NULL;
    ALLOC_OR_JUMP(
	fbv,
	struct function_block_value_t,
	error_free_resources);
    
    struct variable_t *vitr = NULL;
    struct variable_t *variable_copies = NULL;
    for(vitr = fb->header->variables; vitr != NULL; vitr = vitr->hh.next)
    {
	struct variable_t *copy = NULL;
	ALLOC_OR_JUMP(
	    copy,
	    struct variable_t,
	    error_free_resources);

	memcpy(copy, vitr, sizeof(struct variable_t));

	copy->value = NULL;
	copy->prev = NULL;
	copy->next = NULL;
	DL_APPEND(variable_copies, copy);
    }
    
    struct variable_t *variable_copies_table =
	st_link_variables(variable_copies, NULL, NULL);

    fb->var_ref_pool->reset_resolved(fb->var_ref_pool);
    st_resolve_var_refs(fb->var_ref_pool, variable_copies_table);

    fb->var_ref_pool->trigger_resolve_callbacks(fb->var_ref_pool,
						NULL,
						config);

    /* Create values of variables */
    for(struct variable_t *vitr = variable_copies_table;
	vitr != NULL;
	vitr = vitr->hh.next)
    {
	if((vitr->value = vitr->type->create_value_of(vitr->type, config)) == NULL)
	{
	    goto error_free_resources;
	}
    }
    
    struct invoke_iface_t *statement_copies = NULL;
    struct invoke_iface_t *sitr = NULL;
    DL_FOREACH(fb->statements, sitr)
    {
	struct invoke_iface_t *copy = sitr->clone(sitr);

	if(!copy)
	{
	    goto error_free_resources;
	}

	copy->next = NULL;
	copy->prev = NULL;
	copy->call_stack_prev = NULL;
	copy->call_stack_next = NULL;
	
	DL_APPEND(statement_copies, copy);
    }

    fbv->type = self;
    fbv->variables = variable_copies_table;
    fbv->statements = statement_copies;

    memset(&(fbv->value), 0, sizeof(struct value_iface_t));

    fbv->value.display = st_function_block_value_display;
    fbv->value.type_of = st_function_block_value_type_of;
    fbv->value.destroy = st_function_block_value_destroy;
    fbv->value.sub_variable = st_function_block_value_sub_variable;
    fbv->value.invoke_verify = st_function_block_value_invoke_verify;
    fbv->value.invoke_step = st_function_block_value_invoke_step;
    fbv->value.invoke_reset = st_function_block_value_invoke_reset;

    return &(fbv->value);
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

int st_function_block_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config)
{
    struct function_block_value_t *fv =
	CONTAINER_OF(value_of, struct function_block_value_t, value);

    struct variable_t *vitr = NULL;
    DL_FOREACH(fv->variables, vitr)
    {
	int reset = vitr->type->reset_value_of(vitr->type, vitr->value, config);
	if(reset != ESSTEE_OK)
	{
	    return reset;
	}
    }

    struct invoke_iface_t *sitr = NULL;
    DL_FOREACH(fv->statements, sitr)
    {
	int reset = sitr->reset(sitr, config);
	if(reset != ESSTEE_OK)
	{
	    return reset;
	}
    }

    return ESSTEE_OK;
}

st_bitflag_t st_function_block_type_class(
    const struct type_iface_t *self,
    const struct config_iface_t *config)
{
    return FB_TYPE;
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
