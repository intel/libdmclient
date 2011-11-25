/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007, ACCESS Systems Americas, Inc. All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file <momgr.c>
 *
 * @brief Management Object management code
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>
#include <stdbool.h>

#include "config.h"

#include "error_macros.h"
#include "log.h"

#include "syncml_error.h"
#include "momgr.h"
#include "internals.h"

#define OMADM_TOKEN_PROP "?prop="
#define OMADM_TOKEN_LIST "?list="

// defined in defaultroot.c
omadm_mo_interface_t * getDefaultRootPlugin();

/******
 * A valid URI for OMADM is :
 *    uri        = node_uri[ property | list ]
 *    property   = "?prop=" prop_name
 *    list       = "?list=" attribute
 *    node_uri   = "." | [ "./" ] path
 *    path       = segment *( "/" segment )
 *    segment    = *( pchar | "." ) pchar
 *    pchar      = unreserved | escaped | ":" | "@" | "&" | "=" | "+" | "$" | ","
 *    unreserved = alphanum | mark
 *    mark       = "-" | "_" | "!" | "~" | "*" | "'" | "(" | ")"
 *    escaped    = "%" hex hex
 *    hex        = digit | "A" | "B" | "C" | "D" | "E" | "F" |
 *                         "a" | "b" | "c" | "d" | "e" | "f"
 *    alphanum   = alpha | digit
 *    alpha      = lowalpha | upalpha
 *    lowalpha   = "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" |
 *                 "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" |
 *                 "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
 *    upalpha    = "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" |
 *                 "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" |
 *                 "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
 *    digit      = "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" |
 *                 "8" | "9"
 *
 *****/

static bool prv_check_hex(char target)
{
    if (((target >= 'A') && (target <= 'F'))
     || ((target >= 'a') && (target <= 'f'))
     || ((target >= '0') && (target <= '9')))
    {
        return true;
    }
    return false;
}

static bool prv_check_pchar(char target)
{
    if (((target >= 'A') && (target <= 'Z'))
     || ((target >= 'a') && (target <= 'z'))
     || ((target >= '0') && (target <= '9')))
    {
        return true;
    }
    switch (target)
    {
    case ':':
    case '@':
    case '&':
    case '=':
    case '+':
    case '$':
    case ',':
    case '-':
    case '_':
    case '!':
    case '~':
    case '*':
    case '\'':
    case '(':
    case ')':
        return true;
    default:
        break;
    }

    return false;
}

static int prv_validate_path(char * path_str,
                             uint16_t max_depth,
                             uint16_t max_len)
{
    int len = 0;

    if (*path_str == 0)
    {
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;
    }
    while (*path_str != 0 && *path_str != '/')
    {
        switch (*path_str)
        {
        case '.':
            if (*(path_str+1) == '/')
            {
                return OMADM_SYNCML_ERROR_COMMAND_FAILED;
            }
            path_str++;
            len++;
            break;
        case '%':
            if (!prv_check_hex(*(path_str+1))
             && !prv_check_hex(*(path_str+2)))
            {
                return OMADM_SYNCML_ERROR_COMMAND_FAILED;
            }
            path_str += 3;
            len += 3;
            break;
         default:
            if (!prv_check_pchar(*path_str))
            {
                return OMADM_SYNCML_ERROR_COMMAND_FAILED;
            }
            path_str++;
            len++;
            break;
        }
        if (max_len && len > max_len)
        {
            return OMADM_SYNCML_ERROR_URI_TOO_LONG;
        }
    }

    if (*path_str == '/')
    {
        switch (max_depth)
        {
        case 0:
            // unlimited depth
            return prv_validate_path(path_str+1, max_depth, max_len);
        case 1:
            // we already parsed one segment and there is more
            return OMADM_SYNCML_ERROR_URI_TOO_LONG;
        default:
            return prv_validate_path(path_str+1, max_depth-1, max_len);
        }
    }

    return OMADM_SYNCML_ERROR_NONE;
}

static int prv_get_short(const dmtree_plugin_t *plugin,
                         char * iURI,
                         uint16_t * resultP)
{
    int error;
    dmtree_node_t node;

    memset(&node, 0, sizeof(node));
    node.uri = iURI;

    error = plugin->interface->getFunc(&node, plugin->data);
    if (OMADM_SYNCML_ERROR_NONE == error)
    {
        if (1 != sscanf(node.data_buffer, "%hu", resultP))
        {
            error = OMADM_SYNCML_ERROR_SESSION_INTERNAL;
        }
        free(node.data_buffer);
    }

    return error;
}

static void prv_freePlugin(dmtree_plugin_t *oPlugin)
{
    if (oPlugin->URI)
        free(oPlugin->URI);

    if (oPlugin->interface) {
        if (oPlugin->interface->closeFunc)
        {
            oPlugin->interface->closeFunc(oPlugin->data);
        }
        free(oPlugin->interface);
    }

    if (oPlugin->dl_handle)
    {
        dlclose(oPlugin->dl_handle);
    }

    free(oPlugin);
}

static dmtree_plugin_t *prv_findPlugin(const mo_mgr_t iMgr,
                                       const char *iURI)
{
    plugin_elem_t * elem = iMgr.first;

    while(elem
       && strncmp(elem->plugin->URI, iURI, strlen(elem->plugin->URI)))
    {
        elem = elem->next;
    }

    if (elem)
    {
        return elem->plugin;
    }
    return NULL;
}

static void prv_removePlugin(mo_mgr_t * iMgrP,
                             const char *iURI)
{
    plugin_elem_t * elem = iMgrP->first;
    plugin_elem_t * target = NULL;

    if (elem)
    {
        if (strcmp(iURI, elem->plugin->URI))
        {
            while ((elem->next)
                && (strcmp(iURI, elem->next->plugin->URI)))
            {
                elem = elem->next;
            }
            if (elem->next)
            {
                target = elem->next;
                elem->next = elem->next->next;
            }
        }
        else
        {
            target = elem;
            iMgrP->first = iMgrP->first->next;
        }

        if (target)
        {
            prv_freePlugin(target->plugin);
            free(target);
        }
    }
}

static int prv_add_plugin(mo_mgr_t * iMgr,
                          const char *iURI,
                          omadm_mo_interface_t *iPlugin,
                          void * handle)
{
    DMC_ERR_MANAGE;
    plugin_elem_t * newElem = NULL;
    void * pluginData = NULL;

    DMC_LOGF("uri <%s>", iURI);

    DMC_FAIL_ERR(momgr_validate_uri(*iMgr, iURI, NULL, NULL), OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    /* Plugin base URI must be root (".") or a direct child of root ("./[^\/]**/
    // TODO

    DMC_FAIL_ERR(NULL == iPlugin, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == iPlugin->initFunc, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL(iPlugin->initFunc(&pluginData));

    DMC_FAIL_NULL(newElem, (plugin_elem_t *) malloc(sizeof(plugin_elem_t)),
              OMADM_SYNCML_ERROR_DEVICE_FULL);
    memset(newElem, 0, sizeof(plugin_elem_t));
    DMC_FAIL_NULL(newElem->plugin, (dmtree_plugin_t *) malloc(sizeof(dmtree_plugin_t)),
              OMADM_SYNCML_ERROR_DEVICE_FULL);

    memset(newElem->plugin, 0, sizeof(dmtree_plugin_t));

    DMC_FAIL_NULL(newElem->plugin->URI, strdup(iURI),
              OMADM_SYNCML_ERROR_DEVICE_FULL);
    newElem->plugin->interface = iPlugin;
    newElem->plugin->data = pluginData;
    newElem->plugin->dl_handle = handle;

    prv_removePlugin(iMgr, iURI);
    newElem->next = iMgr->first;
    iMgr->first = newElem;

    newElem = NULL;

DMC_ON_ERR:

    if (newElem)
    {
        if (newElem->plugin)
        {
            prv_freePlugin(newElem->plugin);
        }
        free(newElem);
    }

    DMC_LOGF("exit <0x%x>", DMC_ERR);

    return DMC_ERR;
}

void momgr_load_plugin(mo_mgr_t * iMgrP,
                       const char *iFilename)
{
    void * handle = NULL;
    omadm_mo_interface_t * moInterfaceP = NULL;
    omadm_mo_interface_t * (*getMoIfaceF)();

    if (iFilename == NULL)
    {
        return;
    }
    handle = dlopen(iFilename, RTLD_LAZY);
    if (!handle) goto error;

    getMoIfaceF = dlsym(handle, "omadm_get_mo_interface");
    if (!getMoIfaceF) goto error;

    moInterfaceP = getMoIfaceF();
    if ((!moInterfaceP) || (!moInterfaceP->uri)) goto error;

    if (OMADM_SYNCML_ERROR_NONE == prv_add_plugin(iMgrP, moInterfaceP->uri, moInterfaceP, handle))
    {
        handle = NULL;
    }
    // prv_add_plugin() would have free moInterfaceP in case of error
    moInterfaceP = NULL;

error:
    if (handle)
        dlclose (handle);
}

int momgr_init(mo_mgr_t * iMgrP)
{
    int error = OMADM_SYNCML_ERROR_NONE;
    DIR *folderP;
    dmtree_plugin_t * detailPluginP;

    if(!iMgrP)
        return OMADM_SYNCML_ERROR_SESSION_INTERNAL;

    memset(iMgrP, 0, sizeof(mo_mgr_t));

    folderP = opendir(MOBJS_DIR);
    if (folderP != NULL)
    {
        struct dirent *fileP;

        while ((fileP = readdir(folderP)))
        {
            if (DT_REG == fileP->d_type)
            {
                char * filename;

                filename = str_cat_3(MOBJS_DIR, "/", fileP->d_name);
                momgr_load_plugin(iMgrP, filename);
                free(filename);
            }
        }
        closedir(folderP);
    }

    // Check if we have a root plugin
    if (NULL == prv_findPlugin(*iMgrP, "."))
    {
        error = prv_add_plugin(iMgrP, "./", getDefaultRootPlugin(), NULL);
    }

    // retrieve uri limits
    detailPluginP = prv_findPlugin(*iMgrP, "./DevDetail");
    if (detailPluginP && detailPluginP->interface->getFunc)
    {
        error = prv_get_short(detailPluginP, "./DevDetail/URI/MaxDepth", &(iMgrP->max_depth));
        if (OMADM_SYNCML_ERROR_NONE == error)
        {
            error = prv_get_short(detailPluginP, "./DevDetail/URI/MaxTotLen", &(iMgrP->max_total_len));
            if (OMADM_SYNCML_ERROR_NONE == error)
            {
                error = prv_get_short(detailPluginP, "./DevDetail/URI/MaxSegLen", &(iMgrP->max_segment_len));
            }
        }
    }
    else
    {
        error = OMADM_SYNCML_ERROR_SESSION_INTERNAL;
    }

    if (OMADM_SYNCML_ERROR_NONE != error)
    {
        momgr_free(iMgrP);
    }
    return error;
}

void momgr_free(mo_mgr_t * iMgrP)
{
    while(iMgrP->first)
    {
        plugin_elem_t * elem = iMgrP->first;

        iMgrP->first = elem->next;
        prv_freePlugin(elem->plugin);
        free(elem);
    }
}

int momgr_exists(const mo_mgr_t iMgr,
                 const char *iURI,
                 omadmtree_node_type_t * oExists)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_LOGF("momgr_node_exists <%s>", iURI);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI),
               OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->isNodeFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);
    DMC_FAIL(plugin->interface->isNodeFunc(iURI, oExists, plugin->data));

    DMC_LOGF("momgr_node_exists exit <0x%x> %d",
                DMC_ERR, *oExists);

DMC_ON_ERR:

    return DMC_ERR;
}

int momgr_get_value(const mo_mgr_t iMgr,
                    dmtree_node_t * nodeP)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;
    char * childNode = NULL;
    omadmtree_node_type_t exists = OMADM_NODE_NOT_EXIST;
    plugin_elem_t * elem;

    DMC_FAIL_ERR(NULL == nodeP, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_get_value <%s>", nodeP->uri);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, nodeP->uri),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->getFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->getFunc(nodeP, plugin->data));

    if (strcmp(nodeP->uri, ".")) goto DMC_ON_ERR;

    /* Special case for the root node. */
    elem = iMgr.first;
    while(elem)
    {
        if (strcmp(elem->plugin->URI, "."))
        {
            DMC_FAIL_NULL(childNode, strdup(elem->plugin->URI), OMADM_SYNCML_ERROR_DEVICE_FULL);

            DMC_FAIL(elem->plugin->interface->isNodeFunc(childNode, &exists, elem->plugin->data));

            if (exists != OMADM_NODE_NOT_EXIST)
            {
                if (nodeP->data_buffer)
                {
                    char * tmp_str;

                    DMC_FAIL_NULL(tmp_str, str_cat_3(nodeP->data_buffer, "/", childNode+2), OMADM_SYNCML_ERROR_DEVICE_FULL);
                    free(nodeP->data_buffer);
                    nodeP->data_buffer = tmp_str;
                }
                else
                {
                    nodeP->data_buffer = strdup(childNode+2);
                }
            }
            free(childNode);
            childNode = NULL;
        }
        elem= elem->next;
    }
    if (!nodeP->format)
    {
        DMC_FAIL_NULL(nodeP->format, strdup("node"), OMADM_SYNCML_ERROR_DEVICE_FULL);
    }

DMC_ON_ERR:

    if (childNode) free(childNode);

    DMC_LOGF("momgr_get_value exit <0x%x>", DMC_ERR);

    return DMC_ERR;
}

int momgr_set_value(const mo_mgr_t iMgr,
                    const dmtree_node_t * nodeP)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == nodeP, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_set_value <%s>", nodeP->uri);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, nodeP->uri),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->setFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->setFunc(nodeP, plugin->data));

DMC_ON_ERR:

    DMC_LOGF("momgr_set_value exit <0x%x>", DMC_ERR);

    return DMC_ERR;
}

int momgr_get_ACL(const mo_mgr_t iMgr,
                  const char *iURI,
                  char **oACL)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == oACL, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_get_ACL <%s>", iURI);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI),
              OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->getACLFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->getACLFunc(iURI, oACL, plugin->data));

    DMC_LOGF("momgr_get_ACL value <%s>", *oACL);

DMC_ON_ERR:

    DMC_LOGF("momgr_get_ACL exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_set_ACL(const mo_mgr_t iMgr,
                  const char *iURI,
                  const char *iACL)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == iACL, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_set_ACL <%s> <%s>", iURI, iACL);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI),
              OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->setACLFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->setACLFunc(iURI, iACL, plugin->data));

    DMC_LOGF("momgr_set_ACL value <%s>", iACL);

DMC_ON_ERR:

    DMC_LOGF("momgr_set_ACL exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_rename_node(const mo_mgr_t iMgr,
                      const char *iURI,
                      const char *iNewName)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);
    DMC_FAIL_ERR(NULL == iNewName, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_rename_node <%s> <%s>", iURI, iNewName);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->renameFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    // TODO: check that iNewName is valid (here or in dmtree.c
    DMC_FAIL(plugin->interface->renameFunc(iURI, iNewName, plugin->data));

DMC_ON_ERR:

    DMC_LOGF("momgr_set_ACL exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_delete_node(const mo_mgr_t iMgr,
                      const char *iURI)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_delete_node <%s>", iURI);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->deleteFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->deleteFunc(iURI, plugin->data));

DMC_ON_ERR:

    DMC_LOGF("momgr_delete_node exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_exec_node(const mo_mgr_t iMgr,
                    const char *iURI,
                    const char *iData,
                    const char *iCorrelator)
{
    DMC_ERR_MANAGE;
    dmtree_plugin_t *plugin = NULL;

    DMC_FAIL_ERR(NULL == iURI, OMADM_SYNCML_ERROR_SESSION_INTERNAL);

    DMC_LOGF("momgr_exec_node <%s> <%s> <%s>", iURI, iData, iCorrelator);

    DMC_FAIL_NULL(plugin, prv_findPlugin(iMgr, iURI),
                  OMADM_SYNCML_ERROR_NOT_FOUND);

    DMC_FAIL_ERR(NULL == plugin->interface->execFunc,
                 OMADM_SYNCML_ERROR_NOT_ALLOWED);

    DMC_FAIL(plugin->interface->execFunc(iURI, iData, iCorrelator, plugin->data));

DMC_ON_ERR:

    DMC_LOGF("momgr_exec_node exit <%d>", DMC_ERR);

    return DMC_ERR;
}

int momgr_validate_uri(const mo_mgr_t iMgr,
                       const char * uri,
                       char ** oNodeURI,
                       char ** oPropId)
{
    DMC_ERR_MANAGE;

    unsigned int uri_len;
    char * node_uri = NULL;
    char * prop_str;
    char * path_str;

    DMC_FAIL_ERR(!uri, OMADM_SYNCML_ERROR_COMMAND_FAILED);
    DMC_FAIL_ERR(oPropId && !oNodeURI, OMADM_SYNCML_ERROR_COMMAND_FAILED);

    uri_len = strlen(uri);
    DMC_FAIL_ERR(uri_len == 0, OMADM_SYNCML_ERROR_COMMAND_FAILED);
    if (iMgr.max_total_len)
    {
        DMC_FAIL_ERR(uri_len > iMgr.max_total_len, OMADM_SYNCML_ERROR_URI_TOO_LONG);
    }

    DMC_FAIL_ERR(strstr(uri, OMADM_TOKEN_LIST) != NULL, OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED);

    DMC_FAIL_NULL(node_uri, strdup(uri), OMADM_SYNCML_ERROR_DEVICE_FULL);

    prop_str = strstr(node_uri, OMADM_TOKEN_PROP);
    if (prop_str)
    {
        DMC_FAIL_ERR(!oPropId, OMADM_SYNCML_ERROR_NOT_ALLOWED);
        DMC_FAIL_NULL(*oPropId, strdup(prop_str + strlen(OMADM_TOKEN_PROP)), OMADM_SYNCML_ERROR_DEVICE_FULL);
        *prop_str = 0;
        uri_len = strlen(node_uri);
        DMC_FAIL_ERR(uri_len == 0, OMADM_SYNCML_ERROR_COMMAND_FAILED);
    }

    DMC_FAIL_ERR((node_uri)[uri_len-1] == '/', OMADM_SYNCML_ERROR_COMMAND_FAILED);

    path_str = node_uri;

    if ((node_uri)[0] == '.')
    {
        switch ((node_uri)[1])
        {
        case 0:
            path_str = NULL;
            break;
        case '/':
            path_str += 2;
            break;
        default:
            break;
        }
    }
    else
    {
        node_uri = str_cat_2("./", path_str);
        if (!node_uri)
        {
            node_uri = path_str;
            DMC_FAIL(OMADM_SYNCML_ERROR_DEVICE_FULL);
        }
        free(path_str);
        path_str = node_uri + 2;
    }

    if (path_str)
    {
        DMC_FAIL(prv_validate_path(path_str, iMgr.max_depth, iMgr.max_segment_len));
    }

    if (oNodeURI)
    {
        *oNodeURI = node_uri;
    }
    else
    {
        free(node_uri);
    }

    DMC_LOGF("%s exitted with error %d",__FUNCTION__, DMC_ERR);
    return OMADM_SYNCML_ERROR_NONE;

DMC_ON_ERR:

    if (node_uri)
    {
        free(node_uri);
    }
    if (oPropId && *oPropId)
    {
        free(*oPropId);
        *oPropId = NULL;
    }
    DMC_LOGF("%s exitted with error %d",__FUNCTION__, DMC_ERR);

    return DMC_ERR;
}
