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

#include <elements/ivalue.h>
#include <util/iconfig.h>
#include <util/iissues.h>
#include <rt/isystime.h>
#include <rt/icursor.h>

struct qualified_identifier_iface_t {

    int (*verify)(
	const struct qualified_identifier_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*step)(
	struct qualified_identifier_iface_t *self,
	struct cursor_iface_t *cursor,
	const struct systime_iface_t *time,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*reset)(
	struct qualified_identifier_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*allocate)(
	struct qualified_identifier_iface_t *self,
	struct issues_iface_t *issues);
    
    struct qualified_identifier_iface_t * (*clone)(
	struct qualified_identifier_iface_t *self,
	struct issues_iface_t *issues);

    /* Path base interaction */
    int (*set_base)(
    	struct qualified_identifier_iface_t *self,
	struct variable_iface_t *base,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    const char * (*base_identifier)(
    	struct qualified_identifier_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    /* Target invoke */
    int (*target_invoke_verify)(
    	const struct qualified_identifier_iface_t *self,
	const struct invoke_parameters_iface_t *parameters,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*target_invoke_step)(
    	struct qualified_identifier_iface_t *self,
	const struct invoke_parameters_iface_t *parameters,
	struct cursor_iface_t *cursor,
	const struct systime_iface_t *time,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*target_invoke_reset)(
    	struct qualified_identifier_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);	
    
    /* Target access */
    const struct value_iface_t * (*target_value)(
	struct qualified_identifier_iface_t *self);    

    const char * (*target_name)(
	struct qualified_identifier_iface_t *self);

    /* Target modificaton check */
    int (*target_assignable_from)(
    	const struct qualified_identifier_iface_t *self,
	const struct value_iface_t *new_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    /* Target modification */
    int (*target_assign)(
    	struct qualified_identifier_iface_t *self,
	const struct value_iface_t *new_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*target_not)(
    	struct qualified_identifier_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*target_negate)(
    	struct qualified_identifier_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*target_xor)(
    	struct qualified_identifier_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*target_and)(
    	struct qualified_identifier_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*target_or)(
    	struct qualified_identifier_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*target_plus)(
    	struct qualified_identifier_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*target_minus)(
    	struct qualified_identifier_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*target_multiply)(
    	struct qualified_identifier_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*target_divide)(
    	struct qualified_identifier_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*target_modulus)(
    	struct qualified_identifier_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*target_to_power)(
    	struct qualified_identifier_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    /* Destructor */
    void (*destroy)(
    	struct qualified_identifier_iface_t *self);
    
    int constant_reference;
    const struct st_location_t *location;
};