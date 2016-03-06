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

#include <esstee/locations.h>
#include <util/iconfig.h>

struct invoke_iface_t;

struct cursor_t {

    struct invoke_iface_t *call_stack;
    struct invoke_iface_t *exit_context;
    struct invoke_iface_t *return_context;
    struct invoke_iface_t *current;
    
};

void st_switch_current(
    struct cursor_t *cursor,
    struct invoke_iface_t *switch_to,
    const struct config_iface_t *config);
    
void st_push_return_context(
    struct cursor_t *cursor,
    struct invoke_iface_t *context);

void st_pop_return_context(
    struct cursor_t *cursor);

int st_jump_return(
    struct cursor_t *cursor);

void st_push_exit_context(
    struct cursor_t *cursor,
    struct invoke_iface_t *context);

void st_pop_exit_context(
    struct cursor_t *cursor);

void st_jump_exit(
    struct cursor_t *cursor);
