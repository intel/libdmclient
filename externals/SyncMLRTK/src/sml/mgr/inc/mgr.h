/**
 * @file
 * SyncML internal API of the MGR module
 *
 * @target_system   all
 * @target_os       all
 * @description Definitions for internal use within the SyncML implementation
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


#ifndef _MGR_H
  #define _MGR_H


/*************************************************************************
 *  Definitions
 *************************************************************************/


#include <smldef.h>
#include <sml.h>
#include "wsm.h"
#include <xlttagtbl.h>


/**
 * ========================================
 * Definitions used for Instance Management
 * ========================================
 **/



/**
 * Current instance status
 */
typedef enum {
  MGR_IDLE,     /**< instance is idle (available for usage by applications) */
  MGR_USED,     /**< instance is in use, but currently inactive */
  MGR_RECEIVE,  /**< actively used for receiving (locked by application) */
  MGR_SEND,     /**< actively used for sending (locked by application) */
  MGR_ENCODING, /**< actively used for encoding (locked by SyncML) */
  MGR_DECODING  /**< actively used for decoding (locked by SyncML) */
} InstanceStatus_t;



/**
 * structure describing the current status of an instance,
 */
typedef struct instance_info_s {
  #ifndef NOWSM
  InstanceID_t             id;                /**< unique ID of the instance */
  MemPtr_t                 workspaceHandle;   /**< handle to the  first position of the assigned workspace memory */
  #else
  // buffer pointers for NOWSM simplified case
  MemPtr_t                 instanceBuffer;    /**< pointer to instance work buffer */
  MemSize_t                instanceBufSiz;    /**< size of currently allocated buffer */
  Byte_t                   readLocked;        /**< set when buffer is locked for read */
  Byte_t                   writeLocked;       /**< set when buffer is locked for read */
  MemPtr_t                 readPointer;       /**< read pointer */
  MemPtr_t                 writePointer;      /**< write pointer */
  MemPtr_t                 outgoingMsgStart;  /**< set whenever a smlStartMessage is issued, NULL when invalid */
  MemPtr_t                 incomingMsgStart;  /**< set whenever mgrProcessStartMessage starts reading a message, NULL when invalid */
  MemSize_t                maxOutgoingSize;   /**< if<>0, smlXXXCmd will not modify the buffer when there's not enough room */
  #endif
  InstanceStatus_t         status;            /**< current internal state of instance */
  SmlCallbacksPtr_t        callbacks;         /**< Defined callback refererences for this Instance */
  SmlInstanceOptionsPtr_t  instanceOptions;   /**< Defined options for this Instance (e.g. encoding type) */
  VoidPtr_t                userData;          /**< Pointer to a structure, which is passed to the invoked callback functions   */
  #ifndef NOWSM
  VoidPtr_t                workspaceState;    /**< Pointer to a structure defining the current workspace status */
  #endif
  VoidPtr_t                encoderState;      /**< Pointer to a structure defining the current encoder status */
  VoidPtr_t                decoderState;      /**< Pointer to a structure defining the current decoder status */
  #ifndef NOWSM
  struct instance_info_s*  nextInfo;          /**< Pointer to next Instance Info in a list */
  #else
  smlPrintFunc  defaultPrintFunc;             /**< default application callback for displaying strings (is a global in original version) */
  #endif
} *InstanceInfoPtr_t, InstanceInfo_t;


/** Pointers to store the global Tag tables */
typedef struct tokeninfo_s {
    TagPtr_t  SyncML;
    TagPtr_t  MetInf;
    TagPtr_t  DevInf;
} *TokenInfoPtr_t, TokenInfo_t;


#ifndef NOWSM
// Note, version without WSM has NO globals at all
/**
 * structure describing the current status of the global syncml module
 * (holds all global variables within SyncML)
 */
typedef struct syncml_info_s {
  InstanceInfoPtr_t        instanceListAnchor;/**< Anchor of the global list of known SyncML instances */
  SmlOptionsPtr_t          syncmlOptions;     /**< Options valid for this SyncML Process */
  WsmGlobalsPtr_t          wsmGlobals;        /**< Workspace global variables */
  TokenInfoPtr_t           tokTbl;
} *SyncMLInfoPtr_t, SyncMLInfo_t;

/*************************************************************************
 *  External Function Declarations
 *************************************************************************/

SyncMLInfoPtr_t mgrGetSyncMLAnchor(void) MGR_FUNC;
InstanceInfoPtr_t mgrGetInstanceListAnchor(void) MGR_FUNC;
void mgrSetInstanceListAnchor(InstanceInfoPtr_t newListAnchor) MGR_FUNC;

#ifndef __SML_LITE__  /* these API calls are NOT included in the Toolkit lite version */
/* SyncML internal function prototypes */
Ret_t addInfo(InstanceInfoPtr_t pInfo) MGR_FUNC;
InstanceInfoPtr_t findInfo(InstanceID_t id) MGR_FUNC;
Ret_t removeInfo(InstanceID_t id) MGR_FUNC;
#endif

#endif // !defined(NOWSM)

Ret_t mgrResetWorkspace (InstanceID_t id) MGR_FUNC;

#endif // ifndef _MGR_H
