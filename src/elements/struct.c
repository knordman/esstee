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
#include <elements/variable.h>
#include <elements/values.h>
#include <util/macros.h>

#include <uthash.h>
#include <stdio.h>


/**************************************************************************/
/* Struct elements interface                                              */
/**************************************************************************/
int struct_element_type_name_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct struct_element_node_t {
    char *identifier;
    struct st_location_t *identifier_location;
    struct type_iface_t *type;
    UT_hash_handle hh;
};

struct struct_elements_t {
    struct struct_elements_iface_t elements;
    struct struct_element_node_t *nodes;
};

static int struct_elements_extend_by_type(
    struct struct_elements_iface_t *self,
    char *identifier,
    const struct st_location_t *identifier_location,
    struct type_iface_t *type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_elements_t *elements =
	CONTAINER_OF(self, struct struct_elements_t, elements);
    
    struct struct_element_node_t *se = NULL;
    struct st_location_t *se_location = NULL;
    
    struct struct_element_node_t *found = NULL;
    HASH_FIND_STR(elements->nodes, identifier, found);
    if(found)
    {
	const char *message = issues->build_message(
	    issues,
	    "element '%s' is not unique",
	    identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_CONTEXT_ERROR,
	    2,
	    identifier_location,
	    found->identifier_location);

	goto error_free_resources;
    }

    ALLOC_OR_ERROR_JUMP(
	se,
	struct struct_element_node_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	se_location,
	identifier_location,
	issues,
	error_free_resources);

    se->type = type;
    se->identifier = identifier;
    se->identifier_location = se_location;
    
    HASH_ADD_KEYPTR(hh, 
		    elements->nodes, 
		    se->identifier, 
		    strlen(se->identifier), 
		    se);

    return ESSTEE_OK;
    
error_free_resources:
    free(se);
    free(se_location);
    return ESSTEE_ERROR;
}

static int struct_elements_extend_by_type_name(
    struct struct_elements_iface_t *self,
    char *identifier,
    const struct st_location_t *identifier_location,
    char *type_name,
    const struct st_location_t *type_name_location,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_elements_t *elements =
	CONTAINER_OF(self, struct struct_elements_t, elements);

    struct struct_element_node_t *se = NULL;
    struct st_location_t *se_location = NULL;

    struct struct_element_node_t *found = NULL;
    HASH_FIND_STR(elements->nodes, identifier, found);
    if(found)
    {
	const char *message = issues->build_message(
	    issues,
	    "element '%s' is not unique",
	    identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_CONTEXT_ERROR,
	    2,
	    identifier_location,
	    found->identifier_location);

	goto error_free_resources;
    }

    ALLOC_OR_ERROR_JUMP(
	se,
	struct struct_element_node_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	se_location,
	identifier_location,
	issues,
	error_free_resources);

    se->type = NULL;
    se->identifier = identifier;
    se->identifier_location = se_location;
    
    int ref_add_result = type_refs->add(
	type_refs,
	type_name,
	se,
	type_name_location,
	struct_element_type_name_resolved,
	issues);

    if(ref_add_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
        
    HASH_ADD_KEYPTR(hh, 
		    elements->nodes, 
		    se->identifier, 
		    strlen(se->identifier), 
		    se);

    return ESSTEE_OK;
    
error_free_resources:
    free(se);
    free(se_location);
    return ESSTEE_ERROR;
}

static void struct_elements_destroy(
    struct struct_elements_iface_t *self)
{
    /* TODO: struct elements destroy */
}

struct struct_elements_iface_t * st_create_struct_elements(
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_elements_t *se = NULL;
    ALLOC_OR_ERROR_JUMP(
	se,
	struct struct_elements_t,
	issues,
	error_free_resources);

    se->nodes = NULL;
    se->elements.extend_by_type = struct_elements_extend_by_type;
    se->elements.extend_by_type_name = struct_elements_extend_by_type_name;
    se->elements.destroy = struct_elements_destroy;

    return &(se->elements);

error_free_resources:
    return NULL;
}

/**************************************************************************/
/* Struct initializer interface                                           */
/**************************************************************************/
struct struct_init_node_t {
    char *identifier;
    struct st_location_t *identifier_location;
    struct value_iface_t *value;
    UT_hash_handle hh;
};

struct struct_initializer_t {
    struct struct_initializer_iface_t initializer;
    struct value_iface_t value;
    struct struct_init_node_t *nodes;
};

static int struct_initializer_extend(
    struct struct_initializer_iface_t *self,
    char *identifier,
    const struct st_location_t *identifier_location,
    struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_initializer_t *si =
	CONTAINER_OF(self, struct struct_initializer_t, initializer);

    struct struct_init_node_t *se = NULL;
    struct st_location_t *se_location = NULL;
    
    struct struct_init_node_t *found = NULL;
    HASH_FIND_STR(si->nodes, identifier, found);
    if(found)
    {
	const char *message = issues->build_message(
	    issues,
	    "more than one initializer for member '%s'",
	    identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_CONTEXT_ERROR,
	    2,
	    identifier_location,
	    found->identifier_location);

	goto error_free_resources;
    }

    ALLOC_OR_ERROR_JUMP(
	se,
	struct struct_init_node_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	se_location,
	identifier_location,
	issues,
	error_free_resources);

    se->identifier = identifier;
    se->identifier_location = se_location;
    se->value = value;
    
    HASH_ADD_KEYPTR(hh, 
		    si->nodes,
		    se->identifier,
		    strlen(se->identifier), 
		    se);

    return ESSTEE_OK;

error_free_resources:
    free(se);
    free(se_location);
    return ESSTEE_ERROR;
}

static struct value_iface_t * struct_initializer_value(
    struct struct_initializer_iface_t *self)
{
    struct struct_initializer_t *si =
	CONTAINER_OF(self, struct struct_initializer_t, initializer);

    return &(si->value);
}

static void struct_initializer_destroy(
    struct struct_initializer_iface_t *self)
{
    /* TODO: struct elements destroy */
}

/* Value methods */
static const struct struct_initializer_iface_t * struct_initializer_value_struct_initializer(
    const struct value_iface_t *self)
{
    const struct struct_initializer_t *si =
	CONTAINER_OF(self, struct struct_initializer_t, value);

    return &(si->initializer);
}

static void struct_initializer_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: struct initializer value destroy */
}

struct struct_initializer_iface_t * st_create_struct_initializer(
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_initializer_t *si = NULL;

    ALLOC_OR_ERROR_JUMP(
	si,
	struct struct_initializer_t,
	issues,
	error_free_resources);

    si->nodes = NULL;

    memset(&(si->initializer), 0, sizeof(struct struct_initializer_iface_t));
    si->initializer.extend = struct_initializer_extend;
    si->initializer.value = struct_initializer_value;
    si->initializer.destroy = struct_initializer_destroy;

    memset(&(si->value), 0, sizeof(struct value_iface_t));
    si->value.struct_initializer = struct_initializer_value_struct_initializer;
    si->value.destroy = struct_initializer_value_destroy;
    
    return &(si->initializer);
    
error_free_resources:
    return NULL;
}

/**************************************************************************/
/* Value interface                                                        */
/**************************************************************************/
struct struct_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    struct variable_iface_t *elements;
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

    struct variable_iface_t *itr = NULL;
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

	const struct value_iface_t *var_value = itr->value(itr);
	
	written_bytes = var_value->display(var_value,
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

    if(!other_value->struct_initializer)
    {
	issues->new_issue(
	    issues,
	    "the complete struct can only be assigned an initializer in initialization",
	    ESSTEE_CONTEXT_ERROR);

	return ESSTEE_FALSE;
    }
    
    int type_can_hold = sv->type->can_hold(sv->type,
					   other_value,
					   config,
					   issues);

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

    const struct struct_initializer_iface_t *initializer =
	new_value->struct_initializer(new_value);

    const struct struct_initializer_t *si =
	CONTAINER_OF(initializer, struct struct_initializer_t, initializer);

    const struct struct_init_node_t *itr = NULL;

    for(itr = si->nodes; itr != NULL; itr = itr->hh.next)
    {
	struct variable_iface_t *found = NULL;
	HASH_FIND_STR(sv->elements, itr->identifier, found);
	if(!found)
	{
	    issues->internal_error(
		issues,
		__FILE__,
		__FUNCTION__,
		__LINE__);
	    
	    return ESSTEE_ERROR;
	}

	int assign_result = found->assign(found,
					  NULL,
					  itr->value,
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

static struct variable_iface_t * struct_value_sub_variable(
    struct value_iface_t *self,
    const char *identifier,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct struct_value_t *sv =
	CONTAINER_OF(self, struct struct_value_t, value);

    struct variable_iface_t *found = NULL;
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
    struct struct_elements_t *elements;
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
    struct struct_element_node_t *itr = NULL;
    struct variable_iface_t *itr_var = NULL;
    for(itr = st->elements->nodes; itr != NULL; itr = itr->hh.next)
    {
	itr_var = st_create_variable_type(
	    itr->identifier,
	    itr->identifier_location,
	    itr->type,
	    0,
	    config,
	    issues);

	if(!itr_var)
	{
	    goto error_free_resources;
	}

	int create_result = itr_var->create(itr_var,
					    config,
					    issues);
	if(create_result != ESSTEE_OK)
	{
	    goto error_free_resources;
	}

	HASH_ADD_KEYPTR(hh, 
			sv->elements, 
			itr_var->identifier, 
			strlen(itr_var->identifier), 
		        itr_var);
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
    sv->value.class = st_general_value_empty_class;

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

    struct variable_iface_t *itr = NULL;
    for(itr = sv->elements; itr != NULL; itr = itr->hh.next)
    {
	int reset_result = itr->reset(itr, config, issues);

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

    if(!value->struct_initializer)
    {
	issues->new_issue(
	    issues,
	    "a struct type variable can only be initialized when declared by an initializer",
	    ESSTEE_CONTEXT_ERROR);

	return ESSTEE_FALSE;
    }

    const struct struct_initializer_iface_t *initializer =
	value->struct_initializer(value);

    const struct struct_initializer_t *si =
	CONTAINER_OF(initializer, struct struct_initializer_t, initializer);
    
    struct struct_init_node_t *itr = NULL;
    for(itr = si->nodes; itr != NULL; itr = itr->hh.next)
    {
	/* Check that identifier is one of the members */
	struct struct_element_node_t *found = NULL;
	HASH_FIND_STR(st->elements->nodes, itr->identifier, found);
	if(!found)
	{
	    issues->new_issue(
		issues,
		"struct has no member '%s'",
		ESSTEE_TYPE_ERROR,
		itr->identifier);

	    return ESSTEE_FALSE;
	}

	/* Check that the member type can hold the value */
	int member_type_can_hold = found->type->can_hold(found->type,
							 itr->value,
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

    struct struct_element_node_t *se =
	(struct struct_element_node_t *)referrer;

    se->type = (struct type_iface_t *)target;
    
    return ESSTEE_OK;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct type_iface_t * st_create_struct_type(
    struct struct_elements_iface_t *elements,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct struct_type_t *st = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	st,
	struct struct_type_t,
	issues,
	error_free_resources);
    
    struct struct_elements_t *se =
	CONTAINER_OF(elements, struct struct_elements_t, elements);
    
    st->elements = se;

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
