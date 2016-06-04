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

#pragma once

#include <util/iconfig.h>
#include <util/iissues.h>
#include <util/bitflag.h>
#include <rt/isystime.h>

#include <stddef.h>
#include <stdint.h>

struct array_index_t;
struct variable_t;
struct enum_item_t;
struct cursor_iface_t;
struct invoke_parameters_iface_t;
struct duration_t;
struct date_t;
struct tod_t;
struct array_init_value_t;
struct struct_init_value_t;
struct type_iface_t;

#define TEMPORARY_VALUE (1 << 0)
#define CONSTANT_VALUE  (1 << 1)

struct value_iface_t {

    /**
     * Writes the value into the buffer.
     * @param self the "this" pointer.
     * @param buffer start of buffer to write to.
     * @param buffer_size maximum number of characters to write.
     * @param config configuration to take into consideration.
     * @return number of bytes written, if write succeeded.
     * @return ESSTEE_FALSE if buffer was full.
     * @return ESSTEE_ERROR on failure.
     */
    int (*display)(
	const struct value_iface_t *self,
	char *buffer,
	size_t buffer_size,
	const struct config_iface_t *config);

    int (*assign)(
	struct value_iface_t *self,
	const struct value_iface_t *new_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    /* The functions assignable_from, comparable_to and operates_with,
     * may point to the same function, if that is the behaviour of the
     * value */
    
    /**
     * Determines whether a value can be assigned another value.
     * @param self the "this" pointer.
     * @param other_value the value to which assignment is evaluated.
     * @param config configuration to take into consideration.
     * @return ESSTEE_TRUE if assignment is possible. 
     * @return ESSTEE_FALSE if assignment is impossible. 
     */
    int (*assignable_from)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    /**
     * Determines whether a value can be compared to another value.
     * @param self the "this" pointer.
     * @param other_value the value to which comparison is evaluated.
     * @param config configuration to take into consideration.
     * @return ESSTEE_TRUE if comparison is possible. 
     * @return ESSTEE_TYPE_INCOMPATIBLE if comparison is impossible due to type incompatibility. 
     * @return ESSTEE_FALSE if comparison is impossible. 
     */
    int (*comparable_to)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    /**
     * Determines whether a value can be used in a binary expression
     * together with another value.
     *
     * @param self the "this" pointer.
     * @param other_value the value to which operation is evaluated.
     * @param config configuration to take into consideration.
     * @return ESSTEE_TRUE if binary expressions are possible. 
     * @return ESSTEE_FALSE if binary expression are impossible. 
     */    
    int (*operates_with)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    st_bitflag_t (*class)(
	const struct value_iface_t *self);
    
    int (*override_type)(
	const struct value_iface_t *self,
	const struct type_iface_t *type,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    const struct type_iface_t * (*type_of)(
	const struct value_iface_t *self);

    struct value_iface_t * (*index)(
	struct value_iface_t *self,
	struct array_index_t *array_index,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    struct variable_t * (*sub_variable)(
	struct value_iface_t *self,
	const char *identifier,
    	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    struct value_iface_t * (*create_temp_from)(
	const struct value_iface_t *self,
	struct issues_iface_t *issues);
    
    void (*destroy)(
	struct value_iface_t *self);

    int (*invoke_verify)(
	struct value_iface_t *self,
	struct invoke_parameters_iface_t *parameters,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*invoke_step)(
	struct value_iface_t *self,
	struct invoke_parameters_iface_t *parameters,
	struct cursor_iface_t *cursor,
	const struct systime_iface_t *time,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*invoke_reset)(
	struct value_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);	

    int (*not)(
	const struct value_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*negate)(
	const struct value_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    /* Binary comparison operations */
    int (*greater)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*lesser)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*equals)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    /**@addtogroup value_binary_operations Binary operatios
     *
     * @brief Binary operations perfomed with self as the left operand
     * and the other value as right operand.
     *
     * @details The self pointer points to a temporary value created
     * by the create_temp_from function. When the binary operation is
     * executed, the temporary value (previously created) is 1)
     * assigned the value of the left operand, 2) modified by calling
     * the binary operation on the temporary (considered as left
     * operand) with the other value (as right operand). These
     * functions store the result of the value in the self pointer.
     *
     * If the value does not support one operation that function
     * pointer should be set to NULL.
     */

    /**
     * Modifies self by xor:ing with the other_value.
     * @return ESSTEE_OK if operation succeeded
     */
    int (*xor)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*and)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*or)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*plus)(
	struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    int (*minus)(
	struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*multiply)(
	struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*divide)(
	struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*modulus)(
	struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*to_power)(
	struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    /* Data access methods */
    int64_t (*integer)(
	const struct value_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    int (*bool)(
	const struct value_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    double (*real)(
	const struct value_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    const char * (*string)(
	const struct value_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    const struct enum_item_t * (*enumeration)(
	const struct value_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    const struct duration_t * (*duration)(
	const struct value_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);
    
    const struct date_t * (*date)(
	const struct value_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    const struct tod_t * (*tod)(
	const struct value_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    const struct date_tod_t * (*date_tod)(
	const struct value_iface_t *self,
	const struct config_iface_t *config,
	struct issues_iface_t *issues);

    const struct array_init_value_t * (*array_init_value)(
	const struct value_iface_t *self);

    const struct struct_init_value_t * (*struct_init_value)(
	const struct value_iface_t *self);
};
