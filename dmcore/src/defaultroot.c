/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (c) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

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
    // TODO: use constants
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
        retval->uri = strdup(".");
        retval->initFunc = prv_init;
        retval->isNodeFunc = prv_isNode;
        retval->getACLFunc = prv_getACL;
        retval->getFunc = prv_get;
    }

    return retval;
}
