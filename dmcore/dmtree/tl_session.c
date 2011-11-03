/*****************************************************************************
 * Copyright (c) 1999-2008 ACCESS CO., LTD. All rights reserved.
 * Copyright (c) 2007, ACCESS Systems Americas, Inc. All rights reserved.
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file parser_tl.c
 *
 * @brief Parser for the DMClient test language
 *
 ******************************************************************************/

#include "config.h"

#include "error.h"
#include "error_macros.h"
#include "log.h"

#include <string.h>

/* TODO.  Currently we do not check the xmlLastError.code to find out why
   libxml2 failed.  We are just returning all errors as DMC_ERR_CORRUPT
   which is not correct. 
   
   can use xmlreadnameconst so we don't need to constantly deallocate the 
   node name.

*/

#include <libxml/xmlreader.h>
#include <libxml/xmlerror.h>

#include "tl_session.h"

static omadm_tl_session *prv_createomadm_tl_session(char *iSessionID)
{
	omadm_tl_session *retVal = malloc(sizeof(*retVal));

	if (retVal) {
		memset(retVal, 0, sizeof(*retVal));
		retVal->server_id = strdup(iSessionID);
		if (!retVal->server_id) {
			free(retVal);
			retVal = NULL;
		} else
			dmc_ptr_array_make(&retVal->packages, 8,
						 dmc_ptr_array_free_callback);
	}

	return retVal;
}

static void prv_free_command_callback(void *iCommand)
{
	prv_freeCommand((DMTLCommand *) iCommand);
}

static int prv_expect_current_node(xmlTextReaderPtr iReaderPtr,
				   const char *iNodeName, bool iStart)
{
	DMC_ERR_MANAGE;
	xmlChar * name = NULL;
	xmlReaderTypes type = XML_READER_TYPE_ELEMENT;

	if (!iStart)
		type = XML_READER_TYPE_END_ELEMENT;

	if (type != (xmlReaderTypes) xmlTextReaderNodeType(iReaderPtr)) {
		DMC_ERR = DMC_ERR_CORRUPT;
		goto cleanup;
	}

	DMC_FAIL_NULL_LABEL(name, xmlTextReaderName(iReaderPtr),
			    DMC_ERR_OOM, cleanup);

	if (xmlStrcmp((const xmlChar *)iNodeName, name) != 0)
		DMC_ERR = DMC_ERR_CORRUPT;

cleanup:
	if (name)
		xmlFree(name);

	return DMC_ERR;
}

static int prv_expect_node(xmlTextReaderPtr iReaderPtr,
				   const char *iNodeName, bool iStart)
{
	DMC_ERR_MANAGE;
	DMC_ERR = DMC_ERR_CORRUPT;

	if (xmlTextReaderRead(iReaderPtr) == 1)
		DMC_ERR =
		    prv_expect_current_node(iReaderPtr, iNodeName, iStart);

	return DMC_ERR;
}

static char *prv_expect_current_data(xmlTextReaderPtr iReaderPtr,
				     const char *iNodeName)
{
	xmlChar *text = NULL;
	char *ctext = NULL;

	if (!xmlTextReaderIsEmptyElement(iReaderPtr)) {
		if (xmlTextReaderRead(iReaderPtr) != 1)
			goto error;

		if (xmlTextReaderNodeType(iReaderPtr) == XML_READER_TYPE_TEXT ||
		    xmlTextReaderNodeType(iReaderPtr) ==
		    XML_READER_TYPE_SIGNIFICANT_WHITESPACE) {
			text = xmlTextReaderReadString(iReaderPtr);
			if (!text)
				goto error;

			if (xmlTextReaderNodeType(iReaderPtr) ==
			    XML_READER_TYPE_SIGNIFICANT_WHITESPACE) {
				xmlFree(text);
				text = NULL;
			}

			if (prv_expect_node(iReaderPtr, iNodeName, false) !=
			    DMC_ERR_NONE)
				goto error;
		} else {
			if (prv_expect_current_node(iReaderPtr, iNodeName, false)
			    != DMC_ERR_NONE)
				goto error;
		}
	}

	if (!text) {
		ctext = strdup("");
		if (!ctext)
			goto error;
	} else {
		ctext = strdup((char *)text);
		if (!ctext)
			goto error;
		xmlFree(text);
		text = NULL;
	}

	return ctext;

error:

	if (text) {
		xmlFree(text);
		text = NULL;
	}

	return ctext;
}

static char *prv_expect_data(xmlTextReaderPtr iReaderPtr, const char *iNodeName)
{
	char *retVal = NULL;

	if (prv_expect_node(iReaderPtr, iNodeName, true) == DMC_ERR_NONE)
		retVal = prv_expect_current_data(iReaderPtr, iNodeName);

	return retVal;
}

static int prv_process_node(xmlTextReaderPtr iReaderPtr, DMTLNode *oNode)
{
	DMC_ERR_MANAGE;

	memset(oNode, 0, sizeof(*oNode));

	DMC_FAIL(prv_expect_node(iReaderPtr, "node", true));

	DMC_FAIL_NULL(oNode->itsName, prv_expect_data(iReaderPtr, "name"),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("Name: %s", oNode->itsName);

	DMC_FAIL_NULL(oNode->itsFormat, prv_expect_data(iReaderPtr, "format"),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("Format: %s", oNode->itsFormat);

	DMC_FAIL_NULL(oNode->itsType, prv_expect_data(iReaderPtr, "type"),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("Type: %s", oNode->itsType);

	DMC_FAIL_NULL(oNode->itsData, prv_expect_data(iReaderPtr, "data"),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("Data: %s", oNode->itsData);

	DMC_FAIL(prv_expect_node(iReaderPtr, "node", false));

	return DMC_ERR_NONE;

DMC_ON_ERR:

	prv_DMTLNodeFree(oNode);

	return DMC_ERR;
}

static int prv_process_display_params(xmlTextReaderPtr iReaderPtr,
				      DMTLDisplayParams * oParams)
{
	DMC_ERR_MANAGE;
	char *text = NULL;

	memset(oParams, 0, sizeof(DMTLDisplayParams));

	DMC_FAIL(prv_expect_node(iReaderPtr, "ui-params", true));

	DMC_FAIL_NULL(text, prv_expect_data(iReaderPtr, "min"),
		      DMC_ERR_CORRUPT);

	oParams->itsMinSecs = atoi(text);

	DMC_LOGF("Min: %d", oParams->itsMinSecs);

	free(text);

	DMC_FAIL_NULL(text, prv_expect_data(iReaderPtr, "max"),
		      DMC_ERR_CORRUPT);

	oParams->itsMaxSecs = atoi(text);

	DMC_LOGF("Max: %d", oParams->itsMaxSecs);

	free(text);
	text = NULL;

	DMC_FAIL_NULL(oParams->itsMessage, prv_expect_data(iReaderPtr, "text"),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("text: %s", oParams->itsMessage);

	DMC_FAIL(prv_expect_node(iReaderPtr, "ui-params", false));

	return DMC_ERR_NONE;

DMC_ON_ERR:
	
	prv_DMTLDisplayParamsFree(oParams);

	return DMC_ERR;

}

static int prv_process_add_command(xmlTextReaderPtr iReaderPtr,
				   DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	DMTLCommand *command = NULL;

	DMC_FAIL_NULL(command, prv_createCommand(OMADM_COMT_ADD),
		      DMC_ERR_OOM);

	DMC_LOG("Found Add Command");

	DMC_FAIL_NULL(command->itsAddData.itsURI,
		      prv_expect_data(iReaderPtr, "uri"),
		      DMC_ERR_CORRUPT);
	DMC_LOGF("URI: %s", command->itsAddData.itsURI);

	DMC_FAIL(prv_process_node(iReaderPtr, &command->itsAddData.itsNode));

	*oCommand = command;
	return DMC_ERR_NONE;

DMC_ON_ERR:

	if (command)
		prv_freeCommand(command);

	return DMC_ERR;
}

static int prv_process_alert_display(xmlTextReaderPtr iReaderPtr,
				     DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	DMTLCommand * command = NULL;

	DMC_FAIL_NULL(command, prv_createCommand(OMADM_COMT_ALERT_DISPLAY),
		      DMC_ERR_OOM);

	DMC_LOG("Found Alert Display Command");

	DMC_FAIL(prv_process_display_params(iReaderPtr,
						 &command->itsAlertDisplayData.
						 itsParams));

	*oCommand = command;
	return DMC_ERR_NONE;

DMC_ON_ERR:
	
	if (command)
		prv_freeCommand(command);

	return DMC_ERR;
}

static int prv_process_alert_confirm(xmlTextReaderPtr iReaderPtr,
				     DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	char *text = NULL;
	DMTLCommand *command = NULL;

	DMC_FAIL_NULL(command, prv_createCommand(OMADM_COMT_ALERT_CONFIRM),
		      DMC_ERR_OOM);

	DMC_LOG("Found Alert Confirm Command");

	DMC_FAIL(prv_process_display_params(iReaderPtr,
						 &command->itsAlertConfirmData.
						 itsParams));

	DMC_FAIL_NULL(text, prv_expect_data(iReaderPtr, "default"),
		      DMC_ERR_CORRUPT);

	if (strcmp("ok", text) == 0)
		command->itsAlertConfirmData.itsDefault = true;
	else if (strcmp("cancel", text) == 0)
		command->itsAlertConfirmData.itsDefault = false;
	else {
		DMC_ERR = DMC_ERR_CORRUPT;
		goto DMC_ON_ERR;
	}

	free(text);
	text = NULL;

	*oCommand = command;
	return DMC_ERR_NONE;

DMC_ON_ERR:
	
	if (text)
		free(text);

	if (command)
		prv_freeCommand(command);

	return DMC_ERR;
}

static int prv_process_alert_input(xmlTextReaderPtr iReaderPtr,
				   DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	char *text = NULL;
	DMTLCommand *command = NULL;

	DMC_FAIL_NULL(command, prv_createCommand(OMADM_COMT_ALERT_INPUT),
		      DMC_ERR_OOM);

	DMC_LOG("Found Alert Input Command");

	DMC_FAIL(prv_process_display_params(iReaderPtr,
						 &command->itsAlertInputData.
						 itsParams));

	DMC_FAIL_NULL(text, prv_expect_data(iReaderPtr, "type"),
		      DMC_ERR_CORRUPT);
	if (strlen(text) == 0) {
		DMC_ERR = DMC_ERR_CORRUPT;
		goto DMC_ON_ERR;
	}

	command->itsAlertInputData.itsType = text[0];

	DMC_LOGF("Type: %c", command->itsAlertInputData.itsType);

	free(text);

	DMC_FAIL_NULL(text, prv_expect_data(iReaderPtr, "echo-type"),
		      DMC_ERR_CORRUPT);
	if (strlen(text) == 0) {
		DMC_ERR = DMC_ERR_CORRUPT;
		goto DMC_ON_ERR;
	}

	command->itsAlertInputData.itsEchoType = text[0];

	DMC_LOGF("EchoType: %c", command->itsAlertInputData.itsEchoType);

	free(text);

	DMC_FAIL_NULL(text, prv_expect_data(iReaderPtr, "maxlen"),
		      DMC_ERR_CORRUPT);

	command->itsAlertInputData.itsMaxLen = atoi(text);

	DMC_LOGF("MaxLen: %d", command->itsAlertInputData.itsMaxLen);

	free(text);
	text = NULL;

	*oCommand = command;
	return DMC_ERR_NONE;

DMC_ON_ERR:

	if (text)
		free(text);

	if (command)
		prv_freeCommand(command);

	return DMC_ERR;
}

static void prv_free_choice(void *iChoice)
{
	DMTLAlertChoices *choice = (DMTLAlertChoices *) iChoice;
	free(choice->itsChoice);
	free(choice);
}

static int prv_process_alert_choice(xmlTextReaderPtr iReaderPtr,
				    DMTLCommandType iCommandType,
				    DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	char *text = NULL;
	DMTLCommand *command = NULL;
	DMTLAlertChoices *choice = NULL;

	DMC_FAIL_NULL(command, prv_createCommand(iCommandType),
			   DMC_ERR_OOM);

	DMC_LOGF("Found Alert Choice Command %d", iCommandType);

	DMC_FAIL(prv_process_display_params(iReaderPtr,
						 &command->itsAlertChoiceData.
						 itsParams));

	DMC_FAIL(prv_expect_node(iReaderPtr, "list", true));

	if (xmlTextReaderRead(iReaderPtr) != 1) {
		DMC_ERR = DMC_ERR_CORRUPT;
		goto DMC_ON_ERR;
	}

	dmc_ptr_array_make(&command->itsAlertChoiceData.itsChoices, 8,
				 prv_free_choice);

	while (prv_expect_current_node(iReaderPtr, "list", false) !=
	       DMC_ERR_NONE) {
		DMC_FAIL_NULL(text, prv_expect_current_data(iReaderPtr, "text"),
			      DMC_ERR_CORRUPT);

		DMC_FAIL_NULL(choice,
			      (DMTLAlertChoices *)
			      malloc(sizeof(DMTLAlertChoices)),
			      DMC_ERR_OOM);
		choice->itsChoice = text;
		choice->itsDefault = false;

		DMC_FAIL(dmc_ptr_array_append
			 (&command->itsAlertChoiceData.itsChoices, choice));
		choice = NULL;
		text = NULL;

		if (xmlTextReaderRead(iReaderPtr) != 1) {
			DMC_ERR = DMC_ERR_CORRUPT;
			goto DMC_ON_ERR;
		}
	}

	*oCommand = command;
	return DMC_ERR_NONE;

DMC_ON_ERR:

	if (choice)
		free(choice);

	if (text)
		free(text);

	if (command)
		prv_freeCommand(command);

	return DMC_ERR;
}

static int prv_process_copy_command(xmlTextReaderPtr iReaderPtr,
				    DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	DMTLCommand *command = NULL;

	DMC_FAIL_NULL(command, prv_createCommand(OMADM_COMT_COPY),
		      DMC_ERR_OOM);

	DMC_LOG("Found Copy Command");

	DMC_FAIL_NULL(command->itsCopyData.itsSourceURI,
		      prv_expect_data(iReaderPtr, "uri"),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("Source URI: %s",	command->itsCopyData.itsSourceURI);

	DMC_FAIL_NULL(command->itsCopyData.itsTargetURI,
		      prv_expect_data(iReaderPtr, "uri"),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("Target URI: %s",	command->itsCopyData.itsTargetURI);

	*oCommand = command;
	return DMC_ERR_NONE;

DMC_ON_ERR:
	
	if (command)
		prv_freeCommand(command);

	return DMC_ERR;
}

static int prv_process_delete_command(xmlTextReaderPtr iReaderPtr,
				      DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	DMTLCommand * command = NULL;

	DMC_FAIL_NULL(command, prv_createCommand(OMADM_COMT_DELETE),
		      DMC_ERR_OOM);

	DMC_LOG("Found Delete Command");

	DMC_FAIL_NULL(command->itsDeleteData.itsURI,
		      prv_expect_data(iReaderPtr, "uri"),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("Source URI: %s",	command->itsDeleteData.itsURI);

	*oCommand = command;
	return DMC_ERR_NONE;

DMC_ON_ERR:
	
	if (command)
		prv_freeCommand(command);

	return DMC_ERR;
}

static int prv_process_exec_command(xmlTextReaderPtr iReaderPtr,
				    DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	DMTLCommand * command = NULL;

	DMC_FAIL_NULL(command, prv_createCommand(OMADM_COMT_EXEC),
		      DMC_ERR_OOM);

	DMC_LOG("Found Exec Command");

	DMC_FAIL_NULL(command->itsExecData.itsURI,
		      prv_expect_data(iReaderPtr, "uri"),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("URI: %s", command->itsExecData.itsURI);

	DMC_FAIL_NULL(command->itsExecData.itsData,
		      prv_expect_data(iReaderPtr, "data"),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("Data: %s", command->itsExecData.itsData);

	*oCommand = command;
	return DMC_ERR_NONE;

DMC_ON_ERR:

	if (command)
		prv_freeCommand(command);

	return DMC_ERR;
}

static int prv_process_get_command(xmlTextReaderPtr iReaderPtr,
					  DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	DMTLCommand *command = NULL;

	DMC_FAIL_NULL(command, prv_createCommand(OMADM_COMT_GET),
		      DMC_ERR_OOM);

	DMC_LOG("Found Get Command");

	DMC_FAIL_NULL(command->itsGetData.itsURI,
		      prv_expect_data(iReaderPtr, "uri"),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("URI: %s", command->itsGetData.itsURI);

	*oCommand = command;
	return DMC_ERR_NONE;

DMC_ON_ERR:

	if (command)
		prv_freeCommand(command);

	return DMC_ERR;
}

static int prv_process_replace_command(xmlTextReaderPtr iReaderPtr,
				       DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	DMTLCommand * command = NULL;

	DMC_FAIL_NULL(command, prv_createCommand(OMADM_COMT_REPLACE),
		      DMC_ERR_OOM);

	DMC_LOG("Found Replace Command");

	DMC_FAIL_NULL(command->itsReplaceData.itsURI,
		      prv_expect_data(iReaderPtr, "uri"),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("URI: %s", command->itsReplaceData.itsURI);

	DMC_FAIL(prv_process_node(iReaderPtr, &command->itsReplaceData.itsNode));

	*oCommand = command;
	return DMC_ERR_NONE;

DMC_ON_ERR:
	
	if (command)
		prv_freeCommand(command);

	return DMC_ERR;
}

static int prv_process_sleep_command(xmlTextReaderPtr iReaderPtr,
				     DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	DMTLCommand * command = NULL;
	char *text = NULL;

	DMC_FAIL_NULL(command, prv_createCommand(OMADM_COMT_SLEEP),
		      DMC_ERR_OOM);

	DMC_LOG("Found Sleep Command");

	DMC_FAIL_NULL(text, prv_expect_data(iReaderPtr, "seconds"),
		      DMC_ERR_CORRUPT);

	command->itsSleepData.itsSeconds = atoi(text);

	free(text);
	text = NULL;

	DMC_LOGF("Seconds: %d", command->itsSleepData.itsSeconds);

	*oCommand = command;
	return DMC_ERR_NONE;

DMC_ON_ERR:
	
	if (command)
		prv_freeCommand(command);

	return DMC_ERR;
}

static int prv_complete_parameterless_command(const xmlChar* iNodeName, 
					     xmlTextReaderPtr iReaderPtr)
{
	DMC_ERR_MANAGE;

	int xmlerr;
	const xmlChar* name;

	xmlerr = xmlTextReaderIsEmptyElement(iReaderPtr);
	if (xmlerr == -1)
		DMC_FAIL_FORCE(DMC_ERR_CORRUPT);

	if (xmlerr == 0)
	{
		if (xmlTextReaderRead(iReaderPtr) != 1)		
			DMC_FAIL_FORCE(DMC_ERR_CORRUPT);

		if (xmlTextReaderNodeType(iReaderPtr) == XML_READER_TYPE_TEXT ||
		    xmlTextReaderNodeType(iReaderPtr) ==
		    XML_READER_TYPE_SIGNIFICANT_WHITESPACE)
			if (xmlTextReaderRead(iReaderPtr) != 1)		
				DMC_FAIL_FORCE(DMC_ERR_CORRUPT);

		DMC_FAIL_NULL(name, xmlTextReaderConstName(iReaderPtr),
				   DMC_ERR_OOM);

		if (xmlStrcmp((const xmlChar *)iNodeName, name) != 0)
			DMC_ERR = DMC_ERR_CORRUPT;

	}

DMC_ON_ERR:

	return DMC_ERR;
}

static int prv_process_device_details_command(xmlTextReaderPtr iReaderPtr,
					      DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	DMTLCommand * command = NULL;

	DMC_FAIL_NULL(command, prv_createCommand(OMADM_COMT_DEVICE_INFO),
		      DMC_ERR_OOM);

	DMC_LOG("Found Device Details Command");


	*oCommand = command;

DMC_ON_ERR:
	
	return DMC_ERR;
}


static int prv_process_single_command(const xmlChar * iNodeName,
				      xmlTextReaderPtr iReaderPtr,
				      DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	DMC_ERR = DMC_ERR_CORRUPT;

	if (xmlStrcmp((const xmlChar *)"deviceinfo", iNodeName) == 0)
	{
		DMC_FAIL(prv_process_device_details_command(iReaderPtr, oCommand));
		DMC_ERR = prv_complete_parameterless_command(iNodeName, iReaderPtr);
	}
	else
	{
		if (xmlStrcmp((const xmlChar *)"add", iNodeName) == 0)
			DMC_ERR = prv_process_add_command(iReaderPtr, oCommand);
		else if (xmlStrcmp((const xmlChar *)"copy", iNodeName) == 0)
			DMC_ERR = prv_process_copy_command(iReaderPtr, oCommand);
		else if (xmlStrcmp((const xmlChar *)"confirm", iNodeName) == 0)
			DMC_ERR = prv_process_alert_confirm(iReaderPtr, oCommand);
		else if (xmlStrcmp((const xmlChar *)"delete", iNodeName) == 0)
			DMC_ERR = prv_process_delete_command(iReaderPtr, oCommand);
		else if (xmlStrcmp((const xmlChar *)"display", iNodeName) == 0)
			DMC_ERR = prv_process_alert_display(iReaderPtr, oCommand);
		else if (xmlStrcmp((const xmlChar *)"exec", iNodeName) == 0)
			DMC_ERR = prv_process_exec_command(iReaderPtr, oCommand);
		else if (xmlStrcmp((const xmlChar *)"get", iNodeName) == 0)
			DMC_ERR = prv_process_get_command(iReaderPtr, oCommand);
		else if (xmlStrcmp((const xmlChar *)"input", iNodeName) == 0)
			DMC_ERR = prv_process_alert_input(iReaderPtr, oCommand);
		else if (xmlStrcmp((const xmlChar *)"multi-choice", iNodeName) == 0)
			DMC_ERR =
				prv_process_alert_choice(iReaderPtr,
							 OMADM_COMT_ALERT_MULTIPLE,
							 oCommand);
		else if (xmlStrcmp((const xmlChar *)"replace", iNodeName) == 0)
			DMC_ERR = prv_process_replace_command(iReaderPtr, oCommand);
		else if (xmlStrcmp((const xmlChar *)"single-choice", iNodeName) == 0)
			DMC_ERR =
				prv_process_alert_choice(iReaderPtr,
							 OMADM_COMT_ALERT_SINGLE,
							 oCommand);
		else if (xmlStrcmp((const xmlChar *)"sleep", iNodeName) == 0)
			DMC_ERR = prv_process_sleep_command(iReaderPtr, oCommand);
		
		if (DMC_ERR == DMC_ERR_NONE)
			DMC_ERR =
				prv_expect_node(iReaderPtr, (const char *)iNodeName, false);
	}

DMC_ON_ERR:

	return DMC_ERR;
}

static int prv_process_compound_command(xmlTextReaderPtr iReaderPtr,
					DMTLCommandType iCommandType,
					const char *iCommandName,
					DMTLCommand **oCommand)
{
	DMC_ERR_MANAGE;
	DMTLCommand *compoundCommand = NULL;
	DMTLCommand *command = NULL;
	xmlChar *name = NULL;

	DMC_FAIL_NULL(compoundCommand, prv_createCommand(iCommandType),
		      DMC_ERR_OOM);

	dmc_ptr_array_make(&compoundCommand->itsCommands, 8,
				    prv_free_command_callback);

	while (prv_expect_node(iReaderPtr, iCommandName, false) != DMC_ERR_NONE) {
		DMC_FAIL_NULL(name, xmlTextReaderName(iReaderPtr),
			      DMC_ERR_CORRUPT);

		/* Special case.  Gets are not allowed in Atomics */

		if (iCommandType == OMADM_COMT_ATOMIC
		    && xmlStrcmp(name, (const xmlChar *)"get") == 0) {
			DMC_ERR = DMC_ERR_CORRUPT;
			goto DMC_ON_ERR;
		}

		DMC_FAIL(prv_process_single_command(name, iReaderPtr, &command));

		xmlFree(name);
		name = NULL;

		DMC_FAIL(dmc_ptr_array_append
			 (&compoundCommand->itsCommands, command));

		command = NULL;
	}

	*oCommand = compoundCommand;

	return DMC_ERR_NONE;

DMC_ON_ERR: 
	
	if (command)
		prv_freeCommand(command);

	if (name)
		xmlFree(name);

	if (compoundCommand)
		prv_freeCommand(compoundCommand);

	return DMC_ERR;
}

static int prv_process_atomic_command(xmlTextReaderPtr iReaderPtr,
				      DMTLCommand **oCommand)
{
	DMC_LOG("Found Atomic Command");

	return prv_process_compound_command(iReaderPtr, OMADM_COMT_ATOMIC,
					    "atomic", oCommand);
}

static int prv_process_sequence_command(xmlTextReaderPtr iReaderPtr,
					DMTLCommand **oCommand)
{
	DMC_LOG("Found Sequence Command");

	return prv_process_compound_command(iReaderPtr, OMADM_COMT_SEQUENCE,
					    "sequence", oCommand);;
}

static int prv_process_command(xmlTextReaderPtr iReaderPtr,
			       dmc_ptr_array *iPackage)
{
	DMC_ERR_MANAGE;
	xmlChar * name = NULL;
	DMTLCommand *command = NULL;

	DMC_FAIL_NULL(name, xmlTextReaderName(iReaderPtr),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("Command name %s", name);

	if (xmlStrcmp((const xmlChar *) "atomic", name) == 0)
		DMC_ERR = prv_process_atomic_command(iReaderPtr, &command);
	else if (xmlStrcmp((const xmlChar *) "sequence", name) == 0)
		DMC_ERR = prv_process_sequence_command(iReaderPtr, &command);
	else
		DMC_ERR =
		    prv_process_single_command(name, iReaderPtr, &command);

	if (DMC_ERR != DMC_ERR_NONE)
		goto DMC_ON_ERR;

	DMC_FAIL(dmc_ptr_array_append(iPackage, command));

	command = NULL;

DMC_ON_ERR:
	
	if (name)
		xmlFree(name);
	
	if (command)
		prv_freeCommand(command);

	return DMC_ERR;
}

static int prv_process_package(xmlTextReaderPtr iReaderPtr,
			       omadm_tl_session *iSession)
{
	DMC_ERR_MANAGE;
	int xmlErr = 0;
	dmc_ptr_array *package = NULL;

	DMC_FAIL(prv_expect_current_node(iReaderPtr, "package", true));

	DMC_LOG("Found package node");

	DMC_FAIL_NULL(package,
		      (dmc_ptr_array *) malloc(sizeof(dmc_ptr_array)),
		      DMC_ERR_OOM);

	dmc_ptr_array_make(package, 8, prv_free_command_callback);

	do {
		xmlErr = xmlTextReaderRead(iReaderPtr);

		if (xmlErr == 1)
			DMC_ERR = prv_process_command(iReaderPtr, package);
		else if (xmlErr == -1)
			DMC_ERR = DMC_ERR_CORRUPT;
	} while (xmlErr == 1 && DMC_ERR == DMC_ERR_NONE);

	if (xmlErr != 1) {
		DMC_ERR = DMC_ERR_CORRUPT;
		goto DMC_ON_ERR;
	}

	DMC_FAIL(dmc_ptr_array_append(&iSession->packages, package));

	package = NULL;

	DMC_FAIL(prv_expect_current_node(iReaderPtr, "package", false));

	return DMC_ERR_NONE;

DMC_ON_ERR:

	if (package) {
		dmc_ptr_array_free(package);
		free(package);
	}

	return DMC_ERR;
}

static int prv_process_session(xmlTextReaderPtr iReaderPtr,
			       omadm_tl_session **oSession)
{
	DMC_ERR_MANAGE;
	char *serverID = NULL;
	int xmlErr = 0;
	omadm_tl_session *session = NULL;

	DMC_FAIL(prv_expect_node(iReaderPtr, "dmtl", true));

	DMC_LOG("Found root node");

	DMC_FAIL_NULL(serverID, prv_expect_data(iReaderPtr, "server-id"),
		      DMC_ERR_CORRUPT);

	DMC_LOGF("ServerID = %s", serverID);

	DMC_FAIL_NULL(session, prv_createomadm_tl_session(serverID),
		      DMC_ERR_OOM);

	do {
		xmlErr = xmlTextReaderRead(iReaderPtr);
		if (xmlErr == 1)
			DMC_ERR = prv_process_package(iReaderPtr, session);
		else if (xmlErr == -1)
			DMC_ERR = DMC_ERR_CORRUPT;
	} while (xmlErr == 1 && DMC_ERR == DMC_ERR_NONE);

	if (xmlErr != 1) {
		DMC_ERR = DMC_ERR_CORRUPT;
		goto DMC_ON_ERR;
	}

	DMC_FAIL(prv_expect_current_node(iReaderPtr, "dmtl", false));

	*oSession = session;
	session = NULL;

DMC_ON_ERR:

	if (session)
		omadm_tl_session_free(session);

	if (serverID)
		free(serverID);

	return DMC_ERR;
}

#ifdef DMC_LOGGING 

static void prv_trace_node(DMTLNode *iNode)
{
	DMC_LOGUF("  Node Name: %s", iNode->itsName);
	DMC_LOGUF("  Node Format: %s", iNode->itsFormat);
	DMC_LOGUF("  Node Type: %s", iNode->itsType);
	DMC_LOGUF("  Node Data: %s", iNode->itsData);
}

static void prv_trace_display_params(DMTLDisplayParams *iParams)
{
	DMC_LOGUF("  Min: %d", iParams->itsMinSecs);
	DMC_LOGUF("  Max: %d", iParams->itsMaxSecs);
	DMC_LOGUF("  Text: %s", iParams->itsMessage);
}

static void prv_trace_commands(dmc_ptr_array *iCommands)
{
	DMTLCommand *command = NULL;
	unsigned int i = 0;
	unsigned int j = 0;

	for (i = 0; i < dmc_ptr_array_get_size(iCommands); ++i) {
		command = (DMTLCommand *) dmc_ptr_array_get(iCommands, i);
		switch (command->itsType) {
		case OMADM_COMT_ADD:
			DMC_LOGU("ADD Command");
			DMC_LOGUF("URI: %s", command->itsAddData.itsURI);
			prv_trace_node(&command->itsAddData.itsNode);
			break;
		case OMADM_COMT_ALERT_DISPLAY:
			DMC_LOGU("ALERT DISPLAY Command");
			prv_trace_display_params(&command->itsAlertDisplayData.
						 itsParams);
			break;
		case OMADM_COMT_ALERT_CONFIRM:
			DMC_LOGU("ALERT CONFIRM Command");
			prv_trace_display_params(&command->itsAlertConfirmData.
						 itsParams);
			DMC_LOGUF("Default: %d", 
				      command->itsAlertConfirmData.itsDefault);
			break;
		case OMADM_COMT_ALERT_INPUT:
			DMC_LOGU("ALERT INPUT Command");
			prv_trace_display_params(&command->itsAlertInputData.
						 itsParams);
			DMC_LOGUF("Type: %c", 
				      command->itsAlertInputData.itsType);
			DMC_LOGUF("EchoType: %c", 
				      command->itsAlertInputData.itsEchoType);
			DMC_LOGUF("MaxLen: %d", 
				      command->itsAlertInputData.itsMaxLen);
			break;
		case OMADM_COMT_ALERT_SINGLE:
		case OMADM_COMT_ALERT_MULTIPLE:
			DMC_LOGUF("Choice Command %d", command->itsType);
			prv_trace_display_params(&command->itsAlertChoiceData.
						 itsParams);
			DMC_LOGU("Choices:");
			for (j = 0;
			     j <
			     dmc_ptr_array_get_size(&command->
							  itsAlertChoiceData.
							  itsChoices); ++j) {
				DMC_LOGUF("%s",
					dmc_ptr_array_get(&command->
								itsAlertChoiceData.
								itsChoices,
								j));
			}
			break;
		case OMADM_COMT_ATOMIC:
			DMC_LOGU("START ATOMIC");
			prv_trace_commands(&command->itsCommands);
			DMC_LOGU("END ATOMIC");
			break;
		case OMADM_COMT_COPY:
			DMC_LOGU("COPY Command");
			DMC_LOGUF("SOURCE URI: %s",	command->itsCopyData.itsSourceURI);
			DMC_LOGUF("TARGET URI: %s",	command->itsCopyData.itsTargetURI);
			break;
		case OMADM_COMT_DELETE:
			DMC_LOGU("DELETE Command");
			DMC_LOGUF("URI: %s", command->itsDeleteData.itsURI);
			break;
		case OMADM_COMT_EXEC:
			DMC_LOGU("EXEC Command");
			DMC_LOGUF("URI: %s", command->itsExecData.itsURI);
			DMC_LOGUF("DATA: %s", command->itsExecData.itsData);
			break;
		case OMADM_COMT_GET:
			DMC_LOGU("GET Command");
			DMC_LOGUF("URI: %s", command->itsGetData.itsURI);
			break;
		case OMADM_COMT_REPLACE:
			DMC_LOGU("REPLACE Command");
			DMC_LOGUF("URI: %s", command->itsReplaceData.itsURI);
			prv_trace_node(&command->itsReplaceData.itsNode);
			break;
		case OMADM_COMT_SEQUENCE:
			DMC_LOGU("START SEQUENCE");
			prv_trace_commands(&command->itsCommands);
			DMC_LOGU("END SEQUENCE");
			break;
		case OMADM_COMT_DEVICE_INFO:
			DMC_LOGU("DEVICE INFO");
			break;
		default:
			break;
		}

		DMC_LOGU("");
	}
}

void omadm_tl_session_trace(omadm_tl_session *iSession)
{
	unsigned int i = 0;
	dmc_ptr_array *pckg = NULL;

	DMC_LOGUF("ServerID %s", iSession->server_id);

	for (i = 0; i < dmc_ptr_array_get_size(&iSession->packages);
	     ++i) {
		DMC_LOGUF("Pkcg: %d", (i << 1) + 1);

		pckg =
		    (dmc_ptr_array *) dmc_ptr_array_get(&iSession->
								  packages,
								  i);
		prv_trace_commands(pckg);
		DMC_LOGU("");
	}
}

#endif

int omadm_tl_session_parse(const char *iFileName, omadm_tl_session **oSession)
{
	int retVal = DMC_ERR_CORRUPT;
	xmlTextReaderPtr readerPtr = 0;

	DMC_LOGF("Parsing test language file %s", iFileName);

	readerPtr = xmlReaderForFile(iFileName, NULL,
				     XML_PARSE_NOENT | XML_PARSE_NOBLANKS |
				     XML_PARSE_NOCDATA);
	if (readerPtr) {
		retVal = prv_process_session(readerPtr, oSession);

		xmlFreeTextReader(readerPtr);
	}

	DMC_LOGF("Parsing test language err = %x", retVal);

	return retVal;
}

void omadm_tl_session_free(omadm_tl_session *iSession)
{
	if (iSession->server_id) {
		free(iSession->server_id);
		iSession->server_id = NULL;
	}
	dmc_ptr_array_free(&iSession->packages);
	free(iSession);
}
