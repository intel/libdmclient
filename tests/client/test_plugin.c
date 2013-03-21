/*
 * libdmclient test materials
 *
 * Copyright (C) 2012 Intel Corporation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * David Navarro <david.navarro@intel.com>
 *
 */

#include <stdlib.h>
#include <string.h>
#include <omadmtree_mo.h>

#include "syncml_error.h"

static int prv_testInitFN(void **oData)
{
    *oData = NULL;
	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_testIsNodeFN(const char *iURI,
			                omadmtree_node_kind_t *oNodeType,
				            void *iData)
{
	*oNodeType = ((strcmp(iURI, "./test") == 0) || (strcmp(iURI, "./test/execute") == 0)) ? OMADM_NODE_IS_INTERIOR : OMADM_NODE_NOT_EXIST;
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
	else if (!strcmp(iURI, "./test/execute"))
	{
	    *oValue = strdup("Exec=*");
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

static int prv_testGetFN(dmtree_node_t * nodeP,
						 void *iData)
{
	if (!strcmp(nodeP->uri, "./test"))
	{
		nodeP->format = strdup("node");
		if (!nodeP->format)
		{
			return OMADM_SYNCML_ERROR_DEVICE_FULL;
		}
		nodeP->data_buffer = strdup("execute");
		if (!nodeP->data_buffer)
		{
			free(nodeP->format);
			return OMADM_SYNCML_ERROR_DEVICE_FULL;
		}
		nodeP->data_size = strlen(nodeP->data_buffer);
	}
    else
    {
        return OMADM_SYNCML_ERROR_NOT_FOUND;
    }

    return OMADM_SYNCML_ERROR_NONE;
}

static int prv_testExecFN(const char * iURI,
						  const char * cmdData,
						  const char * correlator,
						  void * data)
{
	if (strcmp(iURI, "./test/execute") != 0)
        return OMADM_SYNCML_ERROR_NOT_FOUND;

	return OMADM_SYNCML_ERROR_NONE;
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
		retVal->getFunc = prv_testGetFN;
		retVal->getACLFunc = prv_testGetACLFN;
		retVal->execFunc = prv_testExecFN;
	}

	return retVal;
}
