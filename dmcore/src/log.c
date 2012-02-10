/*
 * libdmclient
 *
 * Copyright (C) 2012 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * David Navarro <david.navarro@intel.com>
 *
 */

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
