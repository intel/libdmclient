/******************************************************************************
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/
/*!
 * @file dmtree.c
 *
 * @brief Interface between SyncMLRTK and internal dm tree.
 *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "internals.h"

#define PRV_CONVERT_CODE(code) if ((code) == OMADM_SYNCML_ERROR_NONE) code = OMADM_SYNCML_ERROR_SUCCESS

static void prv_node_clean(dmtree_node_t node)
{
    if (node.target_uri)
        free(node.target_uri);

    if (node.format)
        free(node.format);

    if (node.type)
        free(node.type);
    // we free data_buffer manually since most of the time it
    // is allocated by the SyncMLRTK
}

static SmlItemPtr_t prv_convert_node_to_item(dmtree_node_t * nodeP)
{
    SmlItemPtr_t itemP;

    itemP = smlAllocItem();
    if (itemP)
    {
        itemP->source = smlAllocSource();
        if (!itemP->source)
        {
            smlFreeItemPtr(itemP);
            itemP = NULL;
            goto error;
        }
        itemP->source->locURI = smlString2Pcdata((String_t)(nodeP->target_uri));
        itemP->data = smlString2Pcdata((String_t)(nodeP->data_buffer));

        itemP->meta = convert_to_meta(nodeP->format, nodeP->type);
    }

error:
    return itemP;
}

static SmlItemListPtr_t prv_convert_array_to_list(dmc_ptr_array nodeArray)
{
    SmlItemListPtr_t listP = NULL;
    unsigned int i;

    for (i =  0 ; i < dmc_ptr_array_get_size(&nodeArray) ; ++i)
    {
        dmtree_node_t * node;
        SmlItemPtr_t itemP;
        SmlItemListPtr_t newListP;

        node = (dmtree_node_t*)dmc_ptr_array_get(&nodeArray, i);
        itemP = prv_convert_node_to_item(node);
        if (!itemP)
        {
            continue;
        }

        newListP = (SmlItemListPtr_t) malloc(sizeof(SmlItemList_t));
        if (newListP)
        {
            newListP->item = itemP;
            newListP->next = listP;
            listP = newListP;
        }
    }

    return listP;
}

static int prv_fill_item(SmlItemPtr_t itemP,
                         dmtree_node_t * nodeP)
{
    itemP->source = smlAllocSource();
    if (!itemP->source)
    {
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;
    }
    itemP->source->locURI = smlString2Pcdata(nodeP->target_uri);
    itemP->data = smlString2Pcdata((char *)(nodeP->data_buffer));
    itemP->meta = convert_to_meta(nodeP->format, nodeP->type);

    return OMADM_SYNCML_ERROR_SUCCESS;
}

int get_server_account (internals_t * internP,
                        char * serverID)
{
#warning TODO: Implement the real stuff
    internP->account = (accountDesc_t *)malloc(sizeof(accountDesc_t));
    internP->account->id = strdup("funambol");
    internP->account->name = strdup("Funambol");
    internP->account->uri = strdup("http://127.0.0.1:8080/funambol/dm");
    internP->account->toServerCred = (authDesc_t *)malloc(sizeof(authDesc_t));
    internP->account->toServerCred->type = AUTH_TYPE_BASIC;
    internP->account->toServerCred->name = strdup("funambol");
    internP->account->toServerCred->secret = strdup("funambol");
    internP->account->toServerCred->data = strdup("");
    internP->account->toClientCred = (authDesc_t *)malloc(sizeof(authDesc_t));
    internP->account->toClientCred->type = AUTH_TYPE_DIGEST;
    internP->account->toClientCred->name = strdup("funambol");
    internP->account->toClientCred->secret = strdup("srvpwd");
    internP->account->toClientCred->data = strdup("");

    return OMADM_SYNCML_ERROR_NONE;
}

SmlReplacePtr_t get_device_info(internals_t * internP)
{
    SmlReplacePtr_t replaceP = NULL;
    dmc_ptr_array device_info;

    if (internP == NULL)
    {
        goto error;
    }

    dmc_ptr_array_make(&device_info, 5, (dmc_ptr_array_des)dmtree_node_free);
    if (OMADM_SYNCML_ERROR_NONE == dmtree_session_device_info(internP->dmtreeH, &device_info))
    {
        replaceP = smlAllocReplace();
        if (replaceP)
        {
            replaceP->itemList = prv_convert_array_to_list(device_info);
        }
        dmc_ptr_array_free(&device_info);
    }

error:
    return replaceP;
}

int get_node(internals_t * internP,
             SmlItemPtr_t itemP,
             SmlItemPtr_t resultP)
{
    int code;
    char * uri;
    dmtree_node_t * node;

    uri = smlPcdata2String(itemP->target->locURI);
    if (!uri) return OMADM_SYNCML_ERROR_NOT_FOUND;

    code = dmtree_session_get(internP->dmtreeH, uri, &node);
    if (OMADM_SYNCML_ERROR_NONE == code)
    {
        code = prv_fill_item(resultP, node);
        dmtree_node_free(node);
    }

    return code;
}

int add_node(internals_t * internP,
             SmlItemPtr_t itemP)
{
    int code;
    dmtree_node_t node;

    memset(&node, 0, sizeof(dmtree_node_t));
    node.target_uri = smlPcdata2String(itemP->target->locURI);
    extract_from_meta(itemP->meta, &(node.format), &(node.type));
    node.data_size = itemP->data->length;
    node.data_buffer = (uint8_t *)(itemP->data->content);

    code = dmtree_session_add(internP->dmtreeH, &node);
    PRV_CONVERT_CODE(code);

    prv_node_clean(node);

    return code;
}

int delete_node(internals_t * internP,
                SmlItemPtr_t itemP)
{
    int code;

    code = dmtree_session_delete(internP->dmtreeH, smlPcdata2String(itemP->target->locURI));
    PRV_CONVERT_CODE(code);

    return code;
}

int replace_node(internals_t * internP,
                 SmlItemPtr_t itemP)
{
    int code;
    dmtree_node_t node;

    memset(&node, 0, sizeof(dmtree_node_t));
    node.target_uri = smlPcdata2String(itemP->target->locURI);
    extract_from_meta(itemP->meta, &(node.format), &(node.type));
    node.data_size = itemP->data->length;
    node.data_buffer = (uint8_t *)(itemP->data->content);

    code = dmtree_session_replace(internP->dmtreeH, &node);
    PRV_CONVERT_CODE(code);

    prv_node_clean(node);

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

    code = dmtree_session_copy(internP->dmtreeH, src_uri, dst_uri);
    PRV_CONVERT_CODE(code);

    free(dst_uri);
    free(src_uri);

    return code;
}
