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
#include <elements/values.h>

int st_explicit_literal_type_resolved(
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
	    ESSTEE_LINK_ERROR,
	    1,
	    location);
	
	return ESSTEE_ERROR;
    }

    struct type_iface_t *literal_type =
	(struct type_iface_t *)target;

    struct value_iface_t *literal =
	(struct value_iface_t *)referrer;

    issues->begin_group(issues);
    int can_hold_result = literal_type->can_hold(literal_type,
						 literal,
						 config,
						 issues);    
    if(can_hold_result != ESSTEE_TRUE)
    {
	issues->new_issue(issues,
			  "type '%s' cannot be specified as an explicit type for literal",
			  ESSTEE_TYPE_ERROR,
			  literal_type->identifier);
	
	issues->set_group_location(issues,
				   1,
				   location);
    }
    issues->end_group(issues);
    
    if(can_hold_result != ESSTEE_TRUE)
    {
	return ESSTEE_ERROR;
    }
    
    issues->begin_group(issues);
    int override_result = literal->override_type(literal,
						 literal_type,
						 config,
						 issues);
    if(override_result != ESSTEE_OK)
    {
	issues->new_issue(issues,
			  "type override failed",
			  ESSTEE_TYPE_ERROR);
	
	issues->set_group_location(issues,
				   1,
				   location);
    }
    issues->end_group(issues);

    if(override_result != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }
    
    return ESSTEE_OK;
}

int st_string_literal_type_resolved(
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
	issues->internal_error(
	    issues,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	return ESSTEE_ERROR;
    }

    struct string_value_t *sv =
	(struct string_value_t *)referrer;

    sv->type = (struct type_iface_t *)target;

    return ESSTEE_OK;
}
