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


#define OMADM_NODE_PROPERTY_VERSION     "VerNo"
#define OMADM_NODE_PROPERTY_TIMESTAMP   "TStamp"
#define OMADM_NODE_PROPERTY_FORMAT      "Format"
#define OMADM_NODE_PROPERTY_TYPE        "Type"
#define OMADM_NODE_PROPERTY_ACL         "ACL"
#define OMADM_NODE_PROPERTY_NAME        "Name"
#define OMADM_NODE_PROPERTY_SIZE        "Size"
#define OMADM_NODE_PROPERTY_TITLE       "Title"

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


/*!
 * @brief Callback to initialize the MO (MANDATORY)
 *
 * @param dataP (out) opaque pointer to MO internal data
 *
 * @returns a SyncML error code
 */
typedef int (*omadm_mo_init_fn) (void ** dataP);

/*!
 * @brief Callback to free the MO
 *
 * @param data (in) MO internal data as created by #omadm_mo_init_fn
 */
typedef void (*omadm_mo_close_fn) (void * data);

/*!
 * @brief Callback to get the type of a node
 *
 * @param uri (in) URL of the node
 * @param type (out) type of the node
 * @param data (in) MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a SyncML error code
 */
typedef int (*omadm_mo_is_node_fn) (const char * uri, omadmtree_node_type_t * type, void * data);

/*!
 * @brief Callback to find the URLs associated to an URN
 *
 * @param urn (in) URN to find
 * @param urlsP (out) null-terminated array of urls, freed by the caller
 * @param data (in) MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a SyncML error code
 */
typedef int (*omadm_mo_find_urn_fn) (const char * urn, char *** urlsP, void * data);

/*!
 * @brief Callback to set the value of a node
 *
 * result is stored in the nodeP parameter. If the targeted node is an
 * interior node, the nodeP->data_buffer must be a char * containing
 * the node's children's names separated by '/'.
 *
 * @param nodeP (in/out) the node to retrieve
 * @param data (in) MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a SyncML error code
 */
typedef int (*omadm_mo_get_fn) (dmtree_node_t * nodeP, void * data);

/*!
 * @brief Callback to get the value of a node
 *
 * The targeted node can already exist. This is used both for ADD
 * and REPLACE SyncML commands.
 * If nodeP-> type is "node", an interior node must be created.
 *
 * @param nodeP (in) the node to store
 * @param data (in) MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a SyncML error code
 */
typedef int (*omadm_mo_set_fn) (const dmtree_node_t * nodeP, void * data);

/*!
 * @brief Callback to get the ACL of a node
 *
 * The ACL string must be allocated by this function. It will be
 * freed by the caller.
 * If the node has no ACL, *aclP must be NULL.
 *
 * @param uri (in) URL of the node
 * @param aclP (out) ACL of the node
 * @param data (in) MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a SyncML error code
 */
typedef int (*omadm_mo_get_ACL_fn) (const char * uri, char ** aclP, void * data);

/*!
 * @brief Callback to set the ACL of a node
 *
 * @param uri (in) URL of the node
 * @param acl (in) ACL of the node
 * @param data (in) MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a SyncML error code
 */
typedef int (*omadm_mo_set_ACL_fn) (const char * uri, const char *acl, void * data);

/*!
 * @brief Callback to rename a node
 *
 * The to parameter contains only the new name of the node, not the
 * complete new URL.
 *
 * @param from (in) URL of the node
 * @param to (in) new name of the node
 * @param data (in) MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a SyncML error code
 */
typedef int (*omadm_mo_rename_fn) (const char * from, const char * to, void * data);

/*!
 * @brief Callback to delete a node
 *
 * @param uri (in) URL of the node
 * @param data (in) MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a SyncML error code
 */
typedef int (*omadm_mo_delete_fn) (const char * uri, void * data);

/*!
 * @brief Callback to execute the function associated to a node
 *
 * @param uri (in) URL of the node
 * @param cmdData (in) parameter past to the EXEC SyncML command
 * @param correlator (in) correlator associated to the EXEC SyncML command
 * @param data (in) MO internal data as created by #omadm_mo_init_fn
 *
 * @returns a SyncML error code
 */
typedef int (*omadm_mo_exec_fn) (const char * uri, const char * cmdData, const char * correlator, void * data);


/*!
 * @brief Structure containing the interface of the MO
 *
 * base_uri and initFunc must be set. Other callbacks can be null.
 * The MO can not be root (i.e. base_uri must differ from ".").
 *
 */
typedef struct
{
    char * base_uri;                    /*!< base URI of the MO */
    omadm_mo_init_fn      initFunc;     /*!< initialization function */
    omadm_mo_close_fn     closeFunc;
    omadm_mo_is_node_fn   isNodeFunc;
    omadm_mo_find_urn_fn  findURNFunc;
    omadm_mo_get_fn       getFunc;
    omadm_mo_set_fn       setFunc;
    omadm_mo_get_ACL_fn   getACLFunc;
    omadm_mo_set_ACL_fn   setACLFunc;
    omadm_mo_rename_fn    renameFunc;
    omadm_mo_delete_fn    deleteFunc;
    omadm_mo_exec_fn      execFunc;
} omadm_mo_interface_t;


/*!
 * @brief Entry point of the shared lib
 *
 * The returned pointer ust be allocated by this function.
 * The caller will call closeFunc (if any) before freeing the pointer.
 * The caller will also free the uri string inside.
 *
 * @returns a pointer tothe MO interface
 */
omadm_mo_interface_t * omadm_get_mo_interface(void);


#ifdef __cplusplus
}
#endif

#endif
