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
 * @file dmtree.h
 *
 * @brief Headers to access the DM tree
 *
 *****************************************************************************/

#ifndef OMADM_DMTREE_H
#define OMADM_DMTREE_H

#include <stdbool.h>
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


#endif
