/******************************************************************************
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/
/*!
 * @file credentials.c
 *
 * @brief Handles server and client authentifications
 *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "internals.h"

#define META_TYPE_BASIC    "syncml:auth-basic"
#define META_TYPE_DIGEST   "syncml:auth-md5"
#define META_TYPE_HMAC     "syncml:auth-MAC"
#define META_TYPE_X509     "syncml:auth-X509"
#define META_TYPE_SECURID  "syncml:auth-securid"
#define META_TYPE_SAFEWORD "syncml:auth-safeword"
#define META_TYPE_DIGIPASS "syncml:auth-digipass"

static char * prv_get_digest_basic(authDesc_t * authP)
{
    char * A;
    char * digest = NULL;

    A = str_cat_3(authP->name, PRV_COLUMN_STR, authP->secret);
    if (A != NULL)
    {
        digest = encode_b64(A, strlen(A));
        free(A);
    }

    return digest;
}

static char * prv_get_digest_md5(authDesc_t * authP)
{
    char * A;
    char * AD;
    char * scheme;
    char * digest = NULL;

    A = str_cat_3(authP->name, PRV_COLUMN_STR, authP->secret);
    if (A != NULL)
    {
        AD = encode_b64_md5(A, strlen(A));
        free(A);
        if (AD != NULL)
        {
            scheme = str_cat_3(AD, PRV_COLUMN_STR, authP->data);
            free(AD);
            if (scheme != NULL)
            {
                digest = encode_b64_md5(scheme, strlen(scheme));
                free(scheme);
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
        if (authP->data) free(authP->data);
        authP->data = encode_b64((char *)authP, 8);
        metaP = create_chal_meta(authP->type, authP->data);
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

int get_server_account(char * serverID,
                       accountDesc_t ** accountP)
{
#warning TODO: Implement the real stuff
    *accountP = (accountDesc_t *)malloc(sizeof(accountDesc_t));
    (*accountP)->id = strdup("funambol");
    (*accountP)->name = strdup("Funambol");
    (*accountP)->uri = strdup("http://127.0.0.1:8080/funambol/dm");
    (*accountP)->toServerCred = (authDesc_t *)malloc(sizeof(authDesc_t));
    (*accountP)->toServerCred->type = AUTH_TYPE_BASIC;
    (*accountP)->toServerCred->name = strdup("funambol");
    (*accountP)->toServerCred->secret = strdup("funambol");
    (*accountP)->toServerCred->data = strdup("");
    (*accountP)->toClientCred = (authDesc_t *)malloc(sizeof(authDesc_t));
    (*accountP)->toClientCred->type = AUTH_TYPE_DIGEST;
    (*accountP)->toClientCred->name = strdup("funambol");
    (*accountP)->toClientCred->secret = strdup("srvpwd");
    (*accountP)->toClientCred->data = strdup("");

    return OMADM_SYNCML_ERROR_NONE;
}
