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

#include <inttypes.h>

typedef uint64_t st_bitflag_t;

#define ESSTEE_OK                               0
#define ESSTEE_TRUE                             0
#define ESSTEE_ERROR                           -1
#define ESSTEE_FALSE                           -2

#define ESSTEE_DIVISION_BY_ZERO               -10
#define ESSTEE_TYPE_OVERFLOW                  -11
#define ESSTEE_TYPE_UNDERFLOW                 -12
#define ESSTEE_TYPE_INCOMPATIBILITY           -13

#define ESSTEE_GENERAL_ERROR_ISSUE        (1 << 0)
#define ESSTEE_GENERAL_WARNING_ISSUE      (1 << 1)

#define ESSTEE_INTERNAL_ERROR             (1 << 0)
#define ESSTEE_MEMORY_ERROR               (1 << 1)
#define ESSTEE_SYNTAX_ERROR               (1 << 2)
#define ESSTEE_PARSE_ERROR                (1 << 3)
#define ESSTEE_LINK_ERROR                 (1 << 4)
#define ESSTEE_RUNTIME_ERROR              (1 << 5)
#define ESSTEE_ARGUMENT_ERROR             (1 << 6)
#define ESSTEE_CONTEXT_ERROR              (1 << 7)
#define ESSTEE_TYPE_ERROR                 (1 << 8)
#define ESSTEE_IO_ERROR                   (1 << 9)

#define ESSTEE_SYNTAX_WARNING            (1 << 10)
#define ESSTEE_BUFFER_WARNING            (1 << 11)

#define ESSTEE_FILTER_ANY_ERROR              0x3ff
#define ESSTEE_FILTER_ANY_WARNING         (1 << 9)
#define ESSTEE_FILTER_ANY_ISSUE 0xffffffffffffffff

#define ISSUE_ERROR_CLASS                 (1 << 0)
#define ISSUE_WARNING_CLASS               (1 << 1)

#define ESSTEE_TYPE_INTEGER_NUMERIC_CLASS (1 << 0)
#define ESSTEE_TYPE_INTEGER_BITDATA_CLASS (1 << 1)
#define ESSTEE_TYPE_REAL_NUMERIC_CLASS    (1 << 2)
#define ESSTEE_TYPE_INTEGER_SIGNED        (1 << 3)
#define ESSTEE_TYPE_INTEGER_UNSIGNED      (1 << 4)
#define ESSTEE_TYPE_BOOL                  (1 << 5)
#define ESSTEE_TYPE_SINT                  (1 << 6)
#define ESSTEE_TYPE_INT                   (1 << 7)
#define ESSTEE_TYPE_DINT                  (1 << 8)
#define ESSTEE_TYPE_LINT                  (1 << 9)
#define ESSTEE_TYPE_USINT                 (1 << 10)
#define ESSTEE_TYPE_UINT                  (1 << 11)
#define ESSTEE_TYPE_UDINT                 (1 << 12)
#define ESSTEE_TYPE_ULINT                 (1 << 13)
#define ESSTEE_TYPE_BYTE                  (1 << 14)
#define ESSTEE_TYPE_WORD                  (1 << 15)
#define ESSTEE_TYPE_DWORD                 (1 << 16)
#define ESSTEE_TYPE_LWORD                 (1 << 17)
#define ESSTEE_TYPE_REAL                  (1 << 18)
#define ESSTEE_TYPE_LREAL                 (1 << 19)
#define ESSTEE_TYPE_STRING                (1 << 20)
#define ESSTEE_TYPE_WSTRING               (1 << 21)
#define ESSTEE_TYPE_DURATION              (1 << 22)
#define ESSTEE_TYPE_DATE                  (1 << 23)
#define ESSTEE_TYPE_TOD                   (1 << 24)
#define ESSTEE_TYPE_DATE_TOD              (1 << 25)
#define ESSTEE_TYPE_DERIVED               (1 << 26)
#define ESSTEE_TYPE_ENUM                  (1 << 27)
#define ESSTEE_TYPE_SUBRANGE              (1 << 28)
#define ESSTEE_TYPE_ARRAY                 (1 << 29)
#define ESSTEE_TYPE_STRUCT                (1 << 30)
#define ESSTEE_TYPE_FB                    (1 << 31)
