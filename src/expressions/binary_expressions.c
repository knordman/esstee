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

#include <expressions/binary_expressions.h>
#include <elements/integers.h>
#include <util/macros.h>

/**************************************************************************/
/* Expression interface                                                   */
/**************************************************************************/
struct binary_expression_t {
    struct expression_iface_t expression;
    struct expression_iface_t *left_operand;
    struct expression_iface_t *right_operand;
    int invoke_state;
    struct st_location_t *location;
    struct value_iface_t *temporary;
};

typedef int (*binary_operation_t)(
    struct value_iface_t *,
    const struct value_iface_t *,
    const struct config_iface_t *,
    struct issues_iface_t *);

typedef int (*binary_comparison_t)(
    const struct value_iface_t *,
    const struct value_iface_t *,
    const struct config_iface_t *,
    struct issues_iface_t *);

static int be_verify_operands(
    struct binary_expression_t *be,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    /* Verify right and left expressions */
    int left_operand_verification = ESSTEE_OK;
    if(be->left_operand->invoke.verify)
    {
	left_operand_verification
	    = be->left_operand->invoke.verify(
		&(be->left_operand->invoke),
		config,
		issues);
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
		issues);
    }
    if(right_operand_verification != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int be_step_operands(
    struct binary_expression_t *be,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    switch(be->invoke_state)
    {
    case 0:
	if(be->left_operand->invoke.step)
	{
	    be->invoke_state = 1;
	    cursor->switch_current(cursor,
				   &(be->left_operand->invoke),
				   config,
				   issues);
	    return INVOKE_RESULT_IN_PROGRESS;
	}

    case 1:
	if(be->right_operand->invoke.step)
	{
	    be->invoke_state = 2;
	    cursor->switch_current(cursor,
				   &(be->right_operand->invoke),
				   config,
				   issues);
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
    struct issues_iface_t *issues)
{
    const struct value_iface_t *left_value
	= be->left_operand->return_value(be->left_operand);

    if(!left_value->create_temp_from)
    {
	issues->new_issue_at(
	    issues,
	    "value cannot be used in an expression",
	    ESSTEE_CONTEXT_ERROR,
	    1,
	    be->left_operand->invoke.location);

	return ESSTEE_ERROR;
    }

    
    /* Create a temporary value for the operation result*/
    be->temporary = left_value->create_temp_from(left_value, issues);
    if(!be->temporary)
    {
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int be_do_operation(
    struct binary_expression_t *be,
    size_t operation_offset,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);

    if(be->temporary->assign(be->temporary, left_value, config, issues) != ESSTEE_OK)
    {
	issues->internal_error(
	    issues,
	    __FILE__,
	    __FUNCTION__,
	    __LINE__);
	return ESSTEE_ERROR;
    }

    const struct value_iface_t *right_value =
	be->right_operand->return_value(be->right_operand);

    binary_operation_t *operation
	= (binary_operation_t *)(((char *)be->temporary) + operation_offset);
    
    if((*operation)(be->temporary, right_value, config, issues) != ESSTEE_OK)
    {
	issues->new_issue_at(
	    issues,
	    "expression evaluation failed",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location,
	    be->right_operand->invoke.location);

	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int be_verify(
    struct invoke_iface_t *self,
    const char *not_supported_message,
    size_t operation_offset,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    if(be_verify_operands(be, config, issues) != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);

    binary_operation_t *operation
	= (binary_operation_t *)(((char *)left_value) + operation_offset);

    if(!operation)
    {
	issues->new_issue_at(
	    issues,
	    not_supported_message,
	    ISSUE_ERROR_CLASS,
	    1,
	    be->left_operand->invoke.location);

	return ESSTEE_ERROR;
    }

    const struct value_iface_t *right_value =
	be->right_operand->return_value(be->right_operand);
    
    issues->begin_group(issues);
    int left_operates_with_right
	= left_value->operates_with(left_value, right_value, config, issues);

    if(left_operates_with_right != ESSTEE_TRUE)
    {
	issues->new_issue(
	    issues,
	    "left value does not support the operation using the right value",
	    ESSTEE_CONTEXT_ERROR);

	issues->set_group_location(
	    issues,
	    2,
	    be->left_operand->invoke.location,
	    be->right_operand->invoke.location);
    }
    issues->end_group(issues);

    if(left_operates_with_right != ESSTEE_TRUE)
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
    struct issues_iface_t *issues)
{
    int verify_result = be_verify(self,
				  not_supported_message,
				  operation_offset,
				  config,
				  issues);
    if(verify_result != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);
    
    return be_do_operation(be, operation_offset, config, issues);
}

static int be_step(
    struct invoke_iface_t *self,
    size_t operation_offset,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    int inner_step_result = be_step_operands(be,
					     cursor,
					     time,
					     config,
					     issues);
    if(inner_step_result != INVOKE_RESULT_FINISHED)
    {
	return inner_step_result;
    }

    if(be_do_operation(be, operation_offset, config, issues) != ESSTEE_OK)
    {
	return INVOKE_RESULT_ERROR;
    }
    
    return INVOKE_RESULT_FINISHED;
}

/* Binary expressions producing true/false (comparisons) */
static int be_create_bool_temporary(
    struct binary_expression_t *be,
    struct issues_iface_t *issues)
{
    /* Create a temporary boolean that can hold the result */
    be->temporary = st_new_bool_value(ESSTEE_FALSE,
				      TEMPORARY_VALUE,
				      NULL, /* TODO: fix dependency on config */
				      issues);
    if(!be->temporary)
    {
	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int be_do_comparison(
    struct binary_expression_t *be,
    size_t operation_offset,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value =
	be->right_operand->return_value(be->right_operand);

    binary_comparison_t *comparison
	= (binary_comparison_t *)(((char *)left_value) + operation_offset);

    int result = (*comparison)(left_value, right_value, config, issues);

    if(result == ESSTEE_ERROR)
    {
	return ESSTEE_ERROR;
    }
    else if(result == ESSTEE_TRUE)
    {
	st_set_bool_value_state(be->temporary, ESSTEE_TRUE);
    }
    else
    {
	st_set_bool_value_state(be->temporary, ESSTEE_FALSE);
    }

    return ESSTEE_OK;
}

static int be_comparison_verify(
    struct invoke_iface_t *self,
    const char *not_supported_message,
    size_t operation_offset,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    if(be_verify_operands(be, config, issues) != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);

    binary_comparison_t *comparison
	= (binary_comparison_t *)(((char *)left_value) + operation_offset);

    if(!comparison)
    {
	issues->new_issue_at(
	    issues,
	    not_supported_message,
	    ISSUE_ERROR_CLASS,
	    1,
	    be->left_operand->invoke.location);

	return ESSTEE_ERROR;
    }

    const struct value_iface_t *right_value
	= be->right_operand->return_value(be->right_operand);
    
    issues->begin_group(issues);
    int left_comparable_to_right
	= left_value->comparable_to(left_value, right_value, config, issues);

    if(left_comparable_to_right != ESSTEE_TRUE)
    {
	issues->new_issue(
	    issues,
	    "left value is not comparable to the right value",
	    ESSTEE_CONTEXT_ERROR);

	issues->set_group_location(
	    issues,
	    2,
	    be->left_operand->invoke.location,
	    be->right_operand->invoke.location);
    }
    issues->end_group(issues);

    if(left_comparable_to_right != ESSTEE_TRUE)
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
    struct issues_iface_t *issues)
{
    int verify_result = be_comparison_verify(self,
					     not_supported_message,
					     operation_offset,
					     config,
					     issues);
    if(verify_result != ESSTEE_OK)
    {
	return ESSTEE_ERROR;
    }

    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);
    
    return be_do_comparison(be, operation_offset, config, issues);
}

static int be_comparison_step(
    struct invoke_iface_t *self,
    size_t operation_offset,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    int inner_step_result = be_step_operands(be,
					     cursor,
					     time,
					     config,
					     issues);
    if(inner_step_result != INVOKE_RESULT_FINISHED)
    {
	return inner_step_result;
    }

    if(be_do_comparison(be, operation_offset, config, issues) != ESSTEE_OK)
    {
	return INVOKE_RESULT_ERROR;
    }
    
    return INVOKE_RESULT_FINISHED;
}

static int binary_expression_reset(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);

    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    be->invoke_state = 0;

    if(be->left_operand->invoke.reset)
    {
	int left_reset = be->left_operand->invoke.reset(
	    &(be->left_operand->invoke), config, issues);

	if(left_reset != ESSTEE_OK)
	{
	    return left_reset;
	}
    }

    if(be->right_operand->invoke.reset)
    {
	int right_reset = be->right_operand->invoke.reset(
	    &(be->right_operand->invoke), config, issues);

	if(right_reset != ESSTEE_OK)
	{
	    return right_reset;
	}
    }

    return ESSTEE_OK;
}

static int binary_expression_allocate(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);

    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    if(be->left_operand->invoke.allocate)
    {
	int allocate_result = be->left_operand->invoke.allocate(
	    &(be->left_operand->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    if(be->right_operand->invoke.allocate)
    {
	int allocate_result = be->right_operand->invoke.allocate(
	    &(be->right_operand->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    return be_create_cloned_temporary(be, issues);
}

static int binary_expression_allocate_bool(
    struct invoke_iface_t *self,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);

    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    if(be->left_operand->invoke.allocate)
    {
	int allocate_result = be->left_operand->invoke.allocate(
	    &(be->left_operand->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    if(be->right_operand->invoke.allocate)
    {
	int allocate_result = be->right_operand->invoke.allocate(
	    &(be->right_operand->invoke),
	    issues);

	if(allocate_result != ESSTEE_OK)
	{
	    return allocate_result;
	}
    }

    return be_create_bool_temporary(be, issues);
}

static const struct value_iface_t * binary_expression_return_value(
    struct expression_iface_t *self)
{
    struct binary_expression_t *be =
	CONTAINER_OF(self, struct binary_expression_t, expression);

    return be->temporary;
}

static void binary_expression_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

static void binary_expression_clone_destroy(
    struct expression_iface_t *self)
{
    /* TODO: destructor */
}

static struct expression_iface_t * binary_expression_clone(
    struct expression_iface_t *self,
    struct issues_iface_t *issues)
{
    struct binary_expression_t *be =
	CONTAINER_OF(self, struct binary_expression_t, expression);

    struct binary_expression_t *copy = NULL;
    struct expression_iface_t *left_copy = NULL;
    struct expression_iface_t *right_copy = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	copy,
	struct binary_expression_t,
	issues,
	error_free_resources);

    memcpy(copy, be, sizeof(struct binary_expression_t));

    if(be->left_operand->clone)
    {
	left_copy = be->left_operand->clone(be->left_operand, issues);
	if(!left_copy)
	{
	    goto error_free_resources;
	}

	copy->left_operand = left_copy;
    }

    if(be->right_operand->clone)
    {
	right_copy = be->right_operand->clone(be->right_operand, issues);
	if(!right_copy)
	{
	    goto error_free_resources;
	}

	copy->right_operand = right_copy;
    }

    copy->expression.destroy = binary_expression_clone_destroy;
    
    return &(copy->expression);
    
error_free_resources:
    free(copy);
    free(left_copy);
    free(right_copy);
    return NULL;
}

/* xor */
static int xor_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_constant_verify(self,
		     "value does not support the xor operation",
		     offsetof(struct value_iface_t, xor),
		     config,
			      issues);    
}

static int xor_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_verify(self,
		     "value does not support the xor operation",
		     offsetof(struct value_iface_t, xor),
		     config,
		     issues);
}

static int xor_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{    
    return be_step(self,
		   offsetof(struct value_iface_t, xor),
		   cursor,
		   time,
		   config,
		   issues);
}

/* and */
static int and_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_constant_verify(self,
			      "value does not support the and operation",
			      offsetof(struct value_iface_t, and),
			      config,
			      issues);    
}

static int and_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_verify(self,
		     "value does not support the and operation",
		     offsetof(struct value_iface_t, and),
		     config,
		     issues);    
}

static int and_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_step(self,
		   offsetof(struct value_iface_t, and),
		   cursor,
		   time,
		   config,
		   issues);
}

static int or_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_constant_verify(self,
			      "value does not support the or operation",
			      offsetof(struct value_iface_t, or),
			      config,
			      issues);    
}

static int or_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_verify(self,
		     "value does not support the or operation",
		     offsetof(struct value_iface_t, or),
		     config,
		     issues);
}

static int or_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_step(self,
		   offsetof(struct value_iface_t, or),
		   cursor,
		   time,
		   config,
		   issues);
}

/* plus */
static int plus_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_constant_verify(self,
			      "value does not support the + operation",
			      offsetof(struct value_iface_t, plus),
			      config,
			      issues);    
}

static int plus_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_verify(self,
		     "value does not support the + operation",
		     offsetof(struct value_iface_t, plus),
		     config,
		     issues);
}

static int plus_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_step(self,
		   offsetof(struct value_iface_t, plus),
		   cursor,
		   time,
		   config,
		   issues);
}

/* minus */
static int minus_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_constant_verify(self,
			      "value does not support the - operation",
			      offsetof(struct value_iface_t, minus),
			      config,
			      issues);
}

static int minus_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_verify(self,
		     "value does not support the - operation",
		     offsetof(struct value_iface_t, minus),
		     config,
		     issues);
}

static int minus_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_step(self,
		   offsetof(struct value_iface_t, minus),
		   cursor,
		   time,
		   config,
		   issues);
}

/* multiply */
static int multiply_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_constant_verify(self,
			      "value does not support the * operation",
			      offsetof(struct value_iface_t, multiply),
			      config,
			      issues);
}

static int multiply_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_verify(self,
		     "value does not support the * operation",
		     offsetof(struct value_iface_t, multiply),
		     config,
		     issues);
}

static int multiply_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_step(self,
		   offsetof(struct value_iface_t, multiply),
		   cursor,
		   time,
		   config,
		   issues);
}

/* divide */
static int division_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_constant_verify(self,
			      "value does not support the / operation",
			      offsetof(struct value_iface_t, divide),
			      config,
			      issues);
}

static int division_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_verify(self,
		     "value does not support the / operation",
		     offsetof(struct value_iface_t, divide),
		     config,
		     issues);
}

static int division_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_step(self,
		   offsetof(struct value_iface_t, divide),
		   cursor,
		   time,
		   config,
		   issues);
}

/* greater */
static int greater_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_comparison_constant_verify(self,
					 "value does not support the > operation",
					 offsetof(struct value_iface_t, greater),
					 config,
					 issues);
}

static int greater_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_comparison_verify(self,
				"value does not support the > operation",
				offsetof(struct value_iface_t, greater),
				config,
				issues);
}

static int greater_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_comparison_step(self,
			      offsetof(struct value_iface_t, greater),
			      cursor,
			      time,
			      config,
			      issues);
}

/* lesser */
static int lesser_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_comparison_constant_verify(self,
					 "value does not support the > operation",
					 offsetof(struct value_iface_t, lesser),
					 config,
					 issues);
}


static int lesser_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_comparison_verify(self,
				"value does not support the > operation",
				offsetof(struct value_iface_t, lesser),
				config,
				issues);
}

static int lesser_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_comparison_step(self,
			      offsetof(struct value_iface_t, lesser),
			      cursor,
			      time,
			      config,
			      issues);
}

/* equals */
static int equals_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_comparison_constant_verify(self,
					 "value does not support the = operation",
					 offsetof(struct value_iface_t, equals),
					 config,
					 issues);

}

static int equals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_comparison_verify(self,
				"value does not support the = operation",
				offsetof(struct value_iface_t, equals),
				config,
				issues);
}

static int equals_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_comparison_step(self,
			      offsetof(struct value_iface_t, equals),
			      cursor,
			      time,
			      config,
			      issues);
}

/* greater or equals */
static int gequals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *e =				       
	CONTAINER_OF(self, struct expression_iface_t, invoke);	
    struct binary_expression_t *be =				
	CONTAINER_OF(e, struct binary_expression_t, expression);	
									
    if(be_verify_operands(be, config, issues) != ESSTEE_OK)		
    {								
	return ESSTEE_ERROR;					
    }								
									
    const struct value_iface_t *left_value =			
	be->left_operand->return_value(be->left_operand);		

    if(!left_value->equals || !left_value->greater)
    {								
	issues->new_issue_at(					
	    issues,							
	    "value does not support the >= operation",
	    ISSUE_ERROR_CLASS,			
	    1,							
	    be->left_operand->invoke.location);

	return ESSTEE_ERROR;
    }								

    return ESSTEE_OK;
}

static int gequals_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    int verify_result = gequals_expression_verify(self,
						  config,
						  issues);
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
    int greater_comparison = left_value->greater(left_value,
						 right_value,
						 config,
						 issues);
    if(greater_comparison == ESSTEE_ERROR)
    {
	error_encountered = 1;
    }
    else if(greater_comparison == ESSTEE_TRUE)
    {
	st_set_bool_value_state(be->temporary, ESSTEE_TRUE);
    }
    else
    {
	int equal_comparison = left_value->equals(left_value,
						  right_value,
						  config,
						  issues);

	if(equal_comparison == ESSTEE_ERROR)
	{
	    error_encountered = 1;
	}
	else if(equal_comparison == ESSTEE_TRUE)
	{
	    st_set_bool_value_state(be->temporary, ESSTEE_TRUE);
	}
	else
	{
	    st_set_bool_value_state(be->temporary, ESSTEE_FALSE);
	}
    }

    if(error_encountered)
    {
	issues->new_issue_at(
	    issues,
	    "unable to compare values by >=",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location,
	    be->right_operand->invoke.location);

	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int gequals_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    int inner_step_result = be_step_operands(be,
					     cursor,
					     time,
					     config,
					     issues);
    if(inner_step_result != INVOKE_RESULT_FINISHED)
    {
	return inner_step_result;
    }

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value =
	be->left_operand->return_value(be->right_operand);
    
    int error_encountered = 0;
    int greater_comparison = left_value->greater(left_value,
						 right_value,
						 config,
						 issues);
    if(greater_comparison == ESSTEE_ERROR)
    {
	error_encountered = 1;
    }
    else if(greater_comparison == ESSTEE_TRUE)
    {
	st_set_bool_value_state(be->temporary, ESSTEE_TRUE);
    }
    else
    {
	int equal_comparison = left_value->equals(left_value,
						  right_value,
						  config,
						  issues);

	if(equal_comparison == ESSTEE_ERROR)
	{
	    error_encountered = 1;
	}
	else if(equal_comparison == ESSTEE_TRUE)
	{
	    st_set_bool_value_state(be->temporary, ESSTEE_TRUE);
	}
	else
	{
	    st_set_bool_value_state(be->temporary, ESSTEE_FALSE);
	}
    }

    if(error_encountered)
    {
	issues->new_issue_at(
	    issues,
	    "unable to compare values by >=",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location,
	    be->right_operand->invoke.location);

	return INVOKE_RESULT_ERROR;
    }

    return INVOKE_RESULT_FINISHED;
}

/* lesser or equals */
static int lequals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *e =				       
	CONTAINER_OF(self, struct expression_iface_t, invoke);	
    struct binary_expression_t *be =				
	CONTAINER_OF(e, struct binary_expression_t, expression);	
									
    if(be_verify_operands(be, config, issues) != ESSTEE_OK)		
    {								
	return ESSTEE_ERROR;					
    }								
									
    const struct value_iface_t *left_value =			
	be->left_operand->return_value(be->left_operand);		

    if(!left_value->equals || !left_value->lesser)
    {								
	issues->new_issue_at(					
	    issues,							
	    "value does not support the <= operation",
	    ISSUE_ERROR_CLASS,			
	    1,							
	    be->left_operand->invoke.location);

	return ESSTEE_ERROR;
    }								

    return ESSTEE_OK;
}

static int lequals_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    int verify_result = lequals_expression_verify(self,
						  config,
						  issues);
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
    int lesser_comparison = left_value->lesser(left_value,
					       right_value,
					       config,
					       issues);
    if(lesser_comparison == ESSTEE_ERROR)
    {
	error_encountered = 1;
    }
    else if(lesser_comparison == ESSTEE_TRUE)
    {
	st_set_bool_value_state(be->temporary, ESSTEE_TRUE);
    }
    else
    {
	int equal_comparison = left_value->equals(left_value,
						  right_value,
						  config,
						  issues);

	if(equal_comparison == ESSTEE_ERROR)
	{
	    error_encountered = 1;
	}
	else if(equal_comparison == ESSTEE_TRUE)
	{
	    st_set_bool_value_state(be->temporary, ESSTEE_TRUE);
	}
	else
	{
	    st_set_bool_value_state(be->temporary, ESSTEE_FALSE);
	}
    }

    if(error_encountered)
    {
	issues->new_issue_at(
	    issues,
	    "unable to compare values by <=",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location,
	    be->right_operand->invoke.location);

	return ESSTEE_ERROR;
    }

    return ESSTEE_OK;
}

static int lequals_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);
    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    int inner_step_result = be_step_operands(be,
					     cursor,
					     time,
					     config,
					     issues);
    if(inner_step_result != INVOKE_RESULT_FINISHED)
    {
	return inner_step_result;
    }

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value =
	be->left_operand->return_value(be->right_operand);

    int error_encountered = 0;
    int lesser_comparison = left_value->lesser(left_value,
					       right_value,
					       config,
					       issues);
    if(lesser_comparison == ESSTEE_ERROR)
    {
	error_encountered = 1;
    }
    else if(lesser_comparison == ESSTEE_TRUE)
    {
	st_set_bool_value_state(be->temporary, ESSTEE_TRUE);
    }
    else
    {
	int equal_comparison = left_value->equals(left_value,
						  right_value,
						  config,
						  issues);

	if(equal_comparison == ESSTEE_ERROR)
	{
	    error_encountered = 1;
	}
	else if(equal_comparison == ESSTEE_TRUE)
	{
	    st_set_bool_value_state(be->temporary, ESSTEE_TRUE);
	}
	else
	{
	    st_set_bool_value_state(be->temporary, ESSTEE_FALSE);
	}
    }

    if(error_encountered)
    {
	issues->new_issue_at(
	    issues,
	    "unable to compare values by <=",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location,
	    be->right_operand->invoke.location);

	return INVOKE_RESULT_ERROR;
    }

    return INVOKE_RESULT_FINISHED;
}

/* not equals */
static int nequals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_comparison_verify(self,
				"value does not support the <> operation",
				offsetof(struct value_iface_t, equals),
				config,
				issues);
}

static int nequals_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    int verify_result = nequals_expression_verify(self,
						  config,
						  issues);
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

    int comparison = left_value->equals(left_value, right_value, config, issues);
    if(comparison == ESSTEE_ERROR)
    {
	issues->new_issue_at(
	    issues,
	    "unable to compare values by <>",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location,
	    be->right_operand->invoke.location);

	return ESSTEE_ERROR;
    }
    else if(comparison == ESSTEE_FALSE)
    {
	st_set_bool_value_state(be->temporary, ESSTEE_TRUE);
    }
    else
    {
	st_set_bool_value_state(be->temporary, ESSTEE_FALSE);
    }

    return ESSTEE_OK;
}

static int nequals_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct expression_iface_t *e =
	CONTAINER_OF(self, struct expression_iface_t, invoke);

    struct binary_expression_t *be =
	CONTAINER_OF(e, struct binary_expression_t, expression);

    int operand_step_result = be_step_operands(be, cursor, time, config, issues);
    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value =
	be->right_operand->return_value(be->right_operand);

    int comparison = left_value->equals(left_value, right_value, config, issues);
    if(comparison == ESSTEE_ERROR)
    {
	issues->new_issue_at(
	    issues,
	    "unable to compare values by <>",
	    ISSUE_ERROR_CLASS,
	    2,
	    be->left_operand->invoke.location,
	    be->right_operand->invoke.location);

	return INVOKE_RESULT_ERROR;
    }
    else if(comparison == ESSTEE_FALSE)
    {
	st_set_bool_value_state(be->temporary, ESSTEE_TRUE);
    }
    else
    {
	st_set_bool_value_state(be->temporary, ESSTEE_FALSE);
    }

    return INVOKE_RESULT_FINISHED;
}

/* mod */
static int mod_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_constant_verify(self,
			      "value does not support the modulus operation",
			      offsetof(struct value_iface_t, modulus),
			      config,
			      issues);
}

static int mod_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_comparison_verify(self,
				"value does not support the modulus operation",
				offsetof(struct value_iface_t, modulus),
				config,
				issues);
}

static int mod_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_step(self,
		   offsetof(struct value_iface_t, modulus),
		   cursor,
		   time,
		   config,
		   issues);
}

/* power */
static int power_expression_constant_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_constant_verify(self,
			      "value does not support the to power operation",
			      offsetof(struct value_iface_t, to_power),
			      config,
			      issues);
}

static int power_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_verify(self,
		     "value does not support the to power operation",
		     offsetof(struct value_iface_t, to_power),
		     config,
		     issues);
}

static int power_expression_step(
    struct invoke_iface_t *self,
    struct cursor_iface_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return be_step(self,
		   offsetof(struct value_iface_t, to_power),
		   cursor,
		   time,
		   config,
		   issues);
}

static struct expression_iface_t * new_binary_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    int (*verify_constant_function)(
	struct invoke_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *errors),
    int (*verify_function)(
	struct invoke_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *errors),
    const struct st_location_t *location,
    int (*step_function)(
	struct invoke_iface_t *,
	struct cursor_iface_t *,
	const struct systime_iface_t *,
	const struct config_iface_t *,
	struct issues_iface_t *),
    int (*allocate_function)(
	struct invoke_iface_t *,
	struct issues_iface_t *),
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    struct binary_expression_t *be = NULL;
    struct st_location_t *be_location = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	be,
	struct binary_expression_t,
	issues,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
    	be_location,
    	location,
	issues,
    	error_free_resources);

    be->location = be_location;
    be->left_operand = left_operand;
    be->right_operand = right_operand;
    be->temporary = NULL;

    memset(&(be->expression), 0, sizeof(struct expression_iface_t));
    
    if((left_operand->invoke.step || left_operand->clone) || (right_operand->invoke.step || right_operand->clone))
    {
	be->expression.invoke.verify = verify_function;
	be->expression.invoke.step = step_function;
	be->expression.invoke.reset = binary_expression_reset;
    }
    else
    {
	be->expression.invoke.verify = verify_constant_function;
    }

    if(left_operand->clone || right_operand->clone)
    {
	be->expression.clone = binary_expression_clone;
    }
    else
    {
	be->expression.clone = NULL;
    }
    
    be->expression.invoke.location = be->location;
    be->expression.invoke.allocate = allocate_function;
    be->expression.return_value = binary_expression_return_value;
    be->expression.destroy = binary_expression_destroy;

    return &(be->expression);
    
error_free_resources:
    /* TODO: determine what to destroy */
    free(be);
    return NULL;
}

/**************************************************************************/
/* Public interface                                                       */
/**************************************************************************/
struct expression_iface_t * st_create_xor_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	xor_expression_constant_verify,
	xor_expression_verify,
	location,
	xor_expression_step,
	binary_expression_allocate,
	config,
	issues);
}

struct expression_iface_t * st_create_and_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	and_expression_constant_verify,
	and_expression_verify,       
	location,
	and_expression_step,
	binary_expression_allocate,
	config,
	issues);
}

struct expression_iface_t * st_create_or_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	or_expression_constant_verify,
	or_expression_verify,
	location,
	or_expression_step,
	binary_expression_allocate,
	config,
	issues);
}

struct expression_iface_t * st_create_greater_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	greater_expression_constant_verify,
	greater_expression_verify,
	location,
	greater_expression_step,
	binary_expression_allocate_bool,
	config,
	issues);
}

struct expression_iface_t * st_create_lesser_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	lesser_expression_constant_verify,
	lesser_expression_verify,	
	location,
	lesser_expression_step,
	binary_expression_allocate_bool,
	config,
	issues);
}

struct expression_iface_t * st_create_equals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	equals_expression_constant_verify,
	equals_expression_verify,
	location,
	equals_expression_step,
	binary_expression_allocate_bool,
	config,
	issues);
}

struct expression_iface_t * st_create_gequals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	gequals_expression_constant_verify,
	gequals_expression_verify,
	location,
	gequals_expression_step,
	binary_expression_allocate_bool,
	config,
	issues);
}

struct expression_iface_t * st_create_lequals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	lequals_expression_constant_verify,
	lequals_expression_verify,
	location,
	lequals_expression_step,	
	binary_expression_allocate_bool,
	config,
	issues);
}

struct expression_iface_t * st_create_nequals_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	nequals_expression_constant_verify,
	nequals_expression_verify,	
	location,
	nequals_expression_step,
	binary_expression_allocate_bool,
	config,
	issues);
}

struct expression_iface_t * st_create_plus_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	plus_expression_constant_verify,
	plus_expression_verify,	
	location,
	plus_expression_step,
	binary_expression_allocate,
	config,
	issues);
}

struct expression_iface_t * st_create_minus_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	minus_expression_constant_verify,
	minus_expression_verify,	
	location,
	minus_expression_step,
	binary_expression_allocate,
	config,
	issues);
}

struct expression_iface_t * st_create_multiply_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	multiply_expression_constant_verify,
	multiply_expression_verify,
	location,
	multiply_expression_step,
	binary_expression_allocate,
	config,
	issues);
}

struct expression_iface_t * st_create_division_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	division_expression_constant_verify,
	division_expression_verify,
	location,
	division_expression_step,
	binary_expression_allocate,
	config,
	issues);
}

struct expression_iface_t * st_create_mod_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	mod_expression_constant_verify,
	mod_expression_verify,
	location,
	mod_expression_step,
	binary_expression_allocate,
	config,
	issues);
}

struct expression_iface_t * st_create_to_power_expression(
    struct expression_iface_t *left_operand,
    struct expression_iface_t *right_operand,
    const struct st_location_t *location,
    const struct config_iface_t *config,
    struct issues_iface_t *issues)
{
    return new_binary_expression(
	left_operand,
	right_operand,
	power_expression_constant_verify,
	power_expression_verify,	
	location,
	power_expression_step,
	binary_expression_allocate,
	config,
	issues);
}
