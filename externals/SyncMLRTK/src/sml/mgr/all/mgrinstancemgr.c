/**
 * @file
 * Managing SyncML Instances
 *
 * @target_system   all
 * @target_os       all
 * @description Core module for managing creation and usage of instances
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
#include <sml.h>
#include <smlerr.h>
#include "libmem.h"
#include "libstr.h"
#include "liblock.h"
#include "wsm.h"
#include "mgr.h"
#include <xltenc.h>



/* Used external functions */
#ifndef NOWSM
  #ifndef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
    extern Ret_t addInfo(InstanceInfoPtr_t pInfo);
    extern InstanceInfoPtr_t findInfo(InstanceID_t id);
    extern Ret_t removeInfo(InstanceID_t id);
  #endif
  SyncMLInfoPtr_t mgrGetSyncMLAnchor(void);
#endif

/* Prototypes of exported SyncML API functions */
SML_API Ret_t smlInitInstance(SmlCallbacksCPtr_t callbacks, SmlInstanceOptionsPtr_t pOptions, VoidPtr_t pUserData, InstanceID_t *pInstanceID);
SML_API Ret_t smlTerminateInstance (InstanceID_t id);
SML_API Ret_t smlLockReadBuffer(InstanceID_t id, MemPtr_t *pReadPosition, MemSize_t *usedSize);
SML_API Ret_t smlUnlockReadBuffer(InstanceID_t id, MemSize_t processedBytes);
#ifdef NOWSM
SML_API Ret_t smlSetMaxOutgoingSize(InstanceID_t id, MemSize_t maxOutgoingSize);
SML_API Ret_t smlSetOutgoingBegin(InstanceID_t id);
#endif
SML_API Ret_t smlLockWriteBuffer(InstanceID_t id, MemPtr_t *pWritePosition, MemSize_t *freeSize);
SML_API Ret_t smlUnlockWriteBuffer(InstanceID_t id, MemSize_t writtenBytes);
SML_API Ret_t smlSetCallbacks (InstanceID_t id, SmlCallbacksCPtr_t pCallbacks);
SML_API Ret_t smlSetUserData (InstanceID_t id, VoidPtr_t pUserData);
// added by luz %%%:
SML_API Ret_t smlGetUserData(InstanceID_t id, VoidPtr_t *ppUserData);
SML_API Ret_t smlGetEncoding(InstanceID_t id, SmlEncoding_t *pEncoding);
#ifndef __SML_LITE__  /* these API calls are NOT included in the Toolkit lite version */
  SML_API Ret_t smlSetEncoding (InstanceID_t id, SmlEncoding_t encoding);
#endif



/* Private function prototypes */
Ret_t freeInstanceOptions (InstanceInfoPtr_t pInstanceInfo);
static Ret_t freeInstanceInfo (InstanceInfoPtr_t pInfo);
Ret_t mgrResetWorkspace (InstanceID_t id);
Ret_t setInstanceOptions (InstanceID_t id, SmlInstanceOptionsPtr_t pOptions);


/*************************************************************************
 *  Public SyncML API Functions
 *************************************************************************/


/**
 * Creates a SyncML instance and assigns a corresponding workspace buffer in
 * which XML documents are assembled or parsed.
 * All callback functions implemented by a particular application are defined.
 * Instance specific options can be passed. This function has to be called
 * before the first synchronization tasks can be performed. A reference valid
 * for a SyncML instance is returned.
 * An instance is active when processing a synchronization request
 * otherwise it is idle. An instance is terminated when smlTerminateInstance
 * is called.
 *
 * @param pCallbacks (IN)
 *        A structure holding references to the callback functions
 *        implemented by the application
 * @param pOptions (IN)
 *        Option settings of a particular SyncML instance
 * @param pUserData (IN)
 *        UserData is a pointer to a void structure the application
 *        can pass into the SyncML Toolkit instance info. It will
 *        be returned to the application with every called callback
 *        function call!\n
 *        NOTE: This is only a pointer, the memory object itself
 *        remains within the responsibility of the calling application.
 *        The memory object will not be copied, moved or freed by the Toolkit.
 * @param pInstanceID (OUT)
 *        Instance ID assigned to the initialized instance
 * @return Error Code
 */
SML_API Ret_t smlInitInstance(SmlCallbacksCPtr_t pCallbacks, SmlInstanceOptionsPtr_t pOptions, VoidPtr_t pUserData, InstanceID_t *pInstanceID)
{

  /* --- Definitions --- */
  InstanceInfoPtr_t pInstanceInfo;
  Ret_t             rc;


  #ifndef NOWSM
    /* --- Check pOptions, which have been passed by the application --- */
    if (!pOptions || !pOptions->workspaceName)
      return SML_ERR_WRONG_USAGE;

    #ifdef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
      /* if ONE instance is already initialized */
      if (mgrGetInstanceListAnchor()!=NULL)
        return SML_ERR_WRONG_USAGE;
    #endif

    /* --- check wether we already know about this instance --- */
    #ifdef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
      pInstanceInfo = mgrGetInstanceListAnchor();
    #else
      pInstanceInfo = (InstanceInfoPtr_t) findInfo(*pInstanceID);
    #endif

    /* --- bail outh when we already have a instance with that id --- */
    if (pInstanceInfo != NULL) return SML_ERR_WRONG_USAGE;


    /* --- Create a workspace for this instance --- */
    LOCKTOOLKIT("smlInitInstance");
    if ((rc = wsmCreate(pOptions->workspaceName, pOptions->workspaceSize, pInstanceID)) != SML_ERR_OK) {
      RELEASETOOLKIT("smlInitInstance after wsmCreate failure");
      return rc;
    }
    RELEASETOOLKIT("smlInitInstance");
  #else // NOWSM
    /* --- Check pOptions, which have been passed by the application --- */
    if (!pOptions || !pOptions->workspaceSize)
      return SML_ERR_WRONG_USAGE;
    // ok so far
    rc=SML_ERR_OK;
  #endif

  /* --- Create an instance info memory object --- */
  pInstanceInfo = (InstanceInfoPtr_t)smlLibMalloc((MemSize_t)sizeof(InstanceInfo_t));
  if (pInstanceInfo==NULL) {
    #ifndef NOWSM
    wsmDestroy(pOptions->workspaceName);
    return SML_ERR_NOT_ENOUGH_SPACE;
    #endif
  }
  #ifdef NOWSM
  else {
    // instance info created, return pointer as instanceID
    *pInstanceID = (InstanceID_t)pInstanceInfo;
  }
  #endif

  smlLibMemset(pInstanceInfo,0,(MemSize_t)sizeof(InstanceInfo_t));



  /* --- Set mandatory instance infos for this instance to defaults --- */
  pInstanceInfo->status=MGR_IDLE;
  pInstanceInfo->encoderState=NULL;                  // no encoding in progress, currently not used
  pInstanceInfo->decoderState=NULL;                  // no decoding in progress, currently not used
  #ifndef NOWSM
  pInstanceInfo->id=*pInstanceID;
  pInstanceInfo->workspaceState=NULL;                // to do: some workspace status info
  pInstanceInfo->nextInfo=NULL;
  #else
  // create a instance buffer
  pInstanceInfo->instanceBufSiz=pOptions->workspaceSize; // get requested size for the buffer
  pInstanceInfo->maxOutgoingSize=pOptions->maxOutgoingSize; // set max outgoing message size
  pInstanceInfo->instanceBuffer=smlLibMalloc(pInstanceInfo->instanceBufSiz);
  if (pInstanceInfo->instanceBuffer==NULL)
    return SML_ERR_NOT_ENOUGH_SPACE;
  // init buffer pointers
  pInstanceInfo->readPointer=pInstanceInfo->instanceBuffer;
  pInstanceInfo->writePointer=pInstanceInfo->instanceBuffer;
  pInstanceInfo->readLocked=0;
  pInstanceInfo->writeLocked=0;
  pInstanceInfo->outgoingMsgStart=NULL;
  pInstanceInfo->incomingMsgStart=NULL;
  #endif


  #ifndef NOWSM
    /* --- Add instance infos memory object to the instance info list --- */
    #ifdef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
      mgrSetInstanceListAnchor(pInstanceInfo);
    #else
      rc = addInfo( pInstanceInfo );
      if (rc!=SML_ERR_OK) return rc;
    #endif
  #endif


  /* --- Set the values of instance Infos as defined by the calling application ---*/

  /* Set user data pointer */
  pInstanceInfo->userData=pUserData;
  /* Set callback functions implemented by applications */
  if (smlSetCallbacks(*pInstanceID, pCallbacks) != SML_ERR_OK) {
    #ifndef NOWSM
    wsmDestroy(pOptions->workspaceName);
    #endif
    return rc;
  }

  // luz: %%% this was called twice, probably this is a bug, so I disabled the second call
  //smlSetCallbacks(*pInstanceID, pCallbacks);

  /* Set other application defined options for that instance */
  if (setInstanceOptions (*pInstanceID, pOptions) != SML_ERR_OK) {
    #ifndef NOWSM
    wsmDestroy(pOptions->workspaceName);
    #endif
    return rc;
  }

  return SML_ERR_OK;

}



/**
 * Terminates a SyncML instance. The instance info is removed from the instances
 * list. Allmemory allocated for the workspace and the options variables is
 * freed.
 *
 * @param id (IN)
 *        ID of the instance to be terminated
 * @return Error Code
 */
SML_API Ret_t smlTerminateInstance (InstanceID_t id)
{

  /* --- Definitions --- */
  InstanceInfoPtr_t pInstanceInfo;

  #ifdef NOWSM
    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
  #else
    Ret_t             rc;

    /* --- Find that instance --- */
    #ifdef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
      pInstanceInfo = mgrGetInstanceListAnchor();
    #else
      pInstanceInfo = (InstanceInfoPtr_t) findInfo(id);
    #endif
  #endif

  if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;

  #ifndef NOWSM
    /* --- Close the workspace --- */
    if (pInstanceInfo->instanceOptions != NULL) {
      LOCKTOOLKIT("smlTerminateInstance");
      rc = wsmDestroy(pInstanceInfo->instanceOptions->workspaceName);
      RELEASETOOLKIT("smlTerminateInstance");
      if (rc!=SML_ERR_OK) {
        //    freeInstanceInfo(pInstanceInfo);
        return rc;
      }
    }

    /* --- Delete instance info and options --- */
    #ifdef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
      mgrSetInstanceListAnchor(NULL);
    #else
      removeInfo(id);
    #endif
  #endif

  freeInstanceInfo (pInstanceInfo);

  return SML_ERR_OK;
}



/**
 * Sets new callback functions to an instance
 *
 * @param id (IN)
 *        ID of the Instance
 * @param pCallbacks (IN)
 *        A structure holding references to the callback functions
 *        implemented by the application
 * @return Return value,\n
 *         SML_ERR_OK if successful
 */
SML_API Ret_t smlSetCallbacks(InstanceID_t id, SmlCallbacksCPtr_t pCallbacks)
{

  /* --- Definitions --- */
  InstanceInfoPtr_t pInstanceInfo;
  SmlCallbacksPtr_t    pCallbacksCopy;

  /* --- Check pCallbacks, which have been passed by the application --- */
  if (!pCallbacks)
    return SML_ERR_WRONG_USAGE;


  #ifdef NOWSM
    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
  #else
    /* --- Find that instance --- */
    #ifdef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
      pInstanceInfo = mgrGetInstanceListAnchor();
    #else
      pInstanceInfo = (InstanceInfoPtr_t) findInfo(id);
    #endif

    if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;
  #endif

  /* --- free old callback structure ---*/
  smlLibFree(pInstanceInfo->callbacks);


  /* --- Use a copy of pCallbacksCopy --- */
  pCallbacksCopy = (SmlCallbacksPtr_t)smlLibMalloc((MemSize_t)sizeof(SmlCallbacks_t));
  if (pCallbacksCopy==NULL) return SML_ERR_NOT_ENOUGH_SPACE;
  smlLibMemcpy(pCallbacksCopy,pCallbacks,(MemSize_t)sizeof(SmlCallbacks_t));


  /* --- set new Callbacks --- */
  pInstanceInfo->callbacks = pCallbacksCopy;

  return SML_ERR_OK;
}



/**
 * Sets a new Pointer to application specific user data,
 * which is passed to all invoked callback functions
 *
 * @param id (IN)
 *        ID of the Instance
 * @param pUserData (IN)
 *        UserData is a pointer to a void structure the application
 *        can pass into the SyncML Toolkit instance info. It will
 *        be returned to the application with every called callback
 *        function call!\n
 *        NOTE: This is only a pointer, the memory object itself
 *        remains within the responsibility of the calling application.
 *        The memory object will not be copied, moved or freed by the Toolkit.
 * @return Return value,\n
 *         SML_ERR_OK if successful
 */
SML_API Ret_t smlSetUserData(InstanceID_t id, VoidPtr_t pUserData)
{

  /* --- Definitions --- */
  InstanceInfoPtr_t pInstanceInfo;


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


  /* --- set new user data pointer ---*/
  pInstanceInfo->userData=pUserData;

  return SML_ERR_OK;
}


/**
 * Returns Pointer to application specific user data,
 * which is passed to all invoked callback functions
 *
 * @param id (IN)
 *        ID of the Instance
 * @param ppUserData (OUT)
 *        Receives current Userdata pointer
 * @return Return value,\n
 *         SML_ERR_OK if successful
 * @note (added by luz %%%)
 */
SML_API Ret_t smlGetUserData(InstanceID_t id, VoidPtr_t *ppUserData)
{

  /* --- Definitions --- */
  InstanceInfoPtr_t pInstanceInfo;

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


  /* --- get userdata pointer ---*/
  *ppUserData = pInstanceInfo->userData;

  return SML_ERR_OK;
} // smlGetUserData


/**
 * Returns Currently set encoding type
 *
 * @param id (IN)
 *        ID of the Instance
 * @param pEncoding (OUT)
 *        Receives current encoding
 * @return Return value,\n
 *         SML_ERR_OK if successful
 * @note (added by luz %%%)
 */
SML_API Ret_t smlGetEncoding(InstanceID_t id, SmlEncoding_t *pEncoding)
{

  /* --- Definitions --- */
  InstanceInfoPtr_t pInstanceInfo;

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

  /* --- get encoding ---*/
  *pEncoding = pInstanceInfo->instanceOptions->encoding;

  return SML_ERR_OK;
} // smlGetEncoding


/**
 * Sets new encoding type for this Instance
 *
 * @param id (IN)
 *        ID of the Instance
 * @param encoding (IN)
 *        Type of Encoding to be used within this Instance
 * @return Return value,\n
 *         SML_ERR_OK if successful
 */
#ifndef __SML_LITE__  /* these API calls are NOT included in the Toolkit lite version */
SML_API Ret_t smlSetEncoding(InstanceID_t id, SmlEncoding_t encoding)
{

  /* --- Definitions --- */
  InstanceInfoPtr_t pInstanceInfo;

  /* --- Check pCallbacks, which have been passed by the application --- */
  if (encoding==SML_UNDEF)
    return SML_ERR_WRONG_USAGE;


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


  /* --- free old callback structure ---*/
  pInstanceInfo->instanceOptions->encoding = encoding;

  return SML_ERR_OK;
}
#endif




/**
 * Locks the workspace buffer, which is assigned to the given instance
 * for reading. After this function is called, the application has
 * access to the workspace buffer, beginning at the address pReadPosition which
 * is returned by this function. SyncML will not change the workspace
 * buffer until smlUnlockReadBuffer is called.
 * pReadPosition returns a pointer to a valid position in the SyncML workspace
 * buffer. The pointer can be used by the application for copying outgoing
 * synchronization data from the buffer into some transport layer. usedSize
 * retrieves the size of synchronization data currently stored in the
 * workspace buffer beginning from the address to which pReadPosition points to.
 * This information is needed by the application when copying XML code out
 * of the buffer (while sending synchronization data)
 *
 * @param id (IN)
 *        ID of the Instance
 * @param pReadPosition (OUT)
 *        Workspace Pointer from which data can be read
 * @param usedSize (OUT)
 *        Size of used data in workspace which may be read
 * @return Return value,\n
 *         SML_ERR_OK if successful
 */
SML_API Ret_t smlLockReadBuffer(InstanceID_t id, MemPtr_t *pReadPosition, MemSize_t *usedSize)
{
  #ifdef NOWSM
    InstanceInfoPtr_t pInstanceInfo;

    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
    if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;
    // must not be already locked here
    if (pInstanceInfo->readLocked)
      return SML_ERR_WRONG_USAGE;
    // everything that is already written can also be read
    *pReadPosition = pInstanceInfo->readPointer;
    // used portion is what is between read and write pointers
    *usedSize = pInstanceInfo->writePointer-pInstanceInfo->readPointer;
    // lock
    pInstanceInfo->readLocked=1;
  #else
    Ret_t rc;

    LOCKTOOLKIT("smlLockReadBuffer");
    /* --- Lock Workspace exclusively for reading and get a "Read" pointer --- */
    rc = wsmLockH(id, SML_FIRST_DATA_ITEM, pReadPosition);
    RELEASETOOLKIT("smlLockReadBuffer");
    if (rc!=SML_ERR_OK) return rc;

    /* --- Check, how much data has to be read ---*/
    LOCKTOOLKIT("smlLockReadBuffer");
    rc = wsmGetUsedSize(id,usedSize);
    RELEASETOOLKIT("smlLockReadBuffer");
    if (rc!=SML_ERR_OK) return rc;
  #endif

  return SML_ERR_OK;
}




/**
 * End the read access of the application to the workspace buffer.
 * SyncML is now owner of the buffer again and is able to manipulate its contents.
 * processedBytes passes the number of bytes, which the application has
 * successfully read and processed (e.g. when the application has copied
 * outgoing synchronization data from the workspace into a communication module).
 * SyncML removes the given number of bytes from the workspace!
 *
 * @param id (IN)
 *        ID of the Instance
 * @param processedBytes (IN)
 *        Actually read and processed bytes
 * @return Return value,\n
 *         SML_ERR_OK if successful
 */
SML_API Ret_t smlUnlockReadBuffer(InstanceID_t id, MemSize_t processedBytes)
{
  #ifdef NOWSM
    InstanceInfoPtr_t pInstanceInfo;

    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
    if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;
    // must be already locked here
    if (!pInstanceInfo->readLocked)
      return SML_ERR_WRONG_USAGE;
    // advance read pointer by number of bytes processed
    if (pInstanceInfo->readPointer+processedBytes>pInstanceInfo->writePointer)
      return SML_ERR_WRONG_USAGE; // too many bytes processed
    // update read pointer
    pInstanceInfo->readPointer+=processedBytes;
    /* %%% moved from here to smlLockWriteBuffer to leave buffer intact until we actually
       need to write more data (important for re-sending retries)
    // auto-reset pointers if we have now read everything
    if (pInstanceInfo->readPointer ==  pInstanceInfo->writePointer) {
      // clear the buffer
      mgrResetWorkspace(pInstanceInfo);
    }
    */
    // unlock
    pInstanceInfo->readLocked=0;
  #else
    Ret_t rc;

    /* --- Pass the number of bytes which have been read --- */
    LOCKTOOLKIT("smlUnlockReadBuffer");
    rc = wsmProcessedBytes (id,processedBytes);
    RELEASETOOLKIT("smlUnlockReadBuffer");
    if (rc!=SML_ERR_OK) return rc;

    /* --- Unlock Workspace --- */
    LOCKTOOLKIT("smlUnlockReadBuffer");
    rc = wsmUnlockH(id);
    RELEASETOOLKIT("smlUnlockReadBuffer");
    if (rc!=SML_ERR_OK) return rc;
  #endif

  return SML_ERR_OK;
}


#ifdef NOWSM

/**
 * Set max outgoing message size
 *
 * @param id (IN)
 *        ID of the Instance
 * @param maxOutgoingSize (IN)
 *        maximum size of outgoing message
 *        (0=no limit except buffer size)
 * @return Return value,\n
 *         SML_ERR_OK if successful
 */
SML_API Ret_t smlSetMaxOutgoingSize(InstanceID_t id, MemSize_t maxOutgoingSize)
{
  InstanceInfoPtr_t pInstanceInfo;

  pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
  if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;

  // set max outgoing message size
  pInstanceInfo->maxOutgoingSize = maxOutgoingSize;

  return SML_ERR_OK;
}


/**
 * Marks the current write pointer position as beginning of a new outgoing
 * message. This is used to track outgoing message size while writing it
 *
 * @param id (IN)
 *        ID of the Instance
 * @return Return value,\n
 *         SML_ERR_OK if successful
 */
SML_API Ret_t smlSetOutgoingBegin(InstanceID_t id)
{
  InstanceInfoPtr_t pInstanceInfo;

  pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
  if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;

  // remember current write pointer
  pInstanceInfo->outgoingMsgStart=pInstanceInfo->writePointer;

  return SML_ERR_OK;
}


/**
 * resets the read pointer to the beginning of the outgoing message
 * (must be set previously using smlSetOutgoingBegin())
 * Note: this can also be used to make sure next smlLockReadBuffer()
 *       does not return unprocessed garbage from a message
 *       written previously to the buffer (as it can be the case
 *       when using the same sml instance for both request and
 *       response processing, as in libsynthesis). In this case
 *       calling smlReadOutgoingAgain() ADVANCES the read
 *       pointer past unprocessed garbage to the point in the
 *       buffer where the next outgoing message starts.
 *
 * @param id (IN)
 *        ID of the Instance
 * @return Return value,\n
 *         SML_ERR_OK if successful
 */
SML_API Ret_t smlReadOutgoingAgain(InstanceID_t id)
{
  InstanceInfoPtr_t pInstanceInfo;

  pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
  if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;

  // reset read pointer back to beginning of outgoing message (can be used for message retries)
  if (pInstanceInfo->outgoingMsgStart==NULL) return SML_ERR_WRONG_USAGE; // no outgoing start set
  pInstanceInfo->readPointer=pInstanceInfo->outgoingMsgStart;

  return SML_ERR_OK;
}


/**
 * gets pointer to message in buffer for dumping purposes
 * @note This is only valid for incoming messages BEFORE outgoing message writing has started
 *       into the same instance buffer!
 *
 * @param id (IN)
 *        ID of the Instance
 * @param outgoing (IN)
 *        if set, outgoing message is returned, incoming otherwise
 * @param message (OUT)
 *        pointer to message
 * @param msgsize (OUT)
 *        message size
 * @return Return value,\n
 *         SML_ERR_OK if successful
 */
SML_API Ret_t smlPeekMessageBuffer(InstanceID_t id, Boolean_t outgoing, MemPtr_t *message, MemSize_t *msgsize)
{
  InstanceInfoPtr_t pInstanceInfo;

  pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
  if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;

  if (outgoing) {
    if (pInstanceInfo->outgoingMsgStart==NULL) return SML_ERR_WRONG_USAGE; // no outgoing message yet
    *message = pInstanceInfo->outgoingMsgStart;
  }
  else {
    if (pInstanceInfo->incomingMsgStart) {
    	// incoming message start already detected, return it
	    *message = pInstanceInfo->incomingMsgStart;
    }
    else {    
    	// apparently not started analyzing, assume read pointer as beginning of message
      if (pInstanceInfo->readPointer==0)
      	return SML_ERR_WRONG_USAGE; // no outgoing message yet
      *message = pInstanceInfo->readPointer;
    }
  }
  if (*message>=pInstanceInfo->writePointer) return SML_ERR_WRONG_USAGE; // invalid pointer positions
  // size is space between start and write pointer
  *msgsize = pInstanceInfo->writePointer-*message;
  return SML_ERR_OK;
}


#endif

/**
 * Locks the workspace buffer, which is assigned to the given
 * instance for writing. After this function is called, the
 * application has access to the workspace buffer, beginning
 * at the address pWritePosition which is returned by this
 * function. SyncML will not change the workspace buffer until
 * smlUnlockWriteBuffer is called.
 * pWritePosition returns a pointer to a valid position in the
 * SyncML workspace buffer. The pointer can be used by the application
 * for copying incoming synchronization data from some transport
 * layer into the buffer. freeSize retrieves the maximum usable
 * size of the workspace buffer beginning from the address to
 * which pWritePosition points to. This information is needed by
 * the application when copying XML code into the buffer (while
 * receiving synchronization data)
 *
 * @param id (IN)
 *        ID of the Instance
 * @param pWritePosition (OUT)
 *        Workspace Pointer to which data can be written
 * @param freeSize (OUT)
 *        Max free Size of available space for data
 * @return Return value,\n
 *         SML_ERR_OK if successful
 */
SML_API Ret_t smlLockWriteBuffer(InstanceID_t id, MemPtr_t *pWritePosition, MemSize_t *freeSize)
{
  #ifdef NOWSM
    InstanceInfoPtr_t pInstanceInfo;
    Boolean_t ogs;

    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
    if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;
    // must not be already locked here
    if (pInstanceInfo->writeLocked)
      return SML_ERR_WRONG_USAGE;
    // auto-reset pointers if buffer is empty by now
    if (pInstanceInfo->readPointer ==  pInstanceInfo->writePointer) {
      // remember if we were at outgoing message start point
      ogs = pInstanceInfo->outgoingMsgStart == pInstanceInfo->writePointer;
      // clear the buffer
      mgrResetWorkspace(pInstanceInfo);
      // restore outgoingMsgStart (to beginning of buffer now!) if it was set before
      if (ogs) pInstanceInfo->outgoingMsgStart=pInstanceInfo->writePointer;
    }

    // return current write pointer
    *pWritePosition = pInstanceInfo->writePointer;
    // free portion is either determined by actual room in buffer, or maximum outgoing size if set
    if (
      pInstanceInfo->maxOutgoingSize &&
      pInstanceInfo->outgoingMsgStart &&
      pInstanceInfo->outgoingMsgStart<pInstanceInfo->writePointer
    ) {
      // calculate what is allowed according to maxOutgoingSize
      *freeSize =
        (pInstanceInfo->maxOutgoingSize) - // maximum outgoing size
        (pInstanceInfo->writePointer-pInstanceInfo->outgoingMsgStart); // size of outgoing message so far
      if (pInstanceInfo->writePointer+*freeSize > pInstanceInfo->instanceBuffer+pInstanceInfo->instanceBufSiz) {
        // actual space in buffer is smaller
        *freeSize =
          (pInstanceInfo->instanceBuffer+pInstanceInfo->instanceBufSiz) - // end of buffer
          pInstanceInfo->writePointer; // current write position
      }
    }
    else {
      // simply return available size in buffer
      *freeSize =
        (pInstanceInfo->instanceBuffer+pInstanceInfo->instanceBufSiz) - // end of buffer
        pInstanceInfo->writePointer; // current write position
    }
    // lock
    pInstanceInfo->writeLocked=1;
  #else
    Ret_t rc;

    /* --- Lock Workspace exclusively for writing and get a "Write" pointer --- */
    LOCKTOOLKIT("smlLockWriteBuffer");
    rc = wsmLockH(id, SML_FIRST_FREE_ITEM, pWritePosition);
    RELEASETOOLKIT("smlLockWriteBuffer");
    if (rc!=SML_ERR_OK) return rc;

    /* --- Check, how much free space is available for writing --- */
    LOCKTOOLKIT("smlLockWriteBuffer");
    rc = wsmGetFreeSize(id, freeSize);
    RELEASETOOLKIT("smlLockWriteBuffer");
    if (rc!=SML_ERR_OK) return rc;
  #endif

  return SML_ERR_OK;
}




/**
 *
 * End the write access of the application to the workspace buffer.
 * SyncML is now owner of the buffer again and is able to manipulate its
 * contents. writtenBytes passes the number of bytes which have been
 * written into the workspace buffer (e.g. when the application has copied
 * incoming synchronization data from a communication module into the
 * workspace). This information is needed by SyncML when processing received
 * synchronization data.
 *
 * @param id (IN)
 *        ID of the Instance
 * @param writtenBytes (IN)
 *        Actually written bytes
 * @return Return value,\n
 *         SML_ERR_OK if successful
 */
SML_API Ret_t smlUnlockWriteBuffer(InstanceID_t id, MemSize_t writtenBytes)
{
  #ifdef NOWSM
    InstanceInfoPtr_t pInstanceInfo;

    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
    if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;
    // must be already locked here
    if (!pInstanceInfo->writeLocked)
      return SML_ERR_WRONG_USAGE;
    if (writtenBytes > 0) {
      // advance write pointer by number of bytes written
      if (pInstanceInfo->writePointer+writtenBytes>pInstanceInfo->instanceBuffer+pInstanceInfo->instanceBufSiz)
        return SML_ERR_WRONG_USAGE; // too many bytes written
      // update write pointer
      pInstanceInfo->writePointer+=writtenBytes;
    }
    // unlock
    pInstanceInfo->writeLocked=0;
  #else
    Ret_t rc;

    if (writtenBytes > 0)
    {
      /* --- Pass the number of bytes which have been written --- */
      LOCKTOOLKIT("smlUnlockWriteBuffer");
      rc = wsmSetUsedSize(id,writtenBytes);
      RELEASETOOLKIT("smlUnlockWriteBuffer");
      if (rc!=SML_ERR_OK) return rc;
    }
    /* --- Unlock Workspace --- */
    LOCKTOOLKIT("smlUnlockWriteBuffer");
    rc = wsmUnlockH(id);
    RELEASETOOLKIT("smlUnlockWriteBuffer");
    if (rc!=SML_ERR_OK) return rc;
  #endif

  return SML_ERR_OK;
}




/*************************************************************************
 *  SyncML internal functions
 *************************************************************************/


/**
 * Reset the Workspace Buffer position to the beginning of the workspace
 * (clears all data in the buffer)
 *
 * @param id (IN)
 *        ID of the Instance
 * @return Return value,\n
 *         SML_ERR_OK if successful
 */
Ret_t mgrResetWorkspace (InstanceID_t id) {
  #ifdef NOWSM
    InstanceInfoPtr_t pInstanceInfo;

    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
    if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;

    pInstanceInfo->readPointer=pInstanceInfo->instanceBuffer;
    pInstanceInfo->writePointer=pInstanceInfo->instanceBuffer;
    pInstanceInfo->outgoingMsgStart=NULL; // no outgoing message in the buffer
    pInstanceInfo->incomingMsgStart=NULL; // no incoming message in the buffer
    return SML_ERR_OK; // ok
  #else
    Ret_t rc;
    LOCKTOOLKIT("mgrResetWorkspace");
    rc=wsmReset (id);
    RELEASETOOLKIT("mgrResetWorkspace");
    return rc;
  #endif
}



/**
 * The options settings of an instance are set to a new value
 *
 * @param id (IN)
 *        Instance ID assigned to the instance
 * @param pOptions (IN)
 *        New option settings of that particular SyncML instance\n
 *        NOTE: only the encoding can be changed during life-time of an
 *        instance. The other parameters of the instance options
 *        (workspace size and name cannot be changed)
 * @return Error Code
 */
Ret_t setInstanceOptions (InstanceID_t id, SmlInstanceOptionsPtr_t pOptions)
{

  /* --- Definitions --- */
  InstanceInfoPtr_t         pInstanceInfo;
  SmlInstanceOptionsPtr_t      pOptionsCopy;


  #ifdef NOWSM
    /* --- Ckeck pOptions, which have been passed by the application --- */
    if (!pOptions || (pOptions->encoding==SML_UNDEF))
      return SML_ERR_WRONG_USAGE;

    pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
  #else
    /* --- Ckeck pOptions, which have been passed by the application --- */
    if (!pOptions || !pOptions->workspaceName|| (pOptions->encoding==SML_UNDEF))
      return SML_ERR_WRONG_USAGE;

    /* --- Find that instance --- */
    #ifdef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */
      pInstanceInfo = mgrGetInstanceListAnchor();
    #else
      pInstanceInfo = (InstanceInfoPtr_t) findInfo(id);
    #endif
  #endif

  if (pInstanceInfo==NULL) return SML_ERR_MGR_INVALID_INSTANCE_INFO;

  /* --- free old instance options ---*/
  freeInstanceOptions(pInstanceInfo);

  /* --- Use a copy of pOptionsCopy --- */
  pOptionsCopy = (SmlInstanceOptionsPtr_t)smlLibMalloc((MemSize_t)sizeof(SmlInstanceOptions_t));
  if (pOptionsCopy==NULL) return SML_ERR_NOT_ENOUGH_SPACE;
  smlLibMemcpy(pOptionsCopy,pOptions,(MemSize_t)sizeof(SmlInstanceOptions_t));

  #ifndef NOWSM
  pOptionsCopy->workspaceName=smlLibStrdup(pOptions->workspaceName);

  if (pOptionsCopy->workspaceName == NULL) {
    pInstanceInfo->instanceOptions=NULL;
    smlLibFree(pOptionsCopy);
    return SML_ERR_NOT_ENOUGH_SPACE;
   }
  #endif

  /* --- Assign the new options --- */
  pInstanceInfo->instanceOptions=pOptionsCopy;


  /* --- Let the new settingds take effect --- */
  /* --- Adjust workspace size ---*/
  /* --- Change workspace name ---*/
  // NOT SUPPORTED FOR YELLOW

  return SML_ERR_OK;
}



/**
 * Free Instances Options
 *
 * @param pInfo (IN)
 *        Pointer to the pInstance Info, which options should be freed
 * @return SML_ERR_OK
 */
Ret_t freeInstanceOptions (InstanceInfoPtr_t pInfo) {

  /* --- Delete instance options (if there are any) --- */
  if (pInfo->instanceOptions!=NULL) {
    #ifndef NOWSM
    if (pInfo->instanceOptions->workspaceName!=NULL)
      smlLibFree(pInfo->instanceOptions->workspaceName);  // don't forget the substructures
    #endif
    smlLibFree(pInfo->instanceOptions);
  }

 return SML_ERR_OK;
}

/**
 * Free the memory of an removed Instance Info
 * (including referenced sub structures)
 *
 * @param id (IN)
 *        ID of the InstanceInfo structure to be freed
 */
static Ret_t freeInstanceInfo(InstanceInfoPtr_t pInfo) {

  if (pInfo) {

    #ifdef NOWSM
    // return the instance buffer
    if (pInfo->instanceBuffer)
      smlLibFree(pInfo->instanceBuffer);
    #else
    if (pInfo->workspaceState)
      smlLibFree(pInfo->workspaceState);
    #endif
    if (pInfo->encoderState)
      xltEncReset((XltEncoderPtr_t)pInfo->encoderState);
    if (pInfo->decoderState)
      xltDecReset((XltDecoderPtr_t)pInfo->decoderState);
    if (pInfo->callbacks)
      smlLibFree(pInfo->callbacks);

    freeInstanceOptions(pInfo);

    smlLibFree(pInfo);
  }

  return SML_ERR_OK;
}
