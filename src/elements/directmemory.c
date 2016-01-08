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

#include <elements/directmemory.h>
#include <util/macros.h>

#include <stddef.h>


struct dmem_iface_t * st_new_direct_memory(
    unsigned direct_memory_bytes)
{
    struct direct_memory_t *dm = NULL;
    ALLOC_OR_JUMP(
	dm,
	struct direct_memory_t,
	error_free_resources);

    dm->dmem.offset = st_direct_memory_offset;
    dm->dmem.reset = st_direct_memory_reset;
    
    return &(dm->dmem);

error_free_resources:
    return NULL;
}


uint8_t * st_direct_memory_offset(
    struct dmem_iface_t *self,
    struct direct_address_t *da,
    struct config_iface_t *conf)
{
    /* TODO: return offset into direct memory based on direct address */
    return NULL;
}

int st_direct_memory_reset(
    struct dmem_iface_t *self)
{
    /* TODO: reset direct memory, to zero, or resetted value?  */
    return ESSTEE_OK;
}

void st_destroy_direct_memory(
    struct dmem_iface_t *self)
{
    /* TODO: destructor for direct memory */
}
