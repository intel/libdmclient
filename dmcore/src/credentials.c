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
 * @file credentials.c
 *
 * @brief Handles server and client authentifications
 *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "internals.h"

#include "error_macros.h"

#define META_TYPE_BASIC    "syncml:auth-basic"
#define META_TYPE_DIGEST   "syncml:auth-md5"
#define META_TYPE_HMAC     "syncml:auth-MAC"
#define META_TYPE_X509     "syncml:auth-X509"
#define META_TYPE_SECURID  "syncml:auth-securid"
#define META_TYPE_SAFEWORD "syncml:auth-safeword"
#define META_TYPE_DIGIPASS "syncml:auth-digipass"

#define VALUE_TYPE_BASIC    "BASIC"
#define VALUE_TYPE_DIGEST   "DIGEST"
#define VALUE_TYPE_HMAC     "HMAC"
#define VALUE_TYPE_X509     "X509"
#define VALUE_TYPE_SECURID  "SECURID"
#define VALUE_TYPE_SAFEWORD "SAFEWORD"
#define VALUE_TYPE_DIGIPASS "DIGIPASS"

#define DMACC_MO_URN    "urn:oma:mo:oma-dm-dmacc:1.0"


static char * prv_get_digest_basic(authDesc_t * authP)
{
    char * A;
    char * digest = NULL;

    A = str_cat_3(authP->name, PRV_COLUMN_STR, authP->secret);
    if (A != NULL)
    {
        digest = encode_b64_str(A);
        free(A);
    }

    return digest;
}

static char * prv_get_digest_md5(authDesc_t * authP)
{
    char * A;
    char * AD;
    char * digest = NULL;

    A = str_cat_3(authP->name, PRV_COLUMN_STR, authP->secret);
    if (A != NULL)
    {
        AD = encode_b64_md5_str(A);
        free(A);
        if (AD != NULL)
        {
            buffer_t dataBuf;

            buf_cat_str_buf(AD, authP->data, &dataBuf);
            free(AD);
            if (dataBuf.buffer)
            {
                digest = encode_b64_md5(dataBuf);
                free(dataBuf.buffer);
            }
        }
    }

    return digest;
}

SmlCredPtr_t get_credentials(authDesc_t * authP)
{
    SmlCredPtr_t credP = NULL;

    switch (authP->type)
    {
    case AUTH_TYPE_BASIC:
        {
            char * digest;

            digest = prv_get_digest_basic(authP);
            if (digest == NULL) goto error;

            credP = smlAllocCred();
            if (credP)
            {
                credP->meta = convert_to_meta("b64", META_TYPE_BASIC);
                set_pcdata_string(credP->data, digest);
            }
            free(digest);
        }
        break;
    case AUTH_TYPE_DIGEST:
        {
            char * digest;

            digest = prv_get_digest_md5(authP);
            if (digest == NULL) goto error;

            credP = smlAllocCred();
            if (credP)
            {
                credP->meta = convert_to_meta("b64", META_TYPE_DIGEST);
                set_pcdata_string(credP->data, digest);
            }
            free(digest);
        }
        break;

    default:
        // Authentification is either done at another level or not supported
        break;
    }

error:
    return credP;
}


int check_credentials(SmlCredPtr_t credP,
                      authDesc_t * authP)
{
    int status = OMADM_SYNCML_ERROR_INVALID_CREDENTIALS;
    authType_t credType;
    char * data = smlPcdata2String(credP->data);

    if (!data) goto error; //smlPcdata2String() returns null only in case of allocation error

    credType = get_from_chal_meta(credP->meta, NULL);

    switch (authP->type)
    {
    case AUTH_TYPE_BASIC:
        {
            if (credType == AUTH_TYPE_BASIC)
            {
                char * digest = prv_get_digest_basic(authP);
                if (!strcmp(digest, data))
                {
                    status = OMADM_SYNCML_ERROR_AUTHENTICATION_ACCEPTED;
                }
            }
        }
        break;
    case AUTH_TYPE_DIGEST:
        {
            if (credType == AUTH_TYPE_DIGEST)
            {
                char * digest = prv_get_digest_md5(authP);
                if (!strcmp(digest, data))
                {
                    status = OMADM_SYNCML_ERROR_AUTHENTICATION_ACCEPTED;
                }
            }
        }
        break;

    default:
        break;
    }

    free(data);

error:
    return status;
}

SmlChalPtr_t get_challenge(authDesc_t * authP)
{
    SmlPcdataPtr_t metaP;
    SmlChalPtr_t chalP;

    switch (authP->type)
    {
    case AUTH_TYPE_BASIC:
        metaP = create_chal_meta(authP->type, NULL);
        break;
    case AUTH_TYPE_DIGEST:
        // TODO generate new nonce
        if (authP->data.buffer) free(authP->data.buffer);
        authP->data.buffer = (uint8_t *)authP;
        authP->data.len = 8;
        authP->data.buffer = (uint8_t *)encode_b64(authP->data);
        authP->data.len = strlen((const char *)(authP->data.buffer));
        metaP = create_chal_meta(authP->type, &(authP->data));
        break;
    default:
        metaP = NULL;
        break;
    }

    if (metaP)
    {
        chalP = (SmlChalPtr_t)malloc(sizeof(SmlChal_t));
        if(chalP)
        {
            chalP->meta = metaP;
        }
        else
        {
            smlFreePcdata(metaP);
        }
    }
    else
    {
        chalP = NULL;
    }

    return chalP;
}

authType_t auth_string_as_type(char * string)
{
    if (!strcmp(string, META_TYPE_BASIC))
        return AUTH_TYPE_BASIC;
    if (!strcmp(string, META_TYPE_DIGEST))
        return AUTH_TYPE_DIGEST;
    if (!strcmp(string, META_TYPE_HMAC))
        return AUTH_TYPE_HMAC;
    if (!strcmp(string, META_TYPE_X509))
        return AUTH_TYPE_X509;
    if (!strcmp(string, META_TYPE_SECURID))
        return AUTH_TYPE_SECURID;
    if (!strcmp(string, META_TYPE_SAFEWORD))
        return AUTH_TYPE_SAFEWORD;
    if (!strcmp(string, META_TYPE_DIGIPASS))
        return AUTH_TYPE_DIGIPASS;

    return AUTH_TYPE_UNKNOWN;
}

char * auth_type_as_string(authType_t type)
{
    switch (type)
    {
    case AUTH_TYPE_HTTP_BASIC:
        return "";
    case AUTH_TYPE_HTTP_DIGEST:
        return "";
    case AUTH_TYPE_BASIC:
        return META_TYPE_BASIC;
    case AUTH_TYPE_DIGEST:
        return META_TYPE_DIGEST;
    case AUTH_TYPE_HMAC:
        return META_TYPE_HMAC;
    case AUTH_TYPE_X509:
        return META_TYPE_X509;
    case AUTH_TYPE_SECURID:
        return META_TYPE_SECURID;
    case AUTH_TYPE_SAFEWORD:
        return META_TYPE_SAFEWORD;
    case AUTH_TYPE_DIGIPASS:
        return META_TYPE_DIGIPASS;
    case AUTH_TYPE_TRANSPORT:
        return "";
    case AUTH_TYPE_UNKNOWN:
    default:
        return "";
    }
}

static authType_t auth_value_as_type(char * string)
{
    if (!strcmp(string, VALUE_TYPE_BASIC))
        return AUTH_TYPE_BASIC;
    if (!strcmp(string, VALUE_TYPE_DIGEST))
        return AUTH_TYPE_DIGEST;
    if (!strcmp(string, VALUE_TYPE_HMAC))
        return AUTH_TYPE_HMAC;
    if (!strcmp(string, VALUE_TYPE_X509))
        return AUTH_TYPE_X509;
    if (!strcmp(string, VALUE_TYPE_SECURID))
        return AUTH_TYPE_SECURID;
    if (!strcmp(string, VALUE_TYPE_SAFEWORD))
        return AUTH_TYPE_SAFEWORD;
    if (!strcmp(string, VALUE_TYPE_DIGIPASS))
        return AUTH_TYPE_DIGIPASS;

    return AUTH_TYPE_UNKNOWN;
}

static int prv_fill_credentials(const mo_mgr_t iMgr,
                                char * uri,
                                authDesc_t * authP)
{
    dmtree_node_t node;
    int code;

    memset(&node, 0, sizeof(dmtree_node_t));

    node.uri = str_cat_2(uri, "/AAuthType");
    if (!node.uri) return OMADM_SYNCML_ERROR_DEVICE_FULL;
    code = momgr_get_value(iMgr, &node);
    if (OMADM_SYNCML_ERROR_NONE == code)
    {
        authP->type = auth_value_as_type(node.data_buffer);
    }
    else if (OMADM_SYNCML_ERROR_NOT_FOUND != code)
    {
        dmtree_node_clean(&node, true);
        return code;
    }
    dmtree_node_clean(&node, true);

    node.uri = str_cat_2(uri, "/AAuthName");
    if (!node.uri) return OMADM_SYNCML_ERROR_DEVICE_FULL;
    code = momgr_get_value(iMgr, &node);
    if (OMADM_SYNCML_ERROR_NONE == code)
    {
        authP->name = node.data_buffer;
    }
    else if (OMADM_SYNCML_ERROR_NOT_FOUND != code)
    {
        dmtree_node_clean(&node, true);
        return code;
    }
    dmtree_node_clean(&node, false);

    node.uri = str_cat_2(uri, "/AAuthSecret");
    if (!node.uri) return OMADM_SYNCML_ERROR_DEVICE_FULL;
    code = momgr_get_value(iMgr, &node);
    if (OMADM_SYNCML_ERROR_NONE == code)
    {
        authP->secret = node.data_buffer;
    }
    else if (OMADM_SYNCML_ERROR_NOT_FOUND != code)
    {
        dmtree_node_clean(&node, true);
        return code;
    }
    dmtree_node_clean(&node, false);

    node.uri = str_cat_2(uri, "/AAuthData");
    if (!node.uri) return OMADM_SYNCML_ERROR_DEVICE_FULL;
    code = momgr_get_value(iMgr, &node);
    if (OMADM_SYNCML_ERROR_NONE == code)
    {
        authP->data.buffer = (uint8_t *)node.data_buffer;
        authP->data.len = node.data_size;
    }
    else if (OMADM_SYNCML_ERROR_NOT_FOUND != code)
    {
        dmtree_node_clean(&node, true);
        return code;
    }
    dmtree_node_clean(&node, false);

    if (NULL == authP->name)
    {
        authP->name = strdup("");
        if (NULL == authP->name) return OMADM_SYNCML_ERROR_DEVICE_FULL;
    }
    if (NULL == authP->secret)
    {
        authP->secret = strdup("");
        if (NULL == authP->secret) return OMADM_SYNCML_ERROR_DEVICE_FULL;
    }

    return code;
}

int get_server_account(const mo_mgr_t iMgr,
                       char * serverID,
                       accountDesc_t ** accountP)
{
    DMC_ERR_MANAGE;

    char * accMoUri = NULL;
    char * accountUri = NULL;
    char * uri = NULL;
    char * subUri = NULL;
    dmtree_node_t node;
    int code;

    memset(&node, 0, sizeof(dmtree_node_t));

    DMC_FAIL(momgr_find_subtree(iMgr, NULL, DMACC_MO_URN, "ServerID", serverID, &accountUri));

    DMC_FAIL_NULL(*accountP, malloc(sizeof(accountDesc_t)), OMADM_SYNCML_ERROR_DEVICE_FULL);
    memset(*accountP, 0, sizeof(accountDesc_t));
    (*accountP)->dmtree_uri = accountUri;
    accountUri = NULL;

    DMC_FAIL_NULL(node.uri, strdup("./DevInfo/DevId"), OMADM_SYNCML_ERROR_DEVICE_FULL);
    DMC_FAIL(momgr_get_value(iMgr, &node));
    (*accountP)->id = node.data_buffer;
    dmtree_node_clean(&node, false);

    // TODO handle IPv4 and IPv6 cases
    DMC_FAIL_NULL(uri, str_cat_2((*accountP)->dmtree_uri, "/AppAddr"), OMADM_SYNCML_ERROR_DEVICE_FULL);
    DMC_FAIL(momgr_find_subtree(iMgr, uri, NULL, "AddrType", "URI", &subUri));
    free(uri);
    uri = NULL;
    DMC_FAIL_NULL(node.uri, str_cat_2(subUri, "/Addr"), OMADM_SYNCML_ERROR_DEVICE_FULL);
    DMC_FAIL(momgr_get_value(iMgr, &node));
    (*accountP)->server_uri = node.data_buffer;
    dmtree_node_clean(&node, false);
    free(subUri);
    subUri = NULL;

    // TODO handle OBEX and HTTP authentification levels
    DMC_FAIL_NULL(uri, str_cat_2((*accountP)->dmtree_uri, "/AppAuth"), OMADM_SYNCML_ERROR_DEVICE_FULL);
    code = momgr_find_subtree(iMgr, uri, NULL, "AAuthLevel", "CLCRED", &subUri);
    switch (code)
    {
    case OMADM_SYNCML_ERROR_NONE:
        DMC_FAIL_NULL((*accountP)->toServerCred, malloc(sizeof(authDesc_t)), OMADM_SYNCML_ERROR_DEVICE_FULL);
        DMC_FAIL(prv_fill_credentials(iMgr, subUri, (*accountP)->toServerCred));
        break;
    case OMADM_SYNCML_ERROR_NOT_FOUND:
        break;
    default:
        DMC_FAIL(code);
    }
    free(subUri);
    subUri = NULL;

    code = momgr_find_subtree(iMgr, uri, NULL, "AAuthLevel", "SRVCRED", &subUri);
    switch (code)
    {
    case OMADM_SYNCML_ERROR_NONE:
        DMC_FAIL_NULL((*accountP)->toClientCred, malloc(sizeof(authDesc_t)), OMADM_SYNCML_ERROR_DEVICE_FULL);
        DMC_FAIL(prv_fill_credentials(iMgr, subUri, (*accountP)->toClientCred));
        break;
    case OMADM_SYNCML_ERROR_NOT_FOUND:
        break;
    default:
        DMC_FAIL(code);
    }
    free(subUri);
    subUri = NULL;

DMC_ON_ERR:

    if (accMoUri) free(accMoUri);
    if (accountUri) free(accountUri);
    if (uri) free(uri);
    if (subUri) free(subUri);
    dmtree_node_clean(&node, true);

    return DMC_ERR;
}

void store_nonce(const mo_mgr_t iMgr,
                 const accountDesc_t * accountP,
                 bool server)
{
    char * subUri = NULL;

    if (OMADM_SYNCML_ERROR_NONE == momgr_find_subtree(iMgr, accountP->dmtree_uri, NULL, "AAuthLevel", server?"CLCRED":"SRVCRED", &subUri))
    {
        dmtree_node_t node;

        memset(&node, 0, sizeof(dmtree_node_t));
        node.uri = str_cat_2(subUri, "/AAuthData");
        if (node.uri)
        {
            node.data_buffer = (char *)(server?accountP->toServerCred->data.buffer:accountP->toClientCred->data.buffer);
            node.data_size = server?accountP->toServerCred->data.len:accountP->toClientCred->data.len;
            momgr_set_value(iMgr, &node);
            free(node.uri);
        }
        free(subUri);
    }
}
