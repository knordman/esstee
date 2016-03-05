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

#include <rt/cursor.h>
#include <elements/iinvoke.h>

#include <utlist.h>


void st_switch_current(
    struct cursor_t *cursor,
    struct invoke_iface_t *switch_to,
    const struct config_iface_t *config)
{
    /* Prepare a new run of the invoke switched to */
    switch_to->reset(switch_to, config);
    
    DL_PREPEND2(cursor->call_stack,
		cursor->current,
		call_stack_prev,
		call_stack_next);
    cursor->current = switch_to;
}