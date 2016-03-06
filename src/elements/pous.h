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
#include <elements/variables.h>
#include <elements/iinvoke.h>

struct header_t {
    struct type_iface_t *types;
    struct variable_t *variables;
};

void st_destroy_header(
    struct header_t *header);

struct function_t {    
    struct header_t *header;

    struct variable_t output;

    struct invoke_iface_t *statements;

    struct st_location_t *location;

    struct namedreference_iface_t *type_ref_pool;
    struct namedreference_iface_t *var_ref_pool;
    
    struct function_t *prev;
    struct function_t *next;
    char *identifier;
    UT_hash_handle hh;
};

void st_destroy_function(
    struct function_t *function);

struct program_t {
    struct header_t *header;

    struct invoke_iface_t *statements;

    struct st_location_t *location;

    struct namedreference_iface_t *type_ref_pool;
    struct namedreference_iface_t *var_ref_pool;
    
    struct program_t *prev;
    struct program_t *next;
    char *identifier;
    UT_hash_handle hh;
};

void st_destroy_program(
    struct program_t *program);

struct function_block_t {
    struct type_iface_t type;
    struct header_t *header;

    struct invoke_iface_t *statements;

    struct st_location_t *location;

    struct namedreference_iface_t *type_ref_pool;
    struct namedreference_iface_t *var_ref_pool;

    struct function_block_t *prev;
    struct function_block_t *next;
    char *identifier;
    UT_hash_handle hh;
};

