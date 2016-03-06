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
#include <elements/iinvoke.h>
#include <elements/variables.h>
#include <elements/pous.h>
#include <elements/query.h>
#include <util/ierrors.h>
#include <util/inamedreference.h>

/**************************************************************************/
/* General linking functionality                                          */
/**************************************************************************/
int st_link_queries(
    struct query_t *query,
    struct variable_t *global_variables,
    struct function_t *functions, 
    struct namedreference_iface_t *var_ref_pool,
    struct namedreference_iface_t *func_ref_pool,
    struct program_t *main,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

struct type_iface_t * st_link_types(
    struct type_iface_t *type_list,
    struct type_iface_t *type_table,
    struct errors_iface_t *errors);

struct variable_t * st_link_variables(
    struct variable_t *variable_list,
    struct variable_t *variable_table,
    struct errors_iface_t *errors);

struct function_t * st_link_functions(
    struct function_t *function_list,
    struct function_t *function_table,
    struct errors_iface_t *errors);

struct program_t * st_link_programs(
    struct program_t *function_list,
    struct program_t *function_table,
    struct errors_iface_t *errors);

int st_link_function_blocks(
    struct function_block_t *function_blocks,
    struct errors_iface_t *errors);

int st_resolve_type_refs(
    struct namedreference_iface_t *type_ref_pool,
    struct type_iface_t *type_table);

int st_resolve_var_refs(
    struct namedreference_iface_t *var_ref_pool,
    struct variable_t *var_table);

int st_resolve_function_refs(
    struct namedreference_iface_t *function_ref_pool,
    struct function_t *function_table);

int st_verify_statements(
    struct invoke_iface_t *statements,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

/**************************************************************************/
/* POUs                                                                   */
/**************************************************************************/
int st_function_return_type_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

/**************************************************************************/
/* Shared                                                                 */
/**************************************************************************/
int st_qualified_identifier_base_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

/**************************************************************************/
/* Expressions                                                            */
/**************************************************************************/
int st_single_identifier_variable_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_function_invocation_term_function_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

/**************************************************************************/
/* Statements                                                             */
/**************************************************************************/
int st_simple_assignment_variable_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_invoke_statement_as_variable_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_invoke_statement_as_func_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_for_statement_variable_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

/**************************************************************************/
/* Variables                                                              */
/**************************************************************************/
int st_variable_type_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

/**************************************************************************/
/* Types                                                                  */
/**************************************************************************/
int st_derived_type_parent_name_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_derived_type_resolve_ancestor(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_subrange_type_storage_type_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_subrange_type_storage_type_check(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_array_type_arrayed_type_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_array_type_arrayed_type_check(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_struct_element_type_name_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_check_function_block_type_refs(
    struct function_block_t *fb,
    const struct config_iface_t *config,
    struct errors_iface_t *errors);

/**************************************************************************/
/* Literals                                                               */
/**************************************************************************/
int st_explicit_literal_type_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);

int st_string_literal_type_resolved(
    void *referrer,
    void *subreferrer,
    void *target,
    st_bitflag_t remark,
    const struct st_location_t *location,
    struct errors_iface_t *errors,
    const struct config_iface_t *config);
