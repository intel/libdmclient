/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file omadm_plugin_devdetails.c
 *
 * @brief C file for the devdetails test plugin
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "static_mo_util.h"

#include "config.h"
#include "syncml_error.h"


#define PRV_BASE_URI "./DevDetail"


static static_node_t gDevDetailNodes[] =
{
    {PRV_BASE_URI, OMADM_NODE_IS_INTERIOR, "Get=*", "URI/DevTyp/OEM/FwV/SwV/HwV/LrgObj"},
    {PRV_BASE_URI"/DevTyp", OMADM_NODE_IS_LEAF, NULL, "mobile"},
    {PRV_BASE_URI"/OEM", OMADM_NODE_IS_LEAF, NULL, "TODO"},
    {PRV_BASE_URI"/FwV", OMADM_NODE_IS_LEAF, NULL, "TODO"},
    {PRV_BASE_URI"/SwV", OMADM_NODE_IS_LEAF, NULL, "TODO"},
    {PRV_BASE_URI"/HwV", OMADM_NODE_IS_LEAF, NULL, "1.0"},
    {PRV_BASE_URI"/LrgObj", OMADM_NODE_IS_LEAF, NULL, "true"},
    {PRV_BASE_URI"/URI", OMADM_NODE_IS_INTERIOR, NULL, "MaxDepth/MaxTotLen/MaxSegLen"},
    {PRV_BASE_URI"/URI/MaxDepth", OMADM_NODE_IS_LEAF, NULL, "0"},
    {PRV_BASE_URI"/URI/MaxTotLen", OMADM_NODE_IS_LEAF, NULL, "0"},
    {PRV_BASE_URI"/URI/MaxSegLen", OMADM_NODE_IS_LEAF, NULL, "0"},

    {NULL, OMADM_NODE_NOT_EXIST, NULL},
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
        retVal->uri = strdup(PRV_BASE_URI);
        retVal->initFunc = prv_initFN;
        retVal->isNodeFunc = static_mo_is_node;
        retVal->getFunc = static_mo_get;
        retVal->getACLFunc = static_mo_getACL;
    }

    return retVal;
}
