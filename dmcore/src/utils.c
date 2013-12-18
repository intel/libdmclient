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
 * @file utils.c
 *
 * @brief Various utility functions.
 *
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "internals.h"

SmlPcdataPtr_t convert_to_meta(char * format,
                               char * type)
{
    SmlPcdataPtr_t metaP = NULL;
    SmlMetInfMetInfPtr_t metInfP;

    if (format == NULL && type == NULL)
    {
        return NULL;
    }

    metInfP = smlAllocMetInfMetInf();
    if (!metInfP) return NULL;

    if (type) metInfP->type = smlString2Pcdata(type);
    if (format) metInfP->format = smlString2Pcdata(format);

    metaP = smlAllocPcdata();
    if (!metaP)
    {
        smlFreeMetinfMetinf(metInfP);
        return NULL;
    }
    metaP->contentType = SML_PCDATA_EXTENSION;
    metaP->extension = SML_EXT_METINF;
    metaP->length = 0;
    metaP->content = metInfP;

    return metaP;
}

void extract_from_meta(SmlPcdataPtr_t metaP,
                       char ** formatP,
                       char ** typeP)
{
    SmlMetInfMetInfPtr_t metInfP;

    if (formatP) *formatP = NULL;
    if (typeP) *typeP = NULL;
    if (!metaP
     || metaP->contentType != SML_PCDATA_EXTENSION
     || metaP->extension != SML_EXT_METINF)
    {
        return;
    }

    metInfP = (SmlMetInfMetInfPtr_t) metaP->content;

    if(formatP)
    {
        *formatP = smlPcdata2String(metInfP->format);
    }
    if (typeP)
    {
        *typeP = smlPcdata2String(metInfP->type);
    }
}

dmclt_authType_t get_from_chal_meta(SmlPcdataPtr_t metaP,
                                    buffer_t * nonceP)
{
    dmclt_authType_t type;
    SmlMetInfMetInfPtr_t metInfP;
    char * item;

    type = DMCLT_AUTH_TYPE_UNKNOWN;
    if (nonceP)
    {
        nonceP->buffer = NULL;
        nonceP->len = 0;
    }

    if (metaP->contentType != SML_PCDATA_EXTENSION
     || metaP->extension != SML_EXT_METINF)
    {
        return type;
    }

    metInfP = (SmlMetInfMetInfPtr_t) metaP->content;

    item = smlPcdata2String(metInfP->type);
    if (item)
    {
        type = auth_string_as_type(item);
        free(item);
    }

    if (nonceP)
    {
        item = smlPcdata2String(metInfP->nextnonce);
        if (item)
        {
            decode_b64(item, nonceP);
            free(item);
        }
    }

    return type;
}

SmlPcdataPtr_t create_chal_meta(dmclt_authType_t type,
                                buffer_t * nonceP)
{
    SmlMetInfMetInfPtr_t metInfP;
    SmlPcdataPtr_t metaP;

    metInfP = smlAllocMetInfMetInf();
    if (!metInfP) return NULL;

    metInfP->type = smlString2Pcdata(auth_type_as_string(type));
    metInfP->format = smlString2Pcdata("b64");
    if (nonceP)
    {
        char * tmp = encode_b64(*nonceP);
        metInfP->nextnonce = smlString2Pcdata(tmp);
        free(tmp);
    }

    metaP = smlAllocPcdata();
    if (!metaP)
    {
        smlFreeMetinfMetinf(metInfP);
        return NULL;
    }
    metaP->contentType = SML_PCDATA_EXTENSION;
    metaP->extension = SML_EXT_METINF;
    metaP->length = 0;
    metaP->content = metInfP;

    return metaP;
 }

void set_pcdata_string(SmlPcdataPtr_t dataP,
                       char * string)
{
    dataP->contentType = SML_PCDATA_STRING;
    dataP->length = strlen(string);
    dataP->content = (VoidPtr_t)strdup(string);
}

void set_pcdata_pcdata(SmlPcdataPtr_t dataP,
                       SmlPcdataPtr_t origP)
{
    dataP->contentType = origP->contentType;

    if (origP->length > 0)
    {
        if (origP->contentType == SML_PCDATA_STRING)
        {
            dataP->content = malloc(origP->length + 1);
        }
        else
        {
            dataP->content = malloc(origP->length);
        }
        if (dataP->content)
        {
            dataP->length = origP->length;
            memcpy(dataP->content, origP->content, origP->length);
            if (origP->contentType == SML_PCDATA_STRING)
            {
                ((char*)(dataP->content))[origP->length] = 0;
            }
        }
    }
}

void set_pcdata_int(SmlPcdataPtr_t dataP,
                    int value)
{
    char buffer[256];

    dataP->contentType = SML_PCDATA_STRING;
    dataP->length = sprintf(buffer, "%d", value);
    dataP->content = (VoidPtr_t)strdup(buffer);
}

void set_pcdata_hex(SmlPcdataPtr_t dataP,
                    int value)
{
    char buffer[256];

    dataP->contentType = SML_PCDATA_STRING;
    dataP->length = sprintf(buffer, "%X", value);
    dataP->content = (VoidPtr_t)strdup(buffer);
}

int pcdata_to_int(SmlPcdataPtr_t dataP)
{
    int result = 0;
    char * asString;

    asString = smlPcdata2String(dataP);
    if (asString)
    {
        sscanf(asString, "%d", &result);
        free(asString);
    }

    return result;
}

char * str_cat_2(const char * first,
                 const char * second)
{
    char * string;

    string = (char *)malloc(strlen(first) + strlen(second) + 1);
    if (string)
    {
        sprintf(string, "%s", first);
        strcat(string, second);
    }

    return string;
}

char * str_cat_3(const char * first,
                 const char * second,
                 const char * third)
{
    char * string;

    string = (char *)malloc(strlen(first) + strlen(second) + strlen(third) + 1);
    if (string)
    {
        sprintf(string, "%s", first);
        strcat(string, second);
        strcat(string, third);
    }

    return string;
}

char * str_cat_5 (const char * first,
                  const char * second,
                  const char * third,
                  const char * fourth,
                  const char * fifth)
{
    char * string;
    char * string_tmp;

    string = NULL;
    string_tmp = str_cat_3(first, second, third);
    if (string_tmp)
    {
        string = str_cat_3(string_tmp, fourth, fifth);
        free(string_tmp);
    }

    return string;
}

char ** strArray_concat(const char ** first,
                        const char ** second)
{
    char ** result;
    int firstCount = 0;
    int secondCount = 0;

    if (NULL == first && NULL == second)
    {
        return NULL;
    }
    if (NULL != first)
    {
        while(NULL != first[firstCount])
        {
            firstCount++;
        }
    }
    if (NULL != second)
    {
        while(NULL != second[secondCount])
        {
            secondCount++;
        }
    }

    result = (char **)malloc((firstCount + secondCount + 1) * sizeof(char*));
    if (NULL != result && NULL != first)
    {
        if (NULL == memcpy(result, first, firstCount * sizeof(char*)))
        {
            free(result);
            result = NULL;
        }
    }
    if (NULL != result && NULL != second)
    {
        if (NULL == memcpy(result + (firstCount * sizeof(char*)), second, secondCount * sizeof(char*)))
        {
            free(result);
            result = NULL;
        }
    }
    if (NULL != result)
    {
        result[firstCount + secondCount] = NULL;
    }

    return result;
}

void strArray_free(char ** array)
{
    int i;

    if (NULL == array)
    {
        return;
    }

    i = 0;
    while(NULL != array[i])
    {
        free(array[i]);
        i++;
    }
    free(array);
}

char ** strArray_add(const char ** array,
                     const char * newStr)
{
    const char * tmp[2];

    tmp[0] = newStr;
    tmp[1] = NULL;

    return strArray_concat(array, tmp);
}

char ** strArray_buildChildList(const char * iBaseUri,
                                const char * iChildList,
                                unsigned int iChildListLength)
{
    char ** result = NULL;
    int nb_child = 0;
    char * childName;
    char * listCopy = NULL;

    listCopy = (char *) malloc(iChildListLength + 1);
    if (NULL == listCopy) return NULL;
    memcpy(listCopy, iChildList, iChildListLength);
    listCopy[iChildListLength] = 0;

    childName = listCopy;
    while(childName && *childName)
    {
        nb_child++;
        childName = strchr(childName, '/');
        if (childName) childName += 1;
    }
    if (0 == nb_child) return NULL;

    result = (char**)malloc((nb_child + 1) * sizeof(char*));
    memset(result, 0, (nb_child + 1) * sizeof(char*));
    if (result)
    {
        nb_child = 0;
        childName = listCopy;
        while(childName && *childName)
        {
            char * slashStr;

            slashStr = strchr(childName, '/');
            if (slashStr)
            {
                *slashStr = 0;
                slashStr++;
            }

            result[nb_child] = str_cat_3(iBaseUri, "/", childName);
            if (NULL == result[nb_child])
            {
                strArray_free(result);
                return NULL;
            }
            nb_child++;
            childName = slashStr;
        }
    }

    free(listCopy);
    return result;
}

void set_new_uri(internals_t * internP,
                 char * uri)
{
    free(internP->account->server_uri);
    internP->account->server_uri = strdup(uri);
}

void add_element(internals_t * internP,
                 basicElement_t * elemP)
{
    elemCell_t * newCell;

    newCell = (elemCell_t *)malloc(sizeof(elemCell_t));
    if(newCell)
    {
        newCell->element = elemP;
        newCell->next = NULL;
        if (internP->elem_last)
        {
            internP->elem_last->next = newCell;
        }
        else
        {
            internP->elem_first = newCell;
        }
        internP->elem_last = newCell;
    }
}

void free_element(elemCell_t * cellP)
{
    smlFreeProtoElement(cellP->element);
    free(cellP);
}

void free_element_list(elemCell_t * cellP)
{
    elemCell_t * tmpCellP;

    while(cellP)
    {
        tmpCellP = cellP;
        cellP = cellP->next;
        free_element(tmpCellP);
    }
}

void refresh_elements(internals_t * internP)
{
    elemCell_t * cellP;

    cellP = internP->elem_first;
    while(cellP != NULL)
    {
        elemCell_t * nextCellP = cellP->next;

        if ((cellP->element->elementType != SML_PE_STATUS)
         && (cellP->element->elementType != SML_PE_RESULTS))
        {
            cellP->next = internP->old_elem;
            internP->old_elem = cellP;
        }
        else
        {
            free_element(cellP);
        }
        cellP = nextCellP;
    }

    internP->elem_first = NULL;
    internP->elem_last = NULL;
}

elemCell_t * retrieve_element(internals_t * internP,
                              char * cmdRef,
                              char * msgRef)
{
    elemCell_t * cellP;
    elemCell_t * prevCellP;
    int iMsgRef;
    int notFound;

    iMsgRef = -1;
    if (strlen(msgRef) != 0)
    {
        int tmp;
        if (1 == sscanf(msgRef, "%d", &tmp))
        {
            iMsgRef = tmp;
        }
    }

    cellP = internP->old_elem;
    prevCellP = NULL;
    notFound = 1;
    while (cellP && notFound)
    {
        if ((cellP->msg_id == iMsgRef) || (iMsgRef == -1))
        {
            char * elemId = smlPcdata2String(cellP->element->cmdID);
            if (elemId)
            {
                notFound = strcmp(elemId, cmdRef);
                free(elemId);
            }
        }
        if (notFound)
        {
            prevCellP = cellP;
            cellP = cellP->next;
        }
    }

    if (cellP)
    {
        if (prevCellP)
        {
            prevCellP->next = cellP->next;
        }
        else
        {
            internP->old_elem = cellP->next;
        }
        cellP->next = NULL;
    }

    return cellP;
}

void put_back_element(internals_t * internP,
                      elemCell_t * cellP)
{
    cellP->next = internP->old_elem;
    internP->old_elem = cellP;
}

char * proto_as_string(SmlProtoElement_t proto)
{
    switch(proto)
    {
    case SML_PE_ADD:
        return "Add";
    case SML_PE_ALERT:
        return "Alert";
    case SML_PE_ATOMIC_START:
        return "Atomic";
    case SML_PE_COPY:
        return "Copy";
    case SML_PE_DELETE:
        return "Delete";
    case SML_PE_EXEC:
        return "Exec";
    case SML_PE_PUT_GET:
    case SML_PE_GET:
        return "Get";
    case SML_PE_MAP:
        return "Map";
    case SML_PE_PUT:
        return "Put";
    case SML_PE_RESULTS:
        return "Result";
    case SML_PE_SEARCH:
        return "Search";
    case SML_PE_SEQUENCE_START:
    case SML_PE_SEQUENCE_END:
    case SML_PE_CMD_GROUP:
        return "Sequence";
    case SML_PE_STATUS:
        return "Status";
    case SML_PE_SYNC_START:
        return "Sync";
    case SML_PE_REPLACE:
        return "Replace";
    case SML_PE_HEADER:
        return "SyncHdr";
    case SML_PE_FINAL:
        return "Final";
    default:
        return "";
    }
}

SmlStatusPtr_t create_status(internals_t * internP,
                             int code,
                             SmlGenericCmdPtr_t pContent)
{
    SmlStatusPtr_t statusP;

    statusP = smlAllocStatus();

    // if pContent is nil, we are replying to an header
    if (pContent)
    {
        set_pcdata_pcdata(statusP->cmdRef, pContent->cmdID);
    }
    else
    {
        set_pcdata_string(statusP->cmdRef, "0");
    }
    set_pcdata_string(statusP->msgRef, internP->reply_ref);
    set_pcdata_string(statusP->cmd, proto_as_string(pContent?pContent->elementType:SML_PE_HEADER));
    set_pcdata_int(statusP->data, code);

    return statusP;
}

void add_target_ref(SmlStatusPtr_t statusP,
                    SmlTargetPtr_t target)
{
    if (!statusP || !target) return;

    statusP->targetRefList = smlAllocTargetRefList();
    if (statusP->targetRefList)
    {
        set_pcdata_pcdata(statusP->targetRefList->targetRef, target->locURI);
    }
}

void add_source_ref(SmlStatusPtr_t statusP,
                    SmlSourcePtr_t source)
{
    if (!statusP || !source) return;

    statusP->sourceRefList = smlAllocSourceRefList();
    if (statusP->sourceRefList)
    {
        set_pcdata_pcdata(statusP->sourceRefList->sourceRef, source->locURI);
    }
}

#define PRV_ALERT_OPTIONS_SEPARATOR     "&"
#define PRV_ALERT_OPTIONS_STRING_MINDT  "MINDT="
#define PRV_ALERT_OPTIONS_STRING_MAXDT  "MAXDT="
#define PRV_ALERT_OPTIONS_STRING_DFLT   "DR="
#define PRV_ALERT_OPTIONS_STRING_MAXLEN "MAXLEN="
#define PRV_ALERT_OPTIONS_STRING_INPUT  "IT="
#define PRV_ALERT_OPTIONS_STRING_ECHO   "ET="

static char * prv_find_value(char * buffer,
                             char * option)
{
    char * value;
    char * index;

    value = NULL;
    if (strlen(option) >= strlen(buffer))
    {
        return NULL;
    }
    index = strstr(buffer, option);
    if (index)
    {
        value = strdup(index + strlen(option));
        if (value)
        {
            index = strstr(value, PRV_ALERT_OPTIONS_SEPARATOR);
            if (index)
            {
                *index = 0;
            }
        }
    }

    return value;
}

#define PRV_ALERT_OPTIONS_VALUE_STRING_ECHO_TEXT   "T"
#define PRV_ALERT_OPTIONS_VALUE_STRING_ECHO_PASSWD "P"

#define PRV_ALERT_OPTIONS_VALUE_STRING_INPUT_ALPHA "A"
#define PRV_ALERT_OPTIONS_VALUE_STRING_INPUT_NUM   "N"
#define PRV_ALERT_OPTIONS_VALUE_STRING_INPUT_DATE  "D"
#define PRV_ALERT_OPTIONS_VALUE_STRING_INPUT_TIME  "T"
#define PRV_ALERT_OPTIONS_VALUE_STRING_INPUT_PHONE "P"
#define PRV_ALERT_OPTIONS_VALUE_STRING_INPUT_IP    "I"

static void prv_fill_alert_options(dmclt_ui_t * alertP,
                                   char * optStr)
{
    char * value;

    value = prv_find_value(optStr, PRV_ALERT_OPTIONS_STRING_MINDT);
    if (value)
    {
        sscanf(value, "%d", &(alertP->min_disp));
        free(value);
    }
    value = prv_find_value(optStr, PRV_ALERT_OPTIONS_STRING_MAXDT);
    if (value)
    {
        sscanf(value, "%d", &(alertP->max_disp));
        free(value);
    }
    value = prv_find_value(optStr, PRV_ALERT_OPTIONS_STRING_DFLT);
    if (value)
    {
        alertP->dflt_resp= value;
    }
    value = prv_find_value(optStr, PRV_ALERT_OPTIONS_STRING_MAXLEN);
    if (value)
    {
        sscanf(value, "%d", &(alertP->max_resp_len));
        free(value);
    }
    value = prv_find_value(optStr, PRV_ALERT_OPTIONS_STRING_INPUT);
    if (value)
    {
        if (!strcmp(value, PRV_ALERT_OPTIONS_VALUE_STRING_INPUT_ALPHA))
        {
            alertP->input_type = DMCLT_UI_INPUT_ALPHA;
        }
        else if (!strcmp(value, PRV_ALERT_OPTIONS_VALUE_STRING_INPUT_NUM))
        {
            alertP->input_type = DMCLT_UI_INPUT_NUM;
        }
        else if (!strcmp(value, PRV_ALERT_OPTIONS_VALUE_STRING_INPUT_DATE))
        {
            alertP->input_type = DMCLT_UI_INPUT_DATE;
        }
        else if (!strcmp(value, PRV_ALERT_OPTIONS_VALUE_STRING_INPUT_TIME))
        {
            alertP->input_type = DMCLT_UI_INPUT_TIME;
        }
        else if (!strcmp(value, PRV_ALERT_OPTIONS_VALUE_STRING_INPUT_PHONE))
        {
            alertP->input_type = DMCLT_UI_INPUT_PHONE;
        }
        else if (!strcmp(value, PRV_ALERT_OPTIONS_VALUE_STRING_INPUT_IP))
        {
            alertP->input_type = DMCLT_UI_INPUT_IP;
        }

        free(value);
    }
    value = prv_find_value(optStr, PRV_ALERT_OPTIONS_STRING_ECHO);
    if (value)
    {
        if (!strcmp(value, PRV_ALERT_OPTIONS_VALUE_STRING_ECHO_TEXT))
        {
            alertP->echo_type = DMCLT_UI_ECHO_TEXT;
        }
        else if (!strcmp(value, PRV_ALERT_OPTIONS_VALUE_STRING_ECHO_PASSWD))
        {
            alertP->echo_type = DMCLT_UI_ECHO_PASSWD;
        }

        free(value);
    }
}

void prv_get_alert_choices(dmclt_ui_t * alertP,
                           SmlItemListPtr_t itemCell)
{
    SmlItemListPtr_t cellP;
    int count;

    count = 0;
    cellP = itemCell;
    while (cellP)
    {
        cellP = cellP->next;
        count++;
    }
    if (0 == count) return;

    alertP->choices = (char**)malloc((count + 1)*sizeof(char*));
    if (!alertP->choices) goto error;

    count = 0;
    while (itemCell)
    {
        if (!itemCell->item) goto error;
        alertP->choices[count] = smlPcdata2String(itemCell->item->data);
        count++;
        itemCell =itemCell->next;
    }
    alertP->choices[count] = NULL;

    return;

error:
    alertP->type = 0;
}

dmclt_ui_t * get_ui_from_sml(SmlAlertPtr_t smlAlertP)
{
    dmclt_ui_t * alertP;
    char * dataStr;
    SmlItemListPtr_t itemCell;

    if (!smlAlertP || !smlAlertP->data) return NULL;

    alertP = (dmclt_ui_t *)malloc(sizeof(dmclt_ui_t));
    if (!alertP) return NULL;
    memset(alertP, 0, sizeof(dmclt_ui_t));

    itemCell = smlAlertP->itemList;
    if (!itemCell) goto end;

    // first item contains options
    if (NULL == itemCell->item) goto end;
    dataStr = smlPcdata2String(itemCell->item->data);
    prv_fill_alert_options(alertP, dataStr);
    if (dataStr) free(dataStr);

    // second item contains display message
    itemCell = itemCell->next;
    if (NULL == itemCell || NULL == itemCell->item) goto end;
    alertP->disp_msg = smlPcdata2String(itemCell->item->data);
    itemCell = itemCell->next;

    dataStr = smlPcdata2String(smlAlertP->data);
    if (NULL == dataStr)
    {
        goto end;
    }
    if (0 == strcmp(dataStr, PRV_ALERT_STRING_DISPLAY))
    {
        alertP->type = DMCLT_UI_TYPE_DISPLAY;
    }
    else if (0 == strcmp(dataStr, PRV_ALERT_STRING_CONFIRM))
    {
        alertP->type = DMCLT_UI_TYPE_CONFIRM;
    }
    else if (0 == strcmp(dataStr, PRV_ALERT_STRING_USER_INPUT))
    {
        alertP->type = DMCLT_UI_TYPE_USER_INPUT;
        alertP->max_resp_len = alertP->max_resp_len?alertP->max_resp_len:PRV_USER_RESP_MAX_LEN;
    }
    else if (0 == strcmp(dataStr, PRV_ALERT_STRING_USER_CHOICE))
    {
        alertP->type = DMCLT_UI_TYPE_USER_CHOICE;
        prv_get_alert_choices(alertP, itemCell);
        alertP->max_resp_len = alertP->max_resp_len?alertP->max_resp_len:PRV_USER_RESP_MAX_LEN;
    }
    else if (0 == strcmp(dataStr, PRV_ALERT_STRING_USER_MULTICHOICE))
    {
        alertP->type = DMCLT_UI_TYPE_USER_MULTICHOICE;
        prv_get_alert_choices(alertP, itemCell);
        alertP->max_resp_len = alertP->max_resp_len?alertP->max_resp_len:PRV_USER_RESP_MAX_LEN;
    }
    else
    {
        goto end;
    }

end:
    if (0 == alertP->type)
    {
        free_dmclt_alert(alertP);
        alertP = NULL;
    }
    if (dataStr) free(dataStr);

    return alertP;
}

void free_dmclt_alert(dmclt_ui_t * alertP)
{
    if (alertP)
    {
        if (alertP->disp_msg) free(alertP->disp_msg);
        if (alertP->choices)
        {
            int i = 0;
            while(alertP->choices[i])
            {
                free(alertP->choices[i]);
                i++;
            }
            free(alertP->choices);
        }
        free(alertP);
    }
}

char * dmtree_node_as_string(dmtree_node_t * node)
{
    char * result;

    result = (char *)malloc(node->data_size + 1);
    if (NULL != result)
    {
        memcpy(result, node->data_buffer, node->data_size);
        result[node->data_size] = 0;
    }

    return result;
}

void dmtree_node_clean(dmtree_node_t *node,
                       bool full)
{
    if (node->uri)
        free(node->uri);

    if (node->format)
        free(node->format);

    if (node->type)
        free(node->type);

    // we free data_buffer manually since most of the time it
    // is allocated by the SyncMLRTK
    if (full && node->data_buffer)
        free(node->data_buffer);

    memset(node, 0, sizeof(dmtree_node_t));
}

void dmtree_node_free(dmtree_node_t *node)
{
    if (node == NULL)
        return;

    dmtree_node_clean(node, true);

    free(node);
}

dmtree_node_t * dmtree_node_dup(const dmtree_node_t * src)
{
    dmtree_node_t * dest;

    if (NULL == src)
    {
        return NULL;
    }

    dest = (dmtree_node_t *)malloc(sizeof(dmtree_node_t));
    if (NULL == dest)
    {
        return NULL;
    }
    memset(dest, 0, sizeof(dmtree_node_t));

    if (src->uri)
    {
        dest->uri = strdup(src->uri);
        if (NULL == dest->uri) goto on_error;
    }
    if (src->format)
    {
        dest->format = strdup(src->format);
        if (NULL == dest->format) goto on_error;
    }
    if (src->type)
    {
        dest->type = strdup(src->type);
        if (NULL == dest->type) goto on_error;
    }
    if (src->data_size && src->data_buffer)
    {
        dest->data_size = src->data_size;
        dest->data_buffer = (char *)malloc(dest->data_size);
        if (NULL == dest->data_buffer) goto on_error;
        memcpy(dest->data_buffer, src->data_buffer, dest->data_size);
    }

    return dest;

on_error:
    dmtree_node_free(dest);
    return NULL;
}

dmtree_node_t * dmtree_node_copy(dmtree_node_t * dest,
                                 const dmtree_node_t * src)
{
    dmtree_node_t * tmp;

    if (NULL == dest)
    {
        return NULL;
    }
    if (NULL == src)
    {
        return dest;
    }

    tmp = dmtree_node_dup(src);
    if (NULL == tmp)
    {
        return NULL;
    }

    if (NULL == dest->uri)
    {
        dest->uri = tmp->uri;
        tmp->uri = NULL;
    }
    if (NULL == dest->type)
    {
        dest->type = tmp->type;
        tmp->type = NULL;
    }
    if (NULL == dest->format)
    {
        dest->format = tmp->format;
        tmp->format = NULL;
    }
    if (NULL == dest->data_buffer)
    {
        dest->data_size = tmp->data_size;
        dest->data_buffer = tmp->data_buffer;
        tmp->data_buffer = NULL;
    }

    dmtree_node_free(tmp);
    return dest;
}
