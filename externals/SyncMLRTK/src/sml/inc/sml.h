/**
 * @file
 * External SyncML API
 *
 * @target_system   all
 * @target_os       all
 * @description Platform independent definition of the SyncML API functions
 * This is the external API exposed to applications using SyncML
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


#ifndef _SML_H
  #define _SML_H


/*************************************************************************
 *  Definitions
 *************************************************************************/

#include <smldef.h>
#include <smldtd.h>




/*************************************************************************
 *  External Functions
 *************************************************************************/

#if !defined(NOWSM) || !__LINK_TOOLKIT_STATIC__

/*
 * ============================
 * General SyncML Functions
 * ============================
 */
SML_API_DEF Ret_t smlInit(SmlOptionsPtr_t pOptions);
SML_API_DEF Ret_t smlSetSyncMLOptions (SmlOptionsPtr_t pOptions);
SML_API_DEF Ret_t smlTerminate(void);

#endif


/*
 * ============================
 * Workspace Handling Functions
 * ============================
 */

SML_API_DEF Ret_t smlLockReadBuffer(InstanceID_t id, MemPtr_t *pReadPosition, MemSize_t *usedSize);
SML_API_DEF Ret_t smlUnlockReadBuffer(InstanceID_t id, MemSize_t processedBytes);
#ifdef NOWSM
SML_API Ret_t smlSetMaxOutgoingSize(InstanceID_t id, MemSize_t maxOutgoingSize);
SML_API Ret_t smlSetOutgoingBegin(InstanceID_t id);
SML_API Ret_t smlReadOutgoingAgain(InstanceID_t id);
SML_API Ret_t smlPeekMessageBuffer(InstanceID_t id, Boolean_t outgoing, MemPtr_t *message, MemSize_t *msgsize);
#endif
SML_API_DEF Ret_t smlLockWriteBuffer(InstanceID_t id, MemPtr_t *pWritePosition, MemSize_t *freeSize);
SML_API_DEF Ret_t smlUnlockWriteBuffer(InstanceID_t id, MemSize_t writtenBytes);



/*
 * ===========================================
 * Protocol Element Building Functions (for Originator)
 * ===========================================
 */

/* Protocol Management */
SML_API_DEF Ret_t smlStartMessage(InstanceID_t id, SmlSyncHdrPtr_t pContent);
SML_API_DEF Ret_t smlStartMessageExt(InstanceID_t id, SmlSyncHdrPtr_t pContent, SmlVersion_t vers); /* %%% added by luz 2003-08-06) */
SML_API_DEF Ret_t smlEndMessage(InstanceID_t id, Boolean_t final);
SML_API_DEF Ret_t smlStartSync(InstanceID_t id, SmlSyncPtr_t pContent);
SML_API_DEF Ret_t smlEndSync(InstanceID_t id);

#ifdef ATOMIC_SEND  /* these API calls are NOT included in the Toolkit lite version */
  SML_API_DEF Ret_t smlStartAtomic(InstanceID_t id, SmlAtomicPtr_t pContent);
  SML_API_DEF Ret_t smlEndAtomic(InstanceID_t id);
#endif
#ifdef SEQUENCE_SEND
  SML_API_DEF Ret_t smlStartSequence(InstanceID_t id, SmlSequencePtr_t pContent);
  SML_API_DEF Ret_t smlEndSequence(InstanceID_t id);
#endif

/* Sync Commands */
#ifdef ADD_SEND
  SML_API_DEF Ret_t smlAddCmd(InstanceID_t id, SmlAddPtr_t pContent);
#endif
SML_API_DEF Ret_t smlAlertCmd(InstanceID_t id, SmlAlertPtr_t pContent);
SML_API_DEF Ret_t smlDeleteCmd(InstanceID_t id, SmlDeletePtr_t pContent);
#ifdef GET_SEND
  SML_API_DEF Ret_t smlGetCmd(InstanceID_t id, SmlGetPtr_t pContent);
#endif
SML_API_DEF Ret_t smlPutCmd(InstanceID_t id, SmlPutPtr_t pContent);
SML_API_DEF Ret_t smlMapCmd(InstanceID_t id, SmlMapPtr_t pContent);
SML_API_DEF Ret_t smlResultsCmd(InstanceID_t id, SmlResultsPtr_t pContent);
SML_API_DEF Ret_t smlStatusCmd(InstanceID_t id, SmlStatusPtr_t pContent);
SML_API_DEF Ret_t smlReplaceCmd(InstanceID_t id, SmlReplacePtr_t pContent);

#ifdef COPY_SEND  /* these API calls are NOT included in the Toolkit lite version */
  SML_API_DEF Ret_t smlCopyCmd(InstanceID_t id, SmlCopyPtr_t pContent);
#endif
SML_API_DEF Ret_t smlMoveCmd(InstanceID_t id, SmlMovePtr_t pContent);
#ifdef EXEC_SEND
  SML_API_DEF Ret_t smlExecCmd(InstanceID_t id, SmlExecPtr_t pContent);
#endif
#ifdef SEARCH_SEND
  SML_API_DEF Ret_t smlSearchCmd(InstanceID_t id, SmlSearchPtr_t pContent);
#endif
SML_API_DEF Ret_t smlStartEvaluation(InstanceID_t id);
SML_API_DEF Ret_t smlEndEvaluation(InstanceID_t id, MemSize_t *freemem);


/*
 * ============================================
 * Command Dispatching Functions (for Receiver)
 * ============================================
 */

SML_API_DEF Ret_t smlProcessData(InstanceID_t id, SmlProcessMode_t mode);



/*
 * ====================================================
 * Callback Functions to be implemented by the Receiver
 * ====================================================
 */

/* Protocol Management */
typedef Ret_t (*smlStartMessageFunc) (InstanceID_t id, VoidPtr_t userData, SmlSyncHdrPtr_t pContent);
typedef Ret_t (*smlEndMessageFunc) (InstanceID_t id, VoidPtr_t userData, Boolean_t final);
typedef Ret_t (*smlStartSyncFunc) (InstanceID_t id, VoidPtr_t userData, SmlSyncPtr_t pContent);
typedef Ret_t (*smlEndSyncFunc) (InstanceID_t id, VoidPtr_t userData);

#ifdef ATOMIC_RECEIVE  /* these callbacks are NOT included in the Toolkit lite version */
  typedef Ret_t (*smlStartAtomicFunc) (InstanceID_t id, VoidPtr_t userData, SmlAtomicPtr_t pContent);
  typedef Ret_t (*smlEndAtomicFunc) (InstanceID_t id, VoidPtr_t userData);
#endif
#ifdef SEQUENCE_RECEIVE
  typedef Ret_t (*smlStartSequenceFunc) (InstanceID_t id, VoidPtr_t userData, SmlSequencePtr_t pContent);
  typedef Ret_t (*smlEndSequenceFunc) (InstanceID_t id, VoidPtr_t userData);
#endif

/* Sync Commands */
typedef Ret_t (*smlAddCmdFunc) (InstanceID_t id, VoidPtr_t userData, SmlAddPtr_t pContent);
typedef Ret_t (*smlAlertCmdFunc) (InstanceID_t id, VoidPtr_t userData, SmlAlertPtr_t pContent);
typedef Ret_t (*smlDeleteCmdFunc) (InstanceID_t id, VoidPtr_t userData, SmlDeletePtr_t pContent);
typedef Ret_t (*smlGetCmdFunc) (InstanceID_t id, VoidPtr_t userData, SmlGetPtr_t pContent);
typedef Ret_t (*smlPutCmdFunc) (InstanceID_t id, VoidPtr_t userData, SmlPutPtr_t pContent);
#ifdef MAP_RECEIVE
  typedef Ret_t (*smlMapCmdFunc) (InstanceID_t id, VoidPtr_t userData, SmlMapPtr_t pContent);
#endif
#ifdef RESULT_RECEIVE
  typedef Ret_t (*smlResultsCmdFunc) (InstanceID_t id, VoidPtr_t userData, SmlResultsPtr_t pContent);
#endif
typedef Ret_t (*smlStatusCmdFunc) (InstanceID_t id, VoidPtr_t userData, SmlStatusPtr_t pContent);
typedef Ret_t (*smlReplaceCmdFunc) (InstanceID_t id, VoidPtr_t userData, SmlReplacePtr_t pContent);

#ifdef COPY_RECEIVE  /* these callbacks are NOT included in the Toolkit lite version */
  typedef Ret_t (*smlCopyCmdFunc) (InstanceID_t id, VoidPtr_t userData, SmlCopyPtr_t param);
#endif
#ifdef EXEC_RECEIVE
  typedef Ret_t (*smlExecCmdFunc) (InstanceID_t id, VoidPtr_t userData, SmlExecPtr_t pContent);
#endif
#ifdef SEARCH_RECEIVE
  typedef Ret_t (*smlSearchCmdFunc) (InstanceID_t id, VoidPtr_t userData, SmlSearchPtr_t pContent);
#endif
  typedef Ret_t (*smlMoveCmdFunc)  (InstanceID_t id, VoidPtr_t userData, SmlMovePtr_t param);


/* Other Callbacks */
typedef Ret_t (*smlHandleErrorFunc) (InstanceID_t id, VoidPtr_t userData);
typedef Ret_t (*smlTransmitChunkFunc) (InstanceID_t id, VoidPtr_t userData);




/**
 * Structure defining references to the applications callback implementations
 **/
typedef struct sml_callbacks_s {
  /* Protocol Management Callbacks */
  smlStartMessageFunc    startMessageFunc;
  smlEndMessageFunc      endMessageFunc;
  smlStartSyncFunc       startSyncFunc;
  smlEndSyncFunc         endSyncFunc;
#ifdef ATOMIC_RECEIVE  /* these callbacks are NOT included in the Toolkit lite version */
    smlStartAtomicFunc     startAtomicFunc;
    smlEndAtomicFunc       endAtomicFunc;
#endif
#ifdef SEQUENCE_RECEIVE
    smlStartSequenceFunc   startSequenceFunc;
    smlEndSequenceFunc     endSequenceFunc;
#endif
  /* Sync Command callbacks */
  smlAddCmdFunc          addCmdFunc;
  smlAlertCmdFunc        alertCmdFunc;
  smlDeleteCmdFunc       deleteCmdFunc;
  smlGetCmdFunc          getCmdFunc;
  smlPutCmdFunc          putCmdFunc;
#ifdef MAP_RECEIVE
  smlMapCmdFunc          mapCmdFunc;
#endif
#ifdef RESULT_RECEIVE
  smlResultsCmdFunc      resultsCmdFunc;
#endif
  smlStatusCmdFunc       statusCmdFunc;
  smlReplaceCmdFunc      replaceCmdFunc;
#ifdef COPY_RECEIVE  /* these callbacks are NOT included in the Toolkit lite version */
    smlCopyCmdFunc         copyCmdFunc;
#endif
#ifdef EXEC_RECEIVE
    smlExecCmdFunc         execCmdFunc;
#endif
#ifdef SEARCH_RECEIVE
    smlSearchCmdFunc       searchCmdFunc;
#endif
  smlMoveCmdFunc         moveCmdFunc;
  /* Other Callbacks */
  smlHandleErrorFunc     handleErrorFunc;
  smlTransmitChunkFunc   transmitChunkFunc;
  //smlPrintFunc           printFunc;
} *SmlCallbacksPtr_t, SmlCallbacks_t;

typedef const SmlCallbacks_t *SmlCallbacksCPtr_t;


/*
 * ============================
 * Instance Management Functions
 * ============================
 */

SML_API_DEF Ret_t smlInitInstance(SmlCallbacksCPtr_t pCallbacks, SmlInstanceOptionsPtr_t pOptions, VoidPtr_t pUserData, InstanceID_t *pId);
SML_API_DEF Ret_t smlTerminateInstance (InstanceID_t id);
SML_API_DEF Ret_t smlSetCallbacks (InstanceID_t id, SmlCallbacksCPtr_t pCallbacks);
SML_API_DEF Ret_t smlSetUserData (InstanceID_t id, VoidPtr_t pUserData);
// added by luz %%%:
SML_API Ret_t smlGetUserData(InstanceID_t id, VoidPtr_t *ppUserData);
SML_API Ret_t smlGetEncoding(InstanceID_t id, SmlEncoding_t *pEncoding);

#ifndef __SML_LITE__  /* these API calls are NOT included in the Toolkit lite version */
  SML_API_DEF Ret_t smlSetEncoding (InstanceID_t id, SmlEncoding_t encoding);
#endif



/*
 * ===================================
 * Some Helper Functions and Utilities
 * ===================================
 */

SML_API_DEF Ret_t smlFreeProtoElement(VoidPtr_t pProtoElement);
SML_API_DEF void smlFreePcdata(SmlPcdataPtr_t pPcdata);
#ifndef __SML_LITE__  /* these API calls are NOT included in the Toolkit lite version */
  SML_API_DEF String_t smlPcdata2String( SmlPcdataPtr_t pcdata );
  SML_API_DEF SmlPcdataPtr_t smlString2Pcdata( String_t str );
  SML_API_DEF SmlPcdataPtr_t smlPcdataDup(SmlPcdataPtr_t pcdata);
  SML_API_DEF MemSize_t smlGetFreeBuffer(InstanceID_t id);
#endif

#ifndef __PALM_OS__  /* these API calls are NOT exported in the Palm OS version */
SML_API_DEF void  *smlLibMemset(void *pObject, int value, MemSize_t count);
SML_API_DEF void  *smlLibMemcpy(void *pTarget, const void *pSource, MemSize_t count);
SML_API_DEF int   smlLibMemcmp(const void *pTarget, const void *pSource, MemSize_t count);
//SML_API_DEF void  *smlLibMalloc(MemSize_t size);
SML_API_DEF void  smlLibFree(void *pObject);
SML_API_DEF MemSize_t smlLibMemsize(const void *pObject);

SML_API_DEF String_t  smlLibStrdup (const char *constStringP);
SML_API_DEF String_t  smlLibStrcpy(const char *pTarget, const char *pSource);
SML_API_DEF int   smlLibStrcmp(const char *pTarget, const char *pSource);
SML_API_DEF int   smlLibStrlen(const char *pString);
SML_API_DEF String_t  smlLibStrncpy(const char *pTarget, const char *pSource, int count);
SML_API_DEF int   smlLibStrncmp(const char *pTarget, const char *pSource, int count);
SML_API_DEF String_t  smlLibStrcat(const char *pTarget, const char *pSource);
SML_API_DEF String_t  smlLibStrchr(const char *pString, char character);
#ifndef __SML_LITE__  /* these API calls are NOT included in the Toolkit lite version */
  SML_API_DEF void  smlLibPrint(const char *text, ...);
  SML_API_DEF void  *smlLibMemmove(void *pTarget, const void *pSource, MemSize_t count);
  SML_API_DEF void  *smlLibRealloc(void *pObject, MemSize_t size);
  SML_API_DEF String_t  smlLibStrncat(const char *pTarget, const char *pSource, int count);
  SML_API_DEF String_t  smlLibStrstr(const char *pString, const char *pSubString);
#endif
#endif



#endif
