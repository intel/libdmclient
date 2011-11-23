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
#include <omadmtree_mo.h>

#include "error_macros.h"

#include "config.h"
#include "syncml_error.h"

/* TODO: rewrite using mo_base */


static int prv_devDetailInitFN(void **oData)
{
    *oData = NULL;
    return OMADM_SYNCML_ERROR_NONE;
}

static void prv_devDetailCloseFN(void *iData)
{
}

static int prv_devDetailIsNodeFN(const char *iURI,
                                 omadmtree_node_type_t *oNodeType,
                                 void *iData)
{
    if (!strcmp(iURI, "./DevDetail") || !strcmp(iURI, "./DevDetail/URI"))
        *oNodeType = OMADM_NODE_IS_INTERIOR;
    else if (!strcmp(iURI, "./DevDetail/DevTyp")
         || !strcmp(iURI, "./DevDetail/OEM")
         || !strcmp(iURI, "./DevDetail/FwV")
         || !strcmp(iURI, "./DevDetail/SwV")
         || !strcmp(iURI, "./DevDetail/HwV")
         || !strcmp(iURI, "./DevDetail/LrgObj")
         || !strcmp(iURI, "./DevDetail/URI/MaxDepth")
         || !strcmp(iURI, "./DevDetail/URI/MaxTotLen")
         || !strcmp(iURI, "./DevDetail/URI/MaxSegLen"))
        *oNodeType = OMADM_NODE_IS_LEAF;
    else
        *oNodeType = OMADM_NODE_NOT_EXIST;

    return OMADM_SYNCML_ERROR_NONE;
}

static int prv_devDetailGetFN(dmtree_node_t * nodeP,
                              void *iData)
{
    DMC_ERR_MANAGE;

    const char *retVal = NULL;

    nodeP->format = NULL;
    nodeP->type = NULL;
    nodeP->data_size = 0;
    nodeP->data_buffer = NULL;

    if (!strcmp(nodeP->uri, "./DevDetail"))
    {
        nodeP->format = strdup("node");
        nodeP->type = strdup("urn:oma:mo:oma-dm-devdetail:1.0");
        retVal = "DevTyp/OEM/FwV/SwV/HwV/LrgObj/URI";
    }
    else if (!strcmp(nodeP->uri, "./DevDetail/URI"))
    {
        nodeP->format = strdup("node");
        retVal = "MaxDepth/MaxTotLen/MaxSegLen";
    }
    else
    {
        DMC_FAIL_NULL(nodeP->format, strdup("chr"), OMADM_SYNCML_ERROR_DEVICE_FULL);
        DMC_FAIL_NULL(nodeP->type, strdup("text/plain"), OMADM_SYNCML_ERROR_DEVICE_FULL);
        if (!strcmp(nodeP->uri, "./DevDetail/DevTyp"))
            retVal = "mobile";
        else if (!strcmp(nodeP->uri, "./DevDetail/OEM"))
            retVal = "TODO";
        else if (!strcmp(nodeP->uri, "./DevDetail/FwV"))
            retVal = "TODO";
        else if (!strcmp(nodeP->uri, "./DevDetail/SwV"))
            retVal = "TODO";
        else if (!strcmp(nodeP->uri, "./DevDetail/HwV"))
            retVal = "1.0";
        else if (!strcmp(nodeP->uri, "./DevDetail/LrgObj"))
            retVal = "true";
        else if (!strcmp(nodeP->uri, "./DevDetail/URI/MaxDepth")) {
            retVal = "16";
        } else if (!strcmp(nodeP->uri, "./DevDetail/URI/MaxTotLen")) {
            retVal = "256";
        } else if (!strcmp(nodeP->uri, "./DevDetail/URI/MaxSegLen")) {
            retVal = "64";
        }
        else
        {
            free(nodeP->format);
            free(nodeP->type);
            DMC_FAIL(OMADM_SYNCML_ERROR_NOT_FOUND);
        }
    }
    DMC_FAIL_NULL(nodeP->data_buffer, strdup(retVal), OMADM_SYNCML_ERROR_DEVICE_FULL);
    nodeP->data_size = strlen(retVal) + 1;

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_devDetailGetACLFN(const char *iURI,
                                 char **oValue,
                                 void *iData)
{
    if (!strcmp(iURI, "./DevDetail"))
    {
        *oValue = strdup("Get=*");
    }
    else
    {
        *oValue = NULL;
    }

    return OMADM_SYNCML_ERROR_NONE;
}

omadm_mo_interface_t * omadm_get_mo_interface()
{
    omadm_mo_interface_t *retVal = NULL;

    retVal = malloc(sizeof(*retVal));
    if (retVal) {
        memset(retVal, 0, sizeof(*retVal));
        retVal->uri = strdup("./DevDetail");
        retVal->initFunc = prv_devDetailInitFN;
        retVal->closeFunc = prv_devDetailCloseFN;
        retVal->isNodeFunc = prv_devDetailIsNodeFN;
        retVal->getFunc = prv_devDetailGetFN;
        retVal->getACLFunc = prv_devDetailGetACLFN;
    }

    return retVal;
}
