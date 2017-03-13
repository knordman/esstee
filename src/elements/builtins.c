/*
Copyright (C) 2017 Kristian Nordman

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

#include <elements/builtins.h>
#include <elements/integers.h>
#include <elements/variable.h>
#include <statements/iinvoke.h>
#include <util/iissues.h>
#include <util/macros.h>

#include <string.h>
#include <stdio.h>

/**************************************************************************/
/* Integer cast function                                                  */
/**************************************************************************/
struct integer_cast_t {
    struct function_iface_t function;
    struct variable_iface_t *in;
    struct variable_iface_t *out;
    char *name;
};

static int integer_cast_finalize_header(
    struct function_iface_t *self,
    struct type_iface_t *global_type_table,
    struct variable_iface_t *global_var_table,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}

static int integer_cast_finalize_statements(
    struct function_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}
    
static int integer_cast_verify_invoke(
    struct function_iface_t *self,
    struct invoke_parameters_iface_t *parameters,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_cast_t *ic =
	CONTAINER_OF(self, struct integer_cast_t, function);

    if(!parameters)
    {
	issues->new_issue(issues,
			  "missing argument to cast function %s",
			  ESSTEE_ARGUMENT_ERROR,
			  ic->name);

	return ESSTEE_ERROR;
    }

    return parameters->verify(parameters,
			      ic->in,
			      config,
			      issues);
}

static int integer_cast_step(
    struct function_iface_t *self,
    struct invoke_parameters_iface_t *parameters,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct integer_cast_t *ic =
	CONTAINER_OF(self, struct integer_cast_t, function);
    
    int input_assign_result = parameters->assign_from(parameters,
						      ic->in,
						      config,
						      issues);
    if(input_assign_result != ESSTEE_OK)
    {
	return INVOKE_RESULT_ERROR;
    }

    const struct value_iface_t *in_value = ic->in->value(ic->in);

    int cast_assign_result = ic->out->cast_assign(ic->out,
						  NULL,
						  in_value,
						  config,
						  issues);
    if(cast_assign_result != ESSTEE_OK)
    {
    	return INVOKE_RESULT_ERROR;
    }
    
    return INVOKE_RESULT_FINISHED;
}

static int integer_cast_reset(
    struct function_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return ESSTEE_OK;
}
    
static const struct value_iface_t * integer_cast_result_value(
    struct function_iface_t *self)
{
    struct integer_cast_t *ic =
	CONTAINER_OF(self, struct integer_cast_t, function);

    return ic->out->value(ic->out);
}

static void integer_cast_destroy(
    struct function_iface_t *self)
{
    /* TODO: integer cast destructor */
}


static const char * cast_integer_types[] = {
    "BOOL",
    "SINT",
    "INT",
    "DINT",
    "LINT",
    "USINT",
    "UINT",
    "UDINT",
    "ULINT",
    "BYTE",
    "WORD",
    "DWORD",
    "LWORD",
};

static struct function_iface_t * create_integer_cast(
    const char *from_name,
    const char *to_name,
    struct type_iface_t *builtin_types)
{
    char name_buffer[32];
    char *name = NULL;
    struct type_iface_t *from_type = NULL;
    struct type_iface_t *to_type = NULL;
    struct integer_cast_t *cast = NULL;
    struct variable_iface_t *in_var = NULL;
    struct variable_iface_t *out_var = NULL;
    
    HASH_FIND_STR(builtin_types, from_name, from_type);
    HASH_FIND_STR(builtin_types, to_name, to_type);

    if(!from_type || !to_type)
    {
	return NULL;
    }

    in_var = st_create_variable_type("IN",
				     NULL,
				     from_type,
				     INPUT_VAR_CLASS,
				     NULL,
				     NULL);
    if(!in_var)
    {
	goto error_free_resources;
    }

    int create_in_result = in_var->create(in_var, NULL, NULL);
    if(create_in_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    out_var = st_create_variable_type("OUT",
				      NULL,
				      to_type,
				      OUTPUT_VAR_CLASS,
				      NULL,
				      NULL);

    if(!out_var)
    {
	goto error_free_resources;
    }

    int create_out_result = out_var->create(out_var, NULL, NULL);
    if(create_out_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    ALLOC_OR_JUMP(
	cast,
	struct integer_cast_t,
	error_free_resources);
    
    snprintf(
	name_buffer,
	sizeof(name_buffer),
	"%s_TO_%s",
	from_name,
	to_name);

    STRDUP_OR_JUMP(
	name,
	name_buffer,
	error_free_resources);

    cast->name = name;
    cast->in = in_var;
    cast->out = out_var;

    memset(&(cast->function), 0, sizeof(struct function_iface_t));
    cast->function.finalize_header = integer_cast_finalize_header;
    cast->function.finalize_statements = integer_cast_finalize_statements;
    cast->function.verify_invoke = integer_cast_verify_invoke;
    cast->function.step = integer_cast_step;
    cast->function.reset = integer_cast_reset;
    cast->function.result_value = integer_cast_result_value;
    cast->function.destroy = integer_cast_destroy;

    cast->function.identifier = cast->name;
    
    return &(cast->function);

error_free_resources:
    /* TODO: determine what to destroy */
    return NULL;
}

struct function_iface_t * st_new_builtin_functions(
    struct type_iface_t *builtin_types)
{
    struct function_iface_t *functions = NULL;

    size_t num_cast_types =
    	sizeof(cast_integer_types)/sizeof(const char *);

    for(int i=0; i < num_cast_types; i++)
    {
    	const char *from_name = cast_integer_types[i];
	
    	for(int j=0; j < num_cast_types; j++)
    	{
    	    const char *to_name = cast_integer_types[j];

    	    if(from_name == to_name) // Yes, comparing pointers not content
    	    {
    		continue;
    	    }

    	    struct function_iface_t *cast_function =
    	    	create_integer_cast(from_name,
    	    			    to_name,
    	    			    builtin_types);

    	    if(!cast_function)
    	    {
    	    	goto error_free_resources;
    	    }

    	    HASH_ADD_KEYPTR(
    		hh,
    		functions,
    		cast_function->identifier,
    		strlen(cast_function->identifier),
    		cast_function);
    	}
    }

    return functions;

error_free_resources:
    /* TODO: what do destroy */
    return NULL;
}

struct function_block_iface_t * st_new_builtin_function_blocks(
    struct type_iface_t *builtin_types)
{
    return NULL;
}
