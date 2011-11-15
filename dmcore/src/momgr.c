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
OMADM_DMTreePlugin * getDefaultRootPlugin();

static void prv_freePlugin(dmtree_plugin_t *oPlugin)
{
	if (oPlugin->URI)
		free(oPlugin->URI);

	if (oPlugin->interface) {
		oPlugin->interface->free(oPlugin->data);
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

int momgr_supports_transactions(const mo_list_t iList,
					const char *uri,
					bool *supports_transactions)
{
	DMC_ERR_MANAGE;
	dmtree_plugin_t *plugin = NULL;

	DMC_FAIL_NULL(plugin, prv_findPlugin(iList, uri),
			   OMADM_SYNCML_ERROR_NOT_FOUND);

	*supports_transactions = plugin->interface->supportTransactions;

DMC_ON_ERR:

	return DMC_ERR;
}

int momgr_add_plugin(mo_list_t * iList,
			         const char *iURI,
			         OMADM_DMTreePlugin *iPlugin)
{
	DMC_ERR_MANAGE;
	unsigned int uriLen = strlen(iURI);
    plugin_elem_t * newElem = NULL;
    void * pluginData = NULL;

	DMC_LOGF("uri <%s>", iURI);

	if (uriLen < 2 || iURI[0] != '.' || iURI[1] != '/'
	    || iURI[uriLen - 1] != '/') {
		DMC_ERR = OMADM_SYNCML_ERROR_SESSION_INTERNAL;
		goto DMC_ON_ERR;
	}

    DMC_FAIL_ERR(NULL == iPlugin, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == iPlugin->create, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL(iPlugin->create("funambol", &pluginData));

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

void momgr_load_plugin(mo_list_t * iList,
                       const char *iFilename)
{
    void * handle = NULL;
    OMADM_PluginDesc * (*getPlugDescF)();
    OMADM_PluginDesc * pluginDescP = NULL;
    OMADM_DMTreePlugin * treePluginP = NULL;
    plugin_elem_t * newElem = NULL;
    void * pluginData = NULL;

    if (iFilename == NULL)
    {
        return;
    }
    handle = dlopen(iFilename, RTLD_LAZY);
    if (!handle) goto error;

    getPlugDescF = dlsym(handle, "omadm_get_plugin_desc");
    if (!getPlugDescF) goto error;

    pluginDescP = getPlugDescF();
    if (!pluginDescP) goto error;
    if ((!pluginDescP->uri) || (!pluginDescP->createFunc)) goto error;

    //TODO: use dmtree_validate_uri(pluginDescP->uri, false)
    if (strlen(pluginDescP->uri) < 2
     || pluginDescP->uri[0] != '.'
     || pluginDescP->uri[1] != '/')
    {
	    goto error;
    }

    treePluginP = pluginDescP->createFunc();
    if (!treePluginP) goto error;

    if (!treePluginP->create) goto error;
    if (OMADM_SYNCML_ERROR_NONE != treePluginP->create("funambol", &pluginData))
    {
        goto error;
    }

    newElem = (plugin_elem_t *) malloc(sizeof(plugin_elem_t));
	if (!newElem) goto error;
	memset(newElem, 0, sizeof(plugin_elem_t));
    newElem->plugin = (dmtree_plugin_t *) malloc(sizeof(dmtree_plugin_t));
	if (!newElem->plugin) goto error;

    memset(newElem->plugin, 0, sizeof(dmtree_plugin_t));
    newElem->plugin->dl_handle = handle;
    newElem->plugin->URI = pluginDescP->uri;
    newElem->plugin->interface = treePluginP;
    newElem->plugin->data = pluginData;

    prv_removePlugin(iList, newElem->plugin->URI);
    newElem->next = iList->first;
    iList->first = newElem;
    newElem = NULL;

    pluginDescP->uri = NULL;
    handle = NULL;
    treePluginP = NULL;

error:
    if (pluginDescP)
    {
        if (pluginDescP->uri)
            free(pluginDescP->uri);
        free(pluginDescP);
    }
    if (treePluginP)
        free(treePluginP);
    if (handle)
        dlclose (handle);
	if (newElem)
	{
	    if (newElem->plugin)
	    {
		    prv_freePlugin(newElem->plugin);
	    }
	    free(newElem);
    }
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
        error = momgr_add_plugin(iListP, "./", getDefaultRootPlugin());
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
			OMADM_NodeType * oExists)
{
	DMC_ERR_MANAGE;
	dmtree_plugin_t *plugin = NULL;
	OMADM_NodeExistsFN fn = NULL;

	DMC_LOGF("momgr_node_exists <%s>", iURI);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
		       OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_NULL(fn, plugin->interface->nodeExists,
	          OMADM_SYNCML_ERROR_NOT_ALLOWED);
    DMC_FAIL(fn(iURI, oExists, plugin->data));

	DMC_LOGF("momgr_node_exists exit <0x%x> %d",
			    DMC_ERR, *oExists);

DMC_ON_ERR:

	return DMC_ERR;
}

int momgr_get_children(const mo_list_t iList,
					const char *iURI,
					dmc_ptr_array *oChildren)
{
	DMC_ERR_MANAGE;
	dmtree_plugin_t *plugin = NULL;
	OMADM_GetNodeChildrenFN fn = NULL;
	unsigned int j = 0;
	char *childNode = NULL;
	OMADM_NodeType exists = OMADM_NODE_NOT_EXIST;
    plugin_elem_t * elem;

	DMC_LOGF("omadm_get_node_children <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
		      OMADM_SYNCML_ERROR_NOT_FOUND);

	DMC_FAIL_NULL(fn, plugin->interface->getNodeChildren,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, oChildren, plugin->data));

	if (strcmp(iURI, "."))
		goto DMC_ON_ERR;

	/* Special case for the root node. */
    elem = iList.first;
	while(elem)
	{
		if ((strcmp(elem->plugin->URI, "./") ||
			   (((unsigned int) (strchr(elem->plugin->URI + 2, '/') -
				elem->plugin->URI)) != strlen(elem->plugin->URI) - 1)))
		{

		    DMC_FAIL_NULL(childNode, strdup(elem->plugin->URI),
					    OMADM_SYNCML_ERROR_DEVICE_FULL);

		    childNode[strlen(childNode) - 1] = 0;

		    for (j = 0; j < dmc_ptr_array_get_size(oChildren)
		          && strcmp(childNode, (char *) dmc_ptr_array_get
				    (oChildren, j)); ++j) ;

		    if (j == dmc_ptr_array_get_size(oChildren)) {
			    DMC_FAIL(elem->plugin->interface->nodeExists(childNode,
								     &exists,
								     elem->plugin->data));

			    if (exists != OMADM_NODE_NOT_EXIST) {
				    DMC_FAIL_ERR(
					    dmc_ptr_array_append(
						    oChildren, childNode),
					    OMADM_SYNCML_ERROR_DEVICE_FULL);
			    } else
				    free(childNode);
		    } else
			    free(childNode);

		    childNode = NULL;
        }
		elem= elem->next;
	}

DMC_ON_ERR:

	DMC_LOGF("omadm_get_node_children exit <0x%x>", DMC_ERR);

	if (childNode)
		free(childNode);

	return DMC_ERR;
}

int momgr_get_value(const mo_list_t iList,
				    const char *iURI, char **oValue)
{
	DMC_ERR_MANAGE;
	dmtree_plugin_t *plugin = NULL;
	OMADM_GetValueFN fn = NULL;

	DMC_LOGF("momgr_get_value <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
		      OMADM_SYNCML_ERROR_NOT_FOUND);

	DMC_FAIL_NULL(fn, plugin->interface->getValue,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, oValue, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("momgr_get_value exit <0x%x>", DMC_ERR);

	return DMC_ERR;
}

int momgr_set_value(const mo_list_t iList,
				    const char *iURI, const char *iValue)
{
	DMC_ERR_MANAGE;
	dmtree_plugin_t *plugin = NULL;
	OMADM_SetValueFN fn = NULL;

	DMC_LOGF("momgr_set_value <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
		      OMADM_SYNCML_ERROR_NOT_FOUND);

	DMC_FAIL_NULL(fn, plugin->interface->setValue,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, iValue, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("momgr_set_value exit <0x%x>", DMC_ERR);

	return DMC_ERR;
}

int momgr_get_meta(const mo_list_t iList,
				   const char *iURI, const char *iProp,
				   char **oMeta)
{
	DMC_ERR_MANAGE;
	dmtree_plugin_t *plugin = NULL;
	OMADM_GetMetaFN fn = NULL;

	DMC_LOGF("momgr_get_meta prop <%s> <%s>", iProp, iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
		      OMADM_SYNCML_ERROR_NOT_FOUND);

	DMC_FAIL_NULL(fn, plugin->interface->getMeta,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, iProp, oMeta, plugin->data));

	DMC_LOGF("momgr_get_meta value <%s> <%s>", *oMeta);

DMC_ON_ERR:

	DMC_LOGF("momgr_get_meta exit <%d>", DMC_ERR);

	return DMC_ERR;
}

int momgr_set_meta(const mo_list_t iList,
				   const char *iURI, const char *iProp,
				   const char *iMeta)
{
	DMC_ERR_MANAGE;
	dmtree_plugin_t *plugin = NULL;
	OMADM_SetMetaFN fn = NULL;

	DMC_LOGF("momgr_set_meta <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
		      OMADM_SYNCML_ERROR_NOT_FOUND);

	DMC_FAIL_NULL(fn, plugin->interface->setMeta,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, iProp, iMeta, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("momgr_set_meta exit <0x%x>", DMC_ERR);

	return DMC_ERR;
}

int momgr_create_non_leaf(const mo_list_t iList,
				 const char *iURI)
{
	DMC_ERR_MANAGE;
	dmtree_plugin_t *plugin = NULL;
	OMADM_CreateNonLeafFN fn = NULL;

	DMC_LOGF("%s  <%s>", __FUNCTION__, iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
		      OMADM_SYNCML_ERROR_NOT_FOUND);

	DMC_FAIL_NULL(fn, plugin->interface->createNonLeaf,
			   OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("%s exit <0x%x>", __FUNCTION__, DMC_ERR);

	return DMC_ERR;
}

int momgr_get_access_rights(const mo_list_t iList,
					const char *iURI,
					OMADM_AccessRights *oRights)
{
	DMC_ERR_MANAGE;
	dmtree_plugin_t *plugin = NULL;
	OMADM_GetAccessRightsFN fn = NULL;

	DMC_LOGF("momgr_get_access_rights  <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
		      OMADM_SYNCML_ERROR_NOT_FOUND);

	DMC_FAIL_NULL(fn, plugin->interface->getAccessRights,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, oRights, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("momgr_get_access_rights exit <0x%x>", DMC_ERR);

	return DMC_ERR;
}

int momgr_delete_node(const mo_list_t iList,
				const char *iURI)
{
	DMC_ERR_MANAGE;
	dmtree_plugin_t *plugin = NULL;
	OMADM_DeleteNodeFN fn = NULL;

	DMC_LOGF("omadm_dmtree: momgr_delete_node <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
		      OMADM_SYNCML_ERROR_NOT_FOUND);

	DMC_FAIL_NULL(fn, plugin->interface->deleteNode,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("omadm_dmtree: momgr_delete_node exit <0x%x>", DMC_ERR);

	return DMC_ERR;
}

int momgr_exec_node(const mo_list_t iList,
				const char *iURI, const char *iData,
				const char *iCorrelator)
{
	DMC_ERR_MANAGE;
	dmtree_plugin_t *plugin = NULL;
	OMADM_ExecNodeFN fn = NULL;

	DMC_LOGF("omadm_dmtree: momgr_exec_node <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iList, iURI),
		      OMADM_SYNCML_ERROR_NOT_FOUND);

	DMC_FAIL_NULL(fn, plugin->interface->execNode,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, iData, iCorrelator, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("omadm_dmtree: momgr_exec_node exit <0x%x>",
			DMC_ERR);

	return DMC_ERR;
}

int momgr_update_nonce(const mo_list_t iList,
				       const char *iServerID,
				       const uint8_t *iNonce,
				       unsigned int iNonceLength,
				       bool iServerCred)
{
	DMC_ERR_MANAGE;
	dmtree_plugin_t *plugin = NULL;
	OMADM_UpdateNonceFN fn = NULL;

	DMC_LOG("momgr_update_nonce");

	DMC_FAIL_NULL(plugin, prv_findPlugin(iList, "./CONFIG"),
		      OMADM_SYNCML_ERROR_NOT_FOUND);

	DMC_FAIL_NULL(fn, plugin->interface->updateNonce,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn
		 (iServerID, iNonce, iNonceLength, iServerCred, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("omadm_dmtree: momgr_update_nonce exit <0x%x>",
		     DMC_ERR);

	return DMC_ERR;
}

int momgr_find_inherited_acl(mo_list_t iList,
					     const char *iURI, char **oACL)
{
	DMC_ERR_MANAGE;

	char *uri = NULL;
	unsigned int uriLen = 0;

	DMC_LOGF("momgr_find_inherited_acl <%s>", iURI);

	*oACL = NULL;

	DMC_FAIL_NULL(uri, strdup(iURI),
		      OMADM_SYNCML_ERROR_DEVICE_FULL);
	uriLen = strlen(uri);

	DMC_ERR = momgr_get_meta(iList, uri,
					  OMADM_NODE_PROPERTY_ACL, oACL);

	while ((uriLen > 0) && (DMC_ERR != OMADM_SYNCML_ERROR_NONE)) {
		while (uriLen > 0)
			if (uri[--uriLen] == '/') {
				uri[uriLen] = 0;
				break;
			}

		DMC_ERR = momgr_get_meta(iList, uri,
						  OMADM_NODE_PROPERTY_ACL,
						  oACL);
	}

DMC_ON_ERR:

	if (uri)
		free(uri);

	DMC_LOGF("momgr_find_inherited_acl <%d>", DMC_ERR);

	return DMC_ERR;
}
