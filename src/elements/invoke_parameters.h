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

#include <elements/iinvoke_parameter.h>
#include <expressions/iexpression.h>

struct invoke_parameters_iface_t * st_create_invoke_parameters(
    struct invoke_parameter_t *first_parameter,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);	
    
struct invoke_parameter_t * st_create_invoke_parameter(
    char *identifier,
    const struct st_location_t *location,
    struct expression_iface_t *assigned,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);	

void st_destroy_invoke_parameter(
    struct invoke_parameter_t *parameter);
