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

#ifndef DMSETTINGS_H
#define DMSETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dyn_buf.h"

typedef struct dmsettings_ dmsettings;

enum dmsettings_settings_type_ {
    DMSETTINGS_TYPE_NOT_EXISTS,
    DMSETTINGS_TYPE_DIR,
    DMSETTINGS_TYPE_VALUE
};

typedef enum dmsettings_settings_type_ dmsettings_settings_type;

int dmsettings_open(dmsettings **handle);
int dmsettings_open_database(const char *datastore, dmsettings **handle);
void dmsettings_close(dmsettings *handle);
int dmsettings_begin_transaction(dmsettings *handle);
int dmsettings_commit_transaction(dmsettings *handle);
int dmsettings_cancel_transaction(dmsettings *handle);
int dmsettings_exists(dmsettings *handle, const char *key,
            dmsettings_settings_type *settings_type);
int dmsettings_get_children(dmsettings *handle, const char *key,
                dmc_ptr_array *children);
int dmsettings_get_value(dmsettings *handle, const char *key, char **value);
int dmsettings_set_value(dmsettings *handle, const char *key, const char
                *value);
int dmsettings_get_meta(dmsettings *handle, const char *key, const char *prop,
                char **value);
int dmsettings_set_meta(dmsettings *handle, const char *key, const char *prop,
                const char *value);
int dmsettings_delete(dmsettings *handle, const char *key);
int dmsettings_create_dir(dmsettings *handle, const char *key);

#ifdef __cplusplus
}
#endif

#endif
