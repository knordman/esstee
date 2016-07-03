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

#include <stdint.h>

struct duration_t {
    double d;
    double h;
    double m;
    double s;
    double ms;
};

struct date_t {
    uint64_t y;
    uint8_t m;
    uint8_t d;
};

struct tod_t {
    uint8_t h;
    uint8_t m;
    uint8_t s;
    uint8_t fs;
};

struct date_tod_t {
    struct date_t date;
    struct tod_t tod;
};

struct type_iface_t * st_new_elementary_date_time_types();

struct value_iface_t * st_new_typeless_duration_value(
    double days,
    double hours,
    double minutes,
    double seconds,
    double milliseconds,
    st_bitflag_t value_class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct value_iface_t * st_new_typeless_date_value(
    uint64_t year,
    uint8_t month,
    uint8_t day,
    st_bitflag_t value_class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct value_iface_t * st_new_typeless_tod_value(
    uint8_t hour,
    uint8_t minute,
    uint8_t second,
    uint8_t partial_second,
    st_bitflag_t value_class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

struct value_iface_t * st_new_typeless_date_tod_value(
    uint64_t year,
    uint8_t month,
    uint8_t day,
    uint8_t hour,
    uint8_t minute,
    uint8_t second,
    uint8_t partial_second,
    st_bitflag_t value_class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

