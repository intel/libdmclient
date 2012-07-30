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
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <omadmclient.h>
#include <libsoup/soup.h>
#include <gio/gio.h>

// implemented in test_plugin.c
omadm_mo_interface_t * test_get_mo_interface();

// HACK
typedef struct
{
    void *  unused1;
    void *  unused2;
    void *  unused3;
    int     unused4;
    int     unused5;
    int     unused6;
    void *  unused7;
    void *  unused8;
    void *  unused9;
    char *  unusedA;
    int     srv_auth;
    int     clt_auth;
} hack_internals_t;

void print_usage(void)
{
    fprintf(stderr, "Usage: testdmclient [-w] [-f FILE | -s SERVERID]\r\n");
    fprintf(stderr, "Launch a DM session with the server SERVERID. If SERVERID is not specified, \"funambol\" is used by default.\r\n\n");
    fprintf(stderr, "  -w\tuse WBXML\r\n");
    fprintf(stderr, "  -f\tuse the file as a server's packet\r\n");
    fprintf(stderr, "  -s\topen a DM session with server SERVERID\r\n");
    fprintf(stderr, "Options f and s are mutually exclusive.\r\n");

}

void output_buffer(FILE * fd, bool isWbxml, dmclt_buffer_t buffer)
{
    int i;

    if (isWbxml)
    {
        unsigned char array[16];

        i = 0;
        while (i < buffer.length)
        {
            int j;
            fprintf(fd, "  ");

            memcpy(array, buffer.data+i, 16);

            for (j = 0 ; j < 16 && i+j < buffer.length; j++)
            {
                fprintf(fd, "%02X ", array[j]);
            }
            while (j < 16)
            {
                fprintf(fd, "   ");
                j++;
            }
            fprintf(fd, "  ");
            for (j = 0 ; j < 16 && i+j < buffer.length; j++)
            {
                if (isprint(array[j]))
                    fprintf(fd, "%c ", array[j]);
                else
                    fprintf(fd, ". ");
            }
            fprintf(fd, "\n");

            i += 16;
        }
    }
    else
    {
        int tab;

        tab = -2;
        for (i = 0 ; i < buffer.length ; i++)
        {
            if (buffer.data[i] == '<')
            {
                int j;
                if (i+1 < buffer.length && buffer.data[i+1] == '/')
                {
                    tab--;
                    if (i != 0 && buffer.data[i-1] == '>')
                    {
                        fprintf(fd, "\n");
                        for(j = 0 ; j < tab*4 ; j++) fprintf(fd, " ");
                    }
                }
                else
                {
                    if (i != 0 && buffer.data[i-1] == '>')
                    {
                        fprintf(fd, "\n");
                        for(j = 0 ; j < tab*4 ; j++) fprintf(fd, " ");
                    }
                    tab++;
                }
            }
            fprintf(fd, "%c", buffer.data[i]);
        }
    }
    fprintf(fd, "\n\n");
    fflush(fd);
}

int uiCallback(void * userData,
               const dmclt_ui_t * alertData,
               char * userReply)
{
    int code = 200;

    fprintf(stderr, "\nAlert received:\n");
    fprintf(stderr, "type: %d\n", alertData->type);
    fprintf(stderr, "min_disp: %d\n", alertData->min_disp);
    fprintf(stderr, "max_disp: %d\n", alertData->max_disp);
    fprintf(stderr, "max_resp_len: %d\n", alertData->max_resp_len);
    fprintf(stderr, "input_type: %d\n", alertData->input_type);
    fprintf(stderr, "echo_type: %d\n", alertData->echo_type);
    fprintf(stderr, "disp_msg: \"%s\"\n", alertData->disp_msg);
    fprintf(stderr, "dflt_resp: \"%s\"\n", alertData->dflt_resp);

    fprintf(stdout, "\n----------- UI -----------\r\n\n");
    fprintf(stdout, "%s\r\n", alertData->disp_msg);
    if (alertData->type >= DMCLT_UI_TYPE_USER_CHOICE)
    {
        int i = 0;
        while(alertData->choices[i])
        {
            fprintf(stdout, "%d: %s\r\n", i+1, alertData->choices[i]);
            i++;
        }
    }
    fprintf(stdout, "\n--------------------------\r\n\n");

    if (alertData->type >= DMCLT_UI_TYPE_CONFIRM)
    {
        char reply[256];

        fprintf(stdout, "? ");
        fflush(stdout);
        memset(reply, 0, 256);
        fgets(reply, 256, stdin);
        if (reply[0] == 0)
            code = 214;

        if(alertData->type == DMCLT_UI_TYPE_CONFIRM)
        {
            if (reply[0] == 'y')
                code = 200;
            else
                code = 304;
        }
        else
        {
            int s;
            for (s = 0 ; 0 != reply[s] && 0x0A != reply[s] ; s++ ) ;
            reply[s] = 0;
            strncpy(userReply, reply, alertData->max_resp_len);
        }
    }

    return code;
}

int sendPacket(SoupSession * soupH,
               char * type,
               dmclt_buffer_t * packet,
               dmclt_buffer_t * reply)
{
    guint status = -1;
    SoupMessage *msgP;

    memset(reply, 0, sizeof(dmclt_buffer_t));

    msgP = soup_message_new("POST", packet->uri);
    if (msgP == NULL) goto error;

    soup_message_set_request(msgP, type,
                             SOUP_MEMORY_COPY, packet->data, packet->length);

    status = soup_session_send_message (soupH, msgP);
    if (status == 200)
    {
        reply->length = msgP->response_body->length;
        reply->data = malloc(reply->length);
        memcpy(reply->data, msgP->response_body->data, reply->length);
    }

    g_object_unref(G_OBJECT(msgP));

error:
    return (int) status;
}

int main(int argc, char *argv[])
{
    dmclt_session session;
    dmclt_buffer_t buffer;
    dmclt_buffer_t reply;
    int c;
    bool isWbxml = false;
    int err;
    int status;
    SoupSession * soupH;
    char * server = NULL;
    char * file = NULL;
    omadm_mo_interface_t * testMoP;
    char * proxyStr;

    g_type_init();
    proxyStr = getenv("http_proxy");
    if (proxyStr)
    {
        SoupURI *proxy = NULL;

        proxy = soup_uri_new(proxyStr);
        soupH = soup_session_sync_new_with_options(SOUP_SESSION_PROXY_URI, proxy,
                                                   NULL);
    }
    else
    {
        soupH = soup_session_sync_new();
    }

    server = NULL;
    file = NULL;
    opterr = 0;

    while ((c = getopt (argc, argv, "ws:f:")) != -1)
    {
        switch (c)
        {
        case 'w':
            isWbxml = true;
            break;
        case 's':
            server = optarg;
            break;
        case 'f':
            file = optarg;
            break;
        case '?':
            print_usage();
            return 1;
        default:
            break;
        }
    }

    if (server && file)
    {
        print_usage();
        return 1;
    }

    session = omadmclient_session_init(isWbxml);
    if (session == NULL)
    {
        fprintf(stderr, "Initialization failed\r\n");
        return 1;
    }
    err = omadmclient_set_UI_callback(session, uiCallback, NULL);
    if (err != DMCLT_ERR_NONE)
    {
        fprintf(stderr, "Initialization failed: %d\r\n", err);
        return err;
    }

    testMoP = test_get_mo_interface();
    if (testMoP)
    {
        err = omadmclient_session_add_mo(session, testMoP);
        if (err != DMCLT_ERR_NONE)
        {
            fprintf(stderr, "Adding test MO failed: %d\r\n", err);
            if (testMoP->base_uri) free(testMoP->base_uri);
            free(testMoP);
        }
    }
    else
    {
        fprintf(stderr, "Loading test MO failed\r\n");
    }
        
    err = omadmclient_session_start(session,
                                    server?server:"funambol",
                                    1);
    if (err != DMCLT_ERR_NONE)
    {
        fprintf(stderr, "Session opening to \"%s\" failed: %d\r\n", server?server:"funambol", err);
        return err;
    }
    
    if (!file)
    {
        do
        {
            err = omadmclient_get_next_packet(session, &buffer);
            if (DMCLT_ERR_NONE == err)
            {
                output_buffer(stderr, isWbxml, buffer);
                status = sendPacket(soupH, isWbxml?"application/vnd.syncml+wbxml":"application/vnd.syncml+xml", &buffer, &reply);
                fprintf(stderr, "Reply from \"%s\": %d\r\n\n", buffer.uri, status);

                omadmclient_clean_buffer(&buffer);

                if (200 == status)
                {
                    if (isWbxml)
                    {
                        output_buffer(stderr, isWbxml, reply);
                    }
                    else
                    {
                        int i;
                        for (i = 0 ; i < reply.length ; i++)
                            fprintf(stderr, "%c", reply.data[i]);
                        fprintf(stderr, "\r\n\n");
                        fflush(stderr);
                    }
                    err = omadmclient_process_reply(session, &reply);
                    omadmclient_clean_buffer(&reply);
                }
            }
        } while (DMCLT_ERR_NONE == err && 200 == status);
    }
    else
    {
        FILE * fd;

        status = 200;

        fd = fopen(file, "r");
        if (!fd)
        {
            fprintf(stderr, "Can not open file %s\r\n", file);
            return 3;
        }
        reply.uri = NULL;
        reply.data = (unsigned char *)malloc(8000);
        reply.length = fread(reply.data, 1, 8000, fd);
        if (reply.length <= 0)
        {
            fprintf(stderr, "Can not read file %s\r\n", file);
            fclose(fd);
            return 3;
        }
        fclose(fd);

        // HACK for test: override status
        ((hack_internals_t *)session)->srv_auth = 212;
        ((hack_internals_t *)session)->clt_auth = 212;

        err = omadmclient_process_reply(session, &reply);
        omadmclient_clean_buffer(&reply);
        if (err != DMCLT_ERR_NONE)
        {
            fprintf(stderr, "SyncML parsing failed.\r\n", file);
            return 1;
        }
        err = omadmclient_get_next_packet(session, &buffer);
        if (DMCLT_ERR_NONE != err)
        {
            fprintf(stderr, "SyncML generation failed.\r\n", file);
            return 2;
        }
        output_buffer(stdout, isWbxml, buffer);
        omadmclient_clean_buffer(&buffer);
    }
    omadmclient_session_close(session);

    // check that we return 0 in case of success
    if (DMCLT_ERR_END == err) err = 0;
    else if (status != 200) err = status;

    return err;
}
