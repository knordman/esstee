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

#include <parser/parser.h>
#include <elements/invoke_parameters.h>


struct invoke_parameter_t * st_new_invoke_parameter(
    char *identifier,
    const struct st_location_t *location,
    struct expression_iface_t *assigned,
    struct parser_t *parser)
{
    struct invoke_parameter_t *param = st_create_invoke_parameter(identifier,
								  location,
								  assigned,
								  parser->config,
								  parser->errors);

    if(!param)
    {
	free(identifier);
	assigned->destroy(assigned);
    }

    return param;
}

struct invoke_parameters_iface_t * st_append_invoke_parameter(
    struct invoke_parameters_iface_t *parameter_group,
    struct invoke_parameter_t *new_parameter,
    struct parser_t *parser)
{
    if(!parameter_group)
    {
	parameter_group = st_create_invoke_parameters(new_parameter,
						      parser->config,
						      parser->errors);

	if(!parameter_group)
	{
	    st_destroy_invoke_parameter(new_parameter);
	    return NULL;
	}
	else
	{
	    return parameter_group;
	}
    }

    int append_result = parameter_group->append(parameter_group,
						new_parameter,
						parser->config,
						parser->errors);
    if(append_result != ESSTEE_OK)
    {
	parameter_group->destroy(parameter_group);
	st_destroy_invoke_parameter(new_parameter);
	parameter_group = NULL;
    }

    return parameter_group;
}
