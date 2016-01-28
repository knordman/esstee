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
#include <util/macros.h>

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

struct value_iface_t * st_extract_value_from_literal(
    struct expression_iface_t *expression,
    struct parser_t *parser)
{
    /* TODO: return literal as value */
    /* the value is in the integer_literal, which also contains the
       expression handle, how to get from the expression to the
       sibling value structrure? */
    return NULL;
}

struct expression_iface_t * st_new_explicit_literal(
    char *type_identifier,
    const struct st_location_t *type_identifier_location,
    struct expression_iface_t *implicit_literal,
    struct parser_t *parser)
{
    /* TODO: explicit type to literal */
    return NULL;
}

static struct expression_iface_t * new_integer_literal(
    char *string,
    const struct st_location_t *string_location,
    int64_t sign_prefix,
    unsigned min_string_length,
    unsigned string_conversion_offset,
    int conversion_base,
    const char *error_message,
    struct parser_t *parser)
{
    struct integer_literal_t *il = NULL;
    struct st_location_t *l = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	il,
	struct integer_literal_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	l,
	string_location,
	parser->errors,
	error_free_resources);

    il->location = l;
    
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

    il->data.num = interpreted * sign_prefix;
    il->data.explicit_type = NULL;

    il->data.value.display = st_integer_value_display;
    il->data.value.assign = NULL;
    il->data.value.reset = NULL;
    il->data.value.explicit_type = st_integer_value_explicit_type;
    il->data.value.index = NULL;
    il->data.value.sub_variable = NULL;
    il->data.value.compatible = st_integer_value_compatible;
    il->data.value.create_temp_from = st_integer_value_create_temp_from;
    il->data.value.destroy = st_integer_literal_value_destroy;

    il->data.value.greater = st_integer_value_greater;
    il->data.value.lesser = st_integer_value_lesser;
    il->data.value.equals = st_integer_value_equals;

    il->data.value.xor = NULL;
    il->data.value.and = NULL;
    il->data.value.or = NULL;
    il->data.value.plus = st_integer_value_plus;
    il->data.value.minus = st_integer_value_minus;
    il->data.value.multiply = st_integer_value_multiply;
    il->data.value.divide = st_integer_value_divide;
    il->data.value.modulus = st_integer_value_modulus;
    il->data.value.to_power = st_integer_value_to_power;
    
    il->data.value.integer = st_integer_value_integer;
    il->data.value.bool = NULL;
    il->data.value.real = NULL;
    il->data.value.string = NULL;
    il->data.value.enumeration = NULL;
    il->data.value.date = NULL;
    il->data.value.tod = NULL;
    il->data.value.date_tod = NULL;
    il->data.value.array_init_value = NULL;
    il->data.value.struct_init_value = NULL;

    il->expression.return_value = st_integer_literal_expression_return_value;
    il->expression.destroy = st_integer_literal_expression_destroy;
    il->expression.invoke.location = st_integer_literal_location;
    il->expression.invoke.step = NULL;
    il->expression.invoke.verify = NULL;
    
    free(string);
    return &(il->expression);

error_free_resources:
    free(il);
    free(l);
    free(string);
    return NULL;
}

struct expression_iface_t * st_new_integer_literal(
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

struct expression_iface_t * st_new_integer_literal_binary(
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

struct expression_iface_t * st_new_integer_literal_octal(
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

struct expression_iface_t * st_new_integer_literal_hex(
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

struct expression_iface_t * st_new_real_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    /* TODO: real literal */
    return NULL;

/* struct literal_t * st_new_real_literal( */
/*     char *string, */
/*     const struct location_t *string_location, */
/*     struct parser_t *parser) */
/* { */
/*     struct real_literal_t *rl = NULL; */

/*     rl = (struct real_literal_t *)malloc(sizeof(struct real_literal_t)); */
/*     if(!rl) */
/*     { */
/* 	MEMORY_ERROR(parser->errors); */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources;		 */
/*     } */

/*     errno = 0; */
/*     rl->value = strtod(string, NULL); */
/*     if(errno != 0) */
/*     { */
/* 	NEW_ERROR_AT("cannot interpret real number.", parser->errors, string_location); */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     rl->literal.literal_class = REAL_LITERAL; */

/*     free(string); */
/*     return &(rl->literal); */

/* error_free_resources: */
/*     free(string); */
/*     free(rl); */
/*     return NULL; */
/* } */
}

struct expression_iface_t * st_new_duration_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    /* TODO: duration literal */
    return NULL;

/* struct literal_t * st_new_duration_literal( */
/*     char *string, */
/*     const struct location_t *string_location, */
/*     struct parser_t *parser) */
/* { */
/*     char *work_buffer; */
/*     enum duration_part_t part_type; */
/*     double part_content; */
/*     struct duration_literal_t *dl = NULL; */
/*     char defined = 0x00; */
/*     char fractions = 0x00; */
/*     unsigned parts_defined = 0; */

/*     dl = (struct duration_literal_t *)malloc(sizeof(struct duration_literal_t)); */
/*     if(!dl) */
/*     { */
/* 	MEMORY_ERROR(parser->errors); */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */
/*     else */
/*     { */
/* 	dl->d = 0.0; */
/* 	dl->h = 0.0; */
/* 	dl->m = 0.0; */
/* 	dl->s = 0.0; */
/* 	dl->ms = 0.0; */
/*     } */

/*     strip_underscores(string); */
/*     if(find_start(string, &(work_buffer)) == ESSTEE_ERROR) */
/*     { */
/* 	INTERNAL_ERROR(parser->errors); // Flex returning wrong kind of string */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */
	
/*     while((part_type = next_duration_part(&(work_buffer), &(part_content), string_location, parser)) != PLITERALS_NOTHING) */
/*     { */
/* 	switch(part_type) */
/* 	{ */
/* 	case PLITERALS_ERROR: */
/* 	    goto error_free_resources; */
			
/* 	case PLITERALS_D: */
/* 	    dl->d = part_content; */
/* 	    defined |= (1 << 0); */
/* 	    fractions |= is_fraction(part_content, 0); */
/* 	    parts_defined++; */
/* 	    break; */

/* 	case PLITERALS_H: */
/* 	    dl->h = part_content; */
/* 	    defined |= (1 << 1); */
/* 	    fractions |= is_fraction(part_content, 1); */
/* 	    parts_defined++; */
/* 	    break; */

/* 	case PLITERALS_M: */
/* 	    dl->m = part_content; */
/* 	    defined |= (1 << 2); */
/* 	    fractions |= is_fraction(part_content, 2); */
/* 	    parts_defined++; */
/* 	    break; */

/* 	case PLITERALS_S: */
/* 	    dl->s = part_content; */
/* 	    defined |= (1 << 3); */
/* 	    fractions |= is_fraction(part_content, 3); */
/* 	    parts_defined++; */
/* 	    break; */

/* 	case PLITERALS_MS: */
/* 	    dl->ms = part_content; */
/* 	    defined |= (1 << 4); */
/* 	    fractions |= is_fraction(part_content, 4); */
/* 	    parts_defined++; */
/* 	    break; */

/* 	default: */
/* 	    INTERNAL_ERROR(parser->errors); */
/* 	    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	    goto error_free_resources; */
/* 	} */
/*     } */

/*     if(defined == 0) */
/*     { */
/* 	INTERNAL_ERROR(parser->errors); // Flex allowed empty literal, which it shouldn't */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */
/*     else if(fractions > defined || (parts_defined > 1 && fractions == defined)) */
/*     { */
/* 	NEW_ERROR_AT("only the last duration part may contain a fraction.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     dl->literal.literal_class = DURATION_LITERAL; */

/*     free(string);	 */
/*     return &(dl->literal); */

/* error_free_resources: */
/*     free(string); */
/*     free(dl); */
/*     return NULL; */
/* } */
}

struct expression_iface_t * st_new_date_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    /* TODO: date literal */
    return NULL;

/* struct literal_t * st_new_date_literal( */
/*     char *string, */
/*     const struct location_t *string_location, */
/*     struct parser_t *parser) */
/* { */
/*     char *year, *month, *day, *start; */
/*     struct date_literal_t *dl = NULL; */

/*     if(find_start(string, &(start)) == ESSTEE_ERROR) */
/*     { */
/* 	INTERNAL_ERROR(parser->errors); // Flex returning wrong kind of string */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     if(find_year_month_day(start, &(year), &(month), &(day)) == ESSTEE_ERROR) */
/*     { */
/* 	INTERNAL_ERROR(parser->errors); // Flex returning wrong kind of string */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     dl = (struct date_literal_t *)malloc(sizeof(struct date_literal_t)); */
/*     if(!dl) */
/*     { */
/* 	MEMORY_ERROR(parser->errors); */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     if(numberize_date_strings( */
/* 	   &(dl->y),  */
/* 	   &(dl->m),  */
/* 	   &(dl->d),  */
/* 	   year,  */
/* 	   month,  */
/* 	   day,  */
/* 	   string_location, */
/* 	   parser) == ESSTEE_ERROR) */
/*     { */
/* 	goto error_free_resources; */
/*     } */

/*     dl->literal.literal_class = DATE_LITERAL; */

/*     free(string); */
/*     return &(dl->literal); */

/* error_free_resources: */
/*     free(string); */
/*     free(dl); */
/*     return NULL; */
/* } */    
}

struct expression_iface_t * st_new_tod_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    /* TODO: tod literal */
    return NULL;
/* struct literal_t * st_new_tod_literal( */
/*     char *string, */
/*     const struct location_t *string_location, */
/*     struct parser_t *parser) */
/* { */
/*     char *start, *h, *m, *s, *ms; */
/*     struct tod_literal_t *tl = NULL; */

/*     if(find_start(string, &(start)) == ESSTEE_ERROR) */
/*     { */
/* 	INTERNAL_ERROR(parser->errors); // Flex returning wrong kind of string */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     if(find_h_m_s_ms(start, &(h), &(m), &(s), &(ms)) == ESSTEE_ERROR) */
/*     { */
/* 	INTERNAL_ERROR(parser->errors); // Flex returning wrong kind of string */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     tl = (struct tod_literal_t *)malloc(sizeof(struct tod_literal_t)); */
/*     if(!tl) */
/*     { */
/* 	MEMORY_ERROR(parser->errors); */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     if(numberize_tod_strings( */
/* 	   &(tl->h),  */
/* 	   &(tl->m),  */
/* 	   &(tl->s),  */
/* 	   &(tl->ms),  */
/* 	   h,  */
/* 	   m,  */
/* 	   s, */
/* 	   ms, */
/* 	   string_location,  */
/* 	   parser) == ESSTEE_ERROR) */
/*     { */
/* 	goto error_free_resources; */
/*     } */

/*     tl->literal.literal_class = TOD_LITERAL; */

/*     free(string); */
/*     return &(tl->literal); */

/* error_free_resources: */
/*     free(string); */
/*     free(tl); */
/*     return NULL; */
/* } */    
}

struct expression_iface_t * st_new_date_tod_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    /* TODO: date tod literal */
    return NULL;

/* struct literal_t * st_new_date_tod_literal( */
/*     char *string, */
/*     const struct location_t *string_location, */
/*     struct parser_t *parser) */
/* { */
/*     char *start, *y, *mon, *d, *h, *min, *s, *ms; */
/*     struct date_tod_literal_t *dtl = NULL; */

/*     if(find_start(string, &(start)) == ESSTEE_ERROR) */
/*     { */
/* 	INTERNAL_ERROR(parser->errors); // Flex returning wrong kind of string */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     if(strlen(start) != 22) */
/*     { */
/* 	INTERNAL_ERROR(parser->errors); // Flex returning wrong kind of string */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     if(find_year_month_day(start, &(y), &(mon), &(d)) == ESSTEE_ERROR) */
/*     { */
/* 	INTERNAL_ERROR(parser->errors); // Flex returning wrong kind of string */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     if(find_h_m_s_ms(d+3, &(h), &(min), &(s), &(ms)) == ESSTEE_ERROR) */
/*     { */
/* 	INTERNAL_ERROR(parser->errors); // Flex returning wrong kind of string */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     dtl = (struct date_tod_literal_t *)malloc(sizeof(struct date_tod_literal_t)); */
/*     if(!dtl) */
/*     { */
/* 	MEMORY_ERROR(parser->errors); */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     if(numberize_date_strings( */
/* 	   &(dtl->y),  */
/* 	   &(dtl->mon),  */
/* 	   &(dtl->d),  */
/* 	   y,  */
/* 	   mon,  */
/* 	   d,  */
/* 	   string_location, */
/* 	   parser) == ESSTEE_ERROR) */
/*     { */
/* 	goto error_free_resources; */
/*     } */

/*     if(numberize_tod_strings( */
/* 	   &(dtl->h),  */
/* 	   &(dtl->min),  */
/* 	   &(dtl->s),  */
/* 	   &(dtl->ms),  */
/* 	   h,  */
/* 	   min,  */
/* 	   s, */
/* 	   ms, */
/* 	   string_location,  */
/* 	   parser) == ESSTEE_ERROR) */
/*     { */
/* 	goto error_free_resources; */
/*     } */

/*     dtl->literal.literal_class = DATE_TOD_LITERAL; */

/*     free(string); */
/*     return &(dtl->literal); */

/* error_free_resources: */
/*     free(string); */
/*     free(dtl); */
/*     return NULL; */
/* } */
}

struct expression_iface_t * st_new_boolean_literal(
    int64_t integer,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    /* TODO: boolean literal */
    return NULL;
/* struct literal_t * st_new_boolean_literal( */
/*     int64_t integer, */
/*     const struct location_t *string_location, */
/*     struct parser_t *parser) */
/* { */
/*     struct integer_literal_t *bl = NULL; */

/*     if(integer != 0 && integer != 1) */
/*     { */
/* 	INTERNAL_ERROR(parser->errors); */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	return NULL; */
/*     } */

/*     bl = (struct integer_literal_t *)malloc(sizeof(struct integer_literal_t)); */
/*     if(!bl)  */
/*     { */
/* 	MEMORY_ERROR(parser->errors); */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	return NULL; */
/*     } */
/*     else */
/*     { */
/* 	bl->value = integer; */
/*     } */

/*     bl->literal.literal_class = BOOLEAN_LITERAL; */

/*     return &(bl->literal); */
/* } */    
}

struct expression_iface_t * st_new_single_string_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    /* TODO: string literal */
    return NULL;

/* struct literal_t * st_new_single_string_literal( */
/*     char *string, */
/*     const struct location_t *string_location, */
/*     struct parser_t *parser) */
/* { */
/*     return string_literal( */
/* 	string, */
/* 	SINGLE_BYTE_STRING_LITERAL, */
/* 	'\'', */
/* 	string_location, */
/* 	parser); */
/* } */
    
/* static struct literal_t * string_literal( */
/*     char *string, */
/*     bitflag_t attr, */
/*     char expected_start, */
/*     const struct location_t *string_location, */
/*     struct parser_t *parser) */
/* { */
/*     struct string_literal_t *sl = NULL; */
/*     int i; */
/*     size_t string_length = strlen(string); */

/*     if(string_length < 2) */
/*     { */
/* 	INTERNAL_ERROR(parser->errors); */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;		 */
/* 	goto error_free_resources; */
/*     } */

/*     sl = (struct string_literal_t *)malloc(sizeof(struct string_literal_t));	 */
/*     if(!sl) */
/*     { */
/* 	MEMORY_ERROR(parser->errors); */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     if(string[0] == expected_start) */
/*     { */
/* 	sl->literal.literal_class = attr; */
/* 	for(i = 0; i < string_length-2; i++) */
/* 	{ */
/* 	    string[i] = string[i+1]; */
/* 	} */
/* 	string[i] = '\0'; */
/* 	sl->value = string; */
/*     } */
/*     else */
/*     { */
/* 	INTERNAL_ERROR(parser->errors); */
/* 	parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;		 */
/* 	goto error_free_resources; */
/*     } */

/*     return &(sl->literal); */

/* error_free_resources: */
/*     free(string); */
/*     free(sl); */
/*     return NULL; */
/* } */
}

struct expression_iface_t * st_new_double_string_literal(
    char *string,
    const struct st_location_t *string_location,
    struct parser_t *parser)
{
    /* TODO: double literal */
    return NULL;

/* struct literal_t * st_new_double_string_literal( */
/*     char *string, */
/*     const struct location_t *string_location, */
/*     struct parser_t *parser) */
/* { */
/*     return string_literal( */
/* 	string, */
/* 	DOUBLE_BYTE_STRING_LITERAL, */
/* 	'\"', */
/* 	string_location, */
/* 	parser); */
/* } */
}

/* static int find_start(char *string, char **start) */
/* { */
/*     /\* Iterate to # *\/ */
/*     size_t string_length = strlen(string); */
/*     size_t i, chars_left; */
/*     for(i = 0, chars_left = string_length-1; i < string_length; i++, chars_left--) */
/*     { */
/* 	if(string[i] == '#' && chars_left > 1) */
/* 	{ */
/* 	    *start = string + i + 1; */
/* 	    return ESSTEE_OK; */
/* 	} */
/*     } */

/*     return ESSTEE_ERROR;	 */
/* } */

/* static char is_fraction(double value, unsigned indicator_bit) */
/* { */
/*     if(fabsf(roundf(value) - value) >= 1e-4)  */
/*     { */
/* 	return (1 << indicator_bit); */
/*     }  */
/*     else  */
/*     { */
/* 	return 0x00; */
/*     } */
/* } */

/* static enum duration_part_t next_duration_part( */
/*     char **work_start, */
/*     double *part_content, */
/*     const struct location_t *string_location, */
/*     struct parser_t *parser) */
/* { */
/*     char *work_end; */
/*     if(*(*work_start) == '\0') */
/*     { */
/* 	return PLITERALS_NOTHING; */
/*     } */

/*     errno = 0; */
/*     *part_content = strtod(*work_start, &(work_end)); */
/*     if(errno != 0) */
/*     { */
/* 	NEW_ERROR_AT("too large or small number in duration.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return PLITERALS_ERROR; */
/*     } */
/*     else if(work_end == *work_start) */
/*     { */
/* 	NEW_ERROR_AT("failed to interpret all duration parts.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return PLITERALS_ERROR;		 */
/*     } */

/*     switch(*work_end) */
/*     { */
/*     case '\0': */
/* 	INTERNAL_ERROR(parser->errors); */
/* 	return PLITERALS_ERROR; */

/*     case 'd': */
/*     case 'D': */
/* 	*work_start = work_end+1; */
/* 	return PLITERALS_D; */

/*     case 'h': */
/*     case 'H': */
/* 	*work_start = work_end+1; */
/* 	return PLITERALS_H; */
			
/*     case 'm': */
/*     case 'M': */
/* 	if(work_end[1] == 's')  */
/* 	{ */
/* 	    *work_start = work_end+2; */
/* 	    return PLITERALS_MS;	 */
/* 	} */
/* 	else */
/* 	{ */
/* 	    *work_start = work_end+1; */
/* 	    return PLITERALS_M; */
/* 	} */
			
/*     case 's': */
/*     case 'S': */
/* 	*work_start = work_end+1; */
/* 	return PLITERALS_S; */
/*     } */

/*     return PLITERALS_ERROR; */
/* } */


/* static int find_year_month_day( */
/*     char *start,  */
/*     char **year,  */
/*     char **month,  */
/*     char **day) */
/* { */
/*     if(strlen(start) < 10) */
/*     { */
/* 	return ESSTEE_ERROR; */
/*     } */

/*     *year = start; */
/*     start[4] = '\0'; */

/*     *month = start + 5; */
/*     start[7] = '\0'; */

/*     *day = start + 8; */
/*     start[10] = '\0'; */

/*     return ESSTEE_OK; */
/* } */

/* static int numberize_date_strings( */
/*     unsigned *nyear,  */
/*     unsigned *nmonth,  */
/*     unsigned *nday, */
/*     char *year,  */
/*     char *month,  */
/*     char *day,  */
/*     const struct location_t *string_location, */
/*     struct parser_t *parser) */
/* { */
/*     errno = 0; */
/*     *nyear = (unsigned)(strtoul(year, NULL, 10)); */
/*     if(errno != 0) */
/*     { */
/* 	NEW_ERROR_AT("cannot interpret year part.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */
/*     *nmonth = (unsigned)(strtoul(month, NULL, 10)); */
/*     if(errno != 0) */
/*     { */
/* 	NEW_ERROR_AT("cannot interpret month part.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */
/*     *nday = (unsigned)(strtoul(day, NULL, 10)); */
/*     if(errno != 0) */
/*     { */
/* 	NEW_ERROR_AT("cannot interpret day part.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */

/*     if(*nmonth < 1 || *nmonth > 12) */
/*     { */
/* 	NEW_ERROR_AT("month must be at least 1 and at most 12.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */
/*     if(*nday < 1 || *nday > 31) */
/*     { */
/* 	NEW_ERROR_AT("day must be at least 1 and at most 31.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */

/*     return ESSTEE_OK; */
/* } */


/* static int find_h_m_s_ms( */
/*     char *start,  */
/*     char **h,  */
/*     char **m,  */
/*     char **s, */
/*     char **ms) */
/* { */
/*     if(strlen(start) != 11) */
/*     { */
/* 	return ESSTEE_ERROR; */
/*     } */

/*     *h = start; */
/*     start[2] = '\0'; */

/*     *m = start + 3; */
/*     start[5] = '\0'; */

/*     *s = start + 6; */
/*     start[8] = '\0'; */

/*     *ms = start + 9; */
/*     start[11] = '\0'; */

/*     return ESSTEE_OK; */
/* } */

/* static int numberize_tod_strings( */
/*     unsigned *nh,  */
/*     unsigned *nm,  */
/*     unsigned *ns, */
/*     unsigned *nms,  */
/*     char *h,  */
/*     char *m,  */
/*     char *s, */
/*     char *ms,  */
/*     const struct location_t *string_location, */
/*     struct parser_t *parser) */
/* { */
/*     errno = 0; */
/*     *nh = (unsigned)(strtoul(h, NULL, 10)); */
/*     if(errno != 0) */
/*     { */
/* 	NEW_ERROR_AT("cannot interpret hour part.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */
/*     *nm = (unsigned)(strtoul(m, NULL, 10)); */
/*     if(errno != 0) */
/*     { */
/* 	NEW_ERROR_AT("cannot interpret minutes part.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */
/*     *ns = (unsigned)(strtoul(s, NULL, 10)); */
/*     if(errno != 0) */
/*     { */
/* 	NEW_ERROR_AT("cannot interpret seconds part.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */
/*     *nms = (unsigned)(strtoul(ms, NULL, 10));	 */
/*     if(errno != 0) */
/*     { */
/* 	NEW_ERROR_AT("cannot interpret fractional seconds part.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */

/*     if(*nh > 23) */
/*     { */
/* 	NEW_ERROR_AT("hours must be at most 24.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */
/*     if(*nm > 59) */
/*     { */
/* 	NEW_ERROR_AT("minutes must be at most 59.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */
/*     if(*ns > 59) */
/*     { */
/* 	NEW_ERROR_AT("seconds must be at most 59.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */
/*     if(*nms > 99) */
/*     { */
/* 	NEW_ERROR_AT("fractional seconds must be at most 99.", parser->errors, string_location);		 */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */
/*     *nms *= 10; */

/*     return ESSTEE_OK; */
/* } */

