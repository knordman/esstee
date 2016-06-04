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

#include <util/iconfig.h>
#include <util/iissues.h>

struct array_sub_index_t;

struct array_index_iface_t {

    int (*append)(
	struct array_index_iface_t *self,
	struct array_sub_index_t *sub_index,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    

};
