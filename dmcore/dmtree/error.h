/******************************************************************************
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file errors.h
 *
 * @brief
 * Device Management error codes
 *
 ******************************************************************************/

#ifndef DMC_ERRORS_H
#define DMC_ERRORS_H

enum dmc_generic_errors
{
	DMC_ERR_NONE = 0,
	DMC_ERR_UNKNOWN,
	DMC_ERR_OOM,
	DMC_ERR_CORRUPT,
	DMC_ERR_OPEN,
	DMC_ERR_READ,
	DMC_ERR_WRITE,
	DMC_ERR_IO,
	DMC_ERR_NOT_FOUND,
	DMC_ERR_ALREADY_EXISTS,
	DMC_ERR_NOT_SUPPORTED,
	DMC_ERR_CANCELLED,
	DMC_ERR_TRANSACTION_IN_PROGRESS,
	DMC_ERR_NOT_IN_TRANSACTION,
	DMC_ERR_DENIED,
	DMC_ERR_BAD_ARGS,
	DMC_ERR_TIMEOUT,
	DMC_ERR_BAD_KEY,
	DMC_ERR_SUBSYSTEM
};

#endif
