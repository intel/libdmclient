/******************************************************************************
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/
/*!
 * @file internals.h
 *
 * @brief Internal structure and functions.
 *
 ******************************************************************************/

#ifndef INTERNALS_H
#define INTERNALS_H

#include <config.h>
#include <syncml_tk_prefix_file.h>
#include <sml.h>
#include <smldef.h>
#include <smldtd.h>
#include <smlmetinfdtd.h>
#include <smldevinfdtd.h>
#include <smlerr.h>
#include <mgrutil.h>
#include <syncml_error.h>
#include <omadmclient.h>
#include "dmtree/dmsession.h"


#define PRV_MAX_WORKSPACE_SIZE 40000
#define PRV_USER_RESP_MAX_LEN  255

#define PRV_ALERT_STRING_DISPLAY          "1100"
#define PRV_ALERT_STRING_CONFIRM          "1101"
#define PRV_ALERT_STRING_USER_INPUT       "1102"
#define PRV_ALERT_STRING_USER_CHOICE      "1103"
#define PRV_ALERT_STRING_USER_MULTICHOICE "1104"

#define PRV_COLUMN_STR ":"

#define PRV_MD5_DIGEST_LEN     16

typedef enum
{
    AUTH_TYPE_UNKNOWN = 0,
    AUTH_TYPE_HTTP_BASIC,
    AUTH_TYPE_HTTP_DIGEST,
    AUTH_TYPE_BASIC,
    AUTH_TYPE_DIGEST,
    AUTH_TYPE_HMAC,
    AUTH_TYPE_X509,
    AUTH_TYPE_SECURID,
    AUTH_TYPE_SAFEWORD,
    AUTH_TYPE_DIGIPASS,
    AUTH_TYPE_TRANSPORT
} authType_t;

typedef struct
{
    authType_t type;
    char *     name;
    char *     secret;
    char *     data;
} authDesc_t;

typedef struct
{
  SmlProtoElement_t elementType;
  SmlPcdataPtr_t    cmdID;
} basicElement_t;

typedef struct _elemCell
{
    basicElement_t *   element;
    int8_t             msg_id;
    struct _elemCell * next;
} elemCell_t;

typedef struct
{
    char *       id;
    char *       name;
    char *       uri;
    authDesc_t * toServerCred;
    authDesc_t * toClientCred;
} accountDesc_t;

typedef struct
{
    InstanceID_t     smlH;
    dmtree_session *  dmtreeH;
    int              session_id;
    int              message_id;
    int              command_id;
    elemCell_t *     elem_first;
    elemCell_t *     elem_last;
    elemCell_t *     old_elem;
    char *           reply_ref;
    accountDesc_t *  account;
    int              srv_auth;
    int              clt_auth;
    dmclt_callback_t alert_cb;
    void *           cb_data;
    SmlSequencePtr_t sequence;
    int              seq_code;
} internals_t;


// implemented in codec.c
char * encode_b64     (char * data, size_t len);
char * decode_b64     (char * data);
char * encode_b64_md5 (char * data, size_t len);
char * encode_md5     (char * data, size_t len);


// implemented in dmtree.c
int             get_server_account (internals_t * internP, char * serverID);
SmlReplacePtr_t get_device_info    (internals_t * internP);
int             get_node           (internals_t * internP, SmlItemPtr_t itemP, SmlItemPtr_t resultP);
int             add_node           (internals_t * internP, SmlItemPtr_t itemP);
int             replace_node       (internals_t * internP, SmlItemPtr_t itemP);
int             delete_node        (internals_t * internP, SmlItemPtr_t itemP);
int             copy_node          (internals_t * internP, SmlItemPtr_t itemP);


// implemented in utils.c
char * str_cat_2 (char * first, char * second);
char * str_cat_3 (char * first, char * second, char * third);
char * str_cat_5 (char * first, char * second, char * third, char * fourth, char* fifth);

void   set_pcdata_string (SmlPcdataPtr_t dataP, char * string);
void   set_pcdata_int    (SmlPcdataPtr_t dataP, int value);
void   set_pcdata_hex    (SmlPcdataPtr_t dataP, int value);
void   set_pcdata_pcdata (SmlPcdataPtr_t dataP, SmlPcdataPtr_t origP);
int    pcdata_to_int     (SmlPcdataPtr_t dataP);
char * proto_as_string   (SmlProtoElement_t proto);

SmlStatusPtr_t create_status  (internals_t * internP, int code, SmlGenericCmdPtr_t pContent);
void           add_target_ref (SmlStatusPtr_t statusP, SmlTargetPtr_t target);
void           add_source_ref (SmlStatusPtr_t statusP, SmlSourcePtr_t source);

authType_t     get_from_chal_meta (SmlPcdataPtr_t metaP, char ** nonceP);
SmlPcdataPtr_t create_chal_meta   (authType_t type, char * nonce);
void           extract_from_meta  (SmlPcdataPtr_t metaP, char ** formatP, char ** typeP);
SmlPcdataPtr_t convert_to_meta    (char * format, char * type);

void         add_element       (internals_t * internP, basicElement_t * elemP);
void         free_element_list (elemCell_t * listP);
void         refresh_elements  (internals_t * internP);
elemCell_t * retrieve_element  (internals_t * internP, char * cmdRef, char * msgRef);
void         put_back_element  (internals_t * internP, elemCell_t * cellP);

dmclt_ui_t * get_ui_from_sml(SmlAlertPtr_t alertP);
void         free_dmclt_alert(dmclt_ui_t * alertP);

void set_new_uri (internals_t * internP, char * uri);


// implemented in callbacks.c
SmlCallbacksPtr_t get_callbacks();


// implemented in credantials.c
SmlCredPtr_t get_credentials     (authDesc_t * authP);
int          check_credentials   (SmlCredPtr_t credP, authDesc_t * authP);
SmlChalPtr_t get_challenge       (authDesc_t * authP);
authType_t   auth_string_as_type (char * string);
char *       auth_type_as_string (authType_t type);

// implemented in package0.c
int decode_package_0   (char * pkg0, int pkg0_len, char ** serverID, int * sessionID, char * flags);
int validate_package_0 (internals_t * internP, char * pkg0, int pkg0_len);


#endif // INTERNALS_H
