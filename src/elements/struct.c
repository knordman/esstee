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

#include <elements/struct.h>
#include <elements/variables.h>
#include <elements/values.h>
#include <util/macros.h>

#include <uthash.h>
#include <stdio.h>

struct struct_element_t {
    char *element_identifier;
    struct st_location_t *identifier_location;
    struct type_iface_t *element_type;
    UT_hash_handle hh;
};

struct struct_element_init_t {
    char *element_identifier;
    struct value_iface_t *element_default_value;
    struct st_location_t *element_identifier_location;
    UT_hash_handle hh;
};

/**************************************************************************/
/* Value interface                                                        */
/**************************************************************************/
struct struct_init_value_t {
    struct value_iface_t value;
    struct struct_element_init_t *init_table;
};

const struct struct_init_value_t * struct_init_value(
    const struct value_iface_t *self)
{
    const struct struct_init_value_t *isv =
	CONTAINER_OF(self, struct struct_init_value_t, value);

    return isv;
}

static void struct_init_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: struct init value destructor */
}

struct struct_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    struct variable_t *elements;
};

static int struct_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    size_t buffer_size_start = buffer_size;

    int start_written_bytes = snprintf(buffer,
				       buffer_size,
				       "(");
    CHECK_WRITTEN_BYTES(start_written_bytes);
    buffer += start_written_bytes;
    buffer_size -= start_written_bytes;

    struct variable_t *itr = NULL;
    int first_element = 1;
    for(itr = sv->elements; itr != NULL; itr = itr->hh.next)
    {
	int written_bytes = 0;
	
	if(first_element)
	{
	    first_element = 0;
	}
	else
	{
	    written_bytes = snprintf(buffer,
				     buffer_size,
				     ",");

	    CHECK_WRITTEN_BYTES(written_bytes);
	    buffer += written_bytes;
	    buffer_size -= written_bytes;
	}
	
	written_bytes = snprintf(buffer,
				 buffer_size,
				 "%s:",
				 itr->identifier);

	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;

	written_bytes = itr->value->display(itr->value,
					    buffer,
					    buffer_size,
					    config);
	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;
    }
    
    int end_written_bytes = snprintf(buffer,
				     buffer_size,
				     ")");
    CHECK_WRITTEN_BYTES(end_written_bytes);
    buffer += end_written_bytes;
    buffer_size -= end_written_bytes;

    return buffer_size_start - buffer_size;
}

static const struct type_iface_t * struct_value_type_of(
    const struct value_iface_t *self)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    return sv->type;
}

static int struct_value_assignable_from(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    int type_can_hold = sv->type->can_hold(sv->type, other_value, config, issues);

    if(type_can_hold != ESSTEE_TRUE)
    {
	return type_can_hold;
    }
    
    return ESSTEE_TRUE;
}

static int struct_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    const struct struct_init_value_t *isv =
	new_value->struct_init_value(new_value);

    struct struct_element_init_t *itr = NULL;
    for(itr = isv->init_table; itr != NULL; itr = itr->hh.next)
    {
	struct variable_t *found = NULL;
	HASH_FIND_STR(sv->elements, itr->element_identifier, found);
	if(!found)
	{
	    issues->internal_error(
		issues,
		__FILE__,
		__FUNCTION__,
		__LINE__);
	    
	    return ESSTEE_ERROR;
	}

	int assign_result = found->value->assign(found->value,
						 itr->element_default_value,
						 config,
						 issues);
	if(assign_result != ESSTEE_OK)
	{
	    return assign_result;
	}
    }

    return ESSTEE_OK;
}

int struct_value_override_type(
    const struct value_iface_t *self,
    const struct type_iface_t *type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    sv->type = type;

    return ESSTEE_OK;
}

static void struct_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: struct value destructor */
}

static struct variable_t * struct_value_sub_variable(
    struct value_iface_t *self,
    const char *identifier,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    struct variable_t *found = NULL;
    HASH_FIND_STR(sv->elements, identifier, found);
    if(!found)
    {
	issues->new_issue(
	    issues,
	    "struct has no member '%s'",
	    ESSTEE_ARGUMENT_ERROR,
	    identifier);

	return NULL;
    }
    
    return found;
}

/**************************************************************************/
/* Type interface                                                         */
/**************************************************************************/
struct struct_type_t {
    struct type_iface_t type;
    struct struct_element_t *elements;
};

static struct value_iface_t * struct_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct struct_type_t *st =
	CONTAINER_OF(self, struct struct_type_t, type);

    struct struct_value_t *sv = NULL;
    ALLOC_OR_ERROR_JUMP(
	sv,
	struct struct_value_t,
	issues,
	error_free_resources);

    sv->elements = NULL;
    struct struct_element_t *itr = NULL;
    for(itr = st->elements; itr != NULL; itr = itr->hh.next)
    {
	struct variable_t *ev = NULL;
	ALLOC_OR_JUMP(
	    ev,
	    struct variable_t,
	    error_free_resources);

	ev->identifier = itr->element_identifier;
	ev->location = NULL;

	ev->next = NULL;
	ev->prev = NULL;
	ev->address = NULL;
	
	ev->value = itr->element_type->create_value_of(itr->element_type,
						       config,
						       issues);
	if(!ev->value)
	{
	    goto error_free_resources;
	}

	ev->type = itr->element_type;

	HASH_ADD_KEYPTR(hh, 
			sv->elements, 
			ev->identifier, 
			strlen(ev->identifier), 
			ev);
    }

    sv->type = self;
    
    memset(&(sv->value), 0, sizeof(struct value_iface_t));
    sv->value.display = struct_value_display;
    sv->value.assign = struct_value_assign;
    sv->value.assignable_from = struct_value_assignable_from;
    sv->value.type_of = struct_value_type_of;
    sv->value.override_type = struct_value_override_type;
    sv->value.destroy = struct_value_destroy;
    sv->value.sub_variable = struct_value_sub_variable;
    sv->value.class = st_value_general_empty_class;

    return &(sv->value);
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

static int struct_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_value_t *sv =
	CONTAINER_OF(value_of, struct struct_value_t, value);

    struct variable_t *itr = NULL;
    for(itr = sv->elements; itr != NULL; itr = itr->hh.next)
    {
	int reset_result = itr->type->reset_value_of(itr->type, itr->value, config, issues);

	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    return ESSTEE_OK;
}

static int struct_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct struct_type_t *st =
	CONTAINER_OF(self, struct struct_type_t, type);

    if(!value->struct_init_value)
    {
	issues->new_issue(
	    issues,
	    "struct type can only be assigned a struct initializer",
	    ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }

    const struct struct_init_value_t *isv = value->struct_init_value(value);
    
    struct struct_element_init_t *itr = NULL;
    for(itr = isv->init_table; itr != NULL; itr = itr->hh.next)
    {
	/* Check that identifier is one of the members */
	struct struct_element_t *found = NULL;
	HASH_FIND_STR(st->elements, itr->element_identifier, found);
	if(!found)
	{
	    issues->new_issue(
		issues,
		"struct has no member '%s'",
		ESSTEE_TYPE_ERROR,
		itr->element_identifier);

	    return ESSTEE_FALSE;
	}

	/* Check that the member type can hold the value */
	int member_type_can_hold = found->element_type->can_hold(found->element_type,
								 itr->element_default_value,
								 config,
								 issues);
	if(member_type_can_hold != ESSTEE_TRUE)
	{
	    return member_type_can_hold;
	}
    }
    
    return ESSTEE_TRUE;
}

static st_bitflag_t struct_type_class(
    const struct type_iface_t *self)
{
    return STRUCT_TYPE;
}

static void struct_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: struct type destructor */
}

/**************************************************************************/
/* Linker callbacks                                                       */
/**************************************************************************/
int struct_element_type_name_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!target)
    {
	const char *message = issues->build_message(
	    issues,
	    "reference to undefined type '%s'",
	    identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_TYPE_ERROR,
	    1,
	    location);

	return ESSTEE_ERROR;
    }

    struct struct_element_t *se =
	(struct struct_element_t *)referrer;

    se->element_type = (struct type_iface_t *)target;
    
    return ESSTEE_OK;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct struct_element_t * st_extend_element_group(
    struct struct_element_t *element_group,
    char *element_identifier,
    const struct st_location_t *identifier_location,
    struct type_iface_t *element_type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_element_t *se = NULL;
    struct st_location_t *se_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	se,
	struct struct_element_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	se_location,
	identifier_location,
	issues,
	error_free_resources);

    se->element_type = element_type;
    se->element_identifier = element_identifier;
    se->identifier_location = se_location;

    struct struct_element_t *found = NULL;
    HASH_FIND_STR(element_group, se->element_identifier, found);
    if(found)
    {
	const char *message = issues->build_message(
	    issues,
	    "element '%s' is not unique",
	    se->element_identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_CONTEXT_ERROR,
	    2,
	    identifier_location,
	    found->identifier_location);

	goto error_free_resources;
    }
    
    HASH_ADD_KEYPTR(hh, 
		    element_group, 
		    se->element_identifier, 
		    strlen(se->element_identifier), 
		    se);

    return element_group;
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

struct struct_element_t * st_extend_element_group_type_name(
    struct struct_element_t *element_group,
    char *element_identifier,
    const struct st_location_t *identifier_location,
    char *element_type_name,
    const struct st_location_t *element_type_name_location,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_element_t *se = NULL;
    struct st_location_t *se_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	se,
	struct struct_element_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	se_location,
	identifier_location,
	issues,
	error_free_resources);

    struct struct_element_t *found = NULL;
    HASH_FIND_STR(element_group, element_identifier, found);
    if(found)
    {
	const char *message = issues->build_message(
	    issues,
	    "element '%s' is not unique",
	    se->element_identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_CONTEXT_ERROR,
	    2,
	    identifier_location,
	    found->identifier_location);

	goto error_free_resources;
    }

    int ref_add_result = type_refs->add(
	type_refs,
	element_type_name,
	se,
	element_type_name_location,
	struct_element_type_name_resolved,
	issues);

    if(ref_add_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    se->element_type = NULL;
    se->element_identifier = element_identifier;
    se->identifier_location = se_location;
    
    HASH_ADD_KEYPTR(hh, 
		    element_group, 
		    se->element_identifier, 
		    strlen(se->element_identifier), 
		    se);

    return element_group;
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

void st_destroy_struct_element_group(
    struct struct_element_t *element_group)
{
    /* TODO: struct element group destroy */
}

struct type_iface_t * st_create_struct_type(
    struct struct_element_t *element_group,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_type_t *st = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	st,
	struct struct_type_t,
	issues,
	error_free_resources);

    st->elements = element_group;

    memset(&(st->type), 0, sizeof(struct type_iface_t));
    st->type.create_value_of = struct_type_create_value_of;
    st->type.reset_value_of = struct_type_reset_value_of;
    st->type.can_hold = struct_type_can_hold;
    st->type.class = struct_type_class;
    st->type.destroy = struct_type_destroy;
    
    return &(st->type);
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

struct struct_element_init_t * st_create_element_initializer(
    char *element_identifier,
    const struct st_location_t *identifier_location,
    struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_element_init_t *se = NULL;
    struct st_location_t *se_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	se,
	struct struct_element_init_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	se_location,
	identifier_location,
	issues,
	error_free_resources);

    se->element_identifier = element_identifier;
    se->element_identifier_location = se_location;
    se->element_default_value = value;
    
    return se;
    
error_free_resources:
    free(se);
    free(se_location);
    return NULL;
}

struct struct_element_init_t * st_extend_element_initializer_group(
    struct struct_element_init_t *initializer_group,
    struct struct_element_init_t *initializer,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_element_init_t *found = NULL;
    HASH_FIND_STR(initializer_group, initializer->element_identifier, found);
    if(found)
    {
	const char *message = issues->build_message(
	    issues,
	    "more than one initializer for member '%s'",
	    initializer->element_identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_CONTEXT_ERROR,
	    2,
	    initializer->element_identifier_location,
	    found->element_identifier_location);

	goto error_free_resources;
    }

    HASH_ADD_KEYPTR(hh, 
		    initializer_group,
		    initializer->element_identifier,
		    strlen(initializer->element_identifier), 
		    initializer);

    return initializer_group;

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

void st_destroy_element_initializer(
    struct struct_element_init_t *initializer)
{
    /* TODO: initializer element destroy */
}

void st_destroy_initializer_group(
    struct struct_element_init_t *initializer_group)
{
    /* TODO: initializer group destroy */
}

struct value_iface_t * st_create_struct_initializer_value(
    struct struct_element_init_t *initializer_group,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_init_value_t *isv = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	isv,
	struct struct_init_value_t,
	issues,
	error_free_resources);

    isv->init_table = initializer_group;

    memset(&(isv->value), 0, sizeof(struct value_iface_t));
    isv->value.struct_init_value = struct_init_value;
    isv->value.destroy = struct_init_value_destroy;

    return &(isv->value);
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}
