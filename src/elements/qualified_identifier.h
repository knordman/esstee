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

#include <elements/iqualified_identifier.h>
#include <util/inamed_ref_pool.h>
#include <elements/array.h>


struct qualified_identifier_iface_t * st_create_qualified_identifier(
    struct named_ref_pool_iface_t *variable_context,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
