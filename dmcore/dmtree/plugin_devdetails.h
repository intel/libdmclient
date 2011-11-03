/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/


/*!
 * @file plugin_devdetails_prv.h 
 *
 * @brief Header file for the devdetails plugin
 *
 */

#ifndef OMADM_PLUGIN_DEVDETAILS_H_
#define OMADM_PLUGIN_DEVDETAILS_H_

#include "dmtree_plugin.h"

#define OMADM_DEVDETAILS_MAX_SEG_LEN 64
#define OMADM_DEVDETAILS_MAX_TOT_LEN 256
#define OMADM_DEVDETAILS_MAX_DEPTH_LEN 16

OMADM_DMTreePlugin *omadm_create_devdetails_plugin();

#endif				// #ifndef OMADM_PLUGIN_DEVDETAILS_H_
