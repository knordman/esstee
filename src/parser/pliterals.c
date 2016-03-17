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

#include <parser/parser.h>
#include <elements/values.h>
#include <util/macros.h>
#include <linker/linker.h>
#include <util/bitflag.h>

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>


static char * strip_underscores(char *string)
{	
    size_t string_length = strlen(string);
    char *cursor = string;
    int i;

    if(string)
    {
	for(i = 0; i < string_length; i++)
	{
	    if(string[i] == '_')
	    {
		string[i] = '\0';
	    }
	    else
	    {
		*cursor = string[i];
		cursor++;
	    }
	}
	*cursor = '\0';
    }

    return string;	
}


struct value_iface_t * st_new_explicit_literal(
    char *type_identifier,
    const struct st_location_t *type_identifier_location,
    struct value_iface_t *implicit_literal,
    struct parser_t *parser)
{
    int ref_add_result = parser->pou_type_ref_pool->add_two_step(
	parser->pou_type_ref_pool,
	type_identifier,
	implicit_literal,
	NULL,
	type_identifier_location,
	NULL,
	st_explicit_literal_type_resolved);

    if(ref_add_result != ESSTEE_OK)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	goto error_free_resources;
    }

    return implicit_literal;
    
error_free_resources:
    /* TODO: clear references */
    return NULL;
}

static struct value_iface_t * new_integer_literal(
    char *string,
    const struct st_location_t *string_location,
    int64_t sign_prefix,
    unsigned min_string_length,
    unsigned string_conversion_offset,
    int conversion_base,
    const char *error_message,
    struct parser_t *parser)
{
    struct value_iface_t *v =
	st_integer_type_create_value_of(NULL, parser->config);

    if(!v)
    {
	goto error_free_resources;
    }
    
    struct integer_value_t *iv =
	CONTAINER_OF(v, struct integer_value_t, value);
    
    strip_underscores(string);
    if(strlen(string) < min_string_length)
    {
	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }

    errno = 0;
    int64_t interpreted = strtol(string+string_conversion_offset,
				 NULL,
				 conversion_base);
    if(errno != 0)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    error_message,
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);
	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	goto error_free_resources;
    }

    ST_SET_FLAGS(iv->class, CONSTANT_VALUE);
    
    iv->num = interpreted * sign_prefix;
    iv->value.type_of = NULL;
    iv->value.assignable_from = NULL;
    iv->value.override_type = st_integer_literal_override_type;
    
    free(string);
    return &(iv->value);

error_free_resources:
    if(v)
    {
	v->destroy(v);
    }
    free(string);
    return NULL;
}

struct value_iface_t * st_new_integer_literal(
    char *string,
    const struct st_location_t *string_location,
    int64_t sign_prefix, 
    struct parser_t *parser)
{
    return new_integer_literal(string,
			       string_location,
			       sign_prefix,
			       1,
			       0,
			       10,
			       "cannot interpret integer literal",
			       parser);
}

struct value_iface_t * st_new_integer_literal_binary(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    return new_integer_literal(string,
			       string_location,
			       1,
			       3,
			       2,
			       2,
			       "cannot interpret binary literal",
			       parser);
}

struct value_iface_t * st_new_integer_literal_octal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    return new_integer_literal(string,
			       string_location,
			       1,
			       3,
			       2,
			       8,
			       "cannot interpret octal literal",
			       parser);
}

struct value_iface_t * st_new_integer_literal_hex(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    return new_integer_literal(string,
			       string_location,
			       1,
			       4,
			       3,
			       16,
			       "cannot interpret hexadecimal literal",
			       parser);
}

struct value_iface_t * st_new_real_literal(
    char *string,
    const struct st_location_t *string_location,
    int64_t sign_prefix,
    struct parser_t *parser)
{
    struct value_iface_t *v = NULL;
    double sp = (double)sign_prefix;
    
    strip_underscores(string);
    if(strlen(string) < 1)
     {
	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }

    v = st_real_type_create_value_of(NULL, parser->config);

    if(!v)
    {
	goto error_free_resources;
    }
        
    errno = 0;
    double interpreted = strtod(string, NULL);
    if(errno != 0)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "cannot interpret real literal",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);
	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	goto error_free_resources;
    }

    struct real_value_t *rv =
	CONTAINER_OF(v, struct real_value_t, value);

    ST_SET_FLAGS(rv->class, CONSTANT_VALUE);
    
    rv->num = interpreted * sp;
    rv->value.type_of = NULL;
    rv->value.assignable_from = NULL;
    rv->value.override_type = st_real_literal_override_type;
    
    free(string);
    return &(rv->value);

error_free_resources:
    if(v)
    {
	v->destroy(v);
    }
    free(string);
    return NULL;
}

enum duration_part_t {
	PLITERALS_D, 
	PLITERALS_H, 
	PLITERALS_M, 
	PLITERALS_S, 
	PLITERALS_MS,
	PLITERALS_NOTHING,
	PLITERALS_ERROR
};

static int find_start(char *string, char **start)
{
    /* Iterate to # */
    size_t string_length = strlen(string);
    size_t i, chars_left;
    for(i = 0, chars_left = string_length-1; i < string_length; i++, chars_left--)
    {
	if(string[i] == '#' && chars_left > 1)
	{
	    *start = string + i + 1;
	    return ESSTEE_OK;
	}
    }

    return ESSTEE_ERROR;	
}

static char is_fraction(double value, unsigned indicator_bit)
{
    if(fabsf(roundf(value) - value) >= 1e-4) 
    {
	return (1 << indicator_bit);
    } 
    else 
    {
	return 0x00;
    }
}

static enum duration_part_t next_duration_part(
    char **work_start,
    double *part_content,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    char *work_end;
    if(*(*work_start) == '\0')
    {
	return PLITERALS_NOTHING;
    }

    errno = 0;
    *part_content = strtod(*work_start, &(work_end));
    if(errno != 0)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "too large or small number in duration",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return PLITERALS_ERROR;
    }
    else if(work_end == *work_start)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "failed to interpret all duration parts",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return PLITERALS_ERROR;		
    }

    switch(*work_end)
    {
    case '\0':
	return PLITERALS_ERROR;

    case 'd':
    case 'D':
	*work_start = work_end+1;
	return PLITERALS_D;

    case 'h':
    case 'H':
	*work_start = work_end+1;
	return PLITERALS_H;
			
    case 'm':
    case 'M':
	if(work_end[1] == 's') 
	{
	    *work_start = work_end+2;
	    return PLITERALS_MS;	
	}
	else
	{
	    *work_start = work_end+1;
	    return PLITERALS_M;
	}
			
    case 's':
    case 'S':
	*work_start = work_end+1;
	return PLITERALS_S;
    }

    return PLITERALS_ERROR;
}

struct value_iface_t * st_new_duration_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    char *work_buffer;
    enum duration_part_t part_type;
    double part_content;
    char defined = 0x00;
    char fractions = 0x00;
    unsigned parts_defined = 0;

    struct value_iface_t *v = st_duration_type_create_value_of(NULL, parser->config);

    if(!v)
    {
	goto error_free_resources;
    }
    
    struct duration_value_t *dv =
	CONTAINER_OF(v, struct duration_value_t, value);

    strip_underscores(string);
    if(find_start(string, &(work_buffer)) == ESSTEE_ERROR)
    {
	parser->errors->internal_error( /* Flex returning wrong type of string */
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);
	    
	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }
	
    while((part_type = next_duration_part(&(work_buffer), &(part_content), string_location, parser)) != PLITERALS_NOTHING)
    {
	switch(part_type)
	{
	case PLITERALS_ERROR:
	    goto error_free_resources;
			
	case PLITERALS_D:
	    dv->duration.d = part_content;
	    defined |= (1 << 0);
	    fractions |= is_fraction(part_content, 0);
	    parts_defined++;
	    break;

	case PLITERALS_H:
	    dv->duration.h = part_content;
	    defined |= (1 << 1);
	    fractions |= is_fraction(part_content, 1);
	    parts_defined++;
	    break;

	case PLITERALS_M:
	    dv->duration.m = part_content;
	    defined |= (1 << 2);
	    fractions |= is_fraction(part_content, 2);
	    parts_defined++;
	    break;

	case PLITERALS_S:
	    dv->duration.s = part_content;
	    defined |= (1 << 3);
	    fractions |= is_fraction(part_content, 3);
	    parts_defined++;
	    break;

	case PLITERALS_MS:
	    dv->duration.ms = part_content;
	    defined |= (1 << 4);
	    fractions |= is_fraction(part_content, 4);
	    parts_defined++;
	    break;

	default:
	    parser->errors->internal_error(
		parser->errors,
		__FILE__,
		__FUNCTION__,
		__LINE__);

	    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	    goto error_free_resources;
	}
    }

    if(defined == 0)
    {
	parser->errors->internal_error( /* Flex returning wrong type of string (empty) */
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }
    else if(fractions > defined || (parts_defined > 1 && fractions == defined))
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "only the last duration part may contain a fraction.",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);
	    
	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	goto error_free_resources;
    }
    
    free(string);
    return v;

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

static int find_year_month_day(
    char *start,
    char **year,
    char **month,
    char **day)
{
    if(strlen(start) < 10)
    {
	return ESSTEE_ERROR;
    }

    *year = start;
    start[4] = '\0';

    *month = start + 5;
    start[7] = '\0';

    *day = start + 8;
    start[10] = '\0';

    return ESSTEE_OK;
}

static int numberize_date_strings(
    struct date_t *date,
    char *year,
    char *month,
    char *day,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    errno = 0;
    date->y = (uint64_t)(strtoul(year, NULL, 10));
    if(errno != 0)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "cannot interpret year",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);
	
	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }

    unsigned long nmonth = strtoul(month, NULL, 10);
    if(errno != 0)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "cannot interpret month",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);
	
	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }
    
    unsigned long nday = strtoul(day, NULL, 10);
    if(errno != 0)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "cannot interpret day",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);
	
	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }

    if(nmonth < 1 || nmonth > 12)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "month must be at least 1 and at most 12",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);
	
	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }
    else
    {
	date->m = (uint8_t)nmonth;
    }
    
    if(nday < 1 || nday > 31)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "day must be at least 1 and at most 31",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);
	
	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }
    else
    {
	date->d = (uint8_t)nday;
    }

    return ESSTEE_OK;
}

struct value_iface_t * st_new_date_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    char *year, *month, *day, *start;

    if(find_start(string, &(start)) == ESSTEE_ERROR)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }

    if(find_year_month_day(start, &(year), &(month), &(day)) == ESSTEE_ERROR)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }
    
    struct value_iface_t *v
	= st_date_type_create_value_of(NULL, parser->config);

    if(!v)
    {
	parser->errors->memory_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }
    
    struct date_value_t *dv =
	CONTAINER_OF(v, struct date_value_t, value);

    if(numberize_date_strings(
	   &(dv->date),
	   year,
	   month,
	   day,
	   string_location,
	   parser) == ESSTEE_ERROR)
    {
	goto error_free_resources;
    }

    free(string);
    return &(dv->value);

error_free_resources:
    /* TODO: determine what to destroy */
    free(string);
    return NULL;
}

static int find_h_m_s_ms(
    char *start,
    char **h,
    char **m,
    char **s,
    char **ms)
{
    if(strlen(start) != 11)
    {
	return ESSTEE_ERROR;
    }

    *h = start;
    start[2] = '\0';

    *m = start + 3;
    start[5] = '\0';

    *s = start + 6;
    start[8] = '\0';

    *ms = start + 9;
    start[11] = '\0';

    return ESSTEE_OK;
}

static int numberize_tod_strings(
    struct tod_t *tod,
    char *h,
    char *m,
    char *s,
    char *fs,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    errno = 0;
    unsigned long hours = strtoul(h, NULL, 10);
    if(errno != 0)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "cannot interpret hours",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }
    
    unsigned long minutes = strtoul(m, NULL, 10);
    if(errno != 0)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "cannot interpret minutes",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }

    unsigned long seconds = strtoul(s, NULL, 10);
    if(errno != 0)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "cannot interpret seconds",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }

    unsigned long fractional_seconds = strtoul(fs, NULL, 10);
    if(errno != 0)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "cannot interpret fractional seconds",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }

    if(hours > 23)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "hours must be at most 23",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }
    else
    {
	tod->h = (uint8_t)hours;
    }
    
    if(minutes > 59)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "minutes must be at most 59",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }
    else
    {
	tod->m = (uint8_t)minutes;
    }
    
    if(seconds > 59)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "seconds must be at most 59",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }
    else
    {
	tod->s = (uint8_t)seconds;
    }
    
    if(fractional_seconds > 99)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "fractional seconds must be at most 99",
	    ISSUE_ERROR_CLASS,
	    1,
	    string_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }
    else
    {
	tod->fs = (uint8_t)fractional_seconds;
    }

    return ESSTEE_OK;
}

struct value_iface_t * st_new_tod_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    char *start, *h, *m, *s, *ms;

    if(find_start(string, &(start)) == ESSTEE_ERROR)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }

    if(find_h_m_s_ms(start, &(h), &(m), &(s), &(ms)) == ESSTEE_ERROR)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }

    struct value_iface_t *v
	= st_tod_type_create_value_of(NULL, parser->config);

    if(!v)
    {
	parser->errors->memory_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }

    struct tod_value_t *tv =
	CONTAINER_OF(v, struct tod_value_t, value);
    
    if(numberize_tod_strings(
	   &(tv->tod),
	   h,
	   m,
	   s,
	   ms,
	   string_location,
	   parser) == ESSTEE_ERROR)
    {
	goto error_free_resources;
    }

    free(string);
    return v;

error_free_resources:
    free(string);
    /* TODO: determine what to destroy */
    return NULL;
}

struct value_iface_t * st_new_date_tod_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    char *start, *y, *mon, *d, *h, *min, *s, *ms;

    if(find_start(string, &(start)) == ESSTEE_ERROR)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }

    if(strlen(start) != 22)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }

    if(find_year_month_day(start, &(y), &(mon), &(d)) == ESSTEE_ERROR)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }

    if(find_h_m_s_ms(d+3, &(h), &(min), &(s), &(ms)) == ESSTEE_ERROR)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }

    struct value_iface_t *v
	= st_date_tod_type_create_value_of(NULL, parser->config);

    if(!v)
    {
	parser->errors->memory_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	goto error_free_resources;
    }

    struct date_tod_value_t *dv =
	CONTAINER_OF(v, struct date_tod_value_t, value);
    
    if(numberize_date_strings(
	   &(dv->dt.date),
	   y,
	   mon,
	   d,
	   string_location,
	   parser) == ESSTEE_ERROR)
    {
	goto error_free_resources;
    }

    if(numberize_tod_strings(
	   &(dv->dt.tod),
	   h,
	   min,
	   s,
	   ms,
	   string_location,
	   parser) == ESSTEE_ERROR)
    {
	goto error_free_resources;
    }

    free(string);
    return v;

error_free_resources:
    free(string);
    /* TODO: determine what to destroy */
    return NULL;
}

struct value_iface_t * st_new_boolean_literal(
    int64_t integer,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    struct value_iface_t *v =
	st_bool_type_create_value_of(NULL, parser->config);

    if(!v)
    {
	goto error_free_resources;
    }
    
    struct integer_value_t *iv =
	CONTAINER_OF(v, struct integer_value_t, value);

    if(integer == 0)
    {
	iv->num = 0;
    }
    else
    {
	iv->num = 1;
    }
    
    iv->value.type_of = NULL;
    iv->value.assignable_from = NULL;
    iv->value.assign = NULL;
    iv->value.override_type = st_integer_literal_override_type;
    
    return &(iv->value);

error_free_resources:
    return NULL;
}

static struct value_iface_t * new_string_literal(
    char *string,
    const struct st_location_t *string_location,
    const char *string_type,
    struct parser_t *parser)
{
    struct value_iface_t *v =
	st_string_type_create_value_of(NULL, parser->config);

    if(!v)
    {
	goto error_free_resources;
    }

    struct string_value_t *sv =
	CONTAINER_OF(v, struct string_value_t, value);

    int ref_add_result = parser->global_type_ref_pool->add(
	parser->global_type_ref_pool,
	string_type,
	sv,
	NULL,
	string_location,
	st_string_literal_type_resolved);

    if(ref_add_result != ESSTEE_OK)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	goto error_free_resources;
    }

    sv->str = string;
    sv->value.assignable_from = NULL;
    sv->value.destroy = st_string_literal_value_destroy;
    
    return &(sv->value);
    
error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}    

struct value_iface_t * st_new_single_string_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    return new_string_literal(string, string_location, "STRING", parser);
}

struct value_iface_t * st_new_double_string_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    return new_string_literal(string, string_location, "WSTRING", parser);
}








