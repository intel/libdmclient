/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file plugin_devinfo.c
 *
 * @brief C file for the devinfo test plugin
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <omadmtree_mo.h>

#include "error_macros.h"

#include "config.h"
#include "syncml_error.h"


/* TODO: rewrite using mo_base */


static int prv_devInfoInitFN(void **oData)
{
    *oData = NULL;
    return OMADM_SYNCML_ERROR_NONE;
}

static void prv_devInfoCloseFN(void *iData)
{
}

static int prv_devInfoIsNodeFN(const char *iURI,
                               omadmtree_node_type_t *oNodeType,
                               void *iData)
{
    if (!strcmp(iURI, "./DevInfo"))
        *oNodeType = OMADM_NODE_IS_INTERIOR;
    else if (!strcmp(iURI, "./DevInfo/DevId")
         || !strcmp(iURI, "./DevInfo/Man")
         || !strcmp(iURI, "./DevInfo/Mod")
         || !strcmp(iURI, "./DevInfo/DmV")
         || !strcmp(iURI, "./DevInfo/Lang"))
        *oNodeType = OMADM_NODE_IS_LEAF;
    else
        *oNodeType = OMADM_NODE_NOT_EXIST;

    return OMADM_SYNCML_ERROR_NONE;
}

static int prv_devInfoGetFN(dmtree_node_t * nodeP,
                            void *iData)
{
    DMC_ERR_MANAGE;

    const char *retVal = NULL;

    nodeP->format = NULL;
    nodeP->type = NULL;
    nodeP->data_size = 0;
    nodeP->data_buffer = NULL;

    if (!strcmp(nodeP->uri, "./DevInfo"))
    {
        nodeP->format = strdup("node");
        nodeP->type = strdup("urn:oma:mo:oma-dm-devinfo:1.0");
        retVal = "DevId/Man/Mod/DmV/Lang";
    }
    else
    {
        DMC_FAIL_NULL(nodeP->format, strdup("chr"), OMADM_SYNCML_ERROR_DEVICE_FULL);
        DMC_FAIL_NULL(nodeP->type, strdup("text/plain"), OMADM_SYNCML_ERROR_DEVICE_FULL);
        if (!strcmp(nodeP->uri, "./DevInfo/DevId"))
            retVal = "TODO";
        else if (!strcmp(nodeP->uri, "./DevInfo/Man"))
            retVal = "TODO";
        else if (!strcmp(nodeP->uri, "./DevInfo/Mod"))
            retVal = "TODO";
        else if (!strcmp(nodeP->uri, "./DevInfo/DmV"))
            retVal = "1.0";
        else if (!strcmp(nodeP->uri, "./DevInfo/Lang"))
            retVal = "TODO";
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

static int prv_devInfoGetACLFN(const char *iURI,
                               char **oValue,
                               void *iData)
{
    if (!strcmp(iURI, "./DevInfo"))
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
        retVal->uri = strdup("./DevInfo/");
        retVal->initFunc = prv_devInfoInitFN;
        retVal->closeFunc = prv_devInfoCloseFN;
        retVal->isNodeFunc = prv_devInfoIsNodeFN;
        retVal->getFunc = prv_devInfoGetFN;
        retVal->getACLFunc = prv_devInfoGetACLFN;
    }

    return retVal;
}
