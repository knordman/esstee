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
#include <elements/integers.h>
#include <elements/reals.h>
#include <elements/strings.h>
#include <elements/date_time.h>

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
    struct type_iface_t *types = NULL;
    
    types = st_new_elementary_integer_types();
    if(!types)
    {
	return NULL;
    }
    elementary_types = types;

    types = st_new_elementary_real_types();
    if(!types)
    {
	st_destroy_types_in_list(elementary_types);
	return NULL;
    }
    DL_CONCAT(elementary_types, types);

    types = st_new_elementary_string_types();
    if(!types)
    {
	st_destroy_types_in_list(elementary_types);
	return NULL;
    }
    DL_CONCAT(elementary_types, types);
    
    types = st_new_elementary_date_time_types();
    if(!types)
    {
	st_destroy_types_in_list(elementary_types);
	return NULL;
    }
    DL_CONCAT(elementary_types, types);

    return elementary_types;
}

void st_destroy_types_in_list(
    struct type_iface_t *types)
{
    /* TODO: destroy types in list */
}
