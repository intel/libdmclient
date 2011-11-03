/******************************************************************************
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file <syncml_error.c>
 *
 * @brief Source file for handling syncml errors
 *
 *****************************************************************************/

#include "config.h"

#include "error.h"

#include "syncml_error.h"

int syncml_from_dmc_err(int dmc_err)
{
	int retval;
 
	switch (dmc_err) {
	case DMC_ERR_NONE:
		retval = OMADM_SYNCML_ERROR_NONE;
		break;
	case DMC_ERR_OOM:
		retval = OMADM_SYNCML_ERROR_DEVICE_FULL;
		break;
	case DMC_ERR_NOT_FOUND:
		retval = OMADM_SYNCML_ERROR_NOT_FOUND;
		break;
	case DMC_ERR_ALREADY_EXISTS:
		retval = OMADM_SYNCML_ERROR_ALREADY_EXISTS;
		break;	       
	case DMC_ERR_NOT_SUPPORTED:
		retval = OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED;
		break;
	case DMC_ERR_CANCELLED:
		retval = OMADM_SYNCML_ERROR_NOT_EXECUTED;
		break;
	default:
		retval = OMADM_SYNCML_ERROR_COMMAND_FAILED;
		break;
	}

	return retval;
}
