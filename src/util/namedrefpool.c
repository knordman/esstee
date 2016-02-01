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

#include <util/namedrefpool.h>
#include <util/macros.h>
#include <esstee/flags.h>

#include <utlist.h>


struct namedreference_iface_t * st_new_named_ref_pool(void)
{
    struct named_ref_pool_t *np = NULL;
    ALLOC_OR_JUMP(np, struct named_ref_pool_t, error_free_resources);

    np->ref_table = NULL;
    np->not_committed = NULL;
    np->iterator = NULL;
    
    np->named_ref.add_two_step = st_named_ref_pool_add_two_step_ref;
    np->named_ref.add = st_named_ref_pool_add_ref;
    np->named_ref.clear = st_named_ref_pool_clear;
    np->named_ref.commit = st_named_ref_pool_commit;
    np->named_ref.merge = st_named_ref_pool_merge;
    np->named_ref.next_unresolved = st_named_ref_pool_next_unresolved;
    np->named_ref.reset_resolved = st_named_ref_pool_reset_resolved;
    np->named_ref.resolve = st_named_ref_pool_resolve;
    np->named_ref.resolve_with_remark = st_named_ref_pool_resolve_with_remark;
    np->named_ref.trigger_resolve_callbacks = st_named_ref_pool_trigger_resolve_callbacks;

    return &(np->named_ref);
    
error_free_resources:
    free(np);
    return NULL;
}

void st_destroy_named_ref_pool(
    struct namedreference_iface_t *self)
{
    struct named_ref_pool_t *np = CONTAINER_OF(self, struct named_ref_pool_t, named_ref);

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

int st_named_ref_pool_add_two_step_ref(
    struct namedreference_iface_t *self,
    const char *identifier,
    void *referrer,
    void *subreferrer,
    const struct st_location_t *location,
    resolved_callback_t callback,
    resolved_callback_t secondary_callback)
{
    struct named_ref_pool_t *np
	= CONTAINER_OF(self, struct named_ref_pool_t, named_ref);

    struct named_ref_referrer_t *referrer_entry = NULL;
    struct named_ref_entry_t *ref_entry = NULL;
    char *ref_identifier = NULL;
    
    ALLOC_OR_JUMP(
	referrer_entry,
	struct named_ref_referrer_t,
	error_free_resources);

    referrer_entry->referrer = referrer;
    referrer_entry->subreferrer = subreferrer;
    memcpy(
	&(referrer_entry->location),
	location,
	sizeof(struct st_location_t));
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
	ALLOC_OR_JUMP(
	    ref_entry,
	    struct named_ref_entry_t,
	    error_free_resources); 

	STRDUP_OR_JUMP(
	    ref_identifier,
	    identifier,
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
    return ESSTEE_ERROR;
}


int st_named_ref_pool_add_ref(
    struct namedreference_iface_t *self,
    const char *identifier,
    void *referrer,
    void *subreferrer,
    const struct st_location_t *location,
    resolved_callback_t callback)
{
    return st_named_ref_pool_add_two_step_ref(
	self,
	identifier,
	referrer,
	subreferrer,
	location,
	callback,
	NULL);
}

int st_named_ref_pool_commit(
    struct namedreference_iface_t *self)
{
    struct named_ref_pool_t *np = CONTAINER_OF(self, struct named_ref_pool_t, named_ref);

    np->not_committed = NULL;

    return ESSTEE_OK;
}

int st_named_ref_pool_clear(
    struct namedreference_iface_t *self)
{
    struct named_ref_pool_t *np = CONTAINER_OF(self, struct named_ref_pool_t, named_ref);

    struct named_ref_referrer_t *itr = NULL;
    DL_FOREACH(np->not_committed, itr)
    {
	itr->referrer = NULL;
	itr->subreferrer = NULL;
	itr->callback = NULL;
    }

    np->not_committed = NULL;

    return ESSTEE_OK;
}
    
const char * st_named_ref_pool_next_unresolved(
    struct namedreference_iface_t *self)
{
    struct named_ref_pool_t *np = CONTAINER_OF(self, struct named_ref_pool_t, named_ref);
    
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

int st_named_ref_pool_resolve(
    struct namedreference_iface_t *self,
    const char *identifier,
    void *target)
{
    return st_named_ref_pool_resolve_with_remark(
	self,
	identifier,
	target,
	0);
}

int st_named_ref_pool_resolve_with_remark(
    struct namedreference_iface_t *self,
    const char *identifier,
    void *target,
    st_bitflag_t remark)
{
    struct named_ref_pool_t *np
	= CONTAINER_OF(self, struct named_ref_pool_t, named_ref);

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

int st_named_ref_pool_reset_resolved(
    struct namedreference_iface_t *self)
{
    struct named_ref_pool_t *np = CONTAINER_OF(self, struct named_ref_pool_t, named_ref);

    struct named_ref_entry_t *itr = NULL;

    for(itr = np->ref_table; itr != NULL; itr = itr->hh.next)
    {
	itr->target = NULL;
    }

    return ESSTEE_OK;
}

int st_named_ref_pool_trigger_resolve_callbacks(
    struct namedreference_iface_t *self,
    struct errors_iface_t *err)
{
    struct named_ref_pool_t *np = CONTAINER_OF(self, struct named_ref_pool_t, named_ref);

    struct named_ref_entry_t *ref_entry_itr = NULL;
    struct named_ref_referrer_t *referrer_itr  = NULL;

    int result = ESSTEE_OK, referrer_result;

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
		    referrer_itr->subreferrer,
		    ref_entry_itr->target,
		    ref_entry_itr->remark,
		    &(referrer_itr->location),
		    err);
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
		    referrer_itr->subreferrer,
		    ref_entry_itr->target,
		    ref_entry_itr->remark,
		    &(referrer_itr->location),
		    err);
	    }

	    if(referrer_result == ESSTEE_ERROR)
	    {
		result = ESSTEE_ERROR;
	    }
	}	
    }
    
    return result;
}
    
struct namedreference_iface_t * st_named_ref_pool_merge(
    struct namedreference_iface_t *self,
    struct namedreference_iface_t *to_merge)
{
    struct named_ref_pool_t *np = CONTAINER_OF(self, struct named_ref_pool_t, named_ref);
    struct named_ref_pool_t *npm = CONTAINER_OF(to_merge, struct named_ref_pool_t, named_ref);

    struct named_ref_entry_t *itr = NULL, *found = NULL, *tmp;
    HASH_ITER(hh, npm->ref_table, itr, tmp)
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

    npm->ref_table = NULL;
    npm->not_committed = NULL;
    npm->iterator = NULL;
    
    return &(np->named_ref);
}
