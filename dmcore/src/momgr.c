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
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>

#include "config.h"

#include "error_macros.h"
#include "log.h"

#include "syncml_error.h"
#include "momgr.h"

// defined in defaultroot.c
omadm_mo_interface_t * getDefaultRootPlugin();

static void prv_freePlugin(dmtree_plugin_t *oPlugin)
{
    if (oPlugin->URI)
        free(oPlugin->URI);

    if (oPlugin->interface) {
        if (oPlugin->interface->closeFunc)
        {
            oPlugin->interface->closeFunc(oPlugin->data);
        }
        free(oPlugin->interface);
    }

    if (oPlugin->dl_handle)
    {
        dlclose(oPlugin->dl_handle);
    }

    free(oPlugin);
}

static dmtree_plugin_t *prv_findPlugin(const mo_list_t iList,
                   const char *iURI)
{
    dmtree_plugin_t *matchingPlugin = NULL;
    unsigned int longest = 0;
    unsigned int pluginURILen = 0;
    plugin_elem_t * elem = iList.first;

    while(elem)
    {
        pluginURILen = strlen(elem->plugin->URI);

        if ((pluginURILen == strlen(iURI) + 1)
            && !strncmp(elem->plugin->URI, iURI, pluginURILen - 1)) {
            matchingPlugin = elem->plugin;
            break;
        } else if ((strstr(iURI, elem->plugin->URI))
               && (pluginURILen > longest)) {
            longest = pluginURILen;
            matchingPlugin = elem->plugin;
        }
        elem = elem->next;
    }

    return matchingPlugin;
}

static void prv_removePlugin(mo_list_t * iListP,
                             const char *iURI)
{
    plugin_elem_t * elem = iListP->first;
    plugin_elem_t * target = NULL;

    if (elem)
    {
        if (strcmp(iURI, elem->plugin->URI))
        {
            while ((elem->next)
                && (strcmp(iURI, elem->next->plugin->URI)))
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
            iListP->first = iListP->first->next;
        }

        if (target)
        {
            prv_freePlugin(target->plugin);
            free(target);
        }
    }
}

static int prv_add_plugin(mo_list_t * iList,
                          const char *iURI,
                          omadm_mo_interface_t *iPlugin,
                          void * handle)
{
    DMC_ERR_MANAGE;
    unsigned int uriLen = strlen(iURI);
    plugin_elem_t * newElem = NULL;
    void * pluginData = NULL;

    DMC_LOGF("uri <%s>", iURI);

    //TODO: use dmtree_validate_uri(pluginDescP->uri, false)
    if (uriLen < 2 || iURI[0] != '.' || iURI[1] != '/'
        || iURI[uriLen - 1] != '/') {
        DMC_ERR = OMADM_SYNCML_ERROR_SESSION_INTERNAL;
        goto DMC_ON_ERR;
    }

    DMC_FAIL_ERR(NULL == iPlugin, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == iPlugin->initFunc, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL(iPlugin->initFunc(&pluginData));

    DMC_FAIL_NULL(newElem, (plugin_elem_t *) malloc(sizeof(plugin_elem_t)),
              OMADM_SYNCML_ERROR_DEVICE_FULL);
    memset(newElem, 0, sizeof(plugin_elem_t));
    DMC_FAIL_NULL(newElem->plugin, (dmtree_plugin_t *) malloc(sizeof(dmtree_plugin_t)),
              OMADM_SYNCML_ERROR_DEVICE_FULL);

    memset(newElem->plugin, 0, sizeof(dmtree_plugin_t));

    DMC_FAIL_NULL(newElem->plugin->URI, strdup(iURI),
              OMADM_SYNCML_ERROR_DEVICE_FULL);
    newElem->plugin->interface = iPlugin;
    newElem->plugin->data = pluginData;
    newElem->plugin->dl_handle = handle;

    prv_removePlugin(iList, iURI);
    newElem->next = iList->first;
    iList->first = newElem;

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

    DMC_LOGF("exit <0x%x>", DMC_ERR);

    return DMC_ERR;
}

void momgr_load_plugin(mo_list_t * iListP,
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
    if ((!moInterfaceP) || (!moInterfaceP->uri)) goto error;

    if (OMADM_SYNCML_ERROR_NONE == prv_add_plugin(iListP, moInterfaceP->uri, moInterfaceP, handle))
    {
        handle = NULL;
    }
    // prv_add_plugin() would have free moInterfaceP in case of error
    moInterfaceP = NULL;

error:
    if (handle)
        dlclose (handle);
}

int momgr_init(mo_list_t * iListP)
{
    int error = OMADM_SYNCML_ERROR_NONE;
    DIR *folderP;

    if(!iListP)
        return OMADM_SYNCML_ERROR_SESSION_INTERNAL;

    iListP->first = NULL;

    folderP = opendir(MOBJS_DIR);
    if (folderP != NULL)
    {
        struct dirent *fileP;

        while ((fileP = readdir(folderP)))
        {
            if (DT_REG == fileP->d_type)
            {
                char * filename;

                filename = str_cat_3(MOBJS_DIR, "/", fileP->d_name);
                momgr_load_plugin(iListP, filename);
                free(filename);
            }
        }
        closedir(folderP);
    }

    // Check if we have a root plugin
    if (NULL == prv_findPlugin(*iListP, "."))
    {
        error = prv_add_plugin(iListP, "./", getDefaultRootPlugin(), NULL);
        if (OMADM_SYNCML_ERROR_NONE != error)
        {
            momgr_free(iListP);
        }
    }

    return error;
}

void momgr_free(mo_list_t * iListP)
{
    while(iListP->first)
    {
        plugin_elem_t * elem = iListP->first;

        iListP->first = elem->next;
        prv_freePlugin(elem->plugin);
        free(elem);
    }
}

int momgr_exists(const mo_list_t iList,
                 const char *iURI,
                 omadmtree_node_type_t * oExists)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_LOGF("momgr_node_exists <%s>", iURI);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
               OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->isNodeFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);
    DMC_FAIL(plugin->interface->isNodeFunc(iURI, oExists, plugin->data));

    DMC_LOGF("momgr_node_exists exit <0x%x> %d",
                DMC_ERR, *oExists);

DMC_ON_ERR:

    return DMC_ERR;
}

int momgr_get_value(const mo_list_t iList,
                    dmtree_node_t * nodeP)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;
    char * childNode = NULL;
    omadmtree_node_type_t exists = OMADM_NODE_NOT_EXIST;
    plugin_elem_t * elem;

    DMC_FAIL_ERR(NULL == nodeP, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_get_value <%s>", nodeP->uri);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iList, nodeP->uri),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->getFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->getFunc(nodeP, plugin->data));

    if (strcmp(nodeP->uri, ".")) goto DMC_ON_ERR;

    /* Special case for the root node. */
    elem = iList.first;
    while(elem)
    {
        if (strcmp(elem->plugin->URI, "./")
         || (((unsigned int) (strchr(elem->plugin->URI + 2, '/') - elem->plugin->URI)) != strlen(elem->plugin->URI) - 1))
        {

            DMC_FAIL_NULL(childNode, strdup(elem->plugin->URI), OMADM_SYNCML_ERROR_DEVICE_FULL);

            childNode[strlen(childNode) - 1] = 0;

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

int momgr_set_value(const mo_list_t iList,
                    const dmtree_node_t * nodeP)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == nodeP, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_set_value <%s>", nodeP->uri);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iList, nodeP->uri),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->setFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->setFunc(nodeP, plugin->data));

DMC_ON_ERR:

    DMC_LOGF("momgr_set_value exit <0x%x>", DMC_ERR);

    return DMC_ERR;
}

int momgr_get_ACL(const mo_list_t iList,
                  const char *iURI,
                  char **oACL)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == oACL, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_get_ACL <%s>", iURI);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
              OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->getACLFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->getACLFunc(iURI, oACL, plugin->data));

    DMC_LOGF("momgr_get_ACL value <%s>", *oACL);

DMC_ON_ERR:

    DMC_LOGF("momgr_get_ACL exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_set_ACL(const mo_list_t iList,
                  const char *iURI,
                  const char *iACL)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == iACL, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_set_ACL <%s> <%s>", iURI, iACL);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
              OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->setACLFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->setACLFunc(iURI, iACL, plugin->data));

    DMC_LOGF("momgr_set_ACL value <%s>", iACL);

DMC_ON_ERR:

    DMC_LOGF("momgr_set_ACL exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_rename_node(const mo_list_t iList,
                      const char *iURI,
                      const char *iNewName)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == iNewName, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_rename_node <%s> <%s>", iURI, iNewName);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->renameFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    // TODO: check that iNewName is valid (here or in dmtree.c
    DMC_FAIL(plugin->interface->renameFunc(iURI, iNewName, plugin->data));

DMC_ON_ERR:

    DMC_LOGF("momgr_set_ACL exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_delete_node(const mo_list_t iList,
                      const char *iURI)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_delete_node <%s>", iURI);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->deleteFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->deleteFunc(iURI, plugin->data));

DMC_ON_ERR:

    DMC_LOGF("momgr_delete_node exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_exec_node(const mo_list_t iList,
                    const char *iURI,
                    const char *iData,
                    const char *iCorrelator)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_exec_node <%s> <%s> <%s>", iURI, iData, iCorrelator);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->execFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->execFunc(iURI, iData, iCorrelator, plugin->data));

DMC_ON_ERR:

    DMC_LOGF("momgr_exec_node exit <%d>", DMC_ERR);

    return DMC_ERR;
}
