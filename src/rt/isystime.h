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

#include <stdint.h>

struct systime_iface_t {

    int (*reset)(
	struct systime_iface_t *self);
    
    uint64_t (*get_time_ms)(
	struct systime_iface_t *self);

    uint64_t (*add_time_ms)(
	struct systime_iface_t *self,
	uint64_t ms);

    uint64_t (*elapsed_from)(
	struct systime_iface_t *self,
	uint64_t timestamp);
};
