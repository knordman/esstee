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

#include <elements/ivalue.h>
#include <elements/iinvoke.h>

#define EXPR_CLASS_COMPILETIME_DEFINED (1 << 0)

struct expression_iface_t {

    struct invoke_iface_t invoke;
	
    const struct value_iface_t * (*return_value)(
	struct expression_iface_t *self);

    st_bitflag_t (*class)(
	struct expression_iface_t *self);
    
    void (*destroy)(
	struct expression_iface_t *self);
};
