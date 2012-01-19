/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file plugin_inbox.c
 *
 * @brief C file for the inbox test plugin
 *
 */

#include <stdlib.h>
#include <string.h>
#include <omadmtree_mo.h>

#include "config.h"
#include "syncml_error.h"

static int prv_inboxInitFN(void **oData)
{
    *oData = NULL;
	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_inboxIsNodeFN(const char *iURI,
			                 omadmtree_node_type_t *oNodeType,
				             void *iData)
{
	*oNodeType = (strcmp(iURI, "./Inbox") == 0) ? OMADM_NODE_IS_INTERIOR : OMADM_NODE_NOT_EXIST;
	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_findURNFN(const char *iURN,
                         char ***oURL,
                         void *iData)
{
    if (!strcmp(iURN, "urn:oma:mo:oma-dm-inbox:1.0"))
    {
        *oURL = (char **)malloc(2*sizeof(char*));
        if (*oURL)
        {
            (*oURL)[0] = strdup("./Inbox");
            (*oURL)[1] = NULL;
            return OMADM_SYNCML_ERROR_NONE;
        }
        return OMADM_SYNCML_ERROR_DEVICE_FULL;
    }
    return OMADM_SYNCML_ERROR_NOT_FOUND;
}

static int prv_inboxSetFN(const dmtree_node_t * nodeP,
                          void * data)
{
    return OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED;
}

static int prv_inboxGetACLFN(const char *iURI,
                             char **oValue,
                             void *iData)
{
	if (!strcmp(iURI, "./Inbox"))
	{
	    *oValue = strdup("Add=*");
	    if (*oValue)
	        return OMADM_SYNCML_ERROR_NONE;
        return OMADM_SYNCML_ERROR_DEVICE_FULL;
    }
    else
    {
        *oValue = NULL;
        return OMADM_SYNCML_ERROR_NOT_FOUND;
    }
}


omadm_mo_interface_t * omadm_get_mo_interface()
{
	omadm_mo_interface_t *retVal = NULL;

	retVal = malloc(sizeof(*retVal));
	if (retVal) {
		memset(retVal, 0, sizeof(*retVal));
		retVal->base_uri = strdup("./Inbox");
		retVal->initFunc = prv_inboxInitFN;
		retVal->isNodeFunc = prv_inboxIsNodeFN;
        retVal->findURNFunc = prv_findURNFN;
		retVal->setFunc = prv_inboxSetFN;
		retVal->getACLFunc = prv_inboxGetACLFN;
	}

	return retVal;
}
