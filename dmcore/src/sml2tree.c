/*
 * libdmclient
 *
 * Copyright (C) 2012 Intel Corporation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * David Navarro <david.navarro@intel.com>
 *
 */

/*!
 * @file sml2tree.c
 *
 * @brief Interface between SyncMLRTK and internal dm tree.
 *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "internals.h"

#define PRV_CONVERT_CODE(code) if ((code) == OMADM_SYNCML_ERROR_NONE) code = OMADM_SYNCML_ERROR_SUCCESS

static int prv_fill_item(SmlItemPtr_t itemP,
                         dmtree_node_t node)
{
    itemP->source = smlAllocSource();
    if (!itemP->source)
    {
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;
    }
    itemP->source->locURI = smlString2Pcdata(node.uri);
    if (node.data_buffer)
    {
        itemP->data = smlString2Pcdata((char *)(node.data_buffer));
    }
    else
    {
        itemP->data = smlString2Pcdata("");
    }
    itemP->meta = convert_to_meta(node.format, node.type);

    return OMADM_SYNCML_ERROR_SUCCESS;
}

static int prv_get(internals_t * internP,
                   const char * uri,
                   SmlItemPtr_t resultP)
{
    int code = OMADM_SYNCML_ERROR_COMMAND_FAILED;
    dmtree_node_t node;

    memset(&node, 0, sizeof(dmtree_node_t));

    node.uri = strdup(uri);
    if (node.uri)
    {
        code = dmtree_get(internP->dmtreeH, &node);
        if (OMADM_SYNCML_ERROR_NONE == code)
        {
            code = prv_fill_item(resultP, node);
            dmtree_node_clean(&node, true);
        }
    }
    return code;
}

static int prv_get_to_list(internals_t * internP,
                           const char * uri,
                           SmlItemListPtr_t * listP)
{
    int code = OMADM_SYNCML_ERROR_COMMAND_FAILED;
    SmlItemPtr_t itemP = NULL;

    itemP = smlAllocItem();
    if (itemP)
    {
        code = prv_get(internP, uri, itemP);

        if (OMADM_SYNCML_ERROR_SUCCESS == code)
        {
            SmlItemListPtr_t newListP;

            newListP = (SmlItemListPtr_t) malloc(sizeof(SmlItemList_t));
            if (newListP)
            {
                newListP->item = itemP;
                newListP->next = *listP;
                *listP = newListP;

                code = OMADM_SYNCML_ERROR_SUCCESS;
            }
        }
        if (OMADM_SYNCML_ERROR_SUCCESS != code)
        {
            smlFreeItemPtr(itemP);
        }
    }

    return code;
}

SmlReplacePtr_t get_device_info(internals_t * internP)
{
    SmlReplacePtr_t replaceP = NULL;
    SmlItemListPtr_t listP = NULL;

    if (internP == NULL)
    {
        goto error;
    }

    if (OMADM_SYNCML_ERROR_SUCCESS != prv_get_to_list(internP, "./DevInfo/Mod", &listP))
        goto error;
    if (OMADM_SYNCML_ERROR_SUCCESS != prv_get_to_list(internP, "./DevInfo/Man", &listP))
        goto error;
    if (OMADM_SYNCML_ERROR_SUCCESS != prv_get_to_list(internP, "./DevInfo/DevId", &listP))
        goto error;
    if (OMADM_SYNCML_ERROR_SUCCESS != prv_get_to_list(internP, "./DevInfo/Lang", &listP))
        goto error;
    if (OMADM_SYNCML_ERROR_SUCCESS != prv_get_to_list(internP, "./DevInfo/DmV", &listP))
        goto error;

    replaceP = smlAllocReplace();
    if (replaceP)
    {
        replaceP->itemList = listP;
        listP = NULL;
    }

error:
    if (listP) smlFreeItemList(listP);
    return replaceP;
}

int get_node(internals_t * internP,
             SmlItemPtr_t itemP,
             SmlItemPtr_t resultP)
{
    int code;
    char * uri;

    uri = smlPcdata2String(itemP->target->locURI);
    if (!uri) return OMADM_SYNCML_ERROR_NOT_FOUND;

    code = prv_get(internP, uri, resultP);

    return code;
}

int add_node(internals_t * internP,
             SmlItemPtr_t itemP)
{
    int code;
    dmtree_node_t node;

    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = smlPcdata2String(itemP->target->locURI);
    extract_from_meta(itemP->meta, &(node.format), &(node.type));
    node.data_size = itemP->data->length;
    if (node.data_size)
    {
        node.data_buffer = itemP->data->content;
    }
    else
    {
        node.data_buffer = strdup("");
    }

    code = dmtree_add(internP->dmtreeH, &node);
    PRV_CONVERT_CODE(code);

    dmtree_node_clean(&node, false);

    return code;
}

int delete_node(internals_t * internP,
                SmlItemPtr_t itemP)
{
    int code;

    code = dmtree_delete(internP->dmtreeH, smlPcdata2String(itemP->target->locURI));
    PRV_CONVERT_CODE(code);

    return code;
}

int replace_node(internals_t * internP,
                 SmlItemPtr_t itemP)
{
    int code;
    dmtree_node_t node;

    memset(&node, 0, sizeof(dmtree_node_t));
    node.uri = smlPcdata2String(itemP->target->locURI);
    extract_from_meta(itemP->meta, &(node.format), &(node.type));
    node.data_size = itemP->data->length;
    if (node.data_size)
    {
        node.data_buffer = itemP->data->content;
    }
    else
    {
        node.data_buffer = strdup("");
    }

    code = dmtree_replace(internP->dmtreeH, &node);
    PRV_CONVERT_CODE(code);

    dmtree_node_clean(&node, false);

    return code;
}

int copy_node(internals_t * internP,
              SmlItemPtr_t itemP)
{
    int code;
    char * src_uri;
    char * dst_uri;

    dst_uri = smlPcdata2String(itemP->target->locURI);
    if (!dst_uri) return OMADM_SYNCML_ERROR_NOT_FOUND;

    src_uri = smlPcdata2String(itemP->source->locURI);
    if (!src_uri)
    {
        free(dst_uri);
        return OMADM_SYNCML_ERROR_NOT_FOUND;
    }

    code = dmtree_copy(internP->dmtreeH, src_uri, dst_uri);
    PRV_CONVERT_CODE(code);

    free(dst_uri);
    free(src_uri);

    return code;
}
