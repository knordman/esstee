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

#include <elements/date_time.h>
#include <elements/values.h>
#include <elements/types.h>
#include <util/macros.h>

#include <utlist.h>
#include <stdio.h>

/**************************************************************************/
/* Value interface                                                        */
/**************************************************************************/
struct duration_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    st_bitflag_t class;
    struct duration_t duration;
};

static int duration_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct duration_value_t *dv =
	CONTAINER_OF(self, struct duration_value_t, value);

    int total_written_bytes = 0;
    if(dv->duration.d > 0.0)
    {
	int written_bytes = snprintf(buffer,
				     buffer_size,
				     "%.2fd",
				     dv->duration.d);
	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;
	total_written_bytes += written_bytes;
    }
    
    if(dv->duration.h > 0.0)
    {
	int written_bytes = snprintf(buffer,
				     buffer_size,
				     "%.2fh",
				     dv->duration.h);
	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;
	total_written_bytes += written_bytes;
    }

    if(dv->duration.m > 0.0)
    {
	int written_bytes = snprintf(buffer,
				     buffer_size,
				     "%.2fm",
				     dv->duration.m);
	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;
	total_written_bytes += written_bytes;
    }

    if(dv->duration.s > 0.0)
    {
	int written_bytes = snprintf(buffer,
				     buffer_size,
				     "%.2fs",
				     dv->duration.s);
	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;
	total_written_bytes += written_bytes;
    }

    if(dv->duration.ms > 0.0)
    {
	int written_bytes = snprintf(buffer,
				     buffer_size,
				     "%.2fms",
				     dv->duration.ms);
	CHECK_WRITTEN_BYTES(written_bytes);
	buffer += written_bytes;
	buffer_size -= written_bytes;
	total_written_bytes += written_bytes;
    }

    if(total_written_bytes == 0)
    {
	int written_bytes = snprintf(buffer,
				     buffer_size,
				     "%.2fms",
				     0.0);
	CHECK_WRITTEN_BYTES(written_bytes);
	total_written_bytes += written_bytes;
    }

    return total_written_bytes;
}

static int duration_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct duration_value_t *dv =
	CONTAINER_OF(self, struct duration_value_t, value);

    const struct duration_t *ov =
	new_value->duration(new_value, config, issues);

    dv->duration.d = ov->d;
    dv->duration.h = ov->h;
    dv->duration.m = ov->m;
    dv->duration.s = ov->s;
    dv->duration.ms = ov->ms;

    return ESSTEE_OK;
}

static int duration_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->duration)
    {
	issues->new_issue(
	    issues,
	    "the other value cannot be interpreted as a duration",
	    ESSTEE_ARGUMENT_ERROR);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

static const struct type_iface_t * duration_value_type_of(
    const struct value_iface_t *self)
{
    struct duration_value_t *dv =
	CONTAINER_OF(self, struct duration_value_t, value);

    return dv->type;
}

static st_bitflag_t duration_value_class(
    const struct value_iface_t *self)
{
    struct duration_value_t *dv =
	CONTAINER_OF(self, struct duration_value_t, value);

    return dv->class;
}

static void duration_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: duration value destructor */
}

static int duration_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct duration_value_t *dv =
	CONTAINER_OF(self, struct duration_value_t, value);

    const struct duration_t *ov =
	other_value->duration(other_value, config, issues);

    if(dv->duration.d > ov->d)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.h > ov->h)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.m > ov->m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.s > ov->s)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.ms > ov->ms)
    {
	return ESSTEE_TRUE;
    }
    
    return ESSTEE_FALSE;
}

static int duration_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct duration_value_t *dv =
	CONTAINER_OF(self, struct duration_value_t, value);

    const struct duration_t *ov =
	other_value->duration(other_value, config, issues);

    if(dv->duration.d < ov->d)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.h < ov->h)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.m < ov->m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.s < ov->s)
    {
	return ESSTEE_TRUE;
    }

    if(dv->duration.ms < ov->ms)
    {
	return ESSTEE_TRUE;
    }
    
    return ESSTEE_FALSE;
}

static const struct duration_t * duration_value_duration(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct duration_value_t *dv =
	CONTAINER_OF(self, struct duration_value_t, value);

    return &(dv->duration);
}

struct date_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    st_bitflag_t class;
    struct date_t date;
};

static int date_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct date_value_t *dv =
	CONTAINER_OF(self, struct date_value_t, value);

    int written_bytes = snprintf(buffer,
				 buffer_size,
				 "%.4" PRIu64 "-%.2" PRIu8 "-%.2" PRIu8,
				 dv->date.y,
				 dv->date.m,
				 dv->date.d);
    CHECK_WRITTEN_BYTES(written_bytes);

    return written_bytes;
}

static int date_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_value_t *dv =
	CONTAINER_OF(self, struct date_value_t, value);

    const struct date_t *ov =
	new_value->date(new_value, config, issues);

    dv->date.y = ov->y;
    dv->date.m = ov->m;
    dv->date.d = ov->d;

    return ESSTEE_OK;
}

static int date_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->date)
    {
	issues->new_issue(
	    issues,
	    "the other value cannot be interpreted as a date",
	    ESSTEE_ARGUMENT_ERROR);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

static const struct type_iface_t * date_value_type_of(
    const struct value_iface_t *self)
{
    struct date_value_t *dv =
	CONTAINER_OF(self, struct date_value_t, value);

    return dv->type;
}

static st_bitflag_t date_value_class(
    const struct value_iface_t *self)
{
    struct date_value_t *dv =
	CONTAINER_OF(self, struct date_value_t, value);

    return dv->class;
}

static void date_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: date destructor */
}

static int date_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_value_t *dv =
	CONTAINER_OF(self, struct date_value_t, value);

    const struct date_t *ov =
	other_value->date(other_value, config, issues);

    if(dv->date.y > ov->y)
    {
	return ESSTEE_TRUE;
    }

    if(dv->date.m > ov->m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->date.d > ov->d)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

static int date_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_value_t *dv =
	CONTAINER_OF(self, struct date_value_t, value);

    const struct date_t *ov =
	other_value->date(other_value, config, issues);

    if(dv->date.y < ov->y)
    {
	return ESSTEE_TRUE;
    }

    if(dv->date.m < ov->m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->date.d < ov->d)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

static const struct date_t * date_value_date(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_value_t *dv =
	CONTAINER_OF(self, struct date_value_t, value);

    return &(dv->date);
}

struct tod_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    st_bitflag_t class;
    struct tod_t tod;
};

static int tod_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct tod_value_t *tv =
	CONTAINER_OF(self, struct tod_value_t, value);

    int written_bytes = snprintf(buffer,
				 buffer_size,
				 "%" PRIu8 "h%" PRIu8 "m%" PRIu8 ".%.2" PRIu8 "s",
				 tv->tod.h,
				 tv->tod.m,
				 tv->tod.s,
				 tv->tod.fs);
    CHECK_WRITTEN_BYTES(written_bytes);

    return written_bytes;
}

static int tod_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct tod_value_t *tv =
	CONTAINER_OF(self, struct tod_value_t, value);

    const struct tod_t *ov =
	new_value->tod(new_value, config, issues);

    tv->tod.h = ov->h;
    tv->tod.m = ov->m;
    tv->tod.s = ov->s;
    tv->tod.fs = ov->fs;

    return ESSTEE_OK;
}

static int tod_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->tod)
    {
	issues->new_issue(
	    issues,
	    "the other value cannot be interpreted as a tod",
	    ESSTEE_ARGUMENT_ERROR);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

static const struct type_iface_t * tod_value_type_of(
    const struct value_iface_t *self)
{
    struct tod_value_t *tv =
	CONTAINER_OF(self, struct tod_value_t, value);

    return tv->type;
}

static st_bitflag_t tod_value_class(
    const struct value_iface_t *self)
{
    struct tod_value_t *tv =
	CONTAINER_OF(self, struct tod_value_t, value);

    return tv->class;
}

static void tod_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: tod value destructor */
}

static int tod_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct tod_value_t *tv =
	CONTAINER_OF(self, struct tod_value_t, value);

    const struct tod_t *ov =
	other_value->tod(other_value, config, issues);

    if(tv->tod.h > ov->h)
    {
	return ESSTEE_TRUE;
    }
    
    if(tv->tod.m > ov->m)
    {
	return ESSTEE_TRUE;
    }

    if(tv->tod.s > ov->s)
    {
	return ESSTEE_TRUE;
    }

    if(tv->tod.fs > ov->fs)
    {
	return ESSTEE_TRUE;
    }
    
    return ESSTEE_FALSE;
}

static int tod_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct tod_value_t *tv =
	CONTAINER_OF(self, struct tod_value_t, value);

    const struct tod_t *ov =
	other_value->tod(other_value, config, issues);

    if(tv->tod.h < ov->h)
    {
	return ESSTEE_TRUE;
    }
    
    if(tv->tod.m < ov->m)
    {
	return ESSTEE_TRUE;
    }

    if(tv->tod.s < ov->s)
    {
	return ESSTEE_TRUE;
    }

    if(tv->tod.fs < ov->fs)
    {
	return ESSTEE_TRUE;
    }
    
    return ESSTEE_FALSE;
}

static const struct tod_t * tod_value_tod(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct tod_value_t *tv =
	CONTAINER_OF(self, struct tod_value_t, value);

    return &(tv->tod);
}

struct date_tod_value_t {
    struct value_iface_t value;
    const struct type_iface_t *type;
    st_bitflag_t class;
    struct date_tod_t dt;
};

static int date_tod_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    const struct date_tod_value_t *dv =
	CONTAINER_OF(self, struct date_tod_value_t, value);

    int total_written_bytes = 0;
    int written_bytes = snprintf(buffer,
				 buffer_size,
				 "%.4" PRIu64 "-%.2" PRIu8 "-%.2" PRIu8 "-",
				 dv->dt.date.y,
				 dv->dt.date.m,
				 dv->dt.date.d);
    CHECK_WRITTEN_BYTES(written_bytes);
    total_written_bytes += written_bytes;
    buffer += written_bytes;
    
    written_bytes = snprintf(buffer,
			     buffer_size,
			     "%" PRIu8 "h%" PRIu8 "m%" PRIu8 ".%.2" PRIu8 "s",
			     dv->dt.tod.h,
			     dv->dt.tod.m,
			     dv->dt.tod.s,
			     dv->dt.tod.fs);
    CHECK_WRITTEN_BYTES(written_bytes);
    total_written_bytes += written_bytes;

    return total_written_bytes;
}

static int date_tod_value_assign(
    struct value_iface_t *self,
    const struct value_iface_t *new_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_tod_value_t *dv =
	CONTAINER_OF(self, struct date_tod_value_t, value);

    const struct date_tod_t *ov =
	new_value->date_tod(new_value, config, issues);

    dv->dt.date.y = ov->date.y;
    dv->dt.date.m = ov->date.m;
    dv->dt.date.d = ov->date.d;
    dv->dt.tod.h = ov->tod.h;
    dv->dt.tod.m = ov->tod.m;
    dv->dt.tod.s = ov->tod.s;
    dv->dt.tod.fs = ov->tod.fs;
    
    return ESSTEE_OK;
}

static int date_tod_value_assigns_and_compares(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!other_value->date_tod)
    {
	issues->new_issue(
	    issues,
	    "the other value cannot be interpreted as a date tod",
	    ESSTEE_ARGUMENT_ERROR);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

static const struct type_iface_t * date_tod_value_type_of(
    const struct value_iface_t *self)
{
    struct date_tod_value_t *dv =
	CONTAINER_OF(self, struct date_tod_value_t, value);

    return dv->type;
}

static st_bitflag_t date_tod_value_class(
    const struct value_iface_t *self)
{
    struct date_tod_value_t *dv =
	CONTAINER_OF(self, struct date_tod_value_t, value);

    return dv->class;
}

static void date_tod_value_destroy(
    struct value_iface_t *self)
{
    /* TODO: date tod value destructor */
}

static int date_tod_value_greater(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_tod_value_t *dv =
	CONTAINER_OF(self, struct date_tod_value_t, value);

    const struct date_tod_t *ov =
	other_value->date_tod(other_value, config, issues);

    if(dv->dt.date.y > ov->date.y)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.date.m > ov->date.m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.date.d > ov->date.d)
    {
	return ESSTEE_TRUE;
    }
    
    if(dv->dt.tod.h > ov->tod.h)
    {
	return ESSTEE_TRUE;
    }
    
    if(dv->dt.tod.m > ov->tod.m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.tod.s > ov->tod.s)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.tod.fs > ov->tod.fs)
    {
	return ESSTEE_TRUE;
    }
    
    return ESSTEE_FALSE;
}

static int date_tod_value_lesser(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_tod_value_t *dv =
	CONTAINER_OF(self, struct date_tod_value_t, value);

    const struct date_tod_t *ov =
	other_value->date_tod(other_value, config, issues);

    if(dv->dt.date.y < ov->date.y)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.date.m < ov->date.m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.date.d < ov->date.d)
    {
	return ESSTEE_TRUE;
    }
    
    if(dv->dt.tod.h < ov->tod.h)
    {
	return ESSTEE_TRUE;
    }
    
    if(dv->dt.tod.m < ov->tod.m)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.tod.s < ov->tod.s)
    {
	return ESSTEE_TRUE;
    }

    if(dv->dt.tod.fs < ov->tod.fs)
    {
	return ESSTEE_TRUE;
    }
    
    return ESSTEE_FALSE;
}

static const struct date_tod_t * date_tod_value_tod(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_tod_value_t *dv =
	CONTAINER_OF(self, struct date_tod_value_t, value);

    return &(dv->dt);
}

/**************************************************************************/
/* Type interface                                                         */
/**************************************************************************/
struct duration_type_t {
    struct type_iface_t type;
    double default_d;
    double default_h;
    double default_m;
    double default_s;
    double default_ms;
};

static struct value_iface_t * duration_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct duration_value_t *dv = NULL;
    ALLOC_OR_ERROR_JUMP(
	dv,
	struct duration_value_t,
	issues,
	error_free_resources);

    dv->type = self;
    
    memset(&(dv->value), 0, sizeof(struct value_iface_t));
    dv->value.display = duration_value_display;
    dv->value.assignable_from = duration_value_assigns_and_compares;
    dv->value.comparable_to = duration_value_assigns_and_compares;
    dv->value.assign = duration_value_assign;
    dv->value.type_of = duration_value_type_of;
    dv->value.destroy = duration_value_destroy;

    dv->value.greater = duration_value_greater;
    dv->value.lesser = duration_value_lesser;
    dv->value.equals = st_general_value_equals;
    dv->value.duration = duration_value_duration;
    dv->value.class = duration_value_class;

    return &(dv->value);
    
error_free_resources:
    return NULL;
}

static int duration_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct duration_type_t *dt =
	CONTAINER_OF(self, struct duration_type_t, type);

    struct duration_value_t *dv =
	CONTAINER_OF(value_of, struct duration_value_t, value);

    dv->duration.d = dt->default_d;
    dv->duration.h = dt->default_h;
    dv->duration.m = dt->default_m;
    dv->duration.s = dt->default_s;
    dv->duration.ms = dt->default_ms;

    return ESSTEE_OK;
}

static int duration_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!value->duration)
    {
	issues->new_issue(
	    issues,
	    "type '%s' can only hold duration values",
	    ESSTEE_TYPE_ERROR,
	    self->identifier);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

static st_bitflag_t duration_type_class(
    const struct type_iface_t *self)
{
    return DURATION_TYPE;
}

static void duration_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: duration type destructor */
}

struct date_type_t {
    struct type_iface_t type;
    uint64_t default_year;
    uint8_t default_month;
    uint8_t default_day;
};

static struct value_iface_t * date_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_value_t *dv = NULL;

    ALLOC_OR_ERROR_JUMP(
	dv,
	struct date_value_t,
	issues,
	error_free_resources);

    dv->type = self;
    
    memset(&(dv->value), 0, sizeof(struct value_iface_t));
    dv->value.display = date_value_display;
    dv->value.assignable_from = date_value_assigns_and_compares;
    dv->value.comparable_to = date_value_assigns_and_compares;
    dv->value.assign = date_value_assign;
    dv->value.type_of = date_value_type_of;
    dv->value.destroy = date_value_destroy;

    dv->value.greater = date_value_greater;
    dv->value.lesser = date_value_lesser;
    dv->value.equals = st_general_value_equals;
    dv->value.date = date_value_date;
    dv->value.class = date_value_class;

    return &(dv->value);
    
error_free_resources:
    return NULL;
}

static int date_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct date_type_t *dt =
	CONTAINER_OF(self, struct date_type_t, type);

    struct date_value_t *dv =
	CONTAINER_OF(value_of, struct date_value_t, value);

    dv->date.y = dt->default_year;
    dv->date.m = dt->default_month;
    dv->date.d = dt->default_day;

    return ESSTEE_OK;
}

static int date_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!value->date)
    {
	issues->new_issue(
	    issues,
	    "type '%s' can only hold date values",
	    ESSTEE_TYPE_ERROR,
	    self->identifier);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

static st_bitflag_t date_type_class(
    const struct type_iface_t *self)
{
    return DATE_TYPE;
}

static void date_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: date type destructor */
}

struct tod_type_t {
    struct type_iface_t type;
    uint8_t default_hour;
    uint8_t default_minute;
    uint8_t default_second;
    uint8_t default_fractional_second;
};

static struct value_iface_t * tod_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct tod_value_t *tv = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	tv,
	struct tod_value_t,
	issues,
	error_free_resources);

    tv->type = self;

    memset(&(tv->value), 0, sizeof(struct value_iface_t));
    tv->value.display = tod_value_display;
    tv->value.assignable_from = tod_value_assigns_and_compares;
    tv->value.comparable_to = tod_value_assigns_and_compares;
    tv->value.assign = tod_value_assign;
    tv->value.type_of = tod_value_type_of;
    tv->value.destroy = tod_value_destroy;

    tv->value.greater = tod_value_greater;
    tv->value.lesser = tod_value_lesser;
    tv->value.equals = st_general_value_equals;
    tv->value.tod = tod_value_tod;
    tv->value.class = tod_value_class;

    return &(tv->value);
    
error_free_resources:
    return NULL;
}

static int tod_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct tod_type_t *tt =
	CONTAINER_OF(self, struct tod_type_t, type);

    struct tod_value_t *tv =
	CONTAINER_OF(value_of, struct tod_value_t, value);

    tv->tod.h = tt->default_hour;
    tv->tod.m = tt->default_minute;
    tv->tod.s = tt->default_second;
    tv->tod.fs = tt->default_fractional_second;

    return ESSTEE_OK;
}

static int tod_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!value->tod)
    {
	issues->new_issue(
	    issues,
	    "type '%s' can only hold tod values",
	    ESSTEE_TYPE_ERROR,
	    self->identifier);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

static st_bitflag_t tod_type_class(
    const struct type_iface_t *self)
{
    return TOD_TYPE;
}

static void tod_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: tod type destructor */
}

struct date_tod_type_t {
    struct type_iface_t type;
    uint64_t default_year;
    uint8_t default_month;
    uint8_t default_day;
    uint8_t default_hour;
    uint8_t default_minute;
    uint8_t default_second;
    uint8_t default_fractional_second;
};

static struct value_iface_t * date_tod_type_create_value_of(
    const struct type_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct date_tod_value_t *dv = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	dv,
	struct date_tod_value_t,
	issues,
	error_free_resources);

    dv->type = self;
    
    memset(&(dv->value), 0, sizeof(struct value_iface_t));
    dv->value.display = date_tod_value_display;
    dv->value.assignable_from = date_tod_value_assigns_and_compares;
    dv->value.comparable_to = date_tod_value_assigns_and_compares;
    dv->value.assign = date_tod_value_assign;
    dv->value.type_of = date_tod_value_type_of;
    dv->value.destroy = date_tod_value_destroy;

    dv->value.greater = date_tod_value_greater;
    dv->value.lesser = date_tod_value_lesser;
    dv->value.equals = st_general_value_equals;
    dv->value.date_tod = date_tod_value_tod;
    dv->value.class = date_tod_value_class;

    return &(dv->value);
    
error_free_resources:
    return NULL;
}

static int date_tod_type_reset_value_of(
    const struct type_iface_t *self,
    struct value_iface_t *value_of,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct date_tod_type_t *dt =
	CONTAINER_OF(self, struct date_tod_type_t, type);

    struct date_tod_value_t *dv =
	CONTAINER_OF(value_of, struct date_tod_value_t, value);

    dv->dt.date.y = dt->default_year;
    dv->dt.date.m = dt->default_month;
    dv->dt.date.d = dt->default_day;
    dv->dt.tod.h = dt->default_hour;
    dv->dt.tod.m = dt->default_minute;
    dv->dt.tod.s = dt->default_second;
    dv->dt.tod.fs = dt->default_fractional_second;
    
    return ESSTEE_OK;
}

static int date_tod_type_can_hold(
    const struct type_iface_t *self,
    const struct value_iface_t *value,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!value->date_tod)
    {
	issues->new_issue(
	    issues,
	    "type '%s' can only hold datetod values",
	    ESSTEE_TYPE_ERROR,
	    self->identifier);

	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

static st_bitflag_t date_tod_type_class(
    const struct type_iface_t *self)
{
    return DATE_TOD_TYPE;
}

static void date_tod_type_destroy(
    struct type_iface_t *self)
{
    /* TODO: date tod type destructor */
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
static struct duration_type_t duration_type_template = {
    .type = {
	.create_value_of = duration_type_create_value_of,
	.reset_value_of = duration_type_reset_value_of,
	.can_hold = duration_type_can_hold,
	.compatible = st_type_general_compatible,
	.class = duration_type_class,
	.destroy = duration_type_destroy,
	.identifier = "TIME",
    },
    .default_d = 0.0,
    .default_h = 0.0,
    .default_m = 0.0,
    .default_s = 0.0,
    .default_ms = 0.0,
};

static struct date_type_t date_type_template = {
    .type = {
	.create_value_of = date_type_create_value_of,
	.reset_value_of = date_type_reset_value_of,
	.can_hold = date_type_can_hold,
	.compatible = st_type_general_compatible,
	.class = date_type_class,
	.destroy = date_type_destroy,
	.identifier = "DATE",
    },
    .default_year = 1,
    .default_month = 1,
    .default_day = 1
};

static struct tod_type_t tod_type_template = {
    .type = {
	.create_value_of = tod_type_create_value_of,
	.reset_value_of = tod_type_reset_value_of,
	.can_hold = tod_type_can_hold,
	.compatible = st_type_general_compatible,
	.class = tod_type_class,
	.destroy = tod_type_destroy,
	.identifier = "TIME_OF_DAY",
    },
    .default_hour = 0,
    .default_minute = 0,
    .default_second = 0,
};

static struct date_tod_type_t date_tod_type_template = {
    .type = {
	.create_value_of = date_tod_type_create_value_of,
	.reset_value_of = date_tod_type_reset_value_of,
	.can_hold = date_tod_type_can_hold,
	.compatible = st_type_general_compatible,
	.class = date_tod_type_class,
	.destroy = date_tod_type_destroy,
	.identifier = "DATE_AND_TIME",
    },
    .default_year = 1,
    .default_month = 1,
    .default_day = 1,
    .default_hour = 0,
    .default_minute = 0,
    .default_second = 0,
};

struct type_iface_t * st_new_elementary_date_time_types()
{
    struct type_iface_t *date_time_type_list = NULL;
    struct duration_type_t *duration_type = NULL;
    struct date_type_t *date_type = NULL;
    struct tod_type_t *tod_type = NULL;
    struct date_tod_type_t *date_tod_type = NULL;
    
    /* Duration type */
    ALLOC_OR_JUMP(
	duration_type,
	struct duration_type_t,
	error_free_resources);

    memcpy(
	duration_type,
	&(duration_type_template),
	sizeof(struct duration_type_t));

    DL_APPEND(date_time_type_list, &(duration_type->type));
    
    /* Date type */
    ALLOC_OR_JUMP(
	date_type,
	struct date_type_t,
	error_free_resources);

    memcpy(
	date_type,
	&(date_type_template),
	sizeof(struct date_type_t));

    DL_APPEND(date_time_type_list, &(date_type->type));
    
    /* Tod type */
    ALLOC_OR_JUMP(
	tod_type,
	struct tod_type_t,
	error_free_resources);

    memcpy(
	tod_type,
	&(tod_type_template),
	sizeof(struct tod_type_t));

    DL_APPEND(date_time_type_list, &(tod_type->type));

    /* Date and tod type */
    ALLOC_OR_JUMP(
	date_tod_type,
	struct date_tod_type_t,
	error_free_resources);

    memcpy(
	date_tod_type,
	&(date_tod_type_template),
	sizeof(struct date_tod_type_t));

    DL_APPEND(date_time_type_list, &(date_tod_type->type));

    return date_time_type_list;
    
error_free_resources:
    free(duration_type);
    free(date_type);
    free(tod_type);
    free(date_tod_type);
    return NULL;
}

struct value_iface_t * st_new_typeless_duration_value(
    double days,
    double hours,
    double minutes,
    double seconds,
    double milliseconds,
    st_bitflag_t value_class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct value_iface_t *v =
	duration_type_create_value_of(NULL, config, issues);

    if(!v)
    {
	return NULL;
    }

    struct duration_value_t *dv =
	CONTAINER_OF(v, struct duration_value_t, value);

    dv->class = value_class;
    dv->duration.d = days;
    dv->duration.h = hours;
    dv->duration.m = minutes;
    dv->duration.s = seconds;
    dv->duration.ms = milliseconds;

    return v;
}

struct value_iface_t * st_new_typeless_date_value(
    uint64_t year,
    uint8_t month,
    uint8_t day,
    st_bitflag_t value_class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct value_iface_t *v =
	date_type_create_value_of(NULL, config, issues);

    if(!v)
    {
	return NULL;
    }

    struct date_value_t *dv =
	CONTAINER_OF(v, struct date_value_t, value);

    dv->class = value_class;
    dv->date.y = year;
    dv->date.m = month;
    dv->date.d = day;

    return v;
}

struct value_iface_t * st_new_typeless_tod_value(
    uint8_t hour,
    uint8_t minute,
    uint8_t second,
    uint8_t partial_second,
    st_bitflag_t value_class,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct value_iface_t *v =
	tod_type_create_value_of(NULL, config, issues);

    if(!v)
    {
	return NULL;
    }

    struct tod_value_t *tv =
	CONTAINER_OF(v, struct tod_value_t, value);

    tv->class = value_class;
    tv->tod.h = hour;
    tv->tod.m = minute;
    tv->tod.s = second;
    tv->tod.fs = partial_second;

    return v;
}

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
    struct issues_iface_t *issues)
{
    struct value_iface_t *v =
	date_tod_type_create_value_of(NULL, config, issues);

    if(!v)
    {
	return NULL;
    }

    struct date_tod_value_t *dv =
	CONTAINER_OF(v, struct date_tod_value_t, value);

    dv->class = value_class;
    dv->dt.date.y = year;
    dv->dt.date.m = month;
    dv->dt.date.d = day;
    dv->dt.tod.h = hour;
    dv->dt.tod.m = minute;
    dv->dt.tod.s = second;
    dv->dt.tod.fs = partial_second;

    return v;
}
