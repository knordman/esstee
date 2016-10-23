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

#pragma once

#include <elements/subrange.h>
#include <elements/ivalue.h>
#include <util/iconfig.h>
#include <util/iissues.h>

struct value_iface_t * st_create_subrange_case_selector(
    struct subrange_iface_t *subrange,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
