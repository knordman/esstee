/*
Copyright (C) 2016 Kristian Nordman

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
#include <util/inamed_ref_pool.h>
#include <elements/subrange.h>
#include <expressions/iexpression.h>

struct array_range_t;
struct listed_value_t;

struct array_index_t {
    struct expression_iface_t *index_expression;
    struct st_location_t *location;
    struct array_index_t *prev;
    struct array_index_t *next;
};

void st_destroy_array_index(
    struct array_index_t *array_index);

struct array_range_t * st_extend_array_range(
    struct array_range_t *array_ranges,
    struct subrange_t *subrange,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

void st_destroy_array_ranges(
    struct array_range_t *array_ranges);

struct listed_value_t * st_extend_array_initializer(
    struct listed_value_t *values,
    struct value_iface_t *multiplier,
    struct value_iface_t *new_value,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

void st_destroy_listed_values(
    struct listed_value_t *values);

struct value_iface_t * st_create_array_init_value(
    struct listed_value_t *values,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct type_iface_t * st_create_array_type(
    struct array_range_t *array_ranges,
    char *arrayed_type_identifier,
    const struct st_location_t *arrayed_type_identifier_location,
    struct value_iface_t *default_value,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
