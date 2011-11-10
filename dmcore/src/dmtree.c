/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007, ACCESS Systems Americas, Inc. All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file <dmtree.c>
 *
 * @brief Main source file for the DM tree
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "config.h"

#include "error_macros.h"
#include "log.h"

#include "syncml_error.h"
#include "dmtree.h"

typedef struct _OMADMPlugin OMADMPlugin;

struct _OMADMPlugin {
	char *URI;
	OMADM_DMTreePlugin *plugin;
	void *data;
	void *dl_handle;
};

static void prv_freeOMADMPlugin(OMADMPlugin *oPlugin)
{
	if (oPlugin->URI)
		free(oPlugin->URI);

	if (oPlugin->plugin) {
		oPlugin->plugin->free(oPlugin->data);
		free(oPlugin->plugin);
	}

    if (oPlugin->dl_handle)
    {
        dlclose(oPlugin->dl_handle);
    }

	free(oPlugin);
}

static void prv_freeOMADMPluginCB(void *oPlugin)
{
	OMADMPlugin *plugin = (OMADMPlugin *) oPlugin;
	prv_freeOMADMPlugin(plugin);
}

static OMADMPlugin *prv_findPlugin(const OMADM_DMTreeContext *iContext,
				   const char *iURI)
{
	unsigned int i = 0;
	OMADMPlugin *plugin = NULL;
	OMADMPlugin *matchingPlugin = NULL;
	unsigned int longest = 0;
	unsigned int pluginURILen = 0;

	for (i = 0; i < dmc_ptr_array_get_size(&iContext->plugins); ++i) {
		plugin =
		    (OMADMPlugin *) dmc_ptr_array_get(&iContext->plugins, i);

		pluginURILen = strlen(plugin->URI);

		if ((pluginURILen == strlen(iURI) + 1)
		    && !strncmp(plugin->URI, iURI, pluginURILen - 1)) {
			matchingPlugin = plugin;
			break;
		} else if ((strstr(iURI, plugin->URI))
			   && (pluginURILen > longest)) {
			longest = pluginURILen;
			matchingPlugin = plugin;
		}
	}

	return matchingPlugin;
}

int omadm_dmtree_supports_transactions(OMADM_DMTreeContext *context,
					const char *uri,
					bool *supports_transactions)
{
	DMC_ERR_MANAGE;
	OMADMPlugin *plugin = NULL;

	DMC_FAIL_NULL(plugin, prv_findPlugin(context, uri),
			   OMADM_SYNCML_ERROR_SESSION_INTERNAL);

	*supports_transactions = plugin->plugin->supportTransactions;

DMC_ON_ERR:

	return DMC_ERR;
}

int omadm_dmtree_create(const char *iServerID, OMADM_DMTreeContext **oContext)
{
	DMC_ERR_MANAGE;
	OMADM_DMTreeContext *retval;

	DMC_FAIL_NULL(retval, malloc(sizeof(*retval)), OMADM_SYNCML_ERROR_DEVICE_FULL);

	dmc_ptr_array_make(&retval->plugins, 8, prv_freeOMADMPluginCB);

	retval->serverID = iServerID;

	*oContext = retval;

	return OMADM_SYNCML_ERROR_NONE;

DMC_ON_ERR:

	free(retval);

	return DMC_ERR;
}

int omadm_dmtree_add_plugin(OMADM_DMTreeContext *iContext,
			    const char *iURI,
			    OMADM_DMTreePlugin *iPlugin)
{
	DMC_ERR_MANAGE;
	OMADMPlugin *plugin = NULL;
	unsigned int uriLen = strlen(iURI);

	DMC_LOGF("uri <%s>", iURI);

	if (uriLen < 2 || iURI[0] != '.' || iURI[1] != '/'
	    || iURI[uriLen - 1] != '/') {
		DMC_ERR = OMADM_SYNCML_ERROR_SESSION_INTERNAL;
		goto DMC_ON_ERR;
	}

	DMC_FAIL_NULL(plugin, (OMADMPlugin *) malloc(sizeof(OMADMPlugin)),
		      OMADM_SYNCML_ERROR_DEVICE_FULL);

	memset(plugin, 0, sizeof(OMADMPlugin));

	DMC_FAIL_NULL(plugin->URI, strdup(iURI),
		      OMADM_SYNCML_ERROR_DEVICE_FULL);

	DMC_FAIL_ERR(dmc_ptr_array_append(&iContext->plugins, plugin),
		       OMADM_SYNCML_ERROR_DEVICE_FULL);

	plugin->plugin = iPlugin;

	plugin = NULL;

DMC_ON_ERR:

	if (plugin)
		prv_freeOMADMPlugin(plugin);

	DMC_LOGF("exit <0x%x>", DMC_ERR);

	return DMC_ERR;
}

void omadm_dmtree_load_plugin(OMADM_DMTreeContext *iContext,
                              const char *iFilename)
{
    void * handle = NULL;
    OMADMPlugin * plugin = NULL;
    OMADM_PluginDesc * (*getPlugDescF)();
    OMADM_PluginDesc * pluginDescP = NULL;
    OMADM_DMTreePlugin * treePluginP = NULL;

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

    plugin = (OMADMPlugin *) malloc(sizeof(OMADMPlugin));
	if (!plugin) goto error;

    memset(plugin, 0, sizeof(OMADMPlugin));
    plugin->dl_handle = handle;
    plugin->URI = pluginDescP->uri;
    plugin->plugin = treePluginP;

    if (dmc_ptr_array_append(&iContext->plugins, plugin))
    {
        goto error;
    }

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
}

int omadm_dmtree_init(OMADM_DMTreeContext *iContext)
{
	DMC_ERR_MANAGE;
	unsigned int i = 0;
	OMADMPlugin *plugin = NULL;

	DMC_LOG("omadm_dmtree_init");

	for (i = 0; i < dmc_ptr_array_get_size(&iContext->plugins); ++i) {
		plugin =
		    (OMADMPlugin *) dmc_ptr_array_get(&iContext->plugins, i);
		DMC_FAIL(plugin->plugin->create(iContext->serverID, &plugin->data));
	}

DMC_ON_ERR:

	DMC_LOGF("omadm_dmtree_init exit <0x%x>", DMC_ERR);

	return DMC_ERR;
}

void omadm_dmtree_free(OMADM_DMTreeContext * oContext)
{
	if (oContext)
	{
		dmc_ptr_array_free(&oContext->plugins);
		free(oContext);
	}
}

int omadm_dmtree_exists(const OMADM_DMTreeContext * iContext,
			const char *iURI,
			OMADM_NodeType * oExists)
{
	DMC_ERR_MANAGE;
	OMADMPlugin *plugin = NULL;
	OMADM_NodeExistsFN fn = NULL;

	DMC_LOGF("omadm_dmtree_node_exists <%s>", iURI);

    if (!strcmp(iURI, "."))
    {
		/* Special case for the root node. */
		*oExists = OMADM_NODE_IS_INTERIOR;
	}
	else
	{
	    DMC_FAIL_NULL(plugin, prv_findPlugin(iContext, iURI),
			       OMADM_SYNCML_ERROR_SESSION_INTERNAL);

	    DMC_FAIL_NULL(fn, plugin->plugin->nodeExists,
		          OMADM_SYNCML_ERROR_NOT_ALLOWED);
	    DMC_FAIL(fn(iURI, oExists, plugin->data));
    }

	DMC_LOGF("omadm_dmtree_node_exists exit <0x%x> %d",
			    DMC_ERR, *oExists);

DMC_ON_ERR:

	return DMC_ERR;
}

int omadm_dmtree_get_children(const OMADM_DMTreeContext *iContext,
					const char *iURI,
					dmc_ptr_array *oChildren)
{
	DMC_ERR_MANAGE;
	OMADMPlugin *plugin = NULL;
	OMADM_GetNodeChildrenFN fn = NULL;
	unsigned int i = 0;
	unsigned int j = 0;
	char *childNode = NULL;
	OMADM_NodeType exists = OMADM_NODE_NOT_EXIST;

	DMC_LOGF("omadm_get_node_children <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iContext, iURI),
		      OMADM_SYNCML_ERROR_SESSION_INTERNAL);

	DMC_FAIL_NULL(fn, plugin->plugin->getNodeChildren,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, oChildren, plugin->data));

	if (strcmp(iURI, "."))
		goto DMC_ON_ERR;

	/* Special case for the root node. */

	for (i = 0; i < dmc_ptr_array_get_size(&iContext->plugins); ++i) {
		plugin =
		    (OMADMPlugin *) dmc_ptr_array_get(&iContext->
							    plugins, i);

		if (!(strcmp(plugin->URI, "./") &&
			   (((unsigned int) (strchr(plugin->URI + 2, '/') -
				plugin->URI)) == strlen(plugin->URI) - 1)))
			continue;

		DMC_FAIL_NULL(childNode, strdup(plugin->URI),
					OMADM_SYNCML_ERROR_DEVICE_FULL);

		childNode[strlen(childNode) - 1] = 0;

		for (j = 0; j < dmc_ptr_array_get_size(oChildren)
		      && strcmp(childNode, (char *) dmc_ptr_array_get
				(oChildren, j)); ++j) ;

		if (j == dmc_ptr_array_get_size(oChildren)) {
			DMC_FAIL(plugin->plugin->nodeExists(childNode,
								 &exists,
								 plugin->data));

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

DMC_ON_ERR:

	DMC_LOGF("omadm_get_node_children exit <0x%x>", DMC_ERR);

	if (childNode)
		free(childNode);

	return DMC_ERR;
}

int omadm_dmtree_get_value(const OMADM_DMTreeContext *iContext,
				    const char *iURI, char **oValue)
{
	DMC_ERR_MANAGE;
	OMADMPlugin *plugin = NULL;
	OMADM_GetValueFN fn = NULL;

	DMC_LOGF("omadm_dmtree_get_value <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iContext, iURI),
		      OMADM_SYNCML_ERROR_SESSION_INTERNAL);

	DMC_FAIL_NULL(fn, plugin->plugin->getValue,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, oValue, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("omadm_dmtree_get_value exit <0x%x>", DMC_ERR);

	return DMC_ERR;
}

int omadm_dmtree_set_value(const OMADM_DMTreeContext *iContext,
				    const char *iURI, const char *iValue)
{
	DMC_ERR_MANAGE;
	OMADMPlugin *plugin = NULL;
	OMADM_SetValueFN fn = NULL;

	DMC_LOGF("omadm_dmtree_set_value <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iContext, iURI),
		      OMADM_SYNCML_ERROR_SESSION_INTERNAL);

	DMC_FAIL_NULL(fn, plugin->plugin->setValue,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, iValue, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("omadm_dmtree_set_value exit <0x%x>", DMC_ERR);

	return DMC_ERR;
}

int omadm_dmtree_get_meta(const OMADM_DMTreeContext *iContext,
				   const char *iURI, const char *iProp,
				   char **oMeta)
{
	DMC_ERR_MANAGE;
	OMADMPlugin *plugin = NULL;
	OMADM_GetMetaFN fn = NULL;

	DMC_LOGF("omadm_dmtree_get_meta prop <%s> <%s>", iProp, iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iContext, iURI),
		      OMADM_SYNCML_ERROR_SESSION_INTERNAL);

	DMC_FAIL_NULL(fn, plugin->plugin->getMeta,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, iProp, oMeta, plugin->data));

	DMC_LOGF("omadm_dmtree_get_meta value <%s> <%s>", *oMeta);

DMC_ON_ERR:

	DMC_LOGF("omadm_dmtree_get_meta exit <%d>", DMC_ERR);

	return DMC_ERR;
}

int omadm_dmtree_set_meta(const OMADM_DMTreeContext *iContext,
				   const char *iURI, const char *iProp,
				   const char *iMeta)
{
	DMC_ERR_MANAGE;
	OMADMPlugin *plugin = NULL;
	OMADM_SetMetaFN fn = NULL;

	DMC_LOGF("omadm_dmtree_set_meta <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iContext, iURI),
		      OMADM_SYNCML_ERROR_SESSION_INTERNAL);

	DMC_FAIL_NULL(fn, plugin->plugin->setMeta,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, iProp, iMeta, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("omadm_dmtree_set_meta exit <0x%x>", DMC_ERR);

	return DMC_ERR;
}

int omadm_dmtree_create_non_leaf(const OMADM_DMTreeContext *iContext,
				 const char *iURI)
{
	DMC_ERR_MANAGE;
	OMADMPlugin *plugin = NULL;
	OMADM_CreateNonLeafFN fn = NULL;

	DMC_LOGF("%s  <%s>", __FUNCTION__, iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iContext, iURI),
		      OMADM_SYNCML_ERROR_SESSION_INTERNAL);

	DMC_FAIL_NULL(fn, plugin->plugin->createNonLeaf,
			   OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("%s exit <0x%x>", __FUNCTION__, DMC_ERR);

	return DMC_ERR;
}

int omadm_dmtree_get_access_rights(const OMADM_DMTreeContext *iContext,
					const char *iURI,
					OMADM_AccessRights *oRights)
{
	DMC_ERR_MANAGE;
	OMADMPlugin *plugin = NULL;
	OMADM_GetAccessRightsFN fn = NULL;

	DMC_LOGF("omadm_dmtree_get_access_rights  <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iContext, iURI),
		      OMADM_SYNCML_ERROR_SESSION_INTERNAL);

	DMC_FAIL_NULL(fn, plugin->plugin->getAccessRights,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, oRights, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("omadm_dmtree_get_access_rights exit <0x%x>", DMC_ERR);

	return DMC_ERR;
}

int omadm_dmtree_delete_node(const OMADM_DMTreeContext *iContext,
				const char *iURI)
{
	DMC_ERR_MANAGE;
	OMADMPlugin *plugin = NULL;
	OMADM_DeleteNodeFN fn = NULL;

	DMC_LOGF("omadm_dmtree: omadm_dmtree_delete_node <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iContext, iURI),
		      OMADM_SYNCML_ERROR_SESSION_INTERNAL);

	DMC_FAIL_NULL(fn, plugin->plugin->deleteNode,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("omadm_dmtree: omadm_dmtree_delete_node exit <0x%x>", DMC_ERR);

	return DMC_ERR;
}

int omadm_dmtree_exec_node(const OMADM_DMTreeContext *iContext,
				const char *iURI, const char *iData,
				const char *iCorrelator)
{
	DMC_ERR_MANAGE;
	OMADMPlugin *plugin = NULL;
	OMADM_ExecNodeFN fn = NULL;

	DMC_LOGF("omadm_dmtree: omadm_dmtree_exec_node <%s>", iURI);

	DMC_FAIL_NULL(plugin, prv_findPlugin(iContext, iURI),
		      OMADM_SYNCML_ERROR_SESSION_INTERNAL);

	DMC_FAIL_NULL(fn, plugin->plugin->execNode,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn(iURI, iData, iCorrelator, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("omadm_dmtree: omadm_dmtree_exec_node exit <0x%x>",
			DMC_ERR);

	return DMC_ERR;
}

int omadm_dmtree_update_nonce(const OMADM_DMTreeContext *iContext,
				       const char *iServerID,
				       const uint8_t *iNonce,
				       unsigned int iNonceLength,
				       bool iServerCred)
{
	DMC_ERR_MANAGE;
	OMADMPlugin *plugin = NULL;
	OMADM_UpdateNonceFN fn = NULL;

	DMC_LOG("omadm_dmtree_update_nonce");

	DMC_FAIL_NULL(plugin, prv_findPlugin(iContext, "./CONFIG"),
		      OMADM_SYNCML_ERROR_SESSION_INTERNAL);

	DMC_FAIL_NULL(fn, plugin->plugin->updateNonce,
		      OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(fn
		 (iServerID, iNonce, iNonceLength, iServerCred, plugin->data));

DMC_ON_ERR:

	DMC_LOGF("omadm_dmtree: omadm_dmtree_update_nonce exit <0x%x>",
		     DMC_ERR);

	return DMC_ERR;
}

int omadm_dmtree_find_inherited_acl(OMADM_DMTreeContext *iContext,
					     const char *iURI, char **oACL)
{
	DMC_ERR_MANAGE;

	char *uri = NULL;
	unsigned int uriLen = 0;

	DMC_LOGF("omadm_dmtree_find_inherited_acl <%s>", iURI);

	*oACL = NULL;

	DMC_FAIL_NULL(uri, strdup(iURI),
		      OMADM_SYNCML_ERROR_DEVICE_FULL);
	uriLen = strlen(uri);

	DMC_ERR = omadm_dmtree_get_meta(iContext, uri,
					  OMADM_NODE_PROPERTY_ACL, oACL);

	while ((uriLen > 0) && (DMC_ERR != OMADM_SYNCML_ERROR_NONE)) {
		while (uriLen > 0)
			if (uri[--uriLen] == '/') {
				uri[uriLen] = 0;
				break;
			}

		DMC_ERR = omadm_dmtree_get_meta(iContext, uri,
						  OMADM_NODE_PROPERTY_ACL,
						  oACL);
	}

    if (DMC_ERR != OMADM_SYNCML_ERROR_NONE
        && !strcmp(uri, "."))
    {
		/* Special case for the root node when no root plugin is present. */
		DMC_FAIL_NULL(*oACL, strdup("Get=*"),
		      OMADM_SYNCML_ERROR_DEVICE_FULL);
	}

DMC_ON_ERR:

	if (uri)
		free(uri);

	DMC_LOGF("omadm_dmtree_find_inherited_acl <%d>", DMC_ERR);

	return DMC_ERR;
}
