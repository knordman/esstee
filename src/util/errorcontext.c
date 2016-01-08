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

#include <util/errorcontext.h>
#include <util/macros.h>
#include <esstee/flags.h>
#include <esstee/locations.h>

#include <utlist.h>
#include <stdarg.h>
#include <stdio.h>

#define ALLOC_ISSUE_MAX_LEN 500
#define INTERNAL_ERROR_FORMAT "an internal error, sorry. (%s, function %s, line %d)"
#define MEMORY_ERROR_FORMAT "out of memory. (%s, function %s, line %d)"

struct errors_iface_t * st_new_error_context(void)
{
    struct error_context_t *ec = NULL;
    struct issue_node_t *internal_error = NULL;
    struct issue_node_t *memory_error = NULL;
    char *internal_error_description = NULL;
    char *memory_error_description = NULL;
    
    ALLOC_OR_JUMP(
	ec,
	struct error_context_t,
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
    
    internal_error->issue.message = internal_error_description;
    internal_error->issue.class = ISSUE_ERROR_CLASS;
    internal_error->issue.locations = NULL;
    internal_error->prev = NULL;
    internal_error->next = NULL;
    
    memory_error->issue.message = memory_error_description;
    memory_error->issue.class = ISSUE_ERROR_CLASS;
    memory_error->issue.locations = NULL;
    memory_error->prev = NULL;
    memory_error->next = NULL;

    ec->internal_error = internal_error;
    ec->internal_error_added = 0;
    ec->memory_error = memory_error;
    ec->memory_error_added = 0;
    ec->iterator = NULL;
    ec->issues = NULL;
    ec->error_count = 0;
    ec->error_count_last_check = 0;
    
    ec->errors.new_issue = st_error_context_new_issue;
    ec->errors.new_issue_at = st_error_context_new_issue_at;
    ec->errors.memory_error = st_error_context_new_memory_error;
    ec->errors.internal_error = st_error_context_new_internal_error;
    ec->errors.next_issue = st_error_context_next_issue;
    ec->errors.merge = st_error_context_merge;
    ec->errors.reset = st_error_context_reset;
    ec->errors.new_error_occured = st_error_context_new_error_occured;
    ec->errors.error_count = st_error_context_error_count;
    
    return &(ec->errors);

error_free_resources:
    free(ec);
    free(internal_error);
    free(memory_error);
    free(internal_error_description);
    free(memory_error_description);
    return NULL;
}

void st_destroy_error_context(
    struct errors_iface_t *error_context)
{
    struct error_context_t *ec =
	CONTAINER_OF(error_context, struct error_context_t, errors);

    struct issue_node_t *itr = NULL, *tmp = NULL;
    DL_FOREACH_SAFE(ec->issues, itr, tmp)
    {
	free(itr);
    }

    if(ec->internal_error)
    {
	free(ec->internal_error);
    }

    if(ec->memory_error)
    {
	free(ec->memory_error);
    }

    free(ec);
}

void st_error_context_new_issue(
    struct errors_iface_t *self,
    const char *message,
    st_bitflag_t issue_class)
{
    struct error_context_t *ec = CONTAINER_OF(self, struct error_context_t, errors);

    struct issue_node_t *in = NULL;
    ALLOC_OR_JUMP(in, struct issue_node_t, error_free_resources);
    
    char *issue_message = NULL;
    STRDUP_OR_JUMP(issue_message, message, error_free_resources);

    in->issue.message = issue_message;
    in->issue.class = issue_class;
    in->issue.locations = NULL;
    ec->error_count += 1;
    DL_APPEND(ec->issues, in);

    return;

error_free_resources:
    free(in);
    free(issue_message);
}

void st_error_context_new_issue_at(
    struct errors_iface_t *self,
    const char *message,
    st_bitflag_t issue_class,
    int location_count,
    ...)
{
    struct error_context_t *ec = CONTAINER_OF(self, struct error_context_t, errors);

    struct issue_node_t *in = NULL;
    ALLOC_OR_ERROR_JUMP(in, struct issue_node_t, self, error_free_resources);
    
    char *issue_message = NULL;
    STRDUP_OR_ERROR_JUMP(issue_message, message, self, error_free_resources);

    va_list ap;
    const struct st_location_t *l = NULL;
    struct st_location_t *locations = NULL, *copy = NULL;

    va_start(ap, location_count);
    for(int i = 0; i < location_count; i++)
    {
	l = va_arg(ap, const struct st_location_t *);		

	LOCDUP_OR_JUMP(
	    copy,
	    l,
	    error_free_resources);

	DL_APPEND(locations, copy);
    }
    va_end(ap);

    in->issue.message = issue_message;
    in->issue.class = issue_class;
    in->issue.locations = locations;
    DL_APPEND(ec->issues, in);    
    ec->error_count += 1;
    
    return;
    
error_free_resources:
    free(in);
    free(issue_message);
}

void st_error_context_new_memory_error(
    struct errors_iface_t *self,
    const char *file,
    const char *function,
    int line)
{
    struct error_context_t *ec = CONTAINER_OF(self, struct error_context_t, errors);

    if(!ec->memory_error_added)
    {
	snprintf(ec->memory_error->issue.message,
		 ALLOC_ISSUE_MAX_LEN,
		 MEMORY_ERROR_FORMAT, 
		 file,
		 function,
		 line);

	DL_APPEND(ec->issues, ec->memory_error);
	ec->memory_error_added = 1;
	ec->error_count += 1;
    }
}

void st_error_context_new_internal_error(
    struct errors_iface_t *self,
    const char *file,
    const char *function,
    int line)
{
    struct error_context_t *ec = CONTAINER_OF(self, struct error_context_t, errors);

    if(!ec->internal_error_added)
    {
	snprintf(ec->internal_error->issue.message,
		 ALLOC_ISSUE_MAX_LEN,
		 INTERNAL_ERROR_FORMAT, 
		 file,
		 function,
		 line);

	DL_APPEND(ec->issues, ec->internal_error);
	ec->internal_error_added = 1;
	ec->error_count += 1;
    }
}

const struct st_issue_t * st_error_context_next_issue(
    struct errors_iface_t *self,
    st_bitflag_t issue_filter)
{
    struct error_context_t *ec = CONTAINER_OF(self, struct error_context_t, errors);

    static int insert_end_marker = 0;

    if(insert_end_marker)
    {
	insert_end_marker = 0;
	return NULL;
    }
    else if(ec->iterator == NULL)
    {
	ec->iterator = ec->issues;
    }
    else
    {
	if(ec->iterator->next == NULL)
	{
	    insert_end_marker = 1;
	}
	ec->iterator = ec->iterator->next;
    }

    return &(ec->iterator->issue);
}

struct errors_iface_t * st_error_context_merge(
    struct errors_iface_t *self,
    struct errors_iface_t *to_merge)
{
    struct error_context_t *ec = CONTAINER_OF(self, struct error_context_t, errors);
    struct error_context_t *ecm = CONTAINER_OF(to_merge, struct error_context_t, errors);

    if(ecm->internal_error_added)
    {
	ecm->internal_error = NULL;
    }

    if(ecm->memory_error_added)
    {
	ecm->memory_error = NULL;
    }
    
    DL_CONCAT(ec->issues, ecm->issues);
    ec->error_count += ecm->error_count;

    ecm->issues = NULL;
    ecm->error_count = 0;
    ecm->error_count_last_check = 0;
    
    return &(ec->errors);
}

int st_error_context_reset(
    struct errors_iface_t *self)
{
    struct error_context_t *ec
	= CONTAINER_OF(self, struct error_context_t, errors);    

    ec->issues = NULL;
    ec->error_count = 0;

    return ESSTEE_OK;
}

int st_error_context_new_error_occured(
    struct errors_iface_t *self)
{
    struct error_context_t *ec
	= CONTAINER_OF(self, struct error_context_t, errors);    

    if(ec->error_count > ec->error_count_last_check)
    {
	ec->error_count_last_check = ec->error_count;
	return ESSTEE_TRUE;
    }
    
    return ESSTEE_FALSE;
}

int st_error_context_error_count(
    struct errors_iface_t *self)
{
    struct error_context_t *ec
	= CONTAINER_OF(self, struct error_context_t, errors);    

    return ec->error_count;
}
