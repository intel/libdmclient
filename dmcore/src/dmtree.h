/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/
/*!
 * @file dmtree.h
 *
 * @brief Headers to access the DM tree
 *
 *****************************************************************************/

#ifndef OMADM_DMTREE_H
#define OMADM_DMTREE_H

#include <stdint.h>
#include <stdbool.h>

#include "dyn_buf.h"
#include "momgr.h"

typedef struct
{
	char *target_uri;
	char *format;
	char *type;
	unsigned int data_size;
	uint8_t *data_buffer;
} dmtree_node_t;

typedef struct
{
	OMADM_DMTreeContext *dmtree;
	char *server_id;
} dmtree_t;

int dmtree_open(const char *server_id, dmtree_t ** handleP);
void dmtree_close(dmtree_t * handle);

int dmtree_get_device_info(dmtree_t * handle, dmc_ptr_array *device_info);

int dmtree_get(dmtree_t * handle, const char *uri, dmtree_node_t **node);
int dmtree_add(dmtree_t * handle, const dmtree_node_t *node);
int dmtree_replace(dmtree_t * handle, const dmtree_node_t *node);
int dmtree_delete(dmtree_t * handle, const char *uri);
int dmtree_copy(dmtree_t * handle, const char *source_uri,	const char *target_uri);

int dmtree_validate_uri(const char *uri, bool allow_props);

void dmtree_node_free(dmtree_node_t *node);

#endif
