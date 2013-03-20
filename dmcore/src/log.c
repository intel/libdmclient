/*
 * libdmclient
 *
 * Copyright (C) 2012 Intel Corporation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
