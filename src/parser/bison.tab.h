/* A Bison parser, made by GNU Bison 3.0.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_SRC_PARSER_BISON_TAB_H_INCLUDED
# define YY_YY_SRC_PARSER_BISON_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 33 "src/parser/bison.y" /* yacc.c:1909  */

#include <esstee/locations.h>
typedef struct st_location_t YYLTYPE;

#define YYLTYPE_IS_DECLARED 1
#define YYLTYPE_IS_TRIVIAL 0

typedef void* yyscan_t; /* Type is defined in flex.h, but flex.h depends on YYLTYPE */
struct parser_t;
#include <util/bitflag.h>

#line 56 "src/parser/bison.tab.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    QUERY_MODE = 258,
    GREATER = 259,
    LESSER = 260,
    GEQUALS = 261,
    LEQUALS = 262,
    EQUALS = 263,
    NEQUALS = 264,
    TO_POWER = 265,
    AND = 266,
    OR = 267,
    NOT = 268,
    XOR = 269,
    MOD = 270,
    unary_minus = 271,
    PROGRAM = 272,
    END_PROGRAM = 273,
    FUNCTION_BLOCK = 274,
    END_FUNCTION_BLOCK = 275,
    FUNCTION = 276,
    END_FUNCTION = 277,
    VAR = 278,
    VAR_INPUT = 279,
    VAR_OUTPUT = 280,
    VAR_TEMP = 281,
    VAR_IN_OUT = 282,
    VAR_EXTERNAL = 283,
    VAR_GLOBAL = 284,
    END_VAR = 285,
    OF = 286,
    ARRAY = 287,
    DIRECT_ADDRESS = 288,
    CONSTANT = 289,
    R_EDGE = 290,
    F_EDGE = 291,
    RETAIN = 292,
    NON_RETAIN = 293,
    AT = 294,
    TYPE = 295,
    END_TYPE = 296,
    STRUCT = 297,
    END_STRUCT = 298,
    ELEMENTARY_TYPE_NAME = 299,
    STRING_TYPE_NAME = 300,
    IF = 301,
    THEN = 302,
    END_IF = 303,
    ELSIF = 304,
    ELSE = 305,
    CASE = 306,
    END_CASE = 307,
    RETURN = 308,
    EXIT = 309,
    ASSIGN = 310,
    FOR = 311,
    END_FOR = 312,
    TO = 313,
    BY = 314,
    DO = 315,
    WHILE = 316,
    END_WHILE = 317,
    REPEAT = 318,
    END_REPEAT = 319,
    UNTIL = 320,
    DOTDOT = 321,
    LITERAL_BOOLEAN_TRUE = 322,
    LITERAL_BOOLEAN_FALSE = 323,
    IDENTIFIER = 324,
    IDENTIFIER_CASE_LABEL = 325,
    IDENTIFER_TYPE_SPEC_LITERAL = 326,
    LITERAL_DECIMAL = 327,
    LITERAL_BINARY = 328,
    LITERAL_OCTAL = 329,
    LITERAL_HEX = 330,
    LITERAL_REAL = 331,
    LITERAL_DURATION = 332,
    LITERAL_TOD = 333,
    LITERAL_DATE = 334,
    LITERAL_DATE_TOD = 335,
    LITERAL_SINGLE_BYTE_STRING = 336,
    LITERAL_DOUBLE_BYTE_STRING = 337
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 94 "src/parser/bison.y" /* yacc.c:1909  */

    char *string;

    struct type_iface_t *type;
    struct variable_t *variable;
    struct header_t *header;

    struct value_iface_t *value;
    struct listed_value_t *listed_value;
    struct enum_item_t *enum_item;
    struct array_init_value_t *array_init_value;
    struct struct_init_value_t *struct_init_value;

    struct subrange_t *subrange;
    struct array_range_t *array_range;
    struct struct_element_init_t *struct_element_init;
    struct struct_element_t *struct_element;
    
    struct direct_address_t *direct_address;
    struct array_index_t *array_index;
    struct qualified_identifier_t *qualified_identifier;
    
    struct expression_iface_t *expression;
    struct invoke_iface_t *invoke;
    struct invoke_parameter_t *invoke_parameter;
    struct case_t *case_clause;
    struct if_statement_t *if_statement;
    struct case_list_element_t *case_list;
    
    st_bitflag_t bitflag;
    int64_t integer;

#line 184 "src/parser/bison.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (yyscan_t yyscanner, struct parser_t *parser);

#endif /* !YY_YY_SRC_PARSER_BISON_TAB_H_INCLUDED  */
