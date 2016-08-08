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

#include <elements/ivariable.h>
#include <util/inamed_ref_pool.h>

struct variable_stub_t;

struct variable_stub_t * st_extend_variable_stubs(
    struct variable_stub_t *stubs,
    char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

void st_destroy_variable_stubs(
    struct variable_stub_t *stubs);

struct variable_stub_t * st_set_variable_stubs_type(
    struct variable_stub_t *stubs,
    struct type_iface_t *type,
    const struct config_iface_t *config,
    struct issues_iface_t *issue);
    
struct variable_stub_t * st_set_variable_stubs_type_name(
    struct variable_stub_t *stubs,
    char *type_name,
    const struct config_iface_t *config,
    struct issues_iface_t *issue);

struct variable_stub_t * st_concatenate_variable_stubs(
    struct variable_stub_t *stubs_one,
    struct variable_stub_t *stubs_two,
    const struct config_iface_t *config,
    struct issues_iface_t *issue);

struct variable_stub_t * st_create_direct_variable_stub(
    char *identifier,
    struct direct_address_t *address,
    char *type_name,
    const struct st_location_t *type_name_location,
    const struct st_location_t *location,
    struct value_iface_t *default_value,
    const struct st_location_t *default_value_location,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct variable_iface_t * st_create_variable_block(
    st_bitflag_t block_class,
    st_bitflag_t retain_flag,
    st_bitflag_t constant_flag,
    struct variable_stub_t *stubs,
    struct named_ref_pool_iface_t *global_var_refs,
    struct named_ref_pool_iface_t *type_refs,
    struct named_ref_pool_iface_t *global_type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issue);

struct variable_iface_t * st_create_variable_type_name(
    char *identifier,
    const struct st_location_t *location, 
    char *type_name,
    const struct st_location_t *type_name_location,
    st_bitflag_t class,
    struct named_ref_pool_iface_t *global_var_refs,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct variable_iface_t * st_create_variable_type(
    char *identifier,
    const struct st_location_t *location, 
    const struct type_iface_t *type,
    st_bitflag_t class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

