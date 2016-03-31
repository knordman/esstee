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

#include <elements/itype.h>
#include <util/bitflag.h>
#include <util/iissues.h>

/**************************************************************************/
/* Elementary types                                                       */
/**************************************************************************/
#define INTEGER_NUMERIC_CLASS (1 << 0)
#define INTEGER_BITDATA_CLASS (1 << 1)
#define REAL_NUMERIC_CLASS    (1 << 2)
#define INTEGER_SIGNED        (1 << 3)
#define INTEGER_UNSIGNED      (1 << 4)
#define INTEGER_BOOL_TYPE     (1 << 5)
#define INTEGER_SINT_TYPE     (1 << 6)
#define INTEGER_INT_TYPE      (1 << 7)
#define INTEGER_DINT_TYPE     (1 << 8)
#define INTEGER_LINT_TYPE     (1 << 9)
#define INTEGER_USINT_TYPE    (1 << 10)
#define INTEGER_UINT_TYPE     (1 << 11)
#define INTEGER_UDINT_TYPE    (1 << 12)
#define INTEGER_ULINT_TYPE    (1 << 13)
#define INTEGER_BYTE_TYPE     (1 << 14)
#define INTEGER_WORD_TYPE     (1 << 15)
#define INTEGER_DWORD_TYPE    (1 << 16)
#define INTEGER_LWORD_TYPE    (1 << 17)
#define REAL_TYPE             (1 << 18)
#define LREAL_TYPE            (1 << 19)
#define STRING_TYPE           (1 << 20)
#define WSTRING_TYPE          (1 << 21)
#define DURATION_TYPE         (1 << 22)
#define DATE_TYPE             (1 << 23)
#define TOD_TYPE              (1 << 24)
#define DATE_TOD_TYPE         (1 << 25)
#define DERIVED_TYPE          (1 << 26)
#define ENUM_TYPE             (1 << 27)
#define SUBRANGE_TYPE         (1 << 28)
#define ARRAY_TYPE            (1 << 29)
#define STRUCT_TYPE           (1 << 30)
#define FB_TYPE               (1 << 31)

const struct st_location_t * st_built_in_type_location_get(
    const struct type_iface_t *self);

int st_type_general_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct type_iface_t * st_new_elementary_types();

void st_destroy_types_in_list(
    struct type_iface_t *types);

/**************************************************************************/
/* Integer types                                                          */
/**************************************************************************/
struct integer_type_t {
    struct type_iface_t type;
    st_bitflag_t class;
    unsigned size;
    int64_t default_value;
    int64_t min;
    int64_t max;
};

struct value_iface_t * st_integer_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

st_bitflag_t st_integer_type_class(
    const struct type_iface_t *self);

void st_integer_type_destroy(
    struct type_iface_t *self);

/* Bool specializations */
struct value_iface_t * st_bool_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct value_iface_t * st_bool_type_create_temp_value(
    struct issues_iface_t *issues);

int st_bool_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_bool_type_true(
    struct value_iface_t *value);

int st_bool_type_false(
    struct value_iface_t *value);

/**************************************************************************/
/* Real types                                                             */
/**************************************************************************/
struct real_type_t {
    struct type_iface_t type;
    unsigned size;
    st_bitflag_t class;
    double default_value;
};

struct value_iface_t * st_real_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_real_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_real_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

st_bitflag_t st_real_type_class(
    const struct type_iface_t *self);

void st_real_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* String types                                                           */
/**************************************************************************/
struct string_type_t {
    struct type_iface_t type;
    size_t length;
    st_bitflag_t class;
    const char *default_value;
};

struct value_iface_t * st_string_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_string_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_string_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

st_bitflag_t st_string_type_class(
    const struct type_iface_t *self);

void st_string_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Duration type                                                          */
/**************************************************************************/
struct duration_type_t {
    struct type_iface_t type;
    double default_d;
    double default_h;
    double default_m;
    double default_s;
    double default_ms;
};

struct value_iface_t * st_duration_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_duration_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_duration_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

st_bitflag_t st_duration_type_class(
    const struct type_iface_t *self);

void st_duration_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Date type                                                              */
/**************************************************************************/
struct date_type_t {
    struct type_iface_t type;
    uint64_t default_year;
    uint8_t default_month;
    uint8_t default_day;
};

struct value_iface_t * st_date_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_date_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_date_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

st_bitflag_t st_date_type_class(
    const struct type_iface_t *self);

void st_date_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Tod type                                                               */
/**************************************************************************/
struct tod_type_t {
    struct type_iface_t type;
    uint8_t default_hour;
    uint8_t default_minute;
    uint8_t default_second;
    uint8_t default_fractional_second;
};

struct value_iface_t * st_tod_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_tod_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_tod_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

st_bitflag_t st_tod_type_class(
    const struct type_iface_t *self);

void st_tod_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Date tod type                                                          */
/**************************************************************************/
struct date_tod_type_t {
    struct type_iface_t type;
    uint64_t default_year;
    uint8_t default_month;
    uint8_t default_day;
    uint8_t default_hour;
    uint8_t default_minute;
    uint8_t default_second;
    uint8_t default_fractional_second;
};

struct value_iface_t * st_date_tod_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);   

int st_date_tod_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_date_tod_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

st_bitflag_t st_date_tod_type_class(
    const struct type_iface_t *self);

void st_date_tod_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Derived type                                                           */
/**************************************************************************/
struct derived_type_t {
    struct type_iface_t type;
    struct type_iface_t *ancestor;
    struct type_iface_t *parent;
    struct st_location_t *location;
    struct value_iface_t *default_value;
    struct st_location_t *default_value_location;
};

const struct st_location_t * st_derived_type_location(
    const struct type_iface_t *self);

struct value_iface_t * st_derived_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_derived_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_derived_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_derived_type_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

st_bitflag_t st_derived_type_class(
    const struct type_iface_t *self);

void st_derived_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Enumerated type                                                        */
/**************************************************************************/
struct enum_item_t {
    char *identifier;
    struct st_location_t *location;
    struct enum_item_t *group;
    UT_hash_handle hh;
};

struct enum_type_t {
    struct type_iface_t type;
    struct enum_item_t *values;
    struct st_location_t *location;
    const struct enum_item_t *default_item;
};

const struct st_location_t * st_enum_type_location(
    const struct type_iface_t *self);

struct value_iface_t * st_enum_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_enum_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_enum_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

st_bitflag_t st_enum_type_class(
    const struct type_iface_t *self);

void st_enum_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Subrange type                                                          */
/**************************************************************************/
struct subrange_t {
    struct value_iface_t *min;
    struct st_location_t *min_location;
    struct value_iface_t *max;
    struct st_location_t *max_location;
};

struct subrange_type_t {
    struct type_iface_t type;
    struct type_iface_t *subranged_type;
    struct value_iface_t *default_value;
    struct st_location_t *default_value_location;
    struct subrange_t *subrange;
};

struct value_iface_t * st_subrange_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_subrange_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_subrange_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

st_bitflag_t st_subrange_type_class(
    const struct type_iface_t *self);

int st_subrange_type_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

void st_subrange_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Array type                                                             */
/**************************************************************************/
struct array_range_t {
    struct subrange_t *subrange;
    size_t entries;
    struct array_range_t *prev;
    struct array_range_t *next;
};

struct array_type_t {
    struct type_iface_t type;
    struct type_iface_t *arrayed_type;
    struct array_range_t *ranges;
    size_t total_elements;
    struct value_iface_t *default_value;
};

int st_array_type_check_array_initializer(
    struct array_range_t *ranges,
    const struct value_iface_t *default_value,
    struct type_iface_t *arrayed_type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_array_type_assign_default_value(
    struct value_iface_t **elements,
    const struct value_iface_t *default_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct value_iface_t * st_array_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_array_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_array_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

st_bitflag_t st_array_type_class(
    const struct type_iface_t *self);

void st_array_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Structure type                                                         */
/**************************************************************************/
struct struct_element_t {
    char *element_identifier;
    struct st_location_t *identifier_location;
    struct type_iface_t *element_type;
    UT_hash_handle hh;
};

struct struct_element_init_t {
    char *element_identifier;
    struct value_iface_t *element_default_value;
    struct st_location_t *element_identifier_location;
    UT_hash_handle hh;
};

struct struct_type_t {
    struct type_iface_t type;
    struct struct_element_t *elements;
};

struct value_iface_t * st_struct_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_struct_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_struct_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

st_bitflag_t st_struct_type_class(
    const struct type_iface_t *self);

void st_struct_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Function block type                                                    */
/**************************************************************************/
const struct st_location_t * st_function_block_type_location(
    const struct type_iface_t *self);

struct value_iface_t * st_function_block_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_function_block_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

st_bitflag_t st_function_block_type_class(
    const struct type_iface_t *self);

void st_function_block_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* String type with defined length                                        */
/**************************************************************************/

/* TODO: structs for string type with defined length */
