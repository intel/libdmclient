/******************************************************************************
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file static_mo_util.h
 *
 * @brief Header file for utility functions for static MOs
 *
 */

#ifndef STATIC_MO_UTIL_H_
#define STATIC_MO_UTIL_H_

#include <omadmtree_mo.h>

typedef struct
{
    char * uri;
    omadmtree_node_type_t type;
    char * acl;
    char * value;
} static_node_t;


int static_mo_is_node(const char *iURI, omadmtree_node_type_t *oNodeType, void *iData);
int static_mo_get(dmtree_node_t * nodeP, void *iData);
int static_mo_getACL(const char *iURI, char **oValue, void *iData);

#endif
