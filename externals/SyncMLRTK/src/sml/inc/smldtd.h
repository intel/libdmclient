/**
 * @file
 * SyncML DTD specific type definitions
 *
 * @target_system   all
 * @target_os       all
 * @description Definition of structures representing DTD elements
 */


/********************************************************************/
/* @note                                                            */
/* These definitions are based on the DTD dated from July, 7th, 00 */
/********************************************************************/

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


#ifndef _SML_DTD_H
  #define _SML_DTD_H


/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/


#include <smldef.h>




/**
 * ===========================
 * Common used SyncML Elements
 * ===========================
 */



/**
 * PCDATA - types of synchronization data which SyncML supports
 */
typedef enum {
  SML_PCDATA_UNDEFINED = 0,
  SML_PCDATA_STRING,    /**< String type */
  SML_PCDATA_OPAQUE,    /**< Opaque type */
  SML_PCDATA_EXTENSION, /**< Extention type - specified by PcdataExtension_t */
  SML_PCDATA_CDATA      /**< XML CDATA type    */
} SmlPcdataType_t;


/**
 * PCDATA - types of extensions for PCData elements
 */
typedef enum {
  SML_EXT_UNDEFINED = 0,
  SML_EXT_METINF, /**< Meta Information */
  SML_EXT_DEVINF, /**< Device Information */
  SML_EXT_LAST    /**< last codepage, needed for loops! */
} SmlPcdataExtension_t;



/**
 * PCDATA - into this structure SyncML wraps the synchronization data itself
 */
typedef struct sml_pcdata_s {
  SmlPcdataType_t       contentType;   /**< The type of data which a PCDATA structure contains */
  SmlPcdataExtension_t  extension;     /**< PCData Extension type */
  MemSize_t             length;        /**< length of the data in this PCDATA structure */
  VoidPtr_t             content;       /**< Pointer to the data itself */
} *SmlPcdataPtr_t, SmlPcdata_t;

/** generic list of PCData elements */
typedef struct sml_pcdata_list_s {
  SmlPcdataPtr_t  data;
  struct sml_pcdata_list_s *next;
} *SmlPcdataListPtr_t, SmlPcdataList_t;

/*
 * Various flags which are actually declared and (EMPTY) elements in
 * SyncML. This assumes at least a 16-bit architecture for the
 * underlying OS. We need to review this if that is deemed a problem.
 */
#define SmlArchive_f            0x8000         /**< Delete flags */
#define SmlSftDel_f             0x4000         /**< Delete flags */
#define SmlMoreData_f           0x0400         /**< MoreData flag */
#define SmlNoResults_f          0x0200         /**< No Results flag  */
#define SmlNoResp_f             0x0100         /**< No Response flag */
#define SmlFinal_f              0x0001         /**< Header flag */
#ifdef __USE_METINF__
#define SmlMetInfSharedMem_f    0x0002         /**< MetInf Shared Memory Flag */
/* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
#define SmlMetInfFieldLevel_f   0x0004         /**< MetInf FieldLevel Flag */
#endif
#ifdef __USE_DEVINF__
#define SmlDevInfSharedMem_f    0x0004         /**< DevInf Shared Memory Flag */
/* SCTSTK - 18/03/2002, S.H. 2002-04-05 : SyncML 1.1 */
#define SmlDevInfUTC_f          0x0001         /**< DevInf utc Flag */
#define SmlDevInfNOfM_f         0x0002         /**< DevInf support n of m Flag */
#define SmlDevInfLargeObject_f  0x0008         /**< DevInf support large object Flag */
/* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
#define SmlDevInfFieldLevel_f   0x0010         /**< DevInf support field level replaces */
#define SmlDevInfNoTruncate_f   0x0020         /**< DevInf support noTruncate Flag */
#define SmlDevInfHierarchical_f 0x0040         /**< DevInf support SupportHierarchicalSync Flag */

#endif


/**
 * Chal
 */
typedef struct sml_chal_s {
  SmlPcdataPtr_t           meta;
} *SmlChalPtr_t, SmlChal_t;

/**
 * Credentials
 */
typedef struct sml_cred_s {
  SmlPcdataPtr_t           meta;       // opt.
  SmlPcdataPtr_t           data;
} *SmlCredPtr_t, SmlCred_t;


/* forward declaration as item is used in filter which is used
   in target which is used in item itself */
//typedef struct sml_item_s;

/**
 * Record or Field level filter
 * %%% Added 2005-08-17 by synthesis/luz for DS 1.2
 */
typedef struct sml_record_or_field_filter_s {
  struct sml_item_s *item;
} *SmlRecordOrFieldFilterPtr_t, SmlRecordOrFieldFilter_t;

/**
 * Filter
 * %%% Added 2005-08-17 by synthesis/luz for DS 1.2
 */
typedef struct sml_filter_s {
  SmlPcdataPtr_t               meta;
  SmlRecordOrFieldFilterPtr_t  field;      // opt.
  SmlRecordOrFieldFilterPtr_t  record;     // opt.
  SmlPcdataPtr_t               filtertype; // opt.
} *SmlFilterPtr_t, SmlFilter_t;



/**
 * Source or target parent location
 * %%% Added 2005-08-17 by synthesis/luz for DS 1.2
 */
typedef struct sml_source_or_target_parent_s {
  SmlPcdataPtr_t           locURI;
} *SmlSourceParentPtr_t, SmlSourceParent_t,
  *SmlTargetParentPtr_t, SmlTargetParent_t;


/**
 * Source or target location
 */
typedef struct sml_source_or_target_s {
  SmlPcdataPtr_t           locURI;
  SmlPcdataPtr_t           locName;    // opt.
  SmlFilterPtr_t           filter;     // opt., for target only
} *SmlSourcePtr_t, SmlSource_t,
  *SmlTargetPtr_t, SmlTarget_t;

typedef struct sml_source_list_s {
  SmlSourcePtr_t           source;
  struct sml_source_list_s  *next;
} *SmlSourceListPtr_t, SmlSourceList_t;



/*
 * ==============================
 * SyncML Message Header Elements
 * ==============================
 */


/**
 * SyncML header
 * As the header is needed for each SyncML message, it's also the parameter
 * of the startMessage call.
 */
typedef struct sml_sync_hdr_s {
  SmlProtoElement_t elementType; // Internal Toolkit Field
  SmlPcdataPtr_t  version;
  SmlPcdataPtr_t  proto;
  SmlPcdataPtr_t  sessionID;
  SmlPcdataPtr_t  msgID;
  Flag_t          flags;      // NoResp
  SmlTargetPtr_t  target;
  SmlSourcePtr_t  source;
  SmlPcdataPtr_t  respURI;    // opt.
  SmlCredPtr_t    cred;       // opt.
  SmlPcdataPtr_t    meta;     // opt.
} *SmlSyncHdrPtr_t, SmlSyncHdr_t;

// SyncML Body and SyncML container is not needed, as there are function calls
// (smlStartMessage(), smlEndMessage()) that let the framework know when to start and end
// the SyncML document



/*
 * =========================
 * Data description elements
 * =========================
 */


/**
 * Data in SyncML is encapsulated in an "item" element.
 */
typedef struct sml_item_s {
  SmlTargetPtr_t        target;       // opt.
  SmlSourcePtr_t        source;       // opt.
  SmlTargetParentPtr_t  targetParent; // opt. (Added by luz 2005-08-17 for DS 1.2)
  SmlSourceParentPtr_t  sourceParent; // opt. (Added by luz 2005-08-17 for DS 1.2)

  SmlPcdataPtr_t        meta;         // opt.
  SmlPcdataPtr_t        data;         // opt.
  Flag_t                flags;        // opt. for MoreData
} *SmlItemPtr_t, SmlItem_t;

typedef struct sml_item_list_s {
  SmlItemPtr_t          item;
  struct sml_item_list_s    *next;
} *SmlItemListPtr_t, SmlItemList_t;


/*
 * ==============================================
 * SyncML Commands (Protocol Management Elements)
 * ==============================================
 */

/**
 * Generic commands:
 * Add, Copy, Replace, Delete, Move (added for DS 1.2, 2005-08-17 by luz)
 */
typedef struct sml_generic_s {
  SmlProtoElement_t       elementType; // Internal Toolkit Field
  SmlPcdataPtr_t        cmdID;
  Flag_t                flags;      // NoResp, Archive (Delete), SftDel (Delete)
  SmlCredPtr_t          cred;       // opt.
  SmlPcdataPtr_t        meta;       // opt.
  SmlItemListPtr_t      itemList;
} *SmlAddPtr_t, SmlAdd_t,
  *SmlCopyPtr_t, SmlCopy_t,
  *SmlMovePtr_t, SmlMove_t,
  *SmlReplacePtr_t, SmlReplace_t,
  *SmlDeletePtr_t, SmlDelete_t,
  *SmlGenericCmdPtr_t, SmlGenericCmd_t;

/**
 * Alert command:
 */
typedef struct sml_alert_s {
  SmlProtoElement_t       elementType; // Internal Toolkit Field
  SmlPcdataPtr_t        cmdID;
  Flag_t                flags;      // NoResp
  SmlCredPtr_t          cred;       // opt.
  SmlPcdataPtr_t        data;       // opt.
  SmlItemListPtr_t      itemList;
} *SmlAlertPtr_t, SmlAlert_t;


/**
 * Atomic/Sequence command:
 */
typedef struct sml_atomic_s {
  SmlProtoElement_t       elementType; // Internal Toolkit Field
  SmlPcdataPtr_t        cmdID;
  Flag_t                flags;      // NoResp
  SmlPcdataPtr_t        meta;       // opt.
} *SmlAtomicPtr_t, SmlAtomic_t,
  *SmlSequencePtr_t, SmlSequence_t;


/**
 * Sync command:
 */
typedef struct sml_sync_s {
  SmlProtoElement_t       elementType; // Internal Toolkit Field
  SmlPcdataPtr_t        cmdID;
  Flag_t                flags;      // NoResp
  SmlCredPtr_t          cred;       // opt.
  SmlTargetPtr_t        target;     // opt.
  SmlSourcePtr_t        source;     // opt.
  SmlPcdataPtr_t        meta;       // opt.
  SmlPcdataPtr_t        noc;        // opt. (SyncML 1.1)
} *SmlSyncPtr_t, SmlSync_t;


/**
 * Exec command:
 */
typedef struct sml_exec_s {
  SmlProtoElement_t       elementType;
  SmlPcdataPtr_t        cmdID;
  Flag_t                flags;      // NoResp
  SmlCredPtr_t          cred;       // opt.
  SmlPcdataPtr_t        meta;       // opt.
  SmlItemPtr_t          item;
} *SmlExecPtr_t, SmlExec_t;


/**
 * Get and Put command:
 */
typedef struct sml_get_put_s {
  SmlProtoElement_t       elementType; // Internal Toolkit Field
  SmlPcdataPtr_t        cmdID;
  Flag_t                flags;      // NoResp
  SmlPcdataPtr_t        lang;       // opt.
  SmlCredPtr_t          cred;       // opt.
  SmlPcdataPtr_t        meta;       // opt.
  SmlItemListPtr_t         itemList;
} *SmlPutPtr_t, SmlPut_t,
  *SmlGetPtr_t, SmlGet_t;


/**
 * Map command:
 */
typedef struct sml_map_item_s {
  SmlTargetPtr_t           target;
  SmlSourcePtr_t           source;
} *SmlMapItemPtr_t, SmlMapItem_t;

typedef struct sml_map_item_list_s {
  SmlMapItemPtr_t          mapItem;
  struct sml_map_item_list_s  *next;
} *SmlMapItemListPtr_t, SmlMapItemList_t;

typedef struct sml_map_s {
  SmlProtoElement_t       elementType; // InternalToolkit Field
  SmlPcdataPtr_t        cmdID;
  SmlTargetPtr_t        target;
  SmlSourcePtr_t        source;
  SmlCredPtr_t          cred;       // opt.
  SmlPcdataPtr_t        meta;       // opt.
  SmlMapItemListPtr_t      mapItemList;
} *SmlMapPtr_t, SmlMap_t;


/**
 * Results command:
 */
typedef struct sml_results_s {
  SmlProtoElement_t       elementType; // Internal Toolkit Field
  SmlPcdataPtr_t        cmdID;
  SmlPcdataPtr_t        msgRef;     // opt.
  SmlPcdataPtr_t        cmdRef;
  SmlPcdataPtr_t        meta;       // opt.
  SmlPcdataPtr_t        targetRef;  // opt.
  SmlPcdataPtr_t        sourceRef;  // opt.
  SmlItemListPtr_t      itemList;
} *SmlResultsPtr_t, SmlResults_t;


/**
 * Search command:
 */
typedef struct sml_search_s {
  SmlProtoElement_t       elementType; // Internal Toolkit Field
  SmlPcdataPtr_t        cmdID;
  Flag_t                flags;      // NoResp, NoResults
  SmlCredPtr_t          cred;       // opt.
  SmlTargetPtr_t        target;     // opt.
  SmlSourceListPtr_t    sourceList;
  SmlPcdataPtr_t        lang;       // opt.
  SmlPcdataPtr_t        meta;
  SmlPcdataPtr_t        data;
} *SmlSearchPtr_t, SmlSearch_t;


/**
 * Status command:
 */
typedef struct sml_target_ref_list_s {
  SmlPcdataPtr_t              targetRef;
  struct sml_target_ref_list_s    *next;
} *SmlTargetRefListPtr_t, SmlTargetRefList_t;

typedef struct sml_source_ref_list_s {
  SmlPcdataPtr_t              sourceRef;
  struct sml_source_ref_list_s    *next;
} *SmlSourceRefListPtr_t, SmlSourceRefList_t;

typedef struct sml_status_s {
  SmlProtoElement_t       elementType; // Internal Toolkit Field
  SmlPcdataPtr_t        cmdID;
  SmlPcdataPtr_t        msgRef; // Opt.
  SmlPcdataPtr_t        cmdRef;
  SmlPcdataPtr_t        cmd;
  SmlTargetRefListPtr_t targetRefList;  // opt.
  SmlSourceRefListPtr_t sourceRefList;  // opt.
  SmlCredPtr_t          cred;           // opt.
  SmlChalPtr_t          chal;           // opt.
  SmlPcdataPtr_t        data;
  SmlItemListPtr_t      itemList;       // opt.
} *SmlStatusPtr_t, SmlStatus_t;


/**
 * a little helper for typecasting
 */
typedef struct sml_unknown_proto_element_s {
  SmlProtoElement_t elementType;  // Internal Toolkit Field
} *SmlUnknownProtoElementPtr_t, SmlUnknownProtoElement_t;


#endif
