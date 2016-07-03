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

#pragma once

#include <elements/itype.h>

struct type_iface_t * st_new_elementary_integer_types();

struct value_iface_t * st_new_typeless_integer_value(
    int64_t num,
    st_bitflag_t value_class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_integer_value_set(
    struct value_iface_t *value,
    int64_t num);
    
struct value_iface_t * st_new_bool_value(
    int state,
    st_bitflag_t value_class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

int st_set_bool_value_state(
    struct value_iface_t *value,
    int state);
