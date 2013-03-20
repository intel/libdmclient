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
 * @file callbacks.c
 *
 * @brief Callbacks for the SyncMLRTK.
 *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "internals.h"

static Ret_t prv_do_generic_cmd_cb(InstanceID_t id,
                                   VoidPtr_t userData,
                                   SmlGenericCmdPtr_t cmdP)
{
    internals_t * internP = (internals_t *)userData;
    SmlStatusPtr_t statusP;
    SmlItemListPtr_t itemCell;

    if (internP->sequence
      && internP->seq_code != OMADM_SYNCML_ERROR_NOT_MODIFIED
      && internP->seq_code != OMADM_SYNCML_ERROR_SUCCESS)
    {
        // do not treat this command
        return SML_ERR_OK;
    }

    if (OMADM_SYNCML_ERROR_AUTHENTICATION_ACCEPTED != internP->srv_auth)
    {
        statusP = create_status(internP, internP->srv_auth, cmdP);

        add_element(internP, (basicElement_t *)statusP);
        return SML_ERR_OK;
    }

    itemCell = cmdP->itemList;
    while (itemCell)
    {
        int code;

        if (internP->sequence && internP->seq_code == OMADM_SYNCML_ERROR_NOT_MODIFIED)
        {
            code = OMADM_SYNCML_ERROR_NOT_EXECUTED;
        }
        else
        {
            switch (cmdP->elementType)
            {
            case SML_PE_ADD:
                code = add_node(internP, itemCell->item);
                break;
            case SML_PE_COPY:
                code = copy_node(internP, itemCell->item);
                break;
            case SML_PE_DELETE:
                code = delete_node(internP, itemCell->item);
                break;
            case SML_PE_REPLACE:
                 code = replace_node(internP, itemCell->item);
                 break;
            case SML_PE_EXEC:
                 code = exec_node(internP, itemCell->item);
                 break;
            default:
                code = OMADM_SYNCML_ERROR_COMMAND_NOT_IMPLEMENTED;
            }
        }
        statusP = create_status(internP, code, cmdP);
        add_target_ref(statusP, itemCell->item->target);
        add_element(internP, (basicElement_t *)statusP);

        itemCell = itemCell->next;
    }

    return SML_ERR_OK;
}

static Ret_t prv_start_message_cb(InstanceID_t id,
                                  VoidPtr_t userData,
                                  SmlSyncHdrPtr_t headerP)
{
    internals_t * internP = (internals_t *)userData;
    SmlStatusPtr_t statusP;
    SmlChalPtr_t challengeP = NULL;
    char * dataStr;

    if (internP->reply_ref)
    {
        free(internP->reply_ref);
    }
    internP->sequence = NULL;
    internP->seq_code = 0;

    internP->reply_ref = smlPcdata2String(headerP->msgID);

    if (headerP->cred)
    {
        internP->srv_auth = check_credentials(headerP->cred, internP->account->toClientCred);
        challengeP= get_challenge(internP->account->toClientCred);
        store_nonce(internP->dmtreeH->MOs, internP->account, false);
    }

    dataStr = smlPcdata2String(headerP->respURI);
    if (dataStr)
    {
        set_new_uri(internP, dataStr);
        free(dataStr);
    }

    statusP = create_status(internP, internP->srv_auth, NULL);
    statusP->chal = challengeP;
    add_target_ref(statusP, headerP->target);
    add_source_ref(statusP, headerP->source);

    add_element(internP, (basicElement_t *)statusP);

    return SML_ERR_OK;
}

static Ret_t prv_end_message_cb(InstanceID_t id,
                                VoidPtr_t userData,
                                Boolean_t final)
{
    return SML_ERR_OK;
}

static Ret_t prv_start_atomic_cb(InstanceID_t id,
                                 VoidPtr_t userData,
                                 SmlAtomicPtr_t atomicP)
{
    internals_t * internP = (internals_t *)userData;
    SmlStatusPtr_t statusP;

    if (OMADM_SYNCML_ERROR_AUTHENTICATION_ACCEPTED != internP->srv_auth)
    {
        statusP = create_status(internP, internP->srv_auth, (SmlGenericCmdPtr_t)atomicP);

        add_element(internP, (basicElement_t *)statusP);
        return SML_ERR_OK;
    }


    return SML_ERR_OK;
}

static Ret_t prv_end_atomic_cb(InstanceID_t id,
                               VoidPtr_t userData)
{
    return SML_ERR_OK;
}

static Ret_t prv_start_sequence_cb(InstanceID_t id,
                                   VoidPtr_t userData,
                                   SmlSequencePtr_t sequenceP)
{
    internals_t * internP = (internals_t *)userData;

    internP->sequence = sequenceP;
    if (internP->srv_auth == OMADM_SYNCML_ERROR_AUTHENTICATION_ACCEPTED)
    {
        internP->seq_code = OMADM_SYNCML_ERROR_SUCCESS;
    }
    else
    {
        internP->seq_code = internP->srv_auth;
    }

    return SML_ERR_OK;
}

static Ret_t prv_end_sequence_cb (InstanceID_t id,
                                  VoidPtr_t userData)
{
    internals_t * internP = (internals_t *)userData;
    SmlStatusPtr_t statusP;

    statusP = create_status(internP, internP->seq_code, (SmlGenericCmdPtr_t)(internP->sequence));
    add_element(internP, (basicElement_t *)statusP);

    internP->sequence = NULL;
    internP->seq_code = 0;

    return SML_ERR_OK;
}

static Ret_t prv_alert_cmd_cb(InstanceID_t id,
                              VoidPtr_t userData,
                              SmlAlertPtr_t alertP)
{
    internals_t * internP = (internals_t *)userData;
    SmlStatusPtr_t statusP;
    int code;
    dmclt_ui_t * dmcAlertP;
    char * answer;

    dmcAlertP = NULL;
    answer = NULL;

    if (internP->sequence)
    {
        switch (internP->seq_code)
        {
        case OMADM_SYNCML_ERROR_NOT_MODIFIED:
            // user aborted sequence
            code = OMADM_SYNCML_ERROR_NOT_EXECUTED;
            goto end;
        case OMADM_SYNCML_ERROR_SUCCESS:
            // everything is fine
            break;
        default:
            // do not treat this command
            return SML_ERR_OK;
        }
    }

    if (OMADM_SYNCML_ERROR_AUTHENTICATION_ACCEPTED != internP->srv_auth)
    {
        code = internP->srv_auth;
        goto end;
    }

    if (NULL == internP->alert_cb)
    {
        code = OMADM_SYNCML_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED;
        goto end;
    }

    dmcAlertP = get_ui_from_sml(alertP);
    if (NULL == dmcAlertP)
    {
        code  = OMADM_SYNCML_ERROR_COMMAND_FAILED;
        goto end;
    }
//TODO: check for UTF8
    answer = (char *) malloc(dmcAlertP->max_resp_len + 1);
    if (NULL == answer)
    {
        code  = OMADM_SYNCML_ERROR_COMMAND_FAILED;
        goto end;
    }

    code = internP->alert_cb(internP->cb_data, dmcAlertP, answer);

    if (internP->sequence && dmcAlertP->type == DMCLT_UI_TYPE_CONFIRM)
    {
        internP->seq_code = code;
    }

end:
    // the SmlAlertPtr_t can be cast as a SmlGenericCmdPtr_t since we only
    // need elementType and cmdID in prvCreateStatus()
    statusP = create_status(internP, code, (SmlGenericCmdPtr_t)alertP);

    if ((code == OMADM_SYNCML_ERROR_SUCCESS)
        && (dmcAlertP->type >= DMCLT_UI_TYPE_USER_INPUT))
    {
        statusP->itemList = smlAllocItemList();
        if (statusP->itemList)
        {
            statusP->itemList->item->data = smlString2Pcdata(answer?answer:"");
        }
    }

    add_element(internP, (basicElement_t *)statusP);

    if (answer) free(answer);
    if (dmcAlertP) free_dmclt_alert(dmcAlertP);

    return SML_ERR_OK;
}

static Ret_t prv_get_cmd_cb(InstanceID_t id,
                            VoidPtr_t userData,
                            SmlGetPtr_t getP)
{
    internals_t * internP = (internals_t *)userData;
    SmlStatusPtr_t statusP;
    SmlItemListPtr_t itemCell;
    SmlItemListPtr_t resultLastCell;
    SmlResultsPtr_t resultP;

    if (internP->sequence
      && internP->seq_code != OMADM_SYNCML_ERROR_NOT_MODIFIED
      && internP->seq_code != OMADM_SYNCML_ERROR_SUCCESS)
    {
        // do not treat this command
        return SML_ERR_OK;
    }

    if (OMADM_SYNCML_ERROR_AUTHENTICATION_ACCEPTED != internP->srv_auth)
    {
        statusP = create_status(internP, internP->srv_auth, (SmlGenericCmdPtr_t)getP);

        add_element(internP, (basicElement_t *)statusP);
        return SML_ERR_OK;
    }

    resultP = smlAllocResults();
    if (!resultP) return SML_ERR_NOT_ENOUGH_SPACE;
    set_pcdata_pcdata(resultP->cmdRef, getP->cmdID);
    resultP->msgRef = smlString2Pcdata(internP->reply_ref);
    smlFreeItemList(resultP->itemList);
    resultP->itemList = NULL;

    resultLastCell = NULL;
    itemCell = getP->itemList;
    while (itemCell)
    {
        int code;

        if (internP->sequence && internP->seq_code == OMADM_SYNCML_ERROR_NOT_MODIFIED)
        {
            code = OMADM_SYNCML_ERROR_NOT_EXECUTED;
        }
        else
        {
            SmlItemListPtr_t newCell;

            newCell = smlAllocItemList();
            if (!newCell)
            {
                code = OMADM_SYNCML_ERROR_COMMAND_FAILED;
            }
            else
            {
                code = get_node(internP, itemCell->item, newCell->item);
            }
            if (code == OMADM_SYNCML_ERROR_SUCCESS)
            {
                if (resultLastCell)
                {
                    resultLastCell->next = newCell;
                }
                else
                {
                    resultP->itemList = newCell;
                }
                resultLastCell = newCell;
            }
            else
            {
                smlFreeItemList(newCell);
            }
        }
        statusP = create_status(internP, code, (SmlGenericCmdPtr_t)getP);
        add_target_ref(statusP, itemCell->item->target);
        add_element(internP, (basicElement_t *)statusP);

        itemCell = itemCell->next;
    }

    if (resultP->itemList)
    {
        add_element(internP, (basicElement_t *)resultP);
    }
    else
    {
        smlFreeResults(resultP);
    }

    return SML_ERR_OK;
}

static Ret_t prv_status_cmd_cb(InstanceID_t id,
                               VoidPtr_t userData,
                               SmlStatusPtr_t statusP)
{
    internals_t * internP = (internals_t *)userData;
    int code;
    char * cmdRef;
    char * msgRef;

    cmdRef = smlPcdata2String(statusP->cmdRef);
    if (!cmdRef) return SML_ERR_WRONG_PARAM;

    msgRef = smlPcdata2String(statusP->msgRef);
    if (!msgRef)
    {
        free(cmdRef);
        return SML_ERR_WRONG_PARAM;
    }

    code = pcdata_to_int(statusP->data);

    if (strcmp(cmdRef, "0"))
    {
        elemCell_t * cellP;

        cellP = retrieve_element(internP, cmdRef, msgRef);
        if (cellP)
        {
            switch (code)
            {
            case OMADM_SYNCML_ERROR_IN_PROGRESS:
                // put it back in the sent command list
                put_back_element(internP, cellP);
                break;
            case OMADM_SYNCML_ERROR_INVALID_CREDENTIALS:
            case OMADM_SYNCML_ERROR_MISSING_CREDENTIALS:
                // resend this with the new header
                add_element(internP, cellP->element);
                free(cellP); // /!\ do not free the element
                break;
            default:
                // nothing more to do
                free_element_list(cellP);
                break;
            }
        }
    }
    else
    {
        // we are dealing with the header
        internP->clt_auth = code;
        if (statusP->chal)
        {
            authType_t type;
            buffer_t newNonce;

            type = get_from_chal_meta(statusP->chal->meta, &newNonce);
            if (type != AUTH_TYPE_UNKNOWN)
            {
                internP->account->toServerCred->type = type;
                if (internP->account->toServerCred->data.buffer) free(internP->account->toServerCred->data.buffer);
                internP->account->toServerCred->data.buffer = newNonce.buffer;
                internP->account->toServerCred->data.len = newNonce.len;
                store_nonce(internP->dmtreeH->MOs, internP->account, true);
            }
        }
    }

    free(cmdRef);
    free(msgRef);

    return SML_ERR_OK;
}

static Ret_t prv_unimplemented_cb(InstanceID_t id,
                                  VoidPtr_t userData,
                                  SmlGenericCmdPtr_t pContent)
{
    internals_t * internP = (internals_t *)userData;
    SmlStatusPtr_t statusP;

    statusP = create_status(internP, OMADM_SYNCML_ERROR_COMMAND_NOT_IMPLEMENTED, pContent);

    add_element(internP, (basicElement_t *)statusP);

    return SML_ERR_OK;
}

static Ret_t prv_handle_error_cb(InstanceID_t id,
                                 VoidPtr_t userData)
{
    return SML_ERR_OK;
}

static Ret_t prv_transmit_chunk_cb(InstanceID_t id,
                                   VoidPtr_t userData)
{
    return SML_ERR_OK;
}


SmlCallbacksPtr_t get_callbacks()
{
    SmlCallbacksPtr_t callbacksP;

    callbacksP = (SmlCallbacksPtr_t)malloc(sizeof(SmlCallbacks_t));
    if (callbacksP)
    {
        memset(callbacksP, 0, sizeof(SmlCallbacks_t));
        callbacksP->startMessageFunc  = prv_start_message_cb;
        callbacksP->endMessageFunc    = prv_end_message_cb;
        callbacksP->startAtomicFunc   = prv_start_atomic_cb;
        callbacksP->endAtomicFunc     = prv_end_atomic_cb;
        callbacksP->startSequenceFunc = prv_start_sequence_cb;
        callbacksP->endSequenceFunc   = prv_end_sequence_cb;
        callbacksP->addCmdFunc        = prv_do_generic_cmd_cb;
        callbacksP->alertCmdFunc      = prv_alert_cmd_cb;
        callbacksP->deleteCmdFunc     = prv_do_generic_cmd_cb;
        callbacksP->execCmdFunc    	  = prv_do_generic_cmd_cb;
        callbacksP->getCmdFunc        = prv_get_cmd_cb;
        callbacksP->statusCmdFunc     = prv_status_cmd_cb;
        callbacksP->replaceCmdFunc    = prv_do_generic_cmd_cb;
        callbacksP->copyCmdFunc       = prv_do_generic_cmd_cb;
        callbacksP->handleErrorFunc   = prv_handle_error_cb;
        callbacksP->transmitChunkFunc = prv_transmit_chunk_cb;

        // Commands not implemented
        callbacksP->startSyncFunc  = (smlStartSyncFunc)prv_unimplemented_cb;
        callbacksP->putCmdFunc     = (smlPutCmdFunc)prv_unimplemented_cb;
        callbacksP->mapCmdFunc     = (smlMapCmdFunc)prv_unimplemented_cb;
        callbacksP->resultsCmdFunc = (smlResultsCmdFunc)prv_unimplemented_cb;
        callbacksP->searchCmdFunc  = (smlSearchCmdFunc)prv_unimplemented_cb;

        // Commands ignored
        callbacksP->endSyncFunc    = NULL;
    }

    return callbacksP;
}
