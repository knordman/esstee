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

#define TEMPORARY_VALUE (1 << 0)

st_bitflag_t st_general_value_empty_class(
    const struct value_iface_t *self,
    const struct config_iface_t *config);

/**************************************************************************/
/* Integer values                                                         */
/**************************************************************************/
struct integer_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    st_bitflag_t class;
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

int st_integer_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_integer_value_compares_and_operates(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

const struct type_iface_t * st_integer_value_type_of(
    const struct value_iface_t *self);

st_bitflag_t st_integer_value_class(
    const struct value_iface_t *self,
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

int st_bool_value_assigns_compares_operates(
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

int st_integer_literal_override_type(
    const struct value_iface_t *self,
    const struct type_iface_t *type,
    const struct config_iface_t *config);

/**************************************************************************/
/* Real values                                                            */
/**************************************************************************/
struct real_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    st_bitflag_t class;
    double num;
};

int st_real_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

int st_real_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config);

int st_real_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_real_value_compares_and_operates(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

const struct type_iface_t * st_real_value_type_of(
    const struct value_iface_t *self);

st_bitflag_t st_real_value_class(
    const struct value_iface_t *self,
    const struct config_iface_t *config);

struct value_iface_t * st_real_value_create_temp_from(
    const struct value_iface_t *self);	

void st_real_value_destroy(
    struct value_iface_t *self);

int st_real_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_real_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_real_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_real_value_plus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_real_value_minus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_real_value_multiply(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_real_value_divide(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_real_value_to_power(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

double st_real_value_real(
    const struct value_iface_t *self,
    const struct config_iface_t *config);

int st_real_literal_override_type(
    const struct value_iface_t *self,
    const struct type_iface_t *type,
    const struct config_iface_t *config);

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
    const struct type_iface_t *type;
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

int st_enum_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

const struct type_iface_t * st_enum_value_type_of(
    const struct value_iface_t *self);

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
    const struct type_iface_t *type;
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

int st_subrange_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_subrange_value_compares_and_operates(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

const struct type_iface_t * st_subrange_value_type_of(
    const struct value_iface_t *self);

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
struct listed_value_t {
    struct value_iface_t *value;
    struct value_iface_t *multiplier;
    struct st_location_t *location;
    struct listed_value_t *prev;
    struct listed_value_t *next;
};

struct array_init_value_t {
    struct value_iface_t value;
    size_t entries;
    struct st_location_t *location;
    struct listed_value_t *values;
};

const struct array_init_value_t * st_array_init_value(
    const struct value_iface_t *self,
    const struct config_iface_t *conf);

struct array_element_t {
    struct value_iface_t *element;
    int64_t key;
    UT_hash_handle hh;
};

struct array_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    struct value_iface_t **elements;
    size_t total_elements;
    const struct array_range_t *ranges;
    const struct type_iface_t *arrayed_type;
};

int st_array_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

int st_array_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_array_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config);

const struct type_iface_t * st_array_value_type_of(
    const struct value_iface_t *self);

struct value_iface_t * st_array_value_index(
    struct value_iface_t *self,
    struct array_index_t *array_index,
    const struct config_iface_t *config);

void st_array_value_destroy(
    struct value_iface_t *self);

void st_array_init_value_destroy(
    struct value_iface_t *self);

/**************************************************************************/
/* Structure value                                                        */
/**************************************************************************/
struct struct_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    struct variable_t *elements;
};

struct struct_init_value_t {
    struct value_iface_t value;
    struct struct_element_init_t *init_table;
};

int st_struct_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

const struct type_iface_t * st_struct_value_type_of(
    const struct value_iface_t *self);

int st_struct_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config);

int st_struct_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config);

void st_struct_value_destroy(
    struct value_iface_t *self);

void st_struct_init_value_destroy(
    struct value_iface_t *self);

struct variable_t * st_struct_value_sub_variable(
    struct value_iface_t *self,
    const char *identifier,
    const struct config_iface_t *config);

const struct struct_init_value_t * st_struct_init_value(
    const struct value_iface_t *self,
    const struct config_iface_t *conf);

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


