/*
Copyright (C) 2016 Kristian Nordman

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

#include <esstee/elements.h>
#include <elements/ivalue.h>
#include <util/iissues.h>
#include <util/iconfig.h>
#include <util/macros.h>

#include <uthash.h>

struct element_node_context_t {
    struct issues_iface_t *issues;
    const struct config_iface_t *config;
};

struct element_node_t {
    struct st_element_t element;
    const struct element_node_context_t *context;
    struct value_iface_t *value;

    struct element_node_t *sub_nodes;

    char *identifier;
    UT_hash_handle hh;
};

struct element_node_t * st_new_element_node(
    const char *identifier,
    struct value_iface_t *value,
    const struct element_node_context_t *context);
