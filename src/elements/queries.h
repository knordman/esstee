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

#include <elements/iqueries.h>
#include <elements/iqualified_identifier.h>
#include <expressions/iexpression.h>
#include <util/inamed_ref_pool.h>

struct query_t * st_create_program_query(
    char *prgm_identifier,
    const struct st_location_t *prgm_identifier_location,
    struct named_ref_pool_iface_t *program_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct query_t * st_create_identifier_query(
    char *prgm_identifier,
    const struct st_location_t *prgm_location,    
    char *identifier,
    const struct st_location_t *identifier_location,
    struct expression_iface_t *assigned,
    struct named_ref_pool_iface_t *program_refs,
    struct named_ref_pool_iface_t *var_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct query_t * st_create_qualified_identifier_query(
    char *prgm_identifier,
    const struct st_location_t *prgm_location,
    struct qualified_identifier_iface_t *qid,
    struct expression_iface_t *assigned,
    struct named_ref_pool_iface_t *program_refs,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

void st_destroy_query(
    struct query_t *query);

struct queries_iface_t * st_create_queries(
    struct query_t *first_query,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
