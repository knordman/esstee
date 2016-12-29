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

#include <elements/variable.h>
#include <elements/derived.h>
#include <util/macros.h>

#include <utlist.h>

/**************************************************************************/
/* Variable interface                                                     */
/**************************************************************************/
struct variable_stub_t {
    char *identifier;
    const struct type_iface_t *type;
    char *type_name;
    struct st_location_t *location;
    struct direct_address_t *address;
    struct variable_stub_t *prev;
    struct variable_stub_t *next;
};

struct variable_t {
    struct variable_iface_t variable;
    struct value_iface_t *value;
    struct variable_stub_t *stub;
    struct variable_iface_t *external_alias;
};

static struct value_iface_t * referred_value(
    struct variable_t *var,
    const struct array_index_iface_t *index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct value_iface_t *value = var->value;

    if(index)
    {
	if(!var->value->index)
	{
	    issues->new_issue(issues,
			      "variable '%s' is not indexable",
			      ESSTEE_CONTEXT_ERROR,
			      var->stub->identifier);

	    return NULL;
	}
	
	value = var->value->index(var->value,
				  index,
				  config,
				  issues);
    }

    return value;
}

static const struct value_iface_t * const_referred_value(
    const struct variable_t *var,
    const struct array_index_iface_t *index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return (const struct value_iface_t *)
	referred_value(
	    (struct variable_t *)var,
	    index,
	    config,
	    issues);
}

static int variable_create(
    struct variable_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    var->value = var->stub->type->create_value_of(var->stub->type, config, issues);

    if(!var->value)
    {
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int external_variable_create(
    struct variable_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}

static int variable_reset(
    struct variable_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->stub->type->reset_value_of(var->stub->type,
					   var->value,
					   config,
					   issues);
}

static int external_variable_reset(
    struct variable_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}

static int variable_assignable_from(
    const struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    if(ST_FLAG_IS_SET(self->class, CONSTANT_VAR_CLASS))
    {
	issues->new_issue(
	    issues,
	    "variable '%s' is constant and cannot be assigned a new value",
	    ESSTEE_CONTEXT_ERROR,
	    1,
	    self->identifier);

	return ESSTEE_FALSE;
    }

    const struct value_iface_t *const_value = const_referred_value(var,
								   index,
								   config,
								   issues);
    if(!const_value)
    {
	return ESSTEE_ERROR;
    }

    return const_value->assignable_from(const_value,
					new_value,
					config,
					issues);
}

static int external_variable_assignable_from(
    const struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->external_alias->assignable_from(var->external_alias,
						index,
						new_value,
						config,
						issues);
}

static int variable_assign(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    struct value_iface_t *value = referred_value(var,
						 index,
						 config,
						 issues);
    if(!value)
    {
	return ESSTEE_ERROR;
    }

    int assign_result = value->assign(value,
				      new_value,
				      config,
				      issues);
    
    if(assign_result == ESSTEE_OK)
    {
	if(var->stub->address)
	{
	    var->stub->type->sync_direct_memory(var->stub->type,
						var->value,
						var->stub->address,
						1);
	}
    }

    return assign_result;
}

static int external_variable_assign(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->external_alias->assign(var->external_alias,
				       index,
				       new_value,
				       config,
				       issues);
}

typedef int (*variable_modifier_t)(
    struct value_iface_t *,
    const struct config_iface_t *,
    struct issues_iface_t *);

static int variable_modifier(
    struct variable_iface_t *self,
    size_t operation_offset,
    const char *not_supported_message,
    const char *not_supported_index_message,
    const struct array_index_iface_t *index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    struct value_iface_t *value = referred_value(var,
						 index,
						 config,
						 issues);
    if(!value)
    {
	return ESSTEE_ERROR;
    }

    variable_modifier_t *operation
	= (variable_modifier_t *)(((char *)value) + operation_offset);
    
    if(!operation)
    {
	const char *message = (index) ?
	    not_supported_index_message :
	    not_supported_message;

	issues->new_issue(issues,
			  message,
			  ESSTEE_CONTEXT_ERROR,
			  var->stub->identifier);

	return ESSTEE_ERROR;
    }

    int operation_result = (*operation)(value,
					config,
					issues);

    if(operation_result == ESSTEE_OK)
    {
	if(var->stub->address)
	{
	    var->stub->type->sync_direct_memory(var->stub->type,
						var->value,
						var->stub->address,
						1);
	}
    }

    return operation_result;
}
    
static int variable_not(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return variable_modifier(self,
			     offsetof(struct value_iface_t, not),
			     "variable '%s' cannot be modified by not",
			     "sub index of variable '%s' cannot be modified by not",
			     index,
			     config,
			     issues);
}

static int external_variable_not(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return variable_not(var->external_alias,
			index,
			config,
			issues);
}

static int variable_negate(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return variable_modifier(self,
			     offsetof(struct value_iface_t, negate),
			     "variable '%s' cannot be modified by negation",
			     "sub index of variable '%s' cannot be modified by negation",
			     index,
			     config,
			     issues);
}

static int external_variable_negate(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return variable_negate(var->external_alias,
			index,
			config,
			issues);
}

typedef int (*variable_modifier_by_value_t)(
    struct value_iface_t *,
    const struct value_iface_t *,
    const struct config_iface_t *,
    struct issues_iface_t *);

static int variable_modifier_by_value(
    struct variable_iface_t *self,
    size_t operation_offset,
    const char *not_supported_message,
    const char *not_supported_index_message,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    struct value_iface_t *value = referred_value(var,
						 index,
						 config,
						 issues);
    if(!value)
    {
	return ESSTEE_ERROR;
    }

    variable_modifier_by_value_t *operation
	= (variable_modifier_by_value_t *)(((char *)value) + operation_offset);
    
    if(!operation)
    {
	const char *message = (index) ?
	    not_supported_index_message :
	    not_supported_message;

	issues->new_issue(issues,
			  message,
			  ESSTEE_CONTEXT_ERROR,
			  var->stub->identifier);

	return ESSTEE_ERROR;
    }

    int operation_result = (*operation)(value,
					other_value,
					config,
					issues);

    if(operation_result == ESSTEE_OK)
    {
	if(var->stub->address)
	{
	    var->stub->type->sync_direct_memory(var->stub->type,
						var->value,
						var->stub->address,
						1);
	}
    }

    return operation_result;
}

static int variable_xor(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return variable_modifier_by_value(
	self,
	offsetof(struct value_iface_t, xor),
	"variable '%s' cannot be modified by xor",
	"sub index of variable '%s' cannot be modified by xor",
	index,
	other_value,
	config,
	issues);
}

static int external_variable_xor(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return variable_xor(var->external_alias,
			index,
			other_value,
			config,
			issues);
}

static int variable_and(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return variable_modifier_by_value(
	self,
	offsetof(struct value_iface_t, and),
	"variable '%s' cannot be modified by and",
	"sub index of variable '%s' cannot be modified by and",
	index,
	other_value,
	config,
	issues);
}

static int external_variable_and(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return variable_and(var->external_alias,
			index,
			other_value,
			config,
			issues);
}

static int variable_or(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return variable_modifier_by_value(
	self,
	offsetof(struct value_iface_t, or),
	"variable '%s' cannot be modified by or",
	"sub index of variable '%s' cannot be modified by or",
	index,
	other_value,
	config,
	issues);
}

static int external_variable_or(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return variable_or(var->external_alias,
			index,
			other_value,
			config,
			issues);
}

static int variable_plus(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return variable_modifier_by_value(
	self,
	offsetof(struct value_iface_t, plus),
	"variable '%s' cannot be modified by addition",
	"sub index of variable '%s' cannot be modified by addition",
	index,
	other_value,
	config,
	issues);
}

static int external_variable_plus(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return variable_plus(var->external_alias,
			index,
			other_value,
			config,
			issues);
}

static int variable_minus(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return variable_modifier_by_value(
	self,
	offsetof(struct value_iface_t, minus),
	"variable '%s' cannot be modified by subtraction",
	"sub index of variable '%s' cannot be modified by subtraction",
	index,
	other_value,
	config,
	issues);
}

static int external_variable_minus(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return variable_minus(var->external_alias,
			  index,
			  other_value,
			  config,
			  issues);
}

static int variable_multiply(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return variable_modifier_by_value(
	self,
	offsetof(struct value_iface_t, multiply),
	"variable '%s' cannot be modified by multiplication",
	"sub index of variable '%s' cannot be modified by multiplication",
	index,
	other_value,
	config,
	issues);
}

static int external_variable_multiply(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return variable_multiply(var->external_alias,
			     index,
			     other_value,
			     config,
			     issues);
}

static int variable_divide(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return variable_modifier_by_value(
	self,
	offsetof(struct value_iface_t, divide),
	"variable '%s' cannot be modified by division",
	"sub index of variable '%s' cannot be modified by division",
	index,
	other_value,
	config,
	issues);
}

static int external_variable_divide(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return variable_divide(var->external_alias,
			   index,
			   other_value,
			   config,
			   issues);
}

static int variable_modulus(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return variable_modifier_by_value(
	self,
	offsetof(struct value_iface_t, modulus),
	"variable '%s' cannot be modified by modulus",
	"sub index of variable '%s' cannot be modified by modulus",
	index,
	other_value,
	config,
	issues);
}

static int external_variable_modulus(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return variable_modulus(var->external_alias,
			   index,
			   other_value,
			   config,
			   issues);
}

static int variable_to_power(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return variable_modifier_by_value(
	self,
	offsetof(struct value_iface_t, to_power),
	"variable '%s' cannot be modified by exponentiation",
	"sub index of variable '%s' cannot be modified by exponentiation",
	index,
	other_value,
	config,
	issues);
}

static int external_variable_to_power(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return variable_to_power(var->external_alias,
			     index,
			     other_value,
			     config,
			     issues);
}

static int variable_invoke_verify(
    const struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct invoke_parameters_iface_t *parameters,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    if(ST_FLAG_IS_SET(self->class, CONSTANT_VAR_CLASS))
    {
	issues->new_issue(issues,
			  "variable '%s' is constant and cannot be invoked",
			  ESSTEE_CONTEXT_ERROR,
			  var->stub->identifier);

	return ESSTEE_ERROR;
    }
    
    const struct value_iface_t *const_value = const_referred_value(var,
								   index,
								   config,
								   issues);
    if(!const_value)
    {
	return ESSTEE_ERROR;
    }

    if(!const_value->invoke_verify)
    {
	issues->new_issue(issues,
			  "variable '%s' cannot be invoked",
			  ESSTEE_TYPE_ERROR,
			  var->stub->identifier);

	return ESSTEE_ERROR;
    }

    return const_value->invoke_verify(const_value,
				      parameters,
				      config,
				      issues);
}

static int external_variable_invoke_verify(
    const struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct invoke_parameters_iface_t *parameters,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
   const struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

   return var->external_alias->invoke_verify(var->external_alias,
					     index,
					     parameters,
					     config,
					     issues);
}

static int variable_invoke_step(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct invoke_parameters_iface_t *parameters,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    struct value_iface_t *value = referred_value(var,
						 index,
						 config,
						 issues);
    if(!value)
    {
	return ESSTEE_ERROR;
    }

    return value->invoke_step(value,
			      parameters,
			      cursor,
			      time,
			      config,
			      issues);
}

static int external_variable_invoke_step(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct invoke_parameters_iface_t *parameters,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->external_alias->invoke_step(var->external_alias,
					    index,
					    parameters,
					    cursor,
					    time,
					    config,
					    issues);
}

static int variable_invoke_reset(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    struct value_iface_t *value = referred_value(var,
						 index,
						 config,
						 issues);
    if(!value)
    {
	return ESSTEE_ERROR;
    }

    
    if(value->invoke_reset)
    {
	return value->invoke_reset(value,
				   config,
				   issues);
    }

    return ESSTEE_OK;
}

static int external_variable_invoke_reset(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->external_alias->invoke_reset(var->external_alias,
					     index,
					     config,
					     issues);
}

static void variable_clone_destroy(
    struct variable_iface_t *self)
{
    /* TODO: variable destroy */
}

static void external_variable_clone_destroy(
    struct variable_iface_t *self)
{
    /* TODO: variable destroy */
}

static struct variable_iface_t * variable_clone(
    struct variable_iface_t *self,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    /* Locally allocated memory, needs destruction in case of error */
    struct variable_t *clone = NULL;

    /* Allocate clone holder */
    ALLOC_OR_ERROR_JUMP(
	clone,
	struct variable_t,
	issues,
	error_free_resources);

    /* Clone the variable data and adjust for the parts that a clone
     * does not share with its ancestor */
    memcpy(clone, var, sizeof(struct variable_t));
    clone->variable.destroy = variable_clone_destroy;
    clone->value = NULL;

    return &(clone->variable);

error_free_resources:
    free(clone);
    return NULL;
}

static struct variable_iface_t * external_variable_clone(
    struct variable_iface_t *self,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    /* Locally allocated memory, needs destruction in case of error */
    struct variable_t *clone = NULL;

    ALLOC_OR_ERROR_JUMP(
	clone,
	struct variable_t,
	issues,
	error_free_resources);

    /* An external clone does not need to cloned, due to the contents,
     * but only to change the destructor (to a dummy doing nothing)
     * avoiding destruction in each clone and the ancestor */
    memcpy(clone, var, sizeof(struct variable_t));
    clone->variable.destroy = external_variable_clone_destroy;

    return &(clone->variable);

error_free_resources:
    free(clone);
    return NULL;
    
}

static struct variable_iface_t * variable_sub_variable(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const char *identifier,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    struct value_iface_t *value = referred_value(var,
						 index,
						 config,
						 issues);
    if(!value)
    {
	return NULL;
    }

    if(!value->sub_variable)
    {
	const char *message = NULL;
	
	if(index)
	{
	    message = issues->build_message(
		issues,
		"element of variable '%s' has no sub variables",
		var->stub->identifier);
	}
	else
	{
	    message = issues->build_message(
		issues,
		"variable '%s' has no sub variables",
		var->stub->identifier);
	}

	issues->new_issue(issues,
			  message,
			  ESSTEE_CONTEXT_ERROR);

	return NULL;
    }

    return value->sub_variable(value,
			       identifier,
			       config,
			       issues);
}

static struct variable_iface_t * external_variable_sub_variable(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const char *identifier,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->external_alias->sub_variable(var->external_alias,
					     index,
					     identifier,
					     config,
					     issues);
}

static const struct value_iface_t * variable_index_value(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    if(!var->value->index)
    {
	issues->new_issue(issues,
			  "variable '%s' is not indexable",
			  ESSTEE_CONTEXT_ERROR,
			  var->stub->identifier);
	return NULL;
    }

    if(var->stub->address)
    {
	var->stub->type->sync_direct_memory(var->stub->type,
					    var->value,
					    var->stub->address,
					    0);
    }
    
    return var->value->index(var->value,
			     index,
			     config,
			     issues);
}

static const struct value_iface_t * external_variable_index_value(
    struct variable_iface_t *self,
    const struct array_index_iface_t *index,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->external_alias->index_value(var->external_alias,
					    index,
					    config,
					    issues);
}

static const struct value_iface_t * variable_value(
    struct variable_iface_t *self)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    if(var->stub->address)
    {
	var->stub->type->sync_direct_memory(var->stub->type,
					    var->value,
					    var->stub->address,
					    0);
    }

    return var->value;
}

static const struct value_iface_t * external_variable_value(
    struct variable_iface_t *self)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->external_alias->value(var->external_alias);
}

static const struct type_iface_t * variable_type(
    const struct variable_iface_t *self)
{
    const struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->stub->type;
}

static const struct type_iface_t * external_variable_type(
    const struct variable_iface_t *self)
{
    const struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->external_alias->type(var->external_alias);
}

static void variable_destroy(
    struct variable_iface_t *self)
{
    /* TODO: variable destroy */
}

static void external_variable_destroy(
    struct variable_iface_t *self)
{
    /* TODO: external variable destroy (only local resources) */
}

/**************************************************************************/
/* Linker callbacks                                                       */
/**************************************************************************/
static int variable_type_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(target == NULL)
    {
	const char *message = issues->build_message(
	    issues,
	    "reference to undefined type '%s'",
	    identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_LINK_ERROR,
	    1,
	    location);

	return ESSTEE_ERROR;
    }

    struct variable_t *var = (struct variable_t *)referrer;
    var->stub->type = (struct type_iface_t *)target;
    
    return ESSTEE_OK;
}

static int external_variable_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(target == NULL)
    {
	const char *message = issues->build_message(
	    issues,
	    "reference to undefined global type '%s'",
	    identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_LINK_ERROR,
	    1,
	    location);

	return ESSTEE_ERROR;
    }

    struct variable_t *var = (struct variable_t *)referrer;
    var->external_alias = (struct variable_iface_t *)target;
    
    return ESSTEE_OK;
}

static int direct_variable_type_post_resolve(
    void *referrer,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var = (struct variable_t *)referrer;
    
    if(!var->stub->type->sync_direct_memory)
    {
	const char *message = NULL;
	if(var->stub->type->identifier)
	{
	    message = issues->build_message(
		issues,
		"variable '%s' cannot be stored to direct memory, its type '%s' does not support the operation",
		var->variable.identifier,
		var->stub->type->identifier);
	}
	else
	{
	    message = issues->build_message(
		issues,
		"variable '%s' cannot be stored to direct memory, its type does not support the operation",
		var->variable.identifier);
	}
	
	issues->new_issue_at(issues,
			     message,
			     ESSTEE_TYPE_ERROR,
			     1,
			     var->variable.location);

	return ESSTEE_ERROR;
    }

    if(!var->stub->type->validate_direct_address)
    {
	/* Types that have a sync function must also have a validation */
	issues->internal_error(issues,
			       __FILE__,
			       __FUNCTION__,
			       __LINE__);
	return ESSTEE_ERROR;
    }

    struct issue_group_iface_t *ig = issues->open_group(issues);

    int valid_result = var->stub->type->validate_direct_address(var->stub->type,
								var->stub->address,
								issues);

    ig->close(ig);
    
    if(valid_result != ESSTEE_OK)
    {
	const char *message = issues->build_message(
	    issues,
	    "invalid direct address for variable '%s'",
	    var->variable.identifier);
	
	ig->main_issue(ig,
		       message,
		       ESSTEE_CONTEXT_ERROR,
		       1,
		       var->variable.location);

	return valid_result;
    }

    return ESSTEE_OK;
}


/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct variable_stub_t * st_extend_variable_stubs(
    struct variable_stub_t *stubs,
    char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_stub_t *stub = NULL;
    struct st_location_t *stub_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	stub,
	struct variable_stub_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	stub_location,
	location,
	issues,
	error_free_resources);

    memset(stub, 0, sizeof(struct variable_stub_t));
    stub->identifier = identifier;
    stub->location = stub_location;
    DL_APPEND(stubs, stub);

    return stubs;

error_free_resources:
    free(stub);
    free(stub_location);
    return NULL;
}

void st_destroy_variable_stubs(
    struct variable_stub_t *stubs)
{
    /* TODO: destroy variable stubs */
}

struct variable_stub_t * st_set_variable_stubs_type(
    struct variable_stub_t *stubs,
    struct type_iface_t *type,
    const struct config_iface_t *config,
    struct issues_iface_t *issue)
{
    stubs->type = type;
    return stubs;
}
    
struct variable_stub_t * st_set_variable_stubs_type_name(
    struct variable_stub_t *stubs,
    char *type_name,
    const struct config_iface_t *config,
    struct issues_iface_t *issue)
{
    stubs->type_name = type_name;
    return stubs;
}

struct variable_stub_t * st_concatenate_variable_stubs(
    struct variable_stub_t *stubs_one,
    struct variable_stub_t *stubs_two,
    const struct config_iface_t *config,
    struct issues_iface_t *issue)
{
    DL_CONCAT(stubs_one, stubs_two);
    return stubs_one;
}
    
struct variable_stub_t * st_create_direct_variable_stub(
    char *identifier,
    struct direct_address_t *address,
    char *type_name,
    const struct st_location_t *type_name_location,
    const struct st_location_t *location,
    struct value_iface_t *default_value,
    const struct st_location_t *default_value_location,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_stub_t *stub = NULL;
    struct st_location_t *stub_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	stub,
	struct variable_stub_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	stub_location,
	location,
	issues,
	error_free_resources);

    memset(stub, 0, sizeof(struct variable_stub_t));
    stub->identifier = identifier;
    stub->address = address;
    stub->location = stub_location;

    if(default_value)
    {
	stub->type_name = NULL;
	stub->type = st_create_derived_type_by_name(NULL,
						    NULL,
						    type_name,
						    type_name_location,
						    default_value,
						    default_value_location,
						    type_refs,
						    config,
						    issues);

	if(!stub->type)
	{
	    goto error_free_resources;
	}
    }
    else
    {
	stub->type_name = type_name;
    }

    struct variable_stub_t *stub_list = NULL;
    DL_APPEND(stub_list, stub);
    return stub_list;
    
error_free_resources:
    free(stub);
    free(stub_location);
    return NULL;
}

struct variable_iface_t * st_create_variable_block(
    st_bitflag_t block_class,
    st_bitflag_t retain_flag,
    st_bitflag_t constant_flag,
    struct variable_stub_t *stubs,
    struct named_ref_pool_iface_t *global_var_refs,
    struct named_ref_pool_iface_t *type_refs,
    struct named_ref_pool_iface_t *global_type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_iface_t *vars = NULL;
    struct variable_t *var = NULL;
    struct variable_stub_t *itr = NULL;
    
    DL_FOREACH(stubs, itr)
    {
	ALLOC_OR_ERROR_JUMP(
	    var,
	    struct variable_t,
	    issues,
	    error_free_resources);

	memset(&(var->variable), 0, sizeof(struct variable_iface_t));
	var->variable.class = block_class|retain_flag|constant_flag;
	var->variable.identifier = itr->identifier;
	var->variable.location = itr->location;
	var->stub = itr;
	    
	if(ST_FLAG_IS_SET(block_class, EXTERNAL_VAR_CLASS))
	{
	    var->variable.create = external_variable_create;
	    var->variable.reset = external_variable_reset;
	    var->variable.clone = external_variable_clone;
	    var->variable.assignable_from = external_variable_assignable_from;
	    var->variable.assign = external_variable_assign;
	    
	    var->variable.not = external_variable_not;
	    var->variable.negate = external_variable_negate;
	    var->variable.xor = external_variable_xor;
	    var->variable.and = external_variable_and;
	    var->variable.or = external_variable_or;
	    var->variable.plus = external_variable_plus;
	    var->variable.minus = external_variable_minus;
	    var->variable.multiply = external_variable_multiply;
	    var->variable.divide = external_variable_divide;
	    var->variable.modulus = external_variable_modulus;
	    var->variable.to_power = external_variable_to_power;

	    var->variable.invoke_verify = external_variable_invoke_verify;
	    var->variable.invoke_step = external_variable_invoke_step;
	    var->variable.invoke_reset = external_variable_invoke_reset;

	    var->variable.sub_variable = external_variable_sub_variable;

	    var->variable.index_value = external_variable_index_value;
	    var->variable.value = external_variable_value;

	    var->variable.type = external_variable_type;

	    var->variable.destroy = external_variable_destroy;

	    int ref_result = global_var_refs->add(
		global_var_refs,
		var->variable.identifier,
		var,
		var->variable.location,
		external_variable_resolved,
		issues);

	    if(ref_result != ESSTEE_OK)
	    {
		goto error_free_resources;
	    }
	}
	else
	{
	    var->variable.create = variable_create;
	    var->variable.reset = variable_reset;
	    var->variable.clone = variable_clone;
	    var->variable.assignable_from = variable_assignable_from;
	    var->variable.assign = variable_assign;
	    
	    var->variable.not = variable_not;
	    var->variable.negate = variable_negate;
	    var->variable.xor = variable_xor;
	    var->variable.and = variable_and;
	    var->variable.or = variable_or;
	    var->variable.plus = variable_plus;
	    var->variable.minus = variable_minus;
	    var->variable.multiply = variable_multiply;
	    var->variable.divide = variable_divide;
	    var->variable.modulus = variable_modulus;
	    var->variable.to_power = variable_to_power;

	    var->variable.invoke_verify = variable_invoke_verify;
	    var->variable.invoke_step = variable_invoke_step;
	    var->variable.invoke_reset = variable_invoke_reset;

	    var->variable.sub_variable = variable_sub_variable;

	    var->variable.index_value = variable_index_value;
	    var->variable.value = variable_value;

	    var->variable.type = variable_type;
	    
	    var->variable.destroy = variable_destroy;

	    if(itr->type_name)
	    {
		struct named_ref_pool_iface_t *correct_type_pool = type_refs;
		
		if(ST_FLAG_IS_SET(block_class, GLOBAL_VAR_CLASS))
		{
		    correct_type_pool = global_type_refs;
		}
		
		int ref_result = correct_type_pool->add(
		    correct_type_pool,
		    itr->type_name,
		    var,
		    var->variable.location,
		    variable_type_resolved,
		    issues);

		if(ref_result != ESSTEE_OK)
		{
		    goto error_free_resources;
		}
	    }

	    if(itr->address)
	    {
		int ref_result = type_refs->add_post_resolve(
		    type_refs,
		    var,
		    direct_variable_type_post_resolve,
		    issues);

		if(ref_result != ESSTEE_OK)
		{
		    goto error_free_resources;
		}
	    }
	}

	DL_APPEND(vars, &(var->variable));
    }

    return vars;

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

struct variable_iface_t * st_create_variable_type_name(
    char *identifier,
    const struct st_location_t *location, 
    char *type_name,
    const struct st_location_t *type_name_location,
    st_bitflag_t class,
    struct named_ref_pool_iface_t *global_var_refs,
    struct named_ref_pool_iface_t *type_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    /* TODO: creating external variables passing that class?? */
    struct variable_stub_t *stub = NULL;
    struct variable_t *var = NULL;
    struct st_location_t *var_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	stub,
	struct variable_stub_t,
	issues,
	error_free_resources);
    
    ALLOC_OR_ERROR_JUMP(
	var,
	struct variable_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	var_location,
	location,
	issues,
	error_free_resources);
	
    stub->identifier = identifier;
    stub->type = NULL;
    stub->type_name = type_name;
    stub->location = var_location;
    stub->address = NULL;

    var->stub = stub;
    var->value = NULL;
    var->external_alias = NULL;

    memset(&(var->variable), 0, sizeof(struct variable_iface_t));
    var->variable.identifier = stub->identifier;
    var->variable.location = stub->location;
    var->variable.destroy = variable_destroy;
    var->variable.class = class;
    
    var->variable.create = variable_create;
    var->variable.reset = variable_reset;
    var->variable.clone = variable_clone;
    var->variable.assignable_from = variable_assignable_from;
    var->variable.assign = variable_assign;
    var->variable.value = variable_value;

    int ref_result = type_refs->add(
	type_refs,
	stub->type_name,
	var,
	var->variable.location,
	variable_type_resolved,
	issues);

    if(ref_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    return &(var->variable);

error_free_resources:
    free(stub);
    free(var);
    return NULL;
}

struct variable_iface_t * st_create_variable_type(
    char *identifier,
    const struct st_location_t *location, 
    const struct type_iface_t *type,
    st_bitflag_t class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_stub_t *stub = NULL;
    struct variable_t *var = NULL;
    struct st_location_t *var_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	stub,
	struct variable_stub_t,
	issues,
	error_free_resources);
    
    ALLOC_OR_ERROR_JUMP(
	var,
	struct variable_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	var_location,
	location,
	issues,
	error_free_resources);
	
    stub->identifier = identifier;
    stub->type = type;
    stub->type_name = NULL;
    stub->location = var_location;
    stub->address = NULL;

    var->stub = stub;
    var->value = NULL;
    var->external_alias = NULL;

    memset(&(var->variable), 0, sizeof(struct variable_iface_t));
    var->variable.identifier = stub->identifier;
    var->variable.location = stub->location;
    var->variable.destroy = variable_destroy;
    var->variable.class = class;
    
    var->variable.create = variable_create;
    var->variable.reset = variable_reset;
    var->variable.assignable_from = variable_assignable_from;
    var->variable.assign = variable_assign;
    var->variable.value = variable_value;

    return &(var->variable);

error_free_resources:
    free(stub);
    free(var);
    return NULL;
}
