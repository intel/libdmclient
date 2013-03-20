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
 * @file <defaultroot.c>
 *
 * @brief Default Root MO
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "omadmtree_mo.h"
#include "syncml_error.h"

static int prv_init(void ** dataP)
{
    *dataP = NULL;
    return OMADM_SYNCML_ERROR_NONE;
}

static int prv_isNode(const char *uri, omadmtree_node_kind_t* node_type, void *data)
{
    if (!strcmp(uri,"."))
    {
        *node_type = OMADM_NODE_IS_INTERIOR;
    }
    else
    {
        *node_type = OMADM_NODE_NOT_EXIST;
    }

    return OMADM_SYNCML_ERROR_NONE;
}

static int prv_getACL(const char * uri, char ** aclP, void * data)
{
    *aclP = NULL;

    if (strcmp(uri,"."))
    {
        return OMADM_SYNCML_ERROR_NOT_FOUND;
    }

    *aclP = strdup("Get=*");

    return OMADM_SYNCML_ERROR_NONE;
}

static int prv_get(dmtree_node_t * nodeP,
                   void *oData)
{
    if (strcmp(nodeP->uri, "."))
    {
        return OMADM_SYNCML_ERROR_NOT_FOUND;
    }

    nodeP->format = strdup("node");
    nodeP->data_size = 0;
    nodeP->data_buffer = NULL;

    return OMADM_SYNCML_ERROR_NONE;
}

omadm_mo_interface_t * getDefaultRootPlugin()
{
    omadm_mo_interface_t *retval = NULL;

    retval = malloc(sizeof(*retval));
    if (retval) {
        memset(retval, 0, sizeof(*retval));
        retval->base_uri = strdup(".");
        retval->initFunc = prv_init;
        retval->isNodeFunc = prv_isNode;
        retval->getACLFunc = prv_getACL;
        retval->getFunc = prv_get;
    }

    return retval;
}
