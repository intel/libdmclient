/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/
/*!
 * @file dmtree.h
 *
 * @brief Headers to access the DM tree
 *
 *****************************************************************************/

#ifndef OMADM_DMTREE_H
#define OMADM_DMTREE_H

#include "momgr.h"

typedef struct
{
    mo_mgr_t MOs;
    char *server_id;
} dmtree_t;

int dmtree_open(const char *server_id, dmtree_t ** handleP);
void dmtree_close(dmtree_t * handle);

int dmtree_get(dmtree_t * handle, dmtree_node_t *node);
int dmtree_add(dmtree_t * handle, dmtree_node_t *node);
int dmtree_replace(dmtree_t * handle, dmtree_node_t *node);
int dmtree_delete(dmtree_t * handle, const char *uri);
int dmtree_copy(dmtree_t * handle, const char *source_uri, const char *target_uri);

void dmtree_node_free(dmtree_node_t *node);

#endif
