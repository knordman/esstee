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

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define ALLOC_ARRAY_OR_JUMP(var, type, size, jump)	\
    do {						\
	var = (type *)malloc(sizeof(type)*size);	\
	if(!var)					\
	{						\
	    goto jump;					\
	}						\
    } while(0)

#define ALLOC_ARRAY_OR_ERROR_JUMP(var, type, size, errors, jump)	\
    do {								\
	var = (type *)malloc(sizeof(type)*size);			\
	if(!var)							\
	{								\
	    errors->memory_error(errors, __FILE__, __FUNCTION__, __LINE__); \
	    goto jump;							\
	}								\
    } while(0)

#define STRDUP_OR_JUMP(var, str, jump)		\
    do {					\
	var = strdup(str);			\
	if(!var)				\
	{					\
	    goto jump;				\
	}					\
    } while(0)

#define STRDUP_OR_ERROR_JUMP(var, str, errors, jump)			\
    do {								\
	var = strdup(str);						\
	if(!var)							\
	{								\
	    errors->memory_error(errors, __FILE__, __FUNCTION__, __LINE__); \
	    goto jump;							\
	}								\
    } while(0)

#define ALLOC_OR_JUMP(var, type, jump) ALLOC_ARRAY_OR_JUMP(var, type, 1, jump)
#define ALLOC_OR_ERROR_JUMP(var, type, errors, jump) ALLOC_ARRAY_OR_ERROR_JUMP(var, type, 1, errors, jump)

/* The Linux container of macro */
#define CONTAINER_OF(ptr, type, member)				\
    ({ const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
       (type *)( (char *)__mptr - offsetof(type,member) ); })

#define LOCDUP_OR_JUMP(var, loc, jump)					\
    do {								\
	var = (struct st_location_t *)malloc(sizeof(struct st_location_t)); \
	if(!var)							\
	{								\
	    goto jump;							\
	}								\
	else								\
	{								\
	    memcpy(var, loc, sizeof(struct st_location_t));		\
	}								\
    } while(0)

#define LOCDUP_OR_ERROR_JUMP(var, loc, errors, jump)			\
    do {								\
	var = (struct st_location_t *)malloc(sizeof(struct st_location_t)); \
	if(!var)							\
	{								\
	    errors->memory_error(errors, __FILE__, __FUNCTION__, __LINE__); \
	    goto jump;							\
	}								\
	else								\
	{								\
	    memcpy(var, loc, sizeof(struct st_location_t));		\
	}								\
    } while(0)

#define CHECK_WRITTEN_BYTES(X)			\
    do {					\
	if(X == 0)				\
	{					\
	    return ESSTEE_FALSE;		\
	}					\
	else if(X < 0)				\
	{					\
	    return ESSTEE_ERROR;		\
	}					\
    } while(0)

#define TYPE_ANCESTOR(X) \
    (X->ancestor) ? X->ancestor(X) : X
