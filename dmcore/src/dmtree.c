/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (c) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file dmtree.c
 *
 * @brief Main source file for the DM tree.
 *
 *****************************************************************************/

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "error_macros.h"
#include "log.h"
#include "dmtree.h"
#include "internals.h"

#include "syncml_error.h"

/* Global constants */

#define OMADM_XML_FORMAT_VAL        "xml"
#define OMADM_NODE_FORMAT_VAL       "node"
#define OMADM_DMTNDS_XML_TYPE_VAL   "application/vnd.syncml.dmtnds+xml"
#define OMADM_DMTNDS_WBXML_TYPE_VAL "application/vnd.syncml.dmtnds+wbxml"

#define OMADM_COMMAND_ADD       "Add="
#define OMADM_COMMAND_GET       "Get="
#define OMADM_COMMAND_REPLACE   "Replace="
#define OMADM_COMMAND_EXECUTE   "Exec="
#define OMADM_COMMAND_DELETE    "Delete="


static int prv_check_acl_command(char *command)
{
    if (strcmp(command, "Get"))
    {
        if (strcmp(command, "Add"))
        {
            if (strcmp(command, "Replace"))
            {
                if (strcmp(command, "Delete"))
                {
                    if (strcmp(command, "Exec"))
                    {
                        return OMADM_SYNCML_ERROR_COMMAND_FAILED;
                    }
                }
            }
        }
    }

    return OMADM_SYNCML_ERROR_NONE;
}

static int prv_check_server_id(const char *server)
{
    if ((NULL == server) || (*server == 0))
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;

    while (*server)
    {
        if (!isascii((int)*server) ||
            !isprint((int)*server) ||
            isspace((int)*server) ||
            *server == '=' ||
            *server == '&' || *server == '*' || *server == '+')
                return OMADM_SYNCML_ERROR_COMMAND_FAILED;
        ++server;
    }

    return OMADM_SYNCML_ERROR_NONE;
}

static int prv_check_acl_syntax(const char *acl)
{
    DMC_ERR_MANAGE;

    char *acl_copy;
    char *cur_entry;
    char *entry_sep = NULL;
    char *cmd_sep = NULL;
    char *svr_sep = NULL;
    char *cur_server;

    DMC_FAIL_NULL(acl_copy, strdup(acl), OMADM_SYNCML_ERROR_DEVICE_FULL);
    cur_entry = acl_copy;

    /* Check entry list */
    while (cur_entry && *cur_entry)
    {
        entry_sep = strchr(cur_entry, '&');
        if (entry_sep) *entry_sep = 0;

        /* Get command name */
        DMC_FAIL_NULL(cmd_sep, strchr(cur_entry, '='), OMADM_SYNCML_ERROR_COMMAND_FAILED);
        *cmd_sep = 0;
        DMC_FAIL(prv_check_acl_command(cur_entry));

        /* Check server list */
        cur_server = cmd_sep + 1;
        DMC_FAIL_ERR(*cur_server == 0, OMADM_SYNCML_ERROR_COMMAND_FAILED);
        if (strcmp(cur_server, "*"))
        {
            do
            {
                /* Get server id */
                svr_sep = strchr(cur_server, '+');
                if (svr_sep) *svr_sep = 0;

                DMC_FAIL(prv_check_server_id(cur_server));

                /* Go next server */
                cur_server = svr_sep ? svr_sep + 1 : NULL;
            } while (cur_server);
        }
        /* Go next entry */
        cur_entry = entry_sep ? entry_sep + 1 : NULL;
    }

DMC_ON_ERR:

    if(acl_copy) free(acl_copy);

    return DMC_ERR;
}

static int prv_get_inherited_acl(dmtree_t * handle,
                                 const char *node_uri,
                                 char ** oACL)
{
    DMC_ERR_MANAGE;

    char *uri = NULL;
    unsigned int uriLen = 0;

    *oACL = NULL;

    DMC_FAIL_NULL(uri, strdup(node_uri), OMADM_SYNCML_ERROR_DEVICE_FULL);
    uriLen = strlen(uri);

    while ((uriLen > 0) && (NULL == *oACL))
    {
        DMC_ERR = momgr_get_ACL(handle->MOs, uri, oACL);
        if (OMADM_SYNCML_ERROR_NOT_FOUND != DMC_ERR)
        {
            DMC_FAIL(DMC_ERR);
        }

        while (uriLen > 0)
        {
            if (uri[--uriLen] == '/')
            {
                uri[uriLen] = 0;
                break;
            }
        }
    }

DMC_ON_ERR:

    if (uri) free(uri);

    return DMC_ERR;
}

static int prv_check_access_rights(dmtree_t * handle,
                                     const char *acl,
                                     const char *cmd_name)
{
    DMC_ERR_MANAGE;

    char * cmd_begin;
    char * cmd_end;

    DMC_FAIL_NULL(cmd_begin, strstr(acl, cmd_name), OMADM_SYNCML_ERROR_PERMISSION_DENIED);

    /* ACL contains this command */
    cmd_begin += strlen(cmd_name);
    cmd_end = strstr(cmd_begin, "&");
    if (cmd_end) *cmd_end = 0;

    /* Check if "*" */
    if (strcmp(cmd_begin, "*"))
    {
        char * server;

        /* Look for server_id in list separated by "+" */
        DMC_FAIL_NULL(server, strstr(cmd_begin, handle->server_id), OMADM_SYNCML_ERROR_PERMISSION_DENIED);

        if (((server > cmd_begin) && (*(server-1) != '+'))
         || ((server[strlen(handle->server_id)] != 0) && (server[strlen(handle->server_id)] != '+')))
        {
            DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_PERMISSION_DENIED);
        }
    }

DMC_ON_ERR:

    return DMC_ERR;
}

static int prv_check_node_acl_rights(dmtree_t * handle,
                                     const char *node_uri,
                                     const char *cmd_name)
{
    DMC_ERR_MANAGE;

    char *acl = NULL;

    DMC_FAIL(prv_get_inherited_acl(handle, node_uri, &acl));
    DMC_FAIL_ERR(!acl, OMADM_SYNCML_ERROR_COMMAND_FAILED);
    DMC_FAIL(prv_check_access_rights(handle, acl, cmd_name));

DMC_ON_ERR:

    if (acl) free(acl);

    return DMC_ERR;
}

int dmtree_get(dmtree_t * handle, dmtree_node_t *node)
{
    DMC_ERR_MANAGE;

    omadmtree_node_type_t node_exists;
    char * target_uri = NULL;
    char * prop_id = NULL;

    DMC_FAIL_ERR(!node || !node->uri, OMADM_SYNCML_ERROR_COMMAND_FAILED);

    DMC_LOGF("%s called. URI %s", __FUNCTION__, node->uri);

    node->format = NULL;
    node->type = NULL;
    node->data_buffer = NULL;
    node->data_size = 0;

    DMC_FAIL(momgr_validate_uri(handle->MOs, node->uri, &target_uri, &prop_id));

    if (prop_id)
    {
        DMC_FAIL(momgr_exists(handle->MOs, target_uri, &node_exists));
        DMC_FAIL_ERR(node_exists == OMADM_NODE_NOT_EXIST, OMADM_SYNCML_ERROR_NOT_FOUND);
        DMC_FAIL(prv_check_node_acl_rights(handle, target_uri, OMADM_COMMAND_GET));

        if (!strcmp(prop_id, OMADM_NODE_PROPERTY_NAME))
        {
            char * name;

            name = strrchr(target_uri, '/');
            if (!name) name = target_uri;
            else name++;

            DMC_FAIL_NULL(node->data_buffer, strdup(name), OMADM_SYNCML_ERROR_DEVICE_FULL);
            node->data_size = strlen(node->data_buffer);
            node->format = strdup("chr");
            node->type = strdup("text/plain");
        }
        else if (!strcmp(prop_id, OMADM_NODE_PROPERTY_ACL))
        {
            DMC_FAIL(momgr_get_ACL(handle->MOs, target_uri, &(node->data_buffer)));
            node->data_size = strlen(node->data_buffer);
            node->format = strdup("chr");
            node->type = strdup("text/plain");
        }
        else if (!strcmp(prop_id, OMADM_NODE_PROPERTY_TITLE))
        {
           DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED);
        }
        else
        {
            DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_NOT_ALLOWED);
        }
    }
    else
    {
        char * tmp_uri;

        DMC_FAIL(momgr_exists(handle->MOs, target_uri, &node_exists));
        DMC_FAIL_ERR(node_exists == OMADM_NODE_NOT_EXIST, OMADM_SYNCML_ERROR_NOT_FOUND);
        DMC_FAIL(prv_check_node_acl_rights(handle, target_uri, OMADM_COMMAND_GET));
        tmp_uri = node->uri;
        node->uri = target_uri;
        DMC_ERR = momgr_get_value(handle->MOs, node);
        node->uri = tmp_uri;
        DMC_FAIL(DMC_ERR);
    }

DMC_ON_ERR:

    DMC_LOGF("%s finished with error %d",__FUNCTION__, DMC_ERR);

    if (target_uri) free(target_uri);
    if (prop_id) free(prop_id);

    return DMC_ERR;
}

int dmtree_delete(dmtree_t * handle, const char *uri)
{
    DMC_ERR_MANAGE;

    char * target_uri = NULL;
    omadmtree_node_type_t node_exists;

    DMC_LOGF("%s called.", __FUNCTION__);

    DMC_FAIL_ERR(!uri, OMADM_SYNCML_ERROR_COMMAND_FAILED);

    DMC_LOGF("deleting %s", uri);

    DMC_FAIL(momgr_validate_uri(handle->MOs, uri, &target_uri, NULL));

    DMC_FAIL(momgr_exists(handle->MOs, target_uri, &node_exists));

    DMC_FAIL_ERR(node_exists == OMADM_NODE_NOT_EXIST, OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL(prv_check_node_acl_rights(handle, target_uri, OMADM_COMMAND_DELETE));

    DMC_FAIL(momgr_delete_node(handle->MOs, target_uri));

DMC_ON_ERR:

    if (target_uri) free(target_uri);

    DMC_LOGF("%s finished with error %d",__FUNCTION__, DMC_ERR);

    return DMC_ERR;
}

int dmtree_add(dmtree_t * handle, dmtree_node_t *node)
{
    DMC_ERR_MANAGE;

    char * target_uri = NULL;
    omadmtree_node_type_t node_exists;
    char *parent_uri = NULL;
    char *parent_acl = NULL;
    char * token = NULL;
    char * new_acl = NULL;
    char * tmp_str = NULL;
    char * tmp_uri = NULL;
    int error;

    DMC_LOGF("%s called.", __FUNCTION__);

    DMC_FAIL_ERR(!node || !node->uri, OMADM_SYNCML_ERROR_COMMAND_FAILED);

    DMC_LOGF("adding %s", node->uri);

    DMC_FAIL(momgr_validate_uri(handle->MOs, node->uri, &target_uri, NULL));

    if ((node->format) && (node->type) &&
        (!strcmp(node->format, OMADM_XML_FORMAT_VAL)) &&
        (!strcmp(node->type, OMADM_DMTNDS_XML_TYPE_VAL) ||
         !strcmp(node->type, OMADM_DMTNDS_WBXML_TYPE_VAL)))
        DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED);

    DMC_FAIL(momgr_exists(handle->MOs, target_uri, &node_exists));

    DMC_FAIL_ERR((node_exists != OMADM_NODE_NOT_EXIST), OMADM_SYNCML_ERROR_ALREADY_EXISTS);

    DMC_FAIL_NULL(parent_uri, strdup(target_uri), OMADM_SYNCML_ERROR_DEVICE_FULL);
    DMC_FAIL_NULL(token, strrchr(parent_uri, '/'), OMADM_SYNCML_ERROR_COMMAND_FAILED);
    *token = 0;
    // If the parent is a leaf node we cannot add. Neither if it does not exist.
    DMC_FAIL(momgr_exists(handle->MOs, parent_uri, &node_exists));
    DMC_FAIL_ERR((node_exists != OMADM_NODE_IS_INTERIOR), OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL(prv_get_inherited_acl(handle, parent_uri, &parent_acl));
    DMC_FAIL_ERR(!parent_acl, OMADM_SYNCML_ERROR_COMMAND_FAILED);
    DMC_FAIL(prv_check_access_rights(handle, parent_acl, OMADM_COMMAND_ADD));

    tmp_uri = node->uri;
    node->uri = target_uri;
    DMC_ERR = momgr_set_value(handle->MOs, node);
    node->uri = tmp_uri;
    DMC_FAIL(DMC_ERR);

    // if parent's ACL does not allow replace, set node's ACL to give creating
    // server Add, Delete and Replace rights
    error = prv_check_access_rights(handle, parent_acl, OMADM_COMMAND_REPLACE);
    switch(error)
    {
    case OMADM_SYNCML_ERROR_NONE:
        // OK
        break;
    case OMADM_SYNCML_ERROR_PERMISSION_DENIED:
        {
            DMC_FAIL_NULL(new_acl, str_cat_3("Get=*&", OMADM_COMMAND_ADD, handle->server_id), OMADM_SYNCML_ERROR_DEVICE_FULL);
            DMC_FAIL_NULL(tmp_str, str_cat_3(new_acl, "&", OMADM_COMMAND_DELETE), OMADM_SYNCML_ERROR_DEVICE_FULL);
            free(new_acl);
            new_acl = NULL;
            DMC_FAIL_NULL(new_acl, str_cat_5(tmp_str, handle->server_id, "&", OMADM_COMMAND_REPLACE, handle->server_id), OMADM_SYNCML_ERROR_DEVICE_FULL);
            DMC_FAIL(momgr_set_ACL(handle->MOs, target_uri, new_acl));
        }
        break;
    default:
        DMC_FAIL(error);
    }

DMC_ON_ERR:

    if (parent_uri) free(parent_uri);
    if (parent_acl) free(parent_acl);
    if (new_acl) free(new_acl);
    if (tmp_str) free(tmp_str);

    DMC_LOGF("%s finished with error %d",__FUNCTION__, DMC_ERR);

    return DMC_ERR;
}

int dmtree_replace(dmtree_t * handle, dmtree_node_t *node)
{
    DMC_ERR_MANAGE;
    omadmtree_node_type_t node_exists;
    char *target_uri = NULL;
    char *prop_id = NULL;

    DMC_LOGF("%s called.", __FUNCTION__);

    if (!node || !node->uri || !node->data_buffer)
        DMC_FAIL(OMADM_SYNCML_ERROR_COMMAND_FAILED);

    DMC_LOGF("replacing %s", node->uri);

    DMC_FAIL(momgr_validate_uri(handle->MOs, node->uri, &target_uri, &prop_id));

    if (prop_id)
    {
        DMC_FAIL(momgr_exists(handle->MOs, target_uri, &node_exists));
        DMC_FAIL_ERR(node_exists == OMADM_NODE_NOT_EXIST, OMADM_SYNCML_ERROR_NOT_FOUND);
        DMC_FAIL(prv_check_node_acl_rights(handle, target_uri, OMADM_COMMAND_REPLACE));

        if (!strcmp(prop_id, OMADM_NODE_PROPERTY_NAME))
        {
            DMC_FAIL_ERR(!strstr(node->data_buffer, "/"), OMADM_SYNCML_ERROR_COMMAND_FAILED);
            DMC_FAIL(momgr_rename_node(handle->MOs, target_uri, node->data_buffer));
        }
        else if (!strcmp(prop_id, OMADM_NODE_PROPERTY_ACL))
        {
            DMC_FAIL(prv_check_acl_syntax(node->data_buffer));
            DMC_FAIL(momgr_set_ACL(handle->MOs, target_uri, node->data_buffer));
        }
        else if (!strcmp(prop_id, OMADM_NODE_PROPERTY_TITLE))
        {
           DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED);
        }
        else
        {
            DMC_FAIL_FORCE(OMADM_SYNCML_ERROR_NOT_ALLOWED);
        }
    }
    else
    {
        char * tmp_uri;

        DMC_FAIL(momgr_exists(handle->MOs, target_uri, &node_exists));
        DMC_FAIL_ERR(node_exists == OMADM_NODE_NOT_EXIST, OMADM_SYNCML_ERROR_NOT_FOUND);
        DMC_FAIL(prv_check_node_acl_rights(handle, target_uri, OMADM_COMMAND_REPLACE));
        tmp_uri = node->uri;
        node->uri = target_uri;
        DMC_ERR = momgr_set_value(handle->MOs, node);
        node->uri = tmp_uri;
        DMC_FAIL(DMC_ERR);
    }

DMC_ON_ERR:

    if (target_uri) free(target_uri);
    if (prop_id) free(prop_id);

    DMC_LOGF("%s finished with error %d",__FUNCTION__, DMC_ERR);

    return DMC_ERR;
}

int dmtree_copy(dmtree_t * handle, const char *source_uri,
               const char *target_uri)
{
    // Ignore for now
    return OMADM_SYNCML_ERROR_COMMAND_NOT_IMPLEMENTED;
}

int dmtree_open(const char *server_id, dmtree_t **handleP)
{
    DMC_ERR_MANAGE;

    dmtree_t *retval = NULL;

dmc_log_open("/tmp/testlog");
    DMC_LOGF("Creating dm handle with server %s", server_id);

    DMC_FAIL_ERR(prv_check_server_id(server_id),
                OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_FAIL_NULL(retval, malloc(sizeof(*retval)),
               OMADM_SYNCML_ERROR_DEVICE_FULL);

    memset(retval, 0, sizeof(*retval));

    DMC_FAIL_NULL(retval->server_id, strdup(server_id),
               OMADM_SYNCML_ERROR_DEVICE_FULL);

    DMC_FAIL(momgr_init(&(retval->MOs)));

    *handleP = retval;

    DMC_LOG("dm handle created correctly");

    return OMADM_SYNCML_ERROR_NONE;

DMC_ON_ERR:

    DMC_LOGF("Failed to create dm handle with error %d", DMC_ERR);

    dmtree_close(retval);

    return DMC_ERR;
}

void dmtree_close(dmtree_t * handle)
{
    DMC_LOG("Freeing dm handle");

    if (handle)
    {
        momgr_free(&(handle->MOs));
        free(handle->server_id);
        free(handle);
    }
}
