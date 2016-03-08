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

#define ESSTEE_OK                  0
#define ESSTEE_TRUE                0
#define ESSTEE_ERROR               -1
#define ESSTEE_FALSE               -2

#define ESSTEE_DIVISION_BY_ZERO   -10
#define ESSTEE_TYPE_OVERFLOW      -11
#define ESSTEE_TYPE_UNDERFLOW     -12
#define ESSTEE_TYPE_INCOMPATIBILITY -13
