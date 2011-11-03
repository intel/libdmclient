/**
 * @file
 * Type definitions for SyncML
 *
 * @target_system   all
 * @target_os       all
 *
 * @description Platform independent header with syncML types and definitions
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




#ifndef _SML_DEF_H
  #define _SML_DEF_H



/*************************************************************************
 *  Definitions
 *************************************************************************/



/**
 * include target/compiler specific defines/options/settings
 */
#include "define.h"


/*
 * ===================================
 * Common all-purpose type definitions
 * ===================================
 */



/*
 * ==================================
 * Basic data types
 * ==================================
 */
typedef short         Short_t;      /**< short integer,  16 Bytes */
typedef long          Long_t;       /**< long integer,   32 Bytes */
typedef char*         String_t;     /**< String pointer,  */
typedef unsigned char Byte_t;       /**< a single byte */
typedef Byte_t        Boolean_t;    /**< a boolean */
#ifndef NULL                        /**< a NULL pointer */
  #define NULL (void*) 0
#endif
#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif


typedef Short_t       Ret_t;        /**< Return Type of API Commands */
typedef Long_t        Length_t;     /**< System dependent string length */
typedef Short_t       MemHandle_t;  /**< Memory object Handle  */
typedef unsigned char *MemPtr_t;    /**< Memory object Pointer */
typedef void          *VoidPtr_t;   /**< Void Pointer */
typedef Long_t        MemSize_t;    /**< System dependent memory object size */
typedef unsigned char MemByte_t;    /**< Memory element */
typedef unsigned int  Flag_t;       /**< A generic flag type. This type is
                                     * used to declare variables in structures
                                     * wherever flags are used.
                                     */


/*
 * ==================================
 * Definitions used in the SyncML API
 * ==================================
 */

/**
 * Application callback function displaying output strings to the user
 */
typedef void (*smlPrintFunc) (String_t outputString);

#ifdef NOWSM
/**
 * structure describing the options and setting of this syncml process
 */
typedef struct sml_options_s {
  smlPrintFunc  defaultPrintFunc;       /**< default application callback for displaying strings, */
  MemSize_t     maxWorkspaceAvailMem;   /**< size which all workspaces in total MUST not exceed */
} *SmlOptionsPtr_t, SmlOptions_t;
#endif



/**
 * Reference of an instance
 */
#ifdef NOWSM
typedef void *InstanceID_t;          /**< without wsm, instance ID is direct pointer to instance info */
#else
typedef MemHandle_t InstanceID_t;    /**< Handle, used as a unique ID of
                                      * an synchronization instance
                                      */
#endif


/**
 * Type of used encoding
 */
typedef enum {
  SML_UNDEF = 0,
  SML_WBXML,
  SML_XML
} SmlEncoding_t;


/**
 * SyncML version
 */
// %%% added luz 2003-07-31:
// %%% added DS 1.2 luz 2005-08-17:
typedef enum {
  SML_VERS_UNDEF = 0,
  SML_VERS_1_0,
  SML_VERS_1_1,
  SML_VERS_1_2,
  SML_NUM_VERS
} SmlVersion_t;

/**
 * structure describing the options of an instance,
 */
typedef struct sml_instance_options_s {
  SmlEncoding_t encoding;               /**< Used encoding type, */
  MemSize_t     workspaceSize;          /**< size of the workspace to allocate (instance buffer size if NOWSM defined) */
  #ifndef NOWSM
  String_t      workspaceName;          /**< name of the workspace */
  #else
  MemSize_t     maxOutgoingSize;        /**< max size of outgoing message, 0 if no restriction   */
  #endif
} *SmlInstanceOptionsPtr_t, SmlInstanceOptions_t;


/**
 * Processing modes
 */
typedef enum {
  SML_DO_NOTHING = 0,
  SML_FIRST_COMMAND,
  SML_NEXT_COMMAND,
  SML_NEXT_MESSAGE,
  SML_ALL_COMMANDS
} SmlProcessMode_t;


/**
 * Requested buffer pointer position
 */
typedef enum {
  SML_FIRST_DATA_ITEM = 0,
  SML_FIRST_FREE_ITEM
} SmlBufPtrPos_t;



/**
 * SyncML Protocol Management and Command Elements (PE)
 */
typedef enum {
    SML_PE_UNDEF = 0,
    SML_PE_ERROR,
    SML_PE_ADD,
    SML_PE_ALERT,
    SML_PE_ATOMIC_START,
    SML_PE_ATOMIC_END,
    SML_PE_COPY,
    SML_PE_DELETE,
    SML_PE_EXEC,
    SML_PE_GET,
    SML_PE_MAP,
    SML_PE_PUT,
    SML_PE_RESULTS,
    SML_PE_SEARCH,
    SML_PE_SEQUENCE_START,
    SML_PE_SEQUENCE_END,
    SML_PE_STATUS,
    SML_PE_SYNC_START,
    SML_PE_SYNC_END,
    SML_PE_REPLACE,
    SML_PE_HEADER,
    SML_PE_PUT_GET,
    SML_PE_CMD_GROUP,
    SML_PE_GENERIC,
    SML_PE_FINAL,
    /* %%% Added 2005-08-17 by synthesis/luz for DS 1.2 */
    SML_PE_MOVE
} SmlProtoElement_t;


#endif
