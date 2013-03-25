/**
 * @file
 * Encoder source file
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
#include "syncml_tk_prefix_file.h" // %%% luz: needed for precompiled headers in eVC++

#include <xltenc.h>
#ifdef __SML_WBXML__
  #include "xltencwbxml.h"
#endif
#ifdef __SML_XML__
  #include "xltencxml.h"
#endif
#include "xlttags.h"
#include "xltmetinf.h"
#include "xltdevinf.h"
#include "smlmetinfdtd.h"
#include "smldevinfdtd.h"
#include <libstr.h>
#include <libmem.h>
#include <smldtd.h>

Ret_t getTNbyPE(SmlProtoElement_t, XltTagID_t*);
Ret_t xltEncBlock(XltTagID_t, XltRO_t, const VoidPtr_t, SmlEncoding_t, BufferMgmtPtr_t, SmlPcdataExtension_t);
Ret_t xltEncList(XltListType_t, XltRO_t, VoidPtr_t, SmlEncoding_t, BufferMgmtPtr_t, SmlPcdataExtension_t);
#ifndef __SML_LITE__
Ret_t xltBuildExtention(SmlPcdataExtension_t, XltRO_t, VoidPtr_t, SmlEncoding_t, BufferMgmtPtr_t);
#endif

typedef struct PEEnc_s
{
    XltTagID_t     tagid;
    SmlProtoElement_t type;
} PEEnc_t, *PEEncPtr_t;

/* luz 2005-08-17: simplified, removed unnecessary copying of the const static table */

static const PEEnc_t cPE_Enc[] =
  {
    { TN_ADD,          SML_PE_ADD            },
    { TN_ALERT,        SML_PE_ALERT          },
    { TN_ATOMIC,       SML_PE_ATOMIC_START   },
    { TN_ATOMIC_END,   SML_PE_ATOMIC_END     },
    { TN_COPY,         SML_PE_COPY           },
    { TN_DELETE,       SML_PE_DELETE         },
    { TN_EXEC,         SML_PE_EXEC           },
    { TN_GET,          SML_PE_GET            },
    { TN_MAP,          SML_PE_MAP            },
    { TN_PUT,          SML_PE_PUT            },
    { TN_RESULTS,      SML_PE_RESULTS        },
    { TN_SEARCH,       SML_PE_SEARCH         },
    { TN_SEQUENCE,     SML_PE_SEQUENCE_START },
    { TN_SEQUENCE_END, SML_PE_SEQUENCE_END   },
    { TN_STATUS,       SML_PE_STATUS         },
    { TN_SYNC,         SML_PE_SYNC_START     },
    { TN_SYNC_END,     SML_PE_SYNC_END       },
    { TN_REPLACE,      SML_PE_REPLACE        },
    { TN_UNDEF,        SML_PE_UNDEF          }
  };


Ret_t getTNbyPE(SmlProtoElement_t pE, XltTagID_t *tagID)
{
    int i = 0;
    while ((cPE_Enc[i].type) != SML_PE_UNDEF)
    {
      if ((cPE_Enc[i].type) == pE)
      {
        *tagID = cPE_Enc[i].tagid;
        return SML_ERR_OK;
      }
      i++;
    }
    *tagID = TN_UNDEF;
    return SML_ERR_XLT_INVAL_PROTO_ELEM;
}


// %%% luz:2003-07-31: added SyncML FPI (formal public identifier) tables
const char * const SyncMLFPI[SML_NUM_VERS] = {
  "???",
  "-//SYNCML//DTD SyncML 1.0//EN",
  "-//SYNCML//DTD SyncML 1.1//EN",
  "-//SYNCML//DTD SyncML 1.2//EN"
};
const char * const SyncMLDevInfFPI[SML_NUM_VERS] = {
  "???",
  "-//SYNCML//DTD DevInf 1.0//EN",
  "-//SYNCML//DTD DevInf 1.1//EN",
  "-//SYNCML//DTD DevInf 1.2//EN"
};
const int SyncMLWBXMLPublicID[SML_NUM_VERS] = {
  0,
  0, // we use the string ID, not the binary value (altough it seems to be defined as 0x0FD1
  0x0FD3,
  0x1201
};
const int SyncMLDevInfWBXMLPublicID[SML_NUM_VERS] = {
  0,
  0,
  0x0FD4, // according to Nokia E70
  0x1203 // according to Nokia E70
};



/**
 * Initializes an XML buffer; Creates XML code for the SyncHdr
 * and appends it to the buffer.
 * Returns 0 if operation was successful.
 *
 * @pre no memory should be allocated for ppEncoder (should be NULL)
 *      pHeader has to contain a valid SyncHdr structure
 *      pBufEnd must point to the end of the (WB)XML buffer
 *      ppBufPos has to be initialized to the start point of the
 *      (WB)XML buffer.
 * @post After the function call ppBufPos points to the
 *       first free byte in the buffer behind the (WB)XML document
 * @param enc (IN)
 *        the encoding constant (SML_WBXML or SML_XML)
 * @param pHeader (IN)
 *        the SyncML header structure
 * @param pBufEnd (IN)
 *        pointer to the end of the buffer to write on
 * @param ppBufPos (IN/OUT)
 *        current position of the bufferpointer
 * @param ppEncoder (IN/OUT)
 *        the encoder object
 * @param vers (IN)
 *        SyncML version
 * @return shows error codes of function,\n
 *         0, if OK\n
 *         Possible Error Codes:
 *         - SML_ERR_XLT_MISSING_CONT
 *         - SML_ERR_XLT_BUF_ERR
 *         - SML_ERR_XLT_INVAL_ELEM_TYPE
 *         - SML_ERR_XLT_INVAL_LIST_TYPE
 *         - SML_ERR_XLT_INVAL_TAG_TYPE
 *         - SML_ERR_XLT_ENC_UNK
 *         - SML_ERR_XLT_INVAL_PROTO_ELEM
 */
Ret_t xltEncInit(SmlEncoding_t enc,
                 const SmlSyncHdrPtr_t pHeader,
                 const MemPtr_t pBufEnd,
                 MemPtr_t *ppBufPos,
                 XltEncoderPtr_t *ppEncoder,
                 SmlVersion_t vers)
{
  // Return variable
  Ret_t _err;

  XltEncoderPtr_t _pEncoder;

  //Structure containing buffer pointers, length and written bytes
  BufferMgmtPtr_t _pBufMgr;

#ifdef __SML_XML__
  MemPtr_t _tmpStr;
  MemPtr_t _xmlver = (MemPtr_t)XML_VERSION;
  MemPtr_t _xmlenc = (MemPtr_t)XML_ENCODING;
  MemByte_t _begpar = XML_BEGPAR;
  MemByte_t _endpar = XML_ENDPAR;
#endif

#ifdef __SML_WBXML__
  MemByte_t _wbxmlver    = XLT_WBXMLVER;
  MemByte_t _charset     = XLT_CHARSET;
  // check if we want to send numeric or textual FPI
  MemByte_t _pubident1   = 0; // default to textual
  MemByte_t _pubident2   = 0;
  const char *_syncmldtd = SyncMLFPI[vers];
  unsigned short fpi     = SyncMLWBXMLPublicID[vers];
  MemByte_t _stablen     = smlLibStrlen(_syncmldtd); // %% not 0x1D constant!
  if (fpi) {
    // numeric FPI available
    _pubident1 = ((fpi>>7) & 0x7F)+0x80; // upper 7 bit
    _pubident2 = (fpi & 0x7F); // lower 7 bits
    _stablen = 0; // no string table
  }
#endif

  //MemByte_t _tmp = 0x00;CURRENTLY NOT USED

  if ((_pEncoder = (XltEncoderPtr_t)smlLibMalloc(sizeof(XltEncoder_t))) == NULL) return SML_ERR_NOT_ENOUGH_SPACE;
  if ((_pBufMgr = (BufferMgmtPtr_t)smlLibMalloc(sizeof(BufferMgmt_t))) == NULL) {
      smlLibFree(_pEncoder);
      return SML_ERR_NOT_ENOUGH_SPACE;
  }

  //set the encoding
  _pEncoder->enc = enc;

  // %%% luz:2003-07-31: added version
  _pEncoder->vers = vers;

  _pEncoder->cur_ext = (SmlPcdataExtension_t)SML_EXT_UNDEFINED;
  _pEncoder->last_ext = (SmlPcdataExtension_t)SML_EXT_UNDEFINED;
  _pEncoder->end_tag_size = 0;
  _pEncoder->space_evaluation = NULL;


  _pBufMgr->smlXltBufferP = *ppBufPos;
  _pBufMgr->smlXltBufferLen = pBufEnd - *ppBufPos;
  _pBufMgr->smlXltStoreBufP = _pBufMgr->smlXltBufferP;
  _pBufMgr->smlXltWrittenBytes = 0;
  _pBufMgr->smlCurExt = _pEncoder->cur_ext;
  _pBufMgr->smlLastExt = _pEncoder->last_ext;
  _pBufMgr->smlActiveExt = SML_EXT_UNDEFINED;
  _pBufMgr->switchExtTag = TN_UNDEF;
  _pBufMgr->spaceEvaluation = 0;
  _pBufMgr->vers = vers;
  _pBufMgr->endTagSize = 0;

  switch (enc)
  {

#ifdef __SML_WBXML__
    case SML_WBXML:
    {

      // Set the WBXML Header Values
      // WBXML Version
      if ((_err = wbxmlWriteTypeToBuffer((MemPtr_t)(&_wbxmlver), TAG, 1, _pBufMgr)) != SML_ERR_OK) break;
      // Public Idetifier - default unknown
      if ((_err = wbxmlWriteTypeToBuffer((MemPtr_t)(&_pubident1), TAG, 1, _pBufMgr)) != SML_ERR_OK) break;
      if ((_err = wbxmlWriteTypeToBuffer((MemPtr_t)(&_pubident2), TAG, 1, _pBufMgr)) != SML_ERR_OK) break;
      // Character set - not yet implemented
      if ((_err = wbxmlWriteTypeToBuffer((MemPtr_t)(&_charset), TAG, 1, _pBufMgr)) != SML_ERR_OK) break;
      // - String table length -  only used for textual FPI
      if ((_err = wbxmlWriteTypeToBuffer((MemPtr_t)(&_stablen), TAG, 1, _pBufMgr)) != SML_ERR_OK) break;
      // FPI - %%% luz:2003-07-31: not constant any more, varies according to SyncML version
      if (fpi==0) {
        // textual FPI
        // - string table consisting of FPI only
        if ((_err = xltAddToBuffer((MemPtr_t) (_syncmldtd) , smlLibStrlen((String_t)_syncmldtd), _pBufMgr)) != SML_ERR_OK) break;
      }
      else {
        // numeric FPI
        // NOP - no string table
      }

      break;
    }
#endif

#ifdef __SML_XML__
    case SML_XML:
    {

      if ((_err = xltAddToBuffer((MemPtr_t)(&_begpar), 1, _pBufMgr)) != SML_ERR_OK) break;
      _tmpStr = (MemPtr_t)"?xml version=\"";
      if ((_err = xltAddToBuffer(_tmpStr, smlLibStrlen((String_t)_tmpStr), _pBufMgr)) != SML_ERR_OK) break;
      _tmpStr = _xmlver;
      if ((_err = xltAddToBuffer(_tmpStr, smlLibStrlen((String_t)_tmpStr), _pBufMgr)) != SML_ERR_OK) break;
      _tmpStr = (MemPtr_t)"\" encoding=\"";
      if ((_err = xltAddToBuffer(_tmpStr, smlLibStrlen((String_t)_tmpStr), _pBufMgr)) != SML_ERR_OK) break;
      _tmpStr = _xmlenc;
      if ((_err = xltAddToBuffer(_tmpStr, smlLibStrlen((String_t)_tmpStr), _pBufMgr)) != SML_ERR_OK) break;
      _tmpStr = (MemPtr_t)"\"?";
      if ((_err = xltAddToBuffer(_tmpStr, smlLibStrlen((String_t)_tmpStr), _pBufMgr)) != SML_ERR_OK) break;
      if ((_err = xltAddToBuffer((MemPtr_t)(&_endpar), 1, _pBufMgr)) != SML_ERR_OK) break;

      break;
    }
#endif

    default:
    {
      _err = SML_ERR_XLT_ENC_UNK;
    }
  }
  if (_err != SML_ERR_OK)
  {
     smlLibFree(_pBufMgr);
     smlLibFree(_pEncoder);
     return _err;
  }

  // SyncML Tag
  if ((_err = xltGenerateTag(TN_SYNCML, TT_BEG, enc, _pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK)
  {
    smlLibFree(_pBufMgr);
    smlLibFree(_pEncoder);
    return _err;
  }

  // Generate SmlSyncHdr

  if ((_err = xltEncBlock(TN_SYNCHDR, REQUIRED, pHeader, enc, _pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK)
  {
    smlLibFree(_pBufMgr);
    smlLibFree(_pEncoder);
    return _err;
  }

  // SyncBody Tag
  if ((_err = xltGenerateTag(TN_SYNCBODY, TT_BEG, enc, _pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK)
  {
    smlLibFree(_pBufMgr);
    smlLibFree(_pEncoder);
    return _err;
  }

  _pEncoder->cur_ext = _pBufMgr->smlCurExt;
  _pEncoder->last_ext = _pBufMgr->smlLastExt;
  _pEncoder->end_tag_size = _pBufMgr->endTagSize;

  *ppBufPos = _pBufMgr->smlXltBufferP;

  smlLibFree(_pBufMgr);

  _pEncoder->final = 0;

  *ppEncoder = (XltEncoderPtr_t)_pEncoder;

  return SML_ERR_OK;
}

/**
 * Generates (WB)XML code and appends it to the XML buffer.
 *
 * @pre pEncoder holds the initialized encoder structure.
 *      the initialization takes place in the xltEncAppend function
 *      pContent has to contain a valid content structure structure
 *      pBufEnd must point to the end of the (WB)XML buffer
 *      ppBufPos has to be initialized to the start point of the
 *      (WB)XML buffer.
 * @post After the function call ppBufPos points to the
 *       first free byte in the buffer behind the (WB)XML document
 * @param pEncoder (IN)
 *        the encoder object
 * @param pe (IN)
 *        the protocol element (PE_ADD, ...)
 * @param pBufEnd (IN)
 *        pointer to the end of the buffer to write on
 * @param pContent (IN)
 *        the content to append to the SyncML document
 * @param ppBufPos (IN/OUT)
 *        current position of the bufferpointer
 * @return shows error codes of function,\n
 *         0, if OK\n
 *         Possible Error Codes:
 *         - SML_ERR_XLT_MISSING_CONT
 *         - SML_ERR_XLT_BUF_ERR
 *         - SML_ERR_XLT_INVAL_ELEM_TYPE
 *         - SML_ERR_XLT_INVAL_LIST_TYPE
 *         - SML_ERR_XLT_INVAL_TAG_TYPE
 *         - SML_ERR_XLT_ENC_UNK
 *         - SML_ERR_XLT_INVAL_PROTO_ELEM
 */
Ret_t xltEncAppend(const XltEncoderPtr_t pEncoder,
                   SmlProtoElement_t pe,
                   const MemPtr_t pBufEnd,
                   const VoidPtr_t pContent,
                   MemPtr_t *ppBufPos)
{
  // Return variable
  Ret_t _err;

  XltTagID_t tagID = TN_UNDEF;

  // encoding type
  SmlEncoding_t _enc;

  //Structure containing buffer pointers, length and written bytes
  BufferMgmtPtr_t _pBufMgr;

  if ((_pBufMgr = (BufferMgmtPtr_t)smlLibMalloc(sizeof(BufferMgmt_t))) == NULL) return SML_ERR_NOT_ENOUGH_SPACE;
  smlLibMemset(_pBufMgr, 0, sizeof(BufferMgmt_t));

  //get the encoding type
  _enc = pEncoder->enc;

  _pBufMgr->vers = pEncoder->vers; // %%% luz:2003-07-31: pass SyncML version to bufmgr
  _pBufMgr->smlXltBufferP = *ppBufPos;
  _pBufMgr->smlXltBufferLen = pBufEnd - *ppBufPos;
  _pBufMgr->smlXltStoreBufP = _pBufMgr->smlXltBufferP;
  _pBufMgr->smlXltWrittenBytes = 0;
  _pBufMgr->smlActiveExt = SML_EXT_UNDEFINED;
  _pBufMgr->switchExtTag = TN_UNDEF;
  _pBufMgr->spaceEvaluation = ((pEncoder->space_evaluation == NULL) ? 0 : 1);
  // %%% luz 2002-09-03: evaluation may not mess with encoder state
  if ( _pBufMgr->spaceEvaluation) {
    // spaceEval state
    _pBufMgr->smlCurExt = pEncoder->space_evaluation->cur_ext;
    _pBufMgr->smlLastExt = pEncoder->space_evaluation->last_ext;
  }
  else {
    // normal encoder state
    _pBufMgr->smlCurExt = pEncoder->cur_ext;
    _pBufMgr->smlLastExt = pEncoder->last_ext;
  }

  _pBufMgr->endTagSize =0;

  _err = getTNbyPE(pe, &tagID);

  _err = xltEncBlock(tagID, REQUIRED, pContent, _enc, _pBufMgr, SML_EXT_UNDEFINED);
  if (_err != SML_ERR_OK)
  {
     smlLibFree(_pBufMgr);
     return _err;
  }


  if (pEncoder->space_evaluation != NULL) {
    // Only calculating size
    pEncoder->space_evaluation->written_bytes += _pBufMgr->smlXltWrittenBytes;
    pEncoder->space_evaluation->end_tag_size += _pBufMgr->endTagSize;
    // save it only into evaluation state
    pEncoder->space_evaluation->cur_ext = _pBufMgr->smlCurExt;
    pEncoder->space_evaluation->last_ext = _pBufMgr->smlLastExt;
  } else {
    // really generating data
    pEncoder->end_tag_size += _pBufMgr->endTagSize;
    // save it into encoder state
    pEncoder->cur_ext = _pBufMgr->smlCurExt;
    pEncoder->last_ext = _pBufMgr->smlLastExt;
  }

  *ppBufPos = _pBufMgr->smlXltBufferP;

  smlLibFree(_pBufMgr);

  return SML_ERR_OK;
}

/**
 * Filnalizes the (WB)XML document and returns the size of written bytes to
 * the workspace module
 *
 * @pre pEncoder holds the initialized encoder structure.
 *      the initialization takes place in the xltEncAppend function
 *      pBufEnd must point to the end of the (WB)XML buffer
 *      ppBufPos has to be initialized to the start point of the
 *      (WB)XML buffer.
 * @post After the function call ppBufPos points to the
 *       first free byte in the buffer behind the (WB)XML document
 * @param pEncoder (IN)
 *        the encoder object
 * @param pBufEnd (IN)
 *        pointer to the end of the buffer to write on
 * @param ppBufPos (IN/OUT)
 *        current position of the bufferpointer
 * @return shows error codes of function,\n
 *         0, if OK\n
 *         Possible Error Codes:
 *         - SML_ERR_XLT_BUF_ERR
 *         - SML_ERR_XLT_MISSING_CONT
 *         - SML_ERR_XLT_INVAL_ELEM_TYPE
 *         - SML_ERR_XLT_INVAL_LIST_TYPE
 *         - SML_ERR_XLT_INVAL_TAG_TYPE
 *         - SML_ERR_XLT_ENC_UNK
 *         - SML_ERR_XLT_INVAL_PROTO_ELEM
 */
Ret_t xltEncTerminate(const XltEncoderPtr_t pEncoder,
                      const MemPtr_t pBufEnd,
                      MemPtr_t *ppBufPos)
{
  // Return variable
  Ret_t _err;

  // encoding type
  SmlEncoding_t _enc;

  //Structure containing buffer pointers, length and written bytes
  BufferMgmtPtr_t _pBufMgr;

  //get the encoding type
  _enc = pEncoder->enc;

  //Initialize buffer variables
  if ((_pBufMgr = smlLibMalloc(sizeof(BufferMgmt_t))) == NULL) {
    smlLibFree(pEncoder);
    return SML_ERR_NOT_ENOUGH_SPACE;
   }

  _pBufMgr->vers = pEncoder->vers; // %%% luz:2003-07-31: pass SyncML version to bufmgr
  _pBufMgr->smlXltWrittenBytes = 0;
  _pBufMgr->smlXltBufferP = *ppBufPos;
  _pBufMgr->smlXltStoreBufP = _pBufMgr->smlXltBufferP;
  _pBufMgr->smlXltBufferLen = pBufEnd - *ppBufPos;
  _pBufMgr->smlCurExt = pEncoder->cur_ext;
  _pBufMgr->smlLastExt = pEncoder->last_ext;
  _pBufMgr->smlActiveExt = pEncoder->cur_ext;
  _pBufMgr->switchExtTag = TN_UNDEF;
  _pBufMgr->spaceEvaluation = ((pEncoder->space_evaluation == NULL) ? 0 : 1);
  _pBufMgr->endTagSize =0;

  if (pEncoder->final == 1)
  {
    // Final Flag
      if ((_err = xltGenerateTag(TN_FINAL, TT_ALL, _enc, _pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK)
    {
      smlLibFree(_pBufMgr);
      xltEncReset(pEncoder);
      return _err;
    }
  }

  // SyncBody End Tag
  if ((_err = xltGenerateTag(TN_SYNCBODY, TT_END, _enc, _pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK)
  {
    smlLibFree(_pBufMgr);
    xltEncReset(pEncoder);
    return _err;
  }

  // SyncML End Tag
  if ((_err = xltGenerateTag(TN_SYNCML, TT_END, _enc, _pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK)
  {
    smlLibFree(_pBufMgr);
    xltEncReset(pEncoder);
    return _err;
  }

  pEncoder->cur_ext = _pBufMgr->smlCurExt;
  pEncoder->last_ext = _pBufMgr->smlLastExt;

  *ppBufPos = _pBufMgr->smlXltBufferP;

  smlLibFree(_pBufMgr);

  xltEncReset(pEncoder);

  return SML_ERR_OK;
}

Ret_t xltEncReset(XltEncoderPtr_t pEncoder)
{
  if ((pEncoder) && (pEncoder->space_evaluation)) {
    smlLibFree(pEncoder->space_evaluation);
    pEncoder->space_evaluation = NULL;
  }
  smlLibFree(pEncoder);
  return SML_ERR_OK;
}


/**
 * Starts an evaluation run which prevents further API-Calls to write tags -
 * just the tag-sizes are calculated. Must be sopped via smlEndEvaluation
 *
 * @param pEncoder (IN)
 *        the encoder object
 * @return Return Code
 */
SML_API Ret_t xltStartEvaluation(XltEncoderPtr_t pEncoder)
{
  XltSpaceEvaluationPtr_t _pSpaceEvaluation;

  if (pEncoder->space_evaluation != NULL)
    return SML_ERR_WRONG_USAGE;

  if ((_pSpaceEvaluation = (XltSpaceEvaluationPtr_t)smlLibMalloc(sizeof(XltSpaceEvaluation_t))) == NULL)
    return SML_ERR_NOT_ENOUGH_SPACE;
  smlLibMemset(_pSpaceEvaluation, 0, sizeof(XltSpaceEvaluation_t));
  // %%% luz 2002-09-03: init encoder state shadow copies for evaluation from real encoder
  _pSpaceEvaluation->cur_ext = pEncoder->cur_ext;
  _pSpaceEvaluation->last_ext = pEncoder->last_ext;

  pEncoder->space_evaluation = _pSpaceEvaluation;
  return SML_ERR_OK;
}


/**
 * Stops an evaluation run which prevents further API-Calls to write tags -
 * the remaining free buffer size after all Tags are written is returned
 *
 * @param id (IN)
 *        instance ID
 * @param pEncoder (IN)
 *        the encoder object
 * @param freemem (IN/OUT)
 *        Size of free buffer for data after all tags are written
 * @return Return Code
 */
SML_API Ret_t xltEndEvaluation(InstanceID_t id, XltEncoderPtr_t pEncoder, MemSize_t *freemem)
{

  MemSize_t _free;
  XltSpaceEvaluationPtr_t _evaluation;

  _evaluation = pEncoder->space_evaluation;

  if (_evaluation == NULL)
    return SML_ERR_WRONG_USAGE;

  _free = smlGetFreeBuffer(id);
  _free -= _evaluation->written_bytes;
  _free -= _evaluation->end_tag_size;
  _free -= pEncoder->end_tag_size;

  *freemem = _free;

  smlLibFree(_evaluation);
  pEncoder->space_evaluation = NULL;

  return SML_ERR_OK;
}


/**
 * Generates a (WB)XML Block for a given tag ID and a given content
 *
 * @pre pContent holds a valid content structure
 *      tagId contains a valid SyncML tag ID
 * @post the (WB)XML buffer in the pBufMgr structure contains the
 *       encoded (WB)XML block
 * @param tagId (IN)
 *        the ID for the tag to generate (TN_ADD, ...)
 * @param reqOptFlag (IN)
 *        flag if the block is required or optional
 * @param pContent (IN)
 *        the content structure of the block
 * @param enc (IN)
 *        the encoding constant (SML_WBXML or SML_XML)
 * @param attFlag (IN)
 *        indicates if the encoded tag contain Attributes
 *        in namespace extensions
 * @param pBufMgr (IN/OUT)
 *        pointer to a structure containing buffer management elements
 * @return shows error codes of function,\n
 *         0, if OK
 */
Ret_t xltEncBlock(XltTagID_t tagId, XltRO_t reqOptFlag, const VoidPtr_t pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr, SmlPcdataExtension_t attFlag)
{

  //Return variable
  Ret_t _err;


  //Check if pContent of a required field is missing
  if ((reqOptFlag == REQUIRED) && (pContent == NULL)) {
    switch ((int)tagId) {
      case TN_ATOMIC_END:
      case TN_SYNC_END:
      case TN_SEQUENCE_END:
        break;
      default:
        return SML_ERR_XLT_MISSING_CONT;
    }
  }
  //Check if pContent of a optional field is missing
  else if ((pContent == NULL) && (tagId != TN_SYNC_END) && (tagId != TN_ATOMIC_END) && (tagId != TN_SEQUENCE_END))
    return SML_ERR_OK;

  //Generate the commands -> see DTD
  switch ((int)tagId){
    case TN_SYNCHDR:
      // SyncHdr Begin Tag
      if ((_err = xltGenerateTag(TN_SYNCHDR, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Version
      if ((_err = xltEncBlock(TN_VERSION, REQUIRED, ((SmlSyncHdrPtr_t) pContent)->version, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Proto
      if ((_err = xltEncBlock(TN_PROTO, REQUIRED, ((SmlSyncHdrPtr_t) pContent)->proto, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // SessionID
      if ((_err = xltEncBlock(TN_SESSIONID, REQUIRED, ((SmlSyncHdrPtr_t) pContent)->sessionID, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // MsgID
      if ((_err = xltEncBlock(TN_MSGID, REQUIRED, ((SmlSyncHdrPtr_t) pContent)->msgID, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Target
      if ((_err = xltEncBlock(TN_TARGET, REQUIRED, ((SmlSyncHdrPtr_t) pContent)->target, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Source
      if ((_err = xltEncBlock(TN_SOURCE, REQUIRED, ((SmlSyncHdrPtr_t) pContent)->source, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // RespURI?
      if ((_err = xltEncBlock(TN_RESPURI, OPTIONAL, ((SmlSyncHdrPtr_t) pContent)->respURI, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // NoResp?
      if ((_err = xltEncBlock(TN_NORESP, OPTIONAL, &((SmlSyncHdrPtr_t) pContent)->flags, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Cred?
      if ((_err = xltEncBlock(TN_CRED, OPTIONAL, ((SmlSyncHdrPtr_t) pContent)->cred, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta?
      if ((_err = xltEncBlock(TN_META, OPTIONAL, ((SmlSyncHdrPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // SyncHdr End Tag
      if ((_err = xltGenerateTag(TN_SYNCHDR, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;

      break;
    case TN_CRED:
      // Begin tag
      if ((_err = xltGenerateTag(TN_CRED, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta?
      if ((_err = xltEncBlock(TN_META, OPTIONAL, ((SmlCredPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Data
      if ((_err = xltEncBlock(TN_DATA, REQUIRED, ((SmlCredPtr_t) pContent)->data, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(TN_CRED, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    case TN_SOURCEPARENT:
    case TN_TARGETPARENT:
      // Begin tag
      if ((_err = xltGenerateTag(tagId, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // LocURI
      if ((_err = xltEncBlock(TN_LOCURI, REQUIRED, ((SmlSourceParentPtr_t) pContent)->locURI, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(tagId, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    case TN_SOURCE:
    case TN_TARGET:
      // Begin tag
      if ((_err = xltGenerateTag(tagId, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // LocURI
      if ((_err = xltEncBlock(TN_LOCURI, REQUIRED, ((SmlSourcePtr_t) pContent)->locURI, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // LocName?
      if ((_err = xltEncBlock(TN_LOCNAME, OPTIONAL, ((SmlSourcePtr_t) pContent)->locName, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Filter?
      if ((_err = xltEncBlock(TN_FILTER, OPTIONAL, ((SmlSourcePtr_t) pContent)->filter, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(tagId, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    case TN_ITEM:
      // Begin tag
      if ((_err = xltGenerateTag(TN_ITEM, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Target?
      if ((_err = xltEncBlock(TN_TARGET, OPTIONAL, ((SmlItemPtr_t) pContent)->target, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Source?
      if ((_err = xltEncBlock(TN_SOURCE, OPTIONAL, ((SmlItemPtr_t) pContent)->source, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // TargetParent?
      if ((_err = xltEncBlock(TN_TARGETPARENT, OPTIONAL, ((SmlItemPtr_t) pContent)->targetParent, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // SourceParent?
      if ((_err = xltEncBlock(TN_SOURCEPARENT, OPTIONAL, ((SmlItemPtr_t) pContent)->sourceParent, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta?
      if ((_err = xltEncBlock(TN_META, OPTIONAL, ((SmlItemPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Data?
      if ((_err = xltEncBlock(TN_DATA, OPTIONAL, ((SmlItemPtr_t) pContent)->data, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // MoreData?
      if ((_err = xltEncBlock(TN_MOREDATA, OPTIONAL, &(((SmlItemPtr_t) pContent)->flags), enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(TN_ITEM, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    /* %%% Added Filter tags 2005-08-17 by synthesis/luz for DS 1.2 */
    case TN_FIELD:
    case TN_RECORD:
      // Begin tag
      if ((_err = xltGenerateTag(tagId, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Item
      if ((_err = xltEncBlock(TN_ITEM, REQUIRED, ((SmlRecordOrFieldFilterPtr_t) pContent)->item, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(tagId, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    case TN_FILTER:
      // Begin tag
      if ((_err = xltGenerateTag(tagId, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta
      if ((_err = xltEncBlock(TN_META, REQUIRED, ((SmlFilterPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Field?
      if ((_err = xltEncBlock(TN_FIELD, OPTIONAL, ((SmlFilterPtr_t) pContent)->field, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Record?
      if ((_err = xltEncBlock(TN_RECORD, OPTIONAL, ((SmlFilterPtr_t) pContent)->record, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // FilterType?
      if ((_err = xltEncBlock(TN_FILTERTYPE, OPTIONAL, ((SmlFilterPtr_t) pContent)->filtertype, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(tagId, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    case TN_REPLACE:
    case TN_ADD:
    case TN_COPY:
    case TN_MOVE:
      // Begin tag
      if ((_err = xltGenerateTag(tagId, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // CmdID
      if ((_err = xltEncBlock(TN_CMDID, REQUIRED, ((SmlGenericCmdPtr_t) pContent)->cmdID, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // NoResp?
      if ((_err = xltEncBlock(TN_NORESP, OPTIONAL, &((SmlGenericCmdPtr_t) pContent)->flags, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Cred?
      if ((_err = xltEncBlock(TN_CRED, OPTIONAL, ((SmlGenericCmdPtr_t) pContent)->cred, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta?
      if ((_err = xltEncBlock(TN_META, OPTIONAL, ((SmlGenericCmdPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Item+
      if ((_err = xltEncList(ITEM_LIST, REQUIRED, ((SmlGenericCmdPtr_t) pContent)->itemList, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(tagId, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    case TN_ALERT:
      // Begin tag
      if ((_err = xltGenerateTag(TN_ALERT, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // CmdID
      if ((_err = xltEncBlock(TN_CMDID, REQUIRED, ((SmlAlertPtr_t) pContent)->cmdID, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // NoResp?
      if ((_err = xltEncBlock(TN_NORESP, OPTIONAL, &((SmlAlertPtr_t) pContent)->flags, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Cred?
      if ((_err = xltEncBlock(TN_CRED, OPTIONAL, ((SmlAlertPtr_t) pContent)->cred, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Correlator?
      if ((_err = xltEncBlock(TN_CORRELATOR, OPTIONAL, ((SmlAlertPtr_t) pContent)->correlator, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Data?
      if ((_err = xltEncBlock(TN_DATA, OPTIONAL, ((SmlAlertPtr_t) pContent)->data, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Item*
      if ((_err = xltEncList(ITEM_LIST, OPTIONAL, ((SmlAlertPtr_t) pContent)->itemList, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(TN_ALERT, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

#if (defined ATOMIC_SEND || defined SEQUENCE_SEND)
    case TN_ATOMIC:
    case TN_SEQUENCE:
      // Begin tag
      if ((_err = xltGenerateTag(tagId, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // CmdID
      if ((_err = xltEncBlock(TN_CMDID, REQUIRED, ((SmlAtomicPtr_t) pContent)->cmdID, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // NoResp?
      if ((_err = xltEncBlock(TN_NORESP, OPTIONAL, &((SmlAtomicPtr_t) pContent)->flags, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta?
      if ((_err = xltEncBlock(TN_META, OPTIONAL, ((SmlAtomicPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      //End tag in TN_ATOMIC_END
      break;

    case TN_ATOMIC_END:
      // End tag
      if ((_err = xltGenerateTag(TN_ATOMIC, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    case TN_SEQUENCE_END:
      // End tag
      if ((_err = xltGenerateTag(TN_SEQUENCE, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;
#endif

    case TN_DELETE:
      // Begin tag
      if ((_err = xltGenerateTag(TN_DELETE, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // CmdID
      if ((_err = xltEncBlock(TN_CMDID, REQUIRED, ((SmlDeletePtr_t) pContent)->cmdID, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // NoResp?
      if ((_err = xltEncBlock(TN_NORESP, OPTIONAL, &((SmlDeletePtr_t) pContent)->flags, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Archive?
      if ((_err = xltEncBlock(TN_ARCHIVE, OPTIONAL, &(((SmlDeletePtr_t) pContent)->flags), enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // SftDel?
      if ((_err = xltEncBlock(TN_SFTDEL, OPTIONAL, &(((SmlDeletePtr_t) pContent)->flags), enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Cred?
      if ((_err = xltEncBlock(TN_CRED, OPTIONAL, ((SmlDeletePtr_t) pContent)->cred, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta?
      if ((_err = xltEncBlock(TN_META, OPTIONAL, ((SmlDeletePtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Item+
      if ((_err = xltEncList(ITEM_LIST, REQUIRED, ((SmlDeletePtr_t) pContent)->itemList, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(TN_DELETE, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

#ifdef EXEC_SEND
    case TN_EXEC:
      // Begin tag
      if ((_err = xltGenerateTag(TN_EXEC, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // CmdID
      if ((_err = xltEncBlock(TN_CMDID, REQUIRED, ((SmlExecPtr_t) pContent)->cmdID, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // NoResp?
      if ((_err = xltEncBlock(TN_NORESP, OPTIONAL, &((SmlExecPtr_t) pContent)->flags, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Cred?
      if ((_err = xltEncBlock(TN_CRED, OPTIONAL, ((SmlExecPtr_t) pContent)->cred, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta?
      if ((_err = xltEncBlock(TN_META, OPTIONAL, ((SmlExecPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Item
      if ((_err = xltEncBlock(ITEM_LIST, REQUIRED, ((SmlExecPtr_t) pContent)->itemList, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(TN_EXEC, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;
#endif

    case TN_GET:
    case TN_PUT:
      // Begin tag
      if ((_err = xltGenerateTag(tagId, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // CmdID
      if ((_err = xltEncBlock(TN_CMDID, REQUIRED, ((SmlGetPtr_t) pContent)->cmdID, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // NoResp?
      if ((_err = xltEncBlock(TN_NORESP, OPTIONAL, &((SmlGetPtr_t) pContent)->flags, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Lang?
      if ((_err = xltEncBlock(TN_LANG, OPTIONAL, ((SmlGetPtr_t) pContent)->lang, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Cred?
      if ((_err = xltEncBlock(TN_CRED, OPTIONAL, ((SmlGetPtr_t) pContent)->cred, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta?
      if ((_err = xltEncBlock(TN_META, OPTIONAL, ((SmlGetPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Item+
      if ((_err = xltEncList(ITEM_LIST, REQUIRED, ((SmlGetPtr_t) pContent)->itemList, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(tagId, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    case TN_MAP:
      // Begin tag
      if ((_err = xltGenerateTag(TN_MAP, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // CmdID
      if ((_err = xltEncBlock(TN_CMDID, REQUIRED, ((SmlMapPtr_t) pContent)->cmdID, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Target
      if ((_err = xltEncBlock(TN_TARGET, REQUIRED, ((SmlMapPtr_t) pContent)->target, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Source
      if ((_err = xltEncBlock(TN_SOURCE, REQUIRED, ((SmlMapPtr_t) pContent)->source, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Cred?
      if ((_err = xltEncBlock(TN_CRED, OPTIONAL, ((SmlMapPtr_t) pContent)->cred, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta?
      if ((_err = xltEncBlock(TN_META, OPTIONAL, ((SmlMapPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Mapitemlist
      if ((_err = xltEncList(MAPITEM_LIST, REQUIRED, ((SmlMapPtr_t) pContent)->mapItemList, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(TN_MAP, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    case TN_MAPITEM:
      // Begin tag
      if ((_err = xltGenerateTag(TN_MAPITEM, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Target
      if ((_err = xltEncBlock(TN_TARGET, REQUIRED, ((SmlMapItemPtr_t) pContent)->target, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Source
      if ((_err = xltEncBlock(TN_SOURCE, REQUIRED, ((SmlMapItemPtr_t) pContent)->source, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(TN_MAPITEM, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    case TN_RESULTS:
      // Begin tag
      if ((_err = xltGenerateTag(TN_RESULTS, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // CmdID
      if ((_err = xltEncBlock(TN_CMDID, REQUIRED, ((SmlResultsPtr_t) pContent)->cmdID, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // MsgRef?
      if ((_err = xltEncBlock(TN_MSGREF, OPTIONAL, ((SmlResultsPtr_t) pContent)->msgRef, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // CmdRef
      if ((_err = xltEncBlock(TN_CMDREF, REQUIRED, ((SmlResultsPtr_t) pContent)->cmdRef, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta?
      if ((_err = xltEncBlock(TN_META, OPTIONAL, ((SmlResultsPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // TargetRef?
      if ((_err = xltEncBlock(TN_TARGETREF, OPTIONAL, ((SmlResultsPtr_t) pContent)->targetRef, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // SourceRef?
      if ((_err = xltEncBlock(TN_SOURCEREF, OPTIONAL, ((SmlResultsPtr_t) pContent)->sourceRef, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Item+
      if ((_err = xltEncList(ITEM_LIST, REQUIRED, ((SmlResultsPtr_t) pContent)->itemList, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(TN_RESULTS, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    case TN_CHAL:
      // Begin tag
      if ((_err = xltGenerateTag(TN_CHAL, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta
      if ((_err = xltEncBlock(TN_META, REQUIRED, ((SmlChalPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(TN_CHAL, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

#ifdef SEARCH_SEND
    case TN_SEARCH:
      // Begin tag
      if ((_err = xltGenerateTag(TN_SEARCH, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // CmdID
      if ((_err = xltEncBlock(TN_CMDID, REQUIRED, ((SmlSearchPtr_t) pContent)->cmdID, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // NoResp?
      if ((_err = xltEncBlock(TN_NORESP, OPTIONAL, &((SmlSearchPtr_t) pContent)->flags, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // NoResults?
      if ((_err = xltEncBlock(TN_NORESULTS, OPTIONAL, &((SmlSearchPtr_t) pContent)->flags, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Cred?
      if ((_err = xltEncBlock(TN_CRED, OPTIONAL, ((SmlSearchPtr_t) pContent)->cred, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Target?
      if ((_err = xltEncBlock(TN_TARGET, OPTIONAL, ((SmlSearchPtr_t) pContent)->target, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Source List
      if ((_err = xltEncList(SOURCE_LIST, REQUIRED, ((SmlSearchPtr_t) pContent)->sourceList, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Lang?
      if ((_err = xltEncBlock(TN_LANG, OPTIONAL, ((SmlSearchPtr_t) pContent)->lang, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta
      if ((_err = xltEncBlock(TN_META, REQUIRED, ((SmlSearchPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Dsta
      if ((_err = xltEncBlock(TN_DATA, REQUIRED, ((SmlSearchPtr_t) pContent)->data, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(TN_SEARCH, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;
#endif

    case TN_STATUS:
      // Begin tag
      if ((_err = xltGenerateTag(TN_STATUS, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // CmdID
      if ((_err = xltEncBlock(TN_CMDID, REQUIRED, ((SmlStatusPtr_t) pContent)->cmdID, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // MsgRef?
      if ((_err = xltEncBlock(TN_MSGREF, REQUIRED, ((SmlStatusPtr_t) pContent)->msgRef, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // CmdRef
      if ((_err = xltEncBlock(TN_CMDREF, REQUIRED, ((SmlStatusPtr_t) pContent)->cmdRef, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Cmd
      if ((_err = xltEncBlock(TN_CMD, REQUIRED, ((SmlStatusPtr_t) pContent)->cmd, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // TargetRefList?
      if ((_err = xltEncList(TARGETREF_LIST, OPTIONAL, ((SmlStatusPtr_t) pContent)->targetRefList, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // SourceRefList?
      if ((_err = xltEncList(SOURCEREF_LIST, OPTIONAL, ((SmlStatusPtr_t) pContent)->sourceRefList, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Cred?
      if ((_err = xltEncBlock(TN_CRED, OPTIONAL, ((SmlStatusPtr_t) pContent)->cred, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Chal?
      if ((_err = xltEncBlock(TN_CHAL, OPTIONAL, ((SmlStatusPtr_t) pContent)->chal, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Data
      if ((_err = xltEncBlock(TN_DATA, REQUIRED, ((SmlStatusPtr_t) pContent)->data, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Item*
      if ((_err = xltEncList(ITEM_LIST, OPTIONAL, ((SmlStatusPtr_t) pContent)->itemList, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag
      if ((_err = xltGenerateTag(TN_STATUS, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    case TN_SYNC:
      // Begin tag
      if ((_err = xltGenerateTag(TN_SYNC, TT_BEG, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // CmdID
      if ((_err = xltEncBlock(TN_CMDID, REQUIRED, ((SmlSyncPtr_t) pContent)->cmdID, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // NoResp?
      if ((_err = xltEncBlock(TN_NORESP, OPTIONAL, &((SmlSyncPtr_t) pContent)->flags, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Cred?
      if ((_err = xltEncBlock(TN_CRED, OPTIONAL, ((SmlSyncPtr_t) pContent)->cred, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Target?
      if ((_err = xltEncBlock(TN_TARGET, OPTIONAL, ((SmlSyncPtr_t) pContent)->target, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Source?
      if ((_err = xltEncBlock(TN_SOURCE, OPTIONAL, ((SmlSyncPtr_t) pContent)->source, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // Meta?
      if ((_err = xltEncBlock(TN_META, OPTIONAL, ((SmlSyncPtr_t) pContent)->meta, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // NumberOfChanges?
      if ((_err = xltEncBlock(TN_NUMBEROFCHANGES, OPTIONAL, ((SmlSyncPtr_t) pContent)->noc, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      // End tag in TN_SYNC_END
      break;

    case TN_SYNC_END:
      //End tag
      if ((_err = xltGenerateTag(TN_SYNC, TT_END, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      break;

    case TN_ARCHIVE:
      //set the flag in the (WB)XML document if the flag is in the pContent
      if ((*((Flag_t*)pContent)) & (SmlArchive_f)) {
        if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      }
      break;

    case TN_SFTDEL:
      //set the flag in the (WB)XML document if the flag is in the pContent
      if ((*((Flag_t *) pContent)) & (SmlSftDel_f)) {
        if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      }
      break;

    case TN_MOREDATA:
      //set the flag in the (WB)XML document if the flag is in the pContent
      if ((*((Flag_t *) pContent)) & (SmlMoreData_f)) {
        if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      }
      break;

    case TN_NORESULTS:
      //set the flag in the (WB)XML document if the flag is in the pContent
      if ((*((Flag_t *) pContent)) & (SmlNoResults_f)) {
        if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      }
      break;

    case TN_NORESP:
      //set the flag in the (WB)XML document if the flag is in the pContent
      if ((*((Flag_t *) pContent)) & (SmlNoResp_f)){
        if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      }
      break;

    case TN_FINAL:
      //set the flag in the (WB)XML document if the flag is in the pContent
      if ((*((Flag_t *) pContent)) & (SmlFinal_f)) {
        if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_UNDEFINED)) != SML_ERR_OK) return _err;
      }
      break;

    default:
    	// all leaf nodes (PCDATA#)
      return xltEncPcdata(tagId, reqOptFlag, pContent, enc, pBufMgr, attFlag);
        
    }
    return SML_ERR_OK;
}

Ret_t xltEncPcdata(XltTagID_t tagId, XltRO_t reqOptFlag, const VoidPtr_t pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr, SmlPcdataExtension_t attFlag)   {
    //Return variable
    Ret_t _err;

    //generate PCDATA begin tag
    if ((_err = xltGenerateTag(tagId, TT_BEG, enc, pBufMgr, attFlag)) != SML_ERR_OK) return _err;

    //write the pContent to the buffer according the encoding type
    switch ((int)enc) {
#ifdef __SML_WBXML__
      case SML_WBXML: {
        switch (((SmlPcdataPtr_t)pContent)->contentType) {
        case SML_PCDATA_STRING:
            if ((_err = wbxmlWriteTypeToBuffer(((SmlPcdataPtr_t)pContent)->content, STR_I, ((SmlPcdataPtr_t)pContent)->length, pBufMgr)) != SML_ERR_OK) return _err;
            break;
        // Note: SML_PCDATA_CDATA case added by luz to allow direct translation from XML to WBXML
        case SML_PCDATA_CDATA:
        case SML_PCDATA_OPAQUE:
            if ((_err = wbxmlWriteTypeToBuffer(((SmlPcdataPtr_t)pContent)->content, OPAQUE, ((SmlPcdataPtr_t)pContent)->length, pBufMgr)) != SML_ERR_OK) return _err;
            break;
#ifdef __USE_EXTENSIONS__
        case SML_PCDATA_EXTENSION:
            if ((_err = xltBuildExtention(((SmlPcdataPtr_t)pContent)->extension, reqOptFlag, ((SmlPcdataPtr_t)pContent)->content, enc, pBufMgr)) != SML_ERR_OK) return _err;
            break;
#endif
        default:
            // 2003-11-24: Tomy to deal with pcdata empty extensions (for example <Meta></Meta> which is valid)
            // refer to xltdec.c to see that empty extensions result in SmlPcdataPtr_t with all fields (data) set to 0
            if (((SmlPcdataPtr_t)pContent)->contentType != SML_PCDATA_UNDEFINED ||
              ((SmlPcdataPtr_t)pContent)->extension != SML_EXT_UNDEFINED ||
              ((SmlPcdataPtr_t)pContent)->length != 0 ||
              ((SmlPcdataPtr_t)pContent)->content != NULL)
              return SML_ERR_XLT_INVAL_PCDATA_TYPE;
            //      return SML_ERR_XLT_INVAL_PCDATA_TYPE;
            // end modified by Tomy
        }; // eof switch(contenttype)
        break;
      } // case WBXML
#endif  // eof WBXML

#ifdef __SML_XML__
      case SML_XML: {
        MemPtr_t _tmpStr;
        MemPtr_t p;
        MemSize_t len,n;
        char c;
        switch (((SmlPcdataPtr_t)pContent)->contentType) {
          // Note: SML_PCDATA_OPAQUE case added by luz to allow direct translation from WBXML to XML
          #ifdef PCDATA_OPAQUE_AS_CDATA
          case SML_PCDATA_OPAQUE:
          #endif
          case SML_PCDATA_CDATA:
          stringAsCData:
          {
              // %%% luz 2006-09-07 : made completely CDATA nesting safe
              // add CDATA
              _tmpStr = (MemPtr_t) "<![CDATA[";
              if ((_err = xltAddToBuffer(_tmpStr, smlLibStrlen((String_t)_tmpStr), pBufMgr)) != SML_ERR_OK) return _err;
              // search and escape possibly contained ]]> sequences
              p=((SmlPcdataPtr_t)pContent)->content;
              len=((SmlPcdataPtr_t)pContent)->length;
              n=0;
              while (n<len-2) {
                if (p[n]==']' && p[n+1]==']' && p[n+2]=='>') {
                  // we must substitute "]]>" with "]]>]<![CDATA[]>"
                  // - copy what we have so far (includes ]]>)
                  n+=3;
                  if ((_err = xltAddToBuffer(p, n, pBufMgr)) != SML_ERR_OK) return _err;
                  // - add extra code needed
                  _tmpStr = (MemPtr_t) "]<![CDATA[]>";
                  if ((_err = xltAddToBuffer(_tmpStr, smlLibStrlen((String_t)_tmpStr), pBufMgr)) != SML_ERR_OK) return _err;
                  // - restart where we are now
                  p+=n; // first char after ]]>
                  len-=n; // remaining length
                  n=0; // processed chars
                }
                else {
                  n++; // just go to next char
                }
              }
              // add remaining data
              if ((_err = xltAddToBuffer(p, len, pBufMgr)) != SML_ERR_OK) return _err;
              // add end of CDATA marker
              _tmpStr = (MemPtr_t) "]]>";
              if ((_err = xltAddToBuffer(_tmpStr, smlLibStrlen((String_t)_tmpStr), pBufMgr)) != SML_ERR_OK) return _err;
              break;
          }
          // Note: SyncFest #5 shows that <![CDATA[ is not correctly parsed by the RTK
          //       so we don't use it and risk the danger of failing on payload which has
          //       XML in it.
          #ifndef PCDATA_OPAQUE_AS_CDATA
          case SML_PCDATA_OPAQUE:
          #endif
          case SML_PCDATA_STRING: {
              // %%% luz 2006-09-07 : check for invalid characters that would break XML and use CDATA instead
              p=((SmlPcdataPtr_t)pContent)->content;
              len=((SmlPcdataPtr_t)pContent)->length;
              while (len-->0) {
                c=*p++;
                if (c=='&' || c=='<' || c=='>')
                  goto stringAsCData; // encode as CDATA
              }
              // ok, string is clean and does not need CDATA
              if ((_err = xltAddToBuffer(((SmlPcdataPtr_t)pContent)->content, ((SmlPcdataPtr_t)pContent)->length, pBufMgr)) != SML_ERR_OK) return _err;
              break;
          }
#ifdef __USE_EXTENSIONS__
          case SML_PCDATA_EXTENSION: {
              if ((_err = xltBuildExtention(((SmlPcdataPtr_t)pContent)->extension, reqOptFlag, ((SmlPcdataPtr_t)pContent)->content, enc, pBufMgr)) != SML_ERR_OK) return _err;
              break;
          }
#endif
          default: {
              return SML_ERR_XLT_INVAL_PCDATA_TYPE;
              // refer to xltdec.c to see that empty extensions result in SmlPcdataPtr_t with all fields (data) set to 0
              if (((SmlPcdataPtr_t)pContent)->contentType != SML_PCDATA_UNDEFINED ||
                ((SmlPcdataPtr_t)pContent)->extension != SML_EXT_UNDEFINED ||
                ((SmlPcdataPtr_t)pContent)->length != 0 ||
                ((SmlPcdataPtr_t)pContent)->content != NULL)
                return SML_ERR_XLT_INVAL_PCDATA_TYPE;
              //      return SML_ERR_XLT_INVAL_PCDATA_TYPE;
              // end modified by Tomy
          }
        } // switch tags
        break;
      } // case XML
#endif // eof XML

      default:
        return SML_ERR_XLT_ENC_UNK;
    } // eof switch(enc)

    //generate PCDATA END tag
    if ((_err = xltGenerateTag(tagId, TT_END, enc, pBufMgr, attFlag)) != SML_ERR_OK) return _err;
    return SML_ERR_OK;
}

/**
 * Generates a list element which is not directly related to a tag
 *
 * @pre pList holds a valid list structure
 *      listId contains a valid SyncML list ID
 * @post the (WB)XML buffer in the pBufMgr structure contains the
 *       encoded (WB)XML list
 * @param listId (IN)
 *        the ID of the list to generate (e.g. TARGET_LIST, ...)
 * @param reqOptFlag (IN)
 *        flag if the block is required or optional
 * @param pList (IN)
 *        reference to the list to process
 * @param enc (IN)
 *        the encoding constant (SML_WBXML or SML_XML)
 * @param pBufMgr (IN/OUT)
 *        pointer to a structure containing buffer management elements
 * @param attFlag (IN)
 *        indicates if the encoded tag contain Attributes
 *        in namespace extensions
 * @return shows error codes of function,\n
 *         0, if OK
 */
Ret_t xltEncList(XltListType_t listId, XltRO_t reqOptFlag, VoidPtr_t pList, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr, SmlPcdataExtension_t attFlag)
{
  //Return variable
  Ret_t _err;

  //check if list is required or not
  if ((reqOptFlag == REQUIRED) && (pList == NULL))
    return SML_ERR_XLT_MISSING_CONT;
  else if (pList == NULL)
    return SML_ERR_OK;

  //encode the different list types
  switch ((int)listId)
  {
    case ITEM_LIST:
    {
      do
      {
        if ((_err = xltEncBlock(TN_ITEM, OPTIONAL, ((SmlItemListPtr_t)pList)->item, enc, pBufMgr, attFlag)) != SML_ERR_OK) return _err;
        pList = ((SmlItemListPtr_t)pList)->next;
      } while ((SmlItemListPtr_t)pList != NULL);

      break;
    }
    case SOURCE_LIST:
    {
      do
      {
        if ((_err = xltEncBlock(TN_SOURCE, OPTIONAL, ((SmlSourceListPtr_t)pList)->source, enc, pBufMgr, attFlag)) != SML_ERR_OK) return _err;
        pList = ((SmlSourceListPtr_t)pList)->next;
      } while ((SmlSourceListPtr_t)pList != NULL);

      break;
    }
    case TARGETREF_LIST:
    {
      do
      {
        if ((_err = xltEncBlock(TN_TARGETREF, OPTIONAL, ((SmlTargetRefListPtr_t)pList)->targetRef, enc, pBufMgr, attFlag)) != SML_ERR_OK) return _err;
        pList = ((SmlTargetRefListPtr_t)pList)->next;
      } while ((SmlTargetRefListPtr_t)pList != NULL);

      break;
    }
    case SOURCEREF_LIST:
    {
      do
      {
        if ((_err = xltEncBlock(TN_SOURCEREF, OPTIONAL, ((SmlSourceRefListPtr_t)pList)->sourceRef, enc, pBufMgr, attFlag)) != SML_ERR_OK) return _err;
        pList = ((SmlSourceRefListPtr_t)pList)->next;
      } while ((SmlSourceRefListPtr_t)pList != NULL);

      break;
    }
    case MAPITEM_LIST:
    {
      do
      {
        if ((_err = xltEncBlock(TN_MAPITEM, OPTIONAL, ((SmlMapItemListPtr_t)pList)->mapItem, enc, pBufMgr, attFlag)) != SML_ERR_OK) return _err;
        pList = ((SmlMapItemListPtr_t)pList)->next;
      } while ((SmlMapItemListPtr_t)pList != NULL);

      break;
    }
    default:
    return SML_ERR_XLT_INVAL_LIST_TYPE;
    }

  return SML_ERR_OK;
}

/**
 * Generates a (WB)XML tag
 *
 * @pre valid parameters
 * @post the buffer contains a new tag
 * @param tagId (IN)
 *        the tag ID
 * @param TagType (IN)
 *        the tag type (begin tag, end tag, ...)
 * @param enc (IN)
 *        the encoding constant (SML_WBXML or SML_XML)
 * @param attFlag (IN)
 *        indicates if the encoded tag contain Attributes
 *        in namespace extensions
 * @param pBufMgr (IN/OUT)
 *        pointer to a structure containing buffer management elements
 * @return shows error codes of function,\n
 *         0, if OK
 */
Ret_t xltGenerateTag(XltTagID_t tagId, XltTagType_t TagType, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr, SmlPcdataExtension_t attFlag)
{

  Ret_t _err;
#ifdef __SML_WBXML__
  MemByte_t _switchpage = XLT_SWITCHPAGE;
#endif

  switch ((int)enc) {
#ifdef __SML_WBXML__
    case SML_WBXML:
        /* in WBXML codepage switches are done for starting tags only */
        if (TagType != TT_END) {
            //codepage switching with wbxml instead of namespace
            if (getCodePage(attFlag) != getCodePage(pBufMgr->smlCurExt)) {
                MemByte_t _newcp      = getCodePage(attFlag);
                if ((_err = wbxmlWriteTypeToBuffer((MemPtr_t)(&_switchpage), TAG, 1, pBufMgr)) != SML_ERR_OK) return _err;
                if ((_err = wbxmlWriteTypeToBuffer((MemPtr_t)(&_newcp), TAG, 1, pBufMgr)) != SML_ERR_OK) return _err;
            }

      if (attFlag != pBufMgr->smlCurExt) {
                pBufMgr->switchExtTag = tagId;
                pBufMgr->smlLastExt = pBufMgr->smlCurExt;
                pBufMgr->smlCurExt = attFlag;
            }
    } // for TagType
      return wbxmlGenerateTag(tagId, TagType, pBufMgr);
#endif
#ifdef __SML_XML__
    case SML_XML:

        if (attFlag != pBufMgr->smlCurExt) {
            pBufMgr->switchExtTag = tagId;
            pBufMgr->smlLastExt = pBufMgr->smlCurExt;
            pBufMgr->smlCurExt = attFlag;
        }
        return xmlGenerateTag(tagId, TagType, pBufMgr, attFlag);
#endif
    default:
        return SML_ERR_XLT_ENC_UNK;
  }

  //return SML_ERR_XLT_ENC_UNK;NOT NEEDED
}

#ifdef __USE_EXTENSIONS__
/* Entrypoint for SubDTD's. If we reached this point we already know
 * a) we have data fora sub-DTD to encode and
 * b) we know which sub-DTD should be encoded.
 * So just call the appropriate sub-DTD encoder and thats it.
 */
Ret_t xltBuildExtention(SmlPcdataExtension_t extId, XltRO_t reqOptFlag, VoidPtr_t pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr) {

  switch (extId) {
#ifdef __USE_METINF__
  case SML_EXT_METINF:
    /* a metaInf DTD always starts with this token */
    return metinfEncBlock(TN_METINF_METINF,reqOptFlag,pContent,enc,pBufMgr,SML_EXT_METINF);
    break;
#endif
#ifdef __USE_DEVINF__
  case SML_EXT_DEVINF:
    /* a deviceInf DTD always starts with this token */
        /* we have to choose, wether we have to encode the DevInf as XML or WBXML */
        /* in the latter case, we need a special treatment of this sub-dtd, as we have */
        /* to put it into a SML_PCDATA_OPAQUE field ... */
        if (enc == SML_XML)
        return devinfEncBlock(TN_DEVINF_DEVINF,reqOptFlag,pContent,enc,pBufMgr,SML_EXT_DEVINF);
        else
            return subdtdEncWBXML(TN_DEVINF_DEVINF,reqOptFlag,pContent,SML_WBXML,pBufMgr,SML_EXT_DEVINF);
    break;
#endif
  /* oops - we don not know about that extension -> bail out */
  default:
    return SML_ERR_XLT_INVAL_EXT;
  }
  //return SML_ERR_OK;CAN NOT BE REACHED
}


/* Sub DTD's need a special treatment when used together with WBXML.
 * We need to eoncode them as a complete WBXML message including headers and stuff
 * and store the result within an SML_PCDATA_OPAQUE datafield.
 * To archieve this we create a new encoder, encode the message and finally
 * copy the result into the allready existing encoder.
 */
#ifdef __SML_WBXML__
Ret_t subdtdEncWBXML(XltTagID_t tagId, XltRO_t reqOptFlag, const VoidPtr_t pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr, SmlPcdataExtension_t attFlag)
{
    #ifdef __USE_DEVINF__
    Ret_t             _err         = SML_ERR_OK;
    #endif

    /* Double the size of SubBufSize for the memory is small for some complex
     * content.
     */ 
    MemSize_t         SubBufSize   = 12000 * 2; // for starters we use 12kB for each sub DTD to encode in WBXML
    BufferMgmtPtr_t   pSubBufMgr   = NULL;

    /* Even the doubled size was still too small. Instead of
       hard-coding the size, make it as large as the buffer we are
       copying into. The size of that one can be configured by the
       user of the toolkit. -- Patrick Ohly */
    MemSize_t VarSubBufSize = pBufMgr->smlXltBufferLen - pBufMgr->smlXltWrittenBytes;
    if (VarSubBufSize > SubBufSize) {
      SubBufSize = VarSubBufSize;
    }

    // first create a sub buffer
    pSubBufMgr = (BufferMgmtPtr_t)smlLibMalloc(sizeof(BufferMgmt_t));
    if (pSubBufMgr == NULL) {
      if (enc && pContent && reqOptFlag && tagId) {
      }
      return SML_ERR_NOT_ENOUGH_SPACE;
    }

    smlLibMemset(pSubBufMgr, 0,sizeof(BufferMgmt_t));
    pSubBufMgr->smlXltBufferLen     = SubBufSize;
    pSubBufMgr->smlXltBufferP       = (MemPtr_t)smlLibMalloc(SubBufSize);
    if (pSubBufMgr->smlXltBufferP == NULL) {
      smlLibFree(pSubBufMgr);
      return SML_ERR_NOT_ENOUGH_SPACE;
    }

    smlLibMemset(pSubBufMgr->smlXltBufferP, 0, SubBufSize);
    pSubBufMgr->smlXltStoreBufP     = pSubBufMgr->smlXltBufferP;
    pSubBufMgr->smlXltWrittenBytes  = 0;
    pSubBufMgr->smlActiveExt        = pBufMgr->smlActiveExt;
    pSubBufMgr->smlCurExt           = pBufMgr->smlCurExt;
    pSubBufMgr->smlLastExt          = pBufMgr->smlLastExt;
    pSubBufMgr->spaceEvaluation     = pBufMgr->spaceEvaluation;
    pSubBufMgr->vers                = pBufMgr->vers;

    // check if we want to use numeric or textual FPI
    if (SyncMLDevInfWBXMLPublicID[pBufMgr->vers]==0) {
      // textual FPI
      // - get the FPI string
      const char *FPIstring = SyncMLDevInfFPI[pBufMgr->vers];
      Short_t FPIsize = smlLibStrlen(FPIstring);
      if (pSubBufMgr->spaceEvaluation == 0) {
        // - create the WBXML header
        pSubBufMgr->smlXltBufferP[0]    = 0x02; // WBXML Version 1.2
        pSubBufMgr->smlXltBufferP[1]    = 0x00; // use Stringtable for ID
        pSubBufMgr->smlXltBufferP[2]    = 0x00; // empty/unknown public ID
        pSubBufMgr->smlXltBufferP[3]    = 0x6A; // charset encoding UTF-8
        pSubBufMgr->smlXltBufferP[4]    = FPIsize; // %%% not fixed=0x1D!! lenght of stringtable = length of FPIsize
        pSubBufMgr->smlXltBufferP      += 5;
        // - Generate textual FPI
        smlLibMemmove(pSubBufMgr->smlXltBufferP, FPIstring, FPIsize);
        pSubBufMgr->smlXltBufferP      += FPIsize;
      }
      // in case of space evaluation, just count the number of written bytes
      pSubBufMgr->smlXltWrittenBytes  = 5 + FPIsize;
    }
    else {
      // numeric FPI, no string table
      if (pSubBufMgr->spaceEvaluation == 0) {
        // - create the WBXML header
        unsigned short fpi=SyncMLDevInfWBXMLPublicID[pBufMgr->vers];
        pSubBufMgr->smlXltBufferP[0]    = 0x02; // WBXML Version 1.2
        pSubBufMgr->smlXltBufferP[1]    = ((fpi>>7) & 0x7F)+0x80; // upper 7 bit
        pSubBufMgr->smlXltBufferP[2]    = (fpi & 0x7F); // lower 7 bits
        pSubBufMgr->smlXltBufferP[3]    = 0x6A; // charset encoding UTF-8
        pSubBufMgr->smlXltBufferP[4]    = 0; // no string table
        pSubBufMgr->smlXltBufferP      += 5;
      }
      // in case of space evaluation, just count the number of written bytes
      pSubBufMgr->smlXltWrittenBytes  = 5;
    }

    // do the encoding
    switch (attFlag) {
      #ifdef __USE_DEVINF__
      case SML_EXT_DEVINF:
        if ((_err = devinfEncBlock(TN_DEVINF_DEVINF,reqOptFlag,pContent,enc,pSubBufMgr,SML_EXT_DEVINF)) != SML_ERR_OK) {
            smlLibFree(pSubBufMgr->smlXltStoreBufP);
            smlLibFree(pSubBufMgr);
            return _err;
        }
        break;
      #endif
      /* oops - we don not know about that extension -> bail out */
      default:
        smlLibFree(pSubBufMgr->smlXltStoreBufP);
        smlLibFree(pSubBufMgr);
        return SML_ERR_XLT_INVAL_EXT;
    }

    #ifdef __USE_DEVINF__
    // move it to the 'real' encoder buffer
    // now set up the OPAQUE field
    if (pBufMgr->spaceEvaluation == 0) {
      pBufMgr->smlXltBufferP[0]     = 0xC3; // OPAQUE data identifier
      pBufMgr->smlXltBufferP       += 1;

      wbxmlOpaqueSize2Buf(pSubBufMgr->smlXltWrittenBytes, pBufMgr);

      smlLibMemmove(pBufMgr->smlXltBufferP, pSubBufMgr->smlXltStoreBufP, pSubBufMgr->smlXltWrittenBytes);
      pBufMgr->smlXltBufferP      += pSubBufMgr->smlXltWrittenBytes;
      pBufMgr->smlXltWrittenBytes += pSubBufMgr->smlXltWrittenBytes;
    } else {
      pBufMgr->smlXltWrittenBytes++;
      wbxmlOpaqueSize2Buf(pSubBufMgr->smlXltWrittenBytes, pBufMgr);
      pBufMgr->smlXltWrittenBytes += pSubBufMgr->smlXltWrittenBytes;
    }

    // clean up the temporary stuff
    smlLibFree(pSubBufMgr->smlXltStoreBufP);
    smlLibFree(pSubBufMgr);

    return _err;
#endif
}
#endif
#endif
