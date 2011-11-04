/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file omadm_plugin_devdetails.c
 *
 * @brief C file for the devdetails plugin
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "error.h"
#include "error_macros.h"
#include "dmsettings.h"

#include "config.h"
#include "syncml_error.h"
#include "plugin_devdetails.h"

/* TODO: rewrite using mo_base */


static int prv_devDetailCreateFN(const char *iServerID, void **oData)
{
    *oData = NULL;
	return OMADM_SYNCML_ERROR_NONE;
}

static void prv_devDetailFreeFN(void *iData)
{
}

static int prv_devDetailNodeExistsFN(const char *iURI,
				     OMADM_NodeType * oNodeType,
				     void *oData)
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

static int prv_devDetailGetAccessRightsFN(const char *iURI,
					  OMADM_AccessRights *
					  oAccessRights, void *oData)
{
	*oAccessRights = OMADM_ACCESS_GET;
	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_devDetailGetNodeChildrenFN(const char *iURI,
					  dmc_ptr_array *oChildren,
					  void *oData)
{
	DMC_ERR_MANAGE;
	char *str = NULL;

	if (!strcmp(iURI, "./DevDetail")) {
		DMC_FAIL_NULL(str, strdup("./DevDetail/URI"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;

		DMC_FAIL_NULL(str, strdup("./DevDetail/DevTyp"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;

		DMC_FAIL_NULL(str, strdup("./DevDetail/OEM"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;

		DMC_FAIL_NULL(str, strdup("./DevDetail/FwV"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;

		DMC_FAIL_NULL(str, strdup("./DevDetail/SwV"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;

		DMC_FAIL_NULL(str, strdup("./DevDetail/HwV"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;

		DMC_FAIL_NULL(str, strdup("./DevDetail/LrgObj"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;
	} else if (!strcmp(iURI, "./DevDetail/URI")) {
		DMC_FAIL_NULL(str, strdup("./DevDetail/URI/MaxDepth"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;

		DMC_FAIL_NULL(str, strdup("./DevDetail/URI/MaxTotLen"),
			      OMADM_SYNCML_ERROR_DEVICE_FULL);
		DMC_FAIL_ERR(dmc_ptr_array_append(oChildren, str),
			       OMADM_SYNCML_ERROR_DEVICE_FULL);
		str = NULL;

		DMC_FAIL_NULL(str, strdup("./DevDetail/URI/MaxSegLen"),
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

static int prv_devDetailGetValueFN(const char *iURI, char **oValue,
				   void *oData)
{
	DMC_ERR_MANAGE;

	const char *retVal = NULL;
	char buffer[64];

	if (!strcmp(iURI, "./DevDetail/DevTyp"))
		retVal = "mobile";
	else if (!strcmp(iURI, "./DevDetail/OEM"))
		retVal = "TODO";
	else if (!strcmp(iURI, "./DevDetail/FwV"))
		retVal = "TODO";
	else if (!strcmp(iURI, "./DevDetail/SwV"))
		retVal = "TODO";
	else if (!strcmp(iURI, "./DevDetail/HwV"))
		retVal = "1.0";
	else if (!strcmp(iURI, "./DevDetail/LrgObj"))
		retVal = "true";
	else if (!strcmp(iURI, "./DevDetail/URI/MaxDepth")) {
		snprintf(buffer, 64, "%u", OMADM_DEVDETAILS_MAX_DEPTH_LEN);
		retVal = buffer;
	} else if (!strcmp(iURI, "./DevDetail/URI/MaxTotLen")) {
		snprintf(buffer, 64, "%u", OMADM_DEVDETAILS_MAX_TOT_LEN);
		retVal = buffer;
	} else if (!strcmp(iURI, "./DevDetail/URI/MaxSegLen")) {
		snprintf(buffer, 64, "%u", OMADM_DEVDETAILS_MAX_SEG_LEN);
		retVal = buffer;
	} else {
		DMC_FAIL(OMADM_SYNCML_ERROR_NOT_FOUND);
	}

	DMC_FAIL_NULL(*oValue, strdup(retVal),
		      OMADM_SYNCML_ERROR_DEVICE_FULL);

DMC_ON_ERR:

	return DMC_ERR;
}

static int prv_devDetailGetMetaFN(const char *iURI, const char *iProp,
					   char **oValue, void *oData)
{
	DMC_ERR_MANAGE;

	const char *retVal = NULL;
	char *str = NULL;
	char buffer[32];
	bool nonLeaf = (!strcmp(iURI, "./DevDetail")
			|| !strcmp(iURI, "./DevDetail/URI"));

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
		if (!strcmp(iURI, "./DevDetail"))
			retVal = "org.openmobilealliance.dm/1.0/DevDetail";
		else
			retVal = (nonLeaf) ? "null" : "text/plain";
	} else if (!strcmp(iProp, OMADM_NODE_PROPERTY_ACL)) {
		DMC_ERR = OMADM_SYNCML_ERROR_NOT_FOUND;
		goto DMC_ON_ERR;
	} else if (!strcmp(iProp, OMADM_NODE_PROPERTY_TITLE))
		retVal = "";
	else if (!strcmp(iProp, OMADM_NODE_PROPERTY_NAME))
		retVal = strrchr(iURI, '/') + 1;	/* This must succeed as we have already check URI */
	else if (!strcmp(iProp, OMADM_NODE_PROPERTY_SIZE)) {
		DMC_FAIL(prv_devDetailGetValueFN(iURI, &str, oData));
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

OMADM_DMTreePlugin *omadm_create_devdetails_plugin()
{
	OMADM_DMTreePlugin *retVal = NULL;

	retVal =
		(OMADM_DMTreePlugin *) malloc(sizeof(*retVal));
	if (retVal) {
		memset(retVal, 0, sizeof(OMADM_DMTreePlugin));
		retVal->create = prv_devDetailCreateFN;
		retVal->free = prv_devDetailFreeFN;
		retVal->nodeExists = prv_devDetailNodeExistsFN;
		retVal->getAccessRights = prv_devDetailGetAccessRightsFN;
		retVal->getNodeChildren = prv_devDetailGetNodeChildrenFN;
		retVal->getValue = prv_devDetailGetValueFN;
		retVal->getMeta = prv_devDetailGetMetaFN;
	}

	return retVal;
}
