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

#include <statements/statements.h>

#include <utlist.h>

#include <stdlib.h>


int st_allocate_statements(
    struct invoke_iface_t *statements,
    struct issues_iface_t *issues)
{
    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(statements, itr)
    {
	if(!itr->allocate)
	{
	    continue;
	}
	
	if(itr->allocate(itr, issues) != ESSTEE_OK)
	{
	    return ESSTEE_ERROR;
	}
    }

    return ESSTEE_OK;
}

int st_verify_statements(
    struct invoke_iface_t *statements,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    int result = ESSTEE_OK;
    struct invoke_iface_t *itr = NULL;
    DL_FOREACH(statements, itr)
    {
	if(itr->verify(itr, config, issues) != ESSTEE_OK)
	{
	    result = ESSTEE_ERROR;
	}
    }

    return result;
}

void st_destroy_statements(
    struct invoke_iface_t *statements)
{
    /* TODO: destroy statements */
}
