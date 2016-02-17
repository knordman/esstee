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
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    if(!target)
    {
	errors->new_issue_at(
	    errors,
	    "reference to undefined type",
	    ISSUE_ERROR_CLASS,
	    1,
	    location);
	return ESSTEE_ERROR;
    }

    struct type_iface_t *literal_type =
	(struct type_iface_t *)target;

    struct value_iface_t *literal =
	(struct value_iface_t *)referrer;

    int type_can_hold = literal_type->can_hold(literal_type,
					       literal,
					       config);
    if(type_can_hold != ESSTEE_OK)
    {
	errors->new_issue_at(
	    errors,
	    "type cannot hold literal value",
	    ISSUE_ERROR_CLASS,
	    1,
	    location);

	return ESSTEE_ERROR;
    }

    int override = literal->override_type(literal, literal_type, config);
    if(override != ESSTEE_OK)
    {
	return override;
    }

    return ESSTEE_OK;
}
