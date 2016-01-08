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

#include <elements/shared.h>
#include <elements/iexpression.h>

#include <utlist.h>

struct query_t {
    struct qualified_identifier_t *qi;
    struct expression_iface_t *new_value;

    struct query_t *prev;
    struct query_t *next;
};

void st_destroy_queries(
    struct query_t *queries);
