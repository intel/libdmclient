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
 * @file package0.c
 *
 * @brief Package0 utility functions.
 *
 ******************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <endian.h>
#include <arpa/inet.h>

#include "internals.h"

/******************************************************************************
 * The Structure of General Notification Initiated Session Alert is:
 *
 * + - - - - - - - - + - - - - - - - - + - - - - - - - - ... - + - - - - - - -
 * |         DIGEST: 16bytes           | HEADER: 8 + len bytes | BODY N bytes
 * + - - - - - - - - + - - - - - - - - + - - - - - - - - ... - + - - - - - - -
 *
 * HEADER and BODY compose the TRIGGER.
 * The BODY is vendor specific. We let the caller extract and handle it.
 * The HEADER is:
 *
 * | VERSION  | UI MODE | INITIATOR | FUTURE USE | SESSION ID | ID LENGTH | ID |
 *
 * VERSION: 10 bits
 * UI MODE: 2 bits
 * INITIATOR: 1 bit
 * FUTURE USE: 27 bits
 * SESSION ID: 16 bits
 * ID LENGTH: 8 bits
 * ID: ID LENGTH bits
 *
 *
 * The DIGEST is computed as:
 *     H(B64(H(server-identifier:password)):nonce:B64(H(TRIGGER)));
 *
 * H() being the MD5 hash function.
 * If the authentification fails with the stored nonce, we retry with the nonce
 * being 0x00000000.
 *
 ******************************************************************************/

#define PRV_HEADER_LEN      8
#define PRV_GNIS_ALERT_SIZE (size_t)(PRV_MD5_DIGEST_LEN + PRV_HEADER_LEN)

#define PRV_VERSION_MASK    0xFFC0000000000000
#define PRV_VERSION_SHIFT   54

#define PRV_UI_MODE_MASK    0x0030000000000000
#define PRV_UI_MODE_SHIFT   52

#define PRV_INITIATOR_MASK  0x0008000000000000
#define PRV_INITIATOR_SHIFT 51

#define PRV_SESSION_ID_MASK     0x0000000000FFFF00
#define PRV_SESSION_ID_SHIFT    8

#define PRV_LENGTH_ID_MASK      0x00000000000000FF
#define PRV_LENGTH_ID_SHIFT     0

#define PRV_VERSION_MIN     11
#define PRV_VERSION_MAX     11

#define PRV_UI_UNDEFINED        0
#define PRV_UI_BACKGROUND       1
#define PRV_UI_INFORMATIVE      2
#define PRV_UI_USER_INTERACTION 3

#define PRV_INIT_CLIENT 0
#define PRV_INIT_SERVER 1

#define PRV_DEFAULT_NONCE "0x00000000"


int decode_package_0(buffer_t pkg0,
                     char ** serverID,
                     int * sessionID,
                     char * flags,
                     int * body_offset)
{
    uint64_t header;
    uint16_t field;

    if (PRV_GNIS_ALERT_SIZE > pkg0.len)
    {
        return OMADM_SYNCML_ERROR_INCOMPLETE_COMMAND;
    }

    memcpy(&header, pkg0.buffer + PRV_MD5_DIGEST_LEN, sizeof(uint64_t));
    header = be64toh(header);

    field = (header & PRV_VERSION_MASK) >> PRV_VERSION_SHIFT;
    if ((field < PRV_VERSION_MIN) || (field > PRV_VERSION_MAX))
    {
        return OMADM_SYNCML_ERROR_COMMAND_NOT_IMPLEMENTED;
    }

    if (NULL != flags)
    {
        field = (header & PRV_UI_MODE_MASK) >> PRV_UI_MODE_SHIFT;
        switch (field)
        {
        case PRV_UI_BACKGROUND:
            *flags &= ~(DMCLT_FLAG_UI_INFORM | DMCLT_FLAG_UI_ACCEPT);
            break;
        case PRV_UI_INFORMATIVE:
            *flags |= DMCLT_FLAG_UI_INFORM;
            break;
        case PRV_UI_USER_INTERACTION:
            *flags |= DMCLT_FLAG_UI_ACCEPT;
            break;
        default:
            break;
        }
    }

    field = (header & PRV_SESSION_ID_MASK) >> PRV_SESSION_ID_SHIFT;
#warning TODO: check if needed
    *sessionID = ntohs(field);

    field = (header & PRV_LENGTH_ID_MASK) >> PRV_LENGTH_ID_SHIFT;
    if (field == 0
    || field + PRV_GNIS_ALERT_SIZE > pkg0.len)
    {
        return OMADM_SYNCML_ERROR_INCOMPLETE_COMMAND;
    }
    *serverID = (char *)malloc(field+1);
    if (!serverID)
    {
        return OMADM_SYNCML_ERROR_COMMAND_FAILED;
    }
    memcpy(*serverID, pkg0.buffer + PRV_GNIS_ALERT_SIZE, field);
    (*serverID)[field] = 0;

    if (body_offset)
    {
        *body_offset = field + PRV_GNIS_ALERT_SIZE;
    }
    return OMADM_SYNCML_ERROR_NONE;
}


int validate_package_0(internals_t * internP,
                       buffer_t pkg0)
{
    int result;
    buffer_t data;
    char * B64_H_serv_pass;
    char * B64_H_trigger;
    char * key;
    char * H_key;

    result = OMADM_SYNCML_ERROR_COMMAND_FAILED;
    B64_H_serv_pass = NULL;
    B64_H_trigger = NULL;
    H_key = NULL;

    key = str_cat_3(internP->account->toClientCred->name,
                    PRV_COLUMN_STR,
                    internP->account->toClientCred->secret);
    if (!key) goto end;

    B64_H_serv_pass = encode_b64_md5_str(key);
    if (!B64_H_serv_pass) goto end;

    data.buffer = pkg0.buffer + PRV_MD5_DIGEST_LEN;
    data.len = pkg0.len - PRV_MD5_DIGEST_LEN;
    B64_H_trigger = encode_b64_md5(data);
    if (!B64_H_trigger) goto end;

    while (result != OMADM_SYNCML_ERROR_NONE
           && result != OMADM_SYNCML_ERROR_INVALID_CREDENTIALS)
    {
        buf_cat_str_buf(B64_H_serv_pass, internP->account->toClientCred->data, &data);
        if (!data.buffer) goto end;
        buf_append_str(&data, B64_H_trigger);

        H_key = encode_md5(data);
        free(data.buffer);
        if (!H_key) goto end;

        if (memcmp(pkg0.buffer, H_key, PRV_MD5_DIGEST_LEN))
        {
            if (result == OMADM_SYNCML_ERROR_COMMAND_FAILED)
            {
                result = OMADM_SYNCML_ERROR_IN_PROGRESS;
                free(internP->account->toClientCred->data.buffer);
                free(H_key);
                H_key = NULL;
                internP->account->toServerCred->data.buffer = (uint8_t *)strdup(PRV_DEFAULT_NONCE);
                internP->account->toServerCred->data.len = strlen(PRV_DEFAULT_NONCE);
            }
            else
            {
                result = OMADM_SYNCML_ERROR_INVALID_CREDENTIALS;
            }
        }
        else
        {
            result = OMADM_SYNCML_ERROR_NONE;
        }
    }

end:
    free(B64_H_serv_pass);
    free(B64_H_trigger);
    free(H_key);
    return result;;
}
