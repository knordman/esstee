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

#include <util/config.h>
#include <util/macros.h>
#include <esstee/flags.h>

#include <uthash.h>
#include <string.h>


static struct bool_option_t bool_options_template[] = {
    { .option = "resolve_links_on_parse_error",
      .value = ESSTEE_FALSE
    },
};

struct config_iface_t * st_new_config(void)
{
    struct config_t *conf = NULL;
    struct bool_option_t *bool_options = NULL;

    ALLOC_OR_JUMP(conf, struct config_t, error_free_resources);
    bool_options = (struct bool_option_t *)malloc(sizeof(bool_options_template));
    if(!bool_options)
    {
	goto error_free_resources;
    }

    memcpy(
	bool_options,
	bool_options_template,
	sizeof(bool_options_template));

    conf->options_chunk = bool_options;
    conf->options_table = NULL;
    
    for(int i=0; i < sizeof(bool_options_template)/sizeof(struct bool_option_t); i++)  
    {
	HASH_ADD_KEYPTR( 
	    hh,  
	    conf->options_table,  
	    conf->options_chunk[i].option,
	    strlen(conf->options_chunk[i].option),  
	    &(conf->options_chunk[i]));
    }
    
    conf->config.get = st_config_get;
    conf->config.set = st_config_set;

    return &(conf->config);
    
error_free_resources:
    free(conf);
    free(bool_options);
    return NULL;
}

int st_config_get(
    const struct config_iface_t *self,
    const char *option)
{
    const struct config_t *conf = CONTAINER_OF(self, struct config_t, config);

    struct bool_option_t *found = NULL;
    
    HASH_FIND_STR(conf->options_table, option, found);
    if(!found)
    {
	return ESSTEE_ERROR;
    }

    return found->value;
}

int st_config_set(
    struct config_iface_t *self,
    const char *option,
    int value)
{
    struct config_t *conf = CONTAINER_OF(self, struct config_t, config);

    struct bool_option_t *found = NULL;
    
    HASH_FIND_STR(conf->options_table, option, found);
    if(!found)
    {
	return ESSTEE_ERROR;
    }

    if(value == ESSTEE_TRUE)
    {
	found->value = ESSTEE_TRUE;
    }
    else
    {
	found->value = ESSTEE_FALSE;
    }

    return ESSTEE_OK;
}

void st_destroy_config(
    struct config_iface_t *self)
{
    struct config_t *conf = CONTAINER_OF(self, struct config_t, config);

    free(conf->options_chunk);
    free(conf);
}
