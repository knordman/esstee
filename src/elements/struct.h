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

struct struct_element_t;
struct struct_element_init_t;

struct struct_element_t * st_extend_element_group(
    struct struct_element_t *element_group,
    char *element_identifier,
    const struct st_location_t *identifier_location,
    struct type_iface_t *element_type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct struct_element_t * st_extend_element_group_type_name(
    struct struct_element_t *element_group,
    char *element_identifier,
    const struct st_location_t *identifier_location,
    char *element_type_name,
    const struct st_location_t *element_type_name_location,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

void st_destroy_struct_element_group(
    struct struct_element_t *element_group);

struct type_iface_t * st_create_struct_type(
    struct struct_element_t *element_group,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct struct_element_init_t * st_create_element_initializer(
    char *element_identifier,
    const struct st_location_t *identifier_location,
    struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct struct_element_init_t * st_extend_element_initializer_group(
    struct struct_element_init_t *initializer_group,
    struct struct_element_init_t *element_initializer,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

void st_destroy_element_initializer(
    struct struct_element_init_t *initializer);

void st_destroy_initializer_group(
    struct struct_element_init_t *initializer_group);

struct value_iface_t * st_create_struct_initializer_value(
    struct struct_element_init_t *initializer_group,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
