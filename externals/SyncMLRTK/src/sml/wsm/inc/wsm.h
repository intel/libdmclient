/**
 * @file
 * SyncML WorkSpace Manager
 *
 * @target_system   All
 * @target_os       All
 * @description Workspace Manager API
 * Manages the SyncML document in memory.
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



#ifndef _WSM_H
#define _WSM_H

#include "smlerr.h"
#include "smldef.h"

#include "syncml_tk_prefix_file.h" // %%% luz: needed for precompiled headers in eVC++

#ifndef NOWSM

#include "wsm_sm.h"


typedef struct WsmOptions_s {
  MemSize_t maxAvailMem;   /**< maximum amount of memory available for all wsm buffers */
} WsmOptions_t;


#ifdef __SML_LITE__
  #ifdef MAX_WSM_BUFFERS
    #error "for __SML_LITE__, MAX_WSM_BUFFERS must not be predefined!"
  #endif
  #define MAX_WSM_BUFFERS 1
#else
  #ifndef MAX_WSM_BUFFERS
    // use default value of 4 (not much for a multi-connection server)
    #define MAX_WSM_BUFFERS 4
  #endif
#endif


/** WSM internal buffer structure */
typedef struct WsmBuf_s {
  String_t    bufName;     /**< external name of buffer */
  MemHandle_t memH;        /**< memory handle */
  MemPtr_t    pFirstFree;  /**< pointer to first free element in buffer */
  MemPtr_t    pFirstData;  /**< pointer to first data element in buffer */
  MemSize_t   size;        /**< size of buffer */
  MemSize_t   usedBytes;   /**< used bytes in buffer */
  Byte_t      flags;
} WsmBuf_t;


/** WSM globals for use with global Anchor */
typedef struct WsmGlobals_s {
  Ret_t           wsmRet;          /**< last WSM return code */
  Byte_t          initWasCalled;   /**< was wsmInit() called? */
  WsmBuf_t        wsmBuf[MAX_WSM_BUFFERS];
  Short_t         wsmIndex;        /**< Index of actual buffer */
  WsmSmGlobals_t  wsmSm;           /**< WSM_SM global; device dependent! */
} *WsmGlobalsPtr_t, WsmGlobals_t;

Ret_t wsmInit (const WsmOptions_t *wsmOpts) WSM_FUNC;
Ret_t wsmCreate (String_t bufName, MemSize_t bufSize, MemHandle_t *wsmH) WSM_FUNC;
Ret_t wsmOpen (String_t bufName, MemHandle_t *wsmH) WSM_FUNC;
Ret_t wsmClose (MemHandle_t wsmH) WSM_FUNC;
Ret_t wsmDestroy (String_t bufName) WSM_FUNC;
Ret_t wsmTerminate (void) WSM_FUNC;
Ret_t wsmProcessedBytes (MemHandle_t wsmH, MemSize_t noBytes) WSM_FUNC;
Ret_t wsmLockH (MemHandle_t wsmH, SmlBufPtrPos_t requestedPos, MemPtr_t *pMem) WSM_FUNC;
Ret_t wsmGetFreeSize(MemHandle_t wsmH, MemSize_t *freeSize) WSM_FUNC;
Ret_t wsmGetUsedSize(MemHandle_t wsmH, MemSize_t *usedSize) WSM_FUNC;
Ret_t wsmUnlockH (MemHandle_t wsmH) WSM_FUNC;
Ret_t wsmSetUsedSize (MemHandle_t wsmH, MemSize_t usedSize) WSM_FUNC;
Ret_t wsmReset (MemHandle_t wsmH) ;

#endif // !defined(NOWSM)

#endif
