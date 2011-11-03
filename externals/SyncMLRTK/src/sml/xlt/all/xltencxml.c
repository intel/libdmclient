/**
 * @file
 * The XML Encoder source file
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

#include "syncml_tk_prefix_file.h" // %%% luz: needed for precompiled headers in eVC++

#include "define.h"
#ifdef __SML_XML__
/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/

#include "xltencxml.h"
#include <libstr.h>
#include "xlttags.h"
#include <libmem.h>

/**
 * Generates a XML tag
 *
 * @pre valid parameters
 * @post the XML tag is written to the XML buffer
 * @param tagId (IN)
 *        the ID for the tag to generate (TN_ADD, ...)
 * @param tagType (IN)
 *        the tag type (e.g. Begin Tag -> TT_BEG, ...)
 * @param attFlag (IN)
 *        indicates if the encoded tag contain Attributes in namespace
 *        extensions
 * @param pBufMgr (IN/OUT)
 *        pointer to a structure containing buffer management elements
 * @return shows error codes of function,\n
 *                  0, if OK
 */
Ret_t xmlGenerateTag(XltTagID_t tagId, XltTagType_t tagType, BufferMgmtPtr_t pBufMgr, SmlPcdataExtension_t attFlag)
{
  Ret_t _err;

  MemByte_t _begpar = XML_BEGPAR;
  MemByte_t _tagdel = XML_TAGDEL;
  MemByte_t _endpar = XML_ENDPAR;
  MemByte_t _nstagstart[] = XML_NSSTART;
  MemByte_t _nstagend[]   = XML_NSEND;


  String_t _tagstr;
  String_t _tagnsattr = NULL;

  if ((_tagstr = (String_t)smlLibMalloc(XML_MAX_TAGLEN)) == NULL) return SML_ERR_NOT_ENOUGH_SPACE;

  if ((_err = getTagString(tagId, _tagstr, attFlag)) != SML_ERR_OK)  {
    smlLibFree(_tagstr);
    return _err;
  }

  if (!_tagstr)  {  // check again as _tagstr might be alterd in getTagString
    smlLibFree(_tagstr);
    return SML_ERR_XLT_INVAL_TAG_TYPE;
  }

    /* the <SyncML> tag _must_ have an xmlns attribute */
  if (attFlag != pBufMgr->smlActiveExt || tagId == TN_SYNCML) {
    // %%% luz:2003-07-31: now uses namespace from table according to version
    if (getExtName(attFlag, &_tagnsattr, pBufMgr->vers) != SML_ERR_OK) {
      smlLibFree(_tagstr);
      return  SML_ERR_XLT_INVAL_TAG_TYPE;
    }
  }
  pBufMgr->smlActiveExt = attFlag;
  //check if content byte has to be added to the tag
  switch (tagType)
  {
    // set the end tag
    case TT_END:
    {
      if ((_err = xltAddToBuffer((MemPtr_t)(&_begpar), 1, pBufMgr)) != SML_ERR_OK) break;
      if ((_err = xltAddToBuffer((MemPtr_t)(&_tagdel), 1, pBufMgr)) != SML_ERR_OK) break;
      if ((_err = xltAddToBuffer((MemPtr_t)_tagstr, smlLibStrlen(_tagstr), pBufMgr)) != SML_ERR_OK) break;
      if ((_err = xltAddToBuffer((MemPtr_t)(&_endpar), 1, pBufMgr)) != SML_ERR_OK) break;
      if (tagId == pBufMgr->switchExtTag) {
        pBufMgr->smlActiveExt = pBufMgr->smlLastExt;
        pBufMgr->smlCurExt    = pBufMgr->smlLastExt;
        pBufMgr->smlLastExt   = attFlag;
      }
      // just forget the stored number ob bytes for this end-tag since written now
    pBufMgr->endTagSize -= (3 + smlLibStrlen(_tagstr));
      break;
    }
    //Empty tag
    case TT_ALL:
    {
      if ((_err = xltAddToBuffer((MemPtr_t)(&_begpar), 1, pBufMgr)) != SML_ERR_OK) break;
      if ((_err = xltAddToBuffer((MemPtr_t)_tagstr, smlLibStrlen(_tagstr), pBufMgr)) != SML_ERR_OK) break;
      if (_tagnsattr) {
        if ((_err = xltAddToBuffer((MemPtr_t)(&_nstagstart), 8, pBufMgr)) != SML_ERR_OK) break;
        if ((_err = xltAddToBuffer((MemPtr_t)_tagnsattr, smlLibStrlen(_tagnsattr), pBufMgr)) != SML_ERR_OK) break;
        if ((_err = xltAddToBuffer((MemPtr_t)&_nstagend, 1, pBufMgr)) != SML_ERR_OK) break;
      }
      if ((_err = xltAddToBuffer((MemPtr_t)(&_tagdel), 1, pBufMgr)) != SML_ERR_OK) break;
      if ((_err = xltAddToBuffer((MemPtr_t)(&_endpar), 1, pBufMgr)) != SML_ERR_OK) break;

      break;
    }
    //Only Begin Tag -> content follows -> content byte has to be added
    case TT_BEG:
    {
      if ((_err = xltAddToBuffer((MemPtr_t)(&_begpar), 1, pBufMgr)) != SML_ERR_OK) break;
      if ((_err = xltAddToBuffer((MemPtr_t)_tagstr, smlLibStrlen(_tagstr), pBufMgr)) != SML_ERR_OK) break;
      if (_tagnsattr) {
        if ((_err = xltAddToBuffer((MemPtr_t)&_nstagstart, 8, pBufMgr)) != SML_ERR_OK) break;
        if ((_err = xltAddToBuffer((MemPtr_t)_tagnsattr, smlLibStrlen(_tagnsattr), pBufMgr)) != SML_ERR_OK) break;
        if ((_err = xltAddToBuffer((MemPtr_t)&_nstagend, 1, pBufMgr)) != SML_ERR_OK) break;
      }
      if ((_err = xltAddToBuffer((MemPtr_t)(&_endpar), 1, pBufMgr)) != SML_ERR_OK) break;

    // remember the number of byte that must follow for the according  end-tag
    pBufMgr->endTagSize += (3 + smlLibStrlen(_tagstr));
      break;
    }
    default:
    {
      smlLibFree(_tagstr);
    smlLibFree(_tagnsattr);
      return SML_ERR_XLT_INVAL_TAG_TYPE;
    }
  }
  smlLibFree(_tagstr);
  smlLibFree(_tagnsattr);
  return _err;
}
#endif
