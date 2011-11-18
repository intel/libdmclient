/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (c) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file omadmtree_mo.h
 *
 * @brief Header file for the dmtree management objects
 *
 */

#ifndef OMADMTREE_MO_H_
#define OMADMTREE_MO_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define OMADM_NODE_PROPERTY_VERSION 	"VerNo"
#define OMADM_NODE_PROPERTY_TIMESTAMP 	"TStamp"
#define OMADM_NODE_PROPERTY_FORMAT		"Format"
#define OMADM_NODE_PROPERTY_TYPE		"Type"
#define OMADM_NODE_PROPERTY_ACL			"ACL"
#define OMADM_NODE_PROPERTY_NAME		"Name"
#define OMADM_NODE_PROPERTY_SIZE		"Size"
#define OMADM_NODE_PROPERTY_TITLE		"Title"

typedef enum
{
	OMADM_NODE_NOT_EXIST,
	OMADM_NODE_IS_INTERIOR,
	OMADM_NODE_IS_LEAF,
} omadmtree_node_type_t;

typedef struct
{
	char *uri;
	char *format;
	char *type;
	unsigned int data_size;
	char *data_buffer;
} dmtree_node_t;


typedef int (*omadm_mo_init_fn) (void ** dataP);
typedef void (*omadm_mo_close_fn) (void * data);

typedef int (*omadm_mo_is_node_fn) (const char * uri, omadmtree_node_type_t * type, void * data);

typedef int (*omadm_mo_get_fn) (dmtree_node_t * nodeP, void * data);
typedef int (*omadm_mo_set_fn) (const dmtree_node_t * nodeP, void * data);

typedef int (*omadm_mo_get_ACL_fn) (const char * uri, char ** aclP, void * data);
typedef int (*omadm_mo_set_ACL_fn) (const char * uri, const char *acl, void * data);

typedef int (*omadm_mo_rename_fn) (const char * from, const char * to, void * data);
typedef int (*omadm_mo_delete_fn) (const char * uri, void * data);
typedef int (*omadm_mo_exec_fn) (const char * uri, const char * cmdData, const char * correlator, void * data);


typedef struct
{
    char * uri;
	omadm_mo_init_fn      initFunc;
	omadm_mo_close_fn     closeFunc;
	omadm_mo_is_node_fn   isNodeFunc;
	omadm_mo_get_fn       getFunc;
	omadm_mo_set_fn       setFunc;
	omadm_mo_get_ACL_fn   getACLFunc;
	omadm_mo_set_ACL_fn   setACLFunc;
	omadm_mo_rename_fn    renameFunc;
	omadm_mo_delete_fn    deleteFunc;
	omadm_mo_exec_fn      execFunc;
} omadm_mo_interface_t;

omadm_mo_interface_t * omadm_get_mo_interface(void);


#ifdef __cplusplus
}
#endif

#endif
