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

#include <elements/itype.h>
#include <elements/variables.h>
#include <elements/shared.h>
#include <util/iissues.h>
#include <util/iconfig.h>
#include <rt/isystime.h>
#include <rt/cursor.h>
#include <esstee/locations.h>

#include <uthash.h>

struct function_iface_t {

    int (*finalize_header)(
	struct function_iface_t *self,
	struct type_iface_t *global_type_table,
	struct variable_t *global_var_table,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*finalize_statements)(
	struct function_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*verify_invoke)(
	struct function_iface_t *self,
	struct invoke_parameter_t *parameters,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*step)(
	struct function_iface_t *self,
	struct invoke_parameter_t *parameters,
	struct cursor_t *cursor,
	const struct systime_iface_t *time,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*reset)(
	struct function_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    struct value_iface_t * (*result_value)(
	struct function_iface_t *self);

    void (*destroy)(
	struct function_iface_t *self);

    char *identifier;
    struct st_location_t *location;
    struct function_iface_t *prev;
    struct function_iface_t *next;
    UT_hash_handle hh;
};
