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

#include <elements/iprogram.h>
#include <elements/variables.h>
#include <elements/ifunction.h>
#include <util/inamed_ref_pool.h>
#include <util/iconfig.h>
#include <util/iissues.h>

struct query_t;

struct queries_iface_t {

    int (*append)(
	struct queries_iface_t *self,
	struct query_t *query,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*finish)(
    	struct queries_iface_t *self,
	struct named_ref_pool_iface_t *var_refs,
	struct named_ref_pool_iface_t *function_refs,
	struct named_ref_pool_iface_t *program_refs,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*link)(
    	struct queries_iface_t *self,
	struct variable_iface_t *global_variables,
	struct function_iface_t *functions,
	struct program_iface_t *programs,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);	
	
    int (*evaluate)(
	struct queries_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*display)(
	struct queries_iface_t *self,
	char *output,
	size_t output_max_len,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    void (*destroy)(
	struct queries_iface_t *self);

};
