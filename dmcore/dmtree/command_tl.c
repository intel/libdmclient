/*****************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007, ACCESS Systems Americas, Inc. All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file command_tl.c
 *
 * @brief Functions for create and deleting dm commands
 *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "error.h"

#include "config.h"
#include "command_tl.h"

void prv_DMTLNodeFree(DMTLNode *iNode)
{
	if (iNode->itsName) {
		free(iNode->itsName);
		iNode->itsName = NULL;
	}

	if (iNode->itsFormat) {
		free(iNode->itsFormat);
		iNode->itsFormat = NULL;
	}

	if (iNode->itsType) {
		free(iNode->itsType);
		iNode->itsType = NULL;
	}

	if (iNode->itsData) {
		free(iNode->itsData);
		iNode->itsData = NULL;
	}
}

void prv_DMTLDisplayParamsFree(DMTLDisplayParams *iDisplay)
{
	if (iDisplay->itsMessage) {
		free(iDisplay->itsMessage);
		iDisplay->itsMessage = NULL;
	}
}

DMTLCommand *prv_createCommand(DMTLCommandType iType)
{
	DMTLCommand *retVal = malloc(sizeof(*retVal));

	if (retVal) {
		memset(retVal, 0, sizeof(*retVal));
		retVal->itsType = iType;
	}

	return retVal;
}

/* Assumes commands are always heap allocated. */

void prv_freeCommand(DMTLCommand *iCommand)
{
	switch (iCommand->itsType) {
	case OMADM_COMT_ADD:
		free(iCommand->itsAddData.itsURI);
		iCommand->itsAddData.itsURI = NULL;
		prv_DMTLNodeFree(&iCommand->itsAddData.itsNode);
		break;
	case OMADM_COMT_ALERT_DISPLAY:
		prv_DMTLDisplayParamsFree(&iCommand->itsAlertDisplayData.
					  itsParams);
		break;
	case OMADM_COMT_ALERT_CONFIRM:
		prv_DMTLDisplayParamsFree(&iCommand->itsAlertConfirmData.
					  itsParams);
		break;
	case OMADM_COMT_ALERT_INPUT:
		prv_DMTLDisplayParamsFree(&iCommand->itsAlertInputData.
					  itsParams);
		if (iCommand->itsAlertInputData.itsDefaultText) {
			free(iCommand->itsAlertInputData.itsDefaultText);
			iCommand->itsAlertInputData.itsDefaultText = NULL;
		}
		break;
	case OMADM_COMT_ALERT_SINGLE:
	case OMADM_COMT_ALERT_MULTIPLE:
		prv_DMTLDisplayParamsFree(&iCommand->itsAlertChoiceData.
					  itsParams);
		dmc_ptr_array_free(&iCommand->itsAlertChoiceData.
					    itsChoices);
		break;
	case OMADM_COMT_COPY:
		free(iCommand->itsCopyData.itsSourceURI);
		iCommand->itsCopyData.itsSourceURI = NULL;
		free(iCommand->itsCopyData.itsTargetURI);
		iCommand->itsCopyData.itsTargetURI = NULL;
		break;
	case OMADM_COMT_DELETE:
		free(iCommand->itsDeleteData.itsURI);
		iCommand->itsDeleteData.itsURI = NULL;
		break;
	case OMADM_COMT_EXEC:
		free(iCommand->itsExecData.itsURI);
		iCommand->itsExecData.itsURI = NULL;
		free(iCommand->itsExecData.itsData);
		iCommand->itsExecData.itsData = NULL;
		break;
	case OMADM_COMT_GET:
		free(iCommand->itsGetData.itsURI);
		iCommand->itsGetData.itsURI = NULL;
		break;
	case OMADM_COMT_REPLACE:
		free(iCommand->itsReplaceData.itsURI);
		iCommand->itsReplaceData.itsURI = NULL;
		prv_DMTLNodeFree(&iCommand->itsReplaceData.itsNode);
		break;
	case OMADM_COMT_SEQUENCE:
	case OMADM_COMT_ATOMIC:
		dmc_ptr_array_free(&iCommand->itsCommands);
		break;
	default:
		break;
	}

	free(iCommand);
}

DMTLGetResult *prv_makeGetResult()
{
	DMTLGetResult *retVal = malloc(sizeof(*retVal));

	if (retVal)
		memset(retVal, 0, sizeof(*retVal));

	return retVal;
}

void prv_freeGetResult(DMTLGetResult *iResult)
{
	if (iResult->itsFormat) {
		free(iResult->itsFormat);
		iResult->itsFormat = NULL;
	}

	if (iResult->itsType) {
		free(iResult->itsType);
		iResult->itsType = NULL;
	}

	if (iResult->itsData) {
		free(iResult->itsData);
		iResult->itsData = NULL;
	}

	iResult->itsDataSize = 0;
}
