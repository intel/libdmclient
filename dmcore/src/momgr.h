/*
 * libdmclient
 *
 * Copyright (C) 2012 Intel Corporation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
    char * last_uri;
    dmtree_plugin_t * last_plugin;
} mo_mgr_t;


mo_mgr_t * momgr_init();
void momgr_free(mo_mgr_t * iMgr);
int momgr_check_mandatory_mo(mo_mgr_t * iMgrP);

int momgr_add_plugin(mo_mgr_t * iMgr, omadm_mo_interface_t *iPlugin);

int momgr_exists(mo_mgr_t * iMgr, const char *iUri, omadmtree_node_kind_t *oExists);

int momgr_get_value(mo_mgr_t * iMgr, dmtree_node_t * nodeP);
int momgr_set_value(mo_mgr_t * iMgr, const dmtree_node_t * nodeP);

int momgr_get_ACL(mo_mgr_t * iMgr, const char *iUri, char **oACL);
int momgr_set_ACL(mo_mgr_t * iMgr, const char *iUri, const char *iACL);

int momgr_rename_node(mo_mgr_t * iMgr, const char *iFrom, const char *iTo);
int momgr_delete_node(mo_mgr_t * iMgr, const char *iUri);
int momgr_exec_node(mo_mgr_t * iMgr, const char *iUri, const char *iData, const char *iCorrelator);

int momgr_validate_uri(mo_mgr_t * iMgr, const char *uri, char ** oNodeURI, char ** oPropId);

int momgr_find_subtree(mo_mgr_t * iMgr, const char * iUri, const char * iUrn, const char * iCriteriaName, const char * iCriteriaValue, char ** oUri);
int momgr_list_uri(mo_mgr_t * iMgr, const char * iUrn, char *** oUri);

#endif
