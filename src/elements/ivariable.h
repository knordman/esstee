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

#include <elements/itype.h>
#include <elements/ivalue.h>
#include <util/iissues.h>
#include <rt/isystime.h>
#include <rt/icursor.h>

#define INPUT_VAR_CLASS			(1 << 0)
#define OUTPUT_VAR_CLASS		(1 << 1)
#define TEMP_VAR_CLASS			(1 << 2)
#define IN_OUT_VAR_CLASS		(1 << 3)
#define EXTERNAL_VAR_CLASS		(1 << 4)
#define GLOBAL_VAR_CLASS		(1 << 5)
#define RETAIN_VAR_CLASS		(1 << 6)
#define CONSTANT_VAR_CLASS		(1 << 7)

struct variable_iface_t {

    int (*create)(
	struct variable_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*reset)(
	struct variable_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    struct variable_iface_t * (*clone)(
	struct variable_iface_t *self,
	struct issues_iface_t *issues);

    /* Modification checks */
    int (*assignable_from)(
	const struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct value_iface_t *new_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    /* Modification of variable value */    
    int (*assign)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct value_iface_t *new_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*cast_assign)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct value_iface_t *new_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*not)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*negate)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*xor)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*and)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*or)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*plus)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*minus)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*multiply)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*divide)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*modulus)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*to_power)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    /* Invoke variable */
    int (*invoke_verify)(
	const struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct invoke_parameters_iface_t *parameters,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*invoke_step)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct invoke_parameters_iface_t *parameters,
	struct cursor_iface_t *cursor,
	const struct systime_iface_t *time,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*invoke_reset)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);	

    /* Sub-variable access */
    struct variable_iface_t * (*sub_variable)(
	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const char *identifier,
    	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    /* Variable value access */
    const struct value_iface_t * (*index_value)(
    	struct variable_iface_t *self,
	const struct array_index_iface_t *index,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    const struct value_iface_t * (*value)(
    	struct variable_iface_t *self);

    /* Variable type access */
    const struct type_iface_t * (*type)(
    	const struct variable_iface_t *self);	

    /* Destructor */
    void (*destroy)(
	struct variable_iface_t *self);

    const char *identifier;
    const struct st_location_t *location;
    st_bitflag_t class;
    UT_hash_handle hh;
    struct variable_iface_t *prev;
    struct variable_iface_t *next;
};
