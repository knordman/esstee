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

#include <statements/iinvoke.h>
#include <elements/ivalue.h>
#include <expressions/iexpression.h>

struct case_list_element_t;
struct case_t;

struct case_list_element_t * st_extend_case_list(
    struct case_list_element_t *case_list,
    struct value_iface_t *value,
    const struct st_location_t *case_value_location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

void st_destroy_case_list(
    struct case_list_element_t *case_list);

struct case_t * st_create_case(
    struct case_list_element_t *case_list,
    const struct st_location_t *location,
    struct invoke_iface_t *statements,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

void st_destroy_case(
    struct case_t *c);

struct case_t * st_extend_cases(
    struct case_t *cases,
    struct case_t *new_case,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct invoke_iface_t * st_create_case_statement(
    struct expression_iface_t *selector,
    struct case_t *cases,
    struct invoke_iface_t *else_statements,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);
