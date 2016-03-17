/*
Copyright (C) 2015 Kristian Nordman

This file is part of esstee. 

esstee is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

nesstee is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with esstee.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <elements/expressions.h>
#include <elements/shared.h>
#include <elements/types.h>
#include <util/macros.h>

#include <utlist.h>
#include <stdio.h>


/**************************************************************************/
/* Wrapped value expression (e.g. literal)                                */
/**************************************************************************/
const struct st_location_t * st_value_expression_location(
    const struct invoke_iface_t *self)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);

    struct value_expression_t *ve
	= CONTAINER_OF(e, struct value_expression_t, expression);

    return ve->location;
}
    
const struct value_iface_t * st_value_expression_return_value(
    struct expression_iface_t *self)
{
    struct value_expression_t *ve
	= CONTAINER_OF(self, struct value_expression_t, expression);

    return ve->value;
}

void st_value_expression_destroy(
    struct expression_iface_t *self)
{
    /* TODO: value expression destructor */
}

/**************************************************************************/
/* Single identifier term, either an enum, or a variable reference        */
/**************************************************************************/
const struct st_location_t * st_single_identifier_term_location(
    const struct invoke_iface_t *self)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct single_identifier_term_t *sit =
	CONTAINER_OF(e, struct single_identifier_term_t, expression);

    return sit->location;
}

struct expression_iface_t * st_single_identifier_term_clone(
    struct expression_iface_t *self)
{
    struct single_identifier_term_t *sit =
	CONTAINER_OF(self, struct single_identifier_term_t, expression);

    struct single_identifier_term_t *copy = NULL;
    ALLOC_OR_JUMP(
	copy,
	struct single_identifier_term_t,
	error_free_resources);

    memcpy(copy, sit, sizeof(struct single_identifier_term_t));
    copy->expression.destroy = st_single_identfier_term_clone_destroy;

    return &(copy->expression);

error_free_resources:
    return NULL;
}

const struct value_iface_t * st_single_identifier_term_var_return_value(
    struct expression_iface_t *self)
{
    struct single_identifier_term_t *sit =
	CONTAINER_OF(self, struct single_identifier_term_t, expression);

    return sit->variable->value;
}

const struct value_iface_t * st_single_identifier_term_enum_return_value(
    struct expression_iface_t *self)
{
    struct single_identifier_term_t *sit = 
	CONTAINER_OF(self, struct single_identifier_term_t, expression);

    return &(sit->inline_enum.value);
}


void st_single_identifier_term_destroy(
    struct expression_iface_t *self)
{
    /* TODO: single identifier term destructor */
}

void st_single_identfier_term_clone_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

/**************************************************************************/
/* Qualified identifier term                                              */
/**************************************************************************/
const struct st_location_t * st_qualified_identifier_term_location(
    const struct invoke_iface_t *self)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct qualified_identifier_term_t *qit
	= CONTAINER_OF(expr, struct qualified_identifier_term_t, expression);

    return qit->location;
}

int st_qualified_identifier_term_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct qualified_identifier_term_t *qit
	= CONTAINER_OF(expr, struct qualified_identifier_term_t, expression);

    return st_qualified_identifier_verify(qit->identifier,
					  errors,
					  config);
}

int st_qualified_identifier_term_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct qualified_identifier_term_t *qit
	= CONTAINER_OF(expr, struct qualified_identifier_term_t, expression);

    return st_qualified_identifier_step(qit->identifier,
					cursor,
					errors,
					config);
}

int st_qualified_identifier_term_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct qualified_identifier_term_t *qit
	= CONTAINER_OF(expr, struct qualified_identifier_term_t, expression);

    int reset = st_qualified_identifier_reset(qit->identifier, config);
    if(reset != ESSTEE_OK)
    {
	return reset;
    }

    return ESSTEE_OK;
}

const struct value_iface_t * st_qualified_identifier_term_return_value(
    struct expression_iface_t *self)
{
    struct qualified_identifier_term_t *qit =
	CONTAINER_OF(self, struct qualified_identifier_term_t, expression);

    /* TODO: return a valid value (for verification) even though the
     * array index is !runtime constant? */
    
    return qit->identifier->target;
}

struct expression_iface_t * st_qualified_identifier_term_clone(
    struct expression_iface_t *self)
{
    struct qualified_identifier_term_t *qit
	= CONTAINER_OF(self, struct qualified_identifier_term_t, expression);

    struct qualified_identifier_term_t *copy = NULL;
    struct qualified_identifier_t *identifier_copy = NULL;

    ALLOC_OR_JUMP(
	copy,
	struct qualified_identifier_term_t,
	error_free_resources);

    memcpy(copy, qit, sizeof(struct qualified_identifier_term_t));

    identifier_copy = st_clone_qualified_identifier(qit->identifier);

    if(!identifier_copy)
    {
	goto error_free_resources;
    }

    copy->identifier = identifier_copy;
    
    return &(copy->expression);

error_free_resources:
    free(copy);
    return NULL;
}

void st_qualified_identifier_term_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor qualified identifier term */
}

/**************************************************************************/
/* Negative prefix term                                                   */
/**************************************************************************/
static int assign_temporary_and_negate(
    struct negative_prefix_term_t *nt,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    const struct value_iface_t *to_negate_value = 
	nt->to_negate->return_value(nt->to_negate);

    int assign_result = ESSTEE_ERROR;
    int negate_result = ESSTEE_ERROR;    
    
    assign_result = nt->temporary->assign(nt->temporary,
					  to_negate_value,
					  config);
    if(assign_result == ESSTEE_OK)
    {
	negate_result = nt->temporary->negate(nt->temporary, config);
    }

    if(assign_result != ESSTEE_OK || negate_result != ESSTEE_OK)
    {
	errors->new_issue_at(
	    errors,
	    "expression evaluation failed",
	    ISSUE_ERROR_CLASS,
	    1,
	    nt->location);
	    
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

const struct st_location_t * st_negative_prefix_term_location(
    const struct invoke_iface_t *self)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct negative_prefix_term_t *nt =
	CONTAINER_OF(expr, struct negative_prefix_term_t, expression);

    return nt->location;
}

int st_negative_prefix_term_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    int verify_result = st_negative_prefix_term_verify(self,
						       config,
						       errors);
    if(verify_result != ESSTEE_OK)
    {
	return verify_result;
    }

    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct negative_prefix_term_t *nt =
	CONTAINER_OF(expr, struct negative_prefix_term_t, expression);
    
    return assign_temporary_and_negate(nt, config, errors);
}

int st_negative_prefix_term_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct negative_prefix_term_t *nt =
	CONTAINER_OF(expr, struct negative_prefix_term_t, expression);

    if(nt->to_negate->invoke.verify)
    {
	int negate_result = nt->to_negate->invoke.verify(&(nt->to_negate->invoke),
							 config,
							 errors);
	if(negate_result != ESSTEE_OK)
	{
	    return negate_result;
	}
    }

    const struct value_iface_t *to_negate_value =
	nt->to_negate->return_value(nt->to_negate);

    if(!to_negate_value->create_temp_from)
    {
	errors->new_issue_at(
	    errors,
	    "value cannot be used in an expression",
	    ISSUE_ERROR_CLASS,
	    1,
	    nt->to_negate->invoke.location(&(nt->to_negate->invoke)));
	return ESSTEE_ERROR;
    }
    else if(!to_negate_value->negate)
    {
	errors->new_issue_at(
	    errors,
	    "value cannot be modified by a minus prefix",
	    ISSUE_ERROR_CLASS,
	    1,
	    nt->to_negate->invoke.location(&(nt->to_negate->invoke)));
	return ESSTEE_ERROR;
    }
    
    nt->temporary = to_negate_value->create_temp_from(to_negate_value);
    if(!nt->temporary)
    {
	errors->memory_error(
	    errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	return ESSTEE_ERROR;
    }
    
    return ESSTEE_OK;
}

int st_negative_prefix_term_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct negative_prefix_term_t *nt =
	CONTAINER_OF(expr, struct negative_prefix_term_t, expression);
    
    switch(nt->invoke_state)
    {
    case 0:
	nt->invoke_state = 1;
	st_switch_current(cursor, &(nt->to_negate->invoke), config);
	return INVOKE_RESULT_IN_PROGRESS;


    case 1:
	if(assign_temporary_and_negate(nt, config, errors) == ESSTEE_ERROR)
	{
	    return INVOKE_RESULT_ERROR;
	}
    }

    return INVOKE_RESULT_FINISHED;
}

int st_negative_prefix_term_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct negative_prefix_term_t *nt =
	CONTAINER_OF(expr, struct negative_prefix_term_t, expression);

    if(nt->to_negate->invoke.reset)
    {
	int reset_result = nt->to_negate->invoke.reset(&(nt->to_negate->invoke),
						       config);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    nt->invoke_state = 0;

    return ESSTEE_OK;
}

const struct value_iface_t * st_negative_prefix_term_return_value(
    struct expression_iface_t *self)
{
    struct negative_prefix_term_t *nt =
	CONTAINER_OF(self, struct negative_prefix_term_t, expression);

    return nt->temporary;
}

struct expression_iface_t * st_negative_prefix_term_clone(
    struct expression_iface_t *self)
{
    struct negative_prefix_term_t *nt =
	CONTAINER_OF(self, struct negative_prefix_term_t, expression);

    struct negative_prefix_term_t *copy = NULL;
    ALLOC_OR_JUMP(
	copy,
	struct negative_prefix_term_t,
	error_free_resources);

    memcpy(copy, nt, sizeof(struct negative_prefix_term_t));
    
    copy->to_negate = nt->to_negate->clone(nt->to_negate);
    copy->expression.destroy = st_negative_prefix_term_clone_destroy;

    return &(copy->expression);

error_free_resources:
    free(copy);
    return NULL;
}

void st_negative_prefix_term_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

void st_negative_prefix_term_clone_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

/**************************************************************************/
/* Function invocation term                                               */
/**************************************************************************/
const struct st_location_t * st_function_invocation_term_location(
    const struct invoke_iface_t *self)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct function_invocation_term_t *ft =
	CONTAINER_OF(expr, struct function_invocation_term_t, expression);

    return ft->location;
}

int st_function_invocation_term_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct function_invocation_term_t *ft =
	CONTAINER_OF(expr, struct function_invocation_term_t, expression);

    return st_verify_invoke_parameters(ft->parameters,
				       ft->function->header->variables,
				       errors,
				       config);
}

int st_function_invocation_term_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct function_invocation_term_t *ft =
	CONTAINER_OF(expr, struct function_invocation_term_t, expression);

    switch(ft->invoke_state)
    {
    case 0:
	st_push_return_context(cursor, self);

	ft->invoke_state = 1;
	int step_result = st_step_invoke_parameters(ft->parameters,
						    cursor,
						    time,
						    config,
						    errors);
	if(step_result != INVOKE_RESULT_FINISHED)
	{
	    return step_result;
	}

    case 1: {
	int assign_result = st_assign_from_invoke_parameters(ft->parameters,
							     ft->function->header->variables,
							     config,
							     errors);
	if(assign_result != ESSTEE_OK)
	{
	    return INVOKE_RESULT_ERROR;
	}
	
	ft->invoke_state = 2;
	st_switch_current(cursor, ft->function->statements, config);
	return INVOKE_RESULT_IN_PROGRESS;
    }
	
    case 2:
    default:
	break;
    }

    st_pop_return_context(cursor);
    
    return INVOKE_RESULT_FINISHED;
}

int st_function_invocation_term_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct function_invocation_term_t *ft =
	CONTAINER_OF(expr, struct function_invocation_term_t, expression);
    
    struct variable_t *itr = NULL;
    DL_FOREACH(ft->function->header->variables, itr)
    {
	int reset_result = itr->type->reset_value_of(itr->type,
						     itr->value,
						     config);

	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    int reset_result = st_reset_invoke_parameters(ft->parameters,
						  config);
    if(reset_result != ESSTEE_OK)
    {
	return reset_result;
    }

    return ESSTEE_OK;
}

const struct value_iface_t * st_function_invocation_term_return_value(
    struct expression_iface_t *self)
{
    struct function_invocation_term_t *ft =
	CONTAINER_OF(self, struct function_invocation_term_t, expression);
    
    return ft->function->output.value;
}

struct expression_iface_t * st_function_invocation_term_clone(
    struct expression_iface_t *self)
{
    struct function_invocation_term_t *ft =
	CONTAINER_OF(self, struct function_invocation_term_t, expression);

    struct function_invocation_term_t *copy = NULL;
    ALLOC_OR_JUMP(
	copy,
	struct function_invocation_term_t,
	error_free_resources);

    memcpy(copy, ft, sizeof(struct function_invocation_term_t));

    struct invoke_parameter_t *parameters_copy =
	st_clone_invoke_parameters(ft->parameters);
    if(!parameters_copy)
    {
	goto error_free_resources;
    }

    copy->parameters = parameters_copy;
    copy->expression.destroy = st_function_invocation_clone_destroy;

    return &(copy->expression);
    
error_free_resources:
    return NULL;
}

void st_function_invocation_term_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

void st_function_invocation_clone_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

/**************************************************************************/
/* Not prefix term                                                        */
/**************************************************************************/
static int assign_temporary_and_not(
    struct not_prefix_term_t *nt,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    const struct value_iface_t *to_not_value = 
	nt->to_not->return_value(nt->to_not);

    int assign_result = ESSTEE_ERROR;
    int not_result = ESSTEE_ERROR;    
    
    assign_result = nt->temporary->assign(nt->temporary,
					  to_not_value,
					  config);
    if(assign_result == ESSTEE_OK)
    {
	not_result = nt->temporary->not(nt->temporary, config);
    }

    if(assign_result != ESSTEE_OK || not_result != ESSTEE_OK)
    {
	errors->new_issue_at(
	    errors,
	    "expression evaluation failed",
	    ISSUE_ERROR_CLASS,
	    1,
	    nt->location);
	    
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

const struct st_location_t * st_not_prefix_term_location(
    const struct invoke_iface_t *self)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct not_prefix_term_t *nt =
	CONTAINER_OF(expr, struct not_prefix_term_t, expression);

    return nt->location;
}

int st_not_prefix_term_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    int verify_result = st_not_prefix_term_verify(self,
						  config,
						  errors);
    if(verify_result != ESSTEE_OK)
    {
	return verify_result;
    }

    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct not_prefix_term_t *nt =
	CONTAINER_OF(expr, struct not_prefix_term_t, expression);
    
    return assign_temporary_and_not(nt, config, errors);
}

int st_not_prefix_term_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct not_prefix_term_t *nt =
	CONTAINER_OF(expr, struct not_prefix_term_t, expression);

    if(nt->to_not->invoke.verify)
    {
	int not_result = nt->to_not->invoke.verify(&(nt->to_not->invoke),
							 config,
							 errors);
	if(not_result != ESSTEE_OK)
	{
	    return not_result;
	}
    }

    const struct value_iface_t *to_not_value =
	nt->to_not->return_value(nt->to_not);

    if(!to_not_value->create_temp_from)
    {
	errors->new_issue_at(
	    errors,
	    "value cannot be used in an expression",
	    ISSUE_ERROR_CLASS,
	    1,
	    nt->to_not->invoke.location(&(nt->to_not->invoke)));
	return ESSTEE_ERROR;
    }
    else if(!to_not_value->negate)
    {
	errors->new_issue_at(
	    errors,
	    "value cannot be modified by a not prefix",
	    ISSUE_ERROR_CLASS,
	    1,
	    nt->to_not->invoke.location(&(nt->to_not->invoke)));
	return ESSTEE_ERROR;
    }
    
    nt->temporary = to_not_value->create_temp_from(to_not_value);
    if(!nt->temporary)
    {
	errors->memory_error(
	    errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	return ESSTEE_ERROR;
    }
    
    return ESSTEE_OK;
}

int st_not_prefix_term_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct not_prefix_term_t *nt =
	CONTAINER_OF(expr, struct not_prefix_term_t, expression);
    
    switch(nt->invoke_state)
    {
    case 0:
	nt->invoke_state = 1;
	st_switch_current(cursor, &(nt->to_not->invoke), config);
	return INVOKE_RESULT_IN_PROGRESS;

    case 1:
	if(assign_temporary_and_not(nt, config, errors) == ESSTEE_ERROR)
	{
	    return INVOKE_RESULT_ERROR;
	}
    }

    return INVOKE_RESULT_FINISHED;
}

int st_not_prefix_term_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct not_prefix_term_t *nt =
	CONTAINER_OF(expr, struct not_prefix_term_t, expression);

    if(nt->to_not->invoke.reset)
    {
	int reset_result = nt->to_not->invoke.reset(&(nt->to_not->invoke),
						       config);
	if(reset_result != ESSTEE_OK)
	{
	    return reset_result;
	}
    }

    nt->invoke_state = 0;

    return ESSTEE_OK;
}

const struct value_iface_t * st_not_prefix_term_return_value(
    struct expression_iface_t *self)
{
    struct not_prefix_term_t *nt =
	CONTAINER_OF(self, struct not_prefix_term_t, expression);

    return nt->temporary;
}

struct expression_iface_t * st_not_prefix_term_clone(
    struct expression_iface_t *self)
{
    struct not_prefix_term_t *nt =
	CONTAINER_OF(self, struct not_prefix_term_t, expression);

    struct not_prefix_term_t *copy = NULL;
    ALLOC_OR_JUMP(
	copy,
	struct not_prefix_term_t,
	error_free_resources);

    memcpy(copy, nt, sizeof(struct not_prefix_term_t));
    
    copy->to_not = nt->to_not->clone(nt->to_not);
    copy->expression.destroy = st_negative_prefix_term_clone_destroy;

    return &(copy->expression);

error_free_resources:
    free(copy);
    return NULL;
}

void st_not_prefix_term_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

void st_not_prefix_term_clone_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

/**************************************************************************/
/* Binary expressions                                                     */
/**************************************************************************/
typedef int (*binary_operation_t)(
    struct value_iface_t *,
    const struct value_iface_t *,
    const struct config_iface_t *);

typedef int (*binary_comparison_t)(
    const struct value_iface_t *,
    const struct value_iface_t *,
    const struct config_iface_t *);

static int be_verify_operands(
    struct binary_expression_t *be,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    /* Verify right and left expressions */
    int left_operand_verification = ESSTEE_OK;
    if(be->left_operand->invoke.verify)
    {
	left_operand_verification
	    = be->left_operand->invoke.verify(
		&(be->left_operand->invoke),
		config,
		errors);
    }
    if(left_operand_verification != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    int right_operand_verification = ESSTEE_OK;
    if(be->right_operand->invoke.verify)
    {
	right_operand_verification
	    = be->right_operand->invoke.verify(
		&(be->right_operand->invoke),
		config,
		errors);
    }
    if(right_operand_verification != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int be_step_operands(
    struct binary_expression_t *be,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    switch(be->invoke_state)
    {
    case 0:
	if(be->left_operand->invoke.step)
	{
	    be->invoke_state = 1;
	    st_switch_current(cursor, &(be->left_operand->invoke), config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 1:
	if(be->right_operand->invoke.step)
	{
	    be->invoke_state = 2;
	    st_switch_current(cursor, &(be->right_operand->invoke), config);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 2:
    default:
	break;
    }
    
    return INVOKE_RESULT_FINISHED;
}

/* Binary expressions producing a new value */
static int be_create_cloned_temporary(
    struct binary_expression_t *be,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    const struct value_iface_t *left_value
	= be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);
    
    if(!left_value->operates_with)
    {
	errors->new_issue_at(
	    errors,
	    "value cannot be used in an expression",
	    ISSUE_ERROR_CLASS,
	    1,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)));

	return ESSTEE_ERROR;
    }

    int left_operates_with_right
	= left_value->operates_with(left_value, right_value, config);

    if(left_operates_with_right != ESSTEE_TRUE)
    {
	errors->new_issue_at(
	    errors,
	    "left value does not support the operation using the right value",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return ESSTEE_ERROR;	
    }
    
    /* Create a temporary value for the operation result*/
    be->temporary = left_value->create_temp_from(left_value);
    if(!be->temporary)
    {
	errors->memory_error(
	    errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int be_do_operation(
    struct binary_expression_t *be,
    size_t operation_offset,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
   const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);

    if(be->temporary->assign(be->temporary, left_value, config) != ESSTEE_OK)
    {
	errors->internal_error(
	    errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);
	return ESSTEE_ERROR;
    }

    const struct value_iface_t *right_value =
	be->right_operand->return_value(be->right_operand);

    binary_operation_t *operation
	= (binary_operation_t *)(((char *)be->temporary) + operation_offset);
    
    if((*operation)(be->temporary, right_value, config) != ESSTEE_OK)
    {
	errors->new_issue_at(
	    errors,
	    "expression evaluation failed",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int be_verify(
    struct invoke_iface_t *self,
    const char *not_supported_message,
    size_t operation_offset,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    if(be_verify_operands(be, config, errors) != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);

    binary_operation_t *operation
	= (binary_operation_t *)(((char *)left_value) + operation_offset);

    if(!operation)
    {
	errors->new_issue_at(
	    errors,
	    not_supported_message,
	    ISSUE_ERROR_CLASS,
	    1,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)));

	return ESSTEE_ERROR;
    }

    if(be_create_cloned_temporary(be, config, errors) != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int be_constant_verify(
    struct invoke_iface_t *self,
    const char *not_supported_message,
    size_t operation_offset,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    int verify_result = be_verify(self,
				  not_supported_message,
				  operation_offset,
				  config,
				  errors);
    if(verify_result != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);
    
    return be_do_operation(be, operation_offset, config, errors);
}

static int be_step(
    struct invoke_iface_t *self,
    size_t operation_offset,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    int inner_step_result = be_step_operands(be,
					     cursor,
					     time,
					     config,
					     errors);
    if(inner_step_result != INVOKE_RESULT_FINISHED)
    {
	return inner_step_result;
    }

    if(be_do_operation(be, operation_offset, config, errors) != ESSTEE_OK)
    {
	return INVOKE_RESULT_ERROR;
    }
    
    return INVOKE_RESULT_FINISHED;
}

/* Binary expressions producing true/false (comparisons) */
static int be_create_bool_temporary(
    struct binary_expression_t *be,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    const struct value_iface_t *left_value
	= be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);
    
    if(!left_value->comparable_to)
    {
	errors->new_issue_at(
	    errors,
	    "value cannot be compared",
	    ISSUE_ERROR_CLASS,
	    1,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)));

	return ESSTEE_ERROR;
    }

    int left_comparable_to_right
	= left_value->comparable_to(left_value, right_value, config);

    if(left_comparable_to_right != ESSTEE_TRUE)
    {
	errors->new_issue_at(
	    errors,
	    "left value is not comparable to the right value",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return ESSTEE_ERROR;	
    }
    
    /* Create a temporary boolean that can hold the result */
    be->temporary = st_bool_type_create_temp_value(config);
    if(!be->temporary)
    {
	errors->memory_error(
	    errors,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);

	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int be_do_comparison(
    struct binary_expression_t *be,
    size_t operation_offset,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value =
	be->right_operand->return_value(be->right_operand);

    binary_comparison_t comparison
	= (binary_comparison_t)(((char *)left_value) + operation_offset);

    int result = comparison(left_value, right_value, config);

    if(result == ESSTEE_ERROR)
    {
	errors->new_issue_at(
	    errors,
	    "expression evaluation failed",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return ESSTEE_ERROR;
    }
    else if(result == ESSTEE_TRUE)
    {
	st_bool_type_true(be->temporary);
    }
    else
    {
	st_bool_type_false(be->temporary);
    }

    return ESSTEE_OK;
}

static int be_comparison_verify(
    struct invoke_iface_t *self,
    const char *not_supported_message,
    size_t operation_offset,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    if(be_verify_operands(be, config, errors) != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);

    binary_comparison_t comparison
	= (binary_comparison_t)(((char *)left_value) + operation_offset);

    if(!comparison)
    {
	errors->new_issue_at(
	    errors,
	    not_supported_message,
	    ISSUE_ERROR_CLASS,
	    1,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)));

	return ESSTEE_ERROR;
    }

    if(be_create_bool_temporary(be, config, errors) != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int be_comparison_constant_verify(
    struct invoke_iface_t *self,
    const char *not_supported_message,
    size_t operation_offset,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    int verify_result = be_comparison_verify(self,
					     not_supported_message,
					     operation_offset,
					     config,
					     errors);
    if(verify_result != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);
    
    return be_do_comparison(be, operation_offset, config, errors);
}

static int be_comparison_step(
    struct invoke_iface_t *self,
    size_t operation_offset,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    int inner_step_result = be_step_operands(be,
					     cursor,
					     time,
					     config,
					     errors);
    if(inner_step_result != INVOKE_RESULT_FINISHED)
    {
	return inner_step_result;
    }

    if(be_do_comparison(be, operation_offset, config, errors) != ESSTEE_OK)
    {
	return INVOKE_RESULT_ERROR;
    }
    
    return INVOKE_RESULT_FINISHED;
}

const struct st_location_t * st_binary_expression_location(
    const struct invoke_iface_t *self)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);

    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    return be->location;
}

int st_binary_expression_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);

    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    be->invoke_state = 0;

    if(be->left_operand->invoke.reset)
    {
	int left_reset = be->left_operand->invoke.reset(
	    &(be->left_operand->invoke), config);

	if(left_reset != ESSTEE_OK)
	{
	    return left_reset;
	}
    }

    if(be->right_operand->invoke.reset)
    {
	int right_reset = be->right_operand->invoke.reset(
	    &(be->right_operand->invoke), config);

	if(right_reset != ESSTEE_OK)
	{
	    return right_reset;
	}
    }

    return ESSTEE_OK;
}

const struct value_iface_t * st_binary_expression_return_value(
    struct expression_iface_t *self)
{
    struct binary_expression_t *be =
	CONTAINER_OF(self, struct binary_expression_t, expression);

    return be->temporary;
}

struct expression_iface_t * st_binary_expression_clone(
    struct expression_iface_t *self)
{
    struct binary_expression_t *be =
	CONTAINER_OF(self, struct binary_expression_t, expression);

    struct binary_expression_t *copy = NULL;
    struct expression_iface_t *left_copy = NULL;
    struct expression_iface_t *right_copy = NULL;
    
    ALLOC_OR_JUMP(
	copy,
	struct binary_expression_t,
	error_free_resources);

    memcpy(copy, be, sizeof(struct binary_expression_t));

    if(be->left_operand->clone)
    {
	left_copy = be->left_operand->clone(be->left_operand);
	if(!left_copy)
	{
	    goto error_free_resources;
	}

	copy->left_operand = left_copy;
    }

    if(be->right_operand->clone)
    {
	right_copy = be->right_operand->clone(be->right_operand);
	if(!right_copy)
	{
	    goto error_free_resources;
	}

	copy->right_operand = right_copy;
    }

    copy->expression.destroy = st_binary_expression_clone_destroy;
    
    return &(copy->expression);
    
error_free_resources:
    free(copy);
    free(left_copy);
    free(right_copy);
    return NULL;
}

void st_binary_expression_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

void st_binary_expression_clone_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

/* xor */
int st_xor_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_constant_verify(self,
		     "value does not support the xor operation",
		     offsetof(struct value_iface_t, xor),
		     config,
		     errors);    
}

int st_xor_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_verify(self,
		     "value does not support the xor operation",
		     offsetof(struct value_iface_t, xor),
		     config,
		     errors);
}

int st_xor_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{    
    return be_step(self,
		   offsetof(struct value_iface_t, xor),
		   cursor,
		   time,
		   config,
		   errors);
}

/* and */
int st_and_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_constant_verify(self,
		     "value does not support the and operation",
		     offsetof(struct value_iface_t, and),
		     config,
		     errors);    
}

int st_and_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_verify(self,
		     "value does not support the and operation",
		     offsetof(struct value_iface_t, and),
		     config,
		     errors);    
}

int st_and_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_step(self,
		   offsetof(struct value_iface_t, and),
		   cursor,
		   time,
		   config,
		   errors);
}

int st_or_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_constant_verify(self,
		     "value does not support the or operation",
		     offsetof(struct value_iface_t, or),
		     config,
		     errors);    
}

int st_or_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_verify(self,
		     "value does not support the or operation",
		     offsetof(struct value_iface_t, or),
		     config,
		     errors);
}

int st_or_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_step(self,
		   offsetof(struct value_iface_t, or),
		   cursor,
		   time,
		   config,
		   errors);
}

/* plus */
int st_plus_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_constant_verify(self,
		     "value does not support the + operation",
		     offsetof(struct value_iface_t, plus),
		     config,
		     errors);    
}

int st_plus_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_verify(self,
		     "value does not support the + operation",
		     offsetof(struct value_iface_t, plus),
		     config,
		     errors);
}

int st_plus_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_step(self,
		   offsetof(struct value_iface_t, plus),
		   cursor,
		   time,
		   config,
		   errors);
}

/* minus */
int st_minus_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_constant_verify(self,
			      "value does not support the - operation",
			      offsetof(struct value_iface_t, minus),
			      config,
			      errors);
}

int st_minus_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_verify(self,
		     "value does not support the - operation",
		     offsetof(struct value_iface_t, minus),
		     config,
		     errors);
}

int st_minus_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_step(self,
		   offsetof(struct value_iface_t, minus),
		   cursor,
		   time,
		   config,
		   errors);
}

/* multiply */
int st_multiply_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_constant_verify(self,
			      "value does not support the * operation",
			      offsetof(struct value_iface_t, multiply),
			      config,
			      errors);
}

int st_multiply_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_verify(self,
		     "value does not support the * operation",
		     offsetof(struct value_iface_t, multiply),
		     config,
		     errors);
}

int st_multiply_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_step(self,
		   offsetof(struct value_iface_t, multiply),
		   cursor,
		   time,
		   config,
		   errors);
}

/* divide */
int st_division_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_constant_verify(self,
			      "value does not support the / operation",
			      offsetof(struct value_iface_t, divide),
			      config,
			      errors);
}

int st_division_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_verify(self,
		     "value does not support the / operation",
		     offsetof(struct value_iface_t, divide),
		     config,
		     errors);
}

int st_division_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_step(self,
		   offsetof(struct value_iface_t, divide),
		   cursor,
		   time,
		   config,
		   errors);
}

/* greater */
int st_greater_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_comparison_constant_verify(self,
					 "value does not support the > operation",
					 offsetof(struct value_iface_t, greater),
					 config,
					 errors);
}

int st_greater_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_comparison_verify(self,
				"value does not support the > operation",
				offsetof(struct value_iface_t, greater),
				config,
				errors);
}

int st_greater_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_comparison_step(self,
			      offsetof(struct value_iface_t, greater),
			      cursor,
			      time,
			      config,
			      errors);
}

/* lesser */
int st_lesser_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_comparison_constant_verify(self,
					 "value does not support the > operation",
					 offsetof(struct value_iface_t, lesser),
					 config,
					 errors);
}


int st_lesser_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_comparison_verify(self,
				"value does not support the > operation",
				offsetof(struct value_iface_t, lesser),
				config,
				errors);
}

int st_lesser_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_comparison_step(self,
			      offsetof(struct value_iface_t, lesser),
			      cursor,
			      time,
			      config,
			      errors);
}

/* equals */
int st_equals_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_comparison_constant_verify(self,
					 "value does not support the > operation",
					 offsetof(struct value_iface_t, equals),
					 config,
					 errors);

}

int st_equals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_comparison_verify(self,
				"value does not support the = operation",
				offsetof(struct value_iface_t, equals),
				config,
				errors);
}

int st_equals_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_comparison_step(self,
			      offsetof(struct value_iface_t, equals),
			      cursor,
			      time,
			      config,
			      errors);
}

/* greater or equals */
int st_gequals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *e =				       
	CONTAINER_OF(self, struct expression_iface_t, invoke);	
    struct binary_expression_t *be =				
	CONTAINER_OF(e, struct binary_expression_t, expression);	
									
    if(be_verify_operands(be, config, errors) != ESSTEE_OK)		
    {								
	return ESSTEE_ERROR;					
    }								
									
    const struct value_iface_t *left_value =			
	be->left_operand->return_value(be->left_operand);		

    if(!left_value->equals || !left_value->greater)
    {								
	errors->new_issue_at(					
	    errors,							
	    "value does not support the >= operation",
	    ISSUE_ERROR_CLASS,			
	    1,							
	    be->left_operand->invoke.location(&(be->left_operand->invoke)));

	return ESSTEE_ERROR;
    }								

    if(be_create_bool_temporary(be, config, errors) != ESSTEE_OK)		
    {								
	return ESSTEE_ERROR;					
    }								

    return ESSTEE_OK;
}

int st_gequals_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    int verify_result = st_gequals_expression_verify(self,
						     config,
						     errors);
    if(verify_result != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value =
	be->left_operand->return_value(be->right_operand);
    
    int error_encountered = 0;
    int greater_comparison = left_value->greater(left_value, right_value, config);
    if(greater_comparison == ESSTEE_ERROR)
    {
	error_encountered = 1;
    }
    else if(greater_comparison == ESSTEE_TRUE)
    {
	st_bool_type_true(be->temporary);
    }
    else
    {
	int equal_comparison = left_value->equals(left_value, right_value, config);

	if(equal_comparison == ESSTEE_ERROR)
	{
	    error_encountered = 1;
	}
	else if(equal_comparison == ESSTEE_TRUE)
	{
	    st_bool_type_true(be->temporary);
	}
	else
	{
	    st_bool_type_false(be->temporary);
	}
    }

    if(error_encountered)
    {
	errors->new_issue_at(
	    errors,
	    "unable to compare values by >=",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

int st_gequals_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    int inner_step_result = be_step_operands(be,
					     cursor,
					     time,
					     config,
					     errors);
    if(inner_step_result != INVOKE_RESULT_FINISHED)
    {
	return inner_step_result;
    }

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value =
	be->left_operand->return_value(be->right_operand);
    
    int error_encountered = 0;
    int greater_comparison = left_value->greater(left_value, right_value, config);
    if(greater_comparison == ESSTEE_ERROR)
    {
	error_encountered = 1;
    }
    else if(greater_comparison == ESSTEE_TRUE)
    {
	st_bool_type_true(be->temporary);
    }
    else
    {
	int equal_comparison = left_value->equals(left_value, right_value, config);

	if(equal_comparison == ESSTEE_ERROR)
	{
	    error_encountered = 1;
	}
	else if(equal_comparison == ESSTEE_TRUE)
	{
	    st_bool_type_true(be->temporary);
	}
	else
	{
	    st_bool_type_false(be->temporary);
	}
    }

    if(error_encountered)
    {
	errors->new_issue_at(
	    errors,
	    "unable to compare values by >=",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return INVOKE_RESULT_ERROR;
    }

    return INVOKE_RESULT_FINISHED;
}

/* lesser or equals */
int st_lequals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *e =				       
	CONTAINER_OF(self, struct expression_iface_t, invoke);	
    struct binary_expression_t *be =				
	CONTAINER_OF(e, struct binary_expression_t, expression);	
									
    if(be_verify_operands(be, config, errors) != ESSTEE_OK)		
    {								
	return ESSTEE_ERROR;					
    }								
									
    const struct value_iface_t *left_value =			
	be->left_operand->return_value(be->left_operand);		

    if(!left_value->equals || !left_value->lesser)
    {								
	errors->new_issue_at(					
	    errors,							
	    "value does not support the <= operation",
	    ISSUE_ERROR_CLASS,			
	    1,							
	    be->left_operand->invoke.location(&(be->left_operand->invoke)));

	return ESSTEE_ERROR;
    }								

    if(be_create_bool_temporary(be, config, errors) != ESSTEE_OK)		
    {								
	return ESSTEE_ERROR;			
    }								

    return ESSTEE_OK;
}

int st_lequals_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    int verify_result = st_gequals_expression_verify(self,
						     config,
						     errors);
    if(verify_result != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value =
	be->left_operand->return_value(be->right_operand);
    
    int error_encountered = 0;
    int lesser_comparison = left_value->lesser(left_value, right_value, config);
    if(lesser_comparison == ESSTEE_ERROR)
    {
	error_encountered = 1;
    }
    else if(lesser_comparison == ESSTEE_TRUE)
    {
	st_bool_type_true(be->temporary);
    }
    else
    {
	int equal_comparison = left_value->equals(left_value, right_value, config);

	if(equal_comparison == ESSTEE_ERROR)
	{
	    error_encountered = 1;
	}
	else if(equal_comparison == ESSTEE_TRUE)
	{
	    st_bool_type_true(be->temporary);
	}
	else
	{
	    st_bool_type_false(be->temporary);
	}
    }

    if(error_encountered)
    {
	errors->new_issue_at(
	    errors,
	    "unable to compare values by <=",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

int st_lequals_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    int inner_step_result = be_step_operands(be,
					     cursor,
					     time,
					     config,
					     errors);
    if(inner_step_result != INVOKE_RESULT_FINISHED)
    {
	return inner_step_result;
    }

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value =
	be->left_operand->return_value(be->right_operand);

    int error_encountered = 0;
    int lesser_comparison = left_value->lesser(left_value, right_value, config);
    if(lesser_comparison == ESSTEE_ERROR)
    {
	error_encountered = 1;
    }
    else if(lesser_comparison == ESSTEE_TRUE)
    {
	st_bool_type_true(be->temporary);
    }
    else
    {
	int equal_comparison = left_value->equals(left_value, right_value, config);

	if(equal_comparison == ESSTEE_ERROR)
	{
	    error_encountered = 1;
	}
	else if(equal_comparison == ESSTEE_TRUE)
	{
	    st_bool_type_true(be->temporary);
	}
	else
	{
	    st_bool_type_false(be->temporary);
	}
    }

    if(error_encountered)
    {
	errors->new_issue_at(
	    errors,
	    "unable to compare values by <=",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return INVOKE_RESULT_ERROR;
    }

    return INVOKE_RESULT_FINISHED;
}

/* not equals */
int st_nequals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_comparison_verify(self,
				"value does not support the <> operation",
				offsetof(struct value_iface_t, equals),
				config,
				errors);
}

int st_nequals_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    int verify_result = st_nequals_expression_verify(self,
						     config,
						     errors);
    if(verify_result != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);
    
    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value =
	be->right_operand->return_value(be->right_operand);

    int comparison = left_value->equals(left_value, right_value, config);
    if(comparison == ESSTEE_ERROR)
    {
	errors->new_issue_at(
	    errors,
	    "unable to compare values by <>",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return ESSTEE_ERROR;
    }
    else if(comparison == ESSTEE_FALSE)
    {
	st_bool_type_true(be->temporary);
    }
    else
    {
	st_bool_type_false(be->temporary);
    }

    return ESSTEE_OK;
}

int st_nequals_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);

    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    int operand_step_result = be_step_operands(be, cursor, time, config, errors);
    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value =
	be->right_operand->return_value(be->right_operand);

    int comparison = left_value->equals(left_value, right_value, config);
    if(comparison == ESSTEE_ERROR)
    {
	errors->new_issue_at(
	    errors,
	    "unable to compare values by <>",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return INVOKE_RESULT_ERROR;
    }
    else if(comparison == ESSTEE_FALSE)
    {
	st_bool_type_true(be->temporary);
    }
    else
    {
	st_bool_type_false(be->temporary);
    }

    return INVOKE_RESULT_FINISHED;
}

/* mod */
int st_mod_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_constant_verify(self,
			      "value does not support the modulus operation",
			      offsetof(struct value_iface_t, modulus),
			      config,
			      errors);
}

int st_mod_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_comparison_verify(self,
				"value does not support the modulus operation",
				offsetof(struct value_iface_t, modulus),
				config,
				errors);
}

int st_mod_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_step(self,
		   offsetof(struct value_iface_t, modulus),
		   cursor,
		   time,
		   config,
		   errors);
}

/* power */
int st_power_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_constant_verify(self,
			      "value does not support the to power operation",
			      offsetof(struct value_iface_t, to_power),
			      config,
			      errors);
}

int st_power_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_verify(self,
		     "value does not support the to power operation",
		     offsetof(struct value_iface_t, to_power),
		     config,
		     errors);
}

int st_power_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    return be_step(self,
		   offsetof(struct value_iface_t, to_power),
		   cursor,
		   time,
		   config,
		   errors);
}
