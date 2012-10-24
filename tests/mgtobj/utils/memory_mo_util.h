/*
 * libdmclient test materials
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

#ifndef MEMORY_MO_UTIL_H_
#define MEMORY_MO_UTIL_H_

#include <omadmtree_mo.h>

typedef struct _memory_node_t
{
    omadmtree_node_kind_t type;
    char * name;
    char * uri;
    char * acl;
    dmtree_node_t * content;
    struct _memory_node_t * children;
    struct _memory_node_t * next;
    struct _memory_node_t * parent;    
} memory_node_t;

int memory_mo_init(char * iURI, char * iACL, void **oData);
void memory_mo_close(void * iData);
int memory_mo_is_node(const char *iURI, omadmtree_node_kind_t *oNodeType, void *iData);
int memory_mo_get(dmtree_node_t * nodeP, void *iData);
int memory_mo_getACL(const char *iURI, char **oValue, void *iData);
int memory_mo_set(const dmtree_node_t * nodeP, void * iData);
int memory_mo_setACL(const char * iURI, const char *iACL, void * iData);
int memory_mo_delete(const char * iURI, void * iData);
int memory_mo_findURN(const char *iURN, char ***oURL, void *iData);

#endif
