/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file plugin_inbox.c
 *
 * @brief C file for the inbox plugin
 *
 */

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "syncml_error.h"

#include "plugin_inbox.h"

static int prv_inboxCreateFN(const char *iServerID, void **oData)
{
    *oData = NULL;
	return OMADM_SYNCML_ERROR_NONE;
}

static void prv_inboxFreeFN(void *iData)
{
}

static int prv_inboxNodeExistsFN(const char *iURI,
					  OMADM_NodeType * oNodeType,
					  void *iData)
{
	*oNodeType =
	    (strcmp(iURI, "./Inbox") ==
	     0) ? OMADM_NODE_IS_INTERIOR : OMADM_NODE_NOT_EXIST;
	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_inboxGetAccessRightsFN(const char *iURI,
					       OMADM_AccessRights *
					       oAccessRights, void *iData)
{
	*oAccessRights = OMADM_ACCESS_ADD;
	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_inboxSetValueFN(const char *iURI, const char *iValue,
					void *oData)
{
	return OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED;
}

static int prv_inboxSetMetaFN(const char *iURI, const char *iProp,
				       const char *iValue, void *oData)
{
	return OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED;
}

OMADM_DMTreePlugin *omadm_create_inbox_plugin()
{
	OMADM_DMTreePlugin *retVal = NULL;

	retVal =  malloc(sizeof(*retVal));
	if (retVal) {
		memset(retVal, 0, sizeof(*retVal));
		retVal->create = prv_inboxCreateFN;
		retVal->free = prv_inboxFreeFN;
		retVal->nodeExists = prv_inboxNodeExistsFN;
		retVal->getAccessRights = prv_inboxGetAccessRightsFN;
		retVal->setValue = prv_inboxSetValueFN;
		retVal->setMeta = prv_inboxSetMetaFN;
		retVal->supportTransactions = true;
	}

	return retVal;
}
