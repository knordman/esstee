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
#include <util/iissues.h>
#include <rt/isystime.h>

struct cursor_iface_t {

    struct invoke_iface_t * (*step)(
	struct cursor_iface_t *self,
	struct systime_iface_t *systime,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    struct invoke_iface_t * (*step_in)(
	struct cursor_iface_t *self,
	struct systime_iface_t *systime,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    struct invoke_iface_t * (*step_out)(
	struct cursor_iface_t *self,
	struct systime_iface_t *systime,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    void (*reset)(
    	struct cursor_iface_t *self);
    
    int (*switch_current)(
	struct cursor_iface_t *self,
	struct invoke_iface_t *switch_to,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    void (*push_return_context)(
	struct cursor_iface_t *self,
	struct invoke_iface_t *context);

    void (*pop_return_context)(
	struct cursor_iface_t *self);

    int (*jump_return)(
	struct cursor_iface_t *self);

    void (*push_exit_context)(
	struct cursor_iface_t *self,
	struct invoke_iface_t *context);

    void (*pop_exit_context)(
	struct cursor_iface_t *self);
    
    int (*jump_exit)(
	struct cursor_iface_t *self);

    const struct st_location_t * (*current_location)(
    	struct cursor_iface_t *self);
    
    void (*destroy)(
	struct cursor_iface_t *self);

};
