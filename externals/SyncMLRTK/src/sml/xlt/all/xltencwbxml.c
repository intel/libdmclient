/**
 * @file
 * The WBXML Encoder source file
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
#ifdef __SML_WBXML__
/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/

#include "xltencwbxml.h"
#include <libmem.h>
#include <libstr.h>
#include "xlttags.h"


/**
 * Converts a element type into its wbxml token
 *
 * @pre valid element type
 * @post return of wbxml token
 * @param elType (IN)
 *        element type
 * @return wbxml token\n
 *                  0, if no matching wbxml token
 */
MemByte_t wbxmlGetGlobToken(XltElementType_t elType)
{

  typedef struct GlobTok_s
  {
    XltElementType_t id;
    MemByte_t        wbxml;
  } GlobTok_t;

  // encoding of global tokens; related to the type XML_ElementType_t
  GlobTok_t globtoken[] =
  {
    { END,    0x01 },  //Tag End
    { STR_I,  0x03 },  //Inline string
    { OPAQUE, 0xC3 },  //Opaque Data
    { UNDEF,  0x00 }
  };

  int i = -1;
  while (globtoken[++i].id != UNDEF)
    if (globtoken[i].id == elType)
      return globtoken[i].wbxml;
  return 0;

}

/**
 * Converts a Long_t opaque size to a wbxml mb_u_int32 and adds it to the buffer
 *
 * @pre size of the content to be written as opaque datatype
 * @post the size is converted to the mb_u_int32 representation and added
 *                  to the buffer
 * @param size (IN)
 *        length of the opaque data
 * @param pBufMgr (IN/OUT)
 *        pointer to a structure containing buffer management elements
 * @return shows error codes of function,\n
 *                  0, if OK
 */
Ret_t wbxmlOpaqueSize2Buf(Long_t size, BufferMgmtPtr_t pBufMgr)
{
  Long_t _thresholdcount = 1;
  Long_t _bytesNeeded = 0;
  MemPtr_t _byteArray;
  MemPtr_t _tmpByteArray;
  int i, j;
  Ret_t _err;

  //j max = number of bytes of size
   for (j=1; j<=sizeof(size); j++)
  {
    //if the size of the content is smaller than the power of 128,j ->
    //one more byte is needed in the mb_u_int32 representation of WBXML
     _thresholdcount = _thresholdcount * 128;
     if(size < _thresholdcount)
     {
       _bytesNeeded = j;
       break;
     }
  }

  if (pBufMgr->spaceEvaluation == 0)
  {
    //allocate number of bytes needed by the mb_u_int32 data type
    if ((_byteArray = smlLibMalloc(_bytesNeeded)) == NULL) return SML_ERR_NOT_ENOUGH_SPACE;

    _tmpByteArray = _byteArray;

    //process al bytes in the mb_u_int32 data type
    for (i=1; i<=_bytesNeeded; i++)
    {
      // the lowest byte needs a 0 in its highest bit -> no | 0x80
      if ((_bytesNeeded - i) == 0)
      {
          *_tmpByteArray = ((unsigned char)(size & 0x7F));
      }
      // all the other byte needs a 1 in its highest bit -> | 0x80
      else
      {
        // only the seven lower bits contain the size value -> >> 7
          *_tmpByteArray = ((unsigned char)(((size >> (7 * (_bytesNeeded - i))) & 0x7F) | 0x80));
        _tmpByteArray++;
      }
    }

    _err = xltAddToBuffer(_byteArray, _bytesNeeded, pBufMgr);

    smlLibFree(_byteArray);
  } else {
    pBufMgr->smlXltWrittenBytes += _bytesNeeded;
    // %%% luz 2002-09-03: return value was missing here.
    _err=SML_ERR_OK;
  }

  return _err;
}

/**
 * Generates a tag for a given tag ID and a given tag type
 *
 * @pre valid parameters
 * @post a new wbxml tag is written to the buffer
 * @param tagId (IN)
 *        the ID for the tag to generate (TN_ADD, ...)
 * @param tagType (IN)
 *        the tag type (e.g. Begin Tag -> TT_BEG, ...)
 * @param pBufMgr (IN/OUT)
 *        pointer to a structure containing buffer management elements
 * @return shows error codes of function,\n
 *                  0, if OK
 */
Ret_t wbxmlGenerateTag(XltTagID_t tagId, XltTagType_t tagType, BufferMgmtPtr_t pBufMgr )
{

  Ret_t _err = SML_ERR_OK;
  MemByte_t _tmp = 0x00;

  //check if content byte has to be added to the tag
  switch (tagType)
  {
    //set the end tag
    case TT_END:
    {
      _tmp = (MemByte_t)wbxmlGetGlobToken(END);
      if (!_tmp) return SML_ERR_XLT_INVAL_TAG_TYPE;
      _err = xltAddToBuffer((&_tmp), 1, pBufMgr);
      // remember the number of byte that must follow for the according  end-tag
    if (_err == SML_ERR_OK) pBufMgr->endTagSize -= 1;
    return _err;
    }
    //Begin and End Tag in one
    case TT_ALL:
    {
      _err = (MemByte_t)getTagByte(tagId, pBufMgr->smlCurExt, &_tmp);
      if ((!_tmp) || (_err != SML_ERR_OK)) return _err;
      return xltAddToBuffer((MemPtr_t)(&_tmp), 1, pBufMgr);
    }
    //Only Begin Tag -> content follows -> content byte has to be added
    case TT_BEG:
    {
      _err = (MemByte_t)getTagByte(tagId, pBufMgr->smlCurExt, &_tmp);
      if ((!_tmp) || (_err != SML_ERR_OK)) return _err;

      _tmp = ((MemByte_t)(_tmp | XLT_CONTBYTE));
      _err = xltAddToBuffer(&_tmp, 1, pBufMgr);
      // remember the number of byte that must follow for the according  end-tag
    if (_err == SML_ERR_OK) pBufMgr->endTagSize += 1;
    return _err;
    }
  default:
    return SML_ERR_XLT_INVAL_TAG_TYPE;
  }

 // return SML_ERR_OK;Unreachable
}

/**
 * Write a content of a certain WBXML element type (e.g.\ STR_I)
 * to the global buffer
 *
 * @pre valid parameters
 * @post the content is written to the wbxml buffer with the leading
 *                  bytes for the opaque data type or the STR_I data type
 * @param pContent (IN)
 *        the character pointer referencing the content to
 *                            write to the buffer
 * @param elType (IN)
 *        the element type to write to the buffer (e.g. STR_I)
 * @param size (IN)
 *        the content length
 * @param pBufMgr (IN/OUT)
 *        pointer to a structure containing buffer management elements
 * @return shows error codes of function,\n
 *                  0, if OK
 */
Ret_t wbxmlWriteTypeToBuffer(const MemPtr_t pContent, XltElementType_t elType, Long_t size, BufferMgmtPtr_t pBufMgr)
{
  Ret_t _err;

  MemByte_t _termstr = XLT_TERMSTR;
  MemByte_t _tmp;

  switch((int)elType)
  {
    case TAG:
    {

      return (xltAddToBuffer(pContent, size, pBufMgr));

    }
    case STR_I:
    {
        _tmp = (MemByte_t)wbxmlGetGlobToken(STR_I);
       if (!_tmp) return SML_ERR_XLT_WBXML_UKN_TOK;

      //add the STR_I identifier
      if ((_err = xltAddToBuffer(&_tmp, 1, pBufMgr)) != SML_ERR_OK) return _err;

      //add the string to the buffer
      if ((_err = xltAddToBuffer(pContent, (!pContent) ? 0 : smlLibStrlen((String_t)pContent), pBufMgr)) != SML_ERR_OK) return _err;

      //add the string terminator '\0'
      if ((_err = xltAddToBuffer(&_termstr, 1, pBufMgr)) != SML_ERR_OK) return _err;

      return SML_ERR_OK;
    }
    case OPAQUE:
    {
      _tmp = (MemByte_t)wbxmlGetGlobToken(OPAQUE);
       if (!_tmp) return SML_ERR_XLT_WBXML_UKN_TOK;



      //add the OPAQUE identifier
      if ((_err = xltAddToBuffer(&_tmp, 1, pBufMgr)) != SML_ERR_OK) return _err;

      //add the pContent length
      if ((_err = wbxmlOpaqueSize2Buf(size, pBufMgr)) != SML_ERR_OK) return _err;

      //add the string buffer
      if ((_err = xltAddToBuffer(pContent, size, pBufMgr)) != SML_ERR_OK) return _err;

      return SML_ERR_OK;
    }
  default:
    return SML_ERR_XLT_INVAL_PCDATA_TYPE;
  }

//  return SML_ERR_OK;unreachable
}
#endif
