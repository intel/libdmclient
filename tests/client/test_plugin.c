/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file test_plugin.c
 *
 * @brief C file for a test plugin
 *
 */

#include <stdlib.h>
#include <string.h>
#include <omadmtree_mo.h>

#include "config.h"
#include "syncml_error.h"

static int prv_testInitFN(void **oData)
{
    *oData = NULL;
	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_testIsNodeFN(const char *iURI,
			                omadmtree_node_type_t *oNodeType,
				            void *iData)
{
	*oNodeType = (strcmp(iURI, "./test") == 0) ? OMADM_NODE_IS_INTERIOR : OMADM_NODE_NOT_EXIST;
	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_testGetACLFN(const char *iURI,
                             char **oValue,
                             void *iData)
{
	if (!strcmp(iURI, "./test"))
	{
	    *oValue = strdup("Get=*");
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


omadm_mo_interface_t * test_get_mo_interface()
{
	omadm_mo_interface_t *retVal = NULL;

	retVal = malloc(sizeof(*retVal));
	if (retVal) {
		memset(retVal, 0, sizeof(*retVal));
		retVal->base_uri = strdup("./test");
		retVal->initFunc = prv_testInitFN;
		retVal->isNodeFunc = prv_testIsNodeFN;
		retVal->getACLFunc = prv_testGetACLFN;
	}

	return retVal;
}
