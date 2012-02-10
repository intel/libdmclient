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
#include <stdio.h>

#include "static_mo_util.h"

#include "config.h"
#include "syncml_error.h"


#define PRV_BASE_URI "./DevInfo"
#define PRV_URN      "urn:oma:mo:oma-dm-devinfo:1.0"


static static_node_t gDevInfoNodes[] =
{
    {PRV_BASE_URI, PRV_URN, OMADM_NODE_IS_INTERIOR, "Get=*", "DevId/Man/Mod/DmV/Lang"},
    {PRV_BASE_URI"/DevId", NULL, OMADM_NODE_IS_LEAF, NULL, "DMCtest"},
    {PRV_BASE_URI"/Man", NULL, OMADM_NODE_IS_LEAF, NULL, "test manufacturer"},
    {PRV_BASE_URI"/Mod", NULL, OMADM_NODE_IS_LEAF, NULL, "test model"},
    {PRV_BASE_URI"/DmV", NULL, OMADM_NODE_IS_LEAF, NULL, "1.0"},
    {PRV_BASE_URI"/Lang", NULL, OMADM_NODE_IS_LEAF, NULL, "test language"},

    {NULL, NULL, OMADM_NODE_NOT_EXIST, NULL},
};

static int prv_initFN(void **oData)
{
    *oData = gDevInfoNodes;
    return OMADM_SYNCML_ERROR_NONE;
}


omadm_mo_interface_t * omadm_get_mo_interface()
{
    omadm_mo_interface_t *retVal = NULL;

    retVal = malloc(sizeof(*retVal));
    if (retVal) {
        memset(retVal, 0, sizeof(*retVal));
        retVal->base_uri = strdup(PRV_BASE_URI);
        retVal->initFunc = prv_initFN;
        retVal->isNodeFunc = static_mo_is_node;
        retVal->findURNFunc = static_mo_findURN;
        retVal->getFunc = static_mo_get;
        retVal->getACLFunc = static_mo_getACL;
    }

    return retVal;
}
