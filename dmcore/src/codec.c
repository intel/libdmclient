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
 * @file codec.c
 *
 * @brief Base64 and MD5 utility functions.
 *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <md5.h>

#include "internals.h"

#define PRV_B64_PADDING '='

static char s_b64_alphabet[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

static void prv_encode_block(uint8_t input[3],
                             char output[4])
{
    memset(output, 0, 4);

    output[0] = s_b64_alphabet[input[0] >> 2];
    output[1] = s_b64_alphabet[((input[0] & 0x03) << 4) | (input[1] >> 4)];
    output[2] = s_b64_alphabet[((input[1] & 0x0F) << 2) | (input[2] >> 6)];
    output[3] = s_b64_alphabet[input[2] & 0x3F];
}

static uint8_t prv_revert_b64(uint8_t value)
{
    if (value >= 'A' && value <= 'Z')
    {
        return (value - 'A');
    }
    if (value >= 'a' && value <= 'z')
    {
        return (26 + value - 'a');
    }
    if (value >= '0' && value <= '9')
    {
        return (52 + value - '0');
    }
    switch (value)
    {
    case '+':
        return 62;
    case '/':
        return 63;
    default:
        return 0;
    }
}

static void prv_decode_block(char input[4],
                             uint8_t output[3])
{
    uint8_t tmp[4];
    int i;

    memset(output, 0, 3);

    for (i = 0; i < 4 ; i++)
    {
        tmp[i] = prv_revert_b64(input[i]);
    }

    output[0] = (tmp[0] << 2) | (tmp[1] >> 4);
    output[1] = (tmp[1] << 4) | (tmp[2] >> 2);
    output[2] = (tmp[2] << 6) | tmp[3];
}

char * encode_b64(buffer_t data)
{
    unsigned int data_index;
    unsigned int result_index;
    char * result = NULL;
    size_t result_len;

    result_len = 4 * (data.len / 3) + 1;
    if (data.len % 3) result_len += 4;

    result = (char *) malloc(result_len);
    if (!result) return NULL;
    memset(result, 0, result_len);

    data_index = 0;
    result_index = 0;
    while (data_index < data.len)
    {
        switch (data.len - data_index)
        {
        case 0:
            // should never happen
            break;
        case 1:
            result[result_index] = s_b64_alphabet[data.buffer[data_index] >> 2];
            result[result_index + 1] = s_b64_alphabet[(data.buffer[data_index] & 0x03) << 4];
            result[result_index + 2] = PRV_B64_PADDING;
            result[result_index + 3] = PRV_B64_PADDING;
            break;
        case 2:
            result[result_index] = s_b64_alphabet[data.buffer[data_index] >> 2];
            result[result_index + 1] = s_b64_alphabet[(data.buffer[data_index] & 0x03) << 4 | (data.buffer[data_index+1] >> 4)];
            result[result_index + 2] = s_b64_alphabet[(data.buffer[data_index+1] & 0x0F) << 2];
            result[result_index + 3] = PRV_B64_PADDING;
            break;
        default:
            prv_encode_block(data.buffer + data_index, result + result_index);
            break;
        }
        data_index += 3;
        result_index += 4;
    }

    return result;
}

char * encode_b64_str(char * string)
{
    buffer_t data;

    data.buffer = (uint8_t *)string;
    data.len = strlen(string);

    return encode_b64(data);
}

void decode_b64(char * data,
                buffer_t * resultP)
{
    unsigned int data_index;
    unsigned int result_index;
    size_t data_len;

    resultP->buffer = NULL;
    resultP->len = 0;

    data_len = strlen(data);
    if (data_len % 4) return;

    resultP->len = (data_len >> 2) * 3;
    resultP->buffer = (unsigned char *) malloc(resultP->len);
    if (NULL == resultP->buffer) return;

    // remove padding
    while (data[data_len - 1] == PRV_B64_PADDING)
    {
        data_len--;
    }

    memset(resultP->buffer, 0, resultP->len);

    data_index = 0;
    result_index = 0;
    while (data_index < data_len)
    {
        prv_decode_block(data + data_index, resultP->buffer + result_index);
        data_index += 4;
        result_index += 3;
    }
    switch(data_index - data_len)
    {
    case 0:
        break;
    case 2:
        {
            uint8_t tmp[2];

            tmp[0] = prv_revert_b64(data[data_len-2]);
            tmp[1] = prv_revert_b64(data[data_len-1]);

            resultP->buffer[result_index - 3] = (tmp[0] << 2) | (tmp[1] >> 4);
            resultP->buffer[result_index - 2] = (tmp[1] << 4);
            resultP->len -= 2;
        }
        break;
    case 3:
        {
            uint8_t tmp[3];

            tmp[0] = prv_revert_b64(data[data_len-3]);
            tmp[1] = prv_revert_b64(data[data_len-2]);
            tmp[2] = prv_revert_b64(data[data_len-1]);

            resultP->buffer[result_index - 3] = (tmp[0] << 2) | (tmp[1] >> 4);
            resultP->buffer[result_index - 2] = (tmp[1] << 4) | (tmp[2] >> 2);
            resultP->buffer[result_index - 1] = (tmp[2] << 6);
            resultP->len -= 1;
       }
        break;
    default:
        // error
        free(resultP->buffer);
        resultP->buffer = NULL;
        resultP->len = 0;
        break;
    }
}

static void prv_compute_md5(buffer_t data,
                            uint8_t * result)
{
	md5_state_t state;

	md5_init(&state);
	md5_append(&state, data.buffer, data.len);
	md5_finish(&state, result);
}

char * encode_b64_md5(buffer_t data)
{
    buffer_t temp;

    uint8_t result[PRV_MD5_DIGEST_LEN+1];

    result[PRV_MD5_DIGEST_LEN] = 0x00;

    prv_compute_md5(data, result);

    temp.buffer = result;
    temp.len = PRV_MD5_DIGEST_LEN;

    return encode_b64(temp);
}

char * encode_b64_md5_str (char * string)
{
    buffer_t data;

    data.buffer = (uint8_t *)string;
    data.len = strlen(string);

    return encode_b64_md5(data);
}

char * encode_md5(buffer_t data)
{
    char result[PRV_MD5_DIGEST_LEN+1];

    result[PRV_MD5_DIGEST_LEN] = 0x00;

    prv_compute_md5(data, (uint8_t*)result);

    return strdup(result);
}

void buf_cat_str_buf(char * string,
                     buffer_t data,
                     buffer_t * output)
{
    int index = 0;

    output->len = strlen(string) + strlen(PRV_COLUMN_STR) + data.len;
    output->buffer = (uint8_t *)malloc(output->len);
    if (NULL == output->buffer)
    {
        output->len = 0;
        return;
    }
    memcpy(output->buffer, string, strlen(string));
    index = strlen(string);
    memcpy(output->buffer + index, PRV_COLUMN_STR, strlen(PRV_COLUMN_STR));
    index += strlen(PRV_COLUMN_STR);
    memcpy(output->buffer + index, data.buffer, data.len);
}

void buf_append_str(buffer_t * dataP,
                    char * string)
{
    uint8_t * newBuf = NULL;
    int newLen = 0;
    int index;

    newLen = strlen(string) + strlen(PRV_COLUMN_STR) + dataP->len;
    newBuf = (uint8_t *)malloc(newLen);
    if (NULL == newBuf)
    {
        return;
    }
    memcpy(newBuf, dataP->buffer, dataP->len);
    index = dataP->len;
    memcpy(newBuf + index, PRV_COLUMN_STR, strlen(PRV_COLUMN_STR));
    index += strlen(PRV_COLUMN_STR);
    memcpy(newBuf, string, strlen(string));

    free(dataP->buffer);
    dataP->buffer = newBuf;
    dataP->len = newLen;
}
