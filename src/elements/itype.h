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

#include <elements/ivalue.h>
#include <util/iconfig.h>
#include <util/bitflag.h>

#include <uthash.h>

struct value_iface_t;

struct type_iface_t {

    const struct st_location_t * (*location)(
	const struct type_iface_t *self);
    
    struct value_iface_t * (*create_value_of)(
	const struct type_iface_t *self,
	const struct config_iface_t *config);

    int (*reset_value_of)(
	const struct type_iface_t *self,
	struct value_iface_t *value_of,
	const struct config_iface_t *config);
    
    int (*can_hold)(
    	const struct type_iface_t *self,
	const struct value_iface_t *value,
	const struct config_iface_t *config);
    
    int (*compatible)(
	const struct type_iface_t *self,
	const struct type_iface_t *other_type,
	const struct config_iface_t *config);

    st_bitflag_t (*class)(
	const struct type_iface_t *self,
	const struct config_iface_t *config);
    
    void (*destroy)(
	struct type_iface_t *self);

    char *identifier;
    UT_hash_handle hh;
    struct type_iface_t *prev;
    struct type_iface_t *next;
};
