/*
 * libdmclient test materials
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
