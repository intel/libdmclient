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

#include "syncml_error.h"


#define PRV_BASE_URI "./DevDetail"
#define PRV_URN      "urn:oma:mo:oma-dm-devdetail:1.0"


static static_node_t gDevDetailNodes[] =
{
    {PRV_BASE_URI, PRV_URN, OMADM_NODE_IS_INTERIOR, "Get=*", "URI/DevTyp/OEM/FwV/SwV/HwV/LrgObj"},
    {PRV_BASE_URI"/DevTyp", NULL, OMADM_NODE_IS_LEAF, NULL, "mobile"},
    {PRV_BASE_URI"/OEM", NULL, OMADM_NODE_IS_LEAF, NULL, "TODO"},
    {PRV_BASE_URI"/FwV", NULL, OMADM_NODE_IS_LEAF, NULL, "TODO"},
    {PRV_BASE_URI"/SwV", NULL, OMADM_NODE_IS_LEAF, NULL, "TODO"},
    {PRV_BASE_URI"/HwV", NULL, OMADM_NODE_IS_LEAF, NULL, "1.0"},
    {PRV_BASE_URI"/LrgObj", NULL, OMADM_NODE_IS_LEAF, NULL, "true"},
    {PRV_BASE_URI"/URI", NULL, OMADM_NODE_IS_INTERIOR, NULL, "MaxDepth/MaxTotLen/MaxSegLen"},
    {PRV_BASE_URI"/URI/MaxDepth", NULL, OMADM_NODE_IS_LEAF, NULL, "0"},
    {PRV_BASE_URI"/URI/MaxTotLen", NULL, OMADM_NODE_IS_LEAF, NULL, "0"},
    {PRV_BASE_URI"/URI/MaxSegLen", NULL, OMADM_NODE_IS_LEAF, NULL, "0"},

    {NULL, NULL, OMADM_NODE_NOT_EXIST, NULL},
};

static int prv_initFN(void **oData)
{
    *oData = gDevDetailNodes;
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
