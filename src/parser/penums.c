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
#include <elements/enums.h>


struct value_iface_t * st_new_enum_value(
    char *identifier,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct value_iface_t *ev = st_create_enum_value(identifier,
						    location,
						    parser->config,
						    parser->errors);
    if(!ev)
    {
	goto error_free_resources;
    }
    
    return ev;
    
error_free_resources:
    free(identifier);
    return NULL;
}

struct enum_group_iface_t * st_append_new_enum_item(
    struct enum_group_iface_t *enum_group,
    char *identifier,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    if(!enum_group)
    {
	enum_group = st_create_enum_group(parser->config,
					  parser->errors);

	if(!enum_group)
	{
	    goto error_free_resources;
	}
    }

    int extend_result = enum_group->extend(enum_group,
					   identifier,
					   location,
					   parser->config,
					   parser->errors);
    if(extend_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    return enum_group;

error_free_resources:
    if(enum_group)
    {
	enum_group->destroy(enum_group);
    }
    free(identifier);
    return NULL;
}
