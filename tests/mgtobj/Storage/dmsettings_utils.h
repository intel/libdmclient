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

#ifndef OMADM_DMSETTINGS_UTILS_H_
#define OMADM_DMSETTINGS_UTILS_H_

#include <omadmtree_mo.h>

#include "dyn_buf.h"

#include "dmsettings.h"

int omadm_dmsettings_utils_node_exists(dmsettings *handle, const char *uri,
                       omadmtree_node_kind_t *node_type);
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
