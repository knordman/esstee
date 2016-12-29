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

#include <expressions/negative_prefix.h>
#include <util/macros.h>

/**************************************************************************/
/* Expression interface                                                   */
/**************************************************************************/
struct negative_prefix_term_t {
    struct expression_iface_t expression;
    struct value_iface_t *temporary;
    struct expression_iface_t *to_negate;
    struct st_location_t *location;
    size_t operation_offset;
    int invoke_state;
};

typedef int (*negation_operation_t)(
    const struct value_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues);

static int assign_temporary_and_negate(
    struct negative_prefix_term_t *nt,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct value_iface_t *to_negate_value = 
	nt->to_negate->return_value(nt->to_negate);

    int assign_result = ESSTEE_ERROR;
    int negate_result = ESSTEE_ERROR;

    struct issue_group_iface_t *ig = issues->open_group(issues);

    assign_result = nt->temporary->assign(nt->temporary,
					  to_negate_value,
					  config,
					  issues);
    if(assign_result == ESSTEE_OK)
    {
	negation_operation_t *operation
	    = (negation_operation_t *)(((char *)nt->temporary) + nt->operation_offset);
	
	negate_result = (*operation)(nt->temporary, config, issues);
    }

    ig->close(ig);

    if(assign_result != ESSTEE_OK || negate_result != ESSTEE_OK)
    {
	ig->main_issue(ig,
		       "expression evaluation failed",
		       ESSTEE_RUNTIME_ERROR,
		       1,
		       nt->location);

	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int negative_prefix_term_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct negative_prefix_term_t *nt =
	CONTAINER_OF(expr, struct negative_prefix_term_t, expression);
	
    if(nt->to_negate->invoke.verify)
    {
	int negate_result = nt->to_negate->invoke.verify(&(nt->to_negate->invoke),
							 config,
							 issues);
	if(negate_result != ESSTEE_OK)
	{
	    return negate_result;
	}
    }

    const struct value_iface_t *to_negate_value =
	nt->to_negate->return_value(nt->to_negate);

    negation_operation_t *operation
	= (negation_operation_t *)(((char *)to_negate_value) + nt->operation_offset);
    
    if(!operation)
    {
	issues->new_issue_at(
	    issues,
	    "value cannot be negated",
	    ESSTEE_CONTEXT_ERROR,
	    1,
	    nt->to_negate->invoke.location);

	return ESSTEE_ERROR;
    }
        
    return ESSTEE_OK;
}

static int negative_prefix_term_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    int verify_result = negative_prefix_term_verify(self,
						    config,
						    issues);
    if(verify_result != ESSTEE_OK)
    {
	return verify_result;
    }

    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct negative_prefix_term_t *nt =
	CONTAINER_OF(expr, struct negative_prefix_term_t, expression);
    
    return assign_temporary_and_negate(nt, config, issues);
}

static int negative_prefix_term_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct negative_prefix_term_t *nt =
	CONTAINER_OF(expr, struct negative_prefix_term_t, expression);
    
    switch(nt->invoke_state)
    {
    case 0:
	nt->invoke_state = 1;
	cursor->switch_current(cursor,
			       &(nt->to_negate->invoke),
			       config,
			       issues);
	return INVOKE_RESULT_IN_PROGRESS;

    case 1:
	if(assign_temporary_and_negate(nt, config, issues) == ESSTEE_ERROR)
	{
	    return INVOKE_RESULT_ERROR;
	}
    }

    return INVOKE_RESULT_FINISHED;
}

static int negative_prefix_term_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct negative_prefix_term_t *nt =
	CONTAINER_OF(expr, struct negative_prefix_term_t, expression);

    if(nt->to_negate->invoke.reset)
    {
	int reset_result = nt->to_negate->invoke.reset(&(nt->to_negate->invoke),
						       config,
						       issues);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    nt->invoke_state = 0;

    return ESSTEE_OK;
}

static const struct value_iface_t * negative_prefix_term_return_value(
    const struct expression_iface_t *self)
{
    const struct negative_prefix_term_t *nt =
	CONTAINER_OF(self, struct negative_prefix_term_t, expression);

    return nt->temporary;
}

static int negative_prefix_term_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct negative_prefix_term_t *nt =
	CONTAINER_OF(expr, struct negative_prefix_term_t, expression);

    if(nt->to_negate->invoke.allocate)
    {
	int allocate_result = nt->to_negate->invoke.allocate(
	    &(nt->to_negate->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    const struct value_iface_t *to_negate_value =
	nt->to_negate->return_value(nt->to_negate);
    
    if(!to_negate_value->create_temp_from)
    {
	issues->new_issue_at(
	    issues,
	    "value cannot be used in an expression",
	    ESSTEE_CONTEXT_ERROR,
	    1,
	    nt->to_negate->invoke.location);
	
	return ESSTEE_ERROR;
    }
    
    nt->temporary = to_negate_value->create_temp_from(to_negate_value, issues);
    if(!nt->temporary)
    {
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

void negative_prefix_term_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

void negative_prefix_term_clone_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

static struct expression_iface_t * negative_prefix_term_clone(
    struct expression_iface_t *self,
    struct issues_iface_t *issues)
{
    struct negative_prefix_term_t *nt =
	CONTAINER_OF(self, struct negative_prefix_term_t, expression);

    struct negative_prefix_term_t *copy = NULL;
    struct expression_iface_t *to_negate_copy = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct negative_prefix_term_t,
	issues,
	error_free_resources);

    memcpy(copy, nt, sizeof(struct negative_prefix_term_t));
    
    if(nt->to_negate->clone)
    {
	to_negate_copy = nt->to_negate->clone(nt->to_negate, issues);

	if(!to_negate_copy)
	{
	    goto error_free_resources;
	}

	copy->to_negate = to_negate_copy;
    }
    else
    {
	copy->to_negate = nt->to_negate;
    }

    copy->expression.destroy = negative_prefix_term_clone_destroy;

    return &(copy->expression);

error_free_resources:
    free(copy);
    free(to_negate_copy);
    return NULL;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
static struct expression_iface_t * create_negative_prefix_term(
    struct expression_iface_t *term,
    const struct st_location_t *location,
    size_t operation_offset,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct negative_prefix_term_t *nt = NULL;
    struct st_location_t *nt_location = NULL;

    ALLOC_OR_ERROR_JUMP(
	nt,
	struct negative_prefix_term_t,
	issues,
	error_free_resources);
    
    LOCDUP_OR_ERROR_JUMP(
	nt_location,
	location,
	issues,
	error_free_resources);

    nt->location = nt_location;
    nt->to_negate = term;
    nt->operation_offset = operation_offset;

    memset(&(nt->expression), 0, sizeof(struct expression_iface_t));
    
    if(nt->to_negate->invoke.step || nt->to_negate->clone)
    {
	nt->expression.invoke.step = negative_prefix_term_step;
	nt->expression.invoke.verify = negative_prefix_term_verify;
	nt->expression.invoke.reset = negative_prefix_term_reset;
    }
    else
    {
	nt->expression.invoke.verify = negative_prefix_term_constant_verify;
    }

    if(nt->to_negate->clone)
    {
	nt->expression.clone = negative_prefix_term_clone;
    }
    
    nt->expression.invoke.location = nt->location;
    nt->expression.invoke.allocate = negative_prefix_term_allocate;
    nt->expression.return_value = negative_prefix_term_return_value;
    nt->expression.destroy = negative_prefix_term_destroy;

    return &(nt->expression);

error_free_resources:
    free(nt);
    free(nt_location);
    return NULL;
}

struct expression_iface_t * st_create_negative_prefix_term(
    struct expression_iface_t *term,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return create_negative_prefix_term(term,
				       location,
				       offsetof(struct value_iface_t, negate),
				       config,
				       issues);
}

struct expression_iface_t * st_create_not_prefix_term(
    struct expression_iface_t *term,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return create_negative_prefix_term(term,
				       location,
				       offsetof(struct value_iface_t, not),
				       config,
				       issues);
}
