/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2006 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/
/*!
 * @file transaction.h
 *
 * @brief
 * OMA Device Management transaction management.
 *
 ******************************************************************************/

#ifndef OMADM_TRANSACTION_H
#define OMADM_TRANSACTION_H

#include <stdbool.h>

#include "dmtree.h"

typedef struct omadm_transaction_ omadm_transaction;
struct omadm_transaction_
{
	bool transaction_in_progress;
	OMADM_DMTreeContext *dm_tree;
};

void omadm_transaction_make(omadm_transaction* transaction, 
			    OMADM_DMTreeContext *dmtree);
bool omadm_transaction_in_progress(omadm_transaction* transaction);
int omadm_transaction_begin(omadm_transaction* transaction);
int omadm_transaction_cancel(omadm_transaction* transaction);
int omadm_transaction_commit(omadm_transaction* transaction);
int omadm_transaction_exists(omadm_transaction* transaction, 
			     const char *uri, OMADM_NodeType *exists);
int omadm_transaction_children(omadm_transaction* transaction,
			       const char *uri, dmc_ptr_array *children);
int omadm_transaction_get_value(omadm_transaction* transaction, const char *uri,
				char **value);
int omadm_transaction_set_value(omadm_transaction* transaction, const char *uri,
				const char *value);
int omadm_transaction_get_meta(omadm_transaction* transaction,  const char *uri,
			       const char *prop, char **meta);
int omadm_transaction_set_meta(omadm_transaction* transaction, const char *uri, 
			       const char *prop, const char *meta);
int omadm_transaction_get_access_rights(omadm_transaction* transaction,  
					const char *uri, OMADM_AccessRights *rights);
int omadm_transaction_delete_node(omadm_transaction* transaction, const char *uri);
int omadm_transaction_exec_node(omadm_transaction* transaction, 
				const char *uri, const char *data, 
				const char *correlator);
int omadm_transaction_update_nonce(omadm_transaction* transaction,
				   const char *serverID, const uint8_t *nonce,
				   unsigned int nonce_length, bool server_cred);
int omadm_transaction_find_inherited_acl(omadm_transaction* transaction, 
					 const char *uri, char **acl);
int omadm_transaction_create_non_leaf(omadm_transaction* transaction, 
				      const char *uri);

#endif				/* OMADM_TRANSACTION_H */
