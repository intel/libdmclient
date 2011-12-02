/******************************************************************************
 * Copyright (C) 2011  Intel Corporation. All rights reserved.
 *****************************************************************************/
/*!
 * @file omadmclient.h
 *
 * @brief Interface to the omadmclient library.
 *
 ******************************************************************************/

#ifndef OMADMCLIENT_H
#define OMADMCLIENT_H

#include <stdint.h>


#define DMCLT_FLAG_NONE         0x00
#define DMCLT_FLAG_WBXML        0x01
#define DMCLT_FLAG_CLIENT_INIT  0x02
#define DMCLT_FLAG_UI_INFORM    0x04
#define DMCLT_FLAG_UI_ACCEPT    0x08

typedef enum
{
    DMCLT_ERR_NONE = 0,
    DMCLT_ERR_END,
    DMCLT_ERR_INTERNAL,
    DMCLT_ERR_MEMORY,
    DMCLT_ERR_USAGE
} dmclt_err_t;

typedef enum
{
    DMCLT_UI_TYPE_UNDEFINED = 0,
    DMCLT_UI_TYPE_DISPLAY,
    DMCLT_UI_TYPE_CONFIRM,
    DMCLT_UI_TYPE_USER_INPUT,
    DMCLT_UI_TYPE_USER_CHOICE,
    DMCLT_UI_TYPE_USER_MULTICHOICE
} dmclt_ui_type_t;

typedef enum
{
    DMCLT_UI_INPUT_UNDEFINED = 0,
    DMCLT_UI_INPUT_ALPHA,
    DMCLT_UI_INPUT_NUM,
    DMCLT_UI_INPUT_DATE,
    DMCLT_UI_INPUT_TIME,
    DMCLT_UI_INPUT_PHONE,
    DMCLT_UI_INPUT_IP
} dmclt_ui_input_t;

typedef enum
{
    DMCLT_UI_ECHO_UNDEFINED = 0,
    DMCLT_UI_ECHO_TEXT,
    DMCLT_UI_ECHO_PASSWD
} dmclt_ui_echo_t;


typedef struct
{
    char *          uri;
    long            length;
    unsigned char * data;
} dmclt_buffer_t;


typedef struct
{
    dmclt_ui_type_t type;
    int min_disp;
    int max_disp;
    int max_resp_len;
    dmclt_ui_input_t input_type;
    dmclt_ui_echo_t echo_type;
    char * disp_msg;
    char * dflt_resp;
    char ** choices;
} dmclt_ui_t;

typedef int (*dmclt_callback_t) (void * userData, const dmclt_ui_t * uiData, char * userReply);

typedef void * dmclt_session;


/*!
 * @brief Opens an OMA DM session to the specified server
 *
 * @param sessionH (out) session handle
 * @param serverID id of the DM server to connect to
 * @param flags (in/out) sessions flags. See DMCLT_FLAG_*
 * @param UICallbacksP callback for user interaction. Can be nil.
 * @param userData past as parameter to UICallbacksP
 *
 * @returns DMCLT_ERR_NONE if successful or one of DMCLT_ERR_*
 */
dmclt_err_t omadmclient_session_open(dmclt_session * sessionH, char * serverID, int sessionID, char * flags, dmclt_callback_t UICallbacksP, void * userData);

/*!
 * @brief Opens an OMA DM session in reply to an package #0
 *
 * @param sessionH (out) session handle
 * @param pkg0 buffer containing the received package #0
 * @param pkg0_len length of the pkg0 buffer
 * @param flags (in/out) sessions flags. See DMCLT_FLAG_*
 * @param UICallbacksP callback for user interaction. Can be nil
 * @param userData past as parameter to UICallbacksP
 *
 * @returns DMCLT_ERR_NONE if successful or one of DMCLT_ERR_*
 */
dmclt_err_t omadmclient_session_open_on_alert(dmclt_session * sessionH, uint8_t * pkg0, int pkg0_len, char * flags, dmclt_callback_t UICallbacksP, void * userData);

/*!
 * @brief Closes an open OMA DM session
 *
 * @param sessionH session handle
 */
void omadmclient_session_close(dmclt_session sessionH);

/*!
 * @brief Retrieves the next packet to be sent to the server
 *
 * @param sessionH session handle
 * @param packetP (out) storage for the packet must be freed by the caller
 *
 * @returns DMCLT_ERR_NONE if successful, DMCLT_ERR_END if the session is over
 *          or one of DMCLT_ERR_*
 */
dmclt_err_t omadmclient_get_next_packet(dmclt_session sessionH, dmclt_buffer_t * packetP);

/*!
 * @brief Processes a packet received from the server
 *
 * @param sessionH session handle
 * @param packetP packet received from the server.
 *
 * @returns DMCLT_ERR_NONE if successful or one of DMCLT_ERR_*
 */
dmclt_err_t omadmclient_process_reply(dmclt_session sessionH, dmclt_buffer_t * packetP);

dmclt_err_t omadmclient_cancel(dmclt_session sessionH);

/*!
 * @brief Frees internal data of a dmclt_buffer_t.
 *        The dmclt_buffer_t remains untouched
 *
 * @param packetP
 */
void omadmclient_clean_buffer(dmclt_buffer_t * packetP);

#endif // OMADMCLIENT_H
