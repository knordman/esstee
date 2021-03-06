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

#include <util/issue_context.h>
#include <util/macros.h>
#include <esstee/issues.h>
#include <esstee/flags.h>
#include <esstee/locations.h>

#include <utlist.h>
#include <stdarg.h>
#include <stdio.h>

#define ALLOC_ISSUE_MAX_LEN 500
#define INTERNAL_ERROR_FORMAT "an internal error, sorry. (%s, function %s, line %d)"
#define MEMORY_ERROR_FORMAT "out of memory. (%s, function %s, line %d)"

struct issue_node_t {
    struct st_issue_t issue;
    int new;
    struct issue_node_t *sub_nodes;
    struct issue_node_t *prev;
    struct issue_node_t *next;
};

struct issue_group_t {
    struct issue_group_iface_t group;
    struct issue_node_t *nodes;
    struct issue_context_t *context;
};

struct issue_context_t {
    struct issues_iface_t issues;

    struct issue_node_t *nodes;
    struct issue_group_t *group;
    int group_mode;
    struct issue_node_t *internal_error;
    struct issue_node_t *memory_error;

    int fatal_error;
    int internal_error_added;
    int memory_error_added;
    char *message_buffer;
    struct issue_node_t *iterator;
    st_bitflag_t last_filter;
};

/**************************************************************************/
/* Help functions                                                         */
/**************************************************************************/
static void destroy_issue_nodes(
    struct issue_node_t *nodes)
{
    struct issue_node_t *node_itr = NULL;
    struct issue_node_t *node_tmp = NULL;
    DL_FOREACH_SAFE(nodes, node_itr, node_tmp)
    {
	struct st_location_t *location_itr = NULL;
	struct st_location_t *location_tmp = NULL;
	DL_FOREACH_SAFE(node_itr->issue.locations, location_itr, location_tmp)
	{
	    free(location_itr);
	}
	
	free(node_itr->issue.message);
	free(node_itr);
    }
}

/**************************************************************************/
/* Issue group interface                                                  */
/**************************************************************************/
static void issue_group_close(
    struct issue_group_iface_t *self)
{
    struct issue_group_t *group =
	CONTAINER_OF(self, struct issue_group_t, group);

    group->context->group_mode = 0;
}

static void issue_group_main_issue(
    struct issue_group_iface_t *self,
    const char *message,
    st_bitflag_t issue_class,
    int location_count,
    ...)
{ 
    struct issue_group_t *group =
	CONTAINER_OF(self, struct issue_group_t, group);

    /* Create a new node with locations*/
    struct issue_node_t *in = NULL;
    char *issue_message = NULL;
    struct st_location_t *locations = NULL;
    struct st_location_t *in_location = NULL;
    
    ALLOC_OR_JUMP(
	in,
	struct issue_node_t,
	error_free_resources);

    STRDUP_OR_JUMP(
	issue_message,
	message,
	error_free_resources);

    va_list ap;
    const struct st_location_t *location = NULL;
    va_start(ap, location_count);
    for(int i = 0; i < location_count; i++)
    {
	location = va_arg(ap, const struct st_location_t *);		

	LOCDUP_OR_JUMP(
	    in_location,
	    location,
	    location_alloc_error);

	DL_APPEND(locations, in_location);
	continue;

    location_alloc_error:
	va_end(ap);
	goto error_free_resources;
    }
    va_end(ap);

    in->issue.message = issue_message;
    in->issue.class = issue_class;
    in->issue.locations = locations;
    in->new = 1;

    /* Add group nodes as sub issues to new node */    
    in->sub_nodes = group->context->group->nodes;

    if(in->sub_nodes)
    {
	in->issue.has_sub_issues = ESSTEE_TRUE;
    }

    /* Reset group */
    group->context->group->nodes = NULL;

    /* Add to issue context nodes */
    DL_APPEND(group->context->nodes, in);
    
    return;
    
error_free_resources:
    free(in);
    free(issue_message);
    struct st_location_t *tmp = NULL;
    struct st_location_t *itr = NULL;
    DL_FOREACH_SAFE(locations, itr, tmp)
    {
	free(itr);
    }
    destroy_issue_nodes(group->context->group->nodes);
    group->context->group->nodes = NULL;
}

/**************************************************************************/
/* Issues interface                                                       */
/**************************************************************************/
static void issue_context_new_issue(
    struct issues_iface_t *self,
    const char *format,
    st_bitflag_t issue_class,
    ...)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);

    struct issue_node_t *in = NULL;
    char *issue_message = NULL;
    
    va_list ap;
    va_start(ap, issue_class);
    int written_bytes = vsnprintf(ic->message_buffer,
				  ALLOC_ISSUE_MAX_LEN,
				  format,
				  ap);
    va_end(ap);
    
    if(written_bytes < 1)
    {
	goto error_free_resources;
    }
    

    ALLOC_OR_JUMP(
	in,
	struct issue_node_t,
	error_free_resources);

    STRDUP_OR_JUMP(
	issue_message,
	ic->message_buffer,
	error_free_resources);

    in->issue.message = issue_message;
    in->issue.class = issue_class;
    in->issue.locations = NULL;
    in->new = 1;
    in->sub_nodes = NULL;

    if(ic->group_mode)
    {
	DL_APPEND(ic->group->nodes, in);	
    }
    else
    {
	DL_APPEND(ic->nodes, in);
    }

    return;

error_free_resources:
    free(in);
    free(issue_message);    
}

static void issue_context_new_issue_at(
    struct issues_iface_t *self,
    const char *message,
    st_bitflag_t issue_class,
    int location_count,
    ...)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);

    struct issue_node_t *in = NULL;
    char *issue_message = NULL;
    struct st_location_t *locations = NULL;
    struct st_location_t *in_location = NULL;
    
    ALLOC_OR_JUMP(
	in,
	struct issue_node_t,
	error_free_resources);

    STRDUP_OR_JUMP(
	issue_message,
	message,
	error_free_resources);

    va_list ap;
    const struct st_location_t *location = NULL;
    va_start(ap, location_count);
    for(int i = 0; i < location_count; i++)
    {
	location = va_arg(ap, const struct st_location_t *);		

	LOCDUP_OR_JUMP(
	    in_location,
	    location,
	    location_alloc_error);

	DL_APPEND(locations, in_location);
	continue;

    location_alloc_error:
	va_end(ap);
	goto error_free_resources;
    }
    va_end(ap);

    in->issue.message = issue_message;
    in->issue.class = issue_class;
    in->issue.locations = locations;
    in->new = 1;
    in->sub_nodes = NULL;
    DL_APPEND(ic->nodes, in);

    return;
    
error_free_resources:
    free(in);
    free(issue_message);
    struct st_location_t *tmp = NULL;
    struct st_location_t *itr = NULL;
    DL_FOREACH_SAFE(locations, itr, tmp)
    {
	free(itr);
    }
}

static const char * issue_context_build_message(
    struct issues_iface_t *self,
    const char *format,
    ...)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);

    va_list ap;
    va_start(ap, format);
    int written_bytes = vsnprintf(ic->message_buffer,
				  ALLOC_ISSUE_MAX_LEN,
				  format,
				  ap);
    va_end(ap);

    if(written_bytes < 0)
    {
	return NULL;
    }
    
    return ic->message_buffer;
}
        
static void issue_context_memory_error(
    struct issues_iface_t *self,
    const char *file,
    const char *function,
    int line)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);

    ic->fatal_error = ESSTEE_TRUE;
    
    if(!ic->memory_error_added)
    {
	snprintf(ic->memory_error->issue.message,
		 ALLOC_ISSUE_MAX_LEN,
		 MEMORY_ERROR_FORMAT, 
		 file,
		 function,
		 line);

	DL_APPEND(ic->nodes, ic->memory_error);
	ic->memory_error_added = 1;
    }
}

static void issue_context_internal_error(
    struct issues_iface_t *self,
    const char *file,
    const char *function,
    int line)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);

    ic->fatal_error = ESSTEE_TRUE;
    
    if(!ic->internal_error_added)
    {
	snprintf(ic->internal_error->issue.message,
		 ALLOC_ISSUE_MAX_LEN,
		 INTERNAL_ERROR_FORMAT, 
		 file,
		 function,
		 line);

	DL_APPEND(ic->nodes, ic->internal_error);
	ic->internal_error_added = 1;
    }
}

static struct issue_group_iface_t * issue_context_open_group(
    struct issues_iface_t *self)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);

    ic->group_mode = 1;
    return &(ic->group->group);
}

static void issue_context_ignore_all(
    struct issues_iface_t *self)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);

    struct issue_node_t *itr = NULL;
    DL_FOREACH(ic->nodes, itr)
    {
	itr->new = 0;
    }
}
        
static const struct st_issue_t * issue_context_fetch(
    struct issues_iface_t *self,
    st_bitflag_t issue_filter)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);

    if(ic->iterator == NULL || issue_filter != ic->last_filter)
    {
	ic->iterator = ic->nodes;
    }

    ic->last_filter = issue_filter;
    
    /* Find the first error not returned that matches the filter */
    for(; ic->iterator != NULL; ic->iterator = ic->iterator->next)
    {
	if(!ic->iterator->new)
	{
	    continue;
	}

	if(ST_FLAG_IS_SET(ic->iterator->issue.class, issue_filter))
	{
	    ic->iterator->new = 0;
	    return &(ic->iterator->issue);
	}
    }

    return NULL;
}

static const struct st_issue_t * issue_context_fetch_sub_issue(
    struct issues_iface_t *self,
    const struct st_issue_t *issue,
    st_bitflag_t filter)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);

    struct issue_node_t *issue_node =
	CONTAINER_OF(issue, struct issue_node_t, issue);

    struct issue_node_t *itr = NULL;
    DL_FOREACH(issue_node->sub_nodes, itr)
    {
	if(!itr->new)
	{
	    continue;
	}
	
	if(ST_FLAG_IS_SET(ic->iterator->issue.class, filter))
	{
	    itr->new = 0;
	    return &(itr->issue);
	}
    }

    return NULL;
}
    
static const struct st_issue_t * issue_context_fetch_and_ignore(
    struct issues_iface_t *self,
    st_bitflag_t issue_filter)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);

    if(ic->iterator == NULL || issue_filter != ic->last_filter)
    {
	ic->iterator = ic->nodes;
    }

    ic->last_filter = issue_filter;
    
    struct issue_node_t *result = NULL;
    for(; ic->iterator != NULL; ic->iterator = ic->iterator->next)
    {
	if(!ic->iterator->new)
	{
	    continue;
	}

	if(ST_FLAG_IS_SET(ic->iterator->issue.class, issue_filter))
	{
	    ic->iterator->new = 0;
	    if(!result)
	    {
		result = ic->iterator;
	    }
	}
    }

    ic->iterator = result;
    return &(ic->iterator->issue);
}

static struct issues_iface_t * issue_context_merge(
    struct issues_iface_t *self,
    struct issues_iface_t *to_merge)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);
    
    struct issue_context_t *ic_to_merge =
	CONTAINER_OF(to_merge, struct issue_context_t, issues);

    /* Avoid double ownership */
    if(ic_to_merge->internal_error_added)
    {
	ic_to_merge->internal_error = NULL;
    }

    if(ic_to_merge->memory_error_added)
    {
	ic_to_merge->memory_error = NULL;
    }

    /* If merge context has the fatal flag set, also set in the result
     * context */
    if(ic_to_merge->fatal_error == ESSTEE_TRUE)
    {
	ic->fatal_error = ESSTEE_TRUE;
    }
    
    DL_CONCAT(ic->nodes, ic_to_merge->nodes);

    ic_to_merge->nodes = NULL;

    return self;
}

static int issue_context_unfetched_issues(
    struct issues_iface_t *self,
    st_bitflag_t issue_filter)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);

    struct issue_node_t *itr = NULL;
    DL_FOREACH(ic->nodes, itr)
    {
	if(!itr->new)
	{
	    continue;
	}

	if(ST_FLAG_IS_SET(itr->issue.class, issue_filter))
	{
	    return ESSTEE_TRUE;
	}
    }

    return ESSTEE_FALSE;
}

static int issue_context_count(
    struct issues_iface_t *self,
    st_bitflag_t issue_filter)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);

    struct issue_node_t *itr = NULL;
    int count = 0;
    DL_FOREACH(ic->nodes, itr)
    {
	if(ST_FLAG_IS_SET(itr->issue.class, issue_filter))
	{
	    count++;
	}
    }

    return count;
}

static int issue_context_fatal_error_occurred(
    struct issues_iface_t *self)
{
    struct issue_context_t *ic =
	CONTAINER_OF(self, struct issue_context_t, issues);

    return ic->fatal_error;
}

static void issue_context_destroy(
    struct issues_iface_t *self,
    st_bitflag_t issue_filter)
{
    /* TODO: issues destructor */
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct issues_iface_t * st_new_issue_context()
{
    struct issue_context_t *ic = NULL;
    struct issue_node_t *internal_error = NULL;
    struct issue_node_t *memory_error = NULL;
    char *internal_error_description = NULL;
    char *memory_error_description = NULL;
    char *message_buffer = NULL;
    struct issue_group_t *group = NULL;	
    
    ALLOC_OR_JUMP(
	ic,
	struct issue_context_t,
	error_free_resources);
    
    ALLOC_OR_JUMP(
	internal_error,
	struct issue_node_t,
	error_free_resources);
    
    ALLOC_OR_JUMP(
	memory_error,
	struct issue_node_t,
	error_free_resources);
    
    ALLOC_ARRAY_OR_JUMP(
	internal_error_description,
	char,
	ALLOC_ISSUE_MAX_LEN,
	error_free_resources);
    
    ALLOC_ARRAY_OR_JUMP(
	memory_error_description,
	char,
	ALLOC_ISSUE_MAX_LEN,
	error_free_resources);

    ALLOC_ARRAY_OR_JUMP(
	message_buffer,
	char,
	ALLOC_ISSUE_MAX_LEN,
	error_free_resources);

    ALLOC_OR_JUMP(
	group,
	struct issue_group_t,
	error_free_resources);

    group->nodes = NULL;
    group->context = ic;
    
    memset(&(group->group), 0, sizeof(struct issue_group_iface_t));
    group->group.close = issue_group_close;
    group->group.main_issue = issue_group_main_issue;
    
    internal_error->issue.message = internal_error_description;
    internal_error->issue.class = ESSTEE_MEMORY_ERROR;
    internal_error->issue.locations = NULL;
    internal_error->prev = NULL;
    internal_error->next = NULL;
    
    memory_error->issue.message = memory_error_description;
    memory_error->issue.class = ESSTEE_INTERNAL_ERROR;
    memory_error->issue.locations = NULL;
    memory_error->prev = NULL;
    memory_error->next = NULL;

    ic->internal_error = internal_error;
    ic->internal_error_added = 0;
    ic->memory_error = memory_error;
    ic->memory_error_added = 0;
    ic->iterator = NULL;
    ic->nodes = NULL;
    ic->message_buffer = message_buffer;
    ic->group_mode = 0;
    ic->group = group;
    ic->fatal_error = ESSTEE_FALSE;
    
    ic->issues.new_issue = issue_context_new_issue;
    ic->issues.new_issue_at = issue_context_new_issue_at;
    ic->issues.build_message = issue_context_build_message;
    ic->issues.memory_error = issue_context_memory_error;
    ic->issues.internal_error = issue_context_internal_error;
    ic->issues.open_group = issue_context_open_group;
    ic->issues.ignore_all = issue_context_ignore_all;
    ic->issues.fetch = issue_context_fetch;
    ic->issues.fetch_sub_issue = issue_context_fetch_sub_issue;
    ic->issues.fetch_and_ignore = issue_context_fetch_and_ignore;
    ic->issues.merge = issue_context_merge;
    ic->issues.unfetched_issues = issue_context_unfetched_issues;
    ic->issues.count = issue_context_count;
    ic->issues.fatal_error_occurred = issue_context_fatal_error_occurred;
    ic->issues.destroy = issue_context_destroy;
    
    return &(ic->issues);

error_free_resources:
    free(ic);
    free(internal_error);
    free(memory_error);
    free(internal_error_description);
    free(memory_error_description);
    free(group);
    return NULL;
}
