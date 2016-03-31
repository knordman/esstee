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

#include <util/named_ref_pool.h>
#include <util/macros.h>

#include <uthash.h>
#include <utlist.h>


struct named_ref_referrer_t {
    void *referrer;
    resolved_callback_t callback;
    resolved_callback_t secondary_callback;
    struct st_location_t *location;
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
    struct named_ref_pool_iface_t named_ref_pool;
    struct named_ref_entry_t *iterator;
    struct named_ref_entry_t *ref_table;
    struct named_ref_referrer_t *not_committed;
};

static int named_ref_pool_add_two_step(
    struct named_ref_pool_iface_t *self,
    const char *identifier,
    void *referrer,
    const struct st_location_t *location,
    resolved_callback_t callback,
    resolved_callback_t secondary_callback,
    struct issues_iface_t *issues)
{
    struct named_ref_pool_t *np =
	CONTAINER_OF(self, struct named_ref_pool_t, named_ref_pool);

    struct named_ref_referrer_t *referrer_entry = NULL;
    struct named_ref_entry_t *ref_entry = NULL;
    struct st_location_t *location_copy = NULL;
    char *ref_identifier = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	referrer_entry,
	struct named_ref_referrer_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	location_copy,
	location,
	issues,
	error_free_resources);

    referrer_entry->referrer = referrer;
    referrer_entry->location = location_copy;
    referrer_entry->callback = callback;
    referrer_entry->secondary_callback = secondary_callback;
    
    struct named_ref_entry_t *found = NULL;
    HASH_FIND_STR(np->ref_table, identifier, found);
    if(found != NULL)
    {
	DL_APPEND(found->referrers, referrer_entry);
	DL_APPEND2(np->not_committed, referrer_entry, nc_prev, nc_next);
    }
    else
    {
	ALLOC_OR_ERROR_JUMP(
	    ref_entry,
	    struct named_ref_entry_t,
	    issues,
	    error_free_resources); 

	STRDUP_OR_ERROR_JUMP(
	    ref_identifier,
	    identifier,
	    issues,
	    error_free_resources);

	ref_entry->identifier = ref_identifier;
	ref_entry->referrers = NULL;
	ref_entry->target = NULL;
	ref_entry->remark = 0;
	
	DL_APPEND(ref_entry->referrers, referrer_entry);
	HASH_ADD_KEYPTR(
	    hh, 
	    np->ref_table, 
	    ref_entry->identifier, 
	    strlen(ref_entry->identifier), 
	    ref_entry);
	DL_APPEND2(np->not_committed, referrer_entry, nc_prev, nc_next);
    }

    return ESSTEE_OK;
    
error_free_resources:
    free(referrer);
    free(ref_entry);
    free(ref_identifier);
    free(location_copy);
    return ESSTEE_ERROR;
}

static int named_ref_pool_add(
    struct named_ref_pool_iface_t *self,
    const char *identifier,
    void *referrer,
    const struct st_location_t *location,
    resolved_callback_t callback,
    struct issues_iface_t *issues)
{
    return named_ref_pool_add_two_step(self,
				       identifier,
				       referrer,
				       location,
				       callback,
				       NULL,
				       issues);
}

static void named_ref_pool_commit(
    struct named_ref_pool_iface_t *self)
{
    struct named_ref_pool_t *np =
	CONTAINER_OF(self, struct named_ref_pool_t, named_ref_pool);

    np->not_committed = NULL;
}

static void named_ref_pool_clear(
    struct named_ref_pool_iface_t *self)
{
    struct named_ref_pool_t *np =
	CONTAINER_OF(self, struct named_ref_pool_t, named_ref_pool);

    struct named_ref_referrer_t *itr = NULL;
    DL_FOREACH(np->not_committed, itr)
    {
	itr->referrer = NULL;
	itr->callback = NULL;
	itr->secondary_callback = NULL;
    }

    np->not_committed = NULL;
}
    
static const char * named_ref_pool_next_unresolved(
    struct named_ref_pool_iface_t *self)
{
    struct named_ref_pool_t *np =
	CONTAINER_OF(self, struct named_ref_pool_t, named_ref_pool);
    
    while(1)
    {
	if(np->ref_table == NULL)
	{
	    break;
	}
	else if(np->iterator == NULL)
	{
	    np->iterator = np->ref_table;
	}
	else
	{
	    np->iterator = np->iterator->hh.next;
	}      

	if(!np->iterator)
	{
	    break;
	}
	else if(!np->iterator->target)
	{
	    return np->iterator->identifier;
	}
    }

    return NULL;
}

static int named_ref_pool_resolve_with_remark(
    struct named_ref_pool_iface_t *self,
    const char *identifier,
    void *target,
    st_bitflag_t remark)
{
    struct named_ref_pool_t *np =
	CONTAINER_OF(self, struct named_ref_pool_t, named_ref_pool);

    struct named_ref_entry_t *found = NULL;

    HASH_FIND_STR(np->ref_table, identifier, found);
    if(!found)
    {
	return ESSTEE_ERROR;
    }

    found->target = target;
    found->remark = remark;

    return ESSTEE_OK;
}

static int named_ref_pool_resolve(
    struct named_ref_pool_iface_t *self,
    const char *identifier,
    void *target)
{
    return named_ref_pool_resolve_with_remark(
	self,
	identifier,
	target,
	0);
}

static int named_ref_pool_reset_resolved(
    struct named_ref_pool_iface_t *self)
{
    struct named_ref_pool_t *np =
	CONTAINER_OF(self, struct named_ref_pool_t, named_ref_pool);

    struct named_ref_entry_t *itr = NULL;

    for(itr = np->ref_table; itr != NULL; itr = itr->hh.next)
    {
	itr->target = NULL;
    }

    return ESSTEE_OK;
}

static int named_ref_pool_trigger_resolve_callbacks(
    struct named_ref_pool_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct named_ref_pool_t *np =
	CONTAINER_OF(self, struct named_ref_pool_t, named_ref_pool);

    struct named_ref_entry_t *ref_entry_itr = NULL;
    struct named_ref_referrer_t *referrer_itr  = NULL;

    int result = ESSTEE_OK;
    int referrer_result;

    /* First sweep */
    for(ref_entry_itr = np->ref_table; ref_entry_itr != NULL; ref_entry_itr = ref_entry_itr->hh.next)
    {
	DL_FOREACH(ref_entry_itr->referrers, referrer_itr)
	{
	    referrer_result = ESSTEE_OK;
	    if(referrer_itr->referrer && referrer_itr->callback)
	    {
		referrer_result = referrer_itr->callback(
		    referrer_itr->referrer,
		    ref_entry_itr->target,
		    ref_entry_itr->remark,
		    ref_entry_itr->identifier,
		    referrer_itr->location,
		    config,
		    issues);
	    }

	    if(referrer_result == ESSTEE_ERROR)
	    {
		result = ESSTEE_ERROR;
	    }
	}	
    }

    /* Secondary sweep */
    for(ref_entry_itr = np->ref_table; ref_entry_itr != NULL; ref_entry_itr = ref_entry_itr->hh.next)
    {
	DL_FOREACH(ref_entry_itr->referrers, referrer_itr)
	{
	    referrer_result = ESSTEE_OK;
	    if(referrer_itr->referrer && referrer_itr->secondary_callback)
	    {
		referrer_result = referrer_itr->secondary_callback(
		    referrer_itr->referrer,
		    ref_entry_itr->target,
		    ref_entry_itr->remark,
		    ref_entry_itr->identifier,
		    referrer_itr->location,
		    config,
		    issues);
	    }

	    if(referrer_result == ESSTEE_ERROR)
	    {
		result = ESSTEE_ERROR;
	    }
	}	
    }
    
    return result;
}
    
static struct named_ref_pool_iface_t * named_ref_pool_merge(
    struct named_ref_pool_iface_t *self,
    struct named_ref_pool_iface_t *to_merge)
{
    struct named_ref_pool_t *np =
	CONTAINER_OF(self, struct named_ref_pool_t, named_ref_pool);
    
    struct named_ref_pool_t *np_to_merge =
	CONTAINER_OF(to_merge, struct named_ref_pool_t, named_ref_pool);

    struct named_ref_entry_t *itr = NULL, *found = NULL, *tmp;
    HASH_ITER(hh, np_to_merge->ref_table, itr, tmp)
    {
	HASH_FIND_STR(np->ref_table, itr->identifier, found);

	if(found)
	{
	    DL_CONCAT(found->referrers, itr->referrers);
	    free(itr->identifier);
	    free(itr);
	}
	else
	{
	    HASH_ADD_KEYPTR(
		hh, 
		np->ref_table, 
		itr->identifier, 
		strlen(itr->identifier), 
		itr);
	}
    }

    np_to_merge->ref_table = NULL;
    np_to_merge->not_committed = NULL;
    np_to_merge->iterator = NULL;
    
    return &(np->named_ref_pool);
}

static void named_ref_pool_destroy(
    struct named_ref_pool_iface_t *self)
{
    struct named_ref_pool_t *np =
	CONTAINER_OF(self, struct named_ref_pool_t, named_ref_pool);

    struct named_ref_entry_t *ref_entry_itr = NULL, *tmp = NULL;
    struct named_ref_referrer_t *referrer_itr  = NULL, *tmp_r = NULL;

    HASH_ITER(hh, np->ref_table, ref_entry_itr, tmp)
    {
	DL_FOREACH_SAFE(ref_entry_itr->referrers, referrer_itr, tmp_r)
	{
	    free(referrer_itr);
	}

	free(ref_entry_itr->identifier);
	free(ref_entry_itr);
    }
}

struct named_ref_pool_iface_t * st_new_named_ref_pool(
    struct issues_iface_t *issues)
{
    struct named_ref_pool_t *np = NULL;
    ALLOC_OR_ERROR_JUMP(
	np,
	struct named_ref_pool_t,
	issues,
	error_free_resources);

    np->ref_table = NULL;
    np->not_committed = NULL;
    np->iterator = NULL;
    
    np->named_ref_pool.add_two_step = named_ref_pool_add_two_step;
    np->named_ref_pool.add = named_ref_pool_add;
    np->named_ref_pool.clear = named_ref_pool_clear;
    np->named_ref_pool.commit = named_ref_pool_commit;
    np->named_ref_pool.merge = named_ref_pool_merge;
    np->named_ref_pool.next_unresolved = named_ref_pool_next_unresolved;
    np->named_ref_pool.reset_resolved = named_ref_pool_reset_resolved;
    np->named_ref_pool.resolve = named_ref_pool_resolve;
    np->named_ref_pool.resolve_with_remark = named_ref_pool_resolve_with_remark;
    np->named_ref_pool.trigger_resolve_callbacks = named_ref_pool_trigger_resolve_callbacks;
    np->named_ref_pool.destroy = named_ref_pool_destroy;

    return &(np->named_ref_pool);
    
error_free_resources:
    free(np);
    return NULL;
}
