/**
 * @file
 * Encoder header file
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

#ifndef _XLT_ENC_H
#define _XLT_ENC_H


#include <smlerr.h>
#include <xltenccom.h>
#include <smldef.h>
#include <smldtd.h>
#include <xlttags.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Type for storing encoder information */
typedef struct XltEncoder_s
{
  SmlEncoding_t enc;
  SmlVersion_t vers; // %%% luz 2003-07-31: added SyncML version here
  SmlPcdataExtension_t cur_ext;
  SmlPcdataExtension_t last_ext;
  Boolean_t final;
  XltSpaceEvaluationPtr_t space_evaluation;
  MemSize_t end_tag_size;
} XltEncoder_t, *XltEncoderPtr_t;


Ret_t xltEncInit(SmlEncoding_t enc, const SmlSyncHdrPtr_t pHeader, const MemPtr_t pBufEnd, MemPtr_t *ppBufPos, XltEncoderPtr_t *ppEncoder, SmlVersion_t vers) XLT_FUNC;
Ret_t xltEncAppend(const XltEncoderPtr_t pEncoder, SmlProtoElement_t pe, const MemPtr_t pBufEnd, const VoidPtr_t pContent, MemPtr_t *ppBufPos) XLT_FUNC;
Ret_t xltEncTerminate(const XltEncoderPtr_t pEncoder, const MemPtr_t pBufEnd, MemPtr_t *ppBufPos) XLT_FUNC;
Ret_t xltEncReset(XltEncoderPtr_t pEncoder) XLT_FUNC;
Ret_t xltGenerateTag(XltTagID_t, XltTagType_t, SmlEncoding_t, BufferMgmtPtr_t, SmlPcdataExtension_t) XLT_FUNC;
Ret_t xltStartEvaluation(XltEncoderPtr_t pEncoder) XLT_FUNC;
Ret_t xltEndEvaluation(InstanceID_t id, XltEncoderPtr_t pEncoder, MemSize_t *freemem) XLT_FUNC;
Ret_t xltEncBlock(XltTagID_t tagId, XltRO_t reqOptFlag, const VoidPtr_t pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr, SmlPcdataExtension_t attFlag) XLT_FUNC;
Ret_t xltBuildExtention(SmlPcdataExtension_t extId, XltRO_t reqOptFlag, VoidPtr_t pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr) XLT_FUNC;
Ret_t xltEncPcdata(XltTagID_t tagId, XltRO_t reqOptFlag, const VoidPtr_t pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr, SmlPcdataExtension_t attFlag) XLT_FUNC;
Ret_t subdtdEncWBXML(XltTagID_t tagId, XltRO_t reqOptFlag, const VoidPtr_t pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr, SmlPcdataExtension_t attFlag) XLT_FUNC;

#ifdef __cplusplus
}
#endif

#endif
