/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file dmsettings_utils.c 
 *
 * @brief C Contains some common routines used by both the config
 *        and the root plugin.
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dmsettings.h"
#include "error_macros.h"
#include "error.h"

#include "dmsettings_utils.h"
#include "syncml_error.h"

int omadm_dmsettings_utils_node_exists(dmsettings *handle, const char *uri,
				       OMADM_NodeType *node_type)
{
	DMC_ERR_MANAGE;
	dmsettings_settings_type settings_type;

	DMC_ERR = dmsettings_exists(handle, uri + 1, &settings_type);

	DMC_FAIL(syncml_from_dmc_err(DMC_ERR));

	switch (settings_type) {
	case DMSETTINGS_TYPE_NOT_EXISTS:
		*node_type = OMADM_NODE_NOT_EXIST;
		break;
	case DMSETTINGS_TYPE_DIR:
		*node_type = OMADM_NODE_IS_INTERIOR;
		break;
	case DMSETTINGS_TYPE_VALUE:
		*node_type = OMADM_NODE_IS_LEAF;
		break;
	default:
		DMC_FAIL_FORCE(DMC_ERR_CORRUPT);
		break;
	}

DMC_ON_ERR:

	return OMADM_SYNCML_ERROR_NONE;
}

int omadm_dmsettings_utils_get_node_children(dmsettings *handle,
					     const char *uri, 
					     dmc_ptr_array *children)
{
	DMC_ERR_MANAGE;

	dmc_ptr_array utils_children;
	unsigned int url_length = strlen(uri);
	unsigned int i = 0;
	char *child_key = NULL;
	char *child_uri = NULL;

	dmc_ptr_array_make(&utils_children, 16, free);

	DMC_FAIL(dmsettings_get_children(handle, uri + 1,
						&utils_children));

	for (i = 0; i < dmc_ptr_array_get_size(&utils_children); ++i) {
		child_key = (char *) dmc_ptr_array_get(&utils_children, i);
		DMC_FAIL_NULL(child_uri, malloc(url_length + strlen(child_key) 
						+ 2), DMC_ERR_OOM);
		sprintf(child_uri,"%s/%s",uri,child_key);
		DMC_FAIL(dmc_ptr_array_append(children, child_uri));
		child_uri = NULL;
	}

DMC_ON_ERR:

	if (child_uri)
		free(child_uri);

	dmc_ptr_array_free(&utils_children);

	return syncml_from_dmc_err(DMC_ERR);
}

int omadm_dmsettings_utils_get_value(dmsettings *handle, const char *uri,
				     char **value)
{
	return syncml_from_dmc_err(
		dmsettings_get_value(handle, uri + 1, value));
}

int omadm_dmsettings_utils_set_value(dmsettings *handle, const char *uri,
				     const char *value)
{
	return syncml_from_dmc_err(
		dmsettings_set_value(handle, uri + 1, value));
}

int omadm_dmsettings_utils_get_meta(dmsettings *handle, const char *uri,
				    const char *prop, char **value)
{
	DMC_ERR_MANAGE;
	char *value_copy = NULL;

	dmsettings_settings_type settings_type;

	DMC_ERR = dmsettings_get_meta(handle, uri + 1, prop, 
					     &value_copy);

	if (DMC_ERR == DMC_ERR_NOT_FOUND) {
		if (!strcmp(prop, OMADM_NODE_PROPERTY_TYPE)) {
			DMC_FAIL(dmsettings_exists(handle, uri + 1,
							&settings_type));
			if (settings_type == OMADM_NODE_IS_INTERIOR) {
				DMC_FAIL_NULL(value_copy, strdup("null"),
						   DMC_ERR_OOM);
			} else if (settings_type == OMADM_NODE_IS_LEAF) {
				DMC_FAIL_NULL(value_copy,
						   strdup("text/plain"), 
						   DMC_ERR_OOM);
			} else
				DMC_ERR = DMC_ERR_NOT_FOUND;
		} else if (!strcmp(prop, OMADM_NODE_PROPERTY_FORMAT)) {
			DMC_FAIL(dmsettings_exists(handle, uri + 1,
							&settings_type));
			if (settings_type == OMADM_NODE_IS_INTERIOR) {
				DMC_FAIL_NULL(value_copy, strdup("node"),
						   DMC_ERR_OOM);
			} else if (settings_type == OMADM_NODE_IS_LEAF) {
				DMC_FAIL_NULL(value_copy, strdup("chr"),
						   DMC_ERR_OOM);
			} else
				DMC_ERR = DMC_ERR_NOT_FOUND;	       
		}
	}

	DMC_FAIL(DMC_ERR);

	*value = value_copy;

DMC_ON_ERR:

	return syncml_from_dmc_err(DMC_ERR);
}

int omadm_dmsettings_utils_set_meta(dmsettings *handle, const char *uri,
				    const char *prop, const char *value)
{
	return syncml_from_dmc_err(
		dmsettings_set_meta(handle, uri + 1, prop, value));
}

int omadm_dmsettings_utils_delete_node(dmsettings *handle, const char *uri)
{
	return syncml_from_dmc_err(dmsettings_delete(handle, uri + 1));
}

int omadm_dmsettings_utils_create_non_leaf(dmsettings *handle, const char *uri)
{
	return syncml_from_dmc_err(dmsettings_create_dir(handle, uri + 1));
}
