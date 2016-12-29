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
#include <elements/subrange.h>


struct subrange_iface_t * st_new_subrange(
    struct value_iface_t *min,
    const struct st_location_t *min_location,
    struct value_iface_t *max,
    const struct st_location_t *max_location,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct subrange_iface_t *sr = st_create_subrange(min,
						     min_location,
						     max,
						     max_location,
						     location,
						     parser->config,
						     parser->errors);
    if(!sr)
    {
	min->destroy(min);
	max->destroy(max);
    }

    return sr;
}

