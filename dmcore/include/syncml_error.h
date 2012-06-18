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
 * @file syncml_error.h
 *
 * @brief Public headers to access the OMA DM Tree
 *
 */

#ifndef OMADM_SYNCML_ERROR_H_
#define OMADM_SYNCML_ERROR_H_

enum omadm_syncml_errors {
    OMADM_SYNCML_ERROR_NONE = 0,
    OMADM_SYNCML_ERROR_IN_PROGRESS = 101,
    OMADM_SYNCML_ERROR_SUCCESS = 200,
    OMADM_SYNCML_ERROR_AUTHENTICATION_ACCEPTED = 212,
    OMADM_SYNCML_ERROR_OPERATION_CANCELED = 214,
    OMADM_SYNCML_ERROR_NOT_EXECUTED = 215,
    OMADM_SYNCML_ERROR_NOT_MODIFIED = 304,
    OMADM_SYNCML_ERROR_INVALID_CREDENTIALS = 401,
    OMADM_SYNCML_ERROR_FORBIDDEN = 403,
    OMADM_SYNCML_ERROR_NOT_FOUND = 404,
    OMADM_SYNCML_ERROR_NOT_ALLOWED = 405,
    OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED = 406,
    OMADM_SYNCML_ERROR_MISSING_CREDENTIALS = 407,
    OMADM_SYNCML_ERROR_INCOMPLETE_COMMAND = 412,
    OMADM_SYNCML_ERROR_URI_TOO_LONG = 414,
    OMADM_SYNCML_ERROR_ALREADY_EXISTS = 418,
    OMADM_SYNCML_ERROR_DEVICE_FULL = 420,
    OMADM_SYNCML_ERROR_PERMISSION_DENIED = 425,
    OMADM_SYNCML_ERROR_COMMAND_FAILED = 500,
    OMADM_SYNCML_ERROR_COMMAND_NOT_IMPLEMENTED = 501,
    OMADM_SYNCML_ERROR_SESSION_INTERNAL = 506,
    OMADM_SYNCML_ERROR_ATOMIC_FAILED = 507
};


#endif /* OMADM_SYNCML_ERROR_H_ */
