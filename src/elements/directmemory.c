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

struct direct_memory_t {
    struct dmem_iface_t dmem;
    uint8_t *storage;
    size_t size;
};

static uint8_t * direct_memory_offset(
    struct dmem_iface_t *self,
    const struct direct_address_t *address,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct direct_memory_t *dm =
	CONTAINER_OF(self, struct direct_memory_t, dmem);

    if(address->byte_offset >= dm->size)
    {
	issues->new_issue(issues,
			  "byte offset '%lu' fall outside the direct memory area of '%lu' bytes",
			  ESSTEE_CONTEXT_ERROR,
			  address->byte_offset,
			  dm->size);
	return NULL;
    }

    size_t extra_byte_offset = address->bit_offset / 8;
    if(address->byte_offset + extra_byte_offset >= dm->size)
    {
	issues->new_issue(issues,
			  "bit offset '%lu' makes the byte offset fall outside the direct memory area of '%lu' bytes",
			  ESSTEE_CONTEXT_ERROR,
			  address->bit_offset,
			  dm->size);
	return NULL;
    }

    size_t offset_last_byte = address->byte_offset
	+ extra_byte_offset
	+ address->field_size_bits / 8;
	
    if(offset_last_byte > dm->size)
    {
	issues->new_issue(issues,
			  "field of size '%lu' at offset '%lu' would extend outside the direct memory area of '%lu' bytes",
			  ESSTEE_CONTEXT_ERROR,
			  address->field_size_bits,
			  address->byte_offset,
			  dm->size);
	return NULL;
    }
    
    return dm->storage + address->byte_offset + extra_byte_offset;
}

static int direct_memory_reset(
    struct dmem_iface_t *self)
{
    return ESSTEE_ERROR;
}

static void direct_memory_destroy(
    struct dmem_iface_t *self)
{
    /* TODO: direct memory destructor */
}

struct dmem_iface_t * st_new_direct_memory(
    size_t direct_memory_bytes)
{
    struct direct_memory_t *dm = NULL;
    uint8_t *storage = NULL;
    
    ALLOC_OR_JUMP(
	dm,
	struct direct_memory_t,
	error_free_resources);

    ALLOC_ARRAY_OR_JUMP(
	storage,
	uint8_t,
	direct_memory_bytes,
	error_free_resources);

    dm->storage = storage;
    dm->size = direct_memory_bytes;
    memset(dm->storage, 0, dm->size);
    
    dm->dmem.offset = direct_memory_offset;
    dm->dmem.reset = direct_memory_reset;
    dm->dmem.destroy = direct_memory_destroy;
    
    return &(dm->dmem);

error_free_resources:
    free(dm);
    free(storage);
    return NULL;
}
