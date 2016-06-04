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
#include <elements/qualified_identifier.h>


struct qualified_part_t * st_new_inner_reference(
    char *identifier,
    struct qualified_part_t *outer,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct qualified_part_t *new_path = st_extend_qualified_path(outer,
								 identifier,
								 location,
								 parser->config,
								 parser->errors);
    if(!new_path)
    {
	st_destroy_qualified_path(outer);
	free(identifier);
    }

    return new_path;
}

struct qualified_part_t * st_attach_array_index_to_inner_ref(
    struct qualified_part_t *inner_ref,
    struct array_index_t *array_index,
    struct parser_t *parser)
{
    struct qualified_part_t *new_path = st_extend_qualified_path_by_index(inner_ref,
									  array_index,
									  parser->config,
									  parser->errors);
    if(!new_path)
    {
	st_destroy_qualified_path(inner_ref);
    }

    return new_path;
}

struct qualified_identifier_iface_t * st_new_qualified_identifier_inner_ref(
    char *base,
    const struct st_location_t *base_location,
    struct qualified_part_t *path,
    struct parser_t *parser)
{
    struct qualified_identifier_iface_t *qid =
	st_create_qualified_identifier(base,
				       base_location,
				       path,
				       parser->pou_var_ref_pool,
				       parser->config,
				       parser->errors);
    if(!qid)
    {
	st_destroy_qualified_path(path);
	free(base);
    }

    return qid;
}

struct qualified_identifier_iface_t * st_new_qualified_identifier_array_index(
    char *base,
    const struct st_location_t *base_location,    
    struct array_index_t *array_index,
    struct parser_t *parser)
{
    struct qualified_identifier_iface_t *qid =
	st_create_qualified_identifier_by_index(base,
						base_location,
						array_index,
						parser->pou_var_ref_pool,
						parser->config,
						parser->errors);
    if(!qid)
    {
	free(base);
	/* TODO: destroy array index */
    }

    return qid;
}
