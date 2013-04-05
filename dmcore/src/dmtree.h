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
    mo_mgr_t * MOs;
    char *server_id;
} dmtree_t;

int dmtree_open(dmtree_t ** handleP);
void dmtree_close(dmtree_t * handle);

int dmtree_setServer(dmtree_t * handle, const char *server_id);

int dmtree_get(dmtree_t * handle, dmtree_node_t *node);
int dmtree_add(dmtree_t * handle, dmtree_node_t *node);
int dmtree_replace(dmtree_t * handle, dmtree_node_t *node);
int dmtree_delete(dmtree_t * handle, const char *uri);
int dmtree_exec(dmtree_t * handle, const char *uri, const char *data, const char *correlator);
int dmtree_copy(dmtree_t * handle, const char *source_uri, const char *target_uri);


#endif
