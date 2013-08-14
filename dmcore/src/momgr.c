/*
 * libdmclient
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

/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007, ACCESS Systems Americas, Inc. All rights reserved.
 *****************************************************************************/

/*!
 * @file <momgr.c>
 *
 * @brief Management Object management code
 *
 * This file is based on the ACCESS source file omadm_dmtree.c.  All
 * identifiers have been renamed by Intel to match the coding standards of the
 * libdmclient.  In addition, the algorithms the functions have been modified to
 * conform to the libdmclient architecture.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "error_macros.h"
#include "log.h"

#include "syncml_error.h"
#include "momgr.h"
#include "internals.h"

#define URN_MO_DMACC     "urn:oma:mo:oma-dm-dmacc:1.0"
#define URN_MO_DEVDETAIL "urn:oma:mo:oma-dm-devdetail:1.0"
#define URN_MO_DEVINFO   "urn:oma:mo:oma-dm-devinfo:1.0"

#define URI(plug) (plug)->interface->base_uri


// defined in defaultroot.c
omadm_mo_interface_t * getDefaultRootPlugin();

static int prv_get_number(char * string,
                          unsigned int length)
{
    int result = 0;
    int mul = 0;
    unsigned int i = 0;

    while (i < length)
    {
        if ('0' <= string[i] && string[i] <= '9')
        {
            if (0 == mul)
            {
                mul = 10;
            }
            else
            {
                result *= mul;
            }
            result += string[i] - '0';
        }
        else
        {
            return -1;
        }
        i++;
    }

    return result;
}

static int prv_get_short(const dmtree_plugin_t *plugin,
                         char * iURI,
                         uint16_t * resultP)
{
    int error;
    dmtree_node_t node;

    memset(&node, 0, sizeof(node));
    node.uri = iURI;

    error = plugin->interface->getFunc(&node, plugin->data);
    if (OMADM_SYNCML_ERROR_NONE == error)
    {
        int number;

        number = prv_get_number(node.data_buffer, node.data_size);
        if (number < 0 || number > 0xFFFF)
        {
            error = OMADM_SYNCML_ERROR_SESSION_INTERNAL;
        }
        *resultP = (uint16_t)number;
    }
    // do not free iURI
    node.uri = NULL;
    dmtree_node_clean(&node, true);

    return error;
}

static void prv_freePlugin(dmtree_plugin_t *iPlugin)
{
    if (iPlugin->interface) {
        if (iPlugin->interface->closeFunc)
        {
            iPlugin->interface->closeFunc(iPlugin->data);
        }
        if (URI(iPlugin)) free(URI(iPlugin));
        free(iPlugin->interface);
    }

    free(iPlugin);
}

static void prv_freeMoDir(mo_dir_t * origin)
{
    mo_dir_t * child;

    child = origin->children;
    while (NULL != child)
    {
        child->parent = NULL;
        prv_freeMoDir(child);
        child = child->next;
    }
    if (NULL != origin->plugin)
    {
        prv_freePlugin(origin->plugin);
    }
    if (NULL != origin->name)
    {
        free(origin->name);
    }
    if (NULL != origin->parent)
    {
        if (origin->parent->children == origin)
        {
            origin->parent->children = origin->next;
        }
        else
        {
            child = origin->parent->children;
            while (NULL != child && child->next != origin)
            {
                child = child->next;
            }
            if (NULL != child)
            {
                child->next = origin->next;
            }
        }
    }
    free(origin);
}

static mo_dir_t * prv_findMoDir(mo_dir_t * origin,
                                const char *iURI,
                                bool exact)
{
    char * name = NULL;
    char * nextName;
    mo_dir_t * child;
    mo_dir_t * result;

    nextName = strchr(iURI, '/');
    if (nextName)
    {
        name = (char*)malloc(nextName - iURI + 1);
        if (NULL == name) return NULL;
        strncpy(name, iURI, nextName - iURI);
        name[nextName - iURI] = 0;
        nextName++;
    }
    else
    {
        name = (char *)iURI;
    }

    child = origin->children;
    while(child && strcmp(child->name, name))
    {
        child = child->next;
    }

    if (NULL != child)
    {
        if (NULL != nextName)
        {
            result = prv_findMoDir(child, nextName, exact);
        }
        else
        {
            result = child;
        }
    }
    else
    {
        if (false == exact)
        {
            result = origin;
        }
        else
        {
            result = NULL;
        }
    }

    if (NULL != nextName) free(name);
    return result;
}

static dmtree_plugin_t * prv_findPlugin(mo_mgr_t *iMgr,
                                        const char *iURI)
{
    char * uriCopy = NULL;
    char * subUri;
    mo_dir_t * dirP;

    if (iMgr->last_uri)
    {
        if (!strcmp(iURI, iMgr->last_uri))
        {
            return iMgr->last_plugin;
        }
        free (iMgr->last_uri);
        iMgr->last_uri = NULL;
    }

    if (!strcmp(iURI, "."))
    {
        return iMgr->root->plugin;
    }

    uriCopy = strdup(iURI);
    if (NULL == uriCopy)
    {
        return NULL;
    }

    subUri = uriCopy;
    if ('.' == subUri[0])
    {
        if ('/' == subUri[1])
        {
            subUri += 2;
        }
        else if (0 == subUri[1])
        {
            // invalid URI
            free(uriCopy);
            return NULL;
        }
    }

    dirP = prv_findMoDir(iMgr->root, subUri, false);
    free(uriCopy);

    while (NULL != dirP && NULL == dirP->plugin)
    {
        dirP = dirP->parent;
    }

    if (NULL != dirP)
    {
        // no need to check return of strdup. Failure will be tested on next call.
        iMgr->last_uri = strdup(iURI);
        iMgr->last_plugin = dirP->plugin;
        return dirP->plugin;
    }

    return NULL;
}

static int prv_createMoDir(mo_mgr_t * iMgr,
                           const char * uri,
                           mo_dir_t ** elemP)
{
    DMC_ERR_MANAGE;
    char * uriCopy = NULL;
    char * curNode;
    mo_dir_t * pointer;
    mo_dir_t * child = NULL;

    *elemP = NULL;
    DMC_FAIL_NULL(uriCopy, strdup(uri), OMADM_SYNCML_ERROR_DEVICE_FULL);
    pointer = iMgr->root;

    curNode = uriCopy;
    if ('.' == curNode[0])
    {
        if ('/' == curNode[1])
        {
            curNode += 2;
        }
        else if (0 == curNode[1])
        {
            // we can not have a root plugin
            DMC_FAIL(OMADM_SYNCML_ERROR_NOT_ALLOWED);
        }
    }
    while (curNode && 0 != *curNode)
    {
        char * nextNode;

        nextNode = strchr(curNode, '/');
        if (nextNode)
        {
            *nextNode = 0;
            nextNode++;
        }
        // find children or create one
        child = pointer->children;
        while(child
           && strcmp(child->name, curNode))
        {
            child = child->next;
        }
        if (NULL == child)
        {
            DMC_FAIL_NULL(child, malloc(sizeof(mo_dir_t)), OMADM_SYNCML_ERROR_DEVICE_FULL);
            memset(child, 0, sizeof(mo_dir_t));
            DMC_FAIL_NULL(child->name, strdup(curNode), OMADM_SYNCML_ERROR_DEVICE_FULL);
            child->parent = pointer;
            child->next = pointer->children;
            pointer->children = child;
        }
        pointer = child;
        child = NULL;
        curNode = nextNode;
    }

    // pointer is on the right one
    *elemP = pointer;

DMC_ON_ERR:
    if (uriCopy) free(uriCopy);
    if (child)
    {
        if (child->name) free(child->name);
        free(child);
    }

    return DMC_ERR;
}

static void prv_findUrn(mo_dir_t * dirP,
                        const char * iUrn,
                        char *** listP)
{
    mo_dir_t * child;

    if (NULL != dirP->plugin)
    {
        if (NULL != dirP->plugin->interface->findURNFunc)
        {
            char ** result;

            if (OMADM_SYNCML_ERROR_NONE
             == dirP->plugin->interface->findURNFunc(iUrn,
                                                     &result,
                                                     dirP->plugin->data))
            {
                char ** tmp;
                tmp = strArray_concat((const char**)*listP, (const char**)result);
                free(*listP);
                free(result);
                *listP = tmp;
            }
        }
    }

    child = dirP->children;
    while (NULL != child)
    {
        prv_findUrn(child, iUrn, listP);
        child = child->next;
    }
}

static void prv_getChildrenUrl(mo_dir_t * dirP,
                               const char * baseUri,
                               char *** listP)
{
    *listP = NULL;
    if (NULL != dirP->plugin)
    {
        if (NULL != dirP->plugin->interface->getFunc)
        {
            dmtree_node_t node;

            memset(&node, 0, sizeof(dmtree_node_t));
            node.uri = strdup(baseUri);

            if (OMADM_SYNCML_ERROR_NONE
             == dirP->plugin->interface->getFunc(&node,
                                                 dirP->plugin->data))
            {
                if (0 != node.data_size)
                {
                    *listP = strArray_buildChildList(baseUri, node.data_buffer, node.data_size);
                    dmtree_node_clean(&node, false);
                }
            }
        }
    }
}

mo_mgr_t * momgr_init()
{
    mo_mgr_t * mgrP = NULL;

    mgrP = (mo_mgr_t *)malloc(sizeof(mo_mgr_t));
    if (NULL == mgrP) return NULL;

    memset(mgrP, 0, sizeof(mo_mgr_t));

    // set the root plugin
    mgrP->root = (mo_dir_t *) malloc(sizeof(mo_dir_t));
    if (NULL == mgrP->root)
    {
        momgr_free(mgrP);
        return NULL;
    }
    memset(mgrP->root, 0, sizeof(mo_dir_t));
    mgrP->root->name = strdup(".");
    if (NULL == mgrP->root->name)
    {
        momgr_free(mgrP);
        return NULL;
    }

    mgrP->root->plugin = (dmtree_plugin_t *) malloc(sizeof(dmtree_plugin_t));
    if (NULL == mgrP->root->plugin)
    {
        momgr_free(mgrP);
        return NULL;
    }
    memset(mgrP->root->plugin, 0, sizeof(dmtree_plugin_t));

    mgrP->root->plugin->interface = getDefaultRootPlugin();
    mgrP->root->plugin->container = mgrP->root;

    return mgrP;
}

void momgr_free(mo_mgr_t * iMgrP)
{
    if (iMgrP->root)
    {
        prv_freeMoDir(iMgrP->root);
    }
    if (iMgrP->last_uri) free(iMgrP->last_uri);
    free(iMgrP);
}

int momgr_check_mandatory_mo(mo_mgr_t * iMgrP)
{
	int error = OMADM_SYNCML_ERROR_NONE;
	char ** urlList = NULL;
    dmtree_plugin_t * pluginP;

    prv_findUrn(iMgrP->root, URN_MO_DMACC, &urlList);
    if (NULL == urlList)
    {
        // missing DMAcc MO
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;
    }
    strArray_free(urlList);

    pluginP = prv_findPlugin(iMgrP, "./DevInfo");
    if (NULL == pluginP)
    {
        // missing DevInfo MO
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;
    }

    pluginP = prv_findPlugin(iMgrP, "./DevDetail");
    if (NULL == pluginP)
    {
        // missing DevDetail MO
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;
    }

	if (pluginP->interface->getFunc)
	{
		error = prv_get_short(pluginP, "./DevDetail/URI/MaxDepth", &(iMgrP->max_depth));
		if (OMADM_SYNCML_ERROR_NONE == error)
		{
			error = prv_get_short(pluginP, "./DevDetail/URI/MaxTotLen", &(iMgrP->max_total_len));
			if (OMADM_SYNCML_ERROR_NONE == error)
			{
				error = prv_get_short(pluginP, "./DevDetail/URI/MaxSegLen", &(iMgrP->max_segment_len));
			}
		}
	}
	else
	{
		return OMADM_SYNCML_ERROR_SESSION_INTERNAL;
	}

	return error;
}

int momgr_add_plugin(mo_mgr_t * iMgr,
                     omadm_mo_interface_t *iPlugin)
{
    DMC_ERR_MANAGE;
    mo_dir_t * newElem = NULL;
    void * pluginData = NULL;
    uint8_t uriOffset = 0;

    DMC_LOGF("uri <%s>", iPlugin->base_uri);

    DMC_FAIL_ERR(NULL == iPlugin, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == iPlugin->initFunc, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL(iPlugin->initFunc(&pluginData));

    if (iPlugin->base_uri)
    {
        if ('.' == iPlugin->base_uri[0])
        {
            switch(iPlugin->base_uri[1])
            {
            case 0:
                // we can not have a root plugin
                DMC_FAIL(OMADM_SYNCML_ERROR_NOT_ALLOWED);
                break;
            case '/':
                uriOffset = 2;
                break;
            default:
                break;
            }
        }
        DMC_FAIL(uri_validate_path(iPlugin->base_uri + uriOffset, iMgr->max_depth, iMgr->max_segment_len));

        // Find where to put the new plugin
        DMC_FAIL(prv_createMoDir(iMgr, iPlugin->base_uri, &newElem));

        DMC_FAIL_NULL(newElem->plugin, (dmtree_plugin_t *) malloc(sizeof(dmtree_plugin_t)),
                  OMADM_SYNCML_ERROR_DEVICE_FULL);

        memset(newElem->plugin, 0, sizeof(dmtree_plugin_t));

        newElem->plugin->interface = iPlugin;
        newElem->plugin->data = pluginData;
        newElem->plugin->container = newElem;

        newElem = NULL;
    }

DMC_ON_ERR:

    if (newElem)
    {
        prv_freeMoDir(newElem);
    }

    DMC_LOGF("exit <0x%x>", DMC_ERR);

    return DMC_ERR;
}

int momgr_exists(mo_mgr_t * iMgr,
                 const char *iURI,
                 omadmtree_node_kind_t * oExists)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_LOGF("momgr_node_exists <%s>", iURI);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->isNodeFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);
    DMC_ERR = plugin->interface->isNodeFunc(iURI, oExists, plugin->data);
    // handle nested plugins
    if ((OMADM_SYNCML_ERROR_NONE == DMC_ERR && OMADM_NODE_NOT_EXIST == *oExists)
     || OMADM_SYNCML_ERROR_NOT_FOUND == DMC_ERR)
    {
        char * subUri;
        mo_dir_t * dirP;

        subUri = (char *)iURI;
        if (('.' == subUri[0]) && ('/' == subUri[1]))
        {
            subUri += 2;
        }
        dirP = prv_findMoDir(iMgr->root, subUri, true);

        if (dirP)
        {
            *oExists = OMADM_NODE_IS_INTERIOR;
            DMC_ERR = OMADM_SYNCML_ERROR_NONE;
        }
        else
        {
            *oExists = OMADM_NODE_NOT_EXIST;
        }
    }
    DMC_LOGF("momgr_node_exists exit <0x%x> %d", DMC_ERR, *oExists);

DMC_ON_ERR:

    return DMC_ERR;
}

int momgr_get_value(mo_mgr_t * iMgr,
                    dmtree_node_t * nodeP)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == nodeP, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_get_value <%s>", nodeP->uri);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, nodeP->uri),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->getFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_ERR = plugin->interface->getFunc(nodeP, plugin->data);

    // handle nested plugins
    if ((OMADM_SYNCML_ERROR_NONE == DMC_ERR && 0 == strcmp(nodeP->format, "node"))
     || OMADM_SYNCML_ERROR_NOT_FOUND == DMC_ERR)
    {
        char * subUri;
        mo_dir_t * dirP;

        subUri = nodeP->uri;
        if (('.' == subUri[0]) && ('/' == subUri[1]))
        {
            subUri += 2;
        }
        dirP = prv_findMoDir(iMgr->root, subUri, false);

        if (dirP)
        {
            mo_dir_t * child;

            child = dirP->children;
            while (NULL != child)
            {
                if (NULL == nodeP->data_buffer)
                {
                    DMC_ERR = OMADM_SYNCML_ERROR_NONE;
                    DMC_FAIL_NULL(nodeP->format, strdup("node"), OMADM_SYNCML_ERROR_DEVICE_FULL);
                    DMC_FAIL_NULL(nodeP->data_buffer, strdup(child->name), OMADM_SYNCML_ERROR_DEVICE_FULL);
                }
                else
                {
                    char * tmp_str;

                    DMC_FAIL_NULL(tmp_str, str_cat_3(nodeP->data_buffer, "/", child->name), OMADM_SYNCML_ERROR_DEVICE_FULL);
                    free(nodeP->data_buffer);
                    nodeP->data_buffer = tmp_str;
                }
                child = child->next;
            }
            if (NULL != dirP->children && NULL != nodeP->data_buffer)
            {
                nodeP->data_size = strlen(nodeP->data_buffer);
            }
        }
    }

DMC_ON_ERR:

    DMC_LOGF("momgr_get_value exit <0x%x>", DMC_ERR);

    return DMC_ERR;
}

int momgr_set_value(mo_mgr_t * iMgr,
                    const dmtree_node_t * nodeP)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == nodeP, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_set_value <%s>", nodeP->uri);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, nodeP->uri),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->setFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->setFunc(nodeP, plugin->data));

DMC_ON_ERR:

    DMC_LOGF("momgr_set_value exit <0x%x>", DMC_ERR);

    return DMC_ERR;
}

int momgr_get_ACL(mo_mgr_t *iMgr,
                  const char *iURI,
                  char **oACL)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == oACL, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_get_ACL <%s>", iURI);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->getACLFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->getACLFunc(iURI, oACL, plugin->data));

    DMC_LOGF("momgr_get_ACL value <%s>", *oACL);

DMC_ON_ERR:

    DMC_LOGF("momgr_get_ACL exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_set_ACL(mo_mgr_t *iMgr,
                  const char *iURI,
                  const char *iACL)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == iACL, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_set_ACL <%s> <%s>", iURI, iACL);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->setACLFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->setACLFunc(iURI, iACL, plugin->data));

    DMC_LOGF("momgr_set_ACL value <%s>", iACL);

DMC_ON_ERR:

    DMC_LOGF("momgr_set_ACL exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_rename_node(mo_mgr_t *iMgr,
                      const char *iURI,
                      const char *iNewName)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;
    char * nameCopy = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == iNewName, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_rename_node <%s> <%s>", iURI, iNewName);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->renameFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    // uri_validate_path() will check for NULL
    nameCopy = strdup(iNewName);
    DMC_FAIL(uri_validate_path(nameCopy, 1, iMgr->max_segment_len));
    DMC_FAIL(plugin->interface->renameFunc(iURI, iNewName, plugin->data));

DMC_ON_ERR:

    if (nameCopy) free(nameCopy);
    DMC_LOGF("momgr_set_ACL exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_delete_node(mo_mgr_t *iMgr,
                      const char *iURI)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_delete_node <%s>", iURI);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->deleteFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->deleteFunc(iURI, plugin->data));

DMC_ON_ERR:

    DMC_LOGF("momgr_delete_node exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_exec_node(mo_mgr_t *iMgr,
                    const char *iURI,
                    const char *iData,
                    const char *iCorrelator)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_exec_node <%s> <%s> <%s>", iURI, iData, iCorrelator);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->execFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->execFunc(iURI, iData, iCorrelator, plugin->data));

DMC_ON_ERR:

    DMC_LOGF("momgr_exec_node exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_validate_uri(mo_mgr_t * iMgr,
                       const char * uri,
                       char ** oNodeURI,
                       char ** oPropId)
{
    return uri_validate(iMgr->max_total_len,
                        iMgr->max_depth,
                        iMgr->max_segment_len,
                        uri,
                        oNodeURI,
                        oPropId);
}

// For DM 1.3 we'll have to extend this function for the case were multiple urls
// are found. For now we request the criteria and we return the first match.
int momgr_find_subtree(mo_mgr_t * iMgr,
                       const char * iUri,
                       const char * iUrn,
                       const char * iCriteriaName,
                       const char * iCriteriaValue,
                       char ** oUri)
{
    DMC_ERR_MANAGE;

    mo_dir_t * dirP;
    char ** urlList = NULL;
    int i;
    bool found = false;
    dmtree_node_t node;

    memset(&node, 0, sizeof(dmtree_node_t));

    DMC_FAIL_ERR(NULL == iUri && NULL == iUrn, OMADM_SYNCML_ERROR_NOT_FOUND);
    DMC_FAIL_ERR(NULL == iCriteriaName, OMADM_SYNCML_ERROR_NOT_FOUND);
    DMC_FAIL_ERR(NULL == iCriteriaValue, OMADM_SYNCML_ERROR_NOT_FOUND);

    if (NULL != iUri)
    {
        char * subUri = (char *)iUri;
        if (('.' == subUri[0]) && ('/' == subUri[1]))
        {
            subUri += 2;
        }

        DMC_FAIL_NULL(dirP, prv_findMoDir(iMgr->root, subUri, false), OMADM_SYNCML_ERROR_NOT_FOUND);
    }
    else
    {
        dirP = iMgr->root;
    }

    if (NULL != iUrn)
    {
        prv_findUrn(dirP, iUrn, &urlList);
    }
    else
    {
        prv_getChildrenUrl(dirP, iUri, &urlList);
    }

    DMC_FAIL_ERR(NULL == urlList, OMADM_SYNCML_ERROR_NOT_FOUND);

    i = 0;
    while(NULL != urlList[i] && false == found)
    {
        DMC_FAIL_NULL(node.uri, str_cat_3(urlList[i], "/", iCriteriaName), OMADM_SYNCML_ERROR_DEVICE_FULL);
        DMC_FAIL(momgr_get_value(iMgr, &node));
        if (strlen(iCriteriaValue) == node.data_size
         && !strncmp(node.data_buffer, iCriteriaValue, node.data_size))
        {
            found = true;
        }
        dmtree_node_clean(&node, true);
        i++;
    }

    if (found)
    {
        DMC_FAIL_NULL(*oUri, strdup(urlList[i - 1]), OMADM_SYNCML_ERROR_DEVICE_FULL);
    }
    else
    {
        DMC_FAIL(OMADM_SYNCML_ERROR_NOT_FOUND);
    }

DMC_ON_ERR:

    if (urlList) strArray_free(urlList);
    dmtree_node_clean(&node, true);

    return DMC_ERR;
}

int momgr_list_uri(mo_mgr_t *iMgr,
                   const char * iUrn,
                   char *** oUri)
{
    DMC_ERR_MANAGE;

    DMC_FAIL_ERR(NULL == iUrn, OMADM_SYNCML_ERROR_COMMAND_FAILED);
    DMC_FAIL_ERR(NULL == oUri, OMADM_SYNCML_ERROR_COMMAND_FAILED);

    prv_findUrn(iMgr->root, iUrn, oUri);

    DMC_FAIL_ERR(NULL == *oUri, OMADM_SYNCML_ERROR_NOT_FOUND);

DMC_ON_ERR:

    return DMC_ERR;
}
 
