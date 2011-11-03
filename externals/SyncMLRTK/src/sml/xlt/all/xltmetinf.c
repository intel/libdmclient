/**
 * @file
 * MetaInf DTD related functions for the en-/decoder
 *
 * @target_system   all
 * @target_os       all
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
#include "syncml_tk_prefix_file.h" // %%% luz: needed for precompiled headers in eVC++

#include "define.h"
#ifdef __USE_METINF__

#include "smlmetinfdtd.h"
#include "xlttags.h"
#include "xltmetinf.h"
#include "xlttagtbl.h"
#include "xltenc.h"
#include "xltencwbxml.h"

#include "define.h"
#include "libstr.h"
#include "smlerr.h"
#include "smldtd.h"
#include "libmem.h"
#include "libutil.h"
/* extern */
 extern SML_API void smlFreeMetinfAnchor(SmlMetInfAnchorPtr_t data) XLT_FUNC;
 extern SML_API void smlFreeMetinfMem(SmlMetInfMemPtr_t data) XLT_FUNC;
 extern SML_API void smlFreeMetinfMetinf(SmlMetInfMetInfPtr_t data) XLT_FUNC;


/* decoder callbacks */
Ret_t buildMetInfAnchorCmd(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlMetInfAnchorPtr_t pAnchor;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pAnchor = (SmlMetInfAnchorPtr_t)smlLibMalloc(sizeof(SmlMetInfAnchor_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pAnchor, 0, sizeof(SmlMetInfAnchor_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pAnchor;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlFreeMetinfAnchor(pAnchor);
        //smlLibFree(pAnchor);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
            /* PCDATA elements */
            case TN_METINF_LAST:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pAnchor->last);
                break;
            case TN_METINF_NEXT:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pAnchor->next);
                break;
            default:
                rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            smlFreeMetinfAnchor(pAnchor);
            //smlLibFree(pAnchor);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeMetinfAnchor(pAnchor);
            //smlLibFree(pAnchor);
            return rc;
        }
    }
    *ppElem = pAnchor;

    return SML_ERR_OK;
}

Ret_t buildMetInfMemCmd(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlMetInfMemPtr_t pMem;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pMem = (SmlMetInfMemPtr_t)smlLibMalloc(sizeof(SmlMetInfMem_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pMem, 0, sizeof(SmlMetInfMem_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pMem;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlFreeMetinfMem(pMem);
        //smlLibFree(pMem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
            /* %%% luz 2005-08-24 :this definition was plain wrong - shared is a flag, not Pcdata!
            case TN_METINF_SHAREDMEM:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pMem->shared);
                break; */

            /* PCDATA elements */
            case TN_METINF_FREEMEM:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pMem->free);
                break;
            case TN_METINF_FREEID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pMem->freeid);
                break;

            /* flags */
            case TN_METINF_SHAREDMEM:
                pMem->flags |= SmlMetInfSharedMem_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;

            default:
                rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            smlFreeMetinfMem(pMem);
            //smlLibFree(pMem);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeMetinfMem(pMem);
            //smlLibFree(pMem);
            return rc;
        }
    }
    *ppElem = pMem;

    return SML_ERR_OK;
}

Ret_t buildMetInfMetInfCmd(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlMetInfMetInfPtr_t pMeta;
    Ret_t rc;
    int foundWrapper = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_ERR_XLT_INVAL_SYNCML_DOC;

    if ((pMeta = (SmlMetInfMetInfPtr_t)smlLibMalloc(sizeof(SmlMetInfMetInf_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pMeta, 0, sizeof(SmlMetInfMetInf_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pMeta;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlFreeMetinfMetinf(pMeta);
        //smlLibFree(pMeta);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
          case TN_METINF_METINF: /* ignore - it's just the wrapper tag */
            foundWrapper = 1;
            break;
          case TN_METINF_FORMAT:
            rc = buildPCData(pDecoder, (VoidPtr_t)&pMeta->format);
            break;
          case TN_METINF_TYPE:
            rc = buildPCData(pDecoder, (VoidPtr_t)&pMeta->type);
            break;
          case TN_METINF_MARK:
            rc = buildPCData(pDecoder, (VoidPtr_t)&pMeta->mark);
            break;
          case TN_METINF_SIZE:
            rc = buildPCData(pDecoder, (VoidPtr_t)&pMeta->size);
            break;
          case TN_METINF_VERSION:
            rc = buildPCData(pDecoder, (VoidPtr_t)&pMeta->version);
            break;
          case TN_METINF_NEXTNONCE:
            rc = buildPCData(pDecoder, (VoidPtr_t)&pMeta->nextnonce);
            break;
          case TN_METINF_ANCHOR:
            rc = buildMetInfAnchorCmd(pDecoder, (VoidPtr_t)&pMeta->anchor);
            break;
          case TN_METINF_MAXMSGSIZE:
            rc = buildPCData(pDecoder, (VoidPtr_t)&pMeta->maxmsgsize);
            break;
          /* SCTSTK - 18/03/2002 S.H. 2002-04-05: SyncML 1.1 */
          case TN_METINF_MAXOBJSIZE:
            rc = buildPCData(pDecoder, (VoidPtr_t)&pMeta->maxobjsize);
            break;
          case TN_METINF_MEM:
            rc = buildMetInfMemCmd(pDecoder, (VoidPtr_t)&pMeta->mem);
            break;
          case TN_METINF_EMI:
            rc = buildPCDataList(pDecoder, (VoidPtr_t)&pMeta->emi);
            break;

          /* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
          case TN_METINF_FIELDLEVEL:
            pMeta->flags |= SmlMetInfFieldLevel_f;
            rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
            break;

          default:
              rc = SML_ERR_XLT_INVAL_SYNCML_DOC;
        }
        if (rc != SML_ERR_OK) {
            smlFreeMetinfMetinf(pMeta);
            //smlLibFree(pMeta);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeMetinfMetinf(pMeta);
            //smlLibFree(pMeta);
            return rc;
        }
    }

  if (foundWrapper) {
    /* Optional Metinf root tag was used in this message.
     * The actual token is the closing root tag.
     * It is required that the scanner points to the first tag _after_
     * <MetInf>...</MetInf>, so we just skip to the next token and continue.
     */
      if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlFreeMetinfMetinf(pMeta);
        //smlLibFree(pMeta);
        return rc;
      }
  }
    *ppElem = pMeta;

    return SML_ERR_OK;
}



/* see xltenc.c:XltEncBlock for description of parameters */
Ret_t metinfEncBlock(XltTagID_t tagId, XltRO_t reqOptFlag, const VoidPtr_t pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr, SmlPcdataExtension_t attFlag) {
  //Return variable
  Ret_t _err;
  SmlPcdataListPtr_t pList = NULL;
  //Check if pContent of a required field is missing
  if ((reqOptFlag == REQUIRED) && (pContent == NULL))
    return SML_ERR_XLT_MISSING_CONT;
  //Check if pContent of a optional field is missing -> if yes we are done
  else if (pContent == NULL)
    return SML_ERR_OK;

  //Generate the commands -> see DTD
  switch (tagId) {
    case TN_METINF_ANCHOR:
      if ((_err = xltGenerateTag(TN_METINF_ANCHOR, TT_BEG, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
      if ((_err = metinfEncBlock(TN_METINF_LAST, OPTIONAL, ((SmlMetInfAnchorPtr_t) pContent)->last, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
      if ((_err = metinfEncBlock(TN_METINF_NEXT, REQUIRED, ((SmlMetInfAnchorPtr_t) pContent)->next, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
      if ((_err = xltGenerateTag(TN_METINF_ANCHOR, TT_END, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
      break;
    case TN_METINF_MEM:
      if ((_err = xltGenerateTag(TN_METINF_MEM, TT_BEG, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
      if ((_err = metinfEncBlock(TN_METINF_SHAREDMEM, OPTIONAL, &(((SmlMetInfMemPtr_t) pContent)->flags), enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
      if ((_err = metinfEncBlock(TN_METINF_FREEMEM,   REQUIRED, ((SmlMetInfMemPtr_t) pContent)->free,   enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
      if ((_err = metinfEncBlock(TN_METINF_FREEID,    REQUIRED, ((SmlMetInfMemPtr_t) pContent)->freeid, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
      if ((_err = xltGenerateTag(TN_METINF_MEM, TT_END, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
      break;
    case TN_METINF_SHAREDMEM:
        //set the flag in the (WB)XML document if the flag is in the pContent
        if ((*((Flag_t *) pContent)) & (SmlMetInfSharedMem_f))
          if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        break;
    case TN_METINF_FIELDLEVEL:
        //set the flag in the (WB)XML document if the flag is in the pContent
        if ((*((Flag_t *) pContent)) & (SmlMetInfFieldLevel_f))
          if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        break;
    case TN_METINF_METINF:
        //if ((_err = xltGenerateTag(TN_METINF_METINF, TT_BEG, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        if ((_err = metinfEncBlock(TN_METINF_FORMAT,    OPTIONAL, ((SmlMetInfMetInfPtr_t) pContent)->format, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        if ((_err = metinfEncBlock(TN_METINF_TYPE,      OPTIONAL, ((SmlMetInfMetInfPtr_t) pContent)->type, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        if ((_err = metinfEncBlock(TN_METINF_MARK,      OPTIONAL, ((SmlMetInfMetInfPtr_t) pContent)->mark, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        if ((_err = metinfEncBlock(TN_METINF_SIZE,      OPTIONAL, ((SmlMetInfMetInfPtr_t) pContent)->size, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        if ((_err = metinfEncBlock(TN_METINF_ANCHOR,    OPTIONAL, ((SmlMetInfMetInfPtr_t) pContent)->anchor, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        if ((_err = metinfEncBlock(TN_METINF_VERSION,   OPTIONAL, ((SmlMetInfMetInfPtr_t) pContent)->version, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        if ((_err = metinfEncBlock(TN_METINF_NEXTNONCE, OPTIONAL, ((SmlMetInfMetInfPtr_t) pContent)->nextnonce, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        if ((_err = metinfEncBlock(TN_METINF_MAXMSGSIZE,OPTIONAL, ((SmlMetInfMetInfPtr_t) pContent)->maxmsgsize, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        // %%% luz 2003-04-24: added maxobjsize generation (was missing = bug in original RTK 4.1)
        if ((_err = metinfEncBlock(TN_METINF_MAXOBJSIZE,OPTIONAL, ((SmlMetInfMetInfPtr_t) pContent)->maxobjsize, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        pList = ((SmlMetInfMetInfPtr_t)pContent)->emi;
            while (pList != NULL) {
                if ((_err = xltEncBlock(TN_METINF_EMI,  OPTIONAL, pList->data, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
                pList = pList->next;
            };

        if ((_err = metinfEncBlock(TN_METINF_MEM,       OPTIONAL, ((SmlMetInfMetInfPtr_t) pContent)->mem, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        /* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
        if ((_err = metinfEncBlock(TN_METINF_FIELDLEVEL,OPTIONAL, &(((SmlMetInfMetInfPtr_t) pContent)->flags), enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;

        //if ((_err = xltGenerateTag(TN_METINF_METINF, TT_END, enc, pBufMgr, SML_EXT_METINF)) != SML_ERR_OK) return _err;
        break;
    default: { // all leaf nodes (PCDATA#)
        return xltEncPcdata(tagId, reqOptFlag, pContent, enc, pBufMgr, attFlag);
      } /* eof default statement from switch tagid */
  } /* eof switch tagid */
  return SML_ERR_OK;
}

#endif /* __USE_METINF__ */

