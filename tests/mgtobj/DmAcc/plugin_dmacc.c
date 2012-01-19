/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file plugin_dmAcc.c
 *
 * @brief C file for the dmacc test plugin
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "static_mo_util.h"

#include "config.h"
#include "syncml_error.h"


#define PRV_BASE_URI "./DMAcc"
#define PRV_URN      "urn:oma:mo:oma-dm-dmacc:1.0"


static static_node_t gDmAccNodes[] =
{
    {PRV_BASE_URI, NULL, OMADM_NODE_IS_INTERIOR, "Get=*", "test/secret"},
    {PRV_BASE_URI"/test", PRV_URN, OMADM_NODE_IS_INTERIOR, "Get=funambol", "AppID/ServerID/Name/AppAddr/AppAuth"},
    {PRV_BASE_URI"/test/AppID", NULL, OMADM_NODE_IS_LEAF, NULL, "w7"},
    {PRV_BASE_URI"/test/ServerID", NULL, OMADM_NODE_IS_LEAF, NULL, "funambol"},
    {PRV_BASE_URI"/test/Name", NULL, OMADM_NODE_IS_LEAF, NULL, "funambol"},
    {PRV_BASE_URI"/test/AppAddr", NULL, OMADM_NODE_IS_INTERIOR, NULL, "url"},
    {PRV_BASE_URI"/test/AppAddr/url", NULL, OMADM_NODE_IS_INTERIOR, NULL, "Addr/AddrType"},
    {PRV_BASE_URI"/test/AppAddr/url/Addr", NULL, OMADM_NODE_IS_LEAF, NULL, "http://127.0.0.1:8080/funambol/dm"},
    {PRV_BASE_URI"/test/AppAddr/url/AddrType", NULL, OMADM_NODE_IS_LEAF, NULL, "URI"},
    {PRV_BASE_URI"/test/AppAuth", NULL, OMADM_NODE_IS_INTERIOR, NULL, "toclient/toserver"},
    {PRV_BASE_URI"/test/AppAuth/toclient", NULL, OMADM_NODE_IS_INTERIOR, NULL, "AAuthLevel/AAuthType/AAuthName/AAuthSecret/AAuthData"},
    {PRV_BASE_URI"/test/AppAuth/toserver", NULL, OMADM_NODE_IS_INTERIOR, NULL, "AAuthLevel/AAuthType/AAuthName/AAuthSecret/AAuthData"},
    {PRV_BASE_URI"/test/AppAuth/toclient/AAuthLevel", NULL, OMADM_NODE_IS_LEAF, NULL, "SRVCRED"},
    {PRV_BASE_URI"/test/AppAuth/toclient/AAuthType", NULL, OMADM_NODE_IS_LEAF, NULL, "DIGEST"},
    {PRV_BASE_URI"/test/AppAuth/toclient/AAuthName", NULL, OMADM_NODE_IS_LEAF, NULL, "funambol"},
    {PRV_BASE_URI"/test/AppAuth/toclient/AAuthSecret", NULL, OMADM_NODE_IS_LEAF, "", "srvpwd"},
    {PRV_BASE_URI"/test/AppAuth/toclient/AAuthData", NULL, OMADM_NODE_IS_LEAF, "", NULL},
    {PRV_BASE_URI"/test/AppAuth/toserver/AAuthLevel", NULL, OMADM_NODE_IS_LEAF, NULL, "CLCRED"},
    {PRV_BASE_URI"/test/AppAuth/toserver/AAuthType", NULL, OMADM_NODE_IS_LEAF, NULL, "BASIC"},
    {PRV_BASE_URI"/test/AppAuth/toserver/AAuthName", NULL, OMADM_NODE_IS_LEAF, NULL, "funambol"},
    {PRV_BASE_URI"/test/AppAuth/toserver/AAuthSecret", NULL, OMADM_NODE_IS_LEAF, "", "funambol"},
    {PRV_BASE_URI"/test/AppAuth/toserver/AAuthData", NULL, OMADM_NODE_IS_LEAF, "", NULL},

    {PRV_BASE_URI"/secret", PRV_URN, OMADM_NODE_IS_INTERIOR, "Get=unused", "AppID/ServerID/Name/AppAddr/AppAuth"},
    {PRV_BASE_URI"/secret/AppID", NULL, OMADM_NODE_IS_LEAF, NULL, "w7"},
    {PRV_BASE_URI"/secret/ServerID", NULL, OMADM_NODE_IS_LEAF, NULL, "unused"},
    {PRV_BASE_URI"/secret/Name", NULL, OMADM_NODE_IS_LEAF, NULL, "ACL testing"},
    {PRV_BASE_URI"/secret/AppAddr", NULL, OMADM_NODE_IS_INTERIOR, NULL, "url"},
    {PRV_BASE_URI"/secret/AppAddr/url", NULL, OMADM_NODE_IS_INTERIOR, NULL, "Addr/AddrType"},
    {PRV_BASE_URI"/secret/AppAddr/url/Addr", NULL, OMADM_NODE_IS_LEAF, NULL, "http://127.0.0.1"},
    {PRV_BASE_URI"/secret/AppAddr/url/AddrType", NULL, OMADM_NODE_IS_LEAF, NULL, "URI"},
    {PRV_BASE_URI"/secret/AppAuth", NULL, OMADM_NODE_IS_INTERIOR, NULL, "toclient/toserver"},
    {PRV_BASE_URI"/secret/AppAuth/toclient", NULL, OMADM_NODE_IS_INTERIOR, NULL, "AAuthLevel/AAuthType/AAuthName/AAuthSecret/AAuthData"},
    {PRV_BASE_URI"/secret/AppAuth/toserver", NULL, OMADM_NODE_IS_INTERIOR, NULL, "AAuthLevel/AAuthType/AAuthName/AAuthSecret/AAuthData"},
    {PRV_BASE_URI"/secret/AppAuth/toclient/AAuthLevel", NULL, OMADM_NODE_IS_LEAF, NULL, "SRVCRED"},
    {PRV_BASE_URI"/secret/AppAuth/toclient/AAuthType", NULL, OMADM_NODE_IS_LEAF, NULL, "BASIC"},
    {PRV_BASE_URI"/secret/AppAuth/toclient/AAuthName", NULL, OMADM_NODE_IS_LEAF, NULL, "unused"},
    {PRV_BASE_URI"/secret/AppAuth/toclient/AAuthSecret", NULL, OMADM_NODE_IS_LEAF, "", "unused"},
    {PRV_BASE_URI"/secret/AppAuth/toclient/AAuthData", NULL, OMADM_NODE_IS_LEAF, "", NULL},
    {PRV_BASE_URI"/secret/AppAuth/toserver/AAuthLevel", NULL, OMADM_NODE_IS_LEAF, NULL, "CLCRED"},
    {PRV_BASE_URI"/secret/AppAuth/toserver/AAuthType", NULL, OMADM_NODE_IS_LEAF, NULL, "BASIC"},
    {PRV_BASE_URI"/secret/AppAuth/toserver/AAuthName", NULL, OMADM_NODE_IS_LEAF, NULL, "unused"},
    {PRV_BASE_URI"/secret/AppAuth/toserver/AAuthSecret", NULL, OMADM_NODE_IS_LEAF, "", "unused"},
    {PRV_BASE_URI"/secret/AppAuth/toserver/AAuthData", NULL, OMADM_NODE_IS_LEAF, "", NULL},

    {NULL, NULL, OMADM_NODE_NOT_EXIST, NULL},
};

static int prv_initFN(void **oData)
{
    *oData = gDmAccNodes;
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
