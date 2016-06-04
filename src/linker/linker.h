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
#include <elements/ivariable.h>
#include <elements/ifunction.h>
#include <elements/iprogram.h>
#include <statements/iinvoke.h>
#include <util/iissues.h>
#include <util/inamed_ref_pool.h>

struct type_iface_t * st_link_types(
    struct type_iface_t *type_list,
    struct type_iface_t *type_table,
    struct issues_iface_t *errors);

struct variable_iface_t * st_link_variables(
    struct variable_iface_t *variable_list,
    struct variable_iface_t *variable_table,
    struct issues_iface_t *errors);

struct function_iface_t * st_link_functions(
    struct function_iface_t *function_list,
    struct function_iface_t *function_table,
    struct issues_iface_t *issues);

struct program_iface_t * st_link_programs(
    struct program_iface_t *program_list,
    struct program_iface_t *program_table,
    struct issues_iface_t *issues);

void st_resolve_type_refs(
    struct named_ref_pool_iface_t *type_ref_pool,
    struct type_iface_t *type_table);

void st_resolve_var_refs(
    struct named_ref_pool_iface_t *var_ref_pool,
    struct variable_iface_t *var_table);

int st_resolve_function_refs(
    struct named_ref_pool_iface_t *function_ref_pool,
    struct function_iface_t *function_table);

void st_resolve_program_refs(
    struct named_ref_pool_iface_t *prgm_ref_pool,
    struct program_iface_t *prgm_table);

void st_resolve_pou_var_refs(
    struct named_ref_pool_iface_t *var_refs,
    struct variable_iface_t *global_var_table,
    struct variable_iface_t *var_table);
