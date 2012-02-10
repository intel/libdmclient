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
 * @file log.h
 *
 * @brief
 * Macros and functions for logging
 *
 ******************************************************************************/

#ifndef DMC_LOG_H
#define DMC_LOG_H

/* TODO: Not thread safe */

int dmc_log_open(const char *log_file_name);
void dmc_log_printf(unsigned int line_number, const char *file_name,
                const char *message, ...);
void dmc_logu_printf(const char *message, ...);
void dmc_log_close(void);

#ifdef DMC_LOGGING
    #define DMC_LOGF(message, ...) dmc_log_printf(__LINE__, \
            __FILE__, message, __VA_ARGS__)
    #define DMC_LOG(message) dmc_log_printf(__LINE__, __FILE__, \
            message)
    #define DMC_LOGUF(message, ...) dmc_logu_printf(message, \
            __VA_ARGS__)
    #define DMC_LOGU(message) dmc_logu_printf(message)
#else
    #define DMC_LOGF(message, ...)
    #define DMC_LOG(message)
    #define DMC_LOGUF(message, ...)
    #define DMC_LOGU(message)
#endif

#endif
