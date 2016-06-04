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

#pragma once

#include <elements/ivariable.h>
#include <util/iconfig.h>
#include <util/iissues.h>
#include <rt/icursor.h>

struct invoke_parameter_t;

struct invoke_parameters_iface_t {

    int (*append)(
	struct invoke_parameters_iface_t *self,
	struct invoke_parameter_t *parameter,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);	
    
    int (*verify)(
	struct invoke_parameters_iface_t *self,
	const struct variable_iface_t *variables,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*step)(
	struct invoke_parameters_iface_t *self,
	struct cursor_iface_t *cursor,
	const struct systime_iface_t *time,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*reset)(
	struct invoke_parameters_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*allocate)(
	struct invoke_parameters_iface_t *self,
	struct issues_iface_t *issues);

    struct invoke_parameters_iface_t * (*clone)(
	struct invoke_parameters_iface_t *self,
	struct issues_iface_t *issues);
    
    int (*assign_from)(
	struct invoke_parameters_iface_t *self,
	struct variable_iface_t *variables,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    void (*destroy)(
	struct invoke_parameters_iface_t *self);

};
