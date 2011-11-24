/******************************************************************************
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file static_mo_util.c
 *
 * @brief C file for utility functions for static MOs
 *
 */

#include <stdlib.h>
#include <string.h>

#include <syncml_error.h>
#include "error_macros.h"
#include "config.h"

#include "static_mo_util.h"

static int prv_find_node(static_node_t * nodes,
                         const char * iURI)
{
    int i;

    i = 0;
    while(nodes[i].uri && strcmp(iURI, nodes[i].uri))
    {
        i++;
    }

    return i;
}

int static_mo_is_node(const char *iURI,
                      omadmtree_node_type_t *oNodeType,
                      void *iData)
{
    static_node_t * nodes = (static_node_t *)iData;
    int i;

    if (!nodes) return OMADM_SYNCML_ERROR_COMMAND_FAILED;

    i = prv_find_node(nodes, iURI);

    *oNodeType = nodes[i].type;

    return OMADM_SYNCML_ERROR_NONE;
}

int static_mo_get(dmtree_node_t * nodeP,
                  void *iData)
{
    DMC_ERR_MANAGE;

    static_node_t * nodes = (static_node_t *)iData;
    int i;

    DMC_FAIL_ERR(!nodeP || !nodeP->uri || !nodes, OMADM_SYNCML_ERROR_COMMAND_FAILED);

    nodeP->format = NULL;
    nodeP->type = NULL;
    nodeP->data_size = 0;
    nodeP->data_buffer = NULL;

    i = prv_find_node(nodes, nodeP->uri);

    switch (nodes[i].type)
    {
    case OMADM_NODE_IS_INTERIOR:
        DMC_FAIL_NULL(nodeP->format, strdup("node"), OMADM_SYNCML_ERROR_DEVICE_FULL);
        break;
    case OMADM_NODE_IS_LEAF:
        DMC_FAIL_NULL(nodeP->format, strdup("chr"), OMADM_SYNCML_ERROR_DEVICE_FULL);
        DMC_FAIL_NULL(nodeP->type, strdup("text/plain"), OMADM_SYNCML_ERROR_DEVICE_FULL);
        break;
    default:
        DMC_FAIL(OMADM_SYNCML_ERROR_NOT_FOUND);
    }

    if (nodes[i].value)
    {
        DMC_FAIL_NULL(nodeP->data_buffer, strdup(nodes[i].value), OMADM_SYNCML_ERROR_DEVICE_FULL);
        nodeP->data_size = strlen(nodes[i].value) + 1;
    }

    return OMADM_SYNCML_ERROR_NONE;

DMC_ON_ERR:
    if (nodeP->format) free (nodeP->format);
    if (nodeP->type) free (nodeP->type);
    // allocating nodeP->data_buffer is the last thing that can fail so no need to free it.

    return DMC_ERR;
}

int static_mo_getACL(const char *iURI,
                     char **oValue,
                     void *iData)
{
    static_node_t * nodes = (static_node_t *)iData;
    int i;

    *oValue = NULL;

    i = prv_find_node(nodes, iURI);

    if (nodes[i].type != OMADM_NODE_NOT_EXIST)
    {
        if (nodes[i].acl)
        {
            *oValue = strdup(nodes[i].acl);
            if (!*oValue) return OMADM_SYNCML_ERROR_DEVICE_FULL;
        }
        return OMADM_SYNCML_ERROR_NONE;
    }

    return OMADM_SYNCML_ERROR_NOT_FOUND;
}
