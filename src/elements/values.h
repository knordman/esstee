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
#define CONSTANT_VALUE  (1 << 1)

/**************************************************************************/
/* General value methods                                                  */
/**************************************************************************/
st_bitflag_t st_general_value_empty_class(
    const struct value_iface_t *self);

int st_general_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

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
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_value_compares_and_operates(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct type_iface_t * st_integer_value_type_of(
    const struct value_iface_t *self);

st_bitflag_t st_integer_value_class(
    const struct value_iface_t *self);

struct value_iface_t * st_integer_value_create_temp_from(
    const struct value_iface_t *self,
    struct issues_iface_t *issues);

void st_integer_value_destroy(
    struct value_iface_t *self);

int st_integer_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_value_negate(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_value_plus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_value_minus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_value_multiply(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_value_divide(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_value_modulus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_value_to_power(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int64_t st_integer_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/* Bool specializations */
int st_bool_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

int st_bool_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_bool_value_assigns_compares_operates(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct value_iface_t * st_bool_value_create_temp_from(
    const struct value_iface_t *self,
    struct issues_iface_t *issues);

int st_bool_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_bool_value_bool(
    const struct value_iface_t *self,
    const struct config_iface_t *conf,
    struct issues_iface_t *issues);

int st_bool_value_not(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_bool_value_xor(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_bool_value_and(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
    
int st_bool_value_or(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_literal_override_type(
    const struct value_iface_t *self,
    const struct type_iface_t *type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

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
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_real_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_real_value_compares_and_operates(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct type_iface_t * st_real_value_type_of(
    const struct value_iface_t *self);

st_bitflag_t st_real_value_class(
    const struct value_iface_t *self);

struct value_iface_t * st_real_value_create_temp_from(
    const struct value_iface_t *self,
    struct issues_iface_t *issues);

void st_real_value_destroy(
    struct value_iface_t *self);

int st_real_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_real_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_real_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_real_value_negate(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_real_value_plus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_real_value_minus(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_real_value_multiply(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_real_value_divide(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_real_value_to_power(
    struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

double st_real_value_real(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_real_literal_override_type(
    const struct value_iface_t *self,
    const struct type_iface_t *type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

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
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_enum_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct type_iface_t * st_enum_value_type_of(
    const struct value_iface_t *self);

void st_enum_value_destroy(
    struct value_iface_t *self);

int st_enum_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct enum_item_t * st_enum_value_enumeration(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

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
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_subrange_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_subrange_value_compares_and_operates(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct type_iface_t * st_subrange_value_type_of(
    const struct value_iface_t *self);

struct value_iface_t * st_subrange_value_create_temp_from(
    const struct value_iface_t *self,
    struct issues_iface_t *issues);

int st_subrange_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_subrange_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_subrange_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

void st_subrange_value_destroy(
    struct value_iface_t *self);

int64_t st_subrange_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

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
    const struct value_iface_t *self);

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
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_array_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct type_iface_t * st_array_value_type_of(
    const struct value_iface_t *self);

struct value_iface_t * st_array_value_index(
    struct value_iface_t *self,
    struct array_index_t *array_index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

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
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_struct_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);    

void st_struct_value_destroy(
    struct value_iface_t *self);

void st_struct_init_value_destroy(
    struct value_iface_t *self);

struct variable_t * st_struct_value_sub_variable(
    struct value_iface_t *self,
    const char *identifier,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct struct_init_value_t * st_struct_init_value(
    const struct value_iface_t *self);

/**************************************************************************/
/* Duration value                                                         */
/**************************************************************************/
struct duration_t {
    double d;
    double h;
    double m;
    double s;
    double ms;
};

struct duration_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    struct duration_t duration;
};

int st_duration_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

int st_duration_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_duration_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct type_iface_t * st_duration_value_type_of(
    const struct value_iface_t *self);

void st_duration_value_destroy(
    struct value_iface_t *self);

int st_duration_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_duration_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct duration_t * st_duration_value_duration(
    const struct value_iface_t *self,
    const struct config_iface_t *conf,
    struct issues_iface_t *issues);

/**************************************************************************/
/* Date value                                                             */
/**************************************************************************/
struct date_t {
    uint64_t y;
    uint8_t m;
    uint8_t d;
};

struct date_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    struct date_t date;
};

int st_date_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

int st_date_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_date_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct type_iface_t * st_date_value_type_of(
    const struct value_iface_t *self);

void st_date_value_destroy(
    struct value_iface_t *self);

int st_date_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_date_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct date_t * st_date_value_date(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/**************************************************************************/
/* Tod value                                                              */
/**************************************************************************/
struct tod_t {
    uint8_t h;
    uint8_t m;
    uint8_t s;
    uint8_t fs;
};

struct tod_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    struct tod_t tod;
};

int st_tod_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

int st_tod_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_tod_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct type_iface_t * st_date_value_type_of(
    const struct value_iface_t *self);

void st_tod_value_destroy(
    struct value_iface_t *self);

int st_tod_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_tod_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct tod_t * st_tod_value_tod(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/**************************************************************************/
/* Date tod value                                                         */
/**************************************************************************/
struct date_tod_t {
    struct date_t date;
    struct tod_t tod;
};

struct date_tod_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    struct date_tod_t dt;
};

int st_date_tod_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

int st_date_tod_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_date_tod_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct type_iface_t * st_date_tod_value_type_of(
    const struct value_iface_t *self);

void st_date_tod_value_destroy(
    struct value_iface_t *self);

int st_date_tod_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_date_tod_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct date_tod_t * st_date_tod_value_tod(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/**************************************************************************/
/* String values                                                          */
/**************************************************************************/
struct string_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    const char *str;
};

int st_string_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

int st_string_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_string_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const struct type_iface_t * st_string_value_type_of(
    const struct value_iface_t *self);

void st_string_value_destroy(
    struct value_iface_t *self);

void st_string_literal_value_destroy(
    struct value_iface_t *self);

int st_string_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

const char * st_string_value_string(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/**************************************************************************/
/* Function block value                                                   */
/**************************************************************************/

struct function_block_value_t {
    struct value_iface_t value;
    struct variable_t *variables;
    struct invoke_iface_t *statements;
    const struct type_iface_t *type;
    int invoke_state;
};

int st_function_block_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config);

const struct type_iface_t * st_function_block_value_type_of(
    const struct value_iface_t *self);

void st_function_block_value_destroy(
    struct value_iface_t *self);

struct variable_t * st_function_block_value_sub_variable(
    struct value_iface_t *self,
    const char *identifier,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_function_block_value_invoke_verify(
    struct value_iface_t *self,
    struct invoke_parameter_t *parameters,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_function_block_value_invoke_step(
    struct value_iface_t *self,
    struct invoke_parameter_t *parameters,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_function_block_value_invoke_reset(
    struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

/**************************************************************************/
/* Inline values                                                          */
/**************************************************************************/
struct inline_enum_value_t {
    struct value_iface_t value;
    struct enum_item_t data;
};

int st_inline_enum_value_comparable_to(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_inline_enum_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

void st_inline_enum_value_destroy(
    struct value_iface_t *self);

const struct enum_item_t * st_inline_enum_value_enumeration(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct subrange_case_value_t {
    struct value_iface_t value;
    struct subrange_t *subrange;
};

int st_subrange_case_value_comparable_to(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_subrange_case_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

void st_subrange_case_value_destroy(
    struct value_iface_t *self);

/**************************************************************************/
/* Direct address term value                                              */
/**************************************************************************/
struct direct_address_term_value_t {
    struct value_iface_t value;
    struct direct_address_t *address;
    int64_t data;
};

int st_direct_address_term_value_comparable_to(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_direct_address_term_value_operates_with(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct value_iface_t * st_direct_address_term_value_create_temp_from(
    const struct value_iface_t *self,
    struct issues_iface_t *issues);

int st_direct_address_term_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_direct_address_term_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
    
int st_direct_address_term_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int64_t st_direct_address_term_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_direct_address_term_value_bool(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

