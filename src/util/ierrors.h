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

#include <util/bitflag.h>
#include <esstee/flags.h>
#include <esstee/issues.h>

struct errors_iface_t {

    void (*new_issue)(
	struct errors_iface_t *self,
	const char *format,
	st_bitflag_t issue_class,
	...);
	
    void (*new_issue_at)(
	struct errors_iface_t *self,
	const char *message,
	st_bitflag_t issue_class,
	int location_count,
	...);

    const char * (*build_message)(
	struct errors_iface_t *self,
	const char *format,
	...);
        
    void (*memory_error)(
	struct errors_iface_t *self,
	const char *file,
	const char *function,
	int line);

    void (*internal_error)(
	struct errors_iface_t *self,
	const char *file,
	const char *function,
	int line);

    const char * (*build_string)(
	const char *format,
	...);

    void (*begin_group)(
	struct errors_iface_t *self);

    void (*set_group_location)(
	struct errors_iface_t self,
	int location_count,
	...);

    void (*ignore_issues)(
	struct errors_iface_t *self);
        
    const struct st_issue_t * (*next_issue)(
	struct errors_iface_t *self,
	st_bitflag_t issue_class_filter);

    const struct st_issue_t * (*next_issue_and_ignore)(
	struct errors_iface_t *self,
	st_bitflag_t issue_class_filter);

    struct errors_iface_t * (*merge)(
	struct errors_iface_t *self,
	struct errors_iface_t *to_merge);

    int (*reset)(
	struct errors_iface_t *self);

    int (*new_error_occured)(
	struct errors_iface_t *self);

    int (*error_count)(
	struct errors_iface_t *self);
};
