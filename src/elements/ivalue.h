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

#include <elements/itype.h>
#include <util/iconfig.h>
#include <util/bitflag.h>

#include <stddef.h>
#include <stdint.h>

#define VALUE_TEMP_CLASS (1 << 0)

struct array_index_t;
struct variable_t;
struct enum_item_t;

struct value_iface_t {

    /**
     * Writes the value into the buffer.
     * @param self the "this" pointer
     * @param buffer start of buffer to write to
     * @param buffer_size maximum number of characters to write
     * @param config configuration to take into consideration
     * @return number of bytes written, if write succeeded, otherwise ESSTEE_ERROR
     */
    int (*display)(
	const struct value_iface_t *self,
	char *buffer,
	size_t buffer_size,
	const struct config_iface_t *config);

    int (*assign)(
	struct value_iface_t *self,
	const struct value_iface_t *new_value,
	const struct config_iface_t *config);

    const struct type_iface_t * (*explicit_type)(
	const struct value_iface_t *self);

    struct value_iface_t * (*index)(
	struct value_iface_t *self,
	struct array_index_t *array_index,
	const struct config_iface_t *config);

    struct variable_t * (*sub_variable)(
	struct value_iface_t *self,
	const char *identifier,
    	const struct config_iface_t *config);

    int (*compatible)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config);
    
    struct value_iface_t * (*create_temp_from)(
	const struct value_iface_t *self);	
    
    void (*destroy)(
	struct value_iface_t *self);
    
    /* Binary comparision operations */
    int (*greater)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config);

    int (*lesser)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config);
    
    int (*equals)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config);

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
     *
     */

    /**
     * Modifies self by xor:ing with the other_value.
     * @return ESSTEE_OK if operation succeeded
     */

    /*
      TODO: finish documentation
      -> compatible

      bin op

      var op (literal op temp)

      COMPILE TIME
      
      left compatible temp
      - temp->integer
      (- temp->integer = 0, cannot be used to check comp with type)

      left compatible literal
      - literal->integer
      - left.type can hold literal->integer

      left compatbile typed literal
      - literal->integer
      (- left.type can hold literal->integer)
      - left.type = literal.type

      left compatible var backed
      - var->integer
      (- left.type can hold var->integer)
      - left.type = var.type
       
      comparable
      
      left comparable temp
      - temp->integer
      
      left comparable literal
      - literal->integer
      - left.can_hold literal->integer (possibly)

      left comparable type literal
      - literal->integer
      (- left.type can hold literal->integer)
      - left.type = literal.type

      left comparable var backed
      - var->integer
      (- left.type can hold var->integer)
      - left.type = var.type



      RUNTIME

      assign may fail at runtime

      left.assign :

      left assign temp
      * left.type.can_hold temp

      left assign literal
      * 

      left assign typed literal
      * 

      left assign var backed
      * 

      
      self is temp, other is literal
      -> other->integer

      self is temp, other is typed literal
      -> other->integer

      self is temp, other is temp
      -> temp->integer
      
      self is temp, other is variable backed
      -> 

      bin comp

      self is variable backed, other literal
      self is variable backed, other temp
      self is variable backed, other variable backed
    */
	
    int (*xor)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config);

    int (*and)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config);
    
    int (*or)(
	const struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config);

    int (*plus)(
	struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config);
    
    int (*minus)(
	struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config);

    int (*multiply)(
	struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config);

    int (*divide)(
	struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config);

    int (*modulus)(
	struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config);

    int (*to_power)(
	struct value_iface_t *self,
	const struct value_iface_t *other_value,
	const struct config_iface_t *config);

    /* Data access methods */
    int64_t (*integer)(
	const struct value_iface_t *self,
	const struct config_iface_t *conf);

    int (*bool)(
	const struct value_iface_t *self,
	const struct config_iface_t *conf);

    double (*real)(
	const struct value_iface_t *self,
	const struct config_iface_t *conf);

    const char * (*string)(
	const struct value_iface_t *self,
	st_bitflag_t string_type,
	const struct config_iface_t *conf);

    const struct enum_item_t * (*enumeration)(
	const struct value_iface_t *self,
	const struct config_iface_t *conf);
    
    const struct date_value_t * (*date)(
	const struct value_iface_t *self,
	const struct config_iface_t *conf);

    const struct tod_value_t * (*tod)(
	const struct value_iface_t *self,
	const struct config_iface_t *conf);

    const struct date_tod_value_t * (*date_tod)(
	const struct value_iface_t *self,
	const struct config_iface_t *conf);

    const struct array_init_value_t * (*array_init_value)(
	const struct value_iface_t *self,
	const struct config_iface_t *conf);

    const struct struct_init_value_t * (*struct_init_value)(
	const struct value_iface_t *self,
	const struct config_iface_t *conf);
};
