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

static int prv_inboxInitFN(void **oData)
{
    *oData = NULL;
	return OMADM_SYNCML_ERROR_NONE;
}

static int prv_inboxIsNodeFN(const char *iURI,
			                 omadmtree_node_kind_t *oNodeType,
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
