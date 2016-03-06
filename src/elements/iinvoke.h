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

#include <util/ierrors.h>
#include <util/iconfig.h>
#include <rt/isystime.h>
#include <rt/cursor.h>
#include <esstee/flags.h>

#define INVOKE_RESULT_FINISHED     1
#define INVOKE_RESULT_IN_PROGRESS  2
#define INVOKE_RESULT_ERROR        3
#define INVOKE_RESULT_ALL_FINISHED 4

struct invoke_iface_t {

    const struct st_location_t * (*location)(
	const struct invoke_iface_t *self);
    
    int (*step)(
	struct invoke_iface_t *self,
	struct cursor_t *cursor,
	const struct systime_iface_t *time,
	const struct config_iface_t *config,
	struct errors_iface_t *errors);

    int (*verify)(
	struct invoke_iface_t *self,
	const struct config_iface_t *config,
	struct errors_iface_t *errors);

    int (*reset)(
	struct invoke_iface_t *self,
	const struct config_iface_t *config);

    struct invoke_iface_t * (*clone)(
    	struct invoke_iface_t *self);

    void (*destroy)(
	struct invoke_iface_t *self);
	
    /* Invoke lists */
    struct invoke_iface_t *call_stack_prev;
    struct invoke_iface_t *call_stack_next;

    struct invoke_iface_t *exit_context_prev;
    struct invoke_iface_t *exit_context_next;

    struct invoke_iface_t *return_context_prev;
    struct invoke_iface_t *return_context_next;
    
    struct invoke_iface_t *prev;
    struct invoke_iface_t *next;
};
