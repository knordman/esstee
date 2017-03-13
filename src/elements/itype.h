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

#include <elements/ivalue.h>
#include <util/iconfig.h>
#include <util/bitflag.h>
#include <util/iissues.h>
#include <elements/idirectmemory.h>

#include <uthash.h>

#define INTEGER_NUMERIC_CLASS (1 << 0)
#define INTEGER_BITDATA_CLASS (1 << 1)
#define REAL_NUMERIC_CLASS    (1 << 2)
#define INTEGER_SIGNED        (1 << 3)
#define INTEGER_UNSIGNED      (1 << 4)
#define INTEGER_BOOL_TYPE     (1 << 5)
#define INTEGER_SINT_TYPE     (1 << 6)
#define INTEGER_INT_TYPE      (1 << 7)
#define INTEGER_DINT_TYPE     (1 << 8)
#define INTEGER_LINT_TYPE     (1 << 9)
#define INTEGER_USINT_TYPE    (1 << 10)
#define INTEGER_UINT_TYPE     (1 << 11)
#define INTEGER_UDINT_TYPE    (1 << 12)
#define INTEGER_ULINT_TYPE    (1 << 13)
#define INTEGER_BYTE_TYPE     (1 << 14)
#define INTEGER_WORD_TYPE     (1 << 15)
#define INTEGER_DWORD_TYPE    (1 << 16)
#define INTEGER_LWORD_TYPE    (1 << 17)
#define REAL_TYPE             (1 << 18)
#define LREAL_TYPE            (1 << 19)
#define STRING_TYPE           (1 << 20)
#define WSTRING_TYPE          (1 << 21)
#define DURATION_TYPE         (1 << 22)
#define DATE_TYPE             (1 << 23)
#define TOD_TYPE              (1 << 24)
#define DATE_TOD_TYPE         (1 << 25)
#define DERIVED_TYPE          (1 << 26)
#define ENUM_TYPE             (1 << 27)
#define SUBRANGE_TYPE         (1 << 28)
#define ARRAY_TYPE            (1 << 29)
#define STRUCT_TYPE           (1 << 30)
#define FB_TYPE               (1 << 31)

struct type_iface_t {

    struct value_iface_t * (*create_value_of)(
	const struct type_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*reset_value_of)(
	const struct type_iface_t *self,
	struct value_iface_t *value_of,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*cast_value_of)(
	const struct type_iface_t *self,
	struct value_iface_t *value_of,
	const struct value_iface_t *cast_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    void (*sync_direct_memory)(
	const struct type_iface_t *self,
	struct value_iface_t *value_of,
	const struct direct_address_t *address,
	int write);

    int (*validate_direct_address)(
	const struct type_iface_t *self,
	struct direct_address_t *address,
	struct issues_iface_t *issues);
	
    int (*can_hold)(
    	const struct type_iface_t *self,
	const struct value_iface_t *value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*compatible)(
	const struct type_iface_t *self,
	const struct type_iface_t *other_type,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    const struct type_iface_t * (*ancestor)(
	const struct type_iface_t *self);

    const struct function_block_iface_t * (*const_function_block_handle)(
    	const struct type_iface_t *self);

    struct function_block_iface_t * (*function_block_handle)(
    	struct type_iface_t *self);
    
    st_bitflag_t (*class)(
	const struct type_iface_t *self);
    
    void (*destroy)(
	struct type_iface_t *self);

    const char *identifier;
    const struct st_location_t *location;
    UT_hash_handle hh;
    struct type_iface_t *prev;
    struct type_iface_t *next;
};
