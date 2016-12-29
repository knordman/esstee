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

struct issue_group_iface_t {

    void (*close)(
    	struct issue_group_iface_t *self);
    
    void (*main_issue)(
	struct issue_group_iface_t *self,
	const char *message,
	st_bitflag_t issue_class,
	int location_count,
	...);

};

struct issues_iface_t {

    void (*new_issue)(
	struct issues_iface_t *self,
	const char *format,
	st_bitflag_t issue_class,
	...);
	
    void (*new_issue_at)(
	struct issues_iface_t *self,
	const char *message,
	st_bitflag_t issue_class,
	int location_count,
	...);

    const char * (*build_message)(
	struct issues_iface_t *self,
	const char *format,
	...);
        
    void (*memory_error)(
	struct issues_iface_t *self,
	const char *file,
	const char *function,
	int line);

    void (*internal_error)(
	struct issues_iface_t *self,
	const char *file,
	const char *function,
	int line);

    struct issue_group_iface_t * (*open_group)(
    	struct issues_iface_t *self);

    void (*ignore_all)(
	struct issues_iface_t *self);
        
    const struct st_issue_t * (*fetch)(
	struct issues_iface_t *self,
	st_bitflag_t issue_filter);

    const struct st_issue_t * (*fetch_sub_issue)(
	struct issues_iface_t *self,
	const struct st_issue_t *issue,
	st_bitflag_t issue_filter);
    
    const struct st_issue_t * (*fetch_and_ignore)(
	struct issues_iface_t *self,
	st_bitflag_t issue_filter);

    struct issues_iface_t * (*merge)(
	struct issues_iface_t *self,
	struct issues_iface_t *to_merge);

    int (*unfetched_issues)(
	struct issues_iface_t *self,
	st_bitflag_t issue_filter);

    int (*count)(
	struct issues_iface_t *self,
	st_bitflag_t issue_filter);

    int (*fatal_error_occurred)(
	struct issues_iface_t *self);
    
    void (*destroy)(
	struct issues_iface_t *self,
	st_bitflag_t issue_filter);	
	
};
