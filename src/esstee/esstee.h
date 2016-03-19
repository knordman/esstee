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

#include <esstee/issues.h>
#include <esstee/elements.h>

#include <stddef.h>

struct st_t;

struct st_t * st_new_instance(
    unsigned direct_memory_bytes);

int st_set_config(
    const char *option,
    int value,
    struct st_t *st);

int st_get_config(
    const char *option,
    struct st_t *st);

int st_load_file(
    struct st_t *st,
    const char *path);

int st_load_buffer(
    const char *identifier, 
    const char *bytes, 
    size_t len, 
    struct st_t *st);

int st_link(
    struct st_t *st);

const struct st_location_t * st_start(
    struct st_t *st,
    const char *program);

const struct st_issue_t * st_next_issue(
    struct st_t *st,
    st_bitflag_t filter);

int st_query(
    struct st_t *st,
    char *output,
    size_t output_max_len,
    const char *query);

struct st_element_t * st_get_element(
    struct st_t *st,
    const char *identifier);

int st_run_cycle(
    struct st_t *st,
    uint64_t ms);

const struct st_location_t * st_step(
    struct st_t *st);

const struct st_location_t * st_step_in(
    struct st_t *st);

const struct st_location_t * st_step_out(
    struct st_t *st);

int st_step_time(
    struct st_t *st,
    uint64_t ms);

void st_destroy(
    struct st_t *st);
