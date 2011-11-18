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
} mo_list_t;


int momgr_init(mo_list_t * iListP);
void momgr_free(mo_list_t * iList);

void momgr_load_plugin(mo_list_t * iList, const char *iFilename);

int momgr_exists(const mo_list_t iList, const char *iUri, omadmtree_node_type_t *oExists);

int momgr_get_value(const mo_list_t iList, dmtree_node_t * nodeP);
int momgr_set_value(const mo_list_t iList, const dmtree_node_t * nodeP);

int momgr_get_ACL(const mo_list_t iList, const char *iUri, char **oACL);
int momgr_set_ACL(const mo_list_t iList, const char *iUri, const char *iACL);

int momgr_rename_node(const mo_list_t iList, const char *iFrom, const char *iTo);
int momgr_delete_node(const mo_list_t iList, const char *iUri);
int momgr_exec_node(const mo_list_t iList, const char *iUri, const char *iData, const char *iCorrelator);


#endif
