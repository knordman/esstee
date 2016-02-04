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

#pragma once

#include <elements/ivalue.h>
#include <elements/types.h>

/**************************************************************************/
/* Integer values                                                         */
/**************************************************************************/
struct integer_value_t {
    struct value_iface_t value;
    const struct type_iface_t *explicit_type;
    int64_t num;
};

int st_integer_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

int st_integer_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config);

int st_integer_value_reset(
    struct value_iface_t *self,
    const struct config_iface_t *config);

const struct type_iface_t * st_integer_value_explicit_type(
    const struct value_iface_t *self);

int st_integer_value_compatible(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

struct value_iface_t * st_integer_value_create_temp_from(
    const struct value_iface_t *self);	

void st_integer_value_destroy(
    struct value_iface_t *self);

int st_integer_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_integer_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_integer_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_integer_value_plus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_integer_value_minus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_integer_value_multiply(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_integer_value_divide(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_integer_value_modulus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_integer_value_to_power(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int64_t st_integer_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config);

/* Bool specializations */
int st_bool_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

int st_bool_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config);

int st_bool_value_compatible(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

struct value_iface_t * st_bool_value_create_temp_from(
    const struct value_iface_t *self);	

int st_bool_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_bool_value_bool(
    const struct value_iface_t *self,
    const struct config_iface_t *conf);

int st_bool_value_xor(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_bool_value_and(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);
    
int st_bool_value_or(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

/**************************************************************************/
/* Real values                                                            */
/**************************************************************************/
struct real_value_t {
    struct value_iface_t value;
    struct type_iface_t *explicit_type;
    double num;
};

/* TODO: Real values, determine value interface functions */

/**************************************************************************/
/* String values                                                          */
/**************************************************************************/
struct string_value_t {
    struct value_iface_t value;
    struct type_iface_t *explicit_type;
    char *str;
};

/* TODO: String values, determine value interface function */

/**************************************************************************/
/* Duration value                                                         */
/**************************************************************************/
struct duration_value_t {
    struct value_iface_t value;
    struct type_iface_t *explicit_type;
    double d;
    double h;
    double m;
    double s;
    double ms;
};

/* TODO: Duration value, determine value interface function */

/**************************************************************************/
/* Date value                                                             */
/**************************************************************************/
struct date_value_t {
    struct value_iface_t value;
    struct type_iface_t *explicit_type;
    unsigned y;
    unsigned m;
    unsigned d;
};

/* TODO: Date value, determine value interface function */

/**************************************************************************/
/* Tod value                                                              */
/**************************************************************************/
struct tod_value_t {
    struct value_iface_t value;
    struct type_iface_t *explicit_type;
    unsigned h;
    unsigned m;
    unsigned s;
    unsigned ms;
};

/* TODO: Tod value, determine value interface function */

/**************************************************************************/
/* Date tod value                                                         */
/**************************************************************************/
struct date_tod_value_t {
    struct value_iface_t value;
    struct type_iface_t *explicit_type;
    unsigned y;
    unsigned mon;
    unsigned d;
    unsigned h;
    unsigned min;
    unsigned s;
    unsigned ms;
};

/* TODO: Date tod value, determine value interface function */

/**************************************************************************/
/* Enumerated value                                                       */
/**************************************************************************/
struct enum_value_t {
    struct value_iface_t value;
    const struct type_iface_t *explicit_type;
    const struct enum_item_t *constant;
};

int st_enum_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

int st_enum_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config);

int st_enum_value_reset(
    struct value_iface_t *self,
    const struct config_iface_t *config);

const struct type_iface_t * st_enum_value_explicit_type(
    const struct value_iface_t *self);

int st_enum_value_compatible(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

void st_enum_value_destroy(
    struct value_iface_t *self);

int st_enum_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

const struct enum_item_t * st_enum_value_enumeration(
    const struct value_iface_t *self,
    const struct config_iface_t *conf);

/**************************************************************************/
/* Subrange value                                                         */
/**************************************************************************/
struct subrange_value_t {
    struct value_iface_t value;
    const struct type_iface_t *explicit_type;
    struct value_iface_t *current;
};

int st_subrange_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

int st_subrange_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config);

int st_subrange_value_reset(
    struct value_iface_t *self,
    const struct config_iface_t *config);

const struct type_iface_t * st_subrange_value_explicit_type(
    const struct value_iface_t *self);

int st_subrange_value_compatible(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

struct value_iface_t * st_subrange_value_create_temp_from(
    const struct value_iface_t *self);	

int st_subrange_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_subrange_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_subrange_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

void st_subrange_value_destroy(
    struct value_iface_t *self);

int64_t st_subrange_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config);

/**************************************************************************/
/* Array value                                                            */
/**************************************************************************/

/* TODO: Array value */

/**************************************************************************/
/* Structure value                                                        */
/**************************************************************************/

/* TODO: Struct value */

/**************************************************************************/
/* Function block value                                                   */
/**************************************************************************/

/* TODO: Function block value */

/**************************************************************************/
/* Inline values                                                          */
/**************************************************************************/
struct subrange_case_value_t {
    struct value_iface_t value;
    struct subrange_t *subrange;
};

struct enum_inline_value_t {
    struct value_iface_t value;
    char *identifier;
};

/* Struct init value */
struct struct_init_value_t {
    struct value_iface_t value;
    struct struct_element_init_t *init_table;
};

/* Array init value */
struct array_init_value_t {
    struct value_iface_t value;
    unsigned entries;
    struct value_iface_t *values;
};
