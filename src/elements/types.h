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

/**************************************************************************/
/* Elementary types                                                       */
/**************************************************************************/
#define INTEGER_NUMERIC_CLASS (1 << 0)
#define INTEGER_BITDATA_CLASS (1 << 1)
#define REAL_NUMERIC_CLASS    (1 << 2)
#define STRING_CLASS          (1 << 3)
#define INTEGER_SIGNED        (1 << 4)
#define INTEGER_UNSIGNED      (1 << 5)
#define INTEGER_BOOL_TYPE     (1 << 6)
#define INTEGER_SINT_TYPE     (1 << 7)
#define INTEGER_INT_TYPE      (1 << 8)
#define INTEGER_DINT_TYPE     (1 << 9)
#define INTEGER_LINT_TYPE     (1 << 10)
#define INTEGER_USINT_TYPE    (1 << 11)
#define INTEGER_UINT_TYPE     (1 << 12)
#define INTEGER_UDINT_TYPE    (1 << 13)
#define INTEGER_ULINT_TYPE    (1 << 14)
#define INTEGER_BYTE_TYPE     (1 << 15)
#define INTEGER_WORD_TYPE     (1 << 16)
#define INTEGER_DWORD_TYPE    (1 << 17)
#define INTEGER_LWORD_TYPE    (1 << 18)
#define REAL_TYPE             (1 << 19)
#define LREAL_TYPE            (1 << 20)
#define STRING_TYPE           (1 << 21)
#define WSTRING_TYPE          (1 << 22)
#define TIME_TYPE             (1 << 23)
#define DATE_TYPE             (1 << 24)
#define TOD_TYPE              (1 << 25)
#define DATE_TOD_TYPE         (1 << 26)
#define DERIVED_TYPE          (1 << 27)

const struct st_location_t * st_built_in_type_location_get(
    const struct type_iface_t *self);

int st_type_general_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config);

struct type_iface_t * st_new_elementary_types(void);

void st_destroy_types_in_list(
    struct type_iface_t *types);

/**************************************************************************/
/* Integer types                                                          */
/**************************************************************************/
struct integer_type_t {
    struct type_iface_t type;
    unsigned size;
    int64_t default_value;
    int64_t min;
    int64_t max;
};

struct value_iface_t * st_integer_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config);

int st_integer_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config);

int st_integer_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config);

void st_integer_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Real types                                                             */
/**************************************************************************/
struct real_type_t {
    struct type_iface_t type;
    unsigned size;
    double default_value;
};

struct value_iface_t * st_real_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config);

int st_real_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config);

int st_real_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config);

void st_real_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* String types                                                           */
/**************************************************************************/
struct string_type_t {
    struct type_iface_t type;
    unsigned length;
    const char *default_value;
};

struct value_iface_t * st_string_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config);

int st_string_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config);

int st_string_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config);

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
    const struct config_iface_t *config);

int st_duration_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config);

int st_duration_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config);

void st_duration_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Date type                                                              */
/**************************************************************************/
struct date_type_t {
    struct type_iface_t type;
    unsigned default_year;
    unsigned default_month;
    unsigned default_day;
};

struct value_iface_t * st_date_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config);

int st_date_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config);

int st_date_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config);

void st_date_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Tod type                                                               */
/**************************************************************************/
struct tod_type_t {
    struct type_iface_t type;
    unsigned default_hour;
    unsigned default_minute;
    unsigned default_second;
};

struct value_iface_t * st_tod_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config);

int st_tod_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config);

int st_tod_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config);

void st_tod_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Date tod type                                                          */
/**************************************************************************/
struct date_tod_type_t {
    struct type_iface_t type;
    unsigned default_year;
    unsigned default_month;
    unsigned default_day;
    unsigned default_hour;
    unsigned default_minute;
    unsigned default_second;
};

struct value_iface_t * st_date_tod_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config);

int st_date_tod_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config);

int st_date_tod_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config);

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
};

const struct st_location_t * st_derived_type_location(
    const struct type_iface_t *self);

struct value_iface_t * st_derived_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config);

int st_derived_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config);

int st_derived_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config);

void st_derived_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Enumerated type                                                        */
/**************************************************************************/
struct enum_item_t {
    char *identifier;
    UT_hash_handle hh;
};

struct enum_type_t {
    struct type_iface_t type;
    struct enum_item_t *values;
    struct st_location_t *location;
    struct enum_item_t *default_item;
};

const struct st_location_t * st_enum_type_location(
    const struct type_iface_t *self);

struct value_iface_t * st_enum_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config);

int st_enum_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config);

int st_enum_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config);

int st_enum_type_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config);

void st_enum_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Subrange type                                                          */
/**************************************************************************/
struct subrange_t {
    struct value_iface_t *min;
    struct value_iface_t *max;
};

struct subrange_type_t {
    struct type_iface_t type;
    struct type_iface_t *subranged_type;
    struct expression_iface_t *default_value;
    struct subrange_t *subrange;
};

const struct st_location_t * st_subrange_type_location(
    const struct type_iface_t *self);

struct value_iface_t * st_subrange_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config);

int st_subrange_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config);

int st_subrange_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config);

int st_subrange_type_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config);

void st_subrange_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Array type                                                             */
/**************************************************************************/
struct array_range_t {
    struct subrange_t *subrange;
    unsigned entries;
    struct array_range_t *prev;
    struct array_range_t *next;
};

struct array_type_t {
    struct type_iface_t type;
    struct type_iface_t *arrayed_type;
    struct array_range_t *ranges;
    struct array_init_value_t *default_value;
};

const struct st_location_t * st_array_type_location(
    const struct type_iface_t *self);

struct value_iface_t * st_array_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config);

int st_array_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config);

void st_array_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Structure type                                                         */
/**************************************************************************/
struct struct_element_t {
    char *element_identifier;
    struct type_iface_t *element_type;
    UT_hash_handle hh;
};

struct struct_element_init_t {
    char *element_identifier;
    struct value_iface_t *element_default_value;
    UT_hash_handle hh;
};

struct struct_type_t {
    struct type_iface_t type;
    struct struct_element_t *elements;
};

const struct st_location_t * st_struct_type_location(
    const struct type_iface_t *self);

struct value_iface_t * st_struct_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config);

int st_struct_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config);

void st_struct_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* Function block type                                                    */
/**************************************************************************/
const struct st_location_t * st_function_block_type_location(
    const struct type_iface_t *self);

struct value_iface_t * st_function_block_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config);

int st_function_block_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config);

void st_function_block_type_destroy(
    struct type_iface_t *self);

/**************************************************************************/
/* String type with defined length                                        */
/**************************************************************************/

/* TODO: structs for string type with defined length */
