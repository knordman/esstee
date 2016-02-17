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

int st_inline_enum_value_display(
    const struct value_iface_t *self,
    char *buffer,
    size_t buffer_size,
    const struct config_iface_t *config)
{
    struct single_identifier_term_t *sit
	= CONTAINER_OF(self, struct single_identifier_term_t, value);

    int written_bytes = snprintf(buffer,
				 buffer_size,
				 "%s",
				 sit->inline_enum.identifier);
    if(written_bytes == 0)
    {
	return ESSTEE_FALSE;
    }
    else if(written_bytes < 0)
    {
	return ESSTEE_ERROR;
    }

    return written_bytes;
}

int st_inline_enum_value_comparable_to(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    if(!other_value->enumeration)
    {
	return ESSTEE_FALSE;
    }

    return ESSTEE_TRUE;
}

int st_inline_enum_value_equals(
    const struct value_iface_t *self,
    const struct value_iface_t *other_value,
    const struct config_iface_t *config)
{
    struct single_identifier_term_t *sit
	= CONTAINER_OF(self, struct single_identifier_term_t, value);

    const struct enum_item_t *other_value_enum = other_value->enumeration(other_value, config);
    
    if(strcmp(sit->inline_enum.identifier, other_value_enum->identifier) == 0)
    {
	return ESSTEE_TRUE;
    }

    return ESSTEE_FALSE;
}

const struct enum_item_t * st_inline_enum_value_enumeration(
    const struct value_iface_t *self,
    const struct config_iface_t *conf)
{
    struct single_identifier_term_t *sit
	= CONTAINER_OF(self, struct single_identifier_term_t, value);

    return &(sit->inline_enum);
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

    return st_inner_resolve_qualified_identifier(qit->identifier, errors, config);
}

int st_qualified_identifier_term_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    /* Step or verified used, if step, means array indices'
     * expressions need to be stepped */

    struct expression_iface_t *expr
	= CONTAINER_OF(self, struct expression_iface_t, invoke);
    
    struct qualified_identifier_term_t *qit
	= CONTAINER_OF(expr, struct qualified_identifier_term_t, expression);
    
    /* qit->identifier->array_index */

    /* Skip already stepped indices */
    struct array_index_t *itr = NULL;
    int skipped = 0;
    DL_FOREACH(qit->identifier->array_index, itr)
    {
	if(skipped >= qit->invoke_state)
	{
	    break;
	}

	skipped++;
    }

    int index_step_result = INVOKE_RESULT_FINISHED;
    if(itr->index_expression->invoke.step)
    {
	struct invoke_iface_t *invokee = &(itr->index_expression->invoke);
	
	index_step_result = invokee->step(invokee,
					  cursor,
					  time,
					  config,
					  errors);

	    if(index_step_result == INVOKE_RESULT_ERROR)
	    {
		qit->invoke_state = 0;
		return INVOKE_RESULT_ERROR;
	    }
	    else if(index_step_result == INVOKE_RESULT_IN_PROGRESS)
	    {
		return INVOKE_RESULT_IN_PROGRESS;
	    }
    }

    if(itr->next)
    {
	qit->invoke_state++;
	return INVOKE_RESULT_IN_PROGRESS;
    }

    /* All indices stepped */
    qit->invoke_state = 0;

    int resolve_result = st_inner_resolve_qualified_identifier(qit->identifier, errors, config);

    if(resolve_result != ESSTEE_OK)
    {
	return INVOKE_RESULT_ERROR;
    }

    return INVOKE_RESULT_FINISHED;
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

#define BINARY_EXPRESSION_VERIFY(operation, not_supported_msg, create_temporary) \
    do {								\
	struct expression_iface_t *e =					\
	    CONTAINER_OF(self, struct expression_iface_t, invoke);	\
	struct binary_expression_t *be =				\
	    CONTAINER_OF(e, struct binary_expression_t, expression);	\
									\
	if(be_verify_operands(be, config, errors) != ESSTEE_OK)		\
	{								\
	    return ESSTEE_ERROR;					\
	}								\
									\
	const struct value_iface_t *left_value =			\
	    be->left_operand->return_value(be->left_operand);		\
	if(!left_value->operation)					\
	{								\
	    errors->new_issue_at(					\
		errors,							\
		not_supported_msg,					\
		ISSUE_ERROR_CLASS,					\
		1,							\
		be->left_operand->invoke.location(&(be->left_operand->invoke))); \
	}								\
									\
	if(create_temporary(be, config, errors) != ESSTEE_OK)		\
	{								\
            return ESSTEE_ERROR;					\
	}								\
    } while(0)

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

#define BINARY_EXPRESSION_STEP(operation, fail_msg)			\
    do {								\
	struct expression_iface_t *e =					\
	    CONTAINER_OF(self, struct expression_iface_t, invoke);	\
									\
	struct binary_expression_t *be =				\
	    CONTAINER_OF(e, struct binary_expression_t, expression);	\
									\
	int operand_step_result = be_step_operands(be, cursor, time, config, errors); \
	if(operand_step_result != INVOKE_RESULT_FINISHED)		\
	{								\
	    return operand_step_result;					\
	}								\
									\
	const struct value_iface_t *left_value =			\
	    be->left_operand->return_value(be->left_operand);		\
	const struct value_iface_t *right_value =			\
	    be->right_operand->return_value(be->right_operand);		\
									\
	if(be->temporary->assign(be->temporary, left_value, config) != ESSTEE_OK) \
	{								\
	    errors->internal_error(errors, __FILE__, __FUNCTION__, __LINE__); \
	    return INVOKE_RESULT_FATAL_ERROR;				\
	}								\
	if(be->temporary->operation(be->temporary, right_value, config) != ESSTEE_OK) \
	{								\
	    errors->new_issue_at(					\
		errors,							\
		fail_msg,						\
		ISSUE_ERROR_CLASS,					\
		2,							\
		be->left_operand->invoke.location(&(be->left_operand->invoke)), \
		be->right_operand->invoke.location(&(be->right_operand->invoke))); \
	    return INVOKE_RESULT_ERROR;					\
	}								\
    } while(0)

#define BINARY_COMPARE_EXPRESSION_STEP(operation, fail_msg)			\
    do {								\
	struct expression_iface_t *e =					\
	    CONTAINER_OF(self, struct expression_iface_t, invoke);	\
									\
	struct binary_expression_t *be =				\
	    CONTAINER_OF(e, struct binary_expression_t, expression);	\
									\
	int operand_step_result = be_step_operands(be, cursor, time, config, errors); \
	if(operand_step_result != INVOKE_RESULT_FINISHED)		\
	{								\
	    return operand_step_result;					\
	}								\
									\
	const struct value_iface_t *left_value =			\
	    be->left_operand->return_value(be->left_operand);		\
	const struct value_iface_t *right_value =			\
	    be->right_operand->return_value(be->right_operand);		\
									\
	int comparison = left_value->operation(left_value, right_value, config); \
	if(comparison == ESSTEE_ERROR)					\
	{								\
	    errors->new_issue_at(					\
		errors,							\
		fail_msg,						\
		ISSUE_ERROR_CLASS,					\
		2,							\
		be->left_operand->invoke.location(&(be->left_operand->invoke)), \
		be->right_operand->invoke.location(&(be->right_operand->invoke))); \
	    return INVOKE_RESULT_ERROR;					\
	}								\
	else if(comparison == ESSTEE_TRUE)				\
	{								\
	    st_bool_type_true(be->temporary);				\
	}								\
	else								\
	{								\
	    st_bool_type_false(be->temporary);				\
	}								\
    } while(0)

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

int st_xor_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_VERIFY(
	xor,
	"value does not support the xor operation",
	be_create_cloned_temporary);

    return ESSTEE_OK;
}

int st_xor_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{    
    BINARY_EXPRESSION_STEP(
	xor,
	"unable to xor values");

    return INVOKE_RESULT_FINISHED;
}

int st_and_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_VERIFY(
        and,
	"value does not support the and operation",
	be_create_cloned_temporary);

    return ESSTEE_OK;
}

int st_and_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_STEP(
	and,
	"unable to and values");

    return INVOKE_RESULT_FINISHED;
}

int st_or_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_VERIFY(
	or,
	"value does not support the or operation",
	be_create_cloned_temporary);

    return ESSTEE_OK;
}

int st_or_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_STEP(
	or,
	"unable to or values");

    return INVOKE_RESULT_FINISHED;
}

int st_plus_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_VERIFY(
	plus,
	"value does not support the + operation",
	be_create_cloned_temporary);

    return ESSTEE_OK;
}

int st_plus_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_STEP(
        plus,
	"unable to add values");

    return INVOKE_RESULT_FINISHED;
}

int st_minus_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_VERIFY(
	minus,
	"value does not support the - operation",
	be_create_cloned_temporary);

    return ESSTEE_OK;
}

int st_minus_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_STEP(
	minus,
	"unable to subtract values");

    return INVOKE_RESULT_FINISHED;
}

int st_multiply_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_VERIFY(
	multiply,
	"value does not support the * operation",
	be_create_cloned_temporary);

    return ESSTEE_OK;
}

int st_multiply_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_STEP(
	multiply,
	"unable to multiply values");

    return INVOKE_RESULT_FINISHED;
}

int st_division_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_VERIFY(
	divide,
	"value does not support the / operation",
	be_create_cloned_temporary);

    return ESSTEE_OK;
}

int st_division_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_STEP(
	divide,
	"unable to divide values");

    return INVOKE_RESULT_FINISHED;
}

int st_greater_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_VERIFY(
        greater,
	"value cannot be compared by >",
	be_create_bool_temporary);

    return ESSTEE_OK;
}

int st_greater_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_COMPARE_EXPRESSION_STEP(
	greater,
	"unable to determine whether one value is larger than the other");
    
    return INVOKE_RESULT_FINISHED;
}

int st_lesser_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_VERIFY(
        lesser,
	"value cannot be compared by <",
	be_create_bool_temporary);

    return ESSTEE_OK;
}

int st_lesser_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_COMPARE_EXPRESSION_STEP(
	lesser,
	"unable to determine whether one value is smaller than the other");
    
    return INVOKE_RESULT_FINISHED;
}

int st_equals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_VERIFY(
        equals,
	"value cannot be compared by =",
	be_create_bool_temporary);

    return ESSTEE_OK;
}

int st_equals_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_COMPARE_EXPRESSION_STEP(
	equals,
	"unable to determine whether one value is equal to the other");
    
    return INVOKE_RESULT_FINISHED;
}

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
	    "value cannot be compared by >=",
	    ISSUE_ERROR_CLASS,			
	    1,							
	    be->left_operand->invoke.location(&(be->left_operand->invoke))); 
    }								

    if(be_create_bool_temporary(be, config, errors) != ESSTEE_OK)		
    {								
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

    int operand_step_result = be_step_operands(be, cursor, time, config, errors);
    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value =
	be->right_operand->return_value(be->right_operand);

    int greater_comparison = left_value->greater(left_value, right_value, config);
    int show_error = 0;
    if(greater_comparison == ESSTEE_ERROR)
    {
	show_error = 1;
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
	    show_error = 1;
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

    if(show_error)
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
	    "value cannot be compared by <=",
	    ISSUE_ERROR_CLASS,					
	    1,							
	    be->left_operand->invoke.location(&(be->left_operand->invoke))); 
    }								

    if(be_create_bool_temporary(be, config, errors) != ESSTEE_OK)		
    {								
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

    int operand_step_result = be_step_operands(be, cursor, time, config, errors);
    if(operand_step_result != INVOKE_RESULT_FINISHED)
    {
	return operand_step_result;
    }

    const struct value_iface_t *left_value =
	be->left_operand->return_value(be->left_operand);
    const struct value_iface_t *right_value =
	be->right_operand->return_value(be->right_operand);

    int lesser_comparison = left_value->lesser(left_value, right_value, config);
    int show_error = 0;
    if(lesser_comparison == ESSTEE_ERROR)
    {
	show_error = 1;
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
	    show_error = 1;
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

    if(show_error)
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

int st_nequals_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_VERIFY(
        equals,
	"value cannot be compared by <>",
	be_create_bool_temporary);

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

int st_mod_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_VERIFY(
	modulus,
	"value does not support the modulus operation",
	be_create_cloned_temporary);

    return ESSTEE_OK;
}

int st_mod_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_STEP(
        modulus,
	"unable to perform modulus operation");

    return INVOKE_RESULT_FINISHED;
}

int st_power_expression_verify(
    struct invoke_iface_t *self,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_VERIFY(
	to_power,
	"value does not support the ** operation",
	be_create_cloned_temporary);

    return ESSTEE_OK;
}

int st_power_expression_step(
    struct invoke_iface_t *self,
    struct cursor_t *cursor,
    const struct systime_iface_t *time,
    const struct config_iface_t *config,
    struct errors_iface_t *errors)
{
    BINARY_EXPRESSION_STEP(
        to_power,
	"unable to perform the ** operation");

    return INVOKE_RESULT_FINISHED;
}
