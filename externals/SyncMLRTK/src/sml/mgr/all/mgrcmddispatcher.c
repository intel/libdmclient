/**
 * @file
 * SyncML Command Dispatcher
 *
 * @target_system   all
 * @target_os       all
 * @description Core module for dispatching parsed commands and invoking
 * callback functions of the application
 */


/*
 * Copyright Notice
 * Copyright (c) Ericsson, IBM, Lotus, Matsushita Communication
 * Industrial Co., Ltd., Motorola, Nokia, Openwave Systems, Inc.,
 * Palm, Inc., Psion, Starfish Software, Symbian, Ltd. (2001).
 * All Rights Reserved.
 * Implementation of all or part of any Specification may require
 * licenses under third party intellectual property rights,
 * including without limitation, patent rights (such a third party
 * may or may not be a Supporter). The Sponsors of the Specification
 * are not responsible and shall not be held responsible in any
 * manner for identifying or failing to identify any or all such
 * third party intellectual property rights.
 *
 * THIS DOCUMENT AND THE INFORMATION CONTAINED HEREIN ARE PROVIDED
 * ON AN "AS IS" BASIS WITHOUT WARRANTY OF ANY KIND AND ERICSSON, IBM,
 * LOTUS, MATSUSHITA COMMUNICATION INDUSTRIAL CO. LTD, MOTOROLA,
 * NOKIA, PALM INC., PSION, STARFISH SOFTWARE AND ALL OTHER SYNCML
 * SPONSORS DISCLAIM ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO ANY WARRANTY THAT THE USE OF THE INFORMATION
 * HEREIN WILL NOT INFRINGE ANY RIGHTS OR ANY IMPLIED WARRANTIES OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT
 * SHALL ERICSSON, IBM, LOTUS, MATSUSHITA COMMUNICATION INDUSTRIAL CO.,
 * LTD, MOTOROLA, NOKIA, PALM INC., PSION, STARFISH SOFTWARE OR ANY
 * OTHER SYNCML SPONSOR BE LIABLE TO ANY PARTY FOR ANY LOSS OF
 * PROFITS, LOSS OF BUSINESS, LOSS OF USE OF DATA, INTERRUPTION OF
 * BUSINESS, OR FOR DIRECT, INDIRECT, SPECIAL OR EXEMPLARY, INCIDENTAL,
 * PUNITIVE OR CONSEQUENTIAL DAMAGES OF ANY KIND IN CONNECTION WITH
 * THIS DOCUMENT OR THE INFORMATION CONTAINED HEREIN, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH LOSS OR DAMAGE.
 *
 * The above notice and this paragraph must be included on all copies
 * of this document that are made.
 *
 */




/*************************************************************************
 *  Definitions
 *************************************************************************/

#include "syncml_tk_prefix_file.h" // %%% luz: needed for precompiled headers in eVC++

#include "define.h"
/* Include Headers */
#include "smldef.h"
#include "smldtd.h"
#include "smlerr.h"

#include "libmem.h"
#include "xltdec.h"
#include "mgr.h"



/* Used external functions */
#ifndef __SML_LITE__  /* these API calls are NOT included in the Toolkit lite version */
  extern Ret_t addInfo(InstanceInfoPtr_t pInfo);
  extern InstanceInfoPtr_t findInfo(InstanceID_t id);
  extern Ret_t removeInfo(InstanceID_t id);
#endif
Ret_t smlLockReadBuffer(InstanceID_t id, MemPtr_t *pReadPosition, MemSize_t *usedSize);
Ret_t smlUnlockReadBuffer(InstanceID_t id, MemSize_t processedBytes);

/* Prototypes of exported SyncML API functions */
extern Ret_t smlProcessData(InstanceID_t id, SmlProcessMode_t mode);

/* Private function prototypes */
static Ret_t mgrProcessNextCommand(InstanceID_t id, InstanceInfoPtr_t pInstanceInfo);
static Ret_t mgrProcessStartMessage(InstanceID_t id, InstanceInfoPtr_t pInstanceInfo);
Ret_t mgrResetWorkspace (InstanceID_t id);



/*************************************************************************
 *  Exported SyncML API functions
 *************************************************************************/




/**
 * Start the parsing of the XML code in the workspace buffer,
 * dispatches the interpreted command and calls the corresponding callback
 * functions provided by the application.
 *
 * @param id (IN)
 *        The SyncML instance id is used for referencing the
 *        workspace buffer from the XML content is parsed
 * @param mode (IN)
 *        Mode of processing, Defines, if only the first or next
 *        XML command is parsed or if all commands are processed
 *        subsequently until the end of the entire workspace buffer
 *        is reached. The NEXT_COMMAND flag defines the blocking mode,
 *        the ALL_COMMANDS tag defines the non-blocking mode.
 * @return Return Code
 */
SML_API Ret_t smlProcessData(InstanceID_t id, SmlProcessMode_t mode)
{
  /* --- Definitions --- */
  InstanceInfoPtr_t   pInstanceInfo;               // state info for the given instanceID
  Ret_t               rc;                          // Temporary return code saver


  #ifdef NOWSM
    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
  #else
    /* --- Find that instance --- */
    #ifdef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
      pInstanceInfo = mgrGetInstanceListAnchor();
    #else
      pInstanceInfo = (InstanceInfoPtr_t) findInfo(id);
    #endif
  #endif

  if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;

  /* --- Are callback functions defined? --- */
  if (pInstanceInfo->callbacks==NULL) return SML_ERR_COMMAND_NOT_HANDLED;

  /* --- Is parsing already in progress? --- */
  if (pInstanceInfo->decoderState==NULL)
    {
    /* No! Parse the Message header section first */
    rc = mgrProcessStartMessage(id, pInstanceInfo);

    if (rc!=SML_ERR_OK) return rc;
    }


  /* --- Parse now the Message body section! --- */
  do {
    rc=mgrProcessNextCommand(id, pInstanceInfo);
  } while (
    // keep processing while no error occurs,
    // AND the document end was not reached (decoderState has been invalidated),
    // AND the ALL_COMMAND mode is used
    (rc==SML_ERR_OK)
    &&((pInstanceInfo->decoderState)!=NULL)
    &&(mode==SML_ALL_COMMANDS)
  );

  if (rc != SML_ERR_OK) {
    // abort, unlock the buffer again without changing it's current position
    smlUnlockReadBuffer(id, (MemSize_t)0);
    // Reset the decoder module (free the decoding object)
    xltDecReset(pInstanceInfo->decoderState);
    // this decoding job is over! reset Instance Info pointer
    pInstanceInfo->decoderState=NULL;
    // Reset the Workspace (the remaining unparsed document fragment will be lost)
    mgrResetWorkspace(id);
  }
  else {
    // %%%luz 2007-09-11: added SML_ERR_CONTINUE to signal processing of message not
    //                    yet complete in NEXT_COMMAND mode (is otherwise not very useful)
    if ((mode==SML_NEXT_COMMAND) && ((pInstanceInfo->decoderState)!=NULL)) {
      rc = SML_ERR_CONTINUE; // processing not yet complete
    }
  }

  return rc;
}


/*************************************************************************
 *  Private Functions
 *************************************************************************/




/**
 * Parses the header information at the beginning of an SyncML document.
 *
 * @param id (IN)
 *        current InstanceID to pass to callback functions
 * @param pInstanceInfo (IN/OUT)
 *        state information of the given InstanceID
 *        (decoder state will be changed)
 * @return Return value of the Parser,\n
 *         SML_ERR_OK if next command was handled successfully
 */
static Ret_t mgrProcessStartMessage(InstanceID_t id, InstanceInfoPtr_t pInstanceInfo)
{


  /* --- Definitions --- */
  Ret_t               rc;                          // Temporary return code saver
  SmlSyncHdrPtr_t     pContent=NULL;               // data of the command to process
  MemPtr_t            pCurrentReadPosition ;        // current Position from which is read
  MemPtr_t            pBeginPosition;              // saves the first position which has been reading
  MemSize_t           usedSize ;                    // size of used memory to be read


  /* --- Get Read Access to the workspace --- */
  rc = smlLockReadBuffer(id, &pCurrentReadPosition, &usedSize);

  if (rc!=SML_ERR_OK) {
    // abort, unlock the buffer again without changing it's current position
    smlUnlockReadBuffer(id, (MemSize_t)0);
    return rc;
    }

  // Remember the position we have started reading
  pBeginPosition=pCurrentReadPosition;
  #ifdef NOWSM
  // luz 2008-02-08: also remember the position of the first byte of the message to allow dumping it AFTER parsing the header
  pInstanceInfo->incomingMsgStart = pBeginPosition;
  #endif

  /* --- Start new decoding sequence and pass returned decoder status structure to instanceInfo --- */
  rc = xltDecInit(pInstanceInfo->instanceOptions->encoding,
                  pCurrentReadPosition+usedSize-1, &pCurrentReadPosition,
                  (XltDecoderPtr_t *)&(pInstanceInfo->decoderState), &pContent);

  if (rc!=SML_ERR_OK) {
    // abort, unlock the buffer again without changing it's current position
    smlUnlockReadBuffer(id, (MemSize_t)0);
    // Reset the decoder module (free the decoding object)
    xltDecReset(pInstanceInfo->decoderState);
    // this decoding job is over! reset Instance Info pointer
    pInstanceInfo->decoderState=NULL;
    // Reset the Workspace (the remaining unparsed document fragment will be lost)
    mgrResetWorkspace(id);
    return rc;
  }

  /* --- End Read Access to the workspace --- */
  rc = smlUnlockReadBuffer(id, (MemSize_t)pCurrentReadPosition-(MemSize_t)pBeginPosition);
  if (rc!=SML_ERR_OK) return rc;

  /* --- Perform callback to handle the beginning of a new message --- */
  if (pInstanceInfo->callbacks->startMessageFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
  rc=pInstanceInfo->callbacks->startMessageFunc(id, pInstanceInfo->userData, pContent);

  if (rc != SML_ERR_OK)
  {
    // abort, unlock the buffer again without changing it's current position
    smlUnlockReadBuffer(id, (MemSize_t)0);
    // Reset the decoder module (free the decoding object)
    xltDecReset(pInstanceInfo->decoderState);
    // this decoding job is over! reset Instance Info pointer
    pInstanceInfo->decoderState=NULL;
    // Reset the Workspace (the remaining unparsed document fragment will be lost)
    mgrResetWorkspace(id);
  }

  return rc;
}



/**
 * Parses the next Sync Command in the sync document.
 *
 * @param id (IN)
 *        current InstanceID to pass to callback functions
 * @param pInstanceInfo (IN)
 *        state information of the given InstanceID
 * @return Return value of the Parser of the called application callback,\n
 *         SML_ERR_OK if next command was handled successfully
 */
static Ret_t mgrProcessNextCommand(InstanceID_t id, InstanceInfoPtr_t pInstanceInfo)
{

  /* --- Definitions --- */
  Ret_t               rc;                          // Temporary return code saver
  SmlProtoElement_t   cmdType;                     // ID of the command to process
  VoidPtr_t           pContent=NULL;               // data of the command to process
  MemPtr_t            pCurrentReadPosition;        // current Position from which is read
  MemPtr_t            pBeginPosition;              // saves the first position which has been reading
  MemSize_t           usedSize;                    // size of used memory to be read
  Boolean_t           final;                       // flag indicates last message within a package


  /* --- Get Read Access to the workspace --- */
  rc = smlLockReadBuffer(id, &pCurrentReadPosition, &usedSize);

  if (rc!=SML_ERR_OK) {
    // abort, unlock the buffer again without changing it's current position
    smlUnlockReadBuffer(id, (MemSize_t)0);
    return rc;
    }

  // Remember the position we have started reading
  pBeginPosition=pCurrentReadPosition;


  /* --- Parse next Command --- */
  rc = xltDecNext(pInstanceInfo->decoderState, pCurrentReadPosition+usedSize, &pCurrentReadPosition, &cmdType, &pContent);

  if (rc!=SML_ERR_OK) {
    // abort, unlock the buffer again without changing it's current position
    smlUnlockReadBuffer(id, (MemSize_t)0);
    // Reset the decoder module (free the decoding object)
    xltDecReset(pInstanceInfo->decoderState);
    // this decoding job is over! reset Instance Info pointer
    pInstanceInfo->decoderState=NULL;
    // Reset the Workspace (the remaining unparsed document fragment will be lost)
    mgrResetWorkspace(id);
    return rc;
  }

  /* --- End Read Access to the workspace --- */
  rc = smlUnlockReadBuffer(id, (MemSize_t)pCurrentReadPosition-(MemSize_t)pBeginPosition);

  if (rc!=SML_ERR_OK) {
    // abort, unlock the buffer again without changing it's current position
    smlUnlockReadBuffer(id, (MemSize_t)0);
    return rc;
  }

  /* --- Did we reach end of synchronization document? --- */
  if (((XltDecoderPtr_t)(pInstanceInfo->decoderState))->finished!=0) {
    final = ((XltDecoderPtr_t)(pInstanceInfo->decoderState))->final; // flag is returned to appl. with callback
    rc=xltDecTerminate(pInstanceInfo->decoderState);

    if (rc!=SML_ERR_OK)
    {
      // abort, unlock the buffer again without changing it's current position
      smlUnlockReadBuffer(id, (MemSize_t)0);
      // Reset the decoder module (free the decoding object)
      xltDecReset(pInstanceInfo->decoderState);
      // this decoding job is over! reset Instance Info pointer
      pInstanceInfo->decoderState=NULL;
      // Reset the Workspace (the remaining unparsed document fragment will be lost)
      mgrResetWorkspace(id);
      return rc;
    }

    // this decoding job is over! reset Instance Info pointer
    // (the decoding object itself has been freed by the decoder)
    pInstanceInfo->decoderState=NULL;

    // Call the callback for handling an message ending
    if (pInstanceInfo->callbacks->endMessageFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;

    rc=pInstanceInfo->callbacks->endMessageFunc(id, pInstanceInfo->userData, final);
    return rc;
  }

  /* --- Dispatch parsed command (and call the applications command handler function)--- */
  switch (cmdType)
  {
    /* Handle ADD Command */
    case SML_PE_ADD:
      if (pInstanceInfo->callbacks->addCmdFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->addCmdFunc (id, pInstanceInfo->userData, pContent);
      break;

    /* Handle ALERT Command */
    case SML_PE_ALERT:
      if (pInstanceInfo->callbacks->alertCmdFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->alertCmdFunc (id, pInstanceInfo->userData, pContent);
      break;

    /* Handle DELETE Command */
    case SML_PE_DELETE:
      if (pInstanceInfo->callbacks->deleteCmdFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->deleteCmdFunc (id, pInstanceInfo->userData, pContent);
      break;

    /* Handle PUT Command */
    case SML_PE_PUT:
      if (pInstanceInfo->callbacks->putCmdFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->putCmdFunc (id, pInstanceInfo->userData, pContent);
      break;

    /* Handle GET Command */
    case SML_PE_GET:
      if (pInstanceInfo->callbacks->getCmdFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->getCmdFunc (id, pInstanceInfo->userData, pContent);
      break;

    #ifdef MAP_RECEIVE
    /* Handle MAP Command */
    case SML_PE_MAP:
      if (pInstanceInfo->callbacks->mapCmdFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->mapCmdFunc (id, pInstanceInfo->userData, pContent);
      break;
    #endif

    #ifdef RESULT_RECEIVE
    /* Handle RESULTS Command */
    case SML_PE_RESULTS:
      if (pInstanceInfo->callbacks->resultsCmdFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->resultsCmdFunc (id, pInstanceInfo->userData, pContent);
      break;
    #endif

    /* Handle STATUS Command */
    case SML_PE_STATUS:
      if (pInstanceInfo->callbacks->statusCmdFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->statusCmdFunc (id, pInstanceInfo->userData, pContent);
      break;

    /* Handle START SYNC Command */
    case SML_PE_SYNC_START:
      if (pInstanceInfo->callbacks->startSyncFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->startSyncFunc (id, pInstanceInfo->userData, pContent);
      break;

    /* Handle END SYNC Command */
    case SML_PE_SYNC_END:
      if (pInstanceInfo->callbacks->endSyncFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->endSyncFunc (id, pInstanceInfo->userData);
      break;

    /* Handle REPLACE Command */
    case SML_PE_REPLACE:
      if (pInstanceInfo->callbacks->replaceCmdFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->replaceCmdFunc (id, pInstanceInfo->userData, pContent);
      break;

    /* Handle Final Flag */
    case SML_PE_FINAL:
      // if a FINAL Flag appears do nothing
      return SML_ERR_OK;
      break;

    #ifdef SEARCH_RECEIVE  /* these API calls are NOT included in the Toolkit lite version */

    /* Handle SEARCH Command */
    case SML_PE_SEARCH:
      if (pInstanceInfo->callbacks->searchCmdFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->searchCmdFunc (id, pInstanceInfo->userData, pContent);
      break;
    #endif

    #ifdef SEQUENCE_RECEIVE
    /* Handle START SEQUENCE Command */
    case SML_PE_SEQUENCE_START:
      if (pInstanceInfo->callbacks->startSequenceFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->startSequenceFunc (id, pInstanceInfo->userData, pContent);
      break;

    /* Handle END SEQUENCE Command */
    case SML_PE_SEQUENCE_END:
      if (pInstanceInfo->callbacks->endSequenceFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->endSequenceFunc (id, pInstanceInfo->userData);
      break;
    #endif

    #ifdef ATOMIC_RECEIVE

    /* Handle START ATOMIC Command */
    case SML_PE_ATOMIC_START:
      if (pInstanceInfo->callbacks->startAtomicFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->startAtomicFunc (id, pInstanceInfo->userData, pContent);
      break;

    /* Handle END ATOMIC Command */
    case SML_PE_ATOMIC_END:
      if (pInstanceInfo->callbacks->endAtomicFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->endAtomicFunc (id, pInstanceInfo->userData);
      break;
    #endif

    #ifdef COPY_RECEIVE

    /* Handle COPY Command */
    case SML_PE_COPY:
      if (pInstanceInfo->callbacks->copyCmdFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->copyCmdFunc (id, pInstanceInfo->userData, pContent);
      break;
    #endif

    /* Handle MOVE Command */
    case SML_PE_MOVE:
      if (pInstanceInfo->callbacks->moveCmdFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->moveCmdFunc (id, pInstanceInfo->userData, pContent);
      break;

    #ifdef EXEC_RECEIVE

    /* Handle EXEC Command */
    case SML_PE_EXEC:
      if (pInstanceInfo->callbacks->execCmdFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
      return pInstanceInfo->callbacks->execCmdFunc (id, pInstanceInfo->userData, pContent);
      break;

    #endif

    /* Handle ERROR DETECTED  */
    //case SML_PE_ERROR:
    //  if (pInstanceInfo->callbacks->handleErrorFunc==NULL) return SML_ERR_COMMAND_NOT_HANDLED;
    //  return pInstanceInfo->callbacks->handleErrorFunc (id, pInstanceInfo->userData);
    //  break;

    /* --- Invalid Command Element --- */
    default:
      return SML_ERR_XLT_INVAL_PROTO_ELEM;
      break;
  } // switch
}

/* eof */
