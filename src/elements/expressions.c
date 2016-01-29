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

#include <elements/expressions.h>
#include <elements/shared.h>
#include <elements/types.h>
#include <util/macros.h>

#include <utlist.h>


/**************************************************************************/
/* Single identifier term, either an enum, or a variable reference        */
/**************************************************************************/
const struct value_iface_t * st_single_identifier_term_var_return_value(
    struct expression_iface_t *self)
{
    struct single_identifier_term_t *sit
	= CONTAINER_OF(self, struct single_identifier_term_t, expression);

    return sit->variable->value;
}

const struct value_iface_t * st_single_identifier_term_enum_return_value(
    struct expression_iface_t *self)
{
    struct single_identifier_term_t *sit
	= CONTAINER_OF(self, struct single_identifier_term_t, expression);

    return &(sit->value);
}

const struct st_location_t * st_single_identifier_term_location(
    const struct invoke_iface_t *self)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct single_identifier_term_t *sit
	= CONTAINER_OF(e, struct single_identifier_term_t, expression);

    return sit->location;
}

void st_single_identifier_term_destroy(
    struct expression_iface_t *self)
{
    /* TODO: single identifier term destructor */
}

/**************************************************************************/
/* Qualified identifier term                                              */
/**************************************************************************/
int st_qualified_identifier_term_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct qualified_identifier_term_t *qit
	= CONTAINER_OF(expr, struct qualified_identifier_term_t, expression);

    return st_inner_resolve_qualified_identifier(qit->identifier, errors);
}

const struct value_iface_t * st_qualified_identifier_term_return_value(
    struct expression_iface_t *self)
{
    struct qualified_identifier_term_t *qit = CONTAINER_OF(
	self, struct qualified_identifier_term_t, expression);

    return qit->identifier->target;
}

const struct st_location_t * st_qualified_identifier_term_location(
    const struct invoke_iface_t *self)
{
    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct qualified_identifier_term_t *qit
	= CONTAINER_OF(expr, struct qualified_identifier_term_t, expression);

    return qit->location;
}

void st_qualified_identifier_term_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor qualified identifier term */
}

/**************************************************************************/
/* Binary expressions                                                     */
/**************************************************************************/
static int verify_operands_and_compatibility(
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

    const struct value_iface_t *left_value
	= be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);
    
    /* Check that left operand and right operand are compatible */    
    if(left_value->compatible(left_value, right_value, config) != ESSTEE_TRUE)
    {
	errors->new_issue_at(
	    errors,
	    "values are not compatible",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

int st_binary_compare_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);

    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    int verified = verify_operands_and_compatibility(
	be,
	config,
	errors);
    if(verified != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }
    
    /* Create a temporary boolean that can hold the result */
    be->temporary = st_bool_type_create_value_of(NULL, config);
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

int st_binary_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);

    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    int verified = verify_operands_and_compatibility(
	be,
	config,
	errors);
    if(verified != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    /* After verification, temporary values are available */
    const struct value_iface_t *left_value
	= be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);
    
    /* Check that temporary may be created from both left and right
     * value. Otherwise either value is not assignable, and they
     * cannot create an assignable tempory. There should be no
     * expression where a + b is ok, but not b + a simply based on the
     * types of a and b. */
    if(!left_value->create_temp_from && !right_value->create_temp_from)
    {
	errors->new_issue_at(
	    errors,
	    "values cannot be used in an expression",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return ESSTEE_ERROR;
    }
    if(!left_value->create_temp_from)
    {
	errors->new_issue_at(
	    errors,
	    "value cannot be used in an expression",
	    ISSUE_ERROR_CLASS,
	    1,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)));

	return ESSTEE_ERROR;
    }
    else if(!right_value->create_temp_from)
    {
	errors->new_issue_at(
	    errors,
	    "value cannot be used in an expression",
	    ISSUE_ERROR_CLASS,
	    1,
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

const struct st_location_t * st_binary_expression_location(
    const struct invoke_iface_t *self)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);

    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    return be->location;
}

const struct value_iface_t * st_binary_expression_return_value(
    struct expression_iface_t *self)
{
    struct binary_expression_t *be =
	CONTAINER_OF(self, struct binary_expression_t, expression);

    return be->temporary;
}

static int binary_expression_step_operands(
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
	    int left_result = be->left_operand->invoke.step(
		&(be->left_operand->invoke),
		cursor,
		time,
		config,
		errors);

	    if(left_result == INVOKE_RESULT_ERROR)
	    {
		be->invoke_state = 0;
		return INVOKE_RESULT_ERROR;
	    }
	    else if(left_result == INVOKE_RESULT_IN_PROGRESS)
	    {
		be->invoke_state = 1;
		return INVOKE_RESULT_IN_PROGRESS;
	    }
	}

    case 1:
	if(be->right_operand->invoke.step)
	{
	    int right_result = be->right_operand->invoke.step(
		&(be->right_operand->invoke),
		cursor,
		time,
		config,
		errors);

	    if(right_result == INVOKE_RESULT_ERROR)
	    {
		be->invoke_state = 0;
		return INVOKE_RESULT_ERROR;
	    }
	    else if(right_result == INVOKE_RESULT_IN_PROGRESS)
	    {
		be->invoke_state = 2;
		return INVOKE_RESULT_IN_PROGRESS;
	    }
	}

    case 2:
    default:
	/* Both operands stepped */
	be->invoke_state = 0;
	break;
    }
    
    return INVOKE_RESULT_FINISHED;
}

static int binary_expression_step_operands_assign_temporary(
    struct binary_expression_t *be,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    int operand_step_result =
	binary_expression_step_operands(be, cursor, time, config, errors);

    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }

    const struct value_iface_t *left_value
	= be->left_operand->return_value(be->left_operand);
  
    if(be->temporary->assign(be->temporary, left_value, config) != ESSTEE_OK)
    {
	errors->internal_error(errors, __FILE__, __FUNCTION__, __LINE__);
	return INVOKE_RESULT_FATAL_ERROR;
    }

    return INVOKE_RESULT_FINISHED;
}

int st_xor_expression_step(
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

    int operand_step_result =
	binary_expression_step_operands_assign_temporary(
	    be, cursor, time, config, errors);

    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }
    
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);
    
    if(be->temporary->xor(be->temporary, right_value, config) != ESSTEE_OK)
    {
	errors->new_issue_at(
	    errors,
	    "unable to xor values",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return INVOKE_RESULT_ERROR;
    }

    return INVOKE_RESULT_FINISHED;
}

int st_and_expression_step(
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

    int operand_step_result =
	binary_expression_step_operands_assign_temporary(
	    be, cursor, time, config, errors);

    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }
    
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);
    
    if(be->temporary->and(be->temporary, right_value, config) != ESSTEE_OK)
    {
	errors->new_issue_at(
	    errors,
	    "unable to and values",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return INVOKE_RESULT_ERROR;
    }

    return INVOKE_RESULT_FINISHED;
}

int st_or_expression_step(
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

    int operand_step_result =
	binary_expression_step_operands_assign_temporary(
	    be, cursor, time, config, errors);

    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }
    
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);
    
    if(be->temporary->or(be->temporary, right_value, config) != ESSTEE_OK)
    {
	errors->new_issue_at(
	    errors,
	    "unable to or values",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return INVOKE_RESULT_ERROR;
    }

    return INVOKE_RESULT_FINISHED;
}

int st_plus_expression_step(
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

    int operand_step_result =
	binary_expression_step_operands_assign_temporary(
	    be, cursor, time, config, errors);

    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }
    
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);
    
    if(be->temporary->plus(be->temporary, right_value, config) != ESSTEE_OK)
    {
	errors->new_issue_at(
	    errors,
	    "unable add values",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return INVOKE_RESULT_ERROR;
    }

    return INVOKE_RESULT_FINISHED;
}

int st_minus_expression_step(
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

    int operand_step_result =
	binary_expression_step_operands_assign_temporary(
	    be, cursor, time, config, errors);

    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }
    
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);
    
    if(be->temporary->minus(be->temporary, right_value, config) != ESSTEE_OK)
    {
	errors->new_issue_at(
	    errors,
	    "unable subtract values",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return INVOKE_RESULT_ERROR;
    }

    return INVOKE_RESULT_FINISHED;
}

int st_multiply_expression_step(
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

    int operand_step_result =
	binary_expression_step_operands_assign_temporary(
	    be, cursor, time, config, errors);

    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }
    
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);
    
    if(be->temporary->multiply(be->temporary, right_value, config) != ESSTEE_OK)
    {
	errors->new_issue_at(
	    errors,
	    "unable multiply values",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return INVOKE_RESULT_ERROR;
    }

    return INVOKE_RESULT_FINISHED;
}

int st_division_expression_step(
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

    int operand_step_result =
	binary_expression_step_operands_assign_temporary(
	    be, cursor, time, config, errors);

    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }
    
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);
    
    if(be->temporary->divide(be->temporary, right_value, config) != ESSTEE_OK)
    {
	errors->new_issue_at(
	    errors,
	    "unable divide values",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));

	return INVOKE_RESULT_ERROR;
    }

    return INVOKE_RESULT_FINISHED;
}

int st_greater_expression_step(
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

    int operand_step_result =
	binary_expression_step_operands(be, cursor, time, config, errors);

    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }

    const struct value_iface_t *left_value
	= be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);

    int comparison = left_value->greater(left_value, right_value, config);

    if(comparison == ESSTEE_ERROR)
    {
	errors->new_issue_at(
	    errors,
	    "unable determine whether one is greater than the other",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));
	
	return INVOKE_RESULT_ERROR;
    }
    else if(comparison == ESSTEE_TRUE)
    {
	st_bool_type_true(be->temporary);
    }
    else
    {
	st_bool_type_false(be->temporary);
    }

    return INVOKE_RESULT_FINISHED;
}

int st_lesser_expression_step(
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

    int operand_step_result =
	binary_expression_step_operands(be, cursor, time, config, errors);

    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }

    const struct value_iface_t *left_value
	= be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);

    int comparison = left_value->lesser(left_value, right_value, config);

    if(comparison == ESSTEE_ERROR)
    {
	errors->new_issue_at(
	    errors,
	    "unable determine whether one is smaller than the other",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));
	
	return INVOKE_RESULT_ERROR;
    }
    else if(comparison == ESSTEE_TRUE)
    {
	st_bool_type_true(be->temporary);
    }
    else
    {
	st_bool_type_false(be->temporary);
    }

    return INVOKE_RESULT_FINISHED;
}

int st_equals_expression_step(
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

    int operand_step_result =
	binary_expression_step_operands(be, cursor, time, config, errors);

    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }

    const struct value_iface_t *left_value
	= be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);

    int comparison = left_value->equals(left_value, right_value, config);

    if(comparison == ESSTEE_ERROR)
    {
	errors->new_issue_at(
	    errors,
	    "unable determine whether one equals the other",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location(&(be->left_operand->invoke)),
	    be->right_operand->invoke.location(&(be->right_operand->invoke)));
	
	return INVOKE_RESULT_ERROR;
    }
    else if(comparison == ESSTEE_TRUE)
    {
	st_bool_type_true(be->temporary);
    }
    else
    {
	st_bool_type_false(be->temporary);
    }

    return INVOKE_RESULT_FINISHED;
}
