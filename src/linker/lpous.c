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


int st_function_return_type_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    if(target == NULL)
    {
	errors->new_issue_at(
	    errors,
	    "undefined type",
	    ISSUE_ERROR_CLASS,
	    1,
	    location);
	return ESSTEE_ERROR;
    }

    struct function_t *function = (struct function_t *)referrer;

    function->output.type = (struct type_iface_t *)target;

    return ESSTEE_OK;
}
