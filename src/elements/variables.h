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

#include <elements/itype.h>
#include <elements/ivalue.h>
#include <elements/idirectmemory.h>
#include <esstee/flags.h>

#define INPUT_VAR_CLASS			(1 << 2)
#define OUTPUT_VAR_CLASS		(1 << 3)
#define TEMP_VAR_CLASS			(1 << 4)
#define IN_OUT_VAR_CLASS		(1 << 5)
#define EXTERNAL_VAR_CLASS		(1 << 6)
#define GLOBAL_VAR_CLASS		(1 << 7)

#define RETAIN_VAR_CLASS		(1 << 8)
#define CONSTANT_VAR_CLASS		(1 << 9)

struct variable_t {

    struct type_iface_t *type;
    struct value_iface_t *value;    
    struct direct_address_t *address;

    struct st_location_t *identifier_location;
    st_bitflag_t class;
    
    struct variable_t *prev;
    struct variable_t *next;    
    char *identifier;
    UT_hash_handle hh; 
};
