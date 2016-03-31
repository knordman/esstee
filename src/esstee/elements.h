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

#include <esstee/flags.h>

#include <stddef.h>
#include <inttypes.h>

struct st_date_tod_t {
    uint64_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t fseconds;
};

struct st_duration_t {
    double days;
    double hours;
    double minutes;
    double seconds;
    double milliseconds;
};

struct st_element_t {

    st_bitflag_t (*type_class)(
	const struct st_element_t *self);

    const char * (*type_identifier)(
	const struct st_element_t *self);

    int (*display)(
	const struct st_element_t *self,
	char *buffer,
	size_t buffer_size);

    struct st_element_t * (*sub_element)(
	struct st_element_t *self,
	const char *identifier);

    struct st_element_t * (*sub_item)(
	struct st_element_t *self,
	const char *item);
    
    int64_t (*as_integer)(
	struct st_element_t *self);

    int (*as_bool)(
	struct st_element_t *self);

    double (*as_real)(
	struct st_element_t *self);

    const char * (*as_text)(
	struct st_element_t *self);

    int (*as_duration)(
    	struct st_element_t *self,
	struct st_duration_t *output);
    
    int (*as_date_tod)(
    	struct st_element_t *self,
	struct st_date_tod_t *output);

    int (*set_by_integer)(
    	struct st_element_t *self,
	int64_t integer);

    int (*set_by_bool)(
    	struct st_element_t *self,
	int bool);
    
    int (*set_by_real)(
	struct st_element_t *self,
	double real);

    int (*set_by_text)(
	struct st_element_t *self,
	const char *text);

    int (*set_by_duration)(
    	struct st_element_t *self,
	const struct st_duration_t *duration);

    int (*set_by_date_tod)(
    	struct st_element_t *self,
	const struct st_date_tod_t *date_tod);
};

