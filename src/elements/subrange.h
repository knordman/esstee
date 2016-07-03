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

struct subrange_t {
    struct value_iface_t *min;
    struct st_location_t *min_location;
    struct value_iface_t *max;
    struct st_location_t *max_location;
};

struct subrange_t * st_create_subrange(
    struct value_iface_t *min,
    const struct st_location_t *min_location,
    struct value_iface_t *max,
    const struct st_location_t *max_location,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

void st_destroy_subrange(
    struct subrange_t *subrange);

struct type_iface_t * st_create_subrange_type(
    char *storage_type_identifier,
    const struct st_location_t *storage_type_identifier_location,
    struct subrange_t *subrange,
    struct value_iface_t *default_value,
    const struct st_location_t *default_value_location,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct value_iface_t * st_create_subrange_value(
    struct subrange_t *subrange,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
