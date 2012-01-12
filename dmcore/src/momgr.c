/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007, ACCESS Systems Americas, Inc. All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file <momgr.c>
 *
 * @brief Management Object management code
 *
 *****************************************************************************/

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
#define URN(plug) (plug)->interface->urn

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
        if (URN(iPlugin)) free(URN(iPlugin));
        free(iPlugin->interface);
    }

    if (iPlugin->dl_handle)
    {
        dlclose(iPlugin->dl_handle);
    }

    free(iPlugin);
}

static dmtree_plugin_t * prv_findPlugin(const mo_mgr_t iMgr,
                                        const char *iURI,
                                        const char *iURN)
{
    plugin_elem_t * elem = iMgr.first;

    while(elem
       && ((iURI && URI(elem->plugin) && strncmp(URI(elem->plugin), iURI, strlen(URI(elem->plugin))))
        || (iURN && URN(elem->plugin) && strncmp(URN(elem->plugin), iURN, strlen(URN(elem->plugin))))))
    {
        elem = elem->next;
    }

    if (elem)
    {
        return elem->plugin;
    }
    return NULL;
}

static void prv_removePlugin(mo_mgr_t * iMgrP,
                             const char *iURx)
{
    plugin_elem_t * elem = iMgrP->first;
    plugin_elem_t * target = NULL;

    if (iURx && elem)
    {
        if ((!URI(elem->plugin) || strcmp(iURx, URI(elem->plugin)))
         && (!URN(elem->plugin) || strcmp(iURx, URN(elem->plugin))))
        {
            while ((elem->next)
                && (!URI(elem->next->plugin) || strcmp(iURx, URI(elem->next->plugin)))
                && (!URN(elem->next->plugin) || strcmp(iURx, URN(elem->next->plugin))))
            {
                elem = elem->next;
            }
            if (elem->next)
            {
                target = elem->next;
                elem->next = elem->next->next;
            }
        }
        else
        {
            target = elem;
            iMgrP->first = iMgrP->first->next;
        }

        if (target)
        {
            prv_freePlugin(target->plugin);
            free(target);
        }
    }
}

static int prv_add_plugin(mo_mgr_t * iMgr,
                          omadm_mo_interface_t *iPlugin,
                          void * handle)
{
    DMC_ERR_MANAGE;
    plugin_elem_t * newElem = NULL;
    void * pluginData = NULL;
    char * uriCopy = NULL;

    DMC_LOGF("uri <%s>", iPlugin->base_uri);

    /* Plugin base URI must be root (".") or a direct child of root ("./[^/]*") The "./" part is optionnal. */
    if (iPlugin->base_uri)
    {
        if ('.' == iPlugin->base_uri[0])
        {
            switch(iPlugin->base_uri[1])
            {
            case 0:
                break;
            case '/':
                uriCopy = strdup(iPlugin->base_uri + 2);
                DMC_FAIL(uri_validate_path(uriCopy, 1, iMgr->max_segment_len));
                break;
            default:
                DMC_FAIL(OMADM_SYNCML_ERROR_SESSION_INTERNAL);
            }
        }
        else
        {
            uriCopy = strdup(iPlugin->base_uri + 2);
            DMC_FAIL(uri_validate_path(uriCopy, 1, iMgr->max_segment_len));
        }
    }

    // check urn is valid

    DMC_FAIL_ERR(NULL == iPlugin, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == iPlugin->initFunc, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL(iPlugin->initFunc(&pluginData));

    DMC_FAIL_NULL(newElem, (plugin_elem_t *) malloc(sizeof(plugin_elem_t)),
              OMADM_SYNCML_ERROR_DEVICE_FULL);
    memset(newElem, 0, sizeof(plugin_elem_t));
    DMC_FAIL_NULL(newElem->plugin, (dmtree_plugin_t *) malloc(sizeof(dmtree_plugin_t)),
              OMADM_SYNCML_ERROR_DEVICE_FULL);

    memset(newElem->plugin, 0, sizeof(dmtree_plugin_t));

    newElem->plugin->interface = iPlugin;
    newElem->plugin->data = pluginData;
    newElem->plugin->dl_handle = handle;

    prv_removePlugin(iMgr, iPlugin->base_uri);
    prv_removePlugin(iMgr, iPlugin->urn);
    newElem->next = iMgr->first;
    iMgr->first = newElem;

    newElem = NULL;

DMC_ON_ERR:

    if (newElem)
    {
        if (newElem->plugin)
        {
            prv_freePlugin(newElem->plugin);
        }
        free(newElem);
    }
    if (uriCopy) free(uriCopy);

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
    if ((!moInterfaceP) || (!moInterfaceP->base_uri && !moInterfaceP->urn)) goto error;

    if (OMADM_SYNCML_ERROR_NONE == prv_add_plugin(iMgrP, moInterfaceP, handle))
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

    // Check if we have all mandatory MOs
    if (NULL == prv_findPlugin(*iMgrP, NULL, URN_MO_DMACC))
    {
        error = OMADM_SYNCML_ERROR_NOT_FOUND;
    }
    else if (NULL == prv_findPlugin(*iMgrP, NULL, URN_MO_DEVDETAIL))
    {
        error = OMADM_SYNCML_ERROR_NOT_FOUND;
    }
    else if (NULL == prv_findPlugin(*iMgrP, NULL, URN_MO_DEVINFO))
    {
        error = OMADM_SYNCML_ERROR_NOT_FOUND;
    }
    else if (NULL == prv_findPlugin(*iMgrP, ".", NULL))
    {
        // Check if we have a root plugin
        error = prv_add_plugin(iMgrP, getDefaultRootPlugin(), NULL);
    }

    if (OMADM_SYNCML_ERROR_NONE == error)
    {
        // retrieve uri limits
        detailPluginP = prv_findPlugin(*iMgrP, "./DevDetail", NULL);
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
    while(iMgrP->first)
    {
        plugin_elem_t * elem = iMgrP->first;

        iMgrP->first = elem->next;
        prv_freePlugin(elem->plugin);
        free(elem);
    }
}

int momgr_exists(const mo_mgr_t iMgr,
                 const char *iURI,
                 omadmtree_node_type_t * oExists)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_LOGF("momgr_node_exists <%s>", iURI);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI, NULL),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->isNodeFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);
    DMC_FAIL(plugin->interface->isNodeFunc(iURI, oExists, plugin->data));

    DMC_LOGF("momgr_node_exists exit <0x%x> %d", DMC_ERR, *oExists);

DMC_ON_ERR:

    return DMC_ERR;
}

int momgr_get_value(const mo_mgr_t iMgr,
                    dmtree_node_t * nodeP)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;
    char * childNode = NULL;
    omadmtree_node_type_t exists = OMADM_NODE_NOT_EXIST;
    plugin_elem_t * elem;

    DMC_FAIL_ERR(NULL == nodeP, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_get_value <%s>", nodeP->uri);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, nodeP->uri, NULL),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->getFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->getFunc(nodeP, plugin->data));

    if (strcmp(nodeP->uri, ".")) goto DMC_ON_ERR;

    /* Special case for the root node. */
    elem = iMgr.first;
    while(elem)
    {
        if (strcmp(URI(elem->plugin), "."))
        {
            DMC_FAIL_NULL(childNode, strdup(URI(elem->plugin)), OMADM_SYNCML_ERROR_DEVICE_FULL);

            DMC_FAIL(elem->plugin->interface->isNodeFunc(childNode, &exists, elem->plugin->data));

            if (exists != OMADM_NODE_NOT_EXIST)
            {
                if (nodeP->data_buffer)
                {
                    char * tmp_str;

                    DMC_FAIL_NULL(tmp_str, str_cat_3(nodeP->data_buffer, "/", childNode+2), OMADM_SYNCML_ERROR_DEVICE_FULL);
                    free(nodeP->data_buffer);
                    nodeP->data_buffer = tmp_str;
                }
                else
                {
                    nodeP->data_buffer = strdup(childNode+2);
                }
            }
            free(childNode);
            childNode = NULL;
        }
        elem= elem->next;
    }
    if (!nodeP->format)
    {
        DMC_FAIL_NULL(nodeP->format, strdup("node"), OMADM_SYNCML_ERROR_DEVICE_FULL);
    }

DMC_ON_ERR:

    if (childNode) free(childNode);

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

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, nodeP->uri, NULL),
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

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI, NULL),
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

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI, NULL),
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

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI, NULL),
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

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI, NULL),
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

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI, NULL),
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

int momgr_get_uri_from_urn(const mo_mgr_t iMgr,
                           const char * iUrn,
                           char ** oUri)
{
    dmtree_plugin_t * plugin;
    int result = OMADM_SYNCML_ERROR_NOT_FOUND;

    *oUri = NULL;

    plugin = prv_findPlugin(iMgr, NULL, iUrn);
    if (plugin)
    {
        if (URI(plugin))
        {
            *oUri = strdup(URI(plugin));
            if (*oUri)
                result = OMADM_SYNCML_ERROR_NONE;
            else
                result = OMADM_SYNCML_ERROR_COMMAND_FAILED;
        }
    }

    return result;
}

int momgr_find_subtree(const mo_mgr_t iMgr,
                       const char * iUri,
                       const char * iCriteriaName,
                       const char * iCriteriaValue,
                       char ** oUri)
{
    DMC_ERR_MANAGE;

    dmtree_plugin_t * plugin;
    char * childList = NULL;
    char ** childUriList = NULL;
    int i = 0;
    bool found = false;
    dmtree_node_t node;

    memset(&node, 0, sizeof(dmtree_node_t));

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iUri, NULL),
                  OMADM_SYNCML_ERROR_NOT_FOUND);
    DMC_FAIL_ERR(NULL == iCriteriaName, OMADM_SYNCML_ERROR_NOT_FOUND);
    DMC_FAIL_ERR(NULL == plugin->interface->getFunc || NULL == plugin->interface->isNodeFunc, OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_NULL(node.uri, strdup(iUri), OMADM_SYNCML_ERROR_DEVICE_FULL);
    DMC_FAIL(plugin->interface->getFunc(&node, plugin->data));
    DMC_FAIL_ERR(node.data_size == 0, OMADM_SYNCML_ERROR_NOT_FOUND);

    childList = node.data_buffer;
    DMC_FAIL_NULL(childUriList, get_child_uri_list(iUri, childList), OMADM_SYNCML_ERROR_DEVICE_FULL);
    dmtree_node_clean(&node, false);

    while (childUriList[i] && !found)
    {
        omadmtree_node_type_t type;

        DMC_FAIL_NULL(node.uri, str_cat_3(childUriList[i], "/", iCriteriaName), OMADM_SYNCML_ERROR_DEVICE_FULL);
        DMC_FAIL(plugin->interface->isNodeFunc(node.uri, &type, plugin->data));
        if(OMADM_NODE_NOT_EXIST != type)
        {
            if (iCriteriaValue
             && OMADM_NODE_IS_LEAF == type)
            {
                DMC_FAIL(plugin->interface->getFunc(&node, plugin->data));
                if (!strcmp(node.data_buffer, iCriteriaValue))
                {
                    found = true;
                }
            }
            else
            {
                found = true;
            }
        }
        dmtree_node_clean(&node, true);
        i++;
    }

    if (found)
    {
        DMC_FAIL_NULL(*oUri, strdup(childUriList[i - 1]), OMADM_SYNCML_ERROR_DEVICE_FULL);
    }
    else
    {
        DMC_FAIL(OMADM_SYNCML_ERROR_NOT_FOUND);
    }

DMC_ON_ERR:

    if (childList) free(childList);
    if (childUriList) free_uri_list(childUriList);
    dmtree_node_clean(&node, true);

    return DMC_ERR;
}
