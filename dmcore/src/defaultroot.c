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
 * @file <defaultroot.c>
 *
 * @brief Default Root MO
 *
 *****************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "omadmtree_mo.h"
#include "syncml_error.h"

static int prv_init(void ** dataP)
{
    *dataP = NULL;
    return OMADM_SYNCML_ERROR_NONE;
}

static int prv_isNode(const char *uri, omadmtree_node_type_t* node_type, void *data)
{
    if (!strcmp(uri,"."))
    {
        *node_type = OMADM_NODE_IS_INTERIOR;
    }
    else
    {
        *node_type = OMADM_NODE_NOT_EXIST;
    }

    return OMADM_SYNCML_ERROR_NONE;
}

static int prv_getACL(const char * uri, char ** aclP, void * data)
{
    *aclP = NULL;

    if (strcmp(uri,"."))
    {
        return OMADM_SYNCML_ERROR_NOT_FOUND;
    }

    *aclP = strdup("Get=*");

    return OMADM_SYNCML_ERROR_NONE;
}

static int prv_get(dmtree_node_t * nodeP,
                   void *oData)
{
    if (strcmp(nodeP->uri, "."))
    {
        return OMADM_SYNCML_ERROR_NOT_FOUND;
    }

    nodeP->format = strdup("node");
    nodeP->data_size = 0;
    nodeP->data_buffer = NULL;

    return OMADM_SYNCML_ERROR_NONE;
}

omadm_mo_interface_t * getDefaultRootPlugin()
{
    omadm_mo_interface_t *retval = NULL;

    retval = malloc(sizeof(*retval));
    if (retval) {
        memset(retval, 0, sizeof(*retval));
        retval->base_uri = strdup(".");
        retval->initFunc = prv_init;
        retval->isNodeFunc = prv_isNode;
        retval->getACLFunc = prv_getACL;
        retval->getFunc = prv_get;
    }

    return retval;
}
