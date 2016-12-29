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
#include <elements/integers.h>
#include <elements/reals.h>
#include <elements/date_time.h>
#include <elements/strings.h>

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

static int explicit_literal_type_resolved(
    void *referrer,
    void *target,
    st_bitflag_t remark,
    const char *identifier,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    if(!target)
    {
	const char *message = issues->build_message(
	    issues,
	    "reference to undefined type '%s'",
	    identifier);
	
	issues->new_issue_at(
	    issues,
	    message,
	    ESSTEE_LINK_ERROR,
	    1,
	    location);
	
	return ESSTEE_ERROR;
    }

    struct type_iface_t *literal_type =
	(struct type_iface_t *)target;

    struct value_iface_t *literal =
	(struct value_iface_t *)referrer;

    struct issue_group_iface_t *ig = issues->open_group(issues);

    int can_hold_result = literal_type->can_hold(literal_type,
						 literal,
						 config,
						 issues);
    ig->close(ig);
    
    if(can_hold_result != ESSTEE_TRUE)
    {
	const char *message = issues->build_message(
	    issues,
	    "type '%s' cannot be specified as an explicit type for literal",
	    literal_type->identifier);

	ig->main_issue(ig,
		       message,
		       ESSTEE_TYPE_ERROR,
		       1,
		       location);

	return ESSTEE_ERROR;
    }

    ig = issues->open_group(issues);

    int override_result = literal->override_type(literal,
						 literal_type,
						 config,
						 issues);
    ig->close(ig);
    
    if(override_result != ESSTEE_OK)
    {
	ig->main_issue(ig,
		       "type override failed",
		       ESSTEE_TYPE_ERROR,
		       1,
		       location);

	return ESSTEE_ERROR;
    }
    
    return ESSTEE_OK;
}

struct value_iface_t * st_new_explicit_literal(
    char *type_identifier,
    const struct st_location_t *type_identifier_location,
    struct value_iface_t *implicit_literal,
    struct parser_t *parser){

    int ref_add_result = parser->pou_type_ref_pool->add_two_step(
	parser->pou_type_ref_pool,
	type_identifier,
	implicit_literal,
	type_identifier_location,
	NULL,
	explicit_literal_type_resolved,
	parser->errors);

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
    const char *message = parser->errors->build_message(
	parser->errors,
	error_message,
	string);
    
    strip_underscores(string);
    if(strlen(string) < min_string_length)
    {
	parser->errors->internal_error(parser->errors,
				       __FILE__,
				       __FUNCTION__,
				       __LINE__);
	return NULL;
    }

    errno = 0;
    int64_t interpreted = strtol(string+string_conversion_offset,
				 NULL,
				 conversion_base);
    if(errno != 0)
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);

	return NULL;
    }

    interpreted *= sign_prefix;

    return st_new_typeless_integer_value(interpreted,
					 CONSTANT_VALUE,
					 parser->config,
					 parser->errors);
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
			       "cannot interpret integer literal '%s'",
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
			       "cannot interpret binary literal '%s'",
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
			       "cannot interpret octal literal '%s'",
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
			       "cannot interpret hexadecimal literal '%s'",
			       parser);
}

struct value_iface_t * st_new_real_literal(
    char *string,
    const struct st_location_t *string_location,
    int64_t sign_prefix,
    struct parser_t *parser)
{
    double sp = (double)sign_prefix;
    
    strip_underscores(string);
    if(strlen(string) < 1)
    {
	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
	return NULL;
    }
        
    errno = 0;
    double interpreted = strtod(string, NULL);
    if(errno != 0)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "cannot interpret real literal '%s'");

	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);
	
	return NULL;
    }

    return st_new_typeless_real_value(sp*interpreted,
				      CONSTANT_VALUE,
				      parser->config,
				      parser->errors);
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
    struct duration_t duration;
    
    strip_underscores(string);
    if(find_start(string, &(work_buffer)) == ESSTEE_ERROR)
    {
	parser->errors->internal_error( /* Flex returning wrong type of string */
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);
	    
	goto error_free_resources;
    }
	
    while((part_type = next_duration_part(&(work_buffer), &(part_content), string_location, parser)) != PLITERALS_NOTHING)
    {
	switch(part_type)
	{
	case PLITERALS_ERROR:
	    goto error_free_resources;
			
	case PLITERALS_D:
	    duration.d = part_content;
	    defined |= (1 << 0);
	    fractions |= is_fraction(part_content, 0);
	    parts_defined++;
	    break;

	case PLITERALS_H:
	    duration.h = part_content;
	    defined |= (1 << 1);
	    fractions |= is_fraction(part_content, 1);
	    parts_defined++;
	    break;

	case PLITERALS_M:
	    duration.m = part_content;
	    defined |= (1 << 2);
	    fractions |= is_fraction(part_content, 2);
	    parts_defined++;
	    break;

	case PLITERALS_S:
	    duration.s = part_content;
	    defined |= (1 << 3);
	    fractions |= is_fraction(part_content, 3);
	    parts_defined++;
	    break;

	case PLITERALS_MS:
	    duration.ms = part_content;
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

	goto error_free_resources;
    }
    else if(fractions > defined || (parts_defined > 1 && fractions == defined))
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "only the last duration part may contain a fraction",
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);
	    
	goto error_free_resources;
    }
    
    free(string);
    return st_new_typeless_duration_value(duration.d,
					  duration.h,
					  duration.m,
					  duration.s,
					  duration.ms,
					  CONSTANT_VALUE,
					  parser->config,
					  parser->errors);

error_free_resources:
    free(string);
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
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "cannot interpret year '%s'",
	    year);
	    
	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);
	
	return ESSTEE_ERROR;
    }

    unsigned long nmonth = strtoul(month, NULL, 10);
    if(errno != 0)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "cannot interpret month '%s'",
	    year);

	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);
	
	return ESSTEE_ERROR;
    }
    
    unsigned long nday = strtoul(day, NULL, 10);
    if(errno != 0)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "cannot interpret day '%s'",
	    day);

	parser->errors->new_issue_at(
	    parser->errors,
	    message, 
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);
	
	return ESSTEE_ERROR;
    }

    if(nmonth < 1 || nmonth > 12)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "month must be at least 1 and at most 12, not '%lu'",
	    nmonth);
	
	parser->errors->new_issue_at(
	    parser->errors,
	    message, 
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);
	
	return ESSTEE_ERROR;
    }
    else
    {
	date->m = (uint8_t)nmonth;
    }
    
    if(nday < 1 || nday > 31)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "day must be at least 1 and at most 31, not '%lu'",
	    nday);

	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);
	
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

	goto error_free_resources;
    }

    if(find_year_month_day(start, &(year), &(month), &(day)) == ESSTEE_ERROR)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	goto error_free_resources;
    }

    struct date_t date;
    
    if(numberize_date_strings(
	   &date,
	   year,
	   month,
	   day,
	   string_location,
	   parser) == ESSTEE_ERROR)
    {
	goto error_free_resources;
    }

    free(string);
    return st_new_typeless_date_value(date.y,
				      date.m,
				      date.d,
				      CONSTANT_VALUE,
				      parser->config,
				      parser->errors);

error_free_resources:
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
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "cannot interpret hours '%s'",
	    h);
	
	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);

	return ESSTEE_ERROR;
    }
    
    unsigned long minutes = strtoul(m, NULL, 10);
    if(errno != 0)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "cannot interpret minutes '%s'",
	    m);

	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);

	return ESSTEE_ERROR;
    }

    unsigned long seconds = strtoul(s, NULL, 10);
    if(errno != 0)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "cannot interpret minutes '%s'",
	    s);

	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }

    unsigned long fractional_seconds = strtoul(fs, NULL, 10);
    if(errno != 0)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "cannot interpret fractional seconds '%s'",
	    fs);
	
	parser->errors->new_issue_at(
	    parser->errors,
	    message, 
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);

	return ESSTEE_ERROR;
    }    

    if(hours > 23)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "hours must be at most 23, not '%lu'",
	    hours);
	
	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);

	return ESSTEE_ERROR;
    }
    else
    {
	tod->h = (uint8_t)hours;
    }
    
    if(minutes > 59)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "minutes must be at most 59, not '%lu'",
	    minutes);

	parser->errors->new_issue_at(
	    parser->errors,
	    message, 
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);

	return ESSTEE_ERROR;
    }
    else
    {
	tod->m = (uint8_t)minutes;
    }
    
    if(seconds > 59)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "seconds must be at most 59, not '%lu'",
	    seconds);

	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);

	return ESSTEE_ERROR;
    }
    else
    {
	tod->s = (uint8_t)seconds;
    }
    
    if(fractional_seconds > 99)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "fractional seconds must be at most 99, not '%lu'",
	    minutes);

	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_IO_ERROR,
	    1,
	    string_location);

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

	goto error_free_resources;
    }

    if(find_h_m_s_ms(start, &(h), &(m), &(s), &(ms)) == ESSTEE_ERROR)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	goto error_free_resources;
    }

    struct tod_t tod;
    
    if(numberize_tod_strings(
	   &tod,
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
    return st_new_typeless_tod_value(tod.h,
				     tod.m,
				     tod.s,
				     tod.fs,
				     CONSTANT_VALUE,
				     parser->config,
				     parser->errors);

error_free_resources:
    free(string);
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

	goto error_free_resources;
    }

    if(strlen(start) != 22)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	goto error_free_resources;
    }

    if(find_year_month_day(start, &(y), &(mon), &(d)) == ESSTEE_ERROR)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	goto error_free_resources;
    }

    if(find_h_m_s_ms(d+3, &(h), &(min), &(s), &(ms)) == ESSTEE_ERROR)
    {
	parser->errors->internal_error(
	    parser->errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	goto error_free_resources;
    }

    struct date_t date;
    struct tod_t tod;

    if(numberize_date_strings(
	   &date,
	   y,
	   mon,
	   d,
	   string_location,
	   parser) == ESSTEE_ERROR)
    {
	goto error_free_resources;
    }

    if(numberize_tod_strings(
	   &tod,
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
    return st_new_typeless_date_tod_value(date.y,
					  date.m,
					  date.d,
					  tod.h,
					  tod.m,
					  tod.s,
					  tod.fs,
					  CONSTANT_VALUE,
					  parser->config,
					  parser->errors);

error_free_resources:
    free(string);

    return NULL;
}

struct value_iface_t * st_new_boolean_literal(
    int64_t integer,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    return st_new_bool_value((integer) ? ESSTEE_TRUE : ESSTEE_FALSE,
			     CONSTANT_VALUE,
			     parser->config,
			     parser->errors);
}

static struct value_iface_t * new_string_literal(
    char *string,
    const struct st_location_t *string_location,
    const char *string_type,
    struct parser_t *parser)
{
    return st_new_string_value(string_type,
			       string,
			       parser->config,
			       parser->errors);
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
