/******************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007 PalmSource, Inc (an ACCESS company). All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file command_tl.h
 *
 * @brief Definitions for the dm command structure
 *
 *****************************************************************************/

#ifndef OMADM_COMMAND_TL_H__
#define OMADM_COMMAND_TL_H__

#include <stdint.h>
#include <stdbool.h>

#include "dyn_buf.h"

enum _DMTLCommandType {
	OMADM_COMT_ADD,
	OMADM_COMT_ALERT_DISPLAY,
	OMADM_COMT_ALERT_CONFIRM,
	OMADM_COMT_ALERT_INPUT,
	OMADM_COMT_ALERT_SINGLE,
	OMADM_COMT_ALERT_MULTIPLE,
	OMADM_COMT_ATOMIC,
	OMADM_COMT_COPY,
	OMADM_COMT_DELETE,
	OMADM_COMT_EXEC,
	OMADM_COMT_GET,
	OMADM_COMT_REPLACE,
	OMADM_COMT_SEQUENCE,
	OMADM_COMT_SLEEP,
	OMADM_COMT_DEVICE_INFO
};

typedef enum _DMTLCommandType DMTLCommandType;

typedef struct DMTLNode_ DMTLNode;

struct DMTLNode_ {
	char *itsName;
	char *itsFormat;
	char *itsType;
	char *itsData;
};

typedef struct DMTLDisplayParams_ DMTLDisplayParams;
struct DMTLDisplayParams_ {
	int32_t itsMinSecs;
	int32_t itsMaxSecs;
	char *itsMessage;
};

typedef struct DMTLAddCommand_ DMTLAddCommand;
struct DMTLAddCommand_ {
	char *itsURI;
	DMTLNode itsNode;
};

typedef struct DMTLCopyCommand_ DMTLCopyCommand;
struct DMTLCopyCommand_ {
	char *itsSourceURI;
	char *itsTargetURI;
};

typedef struct DMTLDeleteCommand_ DMTLDeleteCommand;
struct DMTLDeleteCommand_ {
	char *itsURI;
};

typedef struct DMTLExecCommand_ DMTLExecCommand;
struct DMTLExecCommand_ {
	char *itsURI;
	char *itsData;
};

typedef struct DMTLGetCommand_ DMTLGetCommand;
struct DMTLGetCommand_ {
	char *itsURI;
};

typedef struct DMTLReplaceCommand_ DMTLReplaceCommand;
struct DMTLReplaceCommand_ {
	char *itsURI;
	DMTLNode itsNode;
};

typedef struct DMTLAlertDisplayCommand_ DMTLAlertDisplayCommand;
struct DMTLAlertDisplayCommand_ {
	DMTLDisplayParams itsParams;
};

typedef struct DMTLAlertConfirmCommand_ DMTLAlertConfirmCommand;
struct DMTLAlertConfirmCommand_ {
	DMTLDisplayParams itsParams;
	bool itsDefault;
};

typedef struct DMTLAlertInputCommand_ DMTLAlertInputCommand;
struct DMTLAlertInputCommand_ {
	DMTLDisplayParams itsParams;
	char itsType;
	char itsEchoType;
	uint32_t itsMaxLen;
	char *itsDefaultText;
};

typedef struct DMTLAlertChoices_ DMTLAlertChoices;
struct DMTLAlertChoices_ {
	char *itsChoice;
	bool itsDefault;
};

typedef struct DMTLAlertChoiceCommand_ DMTLAlertChoiceCommand;
struct DMTLAlertChoiceCommand_ {
	DMTLDisplayParams itsParams;
	dmc_ptr_array itsChoices;
};

typedef struct DMTLSleepCommand_ DMTLSleepCommand;
struct DMTLSleepCommand_ {
	uint32_t itsSeconds;
};

typedef struct DMTLCommand_ DMTLCommand;
struct DMTLCommand_ {
	DMTLCommandType itsType;
	union {
		DMTLAddCommand itsAddData;
		DMTLCopyCommand itsCopyData;
		DMTLDeleteCommand itsDeleteData;
		DMTLExecCommand itsExecData;
		DMTLGetCommand itsGetData;
		DMTLReplaceCommand itsReplaceData;
		dmc_ptr_array itsCommands;
		DMTLAlertDisplayCommand itsAlertDisplayData;
		DMTLAlertConfirmCommand itsAlertConfirmData;
		DMTLAlertInputCommand itsAlertInputData;
		DMTLAlertChoiceCommand itsAlertChoiceData;
		DMTLSleepCommand itsSleepData;
	};
};

typedef struct DMTLGetResult_ DMTLGetResult;
struct DMTLGetResult_ {
	unsigned int itsDataSize;
	char *itsFormat;
	char *itsType;
	char *itsData;
};

DMTLCommand *prv_createCommand(DMTLCommandType iType);
void prv_freeCommand(DMTLCommand *iCommand);
void prv_DMTLNodeFree(DMTLNode *iNode);
void prv_DMTLDisplayParamsFree(DMTLDisplayParams *iDisplay);
DMTLGetResult *prv_makeGetResult();
void prv_freeGetResult(DMTLGetResult *iResult);

#endif
