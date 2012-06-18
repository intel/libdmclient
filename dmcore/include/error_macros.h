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
 * @file error_macros.h
 *
 * @brief
 * Macros to help with error management
 *
 ******************************************************************************/

#ifndef DMC_ERROR_MACROS_H
#define DMC_ERROR_MACROS_H

#define DMC_ERR _err
#define DMC_ERR_MANAGE int DMC_ERR = 0
#define DMC_ON_ERR _err_label

#define DMC_FAIL(exp)           \
    do {                        \
        _err = (exp);           \
        if (_err != 0)          \
            goto DMC_ON_ERR;    \
    } while (0)

#define DMC_FAIL_LABEL(exp, label)  \
    do {                            \
        _err = (exp);               \
        if (_err != 0)              \
            goto label;             \
    } while (0)

#define DMC_FAIL_NULL(var, exp, error)  \
    do {                                \
        var = (exp);                    \
        if (var == NULL)                \
        {                               \
            _err = error;               \
            goto DMC_ON_ERR;            \
        }                               \
    } while (0)

#define DMC_FAIL_NULL_LABEL(var, exp, error, label) \
    do {                                            \
        var = (exp);                                \
        if (var == NULL)                            \
        {                                           \
            _err = error;                           \
            goto label;                             \
        }                                           \
    } while (0)

#define DMC_FAIL_ERR(exp, error)    \
    do {                            \
        _err = (exp);               \
        if (_err != 0) {            \
            _err = (error);         \
            goto DMC_ON_ERR;        \
        }                           \
    } while (0)

#define DMC_FAIL_ERR_LABEL(exp, error, label)   \
    do {                                        \
        _err = (exp);                           \
        if (_err != 0) {                        \
            _err = (error);                     \
            goto label;                         \
        }                                       \
    } while (0)

#define DMC_FAIL_FORCE(error)   \
    do {                        \
        _err = (error);         \
        goto DMC_ON_ERR;        \
    } while (0)

#define DMC_FAIL_FORCE_LABEL(error, label)  \
    do {                                    \
        _err = (error);                     \
        goto label;                         \
    } while (0)

#endif
