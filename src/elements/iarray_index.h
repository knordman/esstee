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


#include <util/iconfig.h>
#include <util/iissues.h>
#include <rt/isystime.h>
#include <rt/cursor.h>

struct array_index_element_t {
    const struct expression_iface_t *expression;
    const struct array_index_element_t *next;
};

struct array_index_iface_t {

    int (*step)(
	struct array_index_iface_t *self,
	struct cursor_iface_t *cursor,
	const struct systime_iface_t *time,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*reset)(
	struct array_index_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*allocate)(
	struct array_index_iface_t *self,
	struct issues_iface_t *issues);
    
    struct array_index_iface_t * (*clone)(
	struct array_index_iface_t *self,
	struct issues_iface_t *issues);

    int (*extend)(
	struct array_index_iface_t *self,
	struct expression_iface_t *expression,
	const struct st_location_t *expression_location,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*constant_reference)(
    	struct array_index_iface_t *self);
	
    void (*destroy)(
	struct array_index_iface_t *self);

    const struct array_index_element_t *first_node;
    const struct st_location_t *location;
};
