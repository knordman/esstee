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

%define parse.error verbose
%define api.pure full
%locations

%lex-param {yyscan_t yyscanner}
%parse-param {yyscan_t yyscanner}
%parse-param {struct parser_t *parser}

%initial-action
{
    @$.source = parser->active_buffer;
}

%code requires {
#include <esstee/locations.h>
typedef struct st_location_t YYLTYPE;

#define YYLTYPE_IS_DECLARED 1
#define YYLTYPE_IS_TRIVIAL 0

typedef void* yyscan_t; /* Type is defined in flex.h, but flex.h depends on YYLTYPE */
struct parser_t;
#include <util/bitflag.h>
}
			
%code {
#include <parser/scanneroptions.h>
#include <parser/flex.h>	/* Prototype for yylex */
#include <parser/parser.h>	/* All parser factory functions */
#include <esstee/flags.h>
    
void yyerror(
    YYLTYPE *locp, 
    yyscan_t yyscanner, 
    struct parser_t *parser, 
    const char *error);
    
/* Special definition to get source also in yyloc (location for lhs) */
#define YYLLOC_DEFAULT(Current, Rhs, N)                                 \
    do {                                                                \
	(Current).source = YYRHSLOC(Rhs, 0).source;			\
	if (N)                                                          \
	{                                                               \
	    (Current).first_line   = YYRHSLOC(Rhs, 1).first_line;       \
	    (Current).first_column = YYRHSLOC(Rhs, 1).first_column;     \
	    (Current).last_line    = YYRHSLOC(Rhs, N).last_line;        \
	    (Current).last_column  = YYRHSLOC(Rhs, N).last_column;      \
	}                                                               \
	else                                                            \
	{                                                               \
	    (Current).first_line   = (Current).last_line   =            \
		YYRHSLOC(Rhs, 0).last_line;                             \
	    (Current).first_column = (Current).last_column =            \
		YYRHSLOC(Rhs, 0).last_column;                           \
	}                                                               \
    } while(0)

#define DO_ERROR_STRATEGY(parser)					\
    do {								\
	parser->pou_type_ref_pool->clear(parser->pou_type_ref_pool);	\
	parser->pou_var_ref_pool->clear(parser->pou_var_ref_pool);	\
	parser->global_type_ref_pool->clear(parser->global_type_ref_pool); \
	parser->global_var_ref_pool->clear(parser->global_var_ref_pool); \
	parser->function_ref_pool->clear(parser->function_ref_pool);	\
	int fatal_error = parser->errors->fatal_error_occurred(parser->errors); \
	if(fatal_error == ESSTEE_TRUE)					\
	{								\
	    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;	\
	}								\
	if(parser->error_strategy == PARSER_SKIP_ERROR_STRATEGY)	\
	{								\
	    YYERROR;							\
	}								\
	else if(parser->error_strategy == PARSER_ABORT_ERROR_STRATEGY)	\
	{								\
	    YYABORT;							\
	}								\
	else								\
	{								\
	    YYABORT;							\
	}								\
    } while(0)
 }

%union {
    char *string;

    struct type_iface_t *type;
    struct variable_iface_t *variable;
    struct variable_stub_t *variable_stub;
    struct header_t *header;

    struct value_iface_t *value;
    struct enum_group_iface_t *enum_group;
    struct array_initializer_iface_t *array_initializer;
    struct struct_initializer_iface_t *struct_initializer;
    struct struct_elements_iface_t *struct_elements;
    struct subrange_iface_t *subrange;
    struct array_range_iface_t *array_range;
    
    struct direct_address_t *direct_address;
    struct array_index_iface_t *array_index;

    struct qualified_identifier_iface_t *qualified_identifier;
    
    struct expression_iface_t *expression;
    struct invoke_iface_t *invoke;
    struct invoke_parameter_t *invoke_parameter;
    struct invoke_parameters_iface_t *invoke_parameters;
    struct case_t *case_clause;
    struct if_statement_t *if_statement;
    struct case_list_element_t *case_list;

    struct queries_iface_t *queries;
    struct query_t *query;
    
    st_bitflag_t bitflag;
    int64_t integer;
}

/* Set type (if any) of all terminal symbols */
%token			QUERY_MODE
/* Operators */
%token			GREATER
%token			LESSER
%token			GEQUALS
%token			LEQUALS
%token			EQUALS
%token			NEQUALS
%token			TO_POWER
%token			AND
%token			OR
%token			NOT
%token			XOR
%token			MOD
%nonassoc		NOT
%left			GREATER LESSER GEQUALS LEQUALS EQUALS NEQUALS
%left			AND OR XOR
%left			'+' '-'
%left			'*' '/' MOD
%left			TO_POWER
%left			unary_minus		
 /* POU types */
%token			PROGRAM
%token			END_PROGRAM
%token			FUNCTION_BLOCK
%token			END_FUNCTION_BLOCK
%token			FUNCTION
%token			END_FUNCTION
 /* Variables */
%token			VAR
%token			VAR_INPUT
%token			VAR_OUTPUT
%token			VAR_TEMP
%token			VAR_IN_OUT
%token			VAR_EXTERNAL
%token			VAR_GLOBAL
%token			END_VAR
%token			OF
%token			ARRAY
%token	<string>	DIRECT_ADDRESS
%token			CONSTANT
%token			R_EDGE
%token			F_EDGE
%token			RETAIN
%token			NON_RETAIN
%token			AT
 /* Types */
%token			TYPE
%token			END_TYPE
%token			STRUCT
%token			END_STRUCT
%token	<string>	ELEMENTARY_TYPE_NAME
%token	<string>	STRING_TYPE_NAME
 /* Conditionals */
%token			IF
%token			THEN
%token			END_IF
%token			ELSIF
%token			ELSE
%token			CASE
%token			END_CASE
 /* Statements and expressions */
%token			RETURN
%token			EXIT
%token			ASSIGN
%token			FOR
%token			END_FOR
%token			TO
%token			BY
%token			DO
%token			WHILE
%token			END_WHILE
%token			REPEAT
%token			END_REPEAT
%token			UNTIL
%token			DOTDOT
%token	<integer>	LITERAL_BOOLEAN_TRUE
%token	<integer>	LITERAL_BOOLEAN_FALSE
%token	<string>	IDENTIFIER
%token	<string>	IDENTIFIER_CASE_LABEL
%token	<string>	IDENTIFER_TYPE_SPEC_LITERAL
%token	<string>	LITERAL_DECIMAL
%token	<string>	LITERAL_BINARY
%token	<string>	LITERAL_OCTAL
%token	<string>	LITERAL_HEX
%token	<string>	LITERAL_REAL
%token	<string>	LITERAL_DURATION
%token	<string>	LITERAL_TOD
%token	<string>	LITERAL_DATE
%token	<string>	LITERAL_DATE_TOD
%token	<string>	LITERAL_SINGLE_BYTE_STRING
%token	<string>	LITERAL_DOUBLE_BYTE_STRING
 /* Set types of non-terminals */
%type	<header>	header_blocks
%type	<string>	elementary_type_name
%type	<string>	derived_type_name
%type	<string>	simple_type_name
%type	<string>	simple_type_name_not_string
%type	<string>	simple_type_name_literal
%type	<integer>	sign_prefix
%type	<value>		literal
%type	<value>		literal_no_sign_prefix
%type	<value>		literal_explicit_type
%type	<value>		literal_implicit_type
%type	<value>		literal_implicit_type_possible_sign_prefix
%type	<type>		type_declaration_block
%type	<type>		type_declarations
%type	<type>		type_declaration
%type	<type>		basic_type
%type	<type>		derive_type
%type	<type>		variable_type
%type	<value>		simple_type_initial_value
%type	<type>		subrange_type
%type	<subrange>	subrange
%type	<value>		subrange_point
%type	<type>		enum_type
%type	<enum_group>	enum_items
%type	<type>		array_type
%type	<array_range>	array_range

%type	<value>		array_initialization
%type	<array_initializer> array_initializer
%type	<value>		array_initial_element

%type	<value>		struct_initialization
%type	<struct_initializer> struct_initializer

%type	<type>		struct_type
%type	<struct_elements> struct_elements
			
%type	<value>		string_defined_length
%type	<type>		string_defined_length_type
%type	<variable>	var_declaration_block
%type	<bitflag>	var_start
%type	<bitflag>	retain_specifier
%type	<bitflag>	constant_specifier
%type	<variable_stub>	var_declarations			
%type	<variable_stub>	var_list
%type	<variable_stub>	var_declaration
%type	<direct_address> direct_address
%type	<array_index>	array_index
%type	<expression>	expression
/* %type	<qualified_part> inner_reference */
%type	<qualified_identifier> qualified_identifier
%type	<expression>	term
%type	<invoke_parameters> invoke_parameters
%type	<invoke_parameter> invoke_parameter
%type	<invoke_parameters> possibly_no_invoke_parameters
%type	<invoke>	statements
%type	<invoke>	statement
%type	<invoke>	for_statement
%type	<if_statement>	elsif_clauses
%type	<if_statement>	elsif_clause
%type	<invoke>	else_clause
%type	<case_clause>	cases
%type	<case_clause>	a_case
%type	<case_list>	case_list
%type	<value>		case_list_element
%type	<query>		query

%destructor {/*printf("freeing: %s\n", $$);free($$);*/} <string>

%%

start :
pous				/* File parsing mode */
| QUERY_MODE queries		/* Query mode */
{
    if(st_finish_queries(parser) == ESSTEE_ERROR)
    	DO_ERROR_STRATEGY(parser);
}
;

queries :
query
{
    if(st_append_query($1, parser) == ESSTEE_ERROR)
	DO_ERROR_STRATEGY(parser);
}
| queries ';' query
{
    if(st_append_query($3, parser) == ESSTEE_ERROR)
	DO_ERROR_STRATEGY(parser);
}
;

query :
'[' IDENTIFIER ']'
{
    if(($$ = st_new_query_by_program($2, &@2, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| IDENTIFIER
{
    if(($$ = st_new_query_by_identifier(NULL, NULL, $1, &@1, NULL, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| qualified_identifier
{
    if(($$ = st_new_query_by_qualified_identifier(NULL, NULL, $1, NULL, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| '[' IDENTIFIER ']' '.' IDENTIFIER
{
    if(($$ = st_new_query_by_identifier($2, &@2, $5, &@5, NULL, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| '[' IDENTIFIER ']' '.' qualified_identifier
{
    if(($$ = st_new_query_by_qualified_identifier($2, &@2, $5, NULL, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| IDENTIFIER ASSIGN expression
{
    if(($$ = st_new_query_by_identifier(NULL, NULL, $1, &@1, $3, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| qualified_identifier ASSIGN expression
{
    if(($$ = st_new_query_by_qualified_identifier(NULL, NULL, $1, $3, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| '[' IDENTIFIER ']' '.' IDENTIFIER ASSIGN expression
{
    if(($$ = st_new_query_by_identifier($2, &@2, $5, &@5, $7, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| '[' IDENTIFIER ']' '.' qualified_identifier ASSIGN expression
{
    if(($$ = st_new_query_by_qualified_identifier($2, &@2, $5, $7, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

pous :
pou
{
    if(st_reset_parser_next_pou(parser) == ESSTEE_ERROR)
	DO_ERROR_STRATEGY(parser);
}
| pous pou
{
    if(st_reset_parser_next_pou(parser) == ESSTEE_ERROR)
	DO_ERROR_STRATEGY(parser);
}
;

/* Functions, function blocks and programs (POUs) */
pou :
FUNCTION IDENTIFIER ':' simple_type_name header_blocks statements END_FUNCTION
{
    if(st_new_function_pou($2, &@$, $4, &@4, $5, $6, parser) == ESSTEE_ERROR)
	DO_ERROR_STRATEGY(parser);
}
| PROGRAM IDENTIFIER header_blocks statements END_PROGRAM
{
    if(st_new_program_pou($2, &@$, $3, $4, parser) == ESSTEE_ERROR)
	DO_ERROR_STRATEGY(parser);
}
| FUNCTION_BLOCK IDENTIFIER header_blocks statements END_FUNCTION_BLOCK
{
    if(st_new_function_block_pou($2, &@$, $3, $4, parser) == ESSTEE_ERROR)
	DO_ERROR_STRATEGY(parser);
}
| type_declaration_block
{
    if(st_new_type_block_pou($1, parser) == ESSTEE_ERROR)
	DO_ERROR_STRATEGY(parser);
}
| var_declaration_block
{
    if(st_new_var_block_pou($1, parser) == ESSTEE_ERROR)
	DO_ERROR_STRATEGY(parser);
}
;

header_blocks :
/* No header blocks */
{
    $$ = NULL;
}
| header_blocks type_declaration_block
{
    if(($$ = st_append_types_to_header($1, $2, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| header_blocks var_declaration_block
{
    if(($$ = st_append_vars_to_header($1, $2, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

/* Type names */
elementary_type_name : ELEMENTARY_TYPE_NAME | STRING_TYPE_NAME; 

derived_type_name : IDENTIFIER;

simple_type_name : elementary_type_name | derived_type_name;

simple_type_name_not_string : ELEMENTARY_TYPE_NAME | derived_type_name;

simple_type_name_literal : ELEMENTARY_TYPE_NAME | IDENTIFER_TYPE_SPEC_LITERAL;

/* Literals */
sign_prefix :
'+'
{
    $$ = 1;
}
| '-'
{
    $$ = -1;
}
;

literal : literal_explicit_type | literal_implicit_type_possible_sign_prefix;

literal_no_sign_prefix : literal_explicit_type | literal_implicit_type;

literal_explicit_type :
simple_type_name_literal '#' literal_implicit_type
{
    if(($$ = st_new_explicit_literal($1, &@1, $3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

literal_implicit_type :
LITERAL_DECIMAL
{
    if(($$ = st_new_integer_literal($1, &@1, 1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| LITERAL_BINARY
{
    if(($$ = st_new_integer_literal_binary($1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| LITERAL_OCTAL
{
    if(($$ = st_new_integer_literal_octal($1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| LITERAL_HEX
{
    if(($$ = st_new_integer_literal_hex($1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| LITERAL_REAL
{
    if(($$ = st_new_real_literal($1, &@1, 1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| LITERAL_DURATION
{
    if(($$ = st_new_duration_literal($1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| LITERAL_TOD
{
    if(($$ = st_new_tod_literal($1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| LITERAL_DATE
{
    if(($$ = st_new_date_literal($1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| LITERAL_DATE_TOD
{
    if(($$ = st_new_date_tod_literal($1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| LITERAL_BOOLEAN_TRUE
{
    if(($$ = st_new_boolean_literal($1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| LITERAL_BOOLEAN_FALSE
{
    if(($$ = st_new_boolean_literal($1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| LITERAL_SINGLE_BYTE_STRING
{
    if(($$ = st_new_single_string_literal($1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| LITERAL_DOUBLE_BYTE_STRING
{
    if(($$ = st_new_double_string_literal($1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

literal_implicit_type_possible_sign_prefix :
sign_prefix LITERAL_DECIMAL
{
    if(($$ = st_new_integer_literal($2, &@2, $1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| sign_prefix LITERAL_REAL
{
    if(($$ = st_new_real_literal($2, &@2, $1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| literal_implicit_type
;

/* Type declarations */
type_declaration_block :
TYPE type_declarations END_TYPE
{
    $$ = $2;
}
| TYPE END_TYPE 
{
    $$ = NULL;
}
;

type_declarations :
type_declaration
{
    if(($$ = st_append_type_declaration(NULL, $1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| type_declarations type_declaration
{
    if(($$ = st_append_type_declaration($1, $2, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| error ';'
{
    $$ = NULL;
}
| type_declarations error ';'
{
    $$ = $1;
}
;

type_declaration :
derived_type_name ':' derive_type ';'
{
    if(($$ = st_new_derived_type($1, $3, &@$, NULL, NULL, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| derived_type_name ':' simple_type_name ';'
{
    if(($$ = st_new_derived_type_by_name($1, $3, &@3, &@$, NULL, NULL, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| derived_type_name ':' simple_type_name ASSIGN simple_type_initial_value ';'
{
    if(($$ = st_new_derived_type_by_name($1, $3, &@3, &@$, $5, &@5, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

basic_type :
subrange_type
| enum_type
| array_type 
| string_defined_length_type
;

derive_type :
basic_type
| struct_type
;

variable_type :
basic_type
| simple_type_name ASSIGN simple_type_initial_value
{
    if(($$ = st_new_derived_type_by_name(NULL, $1, &@1, &@$, $3, &@3, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| simple_type_name ASSIGN struct_initialization
{
    if(($$ = st_new_derived_type_by_name(NULL, $1, &@1, &@$, $3, &@3, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

simple_type_initial_value :
literal
| IDENTIFIER
{
    if(($$ = st_new_enum_value($1, &@1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

subrange_type :
simple_type_name_not_string '(' subrange ')'
{
    if(($$ = st_new_subrange_type($1, &@1, $3, NULL, NULL, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);			
}
| simple_type_name_not_string '(' subrange ')' ASSIGN literal_implicit_type_possible_sign_prefix
{
    if(($$ = st_new_subrange_type($1, &@1, $3, $6, &@6, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

subrange :
subrange_point DOTDOT subrange_point
{
    if(($$ = st_new_subrange($1, &@1, $3, &@3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

subrange_point : literal_implicit_type_possible_sign_prefix;

enum_type :
'(' enum_items ')'
{
    if(($$ = st_new_enum_type($2, NULL, NULL, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| '(' enum_items ')' ASSIGN IDENTIFIER
{
    if(($$ = st_new_enum_type($2, $5, &@5, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

enum_items :
IDENTIFIER
{
    if(($$ = st_append_new_enum_item(NULL, $1, &@1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| enum_items ',' IDENTIFIER
{
    if(($$ = st_append_new_enum_item($1, $3, &@3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

array_type :
ARRAY '[' array_range ']' OF simple_type_name
{
    if(($$ = st_new_array_type($3, $6, &@6, NULL, NULL, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| ARRAY '[' array_range ']' OF simple_type_name ASSIGN array_initialization
{
    if(($$ = st_new_array_type($3, $6, &@6, $8, &@8, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

array_range :
subrange
{
    if(($$ = st_add_sub_to_new_array_range(NULL, $1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| array_range ',' subrange
{
    if(($$ = st_add_sub_to_new_array_range($1, $3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

array_initialization :
'[' array_initializer ']'
{
    if(($$ = st_new_array_init_value($2, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

array_initializer :
array_initial_element
{
    if(($$ = st_append_initial_element(NULL, $1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| literal_implicit_type '(' array_initial_element ')'
{
    if(($$ = st_append_initial_elements(NULL, $1, $3, &@3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| array_initializer ',' array_initial_element
{
    if(($$ = st_append_initial_element($1, $3, &@3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| array_initializer ',' literal_implicit_type '(' array_initial_element ')'
{
    if(($$ = st_append_initial_elements($1, $3, $5, &@5, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

array_initial_element :
literal_implicit_type_possible_sign_prefix
| IDENTIFIER
{
    if(($$ = st_new_enum_value($1, &@1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| array_initialization
;

struct_type :
STRUCT struct_elements END_STRUCT
{
    if(($$ = st_new_struct_type($2, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

struct_elements :
IDENTIFIER ':' variable_type ';'
{
    if(($$ = st_add_new_struct_element(NULL, $1, &@1, $3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| struct_elements IDENTIFIER ':' variable_type ';'
{
    if(($$ = st_add_new_struct_element($1, $2, &@2, $4, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| IDENTIFIER ':' simple_type_name ';'
{
    if(($$ = st_add_new_struct_element_by_name(NULL, $1, &@1, $3, &@3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| struct_elements IDENTIFIER ':' simple_type_name ';'
{
    if(($$ = st_add_new_struct_element_by_name($1, $2, &@2, $4, &@4, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);											
}
;

struct_initialization :
'(' struct_initializer ')'
{
    if(($$ = st_struct_initializer_value($2, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

struct_initializer:
IDENTIFIER ASSIGN simple_type_initial_value
{
    if(($$ = st_add_new_struct_element_initializer(NULL, $1, &@1, $3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| IDENTIFIER ASSIGN array_initialization
{
    if(($$ = st_add_new_struct_element_initializer(NULL, $1, &@1, $3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| IDENTIFIER ASSIGN struct_initialization
{
    if(($$ = st_add_new_struct_element_initializer(NULL, $1, &@1, $3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| struct_initializer ',' IDENTIFIER ASSIGN simple_type_initial_value
{
    if(($$ = st_add_new_struct_element_initializer($1, $3, &@3, $5, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| struct_initializer ',' IDENTIFIER ASSIGN array_initialization
{
    if(($$ = st_add_new_struct_element_initializer($1, $3, &@3, $5, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}		
| struct_initializer ',' IDENTIFIER ASSIGN struct_initialization
{
    if(($$ = st_add_new_struct_element_initializer($1, $3, &@3, $5, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

string_defined_length :
'[' literal_implicit_type ']' 
{
    $$ = $2;
}
| '(' literal_implicit_type ')'
{
    $$ = $2;
}
;

string_defined_length_type :
STRING_TYPE_NAME string_defined_length
{
    if(($$ = st_new_string_type($1, $2, &@2, NULL, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| STRING_TYPE_NAME string_defined_length ASSIGN literal
{
    if(($$ = st_new_string_type($1, $2, &@2, $4, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

/* Variable declarations */
var_declaration_block :
var_start constant_specifier retain_specifier var_declarations END_VAR
{
    if(($$ = st_new_var_declaration_block($1, $2, $3, $4, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| var_start constant_specifier retain_specifier END_VAR
{
    $$ = NULL;
}
;

var_start :
VAR
{
    $$ = 0;
}
| VAR_INPUT
{
    $$ = INPUT_VAR_CLASS;
}
| VAR_OUTPUT
{
    $$ = OUTPUT_VAR_CLASS;
}
| VAR_TEMP
{
    $$ = TEMP_VAR_CLASS;
}
| VAR_IN_OUT
{
    $$ = IN_OUT_VAR_CLASS;
}
| VAR_EXTERNAL
{
    $$ = EXTERNAL_VAR_CLASS;
}
| VAR_GLOBAL
{
    $$ = GLOBAL_VAR_CLASS;
}
;

constant_specifier :
/* No specifier */
{
    $$ = 0;
}
| CONSTANT
{
    $$ = CONSTANT_VAR_CLASS;
}
;

retain_specifier :
/* No specifier */
{
    $$ = 0;
}
| RETAIN
{
    $$ = RETAIN_VAR_CLASS;
}
| NON_RETAIN
{
    $$ = 0;
}
;

var_declarations :
var_declaration ';'
{    if(($$ = st_append_var_declarations(NULL, $1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| var_declarations var_declaration ';'
{
    if(($$ = st_append_var_declarations($1, $2, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);											
}
| error ';'
{
    $$ = NULL;
}
| var_declarations error ';'
{
    $$ = $1;
}
;

var_list :
IDENTIFIER
{
    if(($$ = st_append_new_var(NULL, $1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| var_list ',' IDENTIFIER
{
    if(($$ = st_append_new_var($1, $3, &@3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

var_declaration :
var_list ':' variable_type
{
    if(($$ = st_finalize_var_list($1, $3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| var_list ':' simple_type_name
{
    if(($$ = st_finalize_var_list_by_name($1, $3, &@3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| var_list ':' simple_type_name_not_string R_EDGE
{
    if(($$ = st_finalize_var_list_by_edge($1, $3, &@3, ESSTEE_TRUE, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| var_list ':' simple_type_name_not_string F_EDGE
{
    if(($$ = st_finalize_var_list_by_edge($1, $3, &@3, ESSTEE_FALSE, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| AT direct_address ':' simple_type_name
{
    if(st_initialize_direct_memory($2, $4, &@4, &@$, parser) == ESSTEE_ERROR)
	DO_ERROR_STRATEGY(parser);
}	
| AT direct_address ':' simple_type_name ASSIGN simple_type_initial_value
{
    if(st_initialize_direct_memory_explicit($2, $4, &@4, $6, &@$, parser) == ESSTEE_ERROR)
	DO_ERROR_STRATEGY(parser);
}
| IDENTIFIER AT direct_address ':' simple_type_name
{
    if(($$ = st_new_direct_var($1, &@1, &@$, $5, &@5, $3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| IDENTIFIER AT direct_address ':' simple_type_name ASSIGN simple_type_initial_value
{
    if(($$ = st_new_direct_var_explicit($1, &@1, &@$, $5, &@5, $3, $7, &@7, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

/* Direct memory */
direct_address :
DIRECT_ADDRESS
{
    if(($$ = st_new_direct_address($1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

/* Statements and expressions */
array_index :
expression
{
    if(($$ = st_new_array_index($1, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| array_index ',' expression
{
    if(($$ = st_extend_array_index($1, $3, &@3, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
;

expression :
expression XOR expression	/*bool + bool*/
{
    if(($$ = st_new_xor_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression AND expression	/*bool + bool*/
{
    if(($$ = st_new_and_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression OR expression	/*bool + bool*/
{
    if(($$ = st_new_or_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression GREATER expression	/*int, real + int, real*/
{
    if(($$ = st_new_greater_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression LESSER expression	/*int, real + int, real*/
{
    if(($$ = st_new_lesser_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression EQUALS expression	/*bool, int, real, string, enumeartion, ... all*/
{
    if(($$ = st_new_equals_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression GEQUALS expression	/*int, real*/
{
    if(($$ = st_new_gequals_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression LEQUALS expression	/*int,real*/
{
    if(($$ = st_new_lequals_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression NEQUALS expression	/*all*/
{
    if(($$ = st_new_nequals_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression '+' expression	/*int, real*/
{
    if(($$ = st_new_plus_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression '-' expression	/*int, real*/
{
    if(($$ = st_new_minus_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression '*' expression	/*int, real*/
{
    if(($$ = st_new_multiply_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression '/' expression	/*int, real*/
{
    if(($$ = st_new_division_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression MOD expression	/*int*/
{
    if(($$ = st_new_mod_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| expression TO_POWER expression /*int*/
{
    if(($$ = st_new_to_power_expression($1, $3, &@$, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| term
;

qualified_identifier :
IDENTIFIER '[' array_index ']'
{
    if(($$ = st_new_qualified_identifier_by_index($1, &@1, $3, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| IDENTIFIER '.' IDENTIFIER
{
    if(($$ = st_new_qualified_identifier_by_sub_ref($1, &@1, $3, &@3, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| qualified_identifier '.' IDENTIFIER '[' array_index ']'
{
    if(($$ = st_extend_qualified_identifier_by_index($1, $3, &@3, $5, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| qualified_identifier '.' IDENTIFIER
{
    if(($$ = st_extend_qualified_identifier_by_sub_ref($1, $3, &@3, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

term :
literal_no_sign_prefix
{
    if(($$ = st_new_expression_value($1, &@1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| direct_address
{
    if(($$ = st_new_direct_address_term($1, &@1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| IDENTIFIER
{
    if(($$ = st_new_single_identifier_term($1, &@1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| qualified_identifier
{
    if(($$ = st_new_qualified_identifier_term($1, &@1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| IDENTIFIER '(' possibly_no_invoke_parameters ')'
{
    if(($$ = st_new_function_invocation_term($1, &@1, $3, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| '-' term
{
    if(($$ = st_new_negate_term($2, &@1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| NOT term
{
    if(($$ = st_new_not_term($2, &@1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| '(' expression ')'
{
    $$ = $2;
}
;

invoke_parameters :
invoke_parameter
{
    if(($$ = st_append_invoke_parameter(NULL, $1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);

}
| invoke_parameters ',' invoke_parameter
{
    if(($$ = st_append_invoke_parameter($1, $3, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

invoke_parameter :
IDENTIFIER ASSIGN expression
{
    if(($$ = st_new_invoke_parameter($1, &@1, $3, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| expression
{
    if(($$ = st_new_invoke_parameter(NULL, &@1, $1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

possibly_no_invoke_parameters :
/* No invoke parameters */
{
    $$ = NULL;
}
| invoke_parameters
;

statements :
';'
{
    if(($$ = st_new_empty_statement(NULL, &@1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| statements ';'
{
    if(($$ = st_new_empty_statement($1, &@2, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| statement ';'
{
    if(($$ = st_append_to_statement_list(NULL, $1, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| statements statement ';'
{
    if(($$ = st_append_to_statement_list($1, $2, parser)) == NULL)
	DO_ERROR_STRATEGY(parser);
}
| error ';'
{
    $$ = NULL;
}
| statements error ';'
{
    $$ = $1;
}
;

statement :
IDENTIFIER ASSIGN expression
{
    if(($$ = st_new_assignment_statement_simple($1, &@1, $3, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| qualified_identifier ASSIGN expression
{
    if(($$ = st_new_assignment_statement_qualified($1, $3, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| IDENTIFIER '(' possibly_no_invoke_parameters ')'
{
    if(($$ = st_new_invoke_statement($1, &@1, $3, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| IF expression THEN statements elsif_clauses else_clause END_IF
{
    if(($$ = st_new_if_statement($2, &@2, $4, $5, $6, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| CASE expression OF cases else_clause END_CASE
{
    if(($$ = st_new_case_statement($2, $4, $5, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| for_statement
| WHILE expression DO {parser->loop_level++;} statements {parser->loop_level--;} END_WHILE
{
    if(($$ = st_new_while_statement($2, $5, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| REPEAT {parser->loop_level++;} statements {parser->loop_level--;} UNTIL expression END_REPEAT
{
    if(($$ = st_new_repeat_statement($6, $3, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| EXIT
{
    if(($$ = st_new_exit_statement(&@1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| RETURN
{
    if(($$ = st_new_return_statement(&@1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

elsif_clauses :
/* No elsif */
{
    $$ = NULL;
}
| elsif_clauses elsif_clause
{
    if(($$ = st_append_elsif_clause($1, $2, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

elsif_clause :
ELSIF expression THEN statements
{
    if(($$ = st_new_elsif_clause($2, &@2, $4, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

else_clause :
/* No else */
{
    $$ = NULL;
}
| ELSE statements
{
    $$ = $2;
}
;

cases :
a_case
{
    if(($$ = st_append_case(NULL, $1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| cases a_case
{
    if(($$ = st_append_case($1, $2, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

a_case :
case_list ':' statements
{
    if(($$ = st_new_case($1, &@1, $3, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

case_list :
case_list_element
{
    if(($$ = st_append_case_value(NULL, $1, &@1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| case_list ',' case_list_element
{
    if(($$ = st_append_case_value($1, $3, &@3, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

case_list_element :
subrange
{
    if(($$ = st_new_subrange_case_value($1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| literal
| IDENTIFIER_CASE_LABEL
{
    if(($$ = st_new_enum_value($1, &@1, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

for_statement :
FOR IDENTIFIER ASSIGN expression TO expression DO {parser->loop_level++;} statements {parser->loop_level--;} END_FOR
{
    if(($$ = st_new_for_statement($2, &@2, $4, $6, NULL, $9, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
| FOR IDENTIFIER ASSIGN expression TO expression BY expression DO statements END_FOR
{
    if(($$ = st_new_for_statement($2, &@2, $4, $6, $8, $10, &@$, parser)) == NULL)
    	DO_ERROR_STRATEGY(parser);
}
;

%%
void yyerror(
	YYLTYPE *location, 
	yyscan_t yyscanner, 
	struct parser_t *parser, 
	const char *description)
{
    parser->errors->new_issue_at(
	parser->errors,
	description,
	ESSTEE_SYNTAX_ERROR,
	1,
	location);
}
