/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file plugin_root.c
 *
 * @brief C file for the root plugin
 *
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "error.h"
#include "error_macros.h"

#include "dmsettings_utils.h"
#include "syncml_error.h"


static int prv_rootCreateFN(const char *serverID, void **oData)
{
    return dmsettings_open((dmsettings **)oData);
}

static void prv_rootFreeFN(void *iData)
{
    dmsettings_close((dmsettings *)iData);
}

static int prv_rootNodeExistsFN(const char *uri, OMADM_NodeType* node_type,
				void *data)
{
	DMC_ERR_MANAGE;
	dmsettings *settings = (dmsettings *) data;

	DMC_FAIL(omadm_dmsettings_utils_node_exists(settings, uri,
							 node_type));

	if ((*node_type == OMADM_NODE_NOT_EXIST) && !strcmp(uri,"."))
		*node_type = OMADM_NODE_IS_INTERIOR;

DMC_ON_ERR:

	return DMC_ERR;
}

static int prv_rootGetAccessRightsFN(const char *uri,
				     OMADM_AccessType *access_rights,
				     void *data)
{
	*access_rights = OMADM_ACCESS_ADD | OMADM_ACCESS_GET;

	if (strcmp(uri, "."))
		*access_rights |=
			(OMADM_ACCESS_REPLACE | OMADM_ACCESS_DELETE);

	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_rootGetNodeChildrenFN(const char *uri, dmc_ptr_array *children,
					void *data)
{
	dmsettings *settings = (dmsettings *) data;

	return omadm_dmsettings_utils_get_node_children(settings, uri,
							children);
}

static int prv_rootGetValueFN(const char *uri, char **value, void *data)
{
	dmsettings *settings = (dmsettings *) data;

	return omadm_dmsettings_utils_get_value(settings, uri, value);
}

static int prv_rootSetValueFN(const char *uri, const char *value,
			      void *data)
{
	dmsettings *settings = (dmsettings *) data;

	return omadm_dmsettings_utils_set_value(settings, uri, value);
}

static int prv_rootGetMetaFN(const char *uri, const char *prop,
			     char **value, void *data)
{
	DMC_ERR_MANAGE;
	dmsettings *settings = (dmsettings *) data;
	char* value_copy;

	DMC_ERR = omadm_dmsettings_utils_get_meta(settings,
							 uri, prop, value);
	if ((DMC_ERR == DMC_ERR_NOT_FOUND) &&
	    !strcmp(uri,".")) {
		if (!strcmp(prop,OMADM_NODE_PROPERTY_ACL)) {
			DMC_FAIL_NULL(value_copy, strdup("Add=*&Get=*"),
					   OMADM_SYNCML_ERROR_DEVICE_FULL);
			DMC_ERR = OMADM_SYNCML_ERROR_NONE;
			*value = value_copy;
		}
		else if (!strcmp(prop,OMADM_NODE_PROPERTY_FORMAT)) {
			DMC_FAIL_NULL(value_copy, strdup("node"),
					   OMADM_SYNCML_ERROR_DEVICE_FULL);
			DMC_ERR = OMADM_SYNCML_ERROR_NONE;
			*value = value_copy;
		}
		else if (!strcmp(prop,OMADM_NODE_PROPERTY_TYPE)) {
			DMC_FAIL_NULL(value_copy, strdup("null"),
					   OMADM_SYNCML_ERROR_DEVICE_FULL);
			DMC_ERR = OMADM_SYNCML_ERROR_NONE;
			*value = value_copy;
		}
	}

DMC_ON_ERR:

	return DMC_ERR;
}

static int prv_rootSetMetaFN(const char *uri, const char *prop,
				      const char *value, void *data)
{
	dmsettings *settings = (dmsettings *) data;

	return omadm_dmsettings_utils_set_meta(settings, uri, prop,
					       value);
}

static int prv_rootDeleteNodeFN(const char *uri, void *data)
{
	dmsettings *settings = (dmsettings *) data;

	return omadm_dmsettings_utils_delete_node(settings, uri);
}

static int prv_rootCreateNonLeafFN(const char *uri, void *data)
{
	dmsettings *settings = (dmsettings *) data;

	return omadm_dmsettings_utils_create_non_leaf(settings, uri);
}


OMADM_DMTreePlugin *omadm_create_root_plugin()
{
	OMADM_DMTreePlugin *retval = NULL;

	retval = malloc(sizeof(*retval));
	if (retval) {
		memset(retval, 0, sizeof(*retval));
		retval->create = prv_rootCreateFN;
		retval->free = prv_rootFreeFN;
		retval->nodeExists = prv_rootNodeExistsFN;
		retval->getAccessRights = prv_rootGetAccessRightsFN;
		retval->getNodeChildren = prv_rootGetNodeChildrenFN;
		retval->getValue = prv_rootGetValueFN;
		retval->setValue = prv_rootSetValueFN;
		retval->getMeta = prv_rootGetMetaFN;
		retval->setMeta = prv_rootSetMetaFN;
		retval->deleteNode = prv_rootDeleteNodeFN;
		retval->createNonLeaf = prv_rootCreateNonLeafFN;
		retval->supportTransactions = true;
	}

	return retval;
}
