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

struct type_iface_t * st_new_elementary_string_types();

struct type_iface_t * st_new_custom_length_string_type(
    const char *type_name,
    struct value_iface_t *length,
    const struct st_location_t *length_location,
    struct value_iface_t *default_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct value_iface_t * st_new_string_value(
    const char *type_name,
    char *content,
    struct named_ref_pool_iface_t *global_type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
