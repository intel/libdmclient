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


#define PRV_BASE_URI "./DmAcc"
#define PRV_URN      "urn:oma:mo:oma-dm-dmacc:1.0"


static static_node_t gDmAccNodes[] =
{
    {PRV_BASE_URI, OMADM_NODE_IS_INTERIOR, "Get=*", "test/secret"},
    {PRV_BASE_URI"/test", OMADM_NODE_IS_INTERIOR, "Get=funambol", "AppID/ServerID/Name/AppAddr/AppAuth"},
    {PRV_BASE_URI"/test/AppID", OMADM_NODE_IS_LEAF, NULL, "w7"},
    {PRV_BASE_URI"/test/ServerID", OMADM_NODE_IS_LEAF, NULL, "funambol"},
    {PRV_BASE_URI"/test/Name", OMADM_NODE_IS_LEAF, NULL, "funambol"},
    {PRV_BASE_URI"/test/AppAddr", OMADM_NODE_IS_INTERIOR, NULL, "url"},
    {PRV_BASE_URI"/test/AppAddr/url", OMADM_NODE_IS_INTERIOR, NULL, "Addr/AddrType"},
    {PRV_BASE_URI"/test/AppAddr/url/Addr", OMADM_NODE_IS_LEAF, NULL, "http://127.0.0.1:8080/funambol/dm"},
    {PRV_BASE_URI"/test/AppAddr/url/AddrType", OMADM_NODE_IS_LEAF, NULL, "URI"},
    {PRV_BASE_URI"/test/AppAuth", OMADM_NODE_IS_INTERIOR, NULL, "toclient/toserver"},
    {PRV_BASE_URI"/test/AppAuth/toclient", OMADM_NODE_IS_INTERIOR, NULL, "AAuthLevel/AAuthType/AAuthName/AAuthSecret/AAuthData"},
    {PRV_BASE_URI"/test/AppAuth/toserver", OMADM_NODE_IS_INTERIOR, NULL, "AAuthLevel/AAuthType/AAuthName/AAuthSecret/AAuthData"},
    {PRV_BASE_URI"/test/AppAuth/toclient/AAuthLevel", OMADM_NODE_IS_LEAF, NULL, "SRVCRED"},
    {PRV_BASE_URI"/test/AppAuth/toclient/AAuthType", OMADM_NODE_IS_LEAF, NULL, "DIGEST"},
    {PRV_BASE_URI"/test/AppAuth/toclient/AAuthName", OMADM_NODE_IS_LEAF, NULL, "funambol"},
    {PRV_BASE_URI"/test/AppAuth/toclient/AAuthSecret", OMADM_NODE_IS_LEAF, "", "srvpwd"},
    {PRV_BASE_URI"/test/AppAuth/toclient/AAuthData", OMADM_NODE_IS_LEAF, "", NULL},
    {PRV_BASE_URI"/test/AppAuth/toserver/AAuthLevel", OMADM_NODE_IS_LEAF, NULL, "CLCRED"},
    {PRV_BASE_URI"/test/AppAuth/toserver/AAuthType", OMADM_NODE_IS_LEAF, NULL, "BASIC"},
    {PRV_BASE_URI"/test/AppAuth/toserver/AAuthName", OMADM_NODE_IS_LEAF, NULL, "funambol"},
    {PRV_BASE_URI"/test/AppAuth/toserver/AAuthSecret", OMADM_NODE_IS_LEAF, "", "funambol"},
    {PRV_BASE_URI"/test/AppAuth/toserver/AAuthData", OMADM_NODE_IS_LEAF, "", NULL},

    {PRV_BASE_URI"/secret", OMADM_NODE_IS_INTERIOR, "Get=unused", "AppID/ServerID/Name/AppAddr/AppAuth"},
    {PRV_BASE_URI"/secret/AppID", OMADM_NODE_IS_LEAF, NULL, "w7"},
    {PRV_BASE_URI"/secret/ServerID", OMADM_NODE_IS_LEAF, NULL, "unused"},
    {PRV_BASE_URI"/secret/Name", OMADM_NODE_IS_LEAF, NULL, "ACL testing"},
    {PRV_BASE_URI"/secret/AppAddr", OMADM_NODE_IS_INTERIOR, NULL, "url"},
    {PRV_BASE_URI"/secret/AppAddr/url", OMADM_NODE_IS_INTERIOR, NULL, "Addr/AddrType"},
    {PRV_BASE_URI"/secret/AppAddr/url/Addr", OMADM_NODE_IS_LEAF, NULL, "http://127.0.0.1"},
    {PRV_BASE_URI"/secret/AppAddr/url/AddrType", OMADM_NODE_IS_LEAF, NULL, "URI"},
    {PRV_BASE_URI"/secret/AppAuth", OMADM_NODE_IS_INTERIOR, NULL, "toclient/toserver"},
    {PRV_BASE_URI"/secret/AppAuth/toclient", OMADM_NODE_IS_INTERIOR, NULL, "AAuthLevel/AAuthType/AAuthName/AAuthSecret/AAuthData"},
    {PRV_BASE_URI"/secret/AppAuth/toserver", OMADM_NODE_IS_INTERIOR, NULL, "AAuthLevel/AAuthType/AAuthName/AAuthSecret/AAuthData"},
    {PRV_BASE_URI"/secret/AppAuth/toclient/AAuthLevel", OMADM_NODE_IS_LEAF, NULL, "SRVCRED"},
    {PRV_BASE_URI"/secret/AppAuth/toclient/AAuthType", OMADM_NODE_IS_LEAF, NULL, "BASIC"},
    {PRV_BASE_URI"/secret/AppAuth/toclient/AAuthName", OMADM_NODE_IS_LEAF, NULL, "unused"},
    {PRV_BASE_URI"/secret/AppAuth/toclient/AAuthSecret", OMADM_NODE_IS_LEAF, "", "unused"},
    {PRV_BASE_URI"/secret/AppAuth/toclient/AAuthData", OMADM_NODE_IS_LEAF, "", NULL},
    {PRV_BASE_URI"/secret/AppAuth/toserver/AAuthLevel", OMADM_NODE_IS_LEAF, NULL, "CLCRED"},
    {PRV_BASE_URI"/secret/AppAuth/toserver/AAuthType", OMADM_NODE_IS_LEAF, NULL, "BASIC"},
    {PRV_BASE_URI"/secret/AppAuth/toserver/AAuthName", OMADM_NODE_IS_LEAF, NULL, "unused"},
    {PRV_BASE_URI"/secret/AppAuth/toserver/AAuthSecret", OMADM_NODE_IS_LEAF, "", "unused"},
    {PRV_BASE_URI"/secret/AppAuth/toserver/AAuthData", OMADM_NODE_IS_LEAF, "", NULL},

    {NULL, OMADM_NODE_NOT_EXIST, NULL},
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
        retVal->urn = strdup(PRV_URN);
        retVal->initFunc = prv_initFN;
        retVal->isNodeFunc = static_mo_is_node;
        retVal->getFunc = static_mo_get;
        retVal->getACLFunc = static_mo_getACL;
    }

    return retVal;
}
