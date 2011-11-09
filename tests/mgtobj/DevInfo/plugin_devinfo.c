/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file plugin_devinfo.c
 *
 * @brief C file for the devinfo plugin
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "error_macros.h"

#include "config.h"
#include "syncml_error.h"
#include "plugin_devinfo.h"

/* TODO: rewrite using mo_base */


static int prv_devInfoCreateFN(const char *iServerID, void **oData)
{
	*oData = NULL;
	return OMADM_SYNCML_ERROR_NONE;
}

static void prv_devInfoFreeFN(void *iData)
{
}

static int prv_devInfoNodeExistsFN(const char *iURI,
				   OMADM_NodeType * oNodeType, void *oData)
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

static int prv_devInfoGetAccessRightsFN(const char *iURI,
					OMADM_AccessRights *oAccessRights,
					void *oData)
{
	*oAccessRights = OMADM_ACCESS_GET;
	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_devInfoGetNodeChildrenFN(const char *iURI,
					dmc_ptr_array *oChildren,
					void *oData)
{
	DMC_ERR_MANAGE;
	char *str = NULL;

	if (!strcmp(iURI, "./DevInfo")) {
		DMC_FAIL_NULL(str, strdup("./DevInfo/DevId"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;

		DMC_FAIL_NULL(str, strdup("./DevInfo/Man"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;

		DMC_FAIL_NULL(str, strdup("./DevInfo/Mod"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;

		DMC_FAIL_NULL(str, strdup("./DevInfo/DmV"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;

		DMC_FAIL_NULL(str, strdup("./DevInfo/Lang"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;
	}

DMC_ON_ERR:

	if (str)
		free(str);


	return DMC_ERR;
}

static int prv_devInfoGetValueFN(const char *iURI, char **oValue,
				 void *oData)
{
	DMC_ERR_MANAGE;

	const char *retVal = NULL;

	if (!strcmp(iURI, "./DevInfo/DevId"))
		retVal = "TODO";
	else if (!strcmp(iURI, "./DevInfo/Man"))
		retVal = "TODO";
	else if (!strcmp(iURI, "./DevInfo/Mod"))
		retVal = "TODO";
	else if (!strcmp(iURI, "./DevInfo/DmV"))
		retVal = "1.0";
	else if (!strcmp(iURI, "./DevInfo/Lang"))
		retVal = "TODO";
	else {
		DMC_FAIL(OMADM_SYNCML_ERROR_NOT_FOUND);
	}

	DMC_FAIL_NULL(*oValue, strdup(retVal),
		      OMADM_SYNCML_ERROR_DEVICE_FULL);

DMC_ON_ERR:

	return DMC_ERR;
}

static int prv_devInfoGetMetaFN(const char *iURI, const char *iProp,
				char **oValue, void *oData)
{
	DMC_ERR_MANAGE;

	const char *retVal = NULL;
	char *str = NULL;
	char buffer[32];
	bool nonLeaf = !strcmp(iURI, "./DevInfo");

	if (!strcmp(iProp, OMADM_NODE_PROPERTY_VERSION)) {
		if (nonLeaf) {
			DMC_ERR = OMADM_SYNCML_ERROR_NOT_FOUND;
			goto DMC_ON_ERR;
		} else
			retVal = "1.0";
	} else if (!strcmp(iProp, OMADM_NODE_PROPERTY_TIMESTAMP)) {
		if (nonLeaf) {
			DMC_ERR = OMADM_SYNCML_ERROR_NOT_FOUND;
			goto DMC_ON_ERR;
		} else
			retVal = "0";
	} else if (!strcmp(iProp, OMADM_NODE_PROPERTY_FORMAT))
		retVal = (nonLeaf) ? "node" : "chr";
	else if (!strcmp(iProp, OMADM_NODE_PROPERTY_TYPE)) {
		if (!strcmp(iURI, "./DevInfo"))
			retVal = "org.openmobilealliance.dm/1.0/DevInfo";
		else
			retVal = (nonLeaf) ? "null" : "text/plain";
	} else if (!strcmp(iProp, OMADM_NODE_PROPERTY_ACL)) {
		DMC_ERR = OMADM_SYNCML_ERROR_NOT_FOUND;
		goto DMC_ON_ERR;
	} else if (!strcmp(iProp, OMADM_NODE_PROPERTY_TITLE))
		retVal = "";
	else if (!strcmp(iProp, OMADM_NODE_PROPERTY_NAME))
		retVal = strrchr(iURI, '/') + 1;	/* This must succeed as we have already checked URI */
	else if (!strcmp(iProp, OMADM_NODE_PROPERTY_SIZE)) {
		DMC_FAIL(prv_devInfoGetValueFN(iURI, &str, oData));
		sprintf(buffer, "%d", strlen(str));
		free(str);
		retVal = buffer;
	} else {
		DMC_ERR = OMADM_SYNCML_ERROR_NOT_FOUND;
		goto DMC_ON_ERR;
	}

	DMC_FAIL_NULL(*oValue, strdup(retVal),
		      OMADM_SYNCML_ERROR_DEVICE_FULL);

DMC_ON_ERR:

	return DMC_ERR;

}

OMADM_DMTreePlugin *omadm_create_devinfo_plugin()
{
	OMADM_DMTreePlugin *retVal = NULL;

	retVal = malloc(sizeof(*retVal));
	if (retVal) {
		memset(retVal, 0, sizeof(*retVal));
		retVal->create = prv_devInfoCreateFN;
		retVal->free = prv_devInfoFreeFN;
		retVal->nodeExists = prv_devInfoNodeExistsFN;
		retVal->getAccessRights = prv_devInfoGetAccessRightsFN;
		retVal->getNodeChildren = prv_devInfoGetNodeChildrenFN;
		retVal->getValue = prv_devInfoGetValueFN;
		retVal->getMeta = prv_devInfoGetMetaFN;
	}

	return retVal;
}

OMADM_PluginDesc * omadm_get_plugin_desc(void)
{
    OMADM_PluginDesc *plugin_desc;

    plugin_desc = (OMADM_PluginDesc *)malloc(sizeof(OMADM_PluginDesc));
    if (plugin_desc)
    {
        plugin_desc->uri = strdup("./DevInfo/");
        plugin_desc->createFunc = omadm_create_devinfo_plugin;
    }

    return plugin_desc;
}
