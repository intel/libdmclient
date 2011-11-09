/******************************************************************************
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/
/*!
 * @file codec.c
 *
 * @brief Base64 and MD5 utility functions.
 *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <gnutls/gnutls.h>

#include "internals.h"

char * encode_b64(char * data,
                  size_t len)
{
    gnutls_datum_t input;
    gnutls_datum_t output;
    unsigned int index;
    char * string;
    char * result = NULL;

    input.data = (unsigned char *)data;
    input.size = len;

    if (gnutls_pem_base64_encode_alloc("", &input, &output))
    {
        return NULL;
    }

    index = 0;
    while (index < output.size && output.data[index] != 0x0A)
    {
        index++;
    }
    if (index < output.size)
    {
        index++;
        string = (char *)output.data + index;
        while (index < output.size && output.data[index] != 0x0A)
        {
            index++;
        }
        if (index < output.size)
        {
            output.data[index] = 0;
            result = strdup(string);
        }
    }

    gnutls_free(output.data);

    return result;
}

#define PEM_HEADER "-----BEGIN -----\n"
#define PEM_FOOTER "\n-----END -----"

char * decode_b64(char * data)
{
    gnutls_datum_t input;
    gnutls_datum_t output;
    char * result = NULL;

    input.data = (unsigned char *)str_cat_3(PEM_HEADER, data, PEM_FOOTER);
    input.size = strlen((char *)input.data);

    if (gnutls_pem_base64_decode_alloc(NULL, &input, &output))
    {
        return NULL;
    }

    result = strdup((char *)output.data);

    gnutls_free(output.data);

    return result;
}

char * encode_b64_md5(char * data,
                      size_t len)
{
    char result[PRV_MD5_DIGEST_LEN+1];

    result[PRV_MD5_DIGEST_LEN] = 0x00;

    gnutls_hash_fast(GNUTLS_DIG_MD5, data, len, result);

    return encode_b64(result, PRV_MD5_DIGEST_LEN);
}

char * encode_md5(char * data,
                  size_t len)
{
    char result[PRV_MD5_DIGEST_LEN+1];

    result[PRV_MD5_DIGEST_LEN] = 0x00;

    gnutls_hash_fast(GNUTLS_DIG_MD5, data, len, result);

    return strdup(result);
}
