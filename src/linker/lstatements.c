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
#include <elements/statements.h>


int st_simple_assignment_variable_resolved(
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
	    "undefined variable",
	    ISSUE_ERROR_CLASS,
	    1,
	    location);
	return ESSTEE_ERROR;
    }
    
    struct simple_assignment_statement_t *sa =
	(struct simple_assignment_statement_t *)referrer;

    sa->lhs = (struct variable_t *)target;

    return ESSTEE_OK;
}

int st_invoke_statement_as_variable_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    struct invoke_statement_t *is =
	(struct invoke_statement_t *)referrer;

    is->variable = (struct variable_t *)target;

    return ESSTEE_OK;
}

int st_invoke_statement_as_func_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config)
{
    struct invoke_statement_t *is =
	(struct invoke_statement_t *)referrer;

    is->function = (struct function_t *)target;

    return ESSTEE_OK;
}

int st_for_statement_variable_resolved(
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
	    "undefined variable",
	    ISSUE_ERROR_CLASS,
	    1,
	    location);
	return ESSTEE_ERROR;
    }
    
    struct for_statement_t *fs =
	(struct for_statement_t *)referrer;

    fs->variable = (struct variable_t *)target;

    return ESSTEE_OK;
}
	
