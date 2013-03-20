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

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sqlite3.h>

#include "dyn_buf.h"
#include "dmsettings.h"
#include "error.h"
#include "error_macros.h"

#define DMSETTINGS_BUSY_TIMEOUT 30 * 1000

#define DM_SETTINGS_DATABASE "/tmp/dmsettings.db"

#define DM_SETTINGS_CREATE_DATA "CREATE TABLE IF NOT EXISTS explicit \
(key TEXT PRIMARY KEY, value TEXT);"

#define DM_SETTINGS_CREATE_META "CREATE TABLE IF NOT EXISTS meta \
(key TEXT, type TEXT, value TEXT, PRIMARY KEY (key,type));"

#define DM_SETTINGS_BEGIN_TRANSACTION "BEGIN TRANSACTION"

#define DM_SETTINGS_COMMIT_TRANSACTION "COMMIT TRANSACTION"

#define DM_SETTINGS_CANCEL_TRANSACTION "ROLLBACK TRANSACTION"

#define DM_SETTINGS_EXISTS_SETTING "SELECT COUNT(*) FROM explicit \
WHERE key = ? and value NOT NULL;"

#define DM_SETTINGS_EXISTS "SELECT COUNT(*) FROM explicit \
WHERE key = ? or key GLOB ? || '/*';"

/*
 * The database is not really optimised to make this query.
 * Currently it returns one row for direct descendent that is either a
 * leaf or childess non leaf node and one row for each
 * grandchild that is either a leaf or childess non leaf node.
 * The rows containing the grandchildren contain duplicates that
 * need to be eliminated. The only way that I can think of improving
 * this query would be to store more information in the database,
 * which would slow down set and take up more space.
 */

#define DM_SETTINGS_FIND_CHILDREN "SELECT substr(key,length(?)+2) FROM \
 explicit WHERE key GLOB ? || '/*' AND NOT substr(key,length(?)+2) \
 GLOB '*/*/*' ORDER BY key;"

#define DM_SETTINGS_GET_VALUE "SELECT value FROM explicit \
WHERE key = ? and value NOT NULL;"

#define DM_SETTINGS_GET_META "SELECT value FROM meta \
WHERE key = ? and type = ?;"

#define DM_SETTINGS_SET_VALUE "REPLACE INTO explicit \
(key, value) VALUES(?, ?);"

#define DM_SETTINGS_SET_META "REPLACE INTO meta \
(key, type, value) VALUES(?, ?, ?);"

#define DM_SETTINGS_DELETE_EXPLICIT "DELETE FROM explicit WHERE key \
= ? OR key GLOB ? || '/*';"

#define DM_SETTINGS_DELETE_META "DELETE FROM meta WHERE key \
= ? OR key GLOB ? || '/*';"

#define DM_SETTINGS_SAVEPOINT_SET "SAVEPOINT nested_transaction"
#define DM_SETTINGS_SAVEPOINT_CANCEL "ROLLBACK TO nested_transaction"
#define DM_SETTINGS_SAVEPOINT_COMMIT "RELEASE nested_transaction"

struct dmsettings_
{
    sqlite3 *db_handle;
    bool transaction_in_progress;
    bool can_write;
    sqlite3_stmt *stmt_exists_setting;
    sqlite3_stmt *stmt_exists;
    sqlite3_stmt *stmt_find_children;
    sqlite3_stmt *stmt_get_value;
    sqlite3_stmt *stmt_get_meta;
    sqlite3_stmt *stmt_set_value;
    sqlite3_stmt *stmt_set_meta;
    sqlite3_stmt *stmt_delete_explicit;
    sqlite3_stmt *stmt_delete_meta;
    sqlite3_stmt *stmt_begin_transaction;
    sqlite3_stmt *stmt_commit_transaction;
    sqlite3_stmt *stmt_cancel_transaction;
    sqlite3_stmt *stmt_savepoint_set;
    sqlite3_stmt *stmt_savepoint_cancel;
    sqlite3_stmt *stmt_savepoint_commit;
};

static int map_sqlite3_error(int sqlite_error)
{
    int ret_val;

    switch (sqlite_error) {
    case SQLITE_OK:
        ret_val = DMC_ERR_NONE;
        break;
    case SQLITE_PERM:
    case SQLITE_AUTH:
        ret_val = DMC_ERR_DENIED;
        break;
    case SQLITE_NOMEM:
        ret_val = DMC_ERR_OOM;
        break;
    case SQLITE_CANTOPEN:
        ret_val = DMC_ERR_OPEN;
        break;
    case SQLITE_BUSY:
        ret_val = DMC_ERR_TIMEOUT;
        break;
    default:
        ret_val = DMC_ERR_IO;
        break;
    }

    return ret_val;
}

static int prv_create_ro_queries(dmsettings *handle)
{
    DMC_ERR_MANAGE;
    int sqlerr;

    sqlerr = sqlite3_prepare_v2(handle->db_handle,
                    DM_SETTINGS_EXISTS_SETTING, -1,
                    &handle->stmt_exists_setting, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_prepare_v2(handle->db_handle, DM_SETTINGS_EXISTS, -1,
                    &handle->stmt_exists, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_prepare_v2(handle->db_handle,
                    DM_SETTINGS_FIND_CHILDREN, -1,
                    &handle->stmt_find_children, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_prepare_v2(handle->db_handle, DM_SETTINGS_GET_VALUE,
                    -1, &handle->stmt_get_value, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_prepare_v2(handle->db_handle, DM_SETTINGS_GET_META,
                    -1, &handle->stmt_get_meta, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_create_rw_queries(dmsettings *handle)
{
    DMC_ERR_MANAGE;
    int sqlerr;

    sqlerr = sqlite3_prepare_v2(handle->db_handle, DM_SETTINGS_SET_VALUE,
                    -1, &handle->stmt_set_value, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_prepare_v2(handle->db_handle, DM_SETTINGS_SET_META,
                    -1, &handle->stmt_set_meta, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_prepare_v2(handle->db_handle,
                    DM_SETTINGS_BEGIN_TRANSACTION, -1,
                    &handle->stmt_begin_transaction, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_prepare_v2(handle->db_handle,
                    DM_SETTINGS_COMMIT_TRANSACTION, -1,
                    &handle->stmt_commit_transaction, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_prepare_v2(handle->db_handle,
                    DM_SETTINGS_CANCEL_TRANSACTION, -1,
                    &handle->stmt_cancel_transaction, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_prepare_v2(handle->db_handle,
                    DM_SETTINGS_DELETE_EXPLICIT, -1,
                    &handle->stmt_delete_explicit, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_prepare_v2(handle->db_handle,
                    DM_SETTINGS_DELETE_META, -1,
                    &handle->stmt_delete_meta, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_prepare_v2(handle->db_handle,
                    DM_SETTINGS_SAVEPOINT_SET, -1,
                    &handle->stmt_savepoint_set, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_prepare_v2(handle->db_handle,
                    DM_SETTINGS_SAVEPOINT_CANCEL, -1,
                    &handle->stmt_savepoint_cancel, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_prepare_v2(handle->db_handle,
                    DM_SETTINGS_SAVEPOINT_COMMIT, -1,
                    &handle->stmt_savepoint_commit, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_create_database(dmsettings *handle)
{
    DMC_ERR_MANAGE;

    DMC_FAIL(map_sqlite3_error(sqlite3_exec(handle->db_handle,
                             DM_SETTINGS_CREATE_DATA,
                             NULL, NULL, NULL)));

    DMC_FAIL(map_sqlite3_error(sqlite3_exec(handle->db_handle,
                             DM_SETTINGS_CREATE_META,
                             NULL, NULL, NULL)));

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_exec_simple_cmd(sqlite3_stmt *statement)
{
    int sqlerr;

    DMC_ERR_MANAGE;

    sqlerr = sqlite3_step(statement);

    if (sqlerr == SQLITE_OK)
        DMC_ERR = DMC_ERR_IO;
    else if (sqlerr != SQLITE_DONE)
        DMC_ERR = map_sqlite3_error(sqlerr);

    sqlite3_reset(statement);

    return DMC_ERR;
}

static int prv_bind_single_query(sqlite3_stmt *statement, const char *key)
{
    int sqlerr;

    DMC_ERR_MANAGE;

    sqlerr = sqlite3_bind_text(statement, 1, key, -1, SQLITE_STATIC);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_step(statement);

    if (sqlerr == SQLITE_OK)
        DMC_ERR = DMC_ERR_IO;
    else if (sqlerr == SQLITE_DONE)
        DMC_ERR = DMC_ERR_NOT_FOUND;
    else if (sqlerr != SQLITE_ROW)
        DMC_ERR = map_sqlite3_error(sqlerr);

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_bind_double_query(sqlite3_stmt *statement, const char *key,
                    const char *param)
{
    int sqlerr;

    DMC_ERR_MANAGE;

    sqlerr = sqlite3_bind_text(statement, 1, key, -1, SQLITE_STATIC);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_bind_text(statement, 2, param, -1, SQLITE_STATIC);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_step(statement);

    if (sqlerr == SQLITE_OK)
        DMC_ERR = DMC_ERR_IO;
    else if (sqlerr == SQLITE_DONE)
        DMC_ERR = DMC_ERR_NOT_FOUND;
    else if (sqlerr != SQLITE_ROW)
        DMC_ERR = map_sqlite3_error(sqlerr);

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_bind_double_update(sqlite3_stmt *statement, const char *key,
                    const char *value)
{
    DMC_ERR_MANAGE;
    int sqlerr;

    sqlerr = sqlite3_bind_text(statement, 1, key, -1, SQLITE_STATIC);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_bind_text(statement, 2, value, -1, SQLITE_STATIC);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_step(statement);

    if (sqlerr == SQLITE_OK)
        DMC_ERR = DMC_ERR_IO;
    else if (sqlerr != SQLITE_DONE)
        DMC_ERR = map_sqlite3_error(sqlerr);

DMC_ON_ERR:

    sqlite3_reset(statement);
    sqlite3_clear_bindings(statement);

    return DMC_ERR;
}

static int prv_bind_triple_update(sqlite3_stmt *statement, const char *key,
                    const char *prop, const char *value)
{
    int sqlerr;

    DMC_ERR_MANAGE;

    sqlerr = sqlite3_bind_text(statement, 1, key, -1, SQLITE_STATIC);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_bind_text(statement, 2, prop, -1, SQLITE_STATIC);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_bind_text(statement, 3, value, -1, SQLITE_STATIC);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_step(statement);

    if (sqlerr == SQLITE_OK)
        DMC_ERR = DMC_ERR_IO;
    else if (sqlerr != SQLITE_DONE)
        DMC_ERR = map_sqlite3_error(sqlerr);

DMC_ON_ERR:

    sqlite3_reset(statement);
    sqlite3_clear_bindings(statement);

    return DMC_ERR;
}

static int prv_exists_setting(dmsettings *handle, const char *key,
                bool *exists)
{
    DMC_ERR_MANAGE;

    DMC_FAIL(prv_bind_single_query(handle->stmt_exists_setting, key));

    *exists = sqlite3_column_int(handle->stmt_exists_setting , 0) ?
            true : false;

DMC_ON_ERR:

    sqlite3_reset(handle->stmt_exists_setting);
    sqlite3_clear_bindings(handle->stmt_exists_setting);

    return DMC_ERR;
}

static int prv_exists(dmsettings *handle, const char *key, bool *exists)
{
    DMC_ERR_MANAGE;

    DMC_FAIL(prv_bind_double_query(handle->stmt_exists, key, key));

    *exists = sqlite3_column_int(handle->stmt_exists, 0) ? true : false;

DMC_ON_ERR:

    sqlite3_reset(handle->stmt_exists);
    sqlite3_clear_bindings(handle->stmt_exists);

    return DMC_ERR;
}

static int prv_process_child(const char *new_child, dmc_ptr_array *children,
                char **child_copy)
{
    DMC_ERR_MANAGE;
    const char *child;
    uintptr_t name_length;

    if ((child = strchr(new_child, '/')))
        name_length = child - new_child;
    else
        name_length = strlen(new_child);

    if (*child_copy)
    {
        if (strncmp(new_child, *child_copy, name_length))
        {
            DMC_FAIL(dmc_ptr_array_append(children, *child_copy));
            DMC_FAIL_NULL(*child_copy, strndup(new_child,
                                name_length),
                       DMC_ERR_OOM);
        }
    }
    else
        DMC_FAIL_NULL(*child_copy, strndup(new_child, name_length),
                    DMC_ERR_OOM);

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_find_children(dmsettings *handle, const char *key,
                dmc_ptr_array *children)
{
    DMC_ERR_MANAGE;
    const char *full_child;
    char *child_copy = NULL;
    unsigned int i;

    int sqlerr;

    for (i = 1; i <= 3; ++i)
    {
        sqlerr = sqlite3_bind_text(handle->stmt_find_children, i, key,
                        -1, SQLITE_STATIC);

        DMC_FAIL(map_sqlite3_error(sqlerr));
    }

    sqlerr = sqlite3_step(handle->stmt_find_children);

    if (sqlerr == SQLITE_OK)
        DMC_FAIL_FORCE(DMC_ERR_IO);

    while (sqlerr == SQLITE_ROW)
    {
        full_child = (const char *)
                sqlite3_column_text(handle->stmt_find_children,
                            0);

        if (full_child)
            DMC_FAIL(prv_process_child(full_child, children,
                            &child_copy));

        sqlerr = sqlite3_step(handle->stmt_find_children);
    }

    if (sqlerr != SQLITE_DONE)
        DMC_FAIL_FORCE(sqlerr);

    if (child_copy)
    {
        DMC_FAIL(dmc_ptr_array_append(children, child_copy));
        child_copy = NULL;
    }

DMC_ON_ERR:

    if (child_copy)
        free(child_copy);

    sqlite3_reset(handle->stmt_find_children);
    sqlite3_clear_bindings(handle->stmt_find_children);

    return DMC_ERR;
}

static int prv_open_database(const char *datastore, int flags,
                dmsettings **handle)
{
    DMC_ERR_MANAGE;
    dmsettings *settings;
    int sqlerr;

    DMC_FAIL_NULL(settings, malloc(sizeof(*settings)), DMC_ERR_OOM);

    memset(settings, 0, sizeof(*settings));

    sqlerr = sqlite3_open_v2(datastore, &settings->db_handle, flags, NULL);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    sqlerr = sqlite3_busy_timeout(settings->db_handle,
                    DMSETTINGS_BUSY_TIMEOUT);

    DMC_FAIL(map_sqlite3_error(sqlerr));

    *handle = settings;

    return DMC_ERR_NONE;

DMC_ON_ERR:

    free(settings);

    return DMC_ERR;
}

int dmsettings_open_database(const char *datastore, dmsettings **handle)
{
    DMC_ERR_MANAGE;
    dmsettings *settings;

    DMC_FAIL(prv_open_database(datastore, SQLITE_OPEN_READWRITE |
                          SQLITE_OPEN_CREATE, &settings));

    DMC_FAIL_LABEL(prv_create_database(settings), db_opened);
    DMC_FAIL_LABEL(prv_create_ro_queries(settings), db_opened);
    DMC_FAIL_LABEL(prv_create_rw_queries(settings), db_opened);

    settings->can_write = true;
    *handle = settings;

    return DMC_ERR_NONE;

db_opened:

    dmsettings_close(settings);

DMC_ON_ERR:

    return DMC_ERR;
}

int dmsettings_open(dmsettings **handle)
{
    return dmsettings_open_database(DM_SETTINGS_DATABASE, handle);
}

int dmsettings_open_readonly(dmsettings **handle)
{
    DMC_ERR_MANAGE;
    dmsettings *settings;

    DMC_FAIL(prv_open_database(DM_SETTINGS_DATABASE, SQLITE_OPEN_READONLY,
                    &settings));

    DMC_FAIL_LABEL(prv_create_ro_queries(settings), db_opened);

    settings->can_write = false;
    *handle = settings;

    return DMC_ERR_NONE;

db_opened:

    dmsettings_close(settings);

DMC_ON_ERR:

    return DMC_ERR;
}

void dmsettings_close(dmsettings *handle)
{
    if (handle == NULL)
        return;

    if (handle->stmt_exists_setting)
        sqlite3_finalize(handle->stmt_exists_setting);

    if (handle->stmt_exists)
        sqlite3_finalize(handle->stmt_exists);

    if (handle->stmt_find_children)
        sqlite3_finalize(handle->stmt_find_children);

    if (handle->stmt_get_value)
        sqlite3_finalize(handle->stmt_get_value);

    if (handle->stmt_get_meta)
        sqlite3_finalize(handle->stmt_get_meta);

    if (handle->stmt_set_value)
        sqlite3_finalize(handle->stmt_set_value);

    if (handle->stmt_set_value)
        sqlite3_finalize(handle->stmt_set_meta);

    if (handle->stmt_delete_explicit)
        sqlite3_finalize(handle->stmt_delete_explicit);

    if (handle->stmt_delete_meta)
        sqlite3_finalize(handle->stmt_delete_meta);

    if (handle->stmt_begin_transaction)
        sqlite3_finalize(handle->stmt_begin_transaction);

    if (handle->stmt_commit_transaction)
        sqlite3_finalize(handle->stmt_commit_transaction);

    if (handle->stmt_cancel_transaction)
        sqlite3_finalize(handle->stmt_cancel_transaction);

    if (handle->stmt_savepoint_set)
        sqlite3_finalize(handle->stmt_savepoint_set);

    if (handle->stmt_savepoint_cancel)
        sqlite3_finalize(handle->stmt_savepoint_cancel);

    if (handle->stmt_savepoint_commit)
        sqlite3_finalize(handle->stmt_savepoint_commit);

    if (handle->db_handle)
        sqlite3_close(handle->db_handle);

    free(handle);
}

int dmsettings_begin_transaction(dmsettings *handle)
{
    DMC_ERR_MANAGE;

    if (handle->transaction_in_progress)
        DMC_FAIL_FORCE(DMC_ERR_TRANSACTION_IN_PROGRESS);

    if (!handle->can_write)
        DMC_FAIL_FORCE(DMC_ERR_DENIED);

    DMC_FAIL(prv_exec_simple_cmd(handle->stmt_begin_transaction));

    handle->transaction_in_progress = true;

DMC_ON_ERR:

    return DMC_ERR;
}

int dmsettings_commit_transaction(dmsettings *handle)
{
    DMC_ERR_MANAGE;

    if (!handle->transaction_in_progress)
        DMC_FAIL_FORCE(DMC_ERR_NOT_IN_TRANSACTION);

    handle->transaction_in_progress = false;

    DMC_FAIL_LABEL(prv_exec_simple_cmd(handle->stmt_commit_transaction),
            rollback);

    return DMC_ERR_NONE;

rollback:

    /*
     * SQLite3 does not always automatically rollback transactions
     * when commit fails.
     */

    (void) prv_exec_simple_cmd(handle->stmt_cancel_transaction);

DMC_ON_ERR:

    return DMC_ERR;
}

int dmsettings_cancel_transaction(dmsettings *handle)
{
    DMC_ERR_MANAGE;

    if (!handle->transaction_in_progress)
        DMC_FAIL_FORCE(DMC_ERR_NOT_IN_TRANSACTION);

    handle->transaction_in_progress = false;

    DMC_ERR = prv_exec_simple_cmd(handle->stmt_cancel_transaction);

DMC_ON_ERR:

    return DMC_ERR;
}

int dmsettings_exists(dmsettings *handle, const char *key,
            dmsettings_settings_type *type)
{
    DMC_ERR_MANAGE;
    bool exists;

    if (!key)
        DMC_FAIL_FORCE(DMC_ERR_BAD_ARGS);

    DMC_FAIL(prv_exists_setting(handle, key, &exists));

    if (exists)
        *type = DMSETTINGS_TYPE_VALUE;
    else
    {
        DMC_FAIL(prv_exists(handle, key, &exists));
        *type = exists ? DMSETTINGS_TYPE_DIR :
                 DMSETTINGS_TYPE_NOT_EXISTS;
    }

DMC_ON_ERR:

    return DMC_ERR;
}

int dmsettings_get_children(dmsettings *handle, const char *key,
                dmc_ptr_array *children)
{
    DMC_ERR_MANAGE;

    if (!key)
        DMC_FAIL_FORCE(DMC_ERR_BAD_ARGS);

    DMC_FAIL(prv_find_children(handle, key, children));

DMC_ON_ERR:

    return DMC_ERR;
}

int dmsettings_get_value(dmsettings* handle, const char* key, char** value)
{
    DMC_ERR_MANAGE;
    const char *text_sqlite3;
    char *text_copy;

    DMC_FAIL(prv_bind_single_query(handle->stmt_get_value, key));

    text_sqlite3 = (const char*) sqlite3_column_text(handle->stmt_get_value,
                                0);

    if (text_sqlite3)
        DMC_FAIL_NULL(text_copy, strdup(text_sqlite3), DMC_ERR_OOM);
    else
        DMC_FAIL_FORCE(DMC_ERR_NOT_FOUND);

    *value= text_copy;

DMC_ON_ERR:

    sqlite3_reset(handle->stmt_get_value);
    sqlite3_clear_bindings(handle->stmt_get_value);

    return DMC_ERR;
}

int dmsettings_set_value(dmsettings* handle, const char* key, const char* value)
{
    DMC_ERR_MANAGE;
    dmsettings_settings_type type;

    if (!key || !value)
        DMC_FAIL_FORCE(DMC_ERR_BAD_ARGS);

    if (!handle->can_write)
        DMC_FAIL_FORCE(DMC_ERR_DENIED);

    /* This check is necessary otherwise the db could become corrupted */

    DMC_FAIL(dmsettings_exists(handle, key, &type));

    if (type == DMSETTINGS_TYPE_DIR)
        DMC_FAIL_FORCE(DMC_ERR_DENIED);

    DMC_FAIL(prv_bind_double_update(handle->stmt_set_value, key, value));

DMC_ON_ERR:

    return DMC_ERR;
}

int dmsettings_get_meta(dmsettings *handle, const char *key, const char *prop,
                char **value)
{
    DMC_ERR_MANAGE;
    const char *text_sqlite3;
    char *text_copy;

    DMC_FAIL(prv_bind_double_query(handle->stmt_get_meta, key, prop));

    text_sqlite3 = (const char*) sqlite3_column_text(handle->stmt_get_meta,
                                0);

    if (text_sqlite3)
        DMC_FAIL_NULL(text_copy, strdup(text_sqlite3), DMC_ERR_OOM);
    else
        DMC_FAIL_FORCE(DMC_ERR_NOT_FOUND);

    *value = text_copy;

DMC_ON_ERR:

    sqlite3_reset(handle->stmt_get_meta);
    sqlite3_clear_bindings(handle->stmt_get_meta);

    return DMC_ERR;
}

int dmsettings_set_meta(dmsettings *handle, const char *key, const char *prop,
                const char *value)
{
    DMC_ERR_MANAGE;

    if (!key || !prop || !value )
        DMC_FAIL_FORCE(DMC_ERR_BAD_ARGS);

    if (!handle->can_write)
        DMC_FAIL_FORCE(DMC_ERR_DENIED);

    DMC_ERR = prv_bind_triple_update(handle->stmt_set_meta, key, prop,
                     value);

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_delete_generic(dmsettings *handle, const char *key,
                const char *parent)
{
    DMC_ERR_MANAGE;

    DMC_FAIL(prv_bind_double_update(handle->stmt_delete_explicit, key,
                    key));

    if (sqlite3_changes(handle->db_handle) <= 0)
        DMC_FAIL_FORCE(DMC_ERR_NOT_FOUND);

    DMC_FAIL(prv_bind_double_update(handle->stmt_delete_meta, key, key));

    if (parent) {
        DMC_ERR = dmsettings_create_dir(handle, parent);

        if (DMC_ERR != DMC_ERR_ALREADY_EXISTS)
            DMC_FAIL(DMC_ERR);
        else
            DMC_ERR = DMC_ERR_NONE;
    }

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_delete_in_transaction(dmsettings *handle, const char *key,
                    const char *parent)
{
    DMC_ERR_MANAGE;

    DMC_FAIL(prv_exec_simple_cmd(handle->stmt_savepoint_set));

    DMC_FAIL_LABEL(prv_delete_generic(handle, key, parent),
                cancel_transaction);

    DMC_FAIL_LABEL(prv_exec_simple_cmd(handle->stmt_savepoint_commit),
                cancel_transaction);

    return DMC_ERR_NONE;

cancel_transaction:

    prv_exec_simple_cmd(handle->stmt_savepoint_cancel);

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_delete_out_transaction(dmsettings *handle, const char *key,
                    const char *parent)
{
    DMC_ERR_MANAGE;

    DMC_FAIL(dmsettings_begin_transaction(handle));

    DMC_FAIL_LABEL(prv_delete_generic(handle, key, parent),
                cancel_transaction);

    DMC_FAIL_LABEL(dmsettings_commit_transaction(handle),
                cancel_transaction);

    return DMC_ERR_NONE;

cancel_transaction:

    (void) dmsettings_cancel_transaction(handle);

DMC_ON_ERR:

    return DMC_ERR;
}

int dmsettings_delete(dmsettings *handle, const char *key)
{
    DMC_ERR_MANAGE;
    char *parent = NULL;
    const char *parent_path = NULL;
    char *tok;

    if (!key)
        DMC_FAIL_FORCE(DMC_ERR_BAD_ARGS);

    if (!handle->can_write)
        DMC_FAIL_FORCE(DMC_ERR_DENIED);

    DMC_FAIL_NULL(parent, strdup(key), DMC_ERR_OOM);

    tok = strrchr(parent,'/');

    if (tok) {
        *tok = 0;
        if (strchr(parent,'/'))
            parent_path = parent;
    }

    /*
     * If parent_path is not NULL then we need to ensure that this node
     * exists in the explicit table.  Otherwise the act of deleting a node
     * will delete some of its ancestors as well.
     */

    if (handle->transaction_in_progress)
        DMC_FAIL(prv_delete_in_transaction(handle, key, parent_path));
    else
        DMC_FAIL(prv_delete_out_transaction(handle, key, parent_path));

DMC_ON_ERR:

    free(parent);

    return DMC_ERR;
}

int dmsettings_create_dir(dmsettings *handle, const char *key)
{
    DMC_ERR_MANAGE;
    dmsettings_settings_type type;

    if (!key)
        DMC_FAIL_FORCE(DMC_ERR_BAD_ARGS);

    if (!handle->can_write)
        DMC_FAIL_FORCE(DMC_ERR_DENIED);

    /* This check is necessary otherwise the db could become corrupted */

    DMC_FAIL(dmsettings_exists(handle, key, &type));

    if (type != DMSETTINGS_TYPE_NOT_EXISTS)
        DMC_FAIL_FORCE(DMC_ERR_ALREADY_EXISTS);

    DMC_FAIL(prv_bind_double_update(handle->stmt_set_value, key, NULL));

DMC_ON_ERR:

    return DMC_ERR;
}
