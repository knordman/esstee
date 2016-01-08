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

#include <rt/runtime.h>
#include <rt/cursor.h>
#include <esstee/flags.h>

#include <utlist.h>


int st_evaluate_queries(
    struct query_t *queries,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct query_t *itr = NULL;
    DL_FOREACH(queries, itr)
    {
	if(itr->new_value)
	{
	    struct cursor_t query_cursor = {
		.call_stack = NULL,
		.current = NULL
	    };
	
	    int rhs_invoke_status = (itr->new_value->invoke.step == NULL) ?
		INVOKE_RESULT_FINISHED : INVOKE_RESULT_IN_PROGRESS;
	
	    while(rhs_invoke_status == INVOKE_RESULT_IN_PROGRESS)
	    {
		rhs_invoke_status = itr->new_value->invoke.step(
		    &(itr->new_value->invoke),
		    &(query_cursor),
		    NULL,		/* What to do with time here, functions that have fbs */
		    config,
		    errors);
	    }

	    const struct value_iface_t *rhs_value = itr->new_value->return_value(itr->new_value);

	    int assignment_status = itr->qi->target->assign(
		itr->qi->target,
		rhs_value,
		config);

	    if(assignment_status != ESSTEE_OK)
	    {
		errors->new_issue_at(
		    errors,
		    "assignment failed",
		    ISSUE_ERROR_CLASS,
		    2,
		    itr->qi->location,
		    itr->new_value->invoke.location(&(itr->new_value->invoke)));

		return ESSTEE_ERROR;
	    }
	}
    }
    
    return ESSTEE_OK;
}
