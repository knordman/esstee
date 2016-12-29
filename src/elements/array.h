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
#include <elements/iarray_index.h>
#include <expressions/iexpression.h>
#include <util/inamed_ref_pool.h>
#include <elements/subrange.h>


struct array_range_iface_t {

    int (*extend)(
    	struct array_range_iface_t *self,
	struct subrange_iface_t *subrange,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    void (*destroy)(
	struct array_range_iface_t *self);
};

struct array_initializer_iface_t {

    int (*extend_by_value)(
	struct array_initializer_iface_t *self,
	struct value_iface_t *value,
	const struct st_location_t *value_location,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*extend_by_multiplied_value)(
	struct array_initializer_iface_t *self,
	struct value_iface_t *multiplier,
	struct value_iface_t *value,
	const struct st_location_t *location,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    struct value_iface_t * (*value)(
    	struct array_initializer_iface_t *self);
    
    void (*destroy)(
	struct array_initializer_iface_t *self);
};

struct array_index_iface_t * st_create_array_index(
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct array_range_iface_t * st_create_array_range(
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct array_initializer_iface_t * st_create_array_initializer(
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct type_iface_t * st_create_array_type(
    struct array_range_iface_t *array_range,
    char *arrayed_type_identifier,
    const struct st_location_t *arrayed_type_identifier_location,
    struct value_iface_t *default_value,
    const struct st_location_t *default_value_location,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
