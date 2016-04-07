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

#include <esstee/locations.h>
#include <util/bitflag.h>
#include <util/iissues.h>
#include <util/iconfig.h>

typedef int (*resolved_callback_t)(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

typedef int (*post_resolve_callback_t)(
    void *referrer,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct named_ref_pool_iface_t {

    int (*add_two_step)(
	struct named_ref_pool_iface_t *self,
	const char *identifier,
	void *referrer,
	const struct st_location_t *location,
	resolved_callback_t callback,
	resolved_callback_t secondary_callback,
	struct issues_iface_t *issues);

    int (*add)(
	struct named_ref_pool_iface_t *self,
	const char *identifier,
	void *referrer,
	const struct st_location_t *location,
	resolved_callback_t callback,
	struct issues_iface_t *issues);

    int (*add_post_resolve)(
    	struct named_ref_pool_iface_t *self,
	void *referrer,
	post_resolve_callback_t callback,
	struct issues_iface_t *issues);
	
    void (*commit)(
	struct named_ref_pool_iface_t *self);

    void (*clear)(
	struct named_ref_pool_iface_t *self);
    
    const char * (*next_unresolved)(
	struct named_ref_pool_iface_t *self);

    int (*resolve_with_remark)(
	struct named_ref_pool_iface_t *self,
	const char *identifier,
	void *target,
	st_bitflag_t remark);

    int (*resolve)(
	struct named_ref_pool_iface_t *self,
	const char *identifier,
	void *target);

    int (*reset_resolved)(
	struct named_ref_pool_iface_t *self);

    int (*trigger_resolve_callbacks)(
	struct named_ref_pool_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    struct named_ref_pool_iface_t * (*merge)(
	struct named_ref_pool_iface_t *self,
	struct named_ref_pool_iface_t *to_merge);

    void (*destroy)(
	struct named_ref_pool_iface_t *self);

};

