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
#include <elements/shared.h>
#include <util/bitflag.h>


int st_qualified_identifier_base_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(remark == 1)
    {
	if(!target)
	{
	    const char *message = issues->build_message(
		issues,
		"no program '%s' defined",
		identifier);
	    
	    issues->new_issue_at(
		issues,
		message,
		ESSTEE_LINK_ERROR,
		1,
		location);

	    return ESSTEE_ERROR;
	}

	struct qualified_identifier_t *qi =
	    (struct qualified_identifier_t *)referrer;

	qi->program = (struct program_t *)target;
    }
    else
    {
	if(!target)
	{
	    const char *message = issues->build_message(
		issues,
		"reference to undefined variable '%s'",
		identifier);

	    issues->new_issue_at(
		issues,
		message,
		ESSTEE_LINK_ERROR,
		1,
		location);

	    return ESSTEE_ERROR;
	}

	struct qualified_identifier_t *qi =
	    (struct qualified_identifier_t *)referrer;

	qi->variable = (struct variable_t *)target;
    }

    return ESSTEE_OK;
}
