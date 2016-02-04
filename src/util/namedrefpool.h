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

#include <util/inamedreference.h>

#include <uthash.h>

struct named_ref_referrer_t {
    void *referrer;
    void *subreferrer;
    resolved_callback_t callback;
    resolved_callback_t secondary_callback;
    struct st_location_t location;
    
    struct named_ref_referrer_t *prev;
    struct named_ref_referrer_t *next;

    struct named_ref_referrer_t *nc_prev;
    struct named_ref_referrer_t *nc_next;
};

struct named_ref_entry_t {
    char *identifier;
    void *target;
    st_bitflag_t remark;
    
    struct named_ref_referrer_t *referrers;
    
    struct named_ref_entry_t *prev;
    struct named_ref_entry_t *next;    
    UT_hash_handle hh;
};

struct named_ref_pool_t {
    struct namedreference_iface_t named_ref;

    struct named_ref_entry_t *iterator;
    struct named_ref_entry_t *ref_table;
    struct named_ref_referrer_t *not_committed;
};

struct namedreference_iface_t * st_new_named_ref_pool(void);

void st_destroy_named_ref_pool(
    struct namedreference_iface_t *self);

int st_named_ref_pool_add_two_step_ref(
    struct namedreference_iface_t *self,
    const char *identifier,
    void *referrer,
    void *subreferrer,
    const struct st_location_t *location,
    resolved_callback_t callback,
    resolved_callback_t secondary_callback);

int st_named_ref_pool_add_ref(
    struct namedreference_iface_t *self,
    const char *identifier,
    void *referrer,
    void *subreferrer,
    const struct st_location_t *location,
    resolved_callback_t callback);

int st_named_ref_pool_commit(
    struct namedreference_iface_t *self);

int st_named_ref_pool_clear(
    struct namedreference_iface_t *self);

const char * st_named_ref_pool_next_unresolved(
    struct namedreference_iface_t *self);

int st_named_ref_pool_resolve(
    struct namedreference_iface_t *self,
    const char *identifier,
    void *target);

int st_named_ref_pool_resolve_with_remark(
    struct namedreference_iface_t *self,
    const char *identifier,
    void *target,
    st_bitflag_t remark);

int st_named_ref_pool_reset_resolved(
    struct namedreference_iface_t *self);

int st_named_ref_pool_trigger_resolve_callbacks(
    struct namedreference_iface_t *self,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

struct namedreference_iface_t * st_named_ref_pool_merge(
    struct namedreference_iface_t *self,
    struct namedreference_iface_t *to_merge);
