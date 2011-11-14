/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file momgr.h
 *
 * @brief Headers to manage the Management Objects
 *
 */

#ifndef OMADM_MOMGR_PRV_H_
#define OMADM_MOMGR_PRV_H_

#include "dyn_buf.h"
#include "dmtree_plugin.h"

typedef struct
{
	char *URI;
	OMADM_DMTreePlugin *plugin;
	void *data;
	void *dl_handle;
} dmtree_plugin_t;

typedef struct _plugin_elem
{
    dmtree_plugin_t * plugin;
    struct _plugin_elem * next;
} plugin_elem_t;

typedef struct
{
    plugin_elem_t * first;
} mo_list_t;


int momgr_init(mo_list_t * iListP);
void momgr_free(mo_list_t iList);

int momgr_add_plugin(mo_list_t * iList, const char *iURI, OMADM_DMTreePlugin *iPlugin);
void momgr_load_plugin(mo_list_t * iList, const char *iFilename);

void momgr_free_plugin(OMADM_DMTreePlugin *oPlugin);

int momgr_exists(const mo_list_t iList,
			const char *iUri, OMADM_NodeType *oExists);
int momgr_supports_transactions(const mo_list_t iList,
					const char *uri,
					bool *supports_transactions);
int momgr_get_children(const mo_list_t iList,
				const char *iUri,
				dmc_ptr_array *oChildren);

int momgr_get_value(const mo_list_t iList,
			   const char *iUri,  char **oValue);
int momgr_set_value(const mo_list_t iList,
			   const char *iUri, const char *iValue);

int momgr_get_meta(const mo_list_t iList, const char *iUri,
			  const char *iProp, char **oMeta);
int momgr_set_meta(const mo_list_t iList, const char *iUri,
			  const char *iProp, const char *iMeta);

int momgr_create_non_leaf(const mo_list_t iList,
					const char *iUri);

int momgr_get_access_rights(const mo_list_t iList,
					const char *iUri,
					OMADM_AccessRights *oRights);

int momgr_delete_node(const mo_list_t iList,
				const char *iUri);
int momgr_exec_node(const mo_list_t iList,
				const char *iUri, const char *iData,
				const char *iCorrelator);

int momgr_update_nonce(const mo_list_t iList,
				const char *iServerID, const uint8_t *iNonce,
				unsigned int iNonceLength, bool iServerCred);

int momgr_find_inherited_acl(mo_list_t iList,
				    const char *iURI, char **oACL);

#endif
