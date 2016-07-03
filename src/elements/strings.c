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

#include <elements/strings.h>
#include <elements/values.h>
#include <elements/types.h>
#include <util/macros.h>

#include <utlist.h>
#include <stdio.h>
#include <string.h>

/**************************************************************************/
/* String literals                                                        */
/**************************************************************************/
/* Handled as its own value/type due to the strong connection between
 * the literal and its type */
struct literal_string_t {
    struct type_iface_t type;
    struct value_iface_t value;
    char *content;
    st_bitflag_t class;
};

static st_bitflag_t literal_string_type_class(
    const struct type_iface_t *self)
{
    const struct literal_string_t *ls =
	CONTAINER_OF(self, struct literal_string_t, type);

    return ls->class;
}

static int literal_string_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct literal_string_t *ls =
	CONTAINER_OF(self, struct literal_string_t, value);

    int written_bytes = snprintf(buffer,
				 buffer_size,
				 "%s",
				 ls->content);
    CHECK_WRITTEN_BYTES(written_bytes);
    return written_bytes;
}

static int literal_string_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct literal_string_t *ls =
	CONTAINER_OF(self, struct literal_string_t, value);

    if(!other_value->string)
    {
	issues->new_issue(
	    issues,
	    "string literal '%s' can only be compared to string values",
	    ESSTEE_TYPE_ERROR,
	    ls->content);

	return ESSTEE_FALSE;
    }

    if(other_value->type_of)
    {
	const struct type_iface_t *value_type = other_value->type_of(other_value);
	st_bitflag_t value_type_class = value_type->class(value_type);
	st_bitflag_t mask = STRING_TYPE|WSTRING_TYPE;
	
	if(ls->class != (value_type_class & mask))
	{
	    issues->new_issue(
		issues,
		"string literal '%s' can only be compared to strings of type '%s'",
		ESSTEE_TYPE_ERROR,
		ls->content,
		ls->type.identifier);

	    return ESSTEE_FALSE;
	}
    }

    return ESSTEE_TRUE;
}

static const struct type_iface_t * literal_string_type_of(
    const struct value_iface_t *self)
{
    const struct literal_string_t *ls =
	CONTAINER_OF(self, struct literal_string_t, value);

    return &(ls->type);
}

static void literal_string_destroy(
    struct value_iface_t *self)
{
    /* TODO: literal string destroy */
}

static int literal_string_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct literal_string_t *ls =
	CONTAINER_OF(self, struct literal_string_t, value);

    const char *other_string =
	other_value->string(other_value, config, issues);

    if(strcmp(ls->content, other_string) == 0)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

static const char * literal_string_string(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct literal_string_t *ls =
	CONTAINER_OF(self, struct literal_string_t, value);

    return ls->content;
}

/**************************************************************************/
/* Value interface                                                        */
/**************************************************************************/
struct string_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    const char *str;
};

static int string_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct string_value_t *sv =
	CONTAINER_OF(self, struct string_value_t, value);

    int written_bytes = snprintf(buffer,
				 buffer_size,
				 "%s",
				 sv->str);
    CHECK_WRITTEN_BYTES(written_bytes);
    return written_bytes;
}

static int string_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct string_value_t *sv =
	CONTAINER_OF(self, struct string_value_t, value);

    sv->str = new_value->string(new_value, config, issues);

    return ESSTEE_OK;
}

static int string_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct string_value_t *sv =
	CONTAINER_OF(self, struct string_value_t, value);

    return sv->type->can_hold(sv->type, other_value, config, issues);
}

static const struct type_iface_t * string_value_type_of(
    const struct value_iface_t *self)
{
    struct string_value_t *sv =
	CONTAINER_OF(self, struct string_value_t, value);

    return sv->type;
}

static void string_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: string value destroy */
}

static int string_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct string_value_t *sv =
	CONTAINER_OF(self, struct string_value_t, value);

    const char *other_string =
	other_value->string(other_value, config, issues);

    if(strcmp(sv->str, other_string) == 0)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

static const char * string_value_string(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct string_value_t *sv =
	CONTAINER_OF(self, struct string_value_t, value);

    return sv->str;
}

static int string_value_override_type(
	const struct value_iface_t *self,
	const struct type_iface_t *type,
	const struct config_iface_t *config,
	struct issues_iface_t *issues)
{
    struct string_value_t *sv =
	CONTAINER_OF(self, struct string_value_t, value);

    sv->type = type;

    return ESSTEE_OK;
}

/**************************************************************************/
/* Type interface                                                         */
/**************************************************************************/
struct string_type_t {
    struct type_iface_t type;
    size_t length;
    st_bitflag_t class;
    const char *default_value;
};

static struct value_iface_t * string_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct string_value_t *sv = NULL;

    ALLOC_OR_ERROR_JUMP(
	sv,
	struct string_value_t,
	issues,
	error_free_resources);

    sv->type = self;

    memset(&(sv->value), 0, sizeof(struct value_iface_t));

    sv->value.display = string_value_display;
    sv->value.assign = string_value_assign;
    sv->value.assignable_from = string_value_assigns_and_compares;
    sv->value.comparable_to = string_value_assigns_and_compares;
    sv->value.class = st_general_value_empty_class;
    sv->value.type_of = string_value_type_of;
    sv->value.destroy = string_value_destroy;
    sv->value.equals = string_value_equals;
    sv->value.override_type = string_value_override_type;
    sv->value.string = string_value_string;
	
    return &(sv->value);
    
error_free_resources:
    return NULL;
}

static int string_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct string_type_t *st =
	CONTAINER_OF(self, struct string_type_t, type);

    struct string_value_t *sv =
	CONTAINER_OF(value_of, struct string_value_t, value);

    sv->str = st->default_value;

    return ESSTEE_OK;
}

static int string_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct string_type_t *st =
	CONTAINER_OF(self, struct string_type_t, type);

    if(!value->string)
    {
	issues->new_issue(
	    issues,
	    "type '%s' can only hold string values",
	    ESSTEE_TYPE_ERROR,
	    self->identifier);

	return ESSTEE_FALSE;
    }

    if(value->type_of)
    {
	const struct type_iface_t *value_type = value->type_of(value);
	st_bitflag_t value_type_class = value_type->class(value_type);
	st_bitflag_t mask = STRING_TYPE|WSTRING_TYPE;
	
	if(st->class != (value_type_class & mask))
	{
	    issues->new_issue(
		issues,
		"type '%s' can not be assigned a string of type '%s'",
		ESSTEE_TYPE_ERROR,
		self->identifier,
		value_type->identifier);

	    return ESSTEE_FALSE;
	}
    }
    
    if(st->length > 0)
    {
	const char *string = value->string(value, config, issues);
	size_t string_length = strlen(string);
	
	if(string_length > st->length)
	{
	    issues->new_issue(
		issues,
		"type '%s' can at maximum hold strings of length %u, not %u",
		ESSTEE_TYPE_ERROR,
		self->identifier,
		st->length,
		string_length);

	    return ESSTEE_FALSE;
	}
    }

    return ESSTEE_TRUE;
}

static st_bitflag_t string_type_class(
    const struct type_iface_t *self)
{
    const struct string_type_t *st =
	CONTAINER_OF(self, struct string_type_t, type);

    return st->class;
}

static void string_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: string type destructor */
}

struct custom_string_type_t {
    struct string_type_t st;
    struct value_iface_t *default_value;
};

static int custom_string_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct string_type_t *st =
	CONTAINER_OF(self, struct string_type_t, type);

    const struct custom_string_type_t *cst =
	CONTAINER_OF(st, struct custom_string_type_t, st);
    
    struct string_value_t *sv =
	CONTAINER_OF(value_of, struct string_value_t, value);

    if(cst->default_value)
    {
	sv->str = cst->default_value->string(cst->default_value,
					     config,
					     issues);
    }
    else
    {
	sv->str = cst->st.default_value;
    }

    return ESSTEE_OK;
}

static void custom_string_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: custom string type destructor */
}

/**************************************************************************/
/* Public functions                                                       */
/**************************************************************************/
static struct string_type_t string_type_templates[] = {
    {	.type = {
	    .location = NULL,
	    .create_value_of = string_type_create_value_of,
	    .reset_value_of = string_type_reset_value_of,
	    .sync_direct_memory = NULL,
	    .validate_direct_address = NULL,
	    .can_hold = string_type_can_hold,
	    .class = string_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = string_type_destroy,
	    .identifier = "STRING",
	},
	.class = STRING_TYPE,
	.length = 0,
	.default_value = "''",
    },
    {	.type = {
	    .location = NULL,
	    .create_value_of = string_type_create_value_of,
	    .reset_value_of = string_type_reset_value_of,
	    .sync_direct_memory = NULL,
	    .validate_direct_address = NULL,
	    .can_hold = string_type_can_hold,
	    .class = string_type_class,
	    .compatible = st_type_general_compatible,
	    .destroy = string_type_destroy,
	    .identifier = "WSTRING",
	},
	.class = WSTRING_TYPE,
	.length = 0,
	.default_value = "\"\"",
    },
};

struct type_iface_t * st_new_elementary_string_types()
{
    struct string_type_t *string_types = NULL;
    struct type_iface_t *string_type_list = NULL;
    
    size_t num_string_types =
	sizeof(string_type_templates)/sizeof(struct string_type_t);

    ALLOC_ARRAY_OR_JUMP(
	string_types,
	struct string_type_t,
	num_string_types,
	error_free_resources);

    for(int i=0; i < num_string_types; i++)  
    { 
	memcpy(
	    &(string_types[i]),
	    &(string_type_templates[i]),
	    sizeof(struct string_type_t));

	DL_APPEND(string_type_list, &(string_types[i].type));
    }

    return string_type_list;
    
error_free_resources:
    return NULL;
}

struct type_iface_t * st_new_custom_length_string_type(
    const char *type_name,
    struct value_iface_t *length,
    const struct st_location_t *length_location,
    struct value_iface_t *default_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct custom_string_type_t *cst = NULL;

    if(!length->integer)
    {
	issues->new_issue_at(
	    issues,
	    "string length must be an integer",
	    ESSTEE_CONTEXT_ERROR,
	    1,
	    length_location);

	goto error_free_resources;
    }

    int64_t length_int = length->integer(length, config, issues);
    if(length_int < 0)
    {
	issues->new_issue_at(
	    issues,
	    "string length must be a positive integer",
	    ESSTEE_CONTEXT_ERROR,
	    1,
	    length_location);

	goto error_free_resources;
    }
    else if(length_int == 0)
    {
	issues->new_issue_at(
	    issues,
	    "string length must be greater than zero",
	    ESSTEE_CONTEXT_ERROR,
	    1,
	    length_location);

	goto error_free_resources;
    }

    ALLOC_OR_ERROR_JUMP(
	cst,
	struct custom_string_type_t,
	issues,
	error_free_resources);

    cst->default_value = default_value;

    cst->st.length = (size_t)length_int;
    cst->st.type.location = NULL;
    cst->st.type.create_value_of = string_type_create_value_of;
    cst->st.type.reset_value_of = custom_string_type_reset_value_of;
    cst->st.type.sync_direct_memory = NULL;
    cst->st.type.validate_direct_address = NULL;
    cst->st.type.can_hold = string_type_can_hold;
    cst->st.type.class = string_type_class;
    cst->st.type.compatible = st_type_general_compatible;
    cst->st.type.destroy = custom_string_type_destroy;

    if(strcmp(type_name, "STRING") == 0)
    {
	cst->st.default_value = "''";
	cst->st.type.identifier = "STRING";
	cst->st.class = STRING_TYPE;
    }
    else
    {
	cst->st.default_value = "\"\"";
	cst->st.type.identifier = "WSTRING";
	cst->st.class = WSTRING_TYPE;
    }

    return &(cst->st.type);

error_free_resources:
    return NULL;
}

struct value_iface_t * st_new_string_value(
    const char *type_name,
    char *content,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct literal_string_t *ls = NULL;
    ALLOC_OR_ERROR_JUMP(
	ls,
	struct literal_string_t,
	issues,
	error_free_resources);

    memset(&(ls->type), 0, sizeof(struct type_iface_t));

    if(strcmp(type_name, "STRING") == 0)
    {
	ls->type.identifier = "STRING";
	ls->type.class = literal_string_type_class;
	ls->class = STRING_TYPE;
    }
    else if(strcmp(type_name, "WSTRING") == 0)
    {
	ls->type.identifier = "WSTRING";
	ls->type.class = literal_string_type_class;
	ls->class = WSTRING_TYPE;
    }
    else
    {
	issues->internal_error(issues,
			       __FILE__,
			       __FUNCTION__,
			       __LINE__);
	goto error_free_resources;
    }

    memset(&(ls->value), 0, sizeof(struct value_iface_t));
    ls->value.display = literal_string_display;
    ls->value.comparable_to = literal_string_compares;
    ls->value.type_of = literal_string_type_of;
    ls->value.equals = literal_string_equals;
    ls->value.destroy = literal_string_destroy;
    ls->value.string = literal_string_string;

    ls->content = content;
    
    return &(ls->value);
    
error_free_resources:
    free(ls);
    return NULL;
}
