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

#include <elements/subrange_case.h>
#include <util/macros.h>

/**************************************************************************/
/* Value interface                                                        */
/**************************************************************************/
struct subrange_case_value_t {
    struct value_iface_t value;
    struct subrange_t *subrange;
};

static int subrange_case_value_comparable_to(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_case_value_t *sv =
	CONTAINER_OF(self, struct subrange_case_value_t, value);

    int min_comparable = sv->subrange->min->comparable_to(sv->subrange->min,
							  other_value,
							  config,
							  issues);
    if(min_comparable != ESSTEE_TRUE)
    {
	issues->new_issue(issues,
			  "subrange can not be compared to value",
			  ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }
    
    int max_comparable = sv->subrange->max->comparable_to(sv->subrange->max,
							  other_value,
							  config,
							  issues);
    if(max_comparable != ESSTEE_TRUE)
    {
	issues->new_issue(issues,
			  "subrange can not be compared to value",
			  ESSTEE_TYPE_ERROR);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

static int subrange_case_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_case_value_t *sv =
	CONTAINER_OF(self, struct subrange_case_value_t, value);

    int min_greater = sv->subrange->min->greater(sv->subrange->min,
						 other_value,
						 config,
						 issues);

    if(min_greater == ESSTEE_TRUE)
    {
	return ESSTEE_FALSE;
    }
    else if(min_greater == ESSTEE_ERROR)
    {
	return ESSTEE_ERROR;
    }	

    int max_lesser = sv->subrange->max->lesser(sv->subrange->max,
					       other_value,
					       config,
					       issues);

    if(max_lesser == ESSTEE_TRUE)
    {
	return ESSTEE_FALSE;
    }
    else if(max_lesser == ESSTEE_ERROR)
    {
	return ESSTEE_ERROR;
    }
    
    return ESSTEE_TRUE;
}

static void subrange_case_value_destroy()
{
    /* TODO: subrange case value destroy */
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct value_iface_t * st_create_subrange_case_selector(
    struct subrange_t *subrange,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct subrange_case_value_t *scv = NULL;

    ALLOC_OR_ERROR_JUMP(
	scv,
	struct subrange_case_value_t,
	issues,
	error_free_resources);

    scv->subrange = subrange;

    memset(&(scv->value), 0, sizeof(struct value_iface_t));
    scv->value.comparable_to = subrange_case_value_comparable_to;
    scv->value.equals = subrange_case_value_equals;
    scv->value.destroy = subrange_case_value_destroy;

    return &(scv->value);

error_free_resources:
    return NULL;
}
