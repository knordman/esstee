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

#include <elements/block_header.h>
#include <linker/linker.h>


int st_create_header_tables(
    struct header_t *header,
    struct issues_iface_t *issues)
{
    if(!header)
    {
	return ESSTEE_OK;
    }

    int result = ESSTEE_OK;
    
    /* Make hash table from type list */
    int issues_at_type_start = issues->count(issues, ESSTEE_FILTER_ANY_ERROR);
    struct type_iface_t *type_table =
	st_link_types(header->types, NULL, issues);

    if(issues->count(issues, ESSTEE_FILTER_ANY_ERROR) == issues_at_type_start)
    {
	header->types = type_table;
    }
    else
    {
	result = ESSTEE_ERROR;
    }

    /* Make hash table from var list */
    int issues_at_var_start = issues->count(issues, ESSTEE_FILTER_ANY_ERROR);
    struct variable_iface_t *variable_table = st_link_variables(header->variables,
								NULL,
								issues);

    if(issues->count(issues, ESSTEE_FILTER_ANY_ERROR) == issues_at_var_start)
    {
	header->variables = variable_table;
    }
    else
    {
	result = ESSTEE_ERROR;
    }

    return result;
}

void st_destroy_header(
    struct header_t *header)
{
    /* TODO: destroy block header */
}
