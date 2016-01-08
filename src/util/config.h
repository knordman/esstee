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

#include <util/iconfig.h>

#include <uthash.h>

struct bool_option_t {
    const char *option;
    int value;

    UT_hash_handle hh;
};

struct config_t {
    struct config_iface_t config;
    struct bool_option_t *options_table;
    struct bool_option_t *options_chunk;
};

int st_config_get(
    const struct config_iface_t *self,
    const char *option);

int st_config_set(
    struct config_iface_t *self,
    const char *option,
    int value);

struct config_iface_t * st_new_config(void);

void st_destroy_config(
    struct config_iface_t *self);
