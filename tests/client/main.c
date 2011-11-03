#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <omadmclient.h>
#include <libsoup/soup.h>
#include <gio/gio.h>

typedef struct
{
    char * id;
    char * name;
    char * uri;
    void * toServerCred;
    void * toClientCred;
} accountDesc_t;

typedef struct
{
    void *        smlH;
    void *      dmtreeH;
    int                 session_id;
    int                 message_id;
    int                 command_id;
    void *       elem_first;
    void *       elem_last;
    void *       old_elem;
    char *              reply_ref;
    accountDesc_t *    account;
    int                 srv_auth;
    int                 clt_auth;
    dmclt_callback_t alert_cb;
    void *           cb_data;
} * internals_p;

void print_usage(void)
{
    fprintf(stderr, "Usage: testdmclient [-w] [-d] [-f FILE|-u URI]\r\n");
    fprintf(stderr, "Launch a DM session with the server \"funambol\" on localhost.\r\n\n");
    fprintf(stderr, "  -w\tuse WBXML\r\n");
    fprintf(stderr, "  -d\tuse Dbus calls to display UI\r\n");
    fprintf(stderr, "  -f\tuse the file as a server's packet\r\n");
    fprintf(stderr, "  -u\tuse the URI to connect the server\r\n");
    fprintf(stderr, "Options f and u are mutually exclusive.\r\n");

}

void output_buffer(FILE * fd, int flags, dmclt_buffer_t buffer)
{
    int i;

    if (flags&DMCLT_FLAG_WBXML)
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

int uiCallbackDBus(void * userData,
                   const dmclt_ui_t * alertData,
                   char * userReply)
{
    int code = 200;
    GDBusProxy * proxyP = NULL;
    GError * error = NULL;
    GVariant * resultP = NULL;

    fprintf(stderr, "\nAlert received:\n");
    fprintf(stderr, "type: %d\n", alertData->type);
    fprintf(stderr, "min_disp: %d\n", alertData->min_disp);
    fprintf(stderr, "max_disp: %d\n", alertData->max_disp);
    fprintf(stderr, "max_resp_len: %d\n", alertData->max_resp_len);
    fprintf(stderr, "input_type: %d\n", alertData->input_type);
    fprintf(stderr, "echo_type: %d\n", alertData->echo_type);
    fprintf(stderr, "disp_msg: \"%s\"\n", alertData->disp_msg);
    fprintf(stderr, "dflt_resp: \"%s\"\n", alertData->dflt_resp);

#define DMUI_SERVER_NAME "com.intel.roman.ui"
#define DMUI_OBJECT "/com/intel/roman/ui"
#define DMUI_INTERFACE "com.intel.roman.ui.omadm"

    proxyP = g_dbus_proxy_new_for_bus_sync(G_BUS_TYPE_SESSION,
                                           0,
                                           NULL,
                                           DMUI_SERVER_NAME,
                                           DMUI_OBJECT,
                                           DMUI_INTERFACE,
                                           NULL,
                                           &error);

    if (!proxyP)
    {
        if (error)
        {
            fprintf(stderr, "g_dbus_proxy_new_for_bus_sync() failed (%d) \"%s\"\n", error->code, error->message);
            g_error_free(error);
        }
        return 500;
    }

    switch (alertData->type)
    {
    case DMCLT_UI_TYPE_DISPLAY:
        resultP = g_dbus_proxy_call_sync(proxyP,
                                         "Display",
                                         g_variant_new ("(sii)",
                                                        alertData->disp_msg,
                                                        alertData->min_disp, alertData->max_disp),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1, NULL,
                                         &error);
        if (resultP == NULL)
        {
            if (error)
            {
                fprintf(stderr, "g_dbus_proxy_call_sync(Display) failed (%d) \"%s\"\n", error->code, error->message);
                g_error_free(error);
            }
            code = 500;
        }
        else
        {
            g_variant_unref(resultP);
        }
        break;

    case DMCLT_UI_TYPE_CONFIRM:
        resultP = g_dbus_proxy_call_sync(proxyP,
                                         "Confirm",
                                         g_variant_new ("(sii)",
                                                        alertData->disp_msg,
                                                        alertData->min_disp, alertData->max_disp),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1, NULL,
                                         &error);
        if (resultP == NULL)
        {
            if (error)
            {
                fprintf(stderr, "g_dbus_proxy_call_sync(Confirm) failed (%d) \"%s\"\n", error->code, error->message);
                g_error_free(error);
            }
            code = 500;
        }
        else
        {
            g_variant_get(resultP, "(i)", &code);
            g_variant_unref(resultP);
        }
        break;

    case DMCLT_UI_TYPE_USER_INPUT:
        resultP = g_dbus_proxy_call_sync(proxyP,
                                         "UserInput",
                                         g_variant_new ("(siiiibs)",
                                                        alertData->disp_msg,
                                                        alertData->min_disp, alertData->max_disp,
                                                        alertData->max_resp_len, alertData->input_type,
                                                        alertData->echo_type, alertData->dflt_resp),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1, NULL,
                                         &error);
        if (resultP == NULL)
        {
            if (error)
            {
                fprintf(stderr, "g_dbus_proxy_call_sync(Confirm) failed (%d) \"%s\"\n", error->code, error->message);
                g_error_free(error);
            }
            code = 500;
        }
        else
        {
            gchar * reply;
            g_variant_get(resultP, "(i&s)", &code, &reply);
            strncpy(userReply, reply, alertData->max_resp_len);
            userReply[alertData->max_resp_len-1] = 0;
            g_variant_unref(resultP);
        }
        break;

    case DMCLT_UI_TYPE_USER_CHOICE:
    case DMCLT_UI_TYPE_USER_MULTICHOICE:
    {
        GVariantBuilder *builderP;
        int i;

        builderP = g_variant_builder_new(G_VARIANT_TYPE("as"));
        i = 0;
        while(alertData->choices[i])
        {
            g_variant_builder_add(builderP, "s", alertData->choices[i]);
            i++;
        }

        resultP = g_dbus_proxy_call_sync(proxyP,
                                         "UserChoice",
                                         g_variant_new ("(siib(as)s)",
                                                        alertData->disp_msg,
                                                        alertData->min_disp, alertData->max_disp,
                                                        alertData->type == DMCLT_UI_TYPE_USER_MULTICHOICE,
                                                        builderP,
                                                        alertData->dflt_resp),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1, NULL,
                                         &error);
        g_variant_builder_unref(builderP);
        if (resultP == NULL)
        {
            if (error)
            {
                fprintf(stderr, "g_dbus_proxy_call_sync(Confirm) failed (%d) \"%s\"\n", error->code, error->message);
                g_error_free(error);
            }
            code = 500;
        }
        else
        {
            gchar * reply;
            g_variant_get(resultP, "(i&s)", &code, &reply);
            strncpy(userReply, reply, alertData->max_resp_len);
            userReply[alertData->max_resp_len-1] = 0;
            g_variant_unref(resultP);
        }
    }
    break;

    default:
        break;
    }

    g_object_unref(proxyP);
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
    char flags;
    int err;
    int status;
    SoupSession * soupH;
    char * url = NULL;
    char * file = NULL;
    bool dbus = false;

    g_type_init();
    soupH = soup_session_sync_new();

    flags = DMCLT_FLAG_CLIENT_INIT;
    url = NULL;
    file = NULL;
    opterr = 0;

    while ((c = getopt (argc, argv, "wdu:f:")) != -1)
    {
        switch (c)
        {
        case 'w':
            flags |= DMCLT_FLAG_WBXML;
            break;
        case 'd':
            dbus = true;
            break;
        case 'u':
            url = optarg;
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

    if (url && file)
    {
        print_usage();
        return 1;
    }

    err = omadmclient_session_open(&session,
                                   "funambol",
                                   1,
                                   &flags,
                                   dbus?uiCallbackDBus:uiCallback,
                                   NULL);
    if (err != DMCLT_ERR_NONE)
    {
        fprintf(stderr, "Initialization failed: %d\r\n", err);
        return err;
    }
    if (url)
    {
        // override url
        free(((internals_p)session)->account->uri);
        ((internals_p)session)->account->uri = strdup(url);
    }
    if (!file)
    {
        do
        {
            err = omadmclient_get_next_packet(session, &buffer);
            if (DMCLT_ERR_NONE == err)
            {
                output_buffer(stderr, flags, buffer);
                status = sendPacket(soupH, flags&DMCLT_FLAG_WBXML?"application/vnd.syncml+wbxml":"application/vnd.syncml+xml", &buffer, &reply);
                fprintf(stderr, "Reply from \"%s\": %d\r\n\n", buffer.uri, status);

                omadmclient_clean_buffer(&buffer);

                if (200 == status)
                {
                    if (flags&DMCLT_FLAG_WBXML)
                    {
                        output_buffer(stderr, flags, reply);
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

        // override status
        ((internals_p)session)->srv_auth = 212;
        ((internals_p)session)->clt_auth = 212;

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
        output_buffer(stdout, flags, buffer);
        omadmclient_clean_buffer(&buffer);
    }
    omadmclient_session_close(session);

    // check that we return 0 in case of success
    if (DMCLT_ERR_END == err) err = 0;
    else if (status != 200) err = status;

    return err;
}
