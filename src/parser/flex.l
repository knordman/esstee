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

%option noyywrap nounput noinput noyy_top_state
%pointer
%option case-insensitive
%option yylineno
%option stack
%option bison-bridge bison-locations
%option reentrant
%option extra-type="struct scanner_options_t *"

%s case_state
%x comment_state

%{
#include <stdio.h>
#include <string.h>
#include <parser/scanneroptions.h>
#include <parser/bison.tab.h>

/* #define RETURN(X)                                    \ */
/*     do {                                             \ */
/*      fprintf(stderr, "%s : %s\n", yytext, #X);       \ */
/*      return X;                                       \ */
/*     } while(0) */

#define RETURN(X) return X;

#define RETURN_AS_STRING(X)                     \
    do {                                        \
        yylval->string = strdup(yytext);        \
        RETURN(X);                              \
    } while(0)


#define YY_USER_ACTION  yylloc->first_line = yylloc->last_line = yylineno; \
    yylloc->first_column = (yycolumn == 0) ? 1 : yycolumn;              \
    yylloc->last_column = yylloc->first_column+yyleng;                  \
    yyset_column(yylloc->first_column+yyleng, yyscanner);

%}

/* Comment block */
comment_begin                   \(\*
comment_end                     \*\)

/* Misc */
direct_address                  %[IQM][XBWDL]?[[:digit:]]+(\.([[:digit:]]+|\*))?

/* White space */
white_space                     ([[:space:]]{-}[\n])+

/* Numeric literals */
integer_literal                 [[:digit:]](_|[[:digit:]])*

real_literal                    [[:digit:]]+\.[[:digit:]]+
real_literal_exp                {real_literal}e[+|-]?{integer_literal}

base2_literal                   2#[01]+(_|[01])*
base8_literal                   8#[0-7]+(_|[0-7])*
hex_digit                       [[:digit:]a-f]
base16_literal                  16#({hex_digit})+(_|{hex_digit})*

boolean_true                    true
boolean_false                   false

/* Strings */
common_string                   [[:print:]]{-}[$'\"]|$$|$l|$n|$p|$r|$t
single_string_literal           '({common_string}|\"|${hex_digit}{hex_digit})*'
double_string_literal		\"({common_string}|'|${hex_digit}{hex_digit}{hex_digit}{hex_digit})*\"

/* Duration literals */
dur_number                      [[:digit:]]+(\.[[:digit:]]*)?
duration_part                   {dur_number}d|{dur_number}h|{dur_number}m|{dur_number}s|{dur_number}ms
duration_literal                (t|time)#{duration_part}(_?{duration_part}){0,4}

/* Date and time literals */
date_numbers                    [[:digit:]]{4}-[[:digit:]]{2}-[[:digit:]]{2}
tod_numbers                     [[:digit:]]{2}:[[:digit:]]{2}:[[:digit:]]{2}.[[:digit:]]{2}
date_literal                    (d|date)#{date_numbers}
tod_literal                     (tod|time_of_day)#{tod_numbers}
date_tod_literal                (dt|date_and_time)#{date_numbers}-{tod_numbers}

/* Identifiers */
identifier                      _*[[:alpha:]](_|[[:alnum:]])*

%%

%{
    if(yyextra->query_mode_start)
    {
	yyextra->query_mode_start = 0;
	RETURN(QUERY_MODE);
    }
%}

{white_space}                                           /* Ignore */
\n                                                      yyset_column(1, yyscanner);

{boolean_true}                                          {
                                                            yylval->integer = 1;
                                                            RETURN(LITERAL_BOOLEAN_TRUE);
                                                        }
{boolean_false}                                         {
                                                            yylval->integer = 0;
                                                            RETURN(LITERAL_BOOLEAN_FALSE);
                                                        }
{integer_literal}                                       RETURN_AS_STRING(LITERAL_DECIMAL);
{base2_literal}                                         RETURN_AS_STRING(LITERAL_BINARY);
{base8_literal}                                         RETURN_AS_STRING(LITERAL_OCTAL);
{base16_literal}                                        RETURN_AS_STRING(LITERAL_HEX);
{real_literal}                                          |
{real_literal_exp}                                      RETURN_AS_STRING(LITERAL_REAL);
{single_string_literal}                                 RETURN_AS_STRING(LITERAL_SINGLE_BYTE_STRING);
{double_string_literal}                                 RETURN_AS_STRING(LITERAL_DOUBLE_BYTE_STRING);
{duration_literal}                                      RETURN_AS_STRING(LITERAL_DURATION);
{date_literal}                                          RETURN_AS_STRING(LITERAL_DATE);
{tod_literal}                                           RETURN_AS_STRING(LITERAL_TOD);
{date_tod_literal}                                      RETURN_AS_STRING(LITERAL_DATE_TOD);

#                                                       RETURN('#');

bool                                                    RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
sint                                                    RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
int                                                     RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
dint                                                    RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
lint                                                    RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
usint                                                   RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
uint                                                    RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
udint                                                   RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
ulint                                                   RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
real                                                    RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
lreal                                                   RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
time                                                    RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
date                                                    RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
time_of_day                                             RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
tod                                                     RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
date_and_time                                           RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
dt                                                      RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
byte                                                    RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
word                                                    RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
dword                                                   RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
lword                                                   RETURN_AS_STRING(ELEMENTARY_TYPE_NAME);
string                                                  RETURN_AS_STRING(STRING_TYPE_NAME);
wstring                                                 RETURN_AS_STRING(STRING_TYPE_NAME);

program                                                 RETURN(PROGRAM);
end_program                                             RETURN(END_PROGRAM);
function_block                                          RETURN(FUNCTION_BLOCK);
end_function_block                                      RETURN(END_FUNCTION_BLOCK);
function                                                RETURN(FUNCTION);
end_function                                            RETURN(END_FUNCTION);

if                                                      RETURN(IF);
end_if                                                  RETURN(END_IF);
else                                                    RETURN(ELSE);
elsif                                                   RETURN(ELSIF);
then                                                    RETURN(THEN);
case                                                    {
                                                            yy_push_state(case_state, yyscanner); 
                                                            RETURN(CASE);
                                                        }
end_case                                                {
                                                            yy_pop_state(yyscanner);
          					            RETURN(END_CASE);
                                                        }
or                                                      RETURN(OR);
and                                                     RETURN(AND);
&                                                       RETURN(AND);
not                                                     RETURN(NOT);
xor                                                     RETURN(XOR);
\+                                                      RETURN('+');
-                                                       RETURN('-');
\/                                                      RETURN('/');
\*                                                      RETURN('*');
\*\*                                                    RETURN(TO_POWER);

\>                                                      RETURN(GREATER);
\<                                                      RETURN(LESSER);
\>=                                                     RETURN(GEQUALS);
\<=                                                     RETURN(LEQUALS);
=                                                       RETURN(EQUALS);
\<\>                                                    RETURN(NEQUALS);

:=                                                      RETURN(ASSIGN);
\.\.                                                    RETURN(DOTDOT);
\.                                                      RETURN('.');
:                                                       RETURN(':');
;                                                       RETURN(';');
,                                                       RETURN(',');
\(                                                      RETURN('(');
\)                                                      RETURN(')');
\[                                                      RETURN('[');
\]                                                      RETURN(']');

type                                                    RETURN(TYPE);
end_type                                                RETURN(END_TYPE);

struct                                                  RETURN(STRUCT);
end_struct                                              RETURN(END_STRUCT);

var                                                     RETURN(VAR);
var_input                                               RETURN(VAR_INPUT);
var_output                                              RETURN(VAR_OUTPUT);
var_temp                                                RETURN(VAR_TEMP);
var_in_out                                              RETURN(VAR_IN_OUT);
var_external                                            RETURN(VAR_EXTERNAL);
var_global                                              RETURN(VAR_GLOBAL);
end_var                                                 RETURN(END_VAR);

retain                                                  RETURN(RETAIN);
non_retain                                              RETURN(NON_RETAIN);
at                                                      RETURN(AT);
of                                                      RETURN(OF);
r_edge                                                  RETURN(R_EDGE);
f_edge                                                  RETURN(F_EDGE);
constant                                                RETURN(CONSTANT);

array                                                   RETURN(ARRAY);
{direct_address}                                        RETURN_AS_STRING(DIRECT_ADDRESS);

return                                                  RETURN(RETURN);
exit                                                    RETURN(EXIT);
for                                                     RETURN(FOR);
end_for                                                 RETURN(END_FOR);
to                                                      RETURN(TO);
by                                                      RETURN(BY);
do                                                      RETURN(DO);
while                                                   RETURN(WHILE);
end_while                                               RETURN(END_WHILE);
repeat                                                  RETURN(REPEAT);
end_repeat                                              RETURN(END_REPEAT);
until                                                   RETURN(UNTIL);

<case_state>{identifier}/([ \f\n\r\t\v]*:[[:space:]]+)	RETURN_AS_STRING(IDENTIFIER_CASE_LABEL);
{identifier}/#                                          RETURN_AS_STRING(IDENTIFER_TYPE_SPEC_LITERAL);
{identifier}                                            RETURN_AS_STRING(IDENTIFIER);

{comment_begin}						yy_push_state(comment_state, yyscanner);
<comment_state>{comment_end}                            yy_pop_state(yyscanner);
<comment_state>.|\n                                     /* Ignore the whole comment */

.                                                       RETURN(yytext[0]);

%%
