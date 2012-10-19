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

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "error.h"
#include "error_macros.h"

#include "dmsettings_utils.h"
#include "syncml_error.h"

#define PRV_BASE_URI "./Vendor/test"

static char * prv_str_cat(char * first,
                          char * second)
{
    char * string;

    string = (char *)malloc(strlen(first) + strlen(second) + 1);
    if (string)
    {
        sprintf(string, "%s", first);
        strcat(string, second);
    }

    return string;
}


static int prv_rootInitFN(void **oData)
{
    return dmsettings_open((dmsettings **)oData);
}

static void prv_rootCloseFN(void *iData)
{
    dmsettings_close((dmsettings *)iData);
}

static int prv_rootIsNodeFN(const char *iURI,
                            omadmtree_node_kind_t *oNodeType,
                            void *iData)
{
    DMC_ERR_MANAGE;

    dmsettings *settings = (dmsettings *)iData;

    DMC_FAIL(omadm_dmsettings_utils_node_exists(settings, iURI, oNodeType));

    if ((*oNodeType == OMADM_NODE_NOT_EXIST) && !strcmp(iURI, PRV_BASE_URI))
        *oNodeType = OMADM_NODE_IS_INTERIOR;

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_rootGetFN(dmtree_node_t * nodeP,
                         void *iData)
{
    DMC_ERR_MANAGE;

    dmsettings *settings = (dmsettings *)iData;
    omadmtree_node_kind_t type;

    if (strcmp(nodeP->uri, PRV_BASE_URI))
    {
        DMC_FAIL(omadm_dmsettings_utils_node_exists(settings, nodeP->uri, &type));
    }
    else
    {
        type = OMADM_NODE_IS_INTERIOR;
    }

    switch(type)
    {
    case OMADM_NODE_IS_INTERIOR:
    {
        dmc_ptr_array children;
        unsigned int i;

        dmc_ptr_array_make(&children, 16, free);
        DMC_FAIL(omadm_dmsettings_utils_get_node_children(settings, nodeP->uri, &children));
        for (i = 0; i < dmc_ptr_array_get_size(&children); ++i)
        {
            char * path = dmc_ptr_array_get(&children, i);
            char * node_name = strrchr(path,'/');
            if (node_name)
            {
                if (nodeP->data_buffer)
                {
                    char * tmp_str;
                    tmp_str = prv_str_cat(nodeP->data_buffer, node_name);
                    free(nodeP->data_buffer);
                    nodeP->data_buffer = tmp_str;
                }
                else
                {
                    DMC_FAIL_NULL(nodeP->data_buffer, strdup(node_name+1), OMADM_SYNCML_ERROR_DEVICE_FULL);
                }
            }
        }
        dmc_ptr_array_free(&children);

        if (!nodeP->data_buffer)
            nodeP->data_buffer = strdup("");
        DMC_FAIL_NULL(nodeP->format, strdup("node"), OMADM_SYNCML_ERROR_DEVICE_FULL);
        nodeP->type = NULL;
    }
    break;
    case OMADM_NODE_IS_LEAF:
    {
        DMC_FAIL(omadm_dmsettings_utils_get_value(settings, nodeP->uri, &nodeP->data_buffer));
        nodeP->data_size = strlen(nodeP->data_buffer)+1;
        DMC_FAIL(omadm_dmsettings_utils_get_meta(settings, nodeP->uri, OMADM_NODE_PROPERTY_FORMAT, &nodeP->format));
        DMC_FAIL(omadm_dmsettings_utils_get_meta(settings, nodeP->uri, OMADM_NODE_PROPERTY_TYPE, &nodeP->type));
    }
    break;
    default:
        DMC_FAIL(OMADM_NODE_NOT_EXIST);
    }

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_rootSetFN(const dmtree_node_t * nodeP,
                         void * iData)
{
    DMC_ERR_MANAGE;
    dmsettings *settings = (dmsettings *) iData;

    if (!strcmp(nodeP->format, "node"))
    {
        DMC_FAIL(omadm_dmsettings_utils_create_non_leaf(settings, nodeP->uri));
    }
    else
    {
        DMC_FAIL(omadm_dmsettings_utils_set_value(settings, nodeP->uri, nodeP->data_buffer));
        if (nodeP->format)
        {
            DMC_FAIL(omadm_dmsettings_utils_set_meta(settings, nodeP->uri, OMADM_NODE_PROPERTY_FORMAT, nodeP->format));
        }
        if (nodeP->type)
        {
            DMC_FAIL(omadm_dmsettings_utils_set_meta(settings, nodeP->uri, OMADM_NODE_PROPERTY_TYPE, nodeP->type));
        }
    }

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_rootGetACLFN(const char *iURI,
                            char **oValue,
                            void *iData)
{
    DMC_ERR_MANAGE;
    dmsettings *settings = (dmsettings *)iData;

    if (!strcmp(iURI, PRV_BASE_URI))
    {
        DMC_FAIL_NULL(*oValue, strdup("Add=*&Get=*&Replace=*&Delete=*"), OMADM_SYNCML_ERROR_DEVICE_FULL);
    }
    else
    {
        DMC_FAIL(omadm_dmsettings_utils_get_meta(settings, iURI, OMADM_NODE_PROPERTY_ACL, oValue));
    }

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_rootSetACLFN(const char *iURI,
                            const char *acl,
                            void * iData)
{
    dmsettings *settings = (dmsettings *)iData;

    return omadm_dmsettings_utils_set_meta(settings, iURI, OMADM_NODE_PROPERTY_ACL, acl);
}

static int prv_rootDeleteFN(const char *iURI,
                            void * iData)
{
    dmsettings *settings = (dmsettings *)iData;

    return omadm_dmsettings_utils_delete_node(settings, iURI);
}

omadm_mo_interface_t * omadm_get_mo_interface()
{
    omadm_mo_interface_t *retVal = NULL;

    retVal = malloc(sizeof(*retVal));
    if (retVal) {
        memset(retVal, 0, sizeof(*retVal));
        retVal->base_uri = strdup(PRV_BASE_URI);
        retVal->initFunc = prv_rootInitFN;
        retVal->closeFunc = prv_rootCloseFN;
        retVal->isNodeFunc = prv_rootIsNodeFN;
        retVal->getFunc = prv_rootGetFN;
        retVal->getACLFunc = prv_rootGetACLFN;
        retVal->setFunc = prv_rootSetFN;
        retVal->setACLFunc = prv_rootSetACLFN;
        retVal->deleteFunc = prv_rootDeleteFN;
    }

    return retVal;
}
