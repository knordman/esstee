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

struct enum_item_t {
    const char *identifier;
    const struct st_location_t *location;
};

struct enum_group_iface_t {

    int (*extend)(
	struct enum_group_iface_t *self,
	char *identifier,
	const struct st_location_t *location,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
	
    void (*destroy)(
	struct enum_group_iface_t *self);
};

struct enum_group_iface_t * st_create_enum_group(
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct type_iface_t * st_create_enum_type(
    struct enum_group_iface_t *group, 
    const char *default_item,
    const struct st_location_t *default_item_location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct value_iface_t * st_create_enum_value(
    char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
