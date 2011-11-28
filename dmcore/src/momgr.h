/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (c) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

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
    char * URI;
    omadm_mo_interface_t * interface;
    void * data;
    void * dl_handle;
} dmtree_plugin_t;

typedef struct _plugin_elem
{
    dmtree_plugin_t * plugin;
    struct _plugin_elem * next;
} plugin_elem_t;

typedef struct
{
    plugin_elem_t * first;
    uint16_t max_depth;
    uint16_t max_total_len;
    uint16_t max_segment_len;
} mo_mgr_t;


int momgr_init(mo_mgr_t * iMgrP);
void momgr_free(mo_mgr_t * iMgr);

void momgr_load_plugin(mo_mgr_t * iMgr, const char *iFilename);

int momgr_exists(const mo_mgr_t iMgr, const char *iUri, omadmtree_node_type_t *oExists);

int momgr_get_value(const mo_mgr_t iMgr, dmtree_node_t * nodeP);
int momgr_set_value(const mo_mgr_t iMgr, const dmtree_node_t * nodeP);

int momgr_get_ACL(const mo_mgr_t iMgr, const char *iUri, char **oACL);
int momgr_set_ACL(const mo_mgr_t iMgr, const char *iUri, const char *iACL);

int momgr_rename_node(const mo_mgr_t iMgr, const char *iFrom, const char *iTo);
int momgr_delete_node(const mo_mgr_t iMgr, const char *iUri);
int momgr_exec_node(const mo_mgr_t iMgr, const char *iUri, const char *iData, const char *iCorrelator);

int momgr_validate_uri(const mo_mgr_t iMgr, const char *uri, char ** oNodeURI, char ** oPropId);
int momgr_get_uri_from_urn(const mo_mgr_t iMgr, const char * iUrn, char ** oUri);


#endif
