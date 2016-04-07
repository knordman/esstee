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

#include <linker/linker.h>
#include <elements/pous.h>
#include <elements/types.h>
#include <util/macros.h>

#include <utlist.h>
#include <stdio.h>

int st_variable_type_resolved(
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
	    ISSUE_ERROR_CLASS,
	    1,
	    location);

	return ESSTEE_ERROR;
    }

    struct variable_t *var = (struct variable_t *)referrer;
    var->type = (struct type_iface_t *)target;
    
    return ESSTEE_OK;
}

int st_direct_variable_type_post_resolve(
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
	    var->identifier,
	    type_name);

	issues->new_issue_at(issues,
			     message,
			     ESSTEE_TYPE_ERROR,
			     1,
			     var->identifier_location);

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
			  var->identifier);

	issues->set_group_location(issues,
				   1,
				   var->identifier_location);
    }
    issues->end_group(issues);

    if(valid_result != ESSTEE_OK)
    {
	return valid_result;
    }
    
    return ESSTEE_OK;    
}
