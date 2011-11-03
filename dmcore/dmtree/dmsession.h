/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/
/*!
 * @file dmsession.h
 *
 * @brief main API for libomadm
 *
 *****************************************************************************/

#ifndef OMADM_DMSESSION_H
#define OMADM_DMSESSION_H 

#include <stdint.h>

#include "dyn_buf.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dmtree_session_ dmtree_session;

typedef struct dmtree_node_ dmtree_node;
struct dmtree_node_ {
	char *target_uri;
	char *format;
	char *type;
	unsigned int data_size;
	uint8_t *data_buffer;
};

int dmtree_session_create(const char *server_id, dmtree_session **session);
void dmtree_session_free(dmtree_session *session);

int dmtree_session_device_info(dmtree_session *session,
				dmc_ptr_array *device_info);

int dmtree_session_get(dmtree_session *session, const char *uri,
                        dmtree_node **node);
int dmtree_session_add(dmtree_session *session, const dmtree_node *node);
int dmtree_session_replace(dmtree_session *session, const dmtree_node *node);
int dmtree_session_delete(dmtree_session *session, const char *uri);
int dmtree_session_copy(dmtree_session *session, const char *source_uri,
			const char *target_uri);

void dmtree_node_free(dmtree_node *node);

#ifdef __cplusplus
}
#endif

#endif
