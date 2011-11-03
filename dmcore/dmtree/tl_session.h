/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007, ACCESS Systems Americas, Inc. All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file parser_tl.h
 *
 * @brief Definitions for the DM Client test language
 *
 *****************************************************************************/

#ifndef PARSER_TL_PRIV_H__
#define PARSER_TL_PRIV_H__

#include "dyn_buf.h"
#include "command_tl.h"

typedef struct omadm_tl_session_ omadm_tl_session;
struct omadm_tl_session_ {
	char *server_id;
	dmc_ptr_array packages;
};

typedef dmc_ptr_array DMTLPackage;

void omadm_tl_session_free(omadm_tl_session *session);
int omadm_tl_session_parse(const char *file_name, omadm_tl_session **session);

#define omadm_tl_session_get_package_count(session) \
dmc_ptr_array_get_size(&((session)->packages))

#define omadm_tl_session_get_package(session, package) \
	dmc_ptr_array_get(&(session)->packages, package)

#define omadm_tl_session_get_command_count(package) \
dmc_ptr_array_get_size(package)

#define omadm_tl_session_get_command(package, command) \
	dmc_ptr_array_get((package), (command))

#ifdef DMC_LOGGING 
void omadm_tl_session_trace(omadm_tl_session * iSession);
#endif

#endif				/* OTA_PARSER_TL_PRIV_H__ */
