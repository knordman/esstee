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

struct struct_elements_iface_t {

    int (*extend_by_type)(
	struct struct_elements_iface_t *self,
	char *identifier,
	const struct st_location_t *identifier_location,
	struct type_iface_t *type,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*extend_by_type_name)(
	struct struct_elements_iface_t *self,
	char *identifier,
	const struct st_location_t *identifier_location,
	char *type_name,
	const struct st_location_t *type_name_location,
	struct named_ref_pool_iface_t *type_refs,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    void (*destroy)(
     	struct struct_elements_iface_t *self);
};

struct struct_initializer_iface_t {

    int (*extend)(
	struct struct_initializer_iface_t *self,
	char *identifier,
	const struct st_location_t *identifier_location,
	struct value_iface_t *value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
	
    struct value_iface_t * (*value)(
    	struct struct_initializer_iface_t *self);

    void (*destroy)(
     	struct struct_initializer_iface_t *self);
   
    const struct st_location_t *location;
};

struct struct_elements_iface_t * st_create_struct_elements(
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct struct_initializer_iface_t * st_create_struct_initializer(
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct type_iface_t * st_create_struct_type(
    struct struct_elements_iface_t *elements,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
