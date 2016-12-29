/*
Copyright (C) 2016 Kristian Nordman

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
#include <elements/qualified_identifier.h>


struct qualified_identifier_iface_t * st_new_qualified_identifier_by_index(
    char *identifier,
    const struct st_location_t *identifier_location,
    struct array_index_iface_t *index,
    const struct st_location_t *qid_location,
    struct parser_t *parser)
{
    struct qualified_identifier_iface_t *qid =
	st_create_qualified_identifier(parser->pou_var_ref_pool,
				       parser->config,
				       parser->errors);

    if(!qid)
    {
	goto error_free_resources;
    }

    int extend_result = qid->extend_by_index(qid,
					     identifier,
					     identifier_location,
					     index,
					     qid_location,
					     parser->config,
					     parser->errors);
    if(extend_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    return qid;
    
error_free_resources:
    free(identifier);
    index->destroy(index);
    return NULL;
}

struct qualified_identifier_iface_t * st_new_qualified_identifier_by_sub_ref(
    char *identifier,
    const struct st_location_t *identifier_location,
    char *sub_identifier,
    const struct st_location_t *sub_identifier_location,
    const struct st_location_t *qid_location,
    struct parser_t *parser)
{
    struct qualified_identifier_iface_t *qid =
	st_create_qualified_identifier(parser->pou_var_ref_pool,
				       parser->config,
				       parser->errors);

    if(!qid)
    {
	goto error_free_resources;
    }

    int extend_result = qid->extend(qid,
				    identifier,
				    identifier_location,
				    qid_location,
				    parser->config,
				    parser->errors);
    
    if(extend_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }

    extend_result = qid->extend(qid,
				sub_identifier,
				sub_identifier_location,
				qid_location,
				parser->config,
				parser->errors);

    if(extend_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    return qid;
    
error_free_resources:
    if(qid)
    {
	qid->destroy(qid);
    }
    free(identifier);
    return NULL;
}

struct qualified_identifier_iface_t * st_extend_qualified_identifier_by_index(
    struct qualified_identifier_iface_t *qid,
    char *identifier,
    const struct st_location_t *identifier_location,
    struct array_index_iface_t *index,
    const struct st_location_t *qid_location,
    struct parser_t *parser)
{
    int extend_result = qid->extend_by_index(qid,
					     identifier,
					     identifier_location,
					     index,
					     qid_location,
					     parser->config,
					     parser->errors);
    if(extend_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    return qid;
    
error_free_resources:
    free(identifier);
    index->destroy(index);
    qid->destroy(qid);
    return NULL;
}

struct qualified_identifier_iface_t * st_extend_qualified_identifier_by_sub_ref(
    struct qualified_identifier_iface_t *qid,
    char *identifier,
    const struct st_location_t *identifier_location,
    const struct st_location_t *qid_location,
    struct parser_t *parser)
{
    int extend_result = qid->extend(qid,
				    identifier,
				    identifier_location,
				    qid_location,
				    parser->config,
				    parser->errors);
    
    if(extend_result != ESSTEE_OK)
    {
	goto error_free_resources;
    }
    
    return qid;
    
error_free_resources:
    free(identifier);
    qid->destroy(qid);
    return NULL;
}
