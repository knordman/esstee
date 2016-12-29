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

#include <expressions/direct_address_term.h>
#include <elements/integers.h>
#include <util/macros.h>

/**************************************************************************/
/* Value interface                                                        */
/**************************************************************************/
struct direct_address_term_value_t {
    struct value_iface_t value;
    struct direct_address_t *address;
    int64_t data;
};

static int direct_address_term_value_comparable_to(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    if(ST_FLAG_IS_SET(dv->address->class, BIT_UNIT_ADDRESS))
    {
	if(!other_value->bool)
	{
	    issues->new_issue(issues,
			      "a direct address referring to a bit may only be compared booleans",
			      ESSTEE_CONTEXT_ERROR);

	    return ESSTEE_ERROR;
	}
    }
    else
    {
	if(!other_value->integer)
	{
	    issues->new_issue(issues,
			      "multi bit direct address can only be compared to integers",
			      ESSTEE_CONTEXT_ERROR);

	    return ESSTEE_ERROR;
	}
    }

    return ESSTEE_OK;
}

static int direct_address_term_value_operates_with(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->integer)
    {
	issues->new_issue(issues,
			  "multi bit direct address can only be used in expression with integers",
			  ESSTEE_CONTEXT_ERROR);

	return ESSTEE_ERROR;
    }
    
    return ESSTEE_OK;
}

static struct value_iface_t * direct_address_term_value_create_temp_from(
    const struct value_iface_t *self,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    struct value_iface_t *v = NULL;
    if(ST_FLAG_IS_SET(dv->address->class, BIT_UNIT_ADDRESS))
    {
	v = st_new_bool_value(0,
			      TEMPORARY_VALUE, 
			      NULL, /* TODO: fix dependency on config */
			      issues); 
    }
    else
    {
	v = st_new_typeless_integer_value(0,
					  TEMPORARY_VALUE,
					  NULL, /* TODO: fix dependency on config */
					  issues); 
    }

    return v;
}

static int direct_address_term_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    int64_t other_value_num = other_value->integer(other_value,
						   config,
						   issues);

    if(dv->data > other_value_num)
    {
	return ESSTEE_TRUE;
    }
       
    return ESSTEE_FALSE;
}

static int direct_address_term_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    int64_t other_value_num = other_value->integer(other_value,
						   config,
						   issues);

    if(dv->data < other_value_num)
    {
	return ESSTEE_TRUE;
    }
       
    return ESSTEE_FALSE;
}
    
static int direct_address_term_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    int64_t other_value_num;
    
    if(ST_FLAG_IS_SET(dv->address->class, BIT_UNIT_ADDRESS))
    {
	other_value_num = other_value->bool(other_value,
					    config,
					    issues);
    }
    else
    {
	other_value_num = other_value->integer(other_value,
					    config,
					    issues);
    }

    if(dv->data == other_value_num)
    {
	return ESSTEE_TRUE;
    }
       
    return ESSTEE_FALSE;
}

static int64_t direct_address_term_value_integer(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    return dv->data;
}

static int direct_address_term_value_bool(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_address_term_value_t *dv =
	CONTAINER_OF(self, struct direct_address_term_value_t, value);

    if(dv->data == 0)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

/**************************************************************************/
/* Expression interface                                                   */
/**************************************************************************/
struct direct_address_term_t {
    struct expression_iface_t expression;
    struct st_location_t *location;
    struct direct_address_term_value_t content;
};

static const struct value_iface_t * direct_address_term_return_value(
    const struct expression_iface_t *self)
{
    /* This is a special case, where an expression (here a direct
     * address term), needs to modify itself while returning the
     * value. On the other hand, e.g. for array index elements, it is
     * assumed that all expressions are const. */
    struct direct_address_term_t *dt =
	CONTAINER_OF(self, struct direct_address_term_t, expression);

    const struct direct_address_t *address = dt->content.address;
    
    if(ST_FLAG_IS_SET(dt->content.address->class, BIT_UNIT_ADDRESS))
    {
	size_t bit_offset = dt->content.address->bit_offset % 8;
	uint8_t mask = (1 << bit_offset);

	dt->content.data = mask & *(address->storage);
    }
    else
    {
	size_t data_size = address->field_size_bits / 8;
	
	switch(data_size)
	{
	case 8:
	    dt->content.data = *((int64_t *)address->storage);
	case 4:
	    dt->content.data = *((uint32_t *)address->storage);

	case 2:
	    dt->content.data = *((uint16_t *)address->storage);

	case 1:
	    dt->content.data = *(address->storage);
	}
    }

    return &(dt->content.value);
}
    
static void direct_address_term_destroy(
    struct expression_iface_t *self)
{
    /* TODO: direct address term destructor */
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct expression_iface_t * st_create_direct_address_term(
    struct direct_address_t *address,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_address_term_t *dt = NULL;
    struct st_location_t *term_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	dt,
	struct direct_address_term_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	term_location,
	location,
	issues,
	error_free_resources);

    dt->location = term_location;

    memset(&(dt->expression), 0, sizeof(struct expression_iface_t));    
    dt->expression.invoke.location = dt->location;
    dt->expression.return_value = direct_address_term_return_value;
    dt->expression.destroy = direct_address_term_destroy;

    memset(&(dt->content.value), 0, sizeof(struct value_iface_t));
    dt->content.value.comparable_to = direct_address_term_value_comparable_to;
    dt->content.value.create_temp_from = direct_address_term_value_create_temp_from; 
    dt->content.value.equals = direct_address_term_value_equals;
	
    if(address->field_size_bits > 1)
    {
	/* Integer */
	dt->content.value.operates_with = direct_address_term_value_operates_with;
	dt->content.value.greater = direct_address_term_value_greater;
	dt->content.value.lesser = direct_address_term_value_lesser;

	dt->content.value.integer = direct_address_term_value_integer;
    }
    else
    {
	/* Default to bit size if no size is given = boolean */
	address->field_size_bits = 1;
	ST_SET_FLAGS(address->class, BIT_UNIT_ADDRESS);

	dt->content.value.bool = direct_address_term_value_bool;
    }
    
    dt->location = term_location;
    dt->content.address = address;
    dt->content.data = 0;

    return &(dt->expression);

error_free_resources:
    free(dt);
    free(term_location);
    return NULL;
}
