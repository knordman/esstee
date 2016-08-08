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
#include <elements/directmemory.h>
#include <util/macros.h>

static char * extract_address_attributes(
    char *representation,
    st_bitflag_t *attr)
{
    *attr = 0;
    char *itr = representation;
    if(*itr != '%')
    {
	return NULL;
    }

    itr++;
    switch(*itr)
    {
    case 'I':
	*attr |= INPUT_DIRECT_ADDRESS;
	break;

    case 'Q':
	*attr |= OUTPUT_DIRECT_ADDRESS;
	break;

    case 'M':
    default:
	*attr |= MEMORY_DIRECT_ADDRESS;
	break;
    }

    itr++;
    switch(*itr)
    {
    case 'B':
	*attr |= BYTE_UNIT_ADDRESS;
	itr++;
	break;

    case 'W':
	*attr |= WORD_UNIT_ADDRESS;
	itr++;
	break;

    case 'D':
	*attr |= DWORD_UNIT_ADDRESS;
	itr++;
	break;

    case 'L':
	*attr |= LONG_UNIT_ADDRESS;
	itr++;
	break;
	
    case 'X':
	itr++;
    default:
	*attr |= BIT_UNIT_ADDRESS;
	break;
    }

    return itr;
}

static int extract_offset(
    struct direct_address_t *da,
    char *address_representation,
    const struct st_location_t *address_location,
    struct parser_t *parser)
{
    char *index = NULL;
    char *subindex = NULL;
    char *itr = NULL;
    long index_value = 0;
    long subindex_value = 0;
    da->field_size_bits = 0;
    
    itr = address_representation;
    index = itr;
    for(;*itr != '\0'; itr++)
    {
	if(*itr == '.')
	{
	    *itr = '\0';
	    subindex = itr + 1;
	    break;
	}
    }
		
    errno = 0;
    index_value = strtol(index, NULL, 10);
    if(errno != 0)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "cannot interpret index '%s' in '%s'",
	    index,
	    address_representation);
	
	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_ARGUMENT_ERROR,
	    1,
	    address_location);
	
	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }

    if(!ST_FLAG_IS_SET(da->class, BIT_UNIT_ADDRESS) && subindex)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "subindex '%s' in '%s.%s' does not have any meaning for addresses in non bit size'",
	    subindex,
	    index,
	    subindex);

	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_ARGUMENT_ERROR,
	    1,
	    address_location);
	
	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	return ESSTEE_ERROR;
    }
	
    if(subindex)
    {
	subindex_value = strtol(subindex, NULL, 10);
	if(errno != 0)
	{
	    const char *message = parser->errors->build_message(
		parser->errors,
		"cannot interpret subindex '%s' in '%s.%s'",
		subindex,
		index,
		subindex);
	
	    parser->errors->new_issue_at(
		parser->errors,
		message,
		ESSTEE_ARGUMENT_ERROR,
		1,
		address_location);
	
	    parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	    return ESSTEE_ERROR;
	}
    }
    else
    {
	subindex_value = 0;
    }

    if(ST_FLAG_IS_SET(da->class, WORD_UNIT_ADDRESS))
    {
	da->field_size_bits = 2*8;
	index_value *= 2;
    }
    else if(ST_FLAG_IS_SET(da->class, DWORD_UNIT_ADDRESS))
    {
	da->field_size_bits = 4*8;
	index_value *= 4;
    }
    else if(ST_FLAG_IS_SET(da->class, LONG_UNIT_ADDRESS))
    {
	da->field_size_bits = 8*8;
	index_value *= 8;
    }
    else if(ST_FLAG_IS_SET(da->class, BYTE_UNIT_ADDRESS))
    {
	da->field_size_bits = 8;
    }
    else if(ST_FLAG_IS_SET(da->class, BIT_UNIT_ADDRESS))
    {
	da->field_size_bits = 1;
    }    
    
    if(ST_FLAG_IS_SET(da->class, BIT_UNIT_ADDRESS))
    {
	da->byte_offset = index_value;
	da->bit_offset =  subindex_value;
    }
    else
    {
	da->byte_offset = index_value;
	da->bit_offset = 0;
    }

    return ESSTEE_OK;
}

struct direct_address_t * st_new_direct_address(
    char *representation,
    const struct st_location_t *representation_location,
    struct parser_t *parser)
{
    struct direct_address_t *da = NULL;

    ALLOC_OR_ERROR_JUMP(
	da,
	struct direct_address_t,
	parser->errors,
	error_free_resources);
    
    char *address_representation = extract_address_attributes(representation,
							      &(da->class));
    
    if(!address_representation)
    {
	const char *message = parser->errors->build_message(
	    parser->errors,
	    "cannot extract attributes of direct address '%s'");

	parser->errors->new_issue_at(
	    parser->errors,
	    message,
	    ESSTEE_ARGUMENT_ERROR,
	    1,
	    representation_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	goto error_free_resources;
    }

    if(ST_FLAG_IS_SET(da->class, INPUT_DIRECT_ADDRESS))
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "input addresses are not supported (only memory)",
	    ESSTEE_ARGUMENT_ERROR,
	    1,
	    representation_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	goto error_free_resources;
    }
    else if(ST_FLAG_IS_SET(da->class, OUTPUT_DIRECT_ADDRESS))
    {
	parser->errors->new_issue_at(
	    parser->errors,
	    "output addresses are not supported (only memory)",
	    ESSTEE_ARGUMENT_ERROR,
	    1,
	    representation_location);

	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY;
	goto error_free_resources;
    }

    int extract_result = extract_offset(da,
					address_representation,
					representation_location,
					parser);
    
    if(extract_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    parser->errors->begin_group(parser->errors);
    uint8_t *offset = parser->direct_memory->offset(parser->direct_memory,
						    da,
						    parser->config,
						    parser->errors);
    if(!offset)
    {
	parser->errors->new_issue(parser->errors,
				  "invalid direct address",
				  ESSTEE_ARGUMENT_ERROR);

	parser->errors->set_group_location(parser->errors,
					   1,
					   representation_location);
    }
    parser->errors->end_group(parser->errors);
    
    if(!offset)
    {
	goto error_free_resources;
    }
    
    da->storage = offset;
    
    free(representation);

    return da;

error_free_resources:
    free(representation);
    free(da);
    return NULL;
}
