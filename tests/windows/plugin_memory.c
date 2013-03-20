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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "memory_mo_util.h"

#include "syncml_error.h"


#define PRV_BASE_URI "./Vendor/memory"
#define PRV_BASE_ACL "Add=*&Get=*&Replace=*&Delete=*"

static int prv_initFN(void **oData)
{
    return memory_mo_init(PRV_BASE_URI, PRV_BASE_ACL, oData);
}

omadm_mo_interface_t * omadm_get_mo_interface()
{
    omadm_mo_interface_t *retVal = NULL;

    retVal = malloc(sizeof(*retVal));
    if (retVal) {
        memset(retVal, 0, sizeof(*retVal));
        retVal->base_uri = strdup(PRV_BASE_URI);
        retVal->initFunc = prv_initFN;
        retVal->closeFunc = memory_mo_close;
        retVal->isNodeFunc = memory_mo_is_node;
        retVal->findURNFunc = memory_mo_findURN;
        retVal->getFunc = memory_mo_get;
        retVal->getACLFunc = memory_mo_getACL;
        retVal->setFunc = memory_mo_set;
        retVal->setACLFunc = memory_mo_setACL;
        retVal->deleteFunc = memory_mo_delete;
    }

    return retVal;
}
