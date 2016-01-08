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

#include <util/ierrors.h>
#include <esstee/locations.h>
#include <util/bitflag.h>

#define PROGRAM_IN_QUERY_RESOLVE_REMARK (1 << 0)

typedef int (*resolved_callback_t)(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors);

struct namedreference_iface_t {

    int (*add)(
	struct namedreference_iface_t *self,
	const char *identifier,
	void *referrer,
	void *subreferrer,
	const struct st_location_t *location,
	resolved_callback_t callback);

    int (*commit)(
	struct namedreference_iface_t *self);

    int (*clear)(
	struct namedreference_iface_t *self);
    
    const char * (*next_unresolved)(
	struct namedreference_iface_t *self);

    int (*resolve)(
	struct namedreference_iface_t *self,
	const char *identifier,
	void *target);

    int (*resolve_with_remark)(
	struct namedreference_iface_t *self,
	const char *identifier,
	void *target,
	st_bitflag_t remark);

    int (*reset_resolved)(
	struct namedreference_iface_t *self);
    
    int (*trigger_resolve_callbacks)(
	struct namedreference_iface_t *self,
	struct errors_iface_t *err);
    
    struct namedreference_iface_t * (*merge)(
	struct namedreference_iface_t *self,
	struct namedreference_iface_t *to_merge);

};
