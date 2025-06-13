/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

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
#line 33 "src/parser/bison.y"

#include <esstee/locations.h>
typedef struct st_location_t YYLTYPE;

#define YYLTYPE_IS_DECLARED 1
#define YYLTYPE_IS_TRIVIAL 0

typedef void* yyscan_t; /* Type is defined in flex.h, but flex.h depends on YYLTYPE */
struct parser_t;
#include <util/bitflag.h>

#line 61 "src/parser/bison.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    QUERY_MODE = 258,              /* QUERY_MODE  */
    GREATER = 259,                 /* GREATER  */
    LESSER = 260,                  /* LESSER  */
    GEQUALS = 261,                 /* GEQUALS  */
    LEQUALS = 262,                 /* LEQUALS  */
    EQUALS = 263,                  /* EQUALS  */
    NEQUALS = 264,                 /* NEQUALS  */
    TO_POWER = 265,                /* TO_POWER  */
    AND = 266,                     /* AND  */
    OR = 267,                      /* OR  */
    NOT = 268,                     /* NOT  */
    XOR = 269,                     /* XOR  */
    MOD = 270,                     /* MOD  */
    unary_minus = 271,             /* unary_minus  */
    PROGRAM = 272,                 /* PROGRAM  */
    END_PROGRAM = 273,             /* END_PROGRAM  */
    FUNCTION_BLOCK = 274,          /* FUNCTION_BLOCK  */
    END_FUNCTION_BLOCK = 275,      /* END_FUNCTION_BLOCK  */
    FUNCTION = 276,                /* FUNCTION  */
    END_FUNCTION = 277,            /* END_FUNCTION  */
    VAR = 278,                     /* VAR  */
    VAR_INPUT = 279,               /* VAR_INPUT  */
    VAR_OUTPUT = 280,              /* VAR_OUTPUT  */
    VAR_TEMP = 281,                /* VAR_TEMP  */
    VAR_IN_OUT = 282,              /* VAR_IN_OUT  */
    VAR_EXTERNAL = 283,            /* VAR_EXTERNAL  */
    VAR_GLOBAL = 284,              /* VAR_GLOBAL  */
    END_VAR = 285,                 /* END_VAR  */
    OF = 286,                      /* OF  */
    ARRAY = 287,                   /* ARRAY  */
    DIRECT_ADDRESS = 288,          /* DIRECT_ADDRESS  */
    CONSTANT = 289,                /* CONSTANT  */
    R_EDGE = 290,                  /* R_EDGE  */
    F_EDGE = 291,                  /* F_EDGE  */
    RETAIN = 292,                  /* RETAIN  */
    NON_RETAIN = 293,              /* NON_RETAIN  */
    AT = 294,                      /* AT  */
    TYPE = 295,                    /* TYPE  */
    END_TYPE = 296,                /* END_TYPE  */
    STRUCT = 297,                  /* STRUCT  */
    END_STRUCT = 298,              /* END_STRUCT  */
    ELEMENTARY_TYPE_NAME = 299,    /* ELEMENTARY_TYPE_NAME  */
    STRING_TYPE_NAME = 300,        /* STRING_TYPE_NAME  */
    IF = 301,                      /* IF  */
    THEN = 302,                    /* THEN  */
    END_IF = 303,                  /* END_IF  */
    ELSIF = 304,                   /* ELSIF  */
    ELSE = 305,                    /* ELSE  */
    CASE = 306,                    /* CASE  */
    END_CASE = 307,                /* END_CASE  */
    RETURN = 308,                  /* RETURN  */
    EXIT = 309,                    /* EXIT  */
    ASSIGN = 310,                  /* ASSIGN  */
    FOR = 311,                     /* FOR  */
    END_FOR = 312,                 /* END_FOR  */
    TO = 313,                      /* TO  */
    BY = 314,                      /* BY  */
    DO = 315,                      /* DO  */
    WHILE = 316,                   /* WHILE  */
    END_WHILE = 317,               /* END_WHILE  */
    REPEAT = 318,                  /* REPEAT  */
    END_REPEAT = 319,              /* END_REPEAT  */
    UNTIL = 320,                   /* UNTIL  */
    DOTDOT = 321,                  /* DOTDOT  */
    LITERAL_BOOLEAN_TRUE = 322,    /* LITERAL_BOOLEAN_TRUE  */
    LITERAL_BOOLEAN_FALSE = 323,   /* LITERAL_BOOLEAN_FALSE  */
    IDENTIFIER = 324,              /* IDENTIFIER  */
    IDENTIFIER_CASE_LABEL = 325,   /* IDENTIFIER_CASE_LABEL  */
    IDENTIFER_TYPE_SPEC_LITERAL = 326, /* IDENTIFER_TYPE_SPEC_LITERAL  */
    LITERAL_DECIMAL = 327,         /* LITERAL_DECIMAL  */
    LITERAL_BINARY = 328,          /* LITERAL_BINARY  */
    LITERAL_OCTAL = 329,           /* LITERAL_OCTAL  */
    LITERAL_HEX = 330,             /* LITERAL_HEX  */
    LITERAL_REAL = 331,            /* LITERAL_REAL  */
    LITERAL_DURATION = 332,        /* LITERAL_DURATION  */
    LITERAL_TOD = 333,             /* LITERAL_TOD  */
    LITERAL_DATE = 334,            /* LITERAL_DATE  */
    LITERAL_DATE_TOD = 335,        /* LITERAL_DATE_TOD  */
    LITERAL_SINGLE_BYTE_STRING = 336, /* LITERAL_SINGLE_BYTE_STRING  */
    LITERAL_DOUBLE_BYTE_STRING = 337 /* LITERAL_DOUBLE_BYTE_STRING  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 104 "src/parser/bison.y"

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

#line 196 "src/parser/bison.tab.h"

};
typedef union YYSTYPE YYSTYPE;
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
