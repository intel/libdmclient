/**
 * @file
 * Encoder utils header file
 *
 * @target_system   All
 * @target_os       All
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
/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/

#ifndef _XLT_ENC_COM_H
#define _XLT_ENC_COM_H

#include <smldef.h>
#include <smldtd.h>
#include <sml.h>

#include "xlttags.h"


#ifdef __cplusplus
extern "C" {
#endif

/** flags if a (WB)XML block is required or optional */
typedef enum {
  OPTIONAL = 0,
  REQUIRED
} XltRO_t;

/** Tag Types (TT) - begin tags, end tags, ... */
typedef enum {
  TT_END = 0,
  TT_BEG,
  TT_ALL
} XltTagType_t;

/** Structure elements which are not directly relied to a tag */
typedef enum {
  ITEM_LIST,
  TARGET_LIST,
  SOURCE_LIST,
  META_LIST,
  MAPITEM_LIST,
  TARGETREF_LIST,
  SOURCEREF_LIST
} XltListType_t;

/** Type for storing free mem size evaluation information */
typedef struct XltSpaceEvaluation_s
{
  MemSize_t written_bytes;
  MemSize_t end_tag_size;
  // %%% luz 2002-09-03: evaluation needs private shadow copies of these:
  SmlPcdataExtension_t cur_ext;
  SmlPcdataExtension_t last_ext;
} XltSpaceEvaluation_t, *XltSpaceEvaluationPtr_t;


typedef struct bufferMgmt_s
{
  MemPtr_t smlXltBufferP;
  MemPtr_t smlXltStoreBufP;
  MemSize_t smlXltWrittenBytes;
  MemSize_t smlXltBufferLen;
  XltTagID_t           switchExtTag;
  SmlPcdataExtension_t smlCurExt;
  SmlPcdataExtension_t smlLastExt;
  SmlPcdataExtension_t smlActiveExt;
  MemSize_t endTagSize;
  Boolean_t spaceEvaluation;
  // %%% luz:2003-04-24: added syncmlvers
  // %%% luz:2003-07-31: made it an enum, now called vers
  SmlVersion_t vers;
} BufferMgmt_t, *BufferMgmtPtr_t;

Ret_t xltAddToBuffer(const MemPtr_t pContent, MemSize_t size, BufferMgmtPtr_t pBufMgr);

#ifdef __cplusplus
}
#endif

#endif
