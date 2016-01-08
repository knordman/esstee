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

#include <elements/idirectmemory.h>

struct direct_memory_t {
    struct dmem_iface_t dmem;
    /* TODO: add data of direct memory */
};

struct dmem_iface_t * st_new_direct_memory(
    unsigned direct_memory_bytes);

uint8_t * st_direct_memory_offset(
    struct dmem_iface_t *self,
    struct direct_address_t *da,
    struct config_iface_t *conf);

int st_direct_memory_reset(
    struct dmem_iface_t *self);

void st_destroy_direct_memory(
    struct dmem_iface_t *self);
