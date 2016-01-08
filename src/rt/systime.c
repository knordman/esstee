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

#include <rt/systime.h>
#include <util/macros.h>
#include <esstee/flags.h>

#include <stddef.h>


struct systime_iface_t * st_new_systime(void)
{
    struct systime_t *syst = NULL;
    ALLOC_OR_JUMP(
	syst,
	struct systime_t,
	error_free_resources);

    syst->systime.add_time_ms = st_systime_add_time_ms;
    syst->systime.get_time_ms = st_systime_get_time_ms;
    syst->systime.reset = st_systime_reset;

    return &(syst->systime);
    
error_free_resources:
    return NULL;
}

int st_systime_reset(
    struct systime_iface_t *self)
{
    struct systime_t *syst =
	CONTAINER_OF(self, struct systime_t, systime);

    syst->current_time = 0;

    return ESSTEE_OK;
}
    
uint64_t st_systime_get_time_ms(
    struct systime_iface_t *self)
{
    struct systime_t *syst =
	CONTAINER_OF(self, struct systime_t, systime);

    return syst->current_time;    
}

uint64_t st_systime_add_time_ms(
    struct systime_iface_t *self,
    uint64_t ms)
{
    struct systime_t *syst =
	CONTAINER_OF(self, struct systime_t, systime);

    syst->current_time += ms;

    return ESSTEE_OK;
}
