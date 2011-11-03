/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file dmsettings_utils.h
 *
 * @brief Header file for the dmsettings utility functions
 *
 */

#ifndef OMADM_DMSETTINGS_UTILS_H_
#define OMADM_DMSETTINGS_UTILS_H_

#include "dyn_buf.h"

#include "dmtree_plugin.h"

int omadm_dmsettings_utils_node_exists(dmsettings *handle, const char *uri,
				       OMADM_NodeType *node_type);
int omadm_dmsettings_utils_get_node_children(dmsettings *handle,
					     const char *uri, 
					     dmc_ptr_array *children);
int omadm_dmsettings_utils_get_value(dmsettings *handle, const char *uri,
				     char **value);
int omadm_dmsettings_utils_set_value(dmsettings *handle, const char *uri,
				     const char *value);
int omadm_dmsettings_utils_get_meta(dmsettings *handle, const char *uri,
				    const char *prop, char **value);
int omadm_dmsettings_utils_set_meta(dmsettings *handle, const char *uri,
				    const char *prop, const char *value);
int omadm_dmsettings_utils_delete_node(dmsettings *handle, const char *uri);
int omadm_dmsettings_utils_create_non_leaf(dmsettings *handle, const char *uri);

#endif
