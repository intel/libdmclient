/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007, ACCESS Systems Americas, Inc. All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file <defaultroot.c>
 *
 * @brief Default Root MO
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "dmtree_plugin.h"
#include "syncml_error.h"

static int prv_create(const char *unused, void ** dataP)
{
    *dataP = NULL;
    return OMADM_SYNCML_ERROR_NONE;
}

static void prv_free(void *unused)
{
}

static int prv_nodeExists(const char *uri, OMADM_NodeType* node_type, void *data)
{
    if (!strcmp(uri,"."))
    {
		*node_type = OMADM_NODE_IS_INTERIOR;
	}
	else
	{
	    *node_type = OMADM_NODE_NOT_EXIST;
	}

	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_getMeta(const char *uri, const char *prop, char **value, void *data)
{
    *value = NULL;

    if (strcmp(uri,"."))
    {
        return OMADM_SYNCML_ERROR_NOT_FOUND;
    }

	if (!strcmp(prop,OMADM_NODE_PROPERTY_ACL))
	{
		*value = strdup("Get=*");
	}
	else if (!strcmp(prop,OMADM_NODE_PROPERTY_FORMAT))
	{
		*value = strdup("node");
	}
	else if (!strcmp(prop,OMADM_NODE_PROPERTY_TYPE))
	{
		*value = strdup("null");
	}

	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_getAccessRights(const char *uri, OMADM_AccessType *access_rights, void *data)
{
    if (strcmp(uri,"."))
    {
        return OMADM_SYNCML_ERROR_NOT_FOUND;
    }

	*access_rights = OMADM_ACCESS_GET;

	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_getNodeChildren(const char *iURI,
					           dmc_ptr_array *oChildren,
					           void *oData)
{
    if (strcmp(iURI, "."))
    {
        return OMADM_SYNCML_ERROR_NOT_FOUND;
    }

	return OMADM_SYNCML_ERROR_NONE;
}

OMADM_DMTreePlugin * getDefaultRootPlugin()
{
	OMADM_DMTreePlugin *retval = NULL;

	retval = malloc(sizeof(*retval));
	if (retval) {
		memset(retval, 0, sizeof(*retval));
		retval->create = prv_create;
		retval->free = prv_free;
		retval->nodeExists = prv_nodeExists;
		retval->getAccessRights = prv_getAccessRights;
		retval->getMeta = prv_getMeta;
		retval->getNodeChildren = prv_getNodeChildren;
		retval->supportTransactions = false;
	}

	return retval;
}
