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
n
You should have received a copy of the GNU General Public License
along with esstee.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <elements/itype.h>
#include <elements/ivariable.h>
#include <util/iconfig.h>
#include <util/iissues.h>
#include <rt/icursor.h>
#include <rt/isystime.h>

struct program_iface_t {

    int (*finalize_header)(
	struct program_iface_t *self,
	struct type_iface_t *global_type_table,
	struct variable_iface_t *global_var_table,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*finalize_statements)(
	struct program_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*start)(
	struct program_iface_t *self,
	struct cursor_iface_t *cursor,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);	

    int (*run_cycle)(
	struct program_iface_t *self,
	struct cursor_iface_t *cursor,
	struct systime_iface_t *systime,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    struct variable_iface_t * (*variable)(
	struct program_iface_t *self,
	const char *variable_identifier,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*display)(
	struct program_iface_t *self,
	char *buffer,
	size_t buffer_size,
	const struct config_iface_t *config);
    
    void (*destroy)(
	struct program_iface_t *self);

    const char *identifier;
    const struct st_location_t *location;
    struct program_iface_t *prev;
    struct program_iface_t *next;
    UT_hash_handle hh;
};
