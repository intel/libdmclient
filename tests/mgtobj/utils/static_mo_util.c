/*
 * libdmclient test materials
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

#include <stdlib.h>
#include <string.h>

#include <syncml_error.h>
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
                      omadmtree_node_kind_t *oNodeType,
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
    int err = OMADM_SYNCML_ERROR_NONE;

    static_node_t * nodes = (static_node_t *)iData;
    int i;

    if (!nodeP || !nodeP->uri || !nodes)
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;

    nodeP->format = NULL;
    nodeP->type = NULL;
    nodeP->data_size = 0;
    nodeP->data_buffer = NULL;

    i = prv_find_node(nodes, nodeP->uri);

    switch (nodes[i].type)
    {
    case OMADM_NODE_IS_INTERIOR:
        nodeP->format = strdup("node");
	if (!nodeP->format)
        {
	    err = OMADM_SYNCML_ERROR_DEVICE_FULL;
            goto on_error;
        }
        break;
    case OMADM_NODE_IS_LEAF:
        nodeP->format = strdup("chr");
        nodeP->type = strdup("text/plain");
	if (!nodeP->format || !nodeP->type)
	{
	    err = OMADM_SYNCML_ERROR_DEVICE_FULL;
            goto on_error;
        }
        break;
    default:
        err = OMADM_SYNCML_ERROR_NOT_FOUND;
	goto on_error;
    }

    if (nodes[i].value)
    {
        nodeP->data_buffer = strdup(nodes[i].value);
	if (!nodeP->data_buffer)
	{
	    err = OMADM_SYNCML_ERROR_DEVICE_FULL;
            goto on_error;
        }
        nodeP->data_size = strlen(nodes[i].value) + 1;
    }

    return OMADM_SYNCML_ERROR_NONE;

on_error:
    if (nodeP->format) free (nodeP->format);
    if (nodeP->type) free (nodeP->type);
    // allocating nodeP->data_buffer is the last thing that can fail so no need to free it.

    return err;
}

int static_mo_getACL(const char *iURI,
                     char **oValue,
                     void *iData)
{
    static_node_t * nodes = (static_node_t *)iData;
    int i;

    if (!nodes || !oValue || !iURI)
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;

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

int static_mo_findURN(const char *iURN,
                      char ***oURL,
                      void *iData)
{
    static_node_t * nodes = (static_node_t *)iData;
    int i;
    int count;

    if (!nodes || !oURL || !iURN)
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;

    *oURL = NULL;
    count = 0;
    i = 0;
    while(nodes[i].uri)
    {
        if (nodes[i].urn && !strcmp(iURN, nodes[i].urn))
        {
            char ** tmpP;
            count++;
            tmpP = (char **)malloc((count + 1) * sizeof(char*));
            if (NULL == tmpP)
            {
                if (NULL != *oURL)
                {
                    int j;
                    j = 0;
                    while(NULL != (*oURL)[j])
                    {
                        free((*oURL)[j]);
                        j++;
                    }
                    free(*oURL);
                }
                return OMADM_SYNCML_ERROR_DEVICE_FULL;
            }
            tmpP[count - 1] = strdup(nodes[i].uri);
            tmpP[count] = NULL;
            if (count > 1)
            {
                memcpy(tmpP, *oURL, (count-1) * sizeof(char*));
                free(*oURL);
            }
            *oURL = tmpP;
        }
        i++;
    }

    if (count > 0)
    {
        return OMADM_SYNCML_ERROR_NONE;
    }
    return OMADM_SYNCML_ERROR_NOT_FOUND;
}
