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

#include <parser/parser.h>
#include <elements/shared.h>
#include <util/macros.h>
#include <linker/linker.h>

#include <utlist.h>


struct array_index_t * st_append_new_array_index(
    struct array_index_t *index_list,
    struct expression_iface_t *index_expression,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct array_index_t *ai = NULL;
    ALLOC_OR_ERROR_JUMP(
	ai,
	struct array_index_t,
	parser->errors,
	error_free_resources);

    ai->index_expression = index_expression;
    LOCDUP_OR_ERROR_JUMP(
	ai->location,
	location,
	parser->errors,
	error_free_resources);

    DL_APPEND(index_list, ai);

    return index_list;
    
error_free_resources:
    free(ai);
    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
    return NULL;
}

struct invoke_parameter_t * st_new_invoke_parameter(
    char *identifier,
    const struct st_location_t *location,
    struct expression_iface_t *assigned,
    struct parser_t *parser)
{
    struct invoke_parameter_t *ip = NULL;
    struct st_location_t *l = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	ip,
	struct invoke_parameter_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	l,
	location,
	parser->errors,
	error_free_resources);    

    ip->location = l;
    ip->identifier = identifier;
    ip->expression = assigned;

    return ip;
    
error_free_resources:
    free(ip);
    free(l);
    assigned->destroy(assigned);
    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
    return NULL;
}

struct invoke_parameter_t * st_append_invoke_parameter(
    struct invoke_parameter_t *parameter_group,
    struct invoke_parameter_t *new_parameter,
    struct parser_t *parser)
{
    DL_APPEND(parameter_group, new_parameter);
    return parameter_group;
}

struct qualified_identifier_t * st_new_inner_reference(
    char *identifier,
    struct qualified_identifier_t *outer,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct qualified_identifier_t *qi = NULL;
    struct st_location_t *l = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	qi,
	struct qualified_identifier_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	l,
	location,
	parser->errors,
	error_free_resources);

    qi->location = l;
    qi->identifier = identifier;
    qi->array_index = NULL;
    qi->variable = NULL;
    qi->program = NULL;
    qi->target = NULL;

    DL_APPEND(outer, qi);

    return outer;
    
error_free_resources:
    free(l);
    free(qi);
    free(identifier);
    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
    return NULL;
}

struct qualified_identifier_t * st_attach_array_index_to_inner_ref(
    struct qualified_identifier_t *inner_ref,
    struct array_index_t *array_index,
    struct parser_t *parser)
{
    struct qualified_identifier_t *itr;
    DL_FOREACH(inner_ref, itr)
    {
	if(itr->next == NULL);
	{
	    itr->array_index = array_index;
	    break;
	}
    }

    return inner_ref;
}

struct qualified_identifier_t * st_new_qualified_identifier_inner_ref(
    char *identifier,
    struct qualified_identifier_t *inner_reference,
    const struct st_location_t *location_identifier,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct qualified_identifier_t *qi = NULL;
    struct st_location_t *l = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	qi,
	struct qualified_identifier_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	l,
	location,
	parser->errors,
	error_free_resources);

    qi->location = l;
    qi->identifier = identifier;
    qi->array_index = NULL;
    qi->variable = NULL;
    qi->program = NULL;

    parser->pou_var_ref_pool->add(
	parser->pou_var_ref_pool,
	qi->identifier,
	qi,
	NULL,
	location_identifier,
	st_qualified_identifier_base_resolved);
    
    DL_PREPEND(inner_reference, qi);
    
    return qi;
    
error_free_resources:
    free(qi);
    free(l);
    st_destroy_qualified_identifier(inner_reference);
    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
    return NULL;
}

struct qualified_identifier_t * st_new_qualified_identifier_array_index(
    char *identifier,
    struct array_index_t *array_index,
    const struct st_location_t *location_identifier,
    const struct st_location_t *location,
    struct parser_t *parser)
{
    struct qualified_identifier_t *qi = NULL;
    struct st_location_t *l = NULL;
    
    ALLOC_OR_ERROR_JUMP(
	qi,
	struct qualified_identifier_t,
	parser->errors,
	error_free_resources);

    LOCDUP_OR_ERROR_JUMP(
	l,
	location,
	parser->errors,
	error_free_resources);

    qi->location = l;
    qi->identifier = identifier;
    qi->prev = NULL;
    qi->next = NULL;
    qi->array_index = array_index;
    qi->variable = NULL;
    qi->program = NULL;

    /* If there is an array_index, referrer must be variable */
    parser->pou_var_ref_pool->add(
	parser->pou_var_ref_pool,
	qi->identifier,
	qi,
	NULL,
	location_identifier,
	st_qualified_identifier_base_resolved);
    
    return qi;
    
error_free_resources:
    free(identifier);
    free(qi);
    st_destroy_array_index(array_index);
    parser->error_strategy = PARSER_ABORT_ERROR_STRATEGY;
    return NULL;
}
