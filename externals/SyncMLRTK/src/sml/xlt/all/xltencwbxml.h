/**
 * @file
 * The WBXML Encoder header file
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
#ifndef _XLT_ENC_WBXML_H
#define _XLT_ENC_WBXML_H

#include <smlerr.h>
#include <smldef.h>
#include <smldtd.h>
#include "xlttags.h"
#include "xltenccom.h"

#define test 1

/** byte for WBXML String Table Length - not yet implemented yet -> 0x00 */
#define XLT_STABLEN 0x00

/** byte for WBXML charset - not yet implemented - default UTF-8 */
#define XLT_CHARSET 0x6A

/** byte for WBXML Version Number */
#define XLT_WBXMLVER 0x02

/** byte to add to a tag if a content follows */
#define XLT_CONTBYTE 0x40

/** byte to add to a tag if an attribute follows */
#define XLT_ATTRBYTE 0x80

/** termination character for certain WBXML element types (e.g.\ STR_I) */
#define XLT_TERMSTR 0x00

/** public identifier 0x00,0x00 -> unknown, use stringtable */
#define XLT_PUBIDENT1 0x00
#define XLT_PUBIDENT2 0x00

// %%% luz: 2003-07-31: now in xltenc.c's SyncMLFPI table
//#define XLT_DTD_ID "-//SYNCML//DTD SyncML 1.0//EN"

/** switch page tag 0x00 */
#define XLT_SWITCHPAGE 0x00

/** default codepage */
#define XLT_DEFAULTCODEPAGE 0x00

#ifdef __cplusplus
extern "C" {
#endif

/** global tokens in WBXML */
typedef enum {
  UNDEF = 0,
  END,
  STR_I,
  OPAQUE,
  TAG
} XltElementType_t;

Ret_t wbxmlGenerateTag(XltTagID_t tagId, XltTagType_t tagType, BufferMgmtPtr_t pBufMgr);
Ret_t wbxmlWriteTypeToBuffer(const MemPtr_t pContent, XltElementType_t elType, Long_t size, BufferMgmtPtr_t pBufMgr);
Ret_t wbxmlOpaqueSize2Buf(Long_t size, BufferMgmtPtr_t pBufMgr);
MemByte_t wbxmlGetGlobToken(XltElementType_t elType);

#ifdef __cplusplus
}
#endif

#endif
