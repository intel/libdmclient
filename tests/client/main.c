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
#include <dlfcn.h>
#include <dirent.h>
#include <stdbool.h>
#include <omadmclient.h>
#include <curl/curl.h>
#include <config.h>

#define MAX_PLUGIN 16

dmclt_session g_session = NULL;

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

static size_t storeReplyCallback(void * contents,
                                 size_t size,
                                 size_t nmemb,
                                 void * userp)
{
    size_t total = size * nmemb;
    dmclt_buffer_t * reply = (dmclt_buffer_t *)userp;
 
    reply->data = realloc(reply->data, reply->length + total);
    if (reply->data == NULL)
    {
        fprintf(stderr, "Not enough memory\r\n");
        exit(EXIT_FAILURE);
    }
 
    memcpy(&(reply->data[reply->length]), contents, total);
    reply->length += total;
 
    return total;
}

long sendPacket(CURL * curlH,
                char * type,
                dmclt_buffer_t * packet,
                dmclt_buffer_t * reply)
{
    struct curl_slist * headerList = NULL;
    long status = 503;

    memset(reply, 0, sizeof(dmclt_buffer_t));
    if (NULL == curlH)
    {
        return status;
    }

    curl_easy_setopt(curlH, CURLOPT_URL, packet->uri);
    curl_easy_setopt(curlH, CURLOPT_POST, 1);
    headerList = curl_slist_append(headerList, type);
    curl_easy_setopt(curlH, CURLOPT_HTTPHEADER, headerList);
    curl_easy_setopt(curlH, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)(packet->length));
    curl_easy_setopt(curlH, CURLOPT_COPYPOSTFIELDS, (void *)(packet->data));

    curl_easy_setopt(curlH, CURLOPT_WRITEFUNCTION, storeReplyCallback);
    curl_easy_setopt(curlH, CURLOPT_WRITEDATA, (void *)reply); 

    if (CURLE_OK == curl_easy_perform(curlH))
    {
         if (CURLE_OK != curl_easy_getinfo(curlH, CURLINFO_RESPONSE_CODE, &status))
         {
              status = 503;
         }
    }
    curl_slist_free_all(headerList);

    return status;
}

void LoadPlugins(dmclt_session session,
				 void * pluginHandles[MAX_PLUGIN])
{
    DIR *folderP;
    int i;

    i = 0;

    // load plugins from the plugins directory
    folderP = opendir(MO_INSTALL_DIR);
    if (folderP != NULL)
    {
        struct dirent *fileP;

        while ((fileP = readdir(folderP)) && (i < MAX_PLUGIN))
        {
            if (DT_REG == fileP->d_type)
            {
                char * filename;
                void * handle = NULL;
                omadm_mo_interface_t * moInterfaceP = NULL;
                omadm_mo_interface_t * (*getMoIfaceF)();

                filename = (char *)malloc(strlen(MO_INSTALL_DIR) + 1 + strlen(fileP->d_name) + 1);
                if (filename)
                {
                    sprintf(filename, "%s", MO_INSTALL_DIR);
                    strcat(filename, "/");
                    strcat(filename, fileP->d_name);

					handle = dlopen(filename, RTLD_LAZY);
					if (handle)
					{
						getMoIfaceF = dlsym(handle, "omadm_get_mo_interface");
						if (getMoIfaceF)
						{
							moInterfaceP = getMoIfaceF();
							if (moInterfaceP && (moInterfaceP->base_uri))
							{
								if (DMCLT_ERR_NONE == omadmclient_session_add_mo(session, moInterfaceP))
								{
									// store handle
									pluginHandles[i] = handle;
									i++;
									handle = NULL;
								}
								else
								{
									free(moInterfaceP);
								}
							}
						}

						if (handle) dlclose (handle);
					}
					free(filename);
                }
            }
        }
        closedir(folderP);
    }
}

dmclt_session get_dm_session()
{
    return g_session;
}

int main(int argc, char *argv[])
{
    dmclt_buffer_t buffer;
    dmclt_buffer_t reply;
    int c;
    bool isWbxml = false;
    int err;
    long status = 200;
    CURL * curlH;
    char * server = NULL;
    char * file = NULL;
    omadm_mo_interface_t * testMoP;
    char * proxyStr;
    void * pluginHandles[MAX_PLUGIN];

    server = NULL;
    file = NULL;
    opterr = 0;
    memset(pluginHandles, 0, MAX_PLUGIN * sizeof(void*));

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

    g_session = omadmclient_session_init(isWbxml);
    if (g_session == NULL)
    {
        fprintf(stderr, "Initialization failed\r\n");
        return 1;
    }
    err = omadmclient_set_UI_callback(g_session, uiCallback, NULL);
    if (err != DMCLT_ERR_NONE)
    {
        fprintf(stderr, "Initialization failed: %d\r\n", err);
        return err;
    }

    testMoP = test_get_mo_interface();
    if (testMoP)
    {
        err = omadmclient_session_add_mo(g_session, testMoP);
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

    LoadPlugins(g_session, pluginHandles);

    err = omadmclient_session_start(g_session,
                                    server?server:"funambol",
                                    1);
    if (err != DMCLT_ERR_NONE)
    {
        fprintf(stderr, "Session opening to \"%s\" failed: %d\r\n", server?server:"funambol", err);
        return err;
    }
    
    if (!file)
    {
        curlH = curl_easy_init();

        do
        {
            err = omadmclient_get_next_packet(g_session, &buffer);
            if (DMCLT_ERR_NONE == err)
            {
                output_buffer(stderr, isWbxml, buffer);
                switch (buffer.auth_type)
                {
                case DMCLT_AUTH_TYPE_HTTP_BASIC:
                case DMCLT_AUTH_TYPE_HTTP_DIGEST:
                    // establish HTTPS session
                    break;
                case DMCLT_AUTH_TYPE_BASIC:
                case DMCLT_AUTH_TYPE_DIGEST:
                    // do nothing
                    break;
                case DMCLT_AUTH_TYPE_HMAC:
                    // compute HMAC of the message and add it to the transport header
                    break;
                case DMCLT_AUTH_TYPE_X509:
                case DMCLT_AUTH_TYPE_SECURID:
                case DMCLT_AUTH_TYPE_SAFEWORD:
                case DMCLT_AUTH_TYPE_DIGIPASS:
                case DMCLT_AUTH_TYPE_TRANSPORT:
                default:
                    break;
                }
                status = sendPacket(curlH, isWbxml?"Content-Type: application/vnd.syncml+wbxml":"Content-Type: application/vnd.syncml+xml", &buffer, &reply);
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
                    err = omadmclient_process_reply(g_session, &reply);
                    omadmclient_clean_buffer(&reply);
                }
            }
        } while (DMCLT_ERR_NONE == err && 200 == status);

        curl_easy_cleanup(curlH);
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
        ((hack_internals_t *)g_session)->srv_auth = 212;
        ((hack_internals_t *)g_session)->clt_auth = 212;

        err = omadmclient_process_reply(g_session, &reply);
        omadmclient_clean_buffer(&reply);
        if (err != DMCLT_ERR_NONE)
        {
            fprintf(stderr, "SyncML parsing failed.\r\n", file);
            return 1;
        }
        err = omadmclient_get_next_packet(g_session, &buffer);
        if (DMCLT_ERR_NONE != err)
        {
            fprintf(stderr, "SyncML generation failed.\r\n", file);
            return 2;
        }
        output_buffer(stdout, isWbxml, buffer);
        omadmclient_clean_buffer(&buffer);
    }
    omadmclient_session_close(g_session);

    c = 0;
   	while ((c < MAX_PLUGIN) && (pluginHandles[c] != 0))
    {
        dlclose(pluginHandles[c]);
        c++;
    }

    // check that we return 0 in case of success
    if (DMCLT_ERR_END == err) err = 0;
    else if (status != 200) err = status;

    return err;
}
