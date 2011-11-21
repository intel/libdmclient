/******************************************************************************
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/

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
