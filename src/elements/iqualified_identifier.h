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

#include <elements/ivalue.h>
#include <util/iconfig.h>
#include <util/iissues.h>
#include <rt/isystime.h>
#include <rt/icursor.h>

struct qualified_identifier_iface_t {

    int (*verify)(
	struct qualified_identifier_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*step)(
	struct qualified_identifier_iface_t *self,
	struct cursor_iface_t *cursor,
	const struct systime_iface_t *time,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*reset)(
	struct qualified_identifier_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*allocate)(
	struct qualified_identifier_iface_t *self,
	struct issues_iface_t *issues);
    
    struct qualified_identifier_iface_t * (*clone)(
	struct qualified_identifier_iface_t *self,
	struct issues_iface_t *issues);

    int (*set_base)(
    	struct qualified_identifier_iface_t *self,
	struct variable_iface_t *base,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    const char * (*base_identifier)(
    	struct qualified_identifier_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    const struct value_iface_t * (*target)(
	struct qualified_identifier_iface_t *self);

    int (*assign_target)(
    	struct qualified_identifier_iface_t *self,
	const struct value_iface_t *new_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    const char * (*target_name)(
	struct qualified_identifier_iface_t *self);

    void (*destroy)(
    	struct qualified_identifier_iface_t *self);
    
    int constant_reference;
    const struct st_location_t *location;
};
