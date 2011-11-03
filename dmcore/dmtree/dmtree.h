/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file dmtree.h 
 *
 * @brief Public headers to access the OMA DM Tree
 *
 */

#ifndef OMADM_DMTREE_PRV_H_
#define OMADM_DMTREE_PRV_H_

#include "dmsettings.h"

#include "dmtree_plugin.h"

typedef struct _OMADM_DMTreePluginHolder OMADM_DMTreePluginHolder;
struct _OMADM_DMTreePluginHolder {
	char *URI;
	OMADM_DMTreePlugin plugin;
};

typedef struct _OMADM_DMTreeContext OMADM_DMTreeContext;
struct _OMADM_DMTreeContext {
	dmc_ptr_array plugins;
	const char *serverID;
	dmsettings *settings;
};

int omadm_dmtree_create(const char *iServerID, OMADM_DMTreeContext **oContext);
int omadm_dmtree_add_plugin(OMADM_DMTreeContext *iContext, const char *iURI,
				OMADM_DMTreePlugin *iPlugin);

void omadm_dmtree_free_plugin(OMADM_DMTreePlugin *oPlugin);
int omadm_dmtree_init(OMADM_DMTreeContext *iContext);

void omadm_dmtree_free(OMADM_DMTreeContext *oContext);

int omadm_dmtree_exists(const OMADM_DMTreeContext *iContext, 
			const char *iUri, OMADM_NodeType *oExists);
int omadm_dmtree_supports_transactions(OMADM_DMTreeContext *context,
					const char *uri, 
					bool *supports_transactions);
int omadm_dmtree_get_children(const OMADM_DMTreeContext *iContext, 
				const char *iUri, 
				dmc_ptr_array *oChildren);

int omadm_dmtree_get_value(const OMADM_DMTreeContext *iContext,
			   const char *iUri,  char **oValue);
int omadm_dmtree_set_value(const OMADM_DMTreeContext *iContext, 
			   const char *iUri, const char *iValue);

int omadm_dmtree_get_meta(const OMADM_DMTreeContext *iContext, const char *iUri,
			  const char *iProp, char **oMeta);
int omadm_dmtree_set_meta(const OMADM_DMTreeContext *iContext, const char *iUri, 
			  const char *iProp, const char *iMeta);

int omadm_dmtree_create_non_leaf(const OMADM_DMTreeContext *iContext,
					const char *iUri);

int omadm_dmtree_get_access_rights(const OMADM_DMTreeContext *iContext, 
					const char *iUri, 
					OMADM_AccessRights *oRights);

int omadm_dmtree_delete_node(const OMADM_DMTreeContext *iContext, 
				const char *iUri);
int omadm_dmtree_exec_node(const OMADM_DMTreeContext *iContext, 
				const char *iUri, const char *iData, 
				const char *iCorrelator);

int omadm_dmtree_update_nonce(const OMADM_DMTreeContext *iContext,
				const char *iServerID, const uint8_t *iNonce,
				unsigned int iNonceLength, bool iServerCred);

int omadm_dmtree_find_inherited_acl(OMADM_DMTreeContext *iContext, 
				    const char *iURI, char **oACL);

#endif
