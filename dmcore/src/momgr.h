/*
 * libdmclient
 *
 * Copyright (C) 2012 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * David Navarro <david.navarro@intel.com>
 *
 */

/*!
 * @file momgr.h
 *
 * @brief Headers to manage the Management Objects
 *
 */

#ifndef OMADM_MOMGR_PRV_H_
#define OMADM_MOMGR_PRV_H_

#include <stdint.h>

#include "omadmtree_mo.h"

typedef struct
{
    omadm_mo_interface_t * interface;
    void *                 data;
    void *                 dl_handle;
    struct _mo_dir *       container;
} dmtree_plugin_t;

// plugins are stored in a tree structure.
// Be careful to not mix this with the DM tree.
// Functions manipulating this tree structure are named prv_*MoDir()
typedef struct _mo_dir
{
    char *            name;
    dmtree_plugin_t * plugin;
    struct _mo_dir *  children;
    struct _mo_dir *  parent;
    struct _mo_dir *  next;
} mo_dir_t;

typedef struct
{
    mo_dir_t * root;
    uint16_t max_depth;
    uint16_t max_total_len;
    uint16_t max_segment_len;
} mo_mgr_t;


int momgr_init(mo_mgr_t * iMgrP);
void momgr_free(mo_mgr_t * iMgr);

void momgr_load_plugin(mo_mgr_t * iMgr, const char *iFilename);
int momgr_add_plugin(mo_mgr_t * iMgr, omadm_mo_interface_t *iPlugin, void * handle);

int momgr_exists(const mo_mgr_t iMgr, const char *iUri, omadmtree_node_type_t *oExists);

int momgr_get_value(const mo_mgr_t iMgr, dmtree_node_t * nodeP);
int momgr_set_value(const mo_mgr_t iMgr, const dmtree_node_t * nodeP);

int momgr_get_ACL(const mo_mgr_t iMgr, const char *iUri, char **oACL);
int momgr_set_ACL(const mo_mgr_t iMgr, const char *iUri, const char *iACL);

int momgr_rename_node(const mo_mgr_t iMgr, const char *iFrom, const char *iTo);
int momgr_delete_node(const mo_mgr_t iMgr, const char *iUri);
int momgr_exec_node(const mo_mgr_t iMgr, const char *iUri, const char *iData, const char *iCorrelator);

int momgr_validate_uri(const mo_mgr_t iMgr, const char *uri, char ** oNodeURI, char ** oPropId);

int momgr_find_subtree(const mo_mgr_t iMgr, const char * iUri, const char * iUrn, const char * iCriteriaName, const char * iCriteriaValue, char ** oUri);


#endif
