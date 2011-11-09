/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file omadm_dmtree_plugin_prv.h
 *
 * @brief Header file for the inbox plugin
 *
 */

#ifndef OMADM_DMTREE_PLUGIN_H_
#define OMADM_DMTREE_PLUGIN_H_

#include <stdbool.h>
#include <stdint.h>

#include "dyn_buf.h"

#define OMADM_NODE_PROPERTY_VERSION 		"VerNo"
#define OMADM_NODE_PROPERTY_TIMESTAMP 		"TStamp"
#define OMADM_NODE_PROPERTY_FORMAT		"Format"
#define OMADM_NODE_PROPERTY_TYPE		"Type"
#define OMADM_NODE_PROPERTY_ACL			"ACL"
#define OMADM_NODE_PROPERTY_NAME		"Name"
#define OMADM_NODE_PROPERTY_SIZE		"Size"
#define OMADM_NODE_PROPERTY_TITLE		"Title"

enum OMADM_NodeType {
	OMADM_NODE_NOT_EXIST,
	OMADM_NODE_IS_INTERIOR,
	OMADM_NODE_IS_LEAF,
};

typedef enum OMADM_NodeType OMADM_NodeType;

enum OMADM_AccessType {
	OMADM_ACCESS_GET = 1,
	OMADM_ACCESS_ADD = 2,
	OMADM_ACCESS_REPLACE = 4,
	OMADM_ACCESS_DELETE = 8,
	OMADM_ACCESS_EXEC = 16
};

typedef enum OMADM_AccessType OMADM_AccessType;
typedef unsigned int OMADM_AccessRights;
typedef int(*OMADM_CreateFN) (const char *, void **);
typedef void (*OMADM_FreeFN) (void *);

typedef int(*OMADM_NodeExistsFN) (const char *, OMADM_NodeType *, void *);
typedef int(*OMADM_GetNodeChildrenFN) (const char *, dmc_ptr_array *, void *);

typedef int(*OMADM_GetValueFN) (const char *, char **, void *);
typedef int(*OMADM_SetValueFN) (const char *, const char *, void *);

typedef int(*OMADM_GetMetaFN) (const char *, const char *, char **, void *);
typedef int(*OMADM_SetMetaFN) (const char *, const char *, const char *,
			       void *);
typedef int(*OMADM_GetAccessRightsFN) (const char *, OMADM_AccessRights	*,
				       void *);
typedef int(*OMADM_CreateNonLeafFN) (const char *, void *);

typedef int(*OMADM_DeleteNodeFN) (const char *, void *);
typedef int(*OMADM_ExecNodeFN) (const char *, const char *, const char *,
				void *);

typedef int(*OMADM_UpdateNonceFN) (const char *, const uint8_t *, unsigned int,
				   bool, void *);

typedef struct OMADM_DMTreePlugin_ OMADM_DMTreePlugin;

struct OMADM_DMTreePlugin_ {
	OMADM_CreateFN create;
	OMADM_FreeFN free;
	OMADM_NodeExistsFN nodeExists;
	OMADM_GetNodeChildrenFN getNodeChildren;
	OMADM_GetValueFN getValue;
	OMADM_SetValueFN setValue;
	OMADM_GetMetaFN getMeta;
	OMADM_SetMetaFN setMeta;
	OMADM_GetAccessRightsFN getAccessRights;
	OMADM_DeleteNodeFN deleteNode;
	OMADM_ExecNodeFN execNode;
	OMADM_UpdateNonceFN updateNonce;
	OMADM_CreateNonLeafFN createNonLeaf;
	bool supportTransactions;
};

typedef OMADM_DMTreePlugin *(*OMADM_PluginCreateFN) (void);

typedef struct OMADM_PluginDesc OMADM_PluginDesc;

struct OMADM_PluginDesc {
	char *uri;
	OMADM_PluginCreateFN createFunc;
};

OMADM_PluginDesc * omadm_get_plugin_desc(void);

#endif				// #ifndef OMADM_DMTREE_PLUGIN_H_
