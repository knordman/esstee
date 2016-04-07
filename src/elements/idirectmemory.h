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

#define MEMORY_DIRECT_ADDRESS	        (1 << 0)
#define INPUT_DIRECT_ADDRESS    	(1 << 1)
#define OUTPUT_DIRECT_ADDRESS       	(1 << 2)
#define BIT_UNIT_DIRECT_ADDRESS		(1 << 3)
#define BYTE_UNIT_DIRECT_ADDRESS	(1 << 4)
#define WORD_UNIT_DIRECT_ADDRESS	(1 << 5)
#define DWORD_UNIT_DIRECT_ADDRESS	(1 << 6)
#define LONG_UNIT_DIRECT_ADDRESS	(1 << 7)
#define MEMORY_DIRECT_ADDRESS	        (1 << 0)
#define INPUT_DIRECT_ADDRESS    	(1 << 1)
#define OUTPUT_DIRECT_ADDRESS       	(1 << 2)

#define BIT_UNIT_ADDRESS		(1 << 3)
#define BYTE_UNIT_ADDRESS		(1 << 4)
#define WORD_UNIT_ADDRESS		(1 << 5)
#define DWORD_UNIT_ADDRESS		(1 << 6)
#define LONG_UNIT_ADDRESS		(1 << 7)

#include <util/iconfig.h>
#include <util/bitflag.h>
#include <util/iissues.h>

#include <stddef.h>
#include <stdint.h>

struct direct_address_t {
    st_bitflag_t class;
    size_t byte_offset;
    size_t bit_offset;
    size_t field_size_bits;
    uint8_t *storage;
};

struct dmem_iface_t {

    uint8_t * (*offset)(
	struct dmem_iface_t *self,
	const struct direct_address_t *da,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*reset)(
	struct dmem_iface_t *self);

    void (*destroy)(
	struct dmem_iface_t *self);    
};
