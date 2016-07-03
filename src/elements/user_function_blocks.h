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
#include <elements/block_header.h>
#include <statements/iinvoke.h>
#include <util/inamed_ref_pool.h>

struct type_iface_t * st_new_user_function_block(
    char *identifier,
    const struct st_location_t *location,
    struct header_t *header,
    struct invoke_iface_t *statements,
    struct named_ref_pool_iface_t *type_refs,
    struct named_ref_pool_iface_t *var_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
