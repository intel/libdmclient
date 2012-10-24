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
#include <dlfcn.h>
#include <dirent.h>
#include <stdbool.h>

#include "config.h"

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
        if (1 != sscanf(node.data_buffer, "%hu", resultP))
        {
            error = OMADM_SYNCML_ERROR_SESSION_INTERNAL;
        }
        free(node.data_buffer);
    }

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

    if (iPlugin->dl_handle)
    {
        dlclose(iPlugin->dl_handle);
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

static dmtree_plugin_t * prv_findPlugin(const mo_mgr_t iMgr,
                                        const char *iURI)
{
    char * uriCopy = NULL;
    char * subUri;
    mo_dir_t * dirP;

    if (!strcmp(iURI, "."))
    {
        return iMgr.root->plugin;
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

    dirP = prv_findMoDir(iMgr.root, subUri, false);
    free(uriCopy);

    while (NULL != dirP && NULL == dirP->plugin)
    {
        dirP = dirP->parent;
    }

    if (NULL != dirP)
    {
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

int momgr_add_plugin(mo_mgr_t * iMgr,
                     omadm_mo_interface_t *iPlugin,
                     void * handle)
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
        newElem->plugin->dl_handle = handle;
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

void momgr_load_plugin(mo_mgr_t * iMgrP,
                       const char *iFilename)
{
    void * handle = NULL;
    omadm_mo_interface_t * moInterfaceP = NULL;
    omadm_mo_interface_t * (*getMoIfaceF)();

    if (iFilename == NULL)
    {
        return;
    }
    handle = dlopen(iFilename, RTLD_LAZY);
    if (!handle) goto error;

    getMoIfaceF = dlsym(handle, "omadm_get_mo_interface");
    if (!getMoIfaceF) goto error;

    moInterfaceP = getMoIfaceF();
    if ((!moInterfaceP) || (!moInterfaceP->base_uri)) goto error;

    if (OMADM_SYNCML_ERROR_NONE == momgr_add_plugin(iMgrP, moInterfaceP, handle))
    {
        handle = NULL;
    }
    // prv_add_plugin() would have free moInterfaceP in case of error
    moInterfaceP = NULL;

error:
    if (handle)
        dlclose (handle);
}

int momgr_init(mo_mgr_t * iMgrP)
{
    int error = OMADM_SYNCML_ERROR_NONE;
    DIR *folderP;
    dmtree_plugin_t * detailPluginP;

    if(!iMgrP)
        return OMADM_SYNCML_ERROR_SESSION_INTERNAL;

    memset(iMgrP, 0, sizeof(mo_mgr_t));

    // set the root plugin
    iMgrP->root = (mo_dir_t *) malloc(sizeof(mo_dir_t));
    if (NULL == iMgrP->root) return OMADM_SYNCML_ERROR_DEVICE_FULL;
    memset(iMgrP->root, 0, sizeof(mo_dir_t));
    iMgrP->root->name = strdup(".");

    iMgrP->root->plugin = (dmtree_plugin_t *) malloc(sizeof(dmtree_plugin_t));
    if (NULL == iMgrP->root->plugin)
    {
        free(iMgrP->root);
        return OMADM_SYNCML_ERROR_DEVICE_FULL;
    }
    memset(iMgrP->root->plugin, 0, sizeof(dmtree_plugin_t));

    iMgrP->root->plugin->interface = getDefaultRootPlugin();
    iMgrP->root->plugin->container = iMgrP->root;

    // load plugins from the plugins directory
    folderP = opendir(MO_INSTALL_DIR);
    if (folderP != NULL)
    {
        struct dirent *fileP;

        while ((fileP = readdir(folderP)))
        {
            if (DT_REG == fileP->d_type)
            {
                char * filename;

                filename = str_cat_3(MO_INSTALL_DIR, "/", fileP->d_name);
                momgr_load_plugin(iMgrP, filename);
                free(filename);
            }
        }
        closedir(folderP);
    }

    if (OMADM_SYNCML_ERROR_NONE == error)
    {
        // retrieve uri limits
        detailPluginP = prv_findPlugin(*iMgrP, "./DevDetail");
        if (detailPluginP && detailPluginP->interface->getFunc)
        {
            error = prv_get_short(detailPluginP, "./DevDetail/URI/MaxDepth", &(iMgrP->max_depth));
            if (OMADM_SYNCML_ERROR_NONE == error)
            {
                error = prv_get_short(detailPluginP, "./DevDetail/URI/MaxTotLen", &(iMgrP->max_total_len));
                if (OMADM_SYNCML_ERROR_NONE == error)
                {
                    error = prv_get_short(detailPluginP, "./DevDetail/URI/MaxSegLen", &(iMgrP->max_segment_len));
                }
            }
        }
        else
        {
            error = OMADM_SYNCML_ERROR_SESSION_INTERNAL;
        }
    }

    if (OMADM_SYNCML_ERROR_NONE != error)
    {
        momgr_free(iMgrP);
    }
    return error;
}

void momgr_free(mo_mgr_t * iMgrP)
{
    if(iMgrP->root)
    {
        prv_freeMoDir(iMgrP->root);
        iMgrP->root = NULL;
    }
}

int momgr_exists(const mo_mgr_t iMgr,
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
        dirP = prv_findMoDir(iMgr.root, subUri, true);

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

int momgr_get_value(const mo_mgr_t iMgr,
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
        dirP = prv_findMoDir(iMgr.root, subUri, false);

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
                nodeP->data_size = strlen(nodeP->data_buffer) + 1;
            }
        }
    }

DMC_ON_ERR:

    DMC_LOGF("momgr_get_value exit <0x%x>", DMC_ERR);

    return DMC_ERR;
}

int momgr_set_value(const mo_mgr_t iMgr,
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

int momgr_get_ACL(const mo_mgr_t iMgr,
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

int momgr_set_ACL(const mo_mgr_t iMgr,
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

int momgr_rename_node(const mo_mgr_t iMgr,
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
    DMC_FAIL(uri_validate_path(nameCopy, 1, iMgr.max_segment_len));
    DMC_FAIL(plugin->interface->renameFunc(iURI, iNewName, plugin->data));

DMC_ON_ERR:

    if (nameCopy) free(nameCopy);
    DMC_LOGF("momgr_set_ACL exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_delete_node(const mo_mgr_t iMgr,
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

int momgr_exec_node(const mo_mgr_t iMgr,
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

int momgr_validate_uri(const mo_mgr_t iMgr,
                       const char * uri,
                       char ** oNodeURI,
                       char ** oPropId)
{
    return uri_validate(iMgr.max_total_len,
                        iMgr.max_depth,
                        iMgr.max_segment_len,
                        uri,
                        oNodeURI,
                        oPropId);
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

static char ** prv_buildChildList(const char * iBaseUri,
                                  const char * iChildList)
{
    char ** result = NULL;
    int nb_child = 0;
    char * childName;
    char * listCopy = NULL;

    listCopy = strdup(iChildList);
    if (NULL == listCopy) return NULL;

    childName = listCopy;
    while(childName && *childName)
    {
        nb_child++;
        childName = strchr(childName, '/');
        if (childName) childName += 1;
    }
    if (0 == nb_child) return NULL;

    result = (char**)malloc((nb_child + 1) * sizeof(char*));
    memset(result, 0, (nb_child + 1) * sizeof(char*));
    if (result)
    {
        nb_child = 0;
        childName = listCopy;
        while(childName && *childName)
        {
            char * slashStr;

            slashStr = strchr(childName, '/');
            if (slashStr)
            {
                *slashStr = 0;
                slashStr++;
            }

            result[nb_child] = str_cat_3(iBaseUri, "/", childName);
            if (NULL == result[nb_child])
            {
                strArray_free(result);
                return NULL;
            }
            nb_child++;
            childName = slashStr;
        }
    }

    return result;
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
                    *listP = prv_buildChildList(baseUri, node.data_buffer);
                    dmtree_node_clean(&node, false);
                }
            }
        }
    }
}

// For DM 1.3 we'll have to extend this function for the case were multiple urls
// are found. For now we request the criteria and we return the first match.
int momgr_find_subtree(const mo_mgr_t iMgr,
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

        DMC_FAIL_NULL(dirP, prv_findMoDir(iMgr.root, subUri, false), OMADM_SYNCML_ERROR_NOT_FOUND);
    }
    else
    {
        dirP = iMgr.root;
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
        if (!strcmp(node.data_buffer, iCriteriaValue))
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

int momgr_list_uri(const mo_mgr_t iMgr,
                   const char * iUrn,
                   char *** oUri)
{
    DMC_ERR_MANAGE;

    DMC_FAIL_ERR(NULL == iUrn, OMADM_SYNCML_ERROR_COMMAND_FAILED);
    DMC_FAIL_ERR(NULL == oUri, OMADM_SYNCML_ERROR_COMMAND_FAILED);

    prv_findUrn(iMgr.root, iUrn, oUri);

    DMC_FAIL_ERR(NULL == *oUri, OMADM_SYNCML_ERROR_NOT_FOUND);

DMC_ON_ERR:

    return DMC_ERR;
}
 
