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
    struct type_iface_t *type;
    char *type_name;
    struct st_location_t *location;
    struct direct_address_t *address;
    struct variable_stub_t *prev;
    struct variable_stub_t *next;
};

struct variable_t {
    struct variable_iface_t variable;
    struct type_iface_t *type;
    struct value_iface_t *value;
    struct direct_address_t *address;
    struct variable_stub_t *stub;
    struct variable_iface_t *external_alias;
};

static int variable_create(
    struct variable_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    var->value = var->type->create_value_of(var->type, config, issues);

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

    return var->type->reset_value_of(var->type,
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
    struct variable_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    if(ST_FLAG_IS_SET(self->class, CONSTANT_VAR_CLASS))
    {
	const char *message = issues->build_message(
	    issues,
	    "variable is constant and cannot be assigned a new value",
	    self->identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_CONTEXT_ERROR,
	    1,
	    self->location);

	return ESSTEE_FALSE;
    }

    return var->value->assignable_from(var->value,
				       new_value,
				       config,
				       issues);
}

static int external_variable_assignable_from(
    struct variable_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->external_alias->assignable_from(var->external_alias,
						new_value,
						config,
						issues);
}

static int variable_assign(
    struct variable_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    int assign_result = var->value->assign(var->value,
					   new_value,
					   config,
					   issues);
    if(assign_result == ESSTEE_OK)
    {
	if(var->stub->address)
	{
	    var->type->sync_direct_memory(var->type,
					  var->value,
					  var->address,
					  1);
	}
    }

    return assign_result;
}

static int external_variable_assign(
    struct variable_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->external_alias->assign(var->external_alias,
				       new_value,
				       config,
				       issues);
}

static const struct value_iface_t * variable_value(
    struct variable_iface_t *self)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->value;
}

static const struct value_iface_t * external_variable_value(
    struct variable_iface_t *self)
{
    struct variable_t *var =
	CONTAINER_OF(self, struct variable_t, variable);

    return var->external_alias->value(var->external_alias);
}

static void variable_destroy(
    struct variable_iface_t *self)
{
    /* TODO: variable destroy */
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
    var->type = (struct type_iface_t *)target;
    
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
    
    if(!var->type->sync_direct_memory)
    {
	const char *type_name = (var->type->identifier) ? var->type->identifier : "(no explicit name)";
	
	const char *message = issues->build_message(
	    issues,
	    "variable '%s' cannot be stored to direct memory, its type '%s' does not support the operation",
	    var->variable.identifier,
	    type_name);

	issues->new_issue_at(issues,
			     message,
			     ESSTEE_TYPE_ERROR,
			     1,
			     var->variable.location);

	return ESSTEE_ERROR;
    }

    if(!var->type->validate_direct_address)
    {
	issues->internal_error(issues,
			       __FILE__,
			       __FUNCTION__,
			       __LINE__);
	return ESSTEE_ERROR;
    }

    issues->begin_group(issues);
    int valid_result = var->type->validate_direct_address(var->type,
							  var->address,
							  issues);
    if(valid_result != ESSTEE_OK)
    {
	issues->new_issue(issues,
			  "invalid direct address for variable '%s'",
			  ESSTEE_CONTEXT_ERROR,
			  var->variable.identifier);

	issues->set_group_location(issues,
				   1,
				   var->variable.location);
    }
    issues->end_group(issues);

    if(valid_result != ESSTEE_OK)
    {
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
    stub->type = st_create_derived_type_by_name(NULL,
						type_name,
						type_name_location,
						NULL,
						default_value,
						default_value_location,
						type_refs,
						config,
						issues);
    if(!stub->type)
    {
	goto error_free_resources;
    }
    
    return stub;
    
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
	var->type = itr->type;
	var->stub = itr;
	    
	if(ST_FLAG_IS_SET(block_class, EXTERNAL_VAR_CLASS))
	{
	    var->variable.create = external_variable_create;
	    var->variable.reset = external_variable_reset;
	    var->variable.assignable_from = external_variable_assignable_from;
	    var->variable.assign = external_variable_assign;
	    var->variable.value = external_variable_value;
	    var->variable.destroy = variable_destroy;

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
	    var->variable.assignable_from = variable_assignable_from;
	    var->variable.assign = variable_assign;
	    var->variable.value = variable_value;
	    var->variable.destroy = variable_destroy;

	    if(itr->type_name)
	    {
		int ref_result = type_refs->add(
		    type_refs,
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
    var->type = NULL;
    var->value = NULL;
    var->address = NULL;
    var->external_alias = NULL;

    memset(&(var->variable), 0, sizeof(struct variable_iface_t));
    var->variable.identifier = stub->identifier;
    var->variable.location = stub->location;
    
    var->variable.create = variable_create;
    var->variable.reset = variable_reset;
    var->variable.assignable_from = variable_assignable_from;
    var->variable.assign = variable_assign;
    var->variable.value = variable_value;
    var->variable.destroy = variable_destroy;

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
