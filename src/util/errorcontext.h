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

struct issue_node_t {
    struct st_issue_t issue;
    
    struct issue_node_t *prev;
    struct issue_node_t *next;
};

struct error_context_t {
    struct errors_iface_t errors;

    struct issue_node_t *issues;
    
    struct issue_node_t *internal_error;
    struct issue_node_t *memory_error;

    int internal_error_added;
    int memory_error_added;
    int error_count;
    int error_count_last_check;
    
    struct issue_node_t *iterator;
};

struct errors_iface_t * st_new_error_context(void);

void st_destroy_error_context(
    struct errors_iface_t *error_context);

void st_error_context_new_issue(
    struct errors_iface_t *self,
    const char *message,
    st_bitflag_t issue_class);

void st_error_context_new_issue_at(
    struct errors_iface_t *self,
    const char *message,
    st_bitflag_t issue_class,
    int location_count,    
    ...);

void st_error_context_new_memory_error(
    struct errors_iface_t *self,
    const char *file,
    const char *function,
    int line);

void st_error_context_new_internal_error(
    struct errors_iface_t *self,
    const char *file,
    const char *function,
    int line);

const struct st_issue_t * st_error_context_next_issue(
    struct errors_iface_t *self,
    st_bitflag_t issue_class_filter);

struct errors_iface_t * st_error_context_merge(
    struct errors_iface_t *self,
    struct errors_iface_t *to_merge);

int st_error_context_reset(
    struct errors_iface_t *self);

int st_error_context_new_error_occured(
    struct errors_iface_t *self);

int st_error_context_error_count(
    struct errors_iface_t *self);
