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

#include <elements/types.h>
#include <elements/variables.h>
#include <elements/pous.h>
#include <elements/literals.h>
#include <elements/expressions.h>
#include <elements/statements.h>
#include <elements/query.h>
#include <elements/shared.h>
#include <util/ierrors.h>
#include <util/inamedreference.h>
#include <util/bitflag.h>
#include <parser/scanneroptions.h>

#include <parser/bison.tab.h>
#include <parser/flex.h>

/*****************************************************************************/
/* General parser properties                                                 */
/*****************************************************************************/
#define PARSER_SKIP_ERROR_STRATEGY		0
#define PARSER_ABORT_ERROR_STRATEGY		1

struct parser_t {
    yyscan_t yyscanner;
    struct scanner_options_t scanner_options;
    const char *active_buffer;
    
    struct type_iface_t *global_types;	 /* List of defined global types */
    struct variable_t *global_variables; /* List of degined global variables */
    struct function_t *functions;	 /* List of defined functions */
    struct function_block_t *function_blocks; /* List of defined function block types */
    struct program_t *programs;		      /* List of defined programs */

    struct dmem_iface_t *direct_memory;
    const struct config_iface_t *config;
    
    struct namedreference_iface_t *global_type_ref_pool;
    struct namedreference_iface_t *global_var_ref_pool;    
    struct namedreference_iface_t *function_ref_pool;
    struct namedreference_iface_t *pou_type_ref_pool;
    struct namedreference_iface_t *pou_var_ref_pool;

    struct query_t *queries;
    int error_strategy;
    struct errors_iface_t *errors;
};

struct compilation_unit_t {
    const char *source;

    struct type_iface_t *global_types;	 /* List of defined global types */
    struct variable_t *global_variables; /* List of degined global variables */
    struct function_t *functions;	 /* List of defined functions */
    struct function_block_t *function_blocks; /* List of defined function block types */
    struct program_t *programs;		      /* List of defined programs */

    struct namedreference_iface_t *global_type_ref_pool;
    struct namedreference_iface_t *global_var_ref_pool;    
    struct namedreference_iface_t *function_ref_pool;
    
    UT_hash_handle hh;
};

int st_parser_init(
    struct parser_t *parser,
    struct namedreference_iface_t *global_type_ref_pool,
    struct namedreference_iface_t *global_var_ref_pool,
    struct namedreference_iface_t *function_ref_pool,
    struct namedreference_iface_t *pou_type_ref_pool,
    struct namedreference_iface_t *pou_var_ref_pool,
    struct errors_iface_t *errors,
    struct dmem_iface_t *direct_memory,
    struct config_iface_t *config);

int st_parser_reset(struct parser_t *parser);

struct compilation_unit_t * st_parse_file(
    const char *path,
    struct parser_t *parser);

void st_destroy_compilation_unit(
    struct compilation_unit_t *cu);

/**************************************************************************/
/* Query                                                                  */
/**************************************************************************/
int st_new_query_by_identifier(
    char *identifier,
    const struct st_location_t *identifier_location,
    struct expression_iface_t *assign_value,
    struct parser_t *parser);

int st_new_query_by_qualified_identifier(
    struct qualified_identifier_t *qualified_identifier,
    struct expression_iface_t *assign_value,
    struct parser_t *parser);

/**************************************************************************/
/* POUs                                                                   */
/**************************************************************************/
int st_new_function_pou(
    char *identifier,
    const struct st_location_t *location,
    char *return_type_identifier,
    const struct st_location_t *type_identifier_location,
    struct header_t *header,
    struct invoke_iface_t *statements,
    struct parser_t *parser);

int st_new_program_pou(
    char *identifier,
    const struct st_location_t *location,
   struct header_t *header,
    struct invoke_iface_t *statements,
    struct parser_t *parser);

int st_new_function_block_pou(
    char *identifier,
    const struct st_location_t *location,
    struct header_t *header,
    struct invoke_iface_t *statements,
    struct parser_t *parser);

int st_new_type_block_pou(
    struct type_iface_t *types,
    struct parser_t *parser);

int st_new_var_block_pou(
    struct variable_t *variables,
    struct parser_t *parser);

struct header_t * st_append_types_to_header(
    struct header_t *header, 
    struct type_iface_t *type_block, 
    struct parser_t *parser);

struct header_t * st_append_vars_to_header(
    struct header_t *header,
    struct variable_t *var_block,
    struct parser_t *parser);

/**************************************************************************/
/* Literals                                                               */
/**************************************************************************/
struct value_iface_t * st_extract_value_from_literal(
    struct expression_iface_t *expression,
    struct parser_t *parser);

struct expression_iface_t * st_new_explicit_literal(
    char *type_identifier,
    const struct st_location_t *type_identifier_location,
    struct expression_iface_t *implicit_literal,
    struct parser_t *parser);	

struct expression_iface_t * st_new_integer_literal(
    char *string,
    const struct st_location_t *string_location,
    int64_t sign_prefix, 
    struct parser_t *parser);

struct expression_iface_t * st_new_integer_literal_binary(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser);

struct expression_iface_t * st_new_integer_literal_octal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser);

struct expression_iface_t * st_new_integer_literal_hex(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser);

struct expression_iface_t * st_new_real_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser);

struct expression_iface_t * st_new_duration_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser);

struct expression_iface_t * st_new_date_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser);

struct expression_iface_t * st_new_tod_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser);

struct expression_iface_t * st_new_date_tod_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser);

struct expression_iface_t * st_new_boolean_literal(
    int64_t integer,
    const struct st_location_t *string_location,
    struct parser_t *parser);

struct expression_iface_t * st_new_single_string_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser);

struct expression_iface_t * st_new_double_string_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser);

/**************************************************************************/
/* Inline values                                                          */
/**************************************************************************/
struct value_iface_t * st_new_array_init_value(
    struct value_iface_t *values,
    struct parser_t *parser);

struct value_iface_t * st_new_struct_init_value(
    struct struct_element_init_t *element_group,
    struct parser_t *parser);

struct value_iface_t * st_new_subrange_case_value(
    struct subrange_t *subrange,
    struct parser_t *parser);

struct value_iface_t * st_new_enum_inline_value(
    char *identifiepr,
    const struct st_location_t *location,
    struct parser_t *parser);

struct value_iface_t * st_new_bool_temporary_value(    
    struct errors_iface_t *errors);

/**************************************************************************/
/* Types                                                                  */
/**************************************************************************/
struct type_iface_t * st_append_type_declaration(
    struct type_iface_t *type_block,
    struct type_iface_t *type,
    struct parser_t *parser);

struct type_iface_t * st_new_derived_type(
    char *type_name,
    struct type_iface_t *parent_type,
    const struct st_location_t *location,
    struct value_iface_t *initial_value,
    struct parser_t *parser);

struct type_iface_t * st_new_derived_type_by_name(
    char *type_name,
    char *parent_type_name,
    const struct st_location_t *location,
    struct value_iface_t *initial_value,
    struct parser_t *parser);

struct subrange_t * st_new_subrange(
    struct expression_iface_t *min, 
    struct expression_iface_t *max, 
    const struct st_location_t *location,
    struct parser_t *parser);

struct type_iface_t * st_new_subrange_type(
    char *storage_type_identifier,
    const struct st_location_t *storage_type_identifier_location,
    struct subrange_t *subrange,
    struct expression_iface_t *initial_value_literal,
    struct parser_t *parser);

struct enum_item_t * st_append_new_enum_item(
    struct enum_item_t *enumerated_value_group,
    char *identifier,
    const struct st_location_t *location,
    struct parser_t *parser);

struct type_iface_t * st_new_enum_type(
    struct enum_item_t *value_group, 
    char *initial_value_identifier, 
    const struct st_location_t *initial_value_location,
    struct parser_t *parser);

struct array_range_t * st_add_sub_to_new_array_range(
    struct array_range_t *array_ranges,
    struct subrange_t *subrange,
    struct parser_t *parser);

struct value_iface_t * st_append_initial_element(
    struct value_iface_t *values,
    struct value_iface_t *new_value,
    struct parser_t *parser);

struct value_iface_t * st_append_initial_elements(
    struct value_iface_t *values,
    struct expression_iface_t *multiplier,
    struct value_iface_t *new_value,
    struct parser_t *parser);

struct type_iface_t * st_new_array_type(
    struct array_range_t *array_ranges,
    char *arrayed_type_identifier,
    const struct st_location_t *arrayed_type_identifier_location,
    struct value_iface_t *initial_value,
    struct parser_t *parser);

struct struct_element_t * st_add_new_struct_element(
    struct struct_element_t *element_group,
    char *element_identifier,
    const struct st_location_t *identifier_location,
    struct type_iface_t *element_type,
    struct parser_t *parser);

struct type_iface_t * st_new_struct_type(
    struct struct_element_t *elements,
    struct parser_t *parser);

struct struct_element_init_t * st_add_initial_struct_element(
    struct struct_element_init_t *element_group,
    struct struct_element_init_t *new_element,
    struct parser_t *parser);

struct struct_element_init_t * st_new_struct_element_initializer(
    char *element_identifier,
    const struct st_location_t *identifier_location,
    struct value_iface_t *value,
    struct parser_t *parser);

struct type_iface_t * st_new_string_type(
    char *string_type_identifier,
    struct expression_iface_t *length,
    struct expression_iface_t *default_value,
    struct parser_t *parser);

/**************************************************************************/
/* Variables                                                              */
/**************************************************************************/
struct variable_t * st_new_var_declaration_block(
    st_bitflag_t block_class,
    st_bitflag_t retain_flag,
    st_bitflag_t constant_flag,
    struct variable_t *variables,
    struct parser_t *parser);

struct variable_t * st_append_var_declarations(
    struct variable_t *variable_group,
    struct variable_t *new_variables,
    struct parser_t *parser);

struct variable_t * st_append_new_var(
    struct variable_t *variable_group,
    char *variable_name,
    const struct st_location_t *name_location,
    struct parser_t *parser);

struct variable_t * st_finalize_var_list(
    struct variable_t *var_list,
    struct type_iface_t *var_type,
    struct parser_t *parser);

struct variable_t * st_finalize_var_list_by_name(
    struct variable_t *var_list,
    char *type_name,
    const struct st_location_t *name_location,
    struct parser_t *parser);

struct variable_t * st_finalize_var_list_by_edge(
    struct variable_t *var_list,
    char *type_name,
    const struct st_location_t *name_location,
    int rising_edge,
    struct parser_t *parser);

int st_initialize_direct_memory(
    struct direct_address_t *address,
    char *init_type_name,
    const struct st_location_t *name_location,
    const struct st_location_t *location,
    struct parser_t *parser);

int st_initialize_direct_memory_explicit(
    struct direct_address_t *address,
    char *init_type_name,
    const struct st_location_t *name_location,
    struct value_iface_t *value,
    const struct st_location_t *location,
    struct parser_t *parser);

struct variable_t * st_new_direct_var(
    char *name,
    const struct st_location_t *name_location,
    const struct st_location_t *declaration_location,
    char *type_name,
    const struct st_location_t *type_name_location,
    struct direct_address_t *address,
    struct parser_t *parser);

struct variable_t * st_new_direct_var_explicit(
    char *name,
    const struct st_location_t *name_location,
    const struct st_location_t *declaration_location,
    char *type_name,
    const struct st_location_t *type_name_location,
    struct direct_address_t *address,
    struct value_iface_t *initial_value,
    struct parser_t *parser);

/**************************************************************************/
/* Direct memory                                                          */
/**************************************************************************/
struct direct_address_t * st_new_direct_address(
    char *representation,
    const struct st_location_t *representation_location,
    struct parser_t *parser);

/**************************************************************************/
/* Shared structures (expressions and statements)                         */
/**************************************************************************/
struct array_index_t * st_append_new_array_index(
    struct array_index_t *index_list,
    struct expression_iface_t *index_expression,
    const struct st_location_t *location,
    struct parser_t *parser);

struct invoke_parameter_t * st_new_invoke_parameter(
    char *identifier,
    const struct st_location_t *location,
    struct expression_iface_t *assigned,
    struct parser_t *parser);

struct invoke_parameter_t * st_append_invoke_parameter(
    struct invoke_parameter_t *parameter_group,
    struct invoke_parameter_t *new_parameter,
    struct parser_t *parser);

struct qualified_identifier_t * st_new_inner_reference(
    char *identifier,
    struct qualified_identifier_t *outer,
    const struct st_location_t *location,
    struct parser_t *parser);

struct qualified_identifier_t * st_attach_array_index_to_inner_ref(
    struct qualified_identifier_t *inner_ref,
    struct array_index_t *array_index,
    struct parser_t *parser);

struct qualified_identifier_t * st_new_qualified_identifier_inner_ref(
    char *identifier,
    struct qualified_identifier_t *inner_reference,
    const struct st_location_t *location_identifier,
    const struct st_location_t *location,
    struct parser_t *parser);

struct qualified_identifier_t * st_new_qualified_identifier_array_index(
    char *identifier,
    struct array_index_t *array_index,
    const struct st_location_t *location_identifier,
    const struct st_location_t *location,
    struct parser_t *parser);

/**************************************************************************/
/* Expressions                                                            */
/**************************************************************************/
struct expression_iface_t * st_new_qualified_identifier_term(
    struct qualified_identifier_t *qualified_identifier,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_single_identifier_term(
    char *identifier,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_direct_address_term(
    struct direct_address_t *direct_address,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_function_invocation_term(
    char *function_identifier,
    const struct st_location_t *location,
    struct invoke_parameter_t *invoke_parameters,
    struct parser_t *parser);

struct expression_iface_t * st_new_negate_term(
    struct expression_iface_t *term,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_not_term(
    struct expression_iface_t *term,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_xor_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_and_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_or_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_greater_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_lesser_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_equals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_gequals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_lequals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_nequals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_plus_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_minus_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_multiply_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_division_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_mod_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

struct expression_iface_t * st_new_to_power_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    struct parser_t *parser);

/**************************************************************************/
/* Statements                                                             */
/**************************************************************************/
struct invoke_iface_t * st_append_to_statement_list(
    struct invoke_iface_t *statement_list,
    struct invoke_iface_t *statement,
    struct parser_t *parser);

struct invoke_iface_t * st_new_empty_statement(
    struct invoke_iface_t *statement_list,
    const struct st_location_t *location,
    struct parser_t *parser);

struct invoke_iface_t * st_new_assignment_statement_simple(
    char *var_identifier,
    const struct st_location_t *var_location,
    struct expression_iface_t *assignment,
    const struct st_location_t *location,
    struct parser_t *parser);

struct invoke_iface_t * st_new_assignment_statement_qualified(
    struct qualified_identifier_t *qualified_identifier,
    struct expression_iface_t *assignment,
    const struct st_location_t *location,
    struct parser_t *parser);

struct invoke_iface_t * st_new_invoke_statement(
    char *identifier,
    const struct st_location_t *identfier_location,
    struct invoke_parameter_t *invoke_parameters,
    const struct st_location_t *location,
    struct parser_t *parser);

struct if_statement_t * st_new_elsif_clause(
    struct expression_iface_t *condition,
    const struct st_location_t *condition_location,
    struct invoke_iface_t *true_statements,
    const struct st_location_t *location,
    struct parser_t *parser);

struct if_statement_t * st_append_elsif_clause(
    struct if_statement_t *elsif_clauses,
    struct if_statement_t *elsif_clause,
    const struct st_location_t *location,
    struct parser_t *parser);

struct invoke_iface_t * st_new_if_statement(
    struct expression_iface_t *condition,
    const struct st_location_t *condition_location,
    struct invoke_iface_t *true_statements,
    struct if_statement_t *elsif_clauses,
    struct invoke_iface_t *else_statements,
    const struct st_location_t *location,
    struct parser_t *parser);

struct value_iface_t * st_append_case_value(
    struct value_iface_t *case_list,
    struct value_iface_t *case_value,
    struct parser_t *parser);

struct case_t * st_new_case(
    struct value_iface_t *case_value_list,
    struct invoke_iface_t *statements,
    struct parser_t *parser);

struct case_t * st_append_case(
    struct case_t *case_list,
    struct case_t *new_case,
    struct parser_t *parser);

struct invoke_iface_t * st_new_case_statement(
    struct expression_iface_t *switcher,
    struct case_t *case_list,
    struct invoke_iface_t *else_statements,
    const struct st_location_t *location,
    struct parser_t *parser);

struct invoke_iface_t * st_new_for_statement(
    char *variable_identifier,
    const struct st_location_t *identifier_location,
    struct expression_iface_t *start,
    struct expression_iface_t *end,
    struct expression_iface_t *increment,
    struct invoke_iface_t *statements,
    const struct st_location_t *location,
    struct parser_t *parser);

struct invoke_iface_t * st_new_while_statement(
    struct expression_iface_t *while_expression,
    struct invoke_iface_t *true_statements,
    const struct st_location_t *location,
    struct parser_t *parser);

struct invoke_iface_t * st_new_repeat_statement(
    struct expression_iface_t *repeat_expression,
    struct invoke_iface_t *statements,
    const struct st_location_t *location,
    struct parser_t *parser);

struct invoke_iface_t * st_new_exit_statement(
    const struct st_location_t *location,
    struct parser_t *parser);

struct invoke_iface_t * st_new_return_statement(
    const struct st_location_t *location,
    struct parser_t *parser);
