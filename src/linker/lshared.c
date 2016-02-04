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
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    if(ST_FLAG_IS_SET(remark, PROGRAM_IN_QUERY_RESOLVE_REMARK))
    {
	if(!target)
	{
	    errors->new_issue_at(
		errors,
		"no such program",
		ISSUE_ERROR_CLASS,
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
	    errors->new_issue_at(
		errors,
		"undefined variable",
		ISSUE_ERROR_CLASS,
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
