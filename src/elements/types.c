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

#include <elements/types.h>
#include <elements/pous.h>
#include <elements/values.h>
#include <elements/variables.h>
#include <util/macros.h>
#include <util/bitflag.h>
#include <linker/linker.h>

#include <utlist.h>


int st_type_general_compatible(
    const struct type_iface_t *self,
    const struct type_iface_t *other_type,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    st_bitflag_t self_class = self->class(self);
    st_bitflag_t other_type_class = other_type->class(other_type);
    
    if(self_class == other_type_class)
    {
	return ESSTEE_TRUE;
    }

    /* Clear derived and subrange flags from both types */
    st_bitflag_t self_class_cmp = self_class & (~(DERIVED_TYPE|SUBRANGE_TYPE));
    st_bitflag_t other_type_class_cmp = other_type_class & (~(DERIVED_TYPE|SUBRANGE_TYPE));

    if(self_class_cmp == other_type_class_cmp)
    {
	return ESSTEE_TRUE;
    }

    const char *self_name = (self->identifier) ? self->identifier : "anonymous type";
    const char *other_name = (other_type->identifier) ? other_type->identifier : "anonymous type";
    
    issues->new_issue(
	issues,
	"types '%s' and '%s' are incompatible",
	ESSTEE_TYPE_ERROR,
	self_name,
	other_name);

    return ESSTEE_FALSE;
}

struct type_iface_t * st_new_elementary_types() 
{
    struct type_iface_t *elementary_types = NULL; 
    struct integer_type_t *integer_types = NULL;
    struct real_type_t *real_types = NULL;
    struct string_type_t *string_types = NULL;
    
    /* Real types */

    /* String types */

    /* Duration type */
    ALLOC_OR_JUMP(
	duration_type,
	struct duration_type_t,
	error_free_resources);

    memcpy(
	duration_type,
	&(duration_type_template),
	sizeof(struct duration_type_t));

    DL_APPEND(elementary_types, &(duration_type->type));
    
    /* Date type */
    ALLOC_OR_JUMP(
	date_type,
	struct date_type_t,
	error_free_resources);

    memcpy(
	date_type,
	&(date_type_template),
	sizeof(struct date_type_t));

    DL_APPEND(elementary_types, &(date_type->type));
    
    /* Tod type */
    ALLOC_OR_JUMP(
	tod_type,
	struct tod_type_t,
	error_free_resources);

    memcpy(
	tod_type,
	&(tod_type_template),
	sizeof(struct tod_type_t));

    DL_APPEND(elementary_types, &(tod_type->type));

    /* Date and tod type */
    ALLOC_OR_JUMP(
	date_tod_type,
	struct date_tod_type_t,
	error_free_resources);

    memcpy(
	date_tod_type,
	&(date_tod_type_template),
	sizeof(struct date_tod_type_t));

    DL_APPEND(elementary_types, &(date_tod_type->type));

    return elementary_types;
    
error_free_resources:
    free(integer_types);
    free(real_types);
    free(string_types);
    free(duration_type);
    free(date_type);
    free(tod_type);
    free(date_tod_type);
    return NULL;
}

void st_destroy_types_in_list(
    struct type_iface_t *types)
{
    /* TODO: destroy types in list */
}
