/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/
/*!
 * @file transaction.c
 *
 * @brief OMA Device Management transaction management.
 *
 *****************************************************************************/

#include "config.h"

#include "error.h"

#include "transaction.h"

/* TODO transactions are not implemented yet */


void omadm_transaction_make(omadm_transaction* transaction, 
			    OMADM_DMTreeContext *dmtree)
{
	transaction->transaction_in_progress = false;
	transaction->dm_tree = dmtree;
}

bool omadm_transaction_in_progress(omadm_transaction* transaction)
{
	return transaction->transaction_in_progress;
}

int omadm_transaction_begin(omadm_transaction* transaction)
{
	return DMC_ERR_NONE;
}

int omadm_transaction_cancel(omadm_transaction* transaction)
{
	return DMC_ERR_NONE;
}

int omadm_transaction_commit(omadm_transaction* transaction)
{
	return DMC_ERR_NONE;
}

int omadm_transaction_exists(omadm_transaction* transaction, 
			     const char *uri, OMADM_NodeType *exists)
{
	return omadm_dmtree_exists(transaction->dm_tree, uri, exists);
}

int omadm_transaction_children(omadm_transaction* transaction,
			       const char *uri, dmc_ptr_array *children)
{
	return omadm_dmtree_get_children(transaction->dm_tree, uri,
					 children);
}

int omadm_transaction_get_value(omadm_transaction* transaction, const char *uri,
				char **value)
{
	return omadm_dmtree_get_value(transaction->dm_tree, uri, value);
}

int omadm_transaction_set_value(omadm_transaction* transaction, const char *uri,
				const char *value)
{
	return omadm_dmtree_set_value(transaction->dm_tree, uri, value);
}

int omadm_transaction_get_meta(omadm_transaction* transaction,  const char *uri,
			       const char *prop, char **meta)
{
	return omadm_dmtree_get_meta(transaction->dm_tree, uri, prop,
				     meta);
}

int omadm_transaction_set_meta(omadm_transaction* transaction, const char *uri, 
			       const char *prop, const char *meta)
{
	return omadm_dmtree_set_meta(transaction->dm_tree, uri, prop,
				     meta);
}

int omadm_transaction_get_access_rights(omadm_transaction* transaction,  
					const char *uri, OMADM_AccessRights *rights)
{
	return omadm_dmtree_get_access_rights(transaction->dm_tree, uri,
					      rights);
}

int omadm_transaction_delete_node(omadm_transaction* transaction, const char *uri)
{
	return omadm_dmtree_delete_node(transaction->dm_tree, uri);
}

int omadm_transaction_exec_node(omadm_transaction* transaction, 
				const char *uri, const char *data, 
				const char *correlator)
{
	/* Not possible in a transaction */

	return omadm_dmtree_exec_node(transaction->dm_tree, uri, data,
				      correlator);
}

int omadm_transaction_update_nonce(omadm_transaction* transaction,
				   const char *server_id, const uint8_t *nonce,
				   unsigned int nonce_length, bool server_cred)
{
	/* Not possible in a transaction. Happens at the end of the dm session */

	return omadm_dmtree_update_nonce(transaction->dm_tree, server_id,
					 nonce, nonce_length, server_cred);
}

int omadm_transaction_find_inherited_acl(omadm_transaction* transaction, 
					 const char *uri, char **acl)
{
	return omadm_dmtree_find_inherited_acl(transaction->dm_tree, uri, acl);
}

int omadm_transaction_create_non_leaf(omadm_transaction* transaction, 
				      const char *uri)
{
	return omadm_dmtree_create_non_leaf(transaction->dm_tree, uri);
}

