/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007, ACCESS Systems Americas, Inc. All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file dmsession.c
 *
 * @brief Main source file for dmsession.
 *
 *****************************************************************************/

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>

#include "error_macros.h"
#include "log.h"
#include "dyn_buf.h"
#include "dmtree.h"
#include "dmsession.h"
#include "internals.h"

// Temporary
#define OMADM_DEVDETAILS_MAX_SEG_LEN 64
#define OMADM_DEVDETAILS_MAX_TOT_LEN 256
#define OMADM_DEVDETAILS_MAX_DEPTH_LEN 16

#include "transaction.h"
#include "syncml_error.h"

struct dmtree_session_ {
	OMADM_DMTreeContext *dmtree;
	omadm_transaction transaction;
	char *server_id;
};

/* Global constants */

#define OMADM_XML_FORMAT_VAL		"xml"
#define OMADM_NODE_FORMAT_VAL		"node"
#define OMADM_DMTNDS_XML_TYPE_VAL	"application/vnd.syncml.dmtnds+xml"
#define OMADM_DMTNDS_WBXML_TYPE_VAL	"application/vnd.syncml.dmtnds+wbxml"

#define OMADM_REPLACE_ACL		"Add=%s&Delete=%s&Replace=%s&Get=%s"

#define OMADM_COMMAND_ADD 		"Add="
#define OMADM_COMMAND_GET 		"Get="
#define OMADM_COMMAND_REPLACE 		"Replace="
#define OMADM_COMMAND_EXECUTE		"Exec="
#define OMADM_COMMAND_DELETE		"Delete="

#define OMADM_TNDS_STRUCT		(uint8_t) 1
#define OMADM_TNDS_STRUCT_DATA		(uint8_t) 2
#define OMADM_TNDS_ALL			(uint8_t) 3

#define OMADM_PROP_ALL			(uint32_t) 0xFFFFFFFF
#define OMADM_PROP_NONE			(uint32_t) 0
#define OMADM_PROP_ACL			(uint32_t) 1 << 0
#define OMADM_PROP_FORMAT		(uint32_t) 1 << 1
#define OMADM_PROP_NAME			(uint32_t) 1 << 2
#define OMADM_PROP_SIZE			(uint32_t) 1 << 3
#define OMADM_PROP_TITLE		(uint32_t) 1 << 4
#define OMADM_PROP_TSTAMP		(uint32_t) 1 << 5
#define OMADM_PROP_TYPE			(uint32_t) 1 << 6
#define OMADM_PROP_VERNO		(uint32_t) 1 << 7
#define OMADM_PROP_VALUE		(uint32_t) 1 << 8

#define OMADM_ACL_GET_SET		(uint8_t) 1 << 0
#define OMADM_ACL_ADD_SET		(uint8_t) 1 << 1
#define OMADM_ACL_DELETE_SET		(uint8_t) 1 << 2
#define OMADM_ACL_EXEC_SET		(uint8_t) 1 << 3
#define OMADM_ACL_REPLACE_SET		(uint8_t) 1 << 4

static int prv_read_leaf_node(dmtree_session *session, const char *uri,
				dmtree_node **leaf_node)
{
	DMC_ERR_MANAGE;
	dmtree_node *node;
	char *value = NULL;

	DMC_FAIL_NULL(node, malloc(sizeof(*node)),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);

	memset(node, 0, sizeof(*node));

	DMC_FAIL_NULL(node->target_uri, strdup(uri),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);

	DMC_FAIL(omadm_transaction_get_meta(&session->transaction, uri,
						 OMADM_NODE_PROPERTY_FORMAT,
						 &node->format));

	DMC_FAIL(omadm_transaction_get_meta(&session->transaction,
						 uri, OMADM_NODE_PROPERTY_TYPE,
						 &node->type));

	DMC_FAIL(omadm_transaction_get_value(&session->transaction, uri,
						  &value));

	node->data_size = strlen(value);
	node->data_buffer = (uint8_t *) value;

	*leaf_node = node;
	node = NULL;

DMC_ON_ERR:

	dmtree_node_free(node);

	return DMC_ERR;
}

int dmtree_validate_uri(const char *uri, bool allow_props)
{
	DMC_ERR_MANAGE;

	unsigned int segments = 1;
	char *uri_ptr;
	char *old_uri_ptr;
	char *prop_ptr;
	unsigned int seg_len;
	unsigned int uri_len = strlen(uri);
	const char *question;

	if (uri_len > OMADM_DEVDETAILS_MAX_TOT_LEN)
		DMC_FAIL(OMADM_SYNCML_ERROR_URI_TOO_LONG);

	if (strcmp(uri, ".")) {
		if (uri_len < 2)
			DMC_FAIL(OMADM_SYNCML_ERROR_NOT_FOUND);
		else if ((strncmp(uri, "./", 2) && strncmp(uri, ".?", 2))
			 || (uri[uri_len - 1] == '/'))
			DMC_FAIL(OMADM_SYNCML_ERROR_NOT_FOUND);
		else {
			question = strchr(uri, '?');
			if (question) {
				if (!allow_props)
					DMC_FAIL(OMADM_SYNCML_ERROR_NOT_ALLOWED);
				else if (*(question - 1) == '/')
					DMC_FAIL(OMADM_SYNCML_ERROR_NOT_FOUND);
			}
		}
	}

	/* Let's count the segments */

	uri_ptr = strchr(uri, '/');

	while (uri_ptr) {
		++segments;
		old_uri_ptr = uri_ptr + 1;
		uri_ptr = strchr(old_uri_ptr, '/');

		if (uri_ptr)
			seg_len = (unsigned int) (uri_ptr - old_uri_ptr);
		else {
			prop_ptr = strchr(old_uri_ptr, '?');
			if (prop_ptr)
				seg_len = (unsigned int) (prop_ptr - old_uri_ptr);
			else
				seg_len = strlen(old_uri_ptr);
		}

		if (seg_len > OMADM_DEVDETAILS_MAX_SEG_LEN)
			DMC_FAIL(OMADM_SYNCML_ERROR_URI_TOO_LONG);
	}

	if (segments > OMADM_DEVDETAILS_MAX_DEPTH_LEN)
		DMC_FAIL(OMADM_SYNCML_ERROR_URI_TOO_LONG);

DMC_ON_ERR:

	DMC_LOGF("%s exitted with error %d",__FUNCTION__, DMC_ERR);

	return DMC_ERR;
}

static int prv_validate_access_rights(dmtree_session *session, const char *uri,
				      const char *cmd_name)
{
	DMC_ERR_MANAGE;

	int allowed = 0;
	OMADM_AccessRights rights = 0;

	DMC_FAIL(omadm_transaction_get_access_rights(&session->transaction,
							  uri, &rights));

	if (!strcmp(cmd_name, OMADM_COMMAND_GET))
		allowed = rights & OMADM_ACCESS_GET;
	else if (!strcmp(cmd_name, OMADM_COMMAND_ADD))
		allowed = rights & OMADM_ACCESS_ADD;
	else if (!strcmp(cmd_name, OMADM_COMMAND_REPLACE))
		allowed = rights & OMADM_ACCESS_REPLACE;
	else if (!strcmp(cmd_name, OMADM_COMMAND_DELETE))
		allowed = rights & OMADM_ACCESS_DELETE;
	else if (!strcmp(cmd_name, OMADM_COMMAND_EXECUTE))
		allowed = rights & OMADM_ACCESS_EXEC;

	if (!allowed)
		DMC_ERR = OMADM_SYNCML_ERROR_NOT_ALLOWED;

DMC_ON_ERR:

	return DMC_ERR;
}

static int prv_access_granted(dmtree_session* session,
			      const char *acl, const char *cmd_name,
			      bool force_check, bool *granted)
{
	DMC_ERR_MANAGE;
	char *cmd_begin;
	char *cmd_end;
	bool access_granted;
	char *acl_copy = NULL;
	const char *server_id;
	char *save_ptr = NULL;

	/* ACL checks may need to be disabled when adding TNDS docs */

	if (force_check)
		access_granted = true;
	else
	{
		access_granted = false;

		DMC_FAIL_NULL(acl_copy, strdup(acl),
				   OMADM_SYNCML_ERROR_DEVICE_FULL);

		cmd_begin = strstr(acl_copy, cmd_name);
		if (cmd_begin)
		{
			/* ACL contains this command */

			cmd_begin += strlen(cmd_name);
			cmd_end = strstr(cmd_begin, "&");
			if (cmd_end)
				*cmd_end = 0;

			/* Check if "*" */

			if (!strcmp(cmd_begin, "*"))
				access_granted = true;
			else
			{
				/* Look for iServerId
				   It's list separated by "+" */

				server_id = strtok_r(cmd_begin, "+", &save_ptr);

				while (server_id) {
					if (!strcmp(server_id,
						    session->server_id)) {
						access_granted = true;
						break;
					}
					server_id = strtok_r(NULL, "+", &save_ptr);
				}
			}
		}
	}

	*granted = access_granted;

DMC_ON_ERR:

	if (acl_copy)
		free(acl_copy);

	return DMC_ERR;
}


static int prv_check_node_acl_rights(dmtree_session *session,
				     const char *acl_uri,
				     const char *cmd_name,
				     bool force_check)
{
	DMC_ERR_MANAGE;

	char *acl = NULL;
	bool access_granted;

	/* Get ACL Value for this node and command */

	DMC_FAIL(omadm_transaction_find_inherited_acl(&session->transaction,
							   acl_uri, &acl));

	DMC_FAIL(prv_access_granted(session, acl, cmd_name,
					 force_check, &access_granted));

	if (!access_granted)
		DMC_ERR = OMADM_SYNCML_ERROR_PERMISSION_DENIED;

DMC_ON_ERR:

	if (acl)
		free(acl);

	return DMC_ERR;
}

static int prv_check_node_access_rights(dmtree_session *session,
					const char *uri,
					const char *acl_uri,
					const char *cmd_name,
					bool force_check)
{
	DMC_ERR_MANAGE;

	DMC_FAIL(prv_validate_access_rights(session, uri, cmd_name));
	DMC_FAIL(prv_check_node_acl_rights(session, acl_uri, cmd_name,
						force_check));

DMC_ON_ERR:

	return DMC_ERR;
}

static int prv_build_child_list(dmtree_session *session, const char *uri,
				char **child_list)
{
	DMC_ERR_MANAGE;
	dmc_buf buff;
	bool empty = true;
	const char *path;
	const char *node_name;
	unsigned int i = 0;
	dmc_ptr_array children;

	DMC_LOGF("%s called for <%s>", __FUNCTION__, uri);

	dmc_buf_make(&buff, 128);
	dmc_ptr_array_make(&children, 16, free);

	DMC_FAIL(omadm_transaction_children(&session->transaction,
							uri, &children));

	for (i = 0; i < dmc_ptr_array_get_size(&children); ++i) {
		path = dmc_ptr_array_get(&children, i);

		node_name = strrchr(path,'/');
		if (node_name) {
			DMC_FAIL(DMC_ERR);
			if (!empty)
				DMC_FAIL(dmc_buf_append_str
					      (&buff, "/"));
			DMC_FAIL(
				dmc_buf_append_str(&buff,
							       node_name+1));
			empty = false;
		}
	}

	if (!empty) {
		DMC_FAIL(dmc_buf_zero_terminate(&buff));
		*child_list = (char*) dmc_buf_adopt(&buff);
	} else
		DMC_FAIL_NULL(*child_list, strdup(""),
				   OMADM_SYNCML_ERROR_DEVICE_FULL);

DMC_ON_ERR:

	dmc_ptr_array_free(&children);
	dmc_buf_free(&buff);

	DMC_LOGF("%s exit.  Error <%d>", __FUNCTION__, DMC_ERR);

	return DMC_ERR;
}

static int prv_read_non_leaf_node(dmtree_session *session, const char *uri,
					dmtree_node **non_leaf_node)
{
	DMC_ERR_MANAGE;
	dmtree_node *node;
	char *value = NULL;

	DMC_FAIL_NULL(node, malloc(sizeof(*node)),
				OMADM_SYNCML_ERROR_DEVICE_FULL);

	memset(node, 0, sizeof(*node));

	DMC_FAIL_NULL(node->target_uri, strdup(uri),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);

	DMC_FAIL(prv_build_child_list(session, uri, &value));

	DMC_FAIL_NULL(node->format, strdup(OMADM_NODE_FORMAT_VAL),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);

	DMC_FAIL_NULL(node->type, strdup("text/plain"),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);

	node->data_size = strlen(value);
	node->data_buffer = (uint8_t *) value;

	*non_leaf_node = node;

	return OMADM_SYNCML_ERROR_NONE;

DMC_ON_ERR:

	free(value);
	dmtree_node_free(node);

	return DMC_ERR;
}

static int prv_read_prop_node(dmtree_session *session, const char *node_uri,
			      const char *uri, const char *prop,
			      dmtree_node **prop_node)
{
	DMC_ERR_MANAGE;
	dmtree_node *node;
	char *value = NULL;
	const char *node_name;

	DMC_FAIL_NULL(node, malloc(sizeof(*node)),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);

	memset(node, 0, sizeof(*node));

	DMC_FAIL_NULL(node->target_uri, strdup(uri),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);

	DMC_FAIL_NULL(node->format, strdup("chr"),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);

	DMC_FAIL_NULL(node->type, strdup("text/plain"),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);

	if (!strcmp(prop, OMADM_NODE_PROPERTY_NAME)) {
		node_name = strcmp(node_uri, ".") ? strrchr(node_uri, '/') + 1 : ".";
		DMC_FAIL_NULL(value, strdup(node_name),
				   OMADM_SYNCML_ERROR_DEVICE_FULL);
	} else {
		DMC_ERR = omadm_transaction_get_meta(&session->transaction,
							    node_uri, prop,
							    &value);

		if (DMC_ERR == OMADM_SYNCML_ERROR_NOT_FOUND)
			DMC_FAIL_NULL(value, strdup(""),
					   OMADM_SYNCML_ERROR_DEVICE_FULL);
		else
			DMC_FAIL(DMC_ERR);
	}

	node->data_size = strlen(value);
	node->data_buffer = (uint8_t *) value;

	*prop_node = node;

	return OMADM_SYNCML_ERROR_NONE;

DMC_ON_ERR:

	free(value);
	dmtree_node_free(node);

	return DMC_ERR;
}

static int prv_get_node(dmtree_session* session, OMADM_NodeType node_exists,
			const char *uri, dmtree_node **node)
{
	DMC_ERR_MANAGE;

	DMC_LOGF("reading node %s", uri);

	if (node_exists == OMADM_NODE_IS_LEAF) {
		DMC_LOG("Reading leaf node");

		DMC_FAIL(prv_read_leaf_node(session, uri, node));
	} else {

		DMC_LOG("Building child list");

		DMC_FAIL(prv_read_non_leaf_node(session, uri, node));
	}

	DMC_LOGF("value %s format %s type %s", (*node)->data_buffer,
		      (*node)->format, (*node)->type);

DMC_ON_ERR:

	return DMC_ERR;
}

static int prv_get_prop(dmtree_session *session, const char *node_uri,
			const char *uri, dmtree_node **node)
{
	DMC_ERR_MANAGE;
	char *prop_name = strstr(uri, "=") + 1;

	DMC_LOGF("reading property %s", uri);

	if (!strcmp(prop_name, OMADM_NODE_PROPERTY_SIZE) ||
		   !strcmp(prop_name, OMADM_NODE_PROPERTY_VERSION) ||
		   !strcmp(prop_name, OMADM_NODE_PROPERTY_SIZE) ||
		   !strcmp(prop_name, OMADM_NODE_PROPERTY_TITLE))
		DMC_FAIL(OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED);
	else
		DMC_FAIL(prv_read_prop_node(session, node_uri, uri,
						 prop_name, node));

	DMC_LOGF("value %s format %s type %s", (*node)->data_buffer,
		      (*node)->format, (*node)->type);

DMC_ON_ERR:

	return DMC_ERR;
}

int dmtree_session_get(dmtree_session *session, const char *uri, dmtree_node **node)
{
	DMC_ERR_MANAGE;

	OMADM_NodeType node_exists;
	char *node_uri = NULL;
	char *tmp_ptr;

	DMC_LOGF("%s called. URI %s", __FUNCTION__, uri);

	DMC_FAIL(dmtree_validate_uri(uri, true));

	DMC_FAIL_NULL(node_uri, strdup(uri),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);

	tmp_ptr = strstr(node_uri, "?");
	if (tmp_ptr)
		*tmp_ptr = 0;

	DMC_FAIL(omadm_transaction_exists(&session->transaction, node_uri,
					       &node_exists));

	if (node_exists == OMADM_NODE_NOT_EXIST)
		DMC_FAIL(OMADM_SYNCML_ERROR_NOT_FOUND);

	DMC_FAIL(prv_check_node_access_rights(session, node_uri, node_uri,
						   OMADM_COMMAND_GET, false));

	if (!strstr(uri, "?"))
		DMC_FAIL(prv_get_node(session, node_exists, uri, node));
	else if (strstr(uri, "?list=") != NULL)
	{
		DMC_FAIL(
			OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED);
		DMC_LOG("?list not supported");
	}
	else
		DMC_FAIL(prv_get_prop(session, node_uri, uri, node));

DMC_ON_ERR:

	DMC_LOGF("%s finished with error %d",__FUNCTION__, DMC_ERR);

	if (node_uri)
		free(node_uri);

	return DMC_ERR;
}

static int prv_read_and_add_node(dmtree_session *session, const char *uri,
				 dmc_ptr_array *list)
{
	DMC_ERR_MANAGE;
	dmtree_node* node;

	DMC_FAIL(prv_read_leaf_node(session, uri, &node));

	DMC_FAIL_ERR(dmc_ptr_array_append(list, node),
			    OMADM_SYNCML_ERROR_DEVICE_FULL);

	node = NULL;

DMC_ON_ERR:

	dmtree_node_free(node);

	return DMC_ERR;
}

int dmtree_session_delete(dmtree_session *session, const char *uri)
{
	DMC_ERR_MANAGE;
	OMADM_NodeType node_exists;

	DMC_LOGF("%s called.", __FUNCTION__);

	if (!uri)
		DMC_FAIL(OMADM_SYNCML_ERROR_COMMAND_FAILED);

	/* TODO: Copy strip everything uri */

	DMC_LOGF("deleting %s", uri);

	DMC_FAIL(dmtree_validate_uri(uri, false));

	DMC_FAIL(omadm_transaction_exists(&session->transaction,
					       uri, &node_exists));

	if (node_exists == OMADM_NODE_NOT_EXIST)
		DMC_FAIL(OMADM_SYNCML_ERROR_NOT_FOUND);

	DMC_FAIL(prv_check_node_access_rights(session, uri, uri,
						   OMADM_COMMAND_DELETE,
						   false));

	DMC_FAIL(omadm_transaction_delete_node(&session->transaction,uri));

DMC_ON_ERR:

	DMC_LOGF("%s finished with error %d",__FUNCTION__, DMC_ERR);

	return DMC_ERR;
}

static int prv_find_subtree_ancestor(dmtree_session *session, const char *uri,
				     char **subtree_parent, char **subtree_root,
				     OMADM_NodeType *subtree_parent_type,
				     unsigned int *subtree_depth)
{
	DMC_ERR_MANAGE;

	char *str = NULL;
	char *tok = NULL;
	char *parent;
	OMADM_NodeType node_exists = OMADM_NODE_NOT_EXIST;
	unsigned int depth = 0;

	DMC_FAIL_NULL(str, strdup(uri), OMADM_SYNCML_ERROR_DEVICE_FULL);

	/* Find the highest implicit parent interior node with no ACL */

	while ((node_exists == OMADM_NODE_NOT_EXIST) &&
			(tok = strrchr(str, '/'))) {

		*tok = 0;
		DMC_FAIL(omadm_transaction_exists(&session->transaction,
						       str, &node_exists));
		++depth;
	}

	DMC_FAIL_NULL(parent, strdup(str), OMADM_SYNCML_ERROR_DEVICE_FULL);

	*subtree_parent = parent;
	*subtree_parent_type = node_exists;
	if (tok) *tok = '/';
	*subtree_root = str;
	*subtree_depth = depth;

	return OMADM_SYNCML_ERROR_NONE;

DMC_ON_ERR:

	free(str);

	return DMC_ERR;
}

static int prv_update_replace_acl(dmtree_session *session, const char *uri)
{
	DMC_ERR_MANAGE;

	char *acl = NULL;
	const char *acl_server_id = session->server_id;

	/* Set the ACL to Add/Delete/Replace/Get for this server */

	DMC_FAIL_NULL(acl,
			   malloc(strlen(OMADM_REPLACE_ACL) +
				  (4 * strlen(acl_server_id))),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);

	sprintf(acl, OMADM_REPLACE_ACL, acl_server_id, acl_server_id,
		acl_server_id, acl_server_id);

	DMC_FAIL(omadm_transaction_set_meta(&session->transaction,
						 uri,
						 OMADM_NODE_PROPERTY_ACL,
						 acl));

DMC_ON_ERR:

	if (acl)
		free(acl);

	return DMC_ERR;
}

static int prv_update_leaf_node(dmtree_session *session, const dmtree_node *node)
{
	DMC_ERR_MANAGE;

	DMC_FAIL(omadm_transaction_set_value(&session->transaction,
						  node->target_uri,
						  (char*) node->data_buffer));

	if (node->format && strcmp(node->format,"chr"))
		DMC_FAIL(omadm_transaction_set_meta(&session->transaction,
							 node->target_uri,
							 OMADM_NODE_PROPERTY_FORMAT,
							 node->format));

	if (node->type && strcmp(node->type,"text/plain"))
		DMC_FAIL(omadm_transaction_set_meta(&session->transaction,
							 node->target_uri,
							 OMADM_NODE_PROPERTY_TYPE,
							 node->type));

DMC_ON_ERR:

	return DMC_ERR;
}

static int prv_add_leaf_node(dmtree_session *session, const dmtree_node *node,
			     const char *subtree_parent, const char *subtree_root,
			     unsigned int subtree_depth)
{
	DMC_ERR_MANAGE;

	if (!node->data_buffer)
		DMC_FAIL(OMADM_SYNCML_ERROR_COMMAND_FAILED);

	DMC_ERR = prv_check_node_acl_rights(session, subtree_parent,
							OMADM_COMMAND_REPLACE,
							false);

	if (DMC_ERR == OMADM_SYNCML_ERROR_PERMISSION_DENIED)
	{
		/* We may be implicitly adding non-leaf nodes. If we are
		   we need to add replace rights to this server for the
		   highest implicit non-leaf node that we are adding */

		if (subtree_depth > 1)
			DMC_FAIL(prv_update_replace_acl(session,
								subtree_root));
	}
	else
		DMC_FAIL(DMC_ERR);

	DMC_FAIL(prv_update_leaf_node(session, node));

DMC_ON_ERR:

	return DMC_ERR;
}

static int prv_add_non_leaf_node(dmtree_session *session, const dmtree_node *node,
					const char *subtree_parent,
					const char *subtree_root)
{
	DMC_ERR_MANAGE;

	DMC_ERR = prv_check_node_acl_rights(session, subtree_parent,
							OMADM_COMMAND_REPLACE,
							false);

	if (DMC_ERR == OMADM_SYNCML_ERROR_PERMISSION_DENIED)
		DMC_FAIL(prv_update_replace_acl(session, subtree_root));
	else
		DMC_FAIL(DMC_ERR);

	DMC_FAIL(omadm_transaction_create_non_leaf(&session->transaction,
							node->target_uri));

DMC_ON_ERR:

	return DMC_ERR;
}

int dmtree_session_add(dmtree_session *session, const dmtree_node *node)
{
	DMC_ERR_MANAGE;
	OMADM_NodeType node_exists;
	bool non_leaf_node;
	char *subtree_parent = NULL;
	char *subtree_root = NULL;
	unsigned int subtree_depth = 0;

	DMC_LOGF("%s called.", __FUNCTION__);

	if (!node || !node->target_uri)
		DMC_FAIL(OMADM_SYNCML_ERROR_COMMAND_FAILED);

	/* TODO: Copy node and strip everything */

	DMC_LOGF("adding %s", node->target_uri);

	DMC_FAIL(dmtree_validate_uri(node->target_uri, false));

	if ((node->format) && (node->type) &&
	    (!strcmp(node->format, OMADM_XML_FORMAT_VAL)) &&
	    (!strcmp(node->type, OMADM_DMTNDS_XML_TYPE_VAL) ||
	     !strcmp(node->type, OMADM_DMTNDS_WBXML_TYPE_VAL)))
		DMC_FAIL_FORCE(
			OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED);

	DMC_FAIL(omadm_transaction_exists(&session->transaction,
					       node->target_uri,
					       &node_exists));

	if (node_exists != OMADM_NODE_NOT_EXIST)
		DMC_FAIL(OMADM_SYNCML_ERROR_ALREADY_EXISTS);

	/*
	 * Need to check the type of the subtree_parent.
	 * If this is a leaf node we cannot add.
	 */

	DMC_FAIL(prv_find_subtree_ancestor(session, node->target_uri,
						&subtree_parent, &subtree_root,
						&node_exists,
						&subtree_depth));

	DMC_LOGF("Adding subtree depth %u rooted at '%s' to '%s'.",
		      subtree_depth, subtree_root, subtree_parent);

	if (node_exists == OMADM_NODE_IS_LEAF)
		DMC_FAIL(OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(prv_check_node_access_rights(session, node->target_uri,
						   subtree_parent,
						   OMADM_COMMAND_ADD, false));

	non_leaf_node = node->format && !strcmp(OMADM_NODE_FORMAT_VAL, node->format);

	if (omadm_transaction_in_progress(&session->transaction))
	{
		if (non_leaf_node)
			DMC_FAIL(prv_add_non_leaf_node(session, node,
							    subtree_parent,
							    subtree_root));
		else
			DMC_FAIL(prv_add_leaf_node(session, node,
							subtree_parent,
							subtree_root,
							subtree_depth));
	}
	else
	{
		DMC_FAIL(omadm_transaction_begin(
				      &session->transaction));
		if (non_leaf_node)
			DMC_ERR = prv_add_non_leaf_node(session, node,
							       subtree_parent,
							       subtree_root);
		else
			DMC_ERR = prv_add_leaf_node(session, node,
							   subtree_parent,
							   subtree_root,
							   subtree_depth);

		if (DMC_ERR == OMADM_SYNCML_ERROR_NONE)
			DMC_FAIL(omadm_transaction_commit(
					      &session->transaction));
		else
		{
			(void) omadm_transaction_cancel(
				&session->transaction);
			DMC_FAIL_FORCE(DMC_ERR);
		}
	}

DMC_ON_ERR:

	free(subtree_parent);
	free(subtree_root);

	DMC_LOGF("%s finished with error %d",__FUNCTION__, DMC_ERR);

	return DMC_ERR;
}

static int prv_check_acl_command(char *command, uint8_t *cmd_field)
{
	DMC_ERR_MANAGE;

	if (!strcmp(command, "Get")) {
		if (*cmd_field & OMADM_ACL_GET_SET)
			DMC_ERR = OMADM_SYNCML_ERROR_COMMAND_FAILED;
		else
			*cmd_field |= OMADM_ACL_GET_SET;
	}
	else if (!strcmp(command, "Add")) {
		if (*cmd_field & OMADM_ACL_ADD_SET)
			DMC_ERR = OMADM_SYNCML_ERROR_COMMAND_FAILED;
		else
			*cmd_field |= OMADM_ACL_ADD_SET;
	}
	else if (!strcmp(command, "Delete")) {
		if (*cmd_field & OMADM_ACL_DELETE_SET)
			DMC_ERR = OMADM_SYNCML_ERROR_COMMAND_FAILED;
		else
			*cmd_field |= OMADM_ACL_DELETE_SET;
	}
	else if (!strcmp(command, "Exec")) {
		if (*cmd_field & OMADM_ACL_EXEC_SET)
			DMC_ERR = OMADM_SYNCML_ERROR_COMMAND_FAILED;
		else
			*cmd_field |= OMADM_ACL_EXEC_SET;
	}
	else if (!strcmp(command, "Replace")) {
		if (*cmd_field & OMADM_ACL_REPLACE_SET)
			DMC_ERR = OMADM_SYNCML_ERROR_COMMAND_FAILED;
		else
			*cmd_field |= OMADM_ACL_REPLACE_SET;
	}
	else
		DMC_ERR = OMADM_SYNCML_ERROR_COMMAND_FAILED;

	return DMC_ERR;
}

static int prv_check_acl_server(const char *server)
{
	DMC_ERR_MANAGE;

	if (*server == 0)
		DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_COMMAND_FAILED);

	if (strcmp(server, "*"))
		while (*server) {
			if (!isascii((int)*server) ||
			    !isprint((int)*server) ||
			    isspace((int)*server) ||
			    *server == '=' ||
			    *server == '&' || *server == '*' || *server == '+')
				DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_COMMAND_FAILED);
			++server;
		}

DMC_ON_ERR:

	return DMC_ERR;
}

static int prv_check_acl_syntax(const char *acl)
{
	DMC_ERR_MANAGE;

	char *acl_copy;
	char *cur_entry;
	char *entry_sep = NULL;
	char *cmd_sep = NULL;
	char *svr_sep = NULL;
	char *cur_server;
	uint8_t cmd_field = 0;

	DMC_FAIL_NULL(acl_copy, strdup(acl),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);
	cur_entry = acl_copy;

	/* Check entry list */

	while (cur_entry && *cur_entry) {
		entry_sep = strchr(cur_entry, '&');
		if (entry_sep)
			*entry_sep = 0;

		/* Get command name */

		cmd_sep = strchr(cur_entry, '=');
		if (!cmd_sep)
			DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_COMMAND_FAILED);
		*cmd_sep = 0;
		DMC_FAIL(prv_check_acl_command(cur_entry, &cmd_field));

		/* Check server list */

		cur_server = cmd_sep + 1;
		if (*cur_server == 0)
			DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_COMMAND_FAILED);
		do {
			/* Get server id */

			svr_sep = strchr(cur_server, '+');
			if (svr_sep)
				*svr_sep = 0;

			DMC_FAIL(prv_check_acl_server(cur_server));

			/* Go next server */

			cur_server = svr_sep ? svr_sep + 1 : NULL;

		} while (cur_server);

		/* Go next entry */

		cur_entry = entry_sep ? entry_sep + 1 : NULL;
	}

DMC_ON_ERR:

	free(acl_copy);

	return DMC_ERR;
}

static int prv_replace_acl_property(dmtree_session *session,
				    const dmtree_node *node, const char *uri,
				    OMADM_NodeType node_exists)
{
	DMC_ERR_MANAGE;
	char *slash;
	char *parent_uri = NULL;
	const char *acl_uri;

	if (node_exists == OMADM_NODE_IS_LEAF) {
		DMC_FAIL_NULL(parent_uri, strdup(uri),
				   OMADM_SYNCML_ERROR_DEVICE_FULL);
		slash = strrchr(parent_uri,'/');
		if (!slash) {
			/*
			 * This should never happen as leaf nodes by definition
			 * must have a parent.
			 */

			DMC_FAIL(OMADM_SYNCML_ERROR_SESSION_INTERNAL);
		}
		*slash = 0;
		acl_uri = parent_uri;
	} else
		acl_uri = uri;

	DMC_FAIL(prv_check_node_access_rights(session, uri, acl_uri,
						   OMADM_COMMAND_REPLACE,
						   false));

	DMC_FAIL(prv_check_acl_syntax((char*) node->data_buffer));

	/* Write the value */

	DMC_FAIL(omadm_transaction_set_meta(&session->transaction,
						 uri,
						 OMADM_NODE_PROPERTY_ACL,
						 (char*) node->data_buffer));

DMC_ON_ERR:

	free(parent_uri);

	return DMC_ERR;
}

static int prv_replace_node_property(dmtree_session *session,
				     const dmtree_node *node, const char *uri,
				     const char *nv)
{
	DMC_ERR_MANAGE;
	OMADM_NodeType node_exists;

	DMC_LOGF("Attempting to replace %s property of node %s",
		      nv, node->target_uri);

	DMC_FAIL(omadm_transaction_exists(&session->transaction,
					       uri, &node_exists));

	if (node_exists == OMADM_NODE_NOT_EXIST)
		DMC_FAIL(OMADM_SYNCML_ERROR_NOT_FOUND);

	if (!strcmp(nv,OMADM_NODE_PROPERTY_TITLE))
		DMC_FAIL_FORCE(
			OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED);
	else if (!strcmp(nv,OMADM_NODE_PROPERTY_NAME))
		/*
		 * TODO: This is temporary.
		 * We need to support renaming of the nodes.
		 */
		DMC_FAIL_FORCE(
			OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED);
	else if (!strcmp(nv,OMADM_NODE_PROPERTY_ACL))
		DMC_FAIL(prv_replace_acl_property(session, node, uri,
						       node_exists));
	else
		DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_NOT_ALLOWED);

DMC_ON_ERR:

	return DMC_ERR;
}

static int prv_replace_node(dmtree_session *session, const dmtree_node *node)
{
	DMC_ERR_MANAGE;
	OMADM_NodeType node_exists;

	DMC_LOGF("Attempting to replace value of node %s",
		      node->target_uri);

	if ((node->format) && (node->type) &&
	    (!strcmp(node->format, OMADM_XML_FORMAT_VAL))) {
		if (!strcmp(node->type, OMADM_DMTNDS_XML_TYPE_VAL) ||
		    !strcmp(node->type, OMADM_DMTNDS_WBXML_TYPE_VAL))
			DMC_FAIL_FORCE(
				OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED);
		else
			DMC_FAIL(OMADM_SYNCML_ERROR_NOT_ALLOWED);
	}

	DMC_FAIL(omadm_transaction_exists(&session->transaction,
					       node->target_uri,
					       &node_exists));

	if (node_exists == OMADM_NODE_NOT_EXIST)
		DMC_FAIL(OMADM_SYNCML_ERROR_NOT_FOUND);
	else if (node_exists != OMADM_NODE_IS_LEAF)
		DMC_FAIL(OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(prv_check_node_access_rights(session, node->target_uri,
						   node->target_uri,
						   OMADM_COMMAND_REPLACE, false));

	DMC_LOGF("replacing %s value to", node->target_uri,
		node->data_buffer);

	if (omadm_transaction_in_progress(&session->transaction))
		DMC_FAIL(prv_update_leaf_node(session, node));
	else {
		DMC_FAIL(omadm_transaction_begin(
				      &session->transaction));

		DMC_ERR = prv_update_leaf_node(session, node);

		if (DMC_ERR == OMADM_SYNCML_ERROR_NONE)
			DMC_FAIL(omadm_transaction_commit(
					      &session->transaction));
		else
		{
			(void) omadm_transaction_cancel(
				&session->transaction);
			DMC_FAIL_FORCE(DMC_ERR);
		}
	}

DMC_ON_ERR:

	return DMC_ERR;
}

int dmtree_session_replace(dmtree_session *session, const dmtree_node *node)
{
	DMC_ERR_MANAGE;
	char *uri_copy = NULL;
	char *prop;
	const char *nv;
	const char prop_id[] = "?prop=";

	DMC_LOGF("%s called.", __FUNCTION__);

	if (!node || !node->target_uri)
		DMC_FAIL(OMADM_SYNCML_ERROR_COMMAND_FAILED);

	/* TODO: Copy node and strip everything */

	DMC_LOGF("replacing %s", node->target_uri);

	DMC_FAIL(dmtree_validate_uri(node->target_uri, true));

	prop = strstr(node->target_uri, prop_id);
	if (prop)
	{
		DMC_FAIL_NULL(uri_copy, strdup(node->target_uri),
				   OMADM_SYNCML_ERROR_DEVICE_FULL);
		uri_copy[prop - node->target_uri] = 0;
		nv = prop + (sizeof(prop_id) - 1);
		DMC_FAIL(prv_replace_node_property(session, node, uri_copy,
							nv));
	}
	else
		DMC_FAIL(prv_replace_node(session, node));

DMC_ON_ERR:

	free(uri_copy);

	DMC_LOGF("%s finished with error %d",__FUNCTION__, DMC_ERR);

	return DMC_ERR;
}

int dmtree_session_device_info(dmtree_session *session,
			      dmc_ptr_array *device_info)
{
	DMC_ERR_MANAGE;

	DMC_LOG("Retrieving device info.");

	DMC_FAIL(prv_read_and_add_node(session, "./DevInfo/Mod",
					    device_info));
	DMC_FAIL(prv_read_and_add_node(session, "./DevInfo/Man",
					    device_info));
	DMC_FAIL(prv_read_and_add_node(session, "./DevInfo/DevId",
					    device_info));
	DMC_FAIL(prv_read_and_add_node(session, "./DevInfo/Lang",
					    device_info));
	DMC_FAIL(prv_read_and_add_node(session, "./DevInfo/DmV",
					    device_info));

DMC_ON_ERR:

	DMC_LOGF("Device Info retrieved with error %d", DMC_ERR);

	return DMC_ERR;
}

/* TODO.  Might be better to move dmtree initialisation code into dmtree.c */

static int prv_init_dmtree(dmtree_session* session)
{
	DMC_ERR_MANAGE;

	OMADM_DMTreePlugin *plugin = NULL;
    DIR *folderP;

	DMC_FAIL(omadm_dmtree_create(session->server_id, &session->dmtree));

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
                omadm_dmtree_load_plugin(session->dmtree, filename);
                free(filename);
            }
        }
        closedir(folderP);
    }

	DMC_FAIL(omadm_dmtree_init(session->dmtree));

DMC_ON_ERR:

	DMC_LOGF("Initialised DM Tree with: Error... %d", DMC_ERR);

	if (plugin)
		free(plugin);

	return DMC_ERR;
}

int dmtree_session_copy(dmtree_session *session, const char *source_uri,
		       const char *target_uri)
{
	DMC_ERR_MANAGE;
	OMADM_NodeType node_exists;
	const char *cmd_name;
	dmtree_node *leaf_node = NULL;

	DMC_LOGF("%s called.", __FUNCTION__);

	if (!source_uri || !target_uri)
		DMC_FAIL(OMADM_SYNCML_ERROR_COMMAND_FAILED);

	/* TODO: Copy strip everything uri */

	DMC_LOGF("copying %s to %s", source_uri, target_uri);

	DMC_FAIL(dmtree_validate_uri(source_uri, false));

	DMC_FAIL(dmtree_validate_uri(target_uri, false));

	DMC_FAIL(omadm_transaction_exists(&session->transaction,
					       source_uri, &node_exists));

	if (node_exists == OMADM_NODE_NOT_EXIST)
		DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_NOT_FOUND);
	else if (node_exists != OMADM_NODE_IS_LEAF)
		DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(omadm_transaction_exists(&session->transaction,
					       target_uri, &node_exists));

	if (node_exists == OMADM_NODE_IS_INTERIOR)
		DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_NOT_ALLOWED);

	DMC_FAIL(prv_check_node_access_rights(session, source_uri,
						   source_uri,
						   OMADM_COMMAND_GET,
						   false));

	cmd_name =  (node_exists == OMADM_NODE_NOT_EXIST) ? OMADM_COMMAND_ADD :
		OMADM_COMMAND_REPLACE;

	DMC_FAIL(prv_check_node_access_rights(session, target_uri,
						   target_uri, cmd_name,
						   false));

	DMC_FAIL(prv_read_leaf_node(session, source_uri, &leaf_node));
	free(leaf_node->target_uri);
	leaf_node->target_uri = NULL;
	DMC_FAIL_NULL(leaf_node->target_uri, strdup(target_uri),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);
	DMC_FAIL(prv_update_leaf_node(session, leaf_node));

DMC_ON_ERR:

	dmtree_node_free(leaf_node);

	DMC_LOGF("%s finished with error %d",__FUNCTION__, DMC_ERR);

	return DMC_ERR;
}

int dmtree_session_create(const char *server_id, dmtree_session **session)
{
	DMC_ERR_MANAGE;

	dmtree_session *retval = NULL;

	DMC_LOGF("Creating dm session with server %s", server_id);

	DMC_FAIL_ERR(prv_check_acl_server(server_id),
			    OMADM_SYNCML_ERROR_SESSION_INTERNAL);

	DMC_FAIL_NULL(retval, malloc(sizeof(*retval)),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);

	memset(retval, 0, sizeof(*retval));

	DMC_FAIL_NULL(retval->server_id, strdup(server_id),
			   OMADM_SYNCML_ERROR_DEVICE_FULL);

	DMC_FAIL(prv_init_dmtree(retval));

	omadm_transaction_make(&retval->transaction, retval->dmtree);

	*session = retval;

	DMC_LOG("dm session created correctly");

	return OMADM_SYNCML_ERROR_NONE;

DMC_ON_ERR:

	DMC_LOGF("Failed to create dm session with error %d", DMC_ERR);

	dmtree_session_free(retval);

	return DMC_ERR;
}

void dmtree_session_free(dmtree_session *session)
{
	DMC_LOG("Freeing dm session");

	if (session)
	{
		omadm_dmtree_free(session->dmtree);
		free(session->server_id);
		free(session);
	}
}

void dmtree_node_free(dmtree_node *node)
{
	if (node == NULL)
		return;

	if (node->target_uri)
		free(node->target_uri);

	if (node->format)
		free(node->format);

	if (node->type)
		free(node->type);

	if (node->data_buffer)
		free(node->data_buffer);

	free(node);
}
