/******************************************************************************
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

/*!
 * @file log.c
 *
 * @brief
 * Macros and functions for logging
 *
 ******************************************************************************/

#include <stdarg.h>
#include <stdio.h>

#include "config.h"
#include "syncml_error.h"
#include "log.h"

#ifdef DMC_LOGGING
static FILE *g_log_file;
#endif

int dmc_log_open(const char *log_file_name)
{
	int ret_val = OMADM_SYNCML_ERROR_NONE;

#ifdef DMC_LOGGING
	if (!g_log_file)
	{
		g_log_file = fopen(log_file_name, "w");

		if (!g_log_file)
			ret_val = OMADM_SYNCML_ERROR_SESSION_INTERNAL;
	}
#endif

	return ret_val;
}

void dmc_log_close()
{
#ifdef DMC_LOGGING
	if (g_log_file)
		fclose(g_log_file);
#endif
}

#ifdef DMC_LOGGING

void dmc_log_printf(unsigned int line_number, const char *file_name,
				const char *message, ...)
{
	va_list args;

	if (g_log_file) {
		va_start(args, message);
		fprintf(g_log_file, "%s:%u ",file_name, line_number);
		vfprintf(g_log_file, message, args);
		fprintf(g_log_file, "\n");
		va_end(args);
		fflush(g_log_file);
	}
}

void dmc_logu_printf(const char *message, ...)
{
	va_list args;

	if (g_log_file) {
		va_start(args, message);
		vfprintf(g_log_file, message, args);
		fprintf(g_log_file, "\n");
		va_end(args);
		fflush(g_log_file);
	}
}

#endif
