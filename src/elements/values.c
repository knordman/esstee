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
along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <elements/values.h>

st_bitflag_t st_general_value_empty_class(
    const struct value_iface_t *self)
{
    return 0;
}

int st_general_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    int greater = self->greater(self, other_value, config, issues);
    if(greater == ESSTEE_TRUE)
    {
	return ESSTEE_FALSE;
    }

    int lesser = self->lesser(self, other_value, config, issues);
    if(lesser == ESSTEE_TRUE)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}
