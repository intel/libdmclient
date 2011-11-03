/**
 * @file
 * SyncML Command Builder
 *
 * @target_system   all
 * @target_os       all
 * @description Core Module for assembling SyncML compliant documents
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

/* Include Headers */
#include <smldef.h>
#include "xltenc.h"
#include "xltdec.h"
#include "libmem.h"
#include "mgr.h"


/* Used external functions */
extern Ret_t smlLockWriteBuffer(InstanceID_t id, MemPtr_t *pWritePosition, MemSize_t *freeSize);
extern Ret_t smlUnlockWriteBuffer(InstanceID_t id, MemSize_t writtenBytes);

#ifndef __SML_LITE__  /* these API calls are NOT included in the Toolkit lite version */
  extern Ret_t addInfo(InstanceInfoPtr_t pInfo);
  extern InstanceInfoPtr_t findInfo(InstanceID_t id);
  extern Ret_t removeInfo(InstanceID_t id);
#endif

/* Prototypes of exported SyncML API functions */
SML_API Ret_t smlStartMessage(InstanceID_t id, SmlSyncHdrPtr_t pContent);
SML_API Ret_t smlStartMessageExt(InstanceID_t id, SmlSyncHdrPtr_t pContent, SmlVersion_t vers);
SML_API Ret_t smlEndMessage(InstanceID_t id, Boolean_t final);
SML_API Ret_t smlStartSync(InstanceID_t id, SmlSyncPtr_t pContent);
SML_API Ret_t smlEndSync(InstanceID_t id);

#ifdef ATOMIC_SEND  /* these API calls are NOT included in the Toolkit lite version */
  SML_API Ret_t smlStartAtomic(InstanceID_t id, SmlAtomicPtr_t pContent);
  SML_API Ret_t smlEndAtomic(InstanceID_t id);
#endif
#ifdef SEQUENCE_SEND
  SML_API Ret_t smlStartSequence(InstanceID_t id, SmlSequencePtr_t pContent);
  SML_API Ret_t smlEndSequence(InstanceID_t id);
#endif

#ifdef ADD_SEND
  SML_API Ret_t smlAddCmd(InstanceID_t id, SmlAddPtr_t pContent);
#endif
SML_API Ret_t smlAlertCmd(InstanceID_t id, SmlAlertPtr_t pContent);
SML_API Ret_t smlDeleteCmd(InstanceID_t id, SmlDeletePtr_t pContent);
#ifdef GET_SEND
  SML_API Ret_t smlGetCmd(InstanceID_t id, SmlGetPtr_t pContent);
#endif
SML_API Ret_t smlPutCmd(InstanceID_t id, SmlPutPtr_t pContent);
SML_API Ret_t smlMapCmd(InstanceID_t id, SmlMapPtr_t pContent);
SML_API Ret_t smlResultsCmd(InstanceID_t id, SmlResultsPtr_t pContent);
SML_API Ret_t smlStatusCmd(InstanceID_t id, SmlStatusPtr_t pContent);
SML_API Ret_t smlReplaceCmd(InstanceID_t id, SmlReplacePtr_t pContent);

#ifdef COPY_SEND  /* these API calls are NOT included in the Toolkit lite version */
  SML_API Ret_t smlCopyCmd(InstanceID_t id, SmlCopyPtr_t pContent);
#endif
#ifdef EXEC_SEND
  SML_API Ret_t smlExecCmd(InstanceID_t id, SmlExecPtr_t pContent);
#endif
#ifdef SEARCH_SEND
  SML_API Ret_t smlSearchCmd(InstanceID_t id, SmlSearchPtr_t pContent);
#endif
  SML_API Ret_t smlMoveCmd(InstanceID_t id, SmlMovePtr_t pContent);


/* Private function prototypes */
static Ret_t mgrCreateNextCommand(InstanceID_t id, SmlProtoElement_t cmdType, VoidPtr_t pContent);
Ret_t mgrResetWorkspace (InstanceID_t id);




/*************************************************************************
 *  Exported SyncML API functions
 *************************************************************************/


/**
 * Start a SyncML Message
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 * @note (%%% luz 2003-08-06) this entry point is for compatibilty
 *       reasons only and works for SyncML 1.0 only\n
 *       please use smlStartMessageExt() instead in new projects.
 */
SML_API Ret_t smlStartMessage(InstanceID_t id, SmlSyncHdrPtr_t pContent)
{
  /* just call smlStartMessageExt with vers set to SyncML 1.0 */
  return smlStartMessageExt(id,pContent,SML_VERS_1_0);
}


/**
 * Start a SyncML Message
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @param vers (IN)
 *        SyncML version
 * @note (%%% added by luz 2003-08-06 to support SyncML versions other than
 *       1.0 with new vers parameter)
 * @return Return Code
 */
SML_API Ret_t smlStartMessageExt(InstanceID_t id, SmlSyncHdrPtr_t pContent, SmlVersion_t vers)
{

  /* --- Definitions --- */
  InstanceInfoPtr_t   pInstanceInfo;               // pointer the the instance info structure for this id
  Ret_t               rc;
  MemPtr_t            pCurrentWritePosition;       // current Position from to which to write
  MemPtr_t            pBeginPosition;              // saves the first position which has been written
  MemSize_t           freeSize;                    // size of free memory for writing


  #ifdef NOWSM
    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
  #else
    /* --- Retrieve the corresponding instanceInfo structure --- */
    #ifdef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
      pInstanceInfo = mgrGetInstanceListAnchor();
    #else
      pInstanceInfo = (InstanceInfoPtr_t) findInfo(id);
    #endif
  #endif

  if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;


  /* --- Get Write Access to the workspace --- */

  #ifdef NOWSM
  // first remember where outgoing message starts in buffer
  smlSetOutgoingBegin(id);
  #endif

  // possibly, this will cause a buffer reset (if nothing is
  // unread in the buffer), and will auto-readjust the OutgoingBegin.
  rc = smlLockWriteBuffer(id, &pCurrentWritePosition, &freeSize);

  if (rc!=SML_ERR_OK) {
    // abort, unlock the buffer again without changing it's current position
    smlUnlockWriteBuffer(id, (MemSize_t)0);
    return rc;
    }

  /* Remember the position we have started writing */
  pBeginPosition=pCurrentWritePosition;

  /* --- Call the encoder module --- */
  /*     (Saves the returned encoder state to the corresponding instanceInfo structure */
  rc = xltEncInit
    (pInstanceInfo->instanceOptions->encoding,
    pContent,
    pCurrentWritePosition+freeSize,
    &pCurrentWritePosition,
    (XltEncoderPtr_t *)&(pInstanceInfo->encoderState),
    vers
  );

  if (rc!=SML_ERR_OK) {
    // abort, unlock the buffer again without changing it's current position
    smlUnlockWriteBuffer(id, (MemSize_t)0);
    // Reset the encoder module (free the encoding object)
    xltEncReset(pInstanceInfo->encoderState);
    // this encoding job is over! reset instanceInfo pointer
    pInstanceInfo->encoderState=NULL;

    return rc;
    }

  /* --- End Write Access to the workspace --- */
  rc = smlUnlockWriteBuffer(id, (MemSize_t)pCurrentWritePosition-(MemSize_t)pBeginPosition);
  return rc;
}

/**
 * End a SyncML Message
 *
 * @param id (IN)
 *        ID of the used instance
 * @param final (IN)
 *        Final Flag indicates last message within a package
 * @return Return Code
 */
SML_API Ret_t smlEndMessage(InstanceID_t id, Boolean_t final)
{

  /* --- Definitions --- */
  InstanceInfoPtr_t   pInstanceInfo;               // pointer the the instance info structure for this id
  Ret_t               rc;
  MemPtr_t            pCurrentWritePosition;       // current Position from to which to write
  MemPtr_t            pBeginPosition;              // saves the first position which has been written
  MemSize_t           freeSize;                    // size of free memory for writing


  #ifdef NOWSM
    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
  #else
    /* --- Retrieve the corresponding instanceInfo structure --- */
    #ifdef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
      pInstanceInfo = mgrGetInstanceListAnchor();
    #else
      pInstanceInfo = (InstanceInfoPtr_t) findInfo(id);
    #endif
  #endif

  if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;
  // %%% luz 2003-08-19: added NULL check as previously failed encoding will delete encoder
  if (pInstanceInfo->encoderState==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;


  /* --- Get Write Access to the workspace --- */
  rc = smlLockWriteBuffer(id, &pCurrentWritePosition, &freeSize);

  if (rc!=SML_ERR_OK) {
    // abort, unlock the buffer again without changing it's current position
    smlUnlockWriteBuffer(id, (MemSize_t)0);
    return rc;
    }


  /* Remember the position we have started writing */
  pBeginPosition=pCurrentWritePosition;

  /* -- set Final Flag --*/
  ((XltEncoderPtr_t)(pInstanceInfo->encoderState))->final = final;

  /* --- Call the encoder module --- */
  rc = xltEncTerminate(pInstanceInfo->encoderState, pCurrentWritePosition+freeSize,&pCurrentWritePosition);

  if (rc!=SML_ERR_OK) {
    // abort, unlock the buffer again without changing it's current position
    smlUnlockWriteBuffer(id, (MemSize_t)0);
    // this encoding job is over! reset instanceInfo pointer
    pInstanceInfo->encoderState=NULL;

    return rc;
  }

  // this encoding job is over! reset instanceInfo pointer
  // (the decoding object itself has been freed by the decoder)
  pInstanceInfo->encoderState=NULL;

  /* --- End Write Access to the workspace --- */
  rc = smlUnlockWriteBuffer(id, (MemSize_t)pCurrentWritePosition-(MemSize_t)pBeginPosition);


  return rc;
}





/**
 * Start synchronizing
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN) Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlStartSync(InstanceID_t id, SmlSyncPtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_SYNC_START, pContent);
}



/**
 * End synchronizing
 *
 * @param id (IN)
 *        ID of the used instance
 * @return Return Code
 */
SML_API Ret_t smlEndSync(InstanceID_t id)
{
  return mgrCreateNextCommand(id, SML_PE_SYNC_END, NULL);
}


#ifdef ATOMIC_SEND  /* these API calls are NOT included in the Toolkit lite version */

/**
 * Start an atomic sequence
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlStartAtomic(InstanceID_t id, SmlAtomicPtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_ATOMIC_START, pContent);
}


/**
 * End an atomic sequence
 *
 * @param id (IN)
 *        ID of the used instance
 * @return Return Code
 */
SML_API Ret_t smlEndAtomic(InstanceID_t id)
{
  return mgrCreateNextCommand(id, SML_PE_ATOMIC_END, NULL);
}

#endif

#ifdef SEQUENCE_SEND

/**
 * Start a sequence
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlStartSequence(InstanceID_t id, SmlSequencePtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_SEQUENCE_START, pContent);
}



/**
 * End a sequence
 *
 * @param id (IN)
 *        ID of the used instance
 * @return Return Code
 */
SML_API Ret_t smlEndSequence(InstanceID_t id)
{
  return mgrCreateNextCommand(id, SML_PE_SEQUENCE_END, NULL);
}

#endif


#ifdef ADD_SEND
/**
 * Create a Add Command
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlAddCmd(InstanceID_t id, SmlAddPtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_ADD, pContent);
}
#endif


/**
 * Create a Alert Command
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlAlertCmd(InstanceID_t id, SmlAlertPtr_t pContent)
{
   return mgrCreateNextCommand(id, SML_PE_ALERT, pContent);
}




/**
 * Create a Start Message Command
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlDeleteCmd(InstanceID_t id, SmlDeletePtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_DELETE, pContent);
}



#ifdef GET_SEND


/**
 * Create a Get Command
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlGetCmd(InstanceID_t id, SmlGetPtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_GET, pContent);
}

#endif


/**
 * Create a Put Command
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlPutCmd(InstanceID_t id, SmlPutPtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_PUT, pContent);
}



/**
 * Create a Map Command
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlMapCmd(InstanceID_t id, SmlMapPtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_MAP, pContent);
}



/**
 * Create a Results  Command
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlResultsCmd(InstanceID_t id, SmlResultsPtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_RESULTS, pContent);
}





/**
 * Create a Status Command
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlStatusCmd(InstanceID_t id, SmlStatusPtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_STATUS, pContent);
}



/**
 * Create a Replace Command
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlReplaceCmd(InstanceID_t id, SmlReplacePtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_REPLACE, pContent);
}



#ifdef COPY_SEND  /* these API calls are NOT included in the Toolkit lite version */


/**
 * Create a Copy Command
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlCopyCmd(InstanceID_t id, SmlCopyPtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_COPY, pContent);
}

#endif


/**
 * Create a Move Command
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlMoveCmd(InstanceID_t id, SmlMovePtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_MOVE, pContent);
}


#ifdef EXEC_SEND

/**
 * Create a Exec Command
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlExecCmd(InstanceID_t id, SmlExecPtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_EXEC, pContent);
}

#endif

#ifdef SEARCH_SEND

/**
 * Create a Search Command
 *
 * @param id (IN)
 *        ID of the used instance
 * @param pContent (IN)
 *        Data to pass along with that SyncML command
 * @return Return Code
 */
SML_API Ret_t smlSearchCmd(InstanceID_t id, SmlSearchPtr_t pContent)
{
  return mgrCreateNextCommand(id, SML_PE_SEARCH, pContent);
}


#endif


/*************************************************************************
 *  Exported SyncML API functions (FULL-SIZE TOOLKIT ONLY)
 *************************************************************************/

#ifndef __SML_LITE__  /* these API calls are NOT included in the Toolkit lite version */
/**
 * Starts an evaluation run which prevents further API-Calls to write tags -
 * just the tag-sizes are calculated. Must be sopped via smlEndEvaluation
 *
 * @param id (IN)
 *        ID of the used instance
 * @return Return Code
 */
SML_API Ret_t smlStartEvaluation(InstanceID_t id)
{
  InstanceInfoPtr_t   pInstanceInfo;               // pointer the the instance info structure for this id
  Ret_t               rc;

  #ifdef NOWSM
    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
  #else
    /* --- Retrieve the corresponding instanceInfo structure --- */
    #ifdef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
      pInstanceInfo = mgrGetInstanceListAnchor();
    #else
      pInstanceInfo = (InstanceInfoPtr_t) findInfo(id);
    #endif
  #endif

  if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;


  /* --- Initialize Encoder for evaluation mode --- */

  rc = xltStartEvaluation((XltEncoderPtr_t)(pInstanceInfo->encoderState));

  return rc;
}


/**
 * Stops an evaluation run which prevents further API-Calls to write tags -
 * the remaining free buffer size after all Tags are written is returned
 *
 * @param id (IN)
 *        ID of the used instance
 * @param freemem (IN/OUT)
 *        Size of free buffer for data after all tags are written
 * @return Return Code
 */
SML_API Ret_t smlEndEvaluation(InstanceID_t id, MemSize_t *freemem)
{
  InstanceInfoPtr_t   pInstanceInfo;               // pointer the the instance info structure for this id
  Ret_t               rc;

  #ifdef NOWSM
    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
  #else
    /* --- Retrieve the corresponding instanceInfo structure --- */
    #ifdef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
      pInstanceInfo = mgrGetInstanceListAnchor();
    #else
      pInstanceInfo = (InstanceInfoPtr_t) findInfo(id);
    #endif
  #endif

  if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;

  // %%% luz 2002-09-03: encoder can be null here if fatal error occurred before
  if (pInstanceInfo->encoderState==NULL)
    return SML_ERR_WRONG_USAGE;

  rc = xltEndEvaluation(id, (XltEncoderPtr_t)(pInstanceInfo->encoderState), freemem);
  return SML_ERR_OK;
}

#endif


/*************************************************************************
 *  Private Functions
 *************************************************************************/


/**
 * Calls the encoding routines of the Encoder Module for a given Command Type
 * and Command Content
 *
 * @param id (IN)
 *        ID of the Instance
 * @param cmdType (IN)
 *        Type of the command (defined by the Proto Element Enumeration)
 * @param pContent (IN)
 *        Content of the command to encode
 * @return Return value,\n
 *         SML_ERR_OK if command has been encoded successfully
 */
static Ret_t mgrCreateNextCommand(InstanceID_t id, SmlProtoElement_t cmdType, VoidPtr_t pContent)
{
  /* --- Definitions --- */
  InstanceInfoPtr_t   pInstanceInfo;               // pointer the the instance info structure for this id
  Ret_t               rc;
  MemPtr_t            pCurrentWritePosition;       // current Position from to which to write
  MemPtr_t            pBeginPosition;              // saves the first position which has been written
  MemSize_t           freeSize;                    // size of free memory for writing


  #ifdef NOWSM
    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
  #else
    /* --- Retrieve the corresponding instanceInfo structure --- */
    #ifdef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
      pInstanceInfo = mgrGetInstanceListAnchor();
    #else
      pInstanceInfo = (InstanceInfoPtr_t) findInfo(id);
    #endif
  #endif

  if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;
  // %%% luz 2002-11-27: added NULL check as previously failed encoding will delete encoder
  if (pInstanceInfo->encoderState==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;

  /* --- Get Write Access to the workspace --- */
  rc = smlLockWriteBuffer(id, &pCurrentWritePosition, &freeSize);

  if (rc!=SML_ERR_OK)  {
    // abort, unlock the buffer again without changing it's current position
    smlUnlockWriteBuffer(id, (MemSize_t)0);
    return rc;
    }


  // Remember the position we have started writing
  pBeginPosition=pCurrentWritePosition;


  /* --- Call the encoder module --- */
  rc = xltEncAppend(pInstanceInfo->encoderState, cmdType, pCurrentWritePosition+freeSize, pContent, &pCurrentWritePosition);

  if (rc!=SML_ERR_OK) {
    /* check for full buffer and call TransmitChunk */
    if (rc == SML_ERR_XLT_BUF_ERR) {
      // first check wether callback is defined
      if (pInstanceInfo->callbacks->transmitChunkFunc!= NULL) {
        // abort, unlock the buffer again without changing it's current position
        smlUnlockWriteBuffer(id, (MemSize_t)0);
        // call the callback
        pInstanceInfo->callbacks->transmitChunkFunc(id,NULL);
        // lock -> returns the amount of free buffer space
        smlLockWriteBuffer(id, &pCurrentWritePosition, &freeSize);
        pBeginPosition = pCurrentWritePosition;
        // now try again to encode and see wether we now have enough mem available
        rc = xltEncAppend(pInstanceInfo->encoderState, cmdType, pCurrentWritePosition+freeSize, pContent, &pCurrentWritePosition);
        // if rc == SML_ERR_OK continue else
          // return the errorcode
        if( rc != SML_ERR_OK)
        {
          smlUnlockWriteBuffer(id, (MemSize_t)0);
          // Reset the encoder module (free the encoding object)
          xltEncReset(pInstanceInfo->encoderState);
          // this encoding job is over! reset instanceInfo pointer
          pInstanceInfo->encoderState=NULL;
          return rc;
        }
      }
    } else {
      // abort, unlock the buffer again without changing it's current position
      smlUnlockWriteBuffer(id, (MemSize_t)0);
      // Reset the encoder module (free the encoding object)
      xltEncReset(pInstanceInfo->encoderState);
      // this encoding job is over! reset instanceInfo pointer
      pInstanceInfo->encoderState=NULL;
      return rc;
    }
  }
  /* --- End Write Access to the workspace --- */
  rc = smlUnlockWriteBuffer(id, (MemSize_t)pCurrentWritePosition-(MemSize_t)pBeginPosition);
  return rc;
}

/* eof */
