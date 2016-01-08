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


struct direct_address_t * st_new_direct_address(
    char *representation,
    const struct st_location_t *representation_location,
    struct parser_t *parser)
{
    /* TODO: implement construction of new direct address */
    return NULL;
}

/* static char * extract_address_attributes( */
/*     char *representation,  */
/*     bitflag_t *attr) */
/* { */
/*     *attr = 0; */
/*     char *itr = representation; */
/*     if(*itr != '%') */
/*     { */
/* 	return NULL; */
/*     } */

/*     itr++; */
/*     switch(*itr) */
/*     { */
/*     case 'I': */
/* 	*attr |= INPUT_DIRECT_ADDRESS; */
/* 	break; */

/*     case 'Q': */
/* 	*attr |= OUTPUT_DIRECT_ADDRESS; */
/* 	break; */

/*     case 'M': */
/*     default: */
/* 	*attr |= MEMORY_DIRECT_ADDRESS; */
/* 	break; */
/*     } */

/*     itr++; */
/*     switch(*itr) */
/*     { */
/*     case 'B': */
/* 	*attr |= BYTE_UNIT_ADDRESS; */
/* 	itr++; */
/* 	break; */

/*     case 'W': */
/* 	*attr |= WORD_UNIT_ADDRESS; */
/* 	itr++; */
/* 	break; */

/*     case 'D': */
/* 	*attr |= DWORD_UNIT_ADDRESS; */
/* 	itr++; */
/* 	break; */

/*     case 'L': */
/* 	*attr |= LONG_UNIT_ADDRESS; */
/* 	itr++; */
/* 	break; */
	
/*     case 'X': */
/* 	itr++; */
/*     default: */
/* 	*attr |= BIT_UNIT_ADDRESS; */
/* 	break; */
/*     } */

/*     return itr; */
/* } */

/* static int extract_offset( */
/*     unsigned long *byte_offset, */
/*     unsigned long *bit_offset, */
/*     char *address_representation,  */
/*     const struct st_location_t *address_location, */
/*     st_bitflag_t address_attributes,  */
/*     struct parser_t *parser) */
/* { */
/*     char *index = NULL; */
/*     char *subindex = NULL; */
/*     char *itr = NULL; */
/*     long index_value = 0; */
/*     long subindex_value = 0; */
/*     unsigned field_size_bytes = 1; */

/*     itr = address_representation; */
/*     index = itr; */
/*     for(;*itr != '\0'; itr++) */
/*     { */
/* 	if(*itr == '.') */
/* 	{ */
/* 	    *itr = '\0'; */
/* 	    subindex = itr + 1; */
/* 	    break; */
/* 	} */
/*     } */
		
/*     errno = 0; */
/*     index_value = strtol(index, NULL, 10); */
/*     if(errno != 0) */
/*     { */
/* 	parser->new_issue_at( */
/* 	NEW_ERROR_AT( */
/* 	    "cannot interpret index in address.", */
/* 	    parser->errors, */
/* 	    address_location */
/* 	    ); */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */

/*     if(!FLAG_IS_SET(address_attributes, BIT_UNIT_ADDRESS) && subindex) */
/*     { */
/* 	NEW_ERROR_AT( */
/* 	    "subindex does not have any meaning for addresses in non bit size.", */
/* 	    parser->errors, */
/* 	    address_location */
/* 	    ); */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */
	
/*     if(subindex) */
/*     { */
/* 	subindex_value = strtol(subindex, NULL, 10); */
/* 	if(errno != 0) */
/* 	{ */
/* 	    NEW_ERROR_AT( */
/* 		"cannot interpret subindex in address.", */
/* 		parser->errors, */
/* 		address_location); */
/* 	    parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	    return ESSTEE_ERROR; */
/* 	} */
/*     } */
/*     else */
/*     { */
/* 	subindex_value = 0; */
/*     } */

/*     if(FLAG_IS_SET(address_attributes, WORD_UNIT_ADDRESS)) */
/*     { */
/* 	field_size_bytes = 2; */
/*     } */
/*     else if(FLAG_IS_SET(address_attributes, DWORD_UNIT_ADDRESS)) */
/*     { */
/* 	field_size_bytes = 4; */
/*     } */
    
/*     if(st_valid_memory_area_offset( */
/* 	   index_value, */
/* 	   subindex_value, */
/* 	   field_size_bytes, */
/* 	   parser->direct_memory) == ESSTEE_ERROR) */
/*     { */
/* 	NEW_ERROR_AT( */
/* 	    "address is outside available memory area.", */
/* 	    parser->errors, */
/* 	    address_location); */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	return ESSTEE_ERROR; */
/*     } */
    
/*     if(FLAG_IS_SET(address_attributes, BIT_UNIT_ADDRESS)) */
/*     { */
/* 	*byte_offset = index_value; */
/* 	*bit_offset =  subindex_value; */
/*     } */
/*     else */
/*     { */
/* 	*byte_offset = index_value; */
/* 	*bit_offset = 0; */
/*     } */

/*     return ESSTEE_OK; */
/* } */

/* struct direct_address_t * st_new_direct_address( */
/*     char *representation, */
/*     const struct location_t *representation_location, */
/*     struct parser_t *parser) */
/* { */
/*     struct memory_address_t *m = NULL; */
/*     char *address_representation = NULL; */
/*     unsigned long byte_offset = 0, bit_offset = 0; */
/*     bitflag_t attr = 0; */

/*     if((address_representation = extract_address_attributes(representation, &(attr))) == NULL) */
/*     { */
/* 	NEW_ERROR_AT( */
/* 	    "cannot interpret attributes of address.",  */
/* 	    parser->errors,  */
/* 	    representation_location */
/* 	    ); */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     if(FLAG_IS_SET(attr, INPUT_DIRECT_ADDRESS)) */
/*     { */
/* 	NEW_ERROR_AT( */
/* 	    "input addresses not supported (only memory).",  */
/* 	    parser->errors,  */
/* 	    representation_location */
/* 	    ); */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */
/*     else if(FLAG_IS_SET(attr, OUTPUT_DIRECT_ADDRESS)) */
/*     { */
/* 	NEW_ERROR_AT( */
/* 	    "output addresses not supported (only memory).",  */
/* 	    parser->errors,  */
/* 	    representation_location */
/* 	    ); */
/* 	parser->error_strategy = PARSER_SKIP_ERROR_STRATEGY; */
/* 	goto error_free_resources; */
/*     } */

/*     if(extract_offset( */
/* 	   &byte_offset, */
/* 	   &bit_offset, */
/* 	   address_representation,  */
/* 	   representation_location,  */
/* 	   attr,  */
/* 	   parser) == ESSTEE_ERROR) */
/*     { */
/* 	goto error_free_resources; */
/*     } */

/*     m = (struct memory_address_t *)malloc(sizeof(struct memory_address_t)); */
/*     if(!m) */
/*     { */
/* 	goto error_free_resources; */
/*     } */

/*     m->address.address_class = attr; */
/*     m->byte_offset = byte_offset; */
/*     m->bit_offset = bit_offset; */
/*     free(representation); */

/*     return &(m->address); */

/* error_free_resources: */
/*     free(representation); */
/*     free(m); */
/*     return NULL; */
/* } */



