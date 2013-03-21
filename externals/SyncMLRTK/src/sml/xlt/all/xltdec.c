/**
 * @file
 * SyncmML Decoder
 *
 * @target_system  all
 * @target_os      all
 * @description The SyncML parser.
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
/* Definitions                                                           */
/*************************************************************************/
#include "syncml_tk_prefix_file.h" // %%% luz: needed for precompiled headers in eVC++

#include "xltdec.h"
#include "xltdeccom.h"
#include "xlttags.h"
#include "xltutilstack.h"
#include "xlttagtbl.h"
#include "xltmetinf.h"
#include "xltdevinf.h"

#include <smldef.h>
#include <smldtd.h>
#include <smlmetinfdtd.h>
#include <smldevinfdtd.h>
#include <smlerr.h>

#include <libmem.h>
#include <libstr.h>
#include <mgrutil.h>


#ifdef SYDEBUG

#if !defined(DIRECT_APPBASE_GLOBALACCESS) && defined(CONSOLEINFO)
	void ConsolePrintf(const char *text, ...);
	#define SMLERRPRINTFX(lvl,m) CONSOLEPRINTF(m)
#else
	#define SMLERRPRINTFX(lvl,m) PNCDEBUGPRINTFX(lvl,m)
#endif


/**
 * Better parsing error reporting when used within Synthesis SyncML engine
 * added 2006-11-02 by luz@synthesis.ch
 */

// Error texts
#define numSmlErrTexts (SML_ERR_XLT_INVAL_INPUT_DATA-SML_ERR_XLT_MISSING_CONT + 1)
static const char * const smlErrTexts[numSmlErrTexts] = {
  "required field content missing", // 0x2001
  "Buffer too small", // 0x2002
  "Invalid (WBXML) Element Type", // 0x2003
  "Invalid List Type", // 0x2004
  "Invalid Tag Type", // 0x2005
  "???", // 0x2006
  "Unknown Encoding", // 0x2007
  "Invalid Protocol Element", // 0x2008
  "Missing Content of List Elements", // 0x2009
  "Incompatible WBXML Content Format Version", // 0x200A
  "Document does not conform to SyncML DTD", // 0x200B
  "Invalid PCData elem", // 0x200C
  "Unspecified tokenizer error", // 0x200D
  "Document does not conform to WBXML specification", // 0x200E
  "Document contains unknown WBXML token", // 0x200F
  "Non-empty start tag without matching end tag", // 0x2010
  "WBXML document uses unspecified code page", // 0x2011
  "End of buffer reached", // 0x2012
  "Document does not conform to XML 1.0 specification", // 0x2012
  "Document contains unknown XML tag", // 0x2014
  "Invalid Public Identifier", // 0x2015
  "Invalid Codepage Extension", // 0x2016
  "No matching Codepage could be found", // 0x2017
  "Data missing in input structure", // 0x2018
};

// get error text
const char *smlErrorText(Ret_t aRc)
{
  if (aRc>=SML_ERR_XLT_MISSING_CONT && aRc<=SML_ERR_XLT_INVAL_INPUT_DATA) {
    return smlErrTexts[aRc-SML_ERR_XLT_MISSING_CONT];
  }
  // no text found
  return "<no error message found>";
}


// makes hex char out of nibble
static char NibbleToHexDigit(uInt8 aNibble)
{
  aNibble &= 0x0F;
  if (aNibble>9) return 'A'-0x0A+aNibble;
  else return '0'+aNibble;
} // NibbleToHexDigit

#define numBytesShow 32


// show token info
void show_token(XltDecTokenPtr_t aToken, uInt32 aDebugMask)
{
  int i;
  SmlPcdataExtension_t ext;
  TagPtr_t pTags;
  short tagwbxml;
  char *tagxml;
  char *extname;
  char *toktype;

  if (aToken==NULL) return;

  // get token type
  switch (aToken->type) {
    case TOK_TAG_START:
      toktype="start";
      break;
    case TOK_TAG_END:
      toktype="end";
      break;
    case TOK_TAG_EMPTY:
      toktype="empty";
      break;
    case TOK_CONT:
      toktype="content";
      break;
    default:
      toktype="UNKNOWN";
      break;
  }
  // get extension name
  switch (aToken->ext) {
    case SML_EXT_METINF:
      extname="metInf";
      break;
    case SML_EXT_DEVINF:
      extname="devInf";
      break;
    default:
      extname="undef/SyncML";
      break;
  }
  // check if this is a tag
  if (IS_TAG(aToken)) {
    // get tag information
    tagwbxml=-1; // undefined
    tagxml="UNDEFINED";
    for (ext=SML_EXT_UNDEFINED; ext<SML_EXT_LAST; ext++) {
      i=0;
      pTags=getTagTable(ext);
      if (pTags!=NULL) {
        while (((pTags+i)->id) != TN_UNDEF) {
          if (((pTags+i)->id) == aToken->tagid) {
            tagwbxml = (pTags+i)->wbxml;
            #if defined(__SML_XML__) || defined(__SML_WBXML_TEXTTOKENS__)
            tagxml = (pTags+i)->xml;
            #endif
            goto foundtag;
          }
          i++;
        }
      }
    }
  foundtag:
    #if defined(__SML_XML__) || defined(__SML_WBXML_TEXTTOKENS__)
    // we have tag names
    SMLERRPRINTFX(aDebugMask,(
      "- parsing %s token, ext=%s: <%s> (WBXML 0x%hX, tagid=%hd)",
      toktype,
      extname,
      tagxml,
      (short)tagwbxml,
      (short)aToken->tagid
    ));
    #else
    // we only have WBXML tag codes
    SMLERRPRINTFX(aDebugMask,(
      "- parsing  %s token, ext=%s: tag (WBXML 0x%hX, tagid=%hd)",
      toktype,
      extname,
      (short)tagwbxml,
      (short)aToken->tagid
    ));
    #endif
  } // if tag
  else if (IS_CONTENT(aToken)) {
    // content
    SMLERRPRINTFX(aDebugMask,("- parsing content (%s token, ext=%s)",toktype,extname));
  }
  else {
    // unknown
    SMLERRPRINTFX(aDebugMask,("- parsing %s token, ext=%s",toktype,extname));
  }
} // show_token


Ret_t show_decode_error(Ret_t aRc, XltDecScannerPtr_t aScanner, char *aRoutineName)
{
  char hexshow[numBytesShow*3+1];
  char *p;
  unsigned char *b;
  int i;

  if (aRc!=SML_ERR_OK) {
    // get routine name
    if (aRoutineName==NULL)
      aRoutineName="SML"; // generic sml
    // Show error
    SMLERRPRINTFX(DBG_ERROR,("%s: smlErr 0x%hX (%s) while parsing",aRoutineName,aRc,smlErrorText(aRc)));
    // Show details
    if (aScanner && aScanner->curtok) {
      // show what token we are parsing
      show_token(aScanner->curtok,DBG_ERROR);
      // show hex of 32 bytes after scan position
      b = aScanner->getPos(aScanner);
      SMLERRPRINTFX(DBG_ERROR,(
        "- Tag start at 0x%lX, scanner pos at 0x%lX%s, data:",
        aScanner->curtok->start,
        b,
        aScanner->finished ? " (finished)" : ""
      ));
      if (b!=NULL) {
        p=hexshow;
        for (i=0; i<numBytesShow; i++) {
          *p++ = NibbleToHexDigit(*b>>4);
          *p++ = NibbleToHexDigit(*b++);
          *p++ = ' ';
        }
        *p=0;
        b-=numBytesShow; // rewind
        SMLERRPRINTFX(DBG_ERROR,(hexshow));
        p=hexshow;
        for (i=0; i<numBytesShow; i++) {
          *p++ = (*b>=0x20) && (*b<0x7F) ? *b : '_';
          b++;
        }
        *p=0;
        SMLERRPRINTFX(DBG_ERROR,(hexshow));
      }
    }
    else {
      // no scanner or no current token
      SMLERRPRINTFX(DBG_ERROR,(
        "%s: smlErr 0x%hX while parsing no current tag",
        aRoutineName,
        aRc
      ));
    }
  }
  // return error code
  return aRc;
} // show_decode_error

#endif // SYDEBUG


#ifdef __USE_EXTENSIONS__
/* prototype for function in xltdecwbxml.c */
void subdtdDecodeWbxml(XltDecoderPtr_t pDecoder,SmlPcdataPtr_t *ppPcdata);
#endif

/**
 * Tries to concatenate two Pcdata elements. Only works when the two
 * elements are of the same type (e.g. SML_PCDATA_STRING). Returns a
 * pointer to the new Pcdata element or NULL if concatenation failed.
 */
static SmlPcdataPtr_t concatPCData(SmlPcdataPtr_t pDat1, const SmlPcdataPtr_t pDat2);


/**
 * appendXXXList
 *
 * These are auxiliary functions for building SyncML elements that contain
 * lists of certain other data structures (e.g. Items). They take an
 * existing list (e.g. of type ItemListPtr_t) and append an appropriate
 * element at the end. If the ListPtr points to NULL a new list is created.
 *
 * @pre The scanner's current token is the start tag (may be
 *      empty) of the SyncML element to be appended to the list.
 * @post The scanner's current token is the end tag (or empty
 *       start tag) of the SyncML element that was added to the list.
 * @param pDecoder (IN/OUT)
 *        the decoder
 * @param ppXXXList (IN/OUT)
 *        NULL or an initialized list, to which element will be appended
 * @return SML_ERR_OK, if an element was successfully appended\n
 *         else error code
 */
static Ret_t appendItemList(XltDecoderPtr_t pDecoder, SmlItemListPtr_t *ppItemList);
#ifdef SEARCH_RECEIVE
static Ret_t appendSourceList(XltDecoderPtr_t pDecoder, SmlSourceListPtr_t *ppSourceList);
#endif
#ifdef MAPITEM_RECEIVE
  static Ret_t appendMapItemList(XltDecoderPtr_t pDecoder, SmlMapItemListPtr_t *ppMapItemList);
#endif
static Ret_t appendTargetRefList(XltDecoderPtr_t pDecoder, SmlTargetRefListPtr_t *ppTargetRefList);
static Ret_t appendSourceRefList(XltDecoderPtr_t pDecoder, SmlSourceRefListPtr_t *ppSourceRefList);

/* if the commands are not defined we let the functions point to NULL */
#ifndef RESULT_RECEIVE
#define buildResults NULL
#endif

#ifndef MAP_RECEIVE
#define buildMap NULL
#endif

#ifndef EXEC_RECEIVE
#define buildExec NULL
#endif

#if !defined(ATOMIC_RECEIVE) && !defined(SEQUENCE_RECEIVE)
#define buildAtomOrSeq NULL
#endif

#ifndef SEARCH_RECEIVE
#define buildSearch NULL
#endif


typedef struct PEBuilder_s
{
    XltTagID_t     tagid;
    SmlProtoElement_t type;
    Ret_t (*build)(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem);
} PEBuilder_t, *PEBuilderPtr_t;

/* luz 2005-08-17: simplified, removed unnecessary copying of the const static table */

static const PEBuilder_t cPE_Dec[] = {
  { TN_ADD,       SML_PE_ADD,             buildGenericCmd },
  { TN_ALERT,     SML_PE_ALERT,           buildAlert      },
  { TN_ATOMIC,    SML_PE_ATOMIC_START,    buildAtomOrSeq  },
  { TN_COPY,      SML_PE_COPY,            buildGenericCmd },
  { TN_DELETE,    SML_PE_DELETE,          buildGenericCmd },
  { TN_EXEC,      SML_PE_EXEC,            buildExec       },
  { TN_GET,       SML_PE_GET,             buildPutOrGet   },
  { TN_MAP,       SML_PE_MAP,             buildMap        },
  { TN_PUT,       SML_PE_PUT,             buildPutOrGet   },
  { TN_RESULTS,   SML_PE_RESULTS,         buildResults    },
  { TN_SEARCH,    SML_PE_SEARCH,          buildSearch     },
  { TN_SEQUENCE,  SML_PE_SEQUENCE_START,  buildAtomOrSeq  },
  { TN_STATUS,    SML_PE_STATUS,          buildStatus     },
  { TN_SYNC,      SML_PE_SYNC_START,      buildSync       },
  { TN_REPLACE,   SML_PE_REPLACE,         buildGenericCmd },
  { TN_MOVE,      SML_PE_MOVE,            buildGenericCmd },
  { TN_UNDEF,     SML_PE_UNDEF,           0               }
};


/*************************************************************************/
/* External Functions                                                    */
/*************************************************************************/
/**
 * Description see XLTDec.h header file.
 */
Ret_t
xltDecInit(const SmlEncoding_t enc,
        const MemPtr_t pBufEnd,
        MemPtr_t *ppBufPos,
        XltDecoderPtr_t *ppDecoder,
        SmlSyncHdrPtr_t *ppSyncHdr)
{
    XltDecoderPtr_t pDecoder;
    Ret_t rc;


    /* create new decoder object */
    if ((pDecoder = (XltDecoderPtr_t)smlLibMalloc(sizeof(XltDecoder_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    pDecoder->finished = 0;
    pDecoder->final = 0;
    pDecoder->scanner = NULL;
    if ((rc = xltUtilCreateStack(&pDecoder->tagstack, 10)) != SML_ERR_OK) {
        xltDecTerminate(pDecoder);
        return rc;
    }

#ifdef __SML_WBXML__
    if (enc == SML_WBXML)
    {
        rc = xltDecWbxmlInit(pBufEnd, ppBufPos, &pDecoder->scanner);
        if (rc == SML_ERR_OK)
        {
            pDecoder->charset = pDecoder->scanner->charset;
            pDecoder->charsetStr = NULL;
        }
    } else
#endif

#ifdef __SML_XML__
    if (enc == SML_XML)
    {

        rc = xltDecXmlInit(pBufEnd, ppBufPos, &pDecoder->scanner);
        if (rc == SML_ERR_OK)
        {
            pDecoder->charset = 0;
            pDecoder->charsetStr = pDecoder->scanner->charsetStr;
        }
    } else
#endif

    {
        rc = SML_ERR_XLT_ENC_UNK;
    }

    if (rc != SML_ERR_OK)
    {
        xltDecTerminate((XltDecoderPtr_t)pDecoder);
        return rc;
    }

    /* try to find SyncHdr element, first comes the SyncML tag... */
    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        xltDecTerminate((XltDecoderPtr_t)pDecoder);
        return rc;
    }
    if (!IS_START(pDecoder->scanner->curtok) ||
            (pDecoder->scanner->curtok->tagid != TN_SYNCML)) {
        smlFreePcdata(pDecoder->scanner->curtok->pcdata);
        xltDecTerminate((XltDecoderPtr_t)pDecoder);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pDecoder->scanner,"xltDecInit");
    }

    /* ... then the SyncHdr */
    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        xltDecTerminate((XltDecoderPtr_t)pDecoder);
        return rc;
    }
    if ((rc = buildSyncHdr(pDecoder, (VoidPtr_t)ppSyncHdr)) != SML_ERR_OK) {
        xltDecTerminate((XltDecoderPtr_t)pDecoder);
        return rc;
    }

    *ppBufPos = pDecoder->scanner->getPos(pDecoder->scanner);

    *ppDecoder = (XltDecoderPtr_t)pDecoder;

    return SML_ERR_OK;
}

/**
 * Description see XLTDec.h header file.
 */
Ret_t
xltDecNext(XltDecoderPtr_t pDecoder,
        const MemPtr_t pBufEnd,
        MemPtr_t *ppBufPos,
        SmlProtoElement_t *pe,
        VoidPtr_t *ppContent)
{
    XltDecoderPtr_t pDecPriv = (XltDecoderPtr_t)pDecoder;
    XltDecScannerPtr_t pScanner = pDecPriv->scanner;
    XltTagID_t tagid;
    Ret_t rc;
    int i;

    pScanner->setBuf(pScanner, *ppBufPos, pBufEnd);

    /* if we are still outside the SyncBody, look for SyncBody start tag */
    if ((rc = pDecPriv->tagstack->top(pDecPriv->tagstack, &tagid)) != SML_ERR_OK)
        return rc;
    if (tagid == TN_SYNCML) {
        if (((rc = nextToken(pDecPriv)) != SML_ERR_OK)) {
            return rc;
        }
        if (!((IS_START(pScanner->curtok)) &&
             (pScanner->curtok->tagid == TN_SYNCBODY))) {
            return SML_DECODEERROR(SML_ERR_XLT_INVAL_PROTO_ELEM,pScanner,"xltDecNext");
        }
    }

    if ((rc = nextToken(pDecPriv)) != SML_ERR_OK)
        return rc;

    /* if we find a SyncML protocol element build the corresponding
       data structure */
    if ((IS_START_OR_EMPTY(pScanner->curtok)) && (pScanner->curtok->tagid != TN_FINAL)) {

        i = 0;
        while ((cPE_Dec[i].tagid) != TN_UNDEF)
        {
            if ((cPE_Dec[i].tagid) == pScanner->curtok->tagid)
            {
                *pe = (cPE_Dec[i].type);
                if ((rc = cPE_Dec[i].build(pDecPriv, ppContent)) != SML_ERR_OK)
                    return rc;
                /* T.K. adjust the SML_PE_ for 'generic' structures
                if (*pe == SML_PE_GENERIC) {
                    SmlGenericCmdPtr_t g = *ppContent;
                    switch ((int) (cPE_Dec[i].tagid)) {
                    case TN_ADD    : g->elementType = SML_PE_ADD;     break;
                    case TN_COPY   : g->elementType = SML_PE_COPY;    break;
                    case TN_DELETE : g->elementType = SML_PE_DELETE;  break;
                    case TN_REPLACE: g->elementType = SML_PE_REPLACE; break;
                    }
                }
                */
                /* %%% luz 2005-08-17: we can do this a lot easier... */
                /* if the procduced element has no distinct element type now,
                   but is still SML_PE_GENERIC, we need to set it to the correct distinct value */
                if (((SmlGenericCmdPtr_t)(*ppContent))->elementType == SML_PE_GENERIC) {
                  ((SmlGenericCmdPtr_t)(*ppContent))->elementType = cPE_Dec[i].type;
                }

                break;
            }
            i++;
        }
        if ((cPE_Dec[i].tagid) == TN_UNDEF)
        {
            *pe = SML_PE_UNDEF;
            *ppContent = NULL;
            return SML_DECODEERROR(SML_ERR_XLT_INVAL_PROTO_ELEM,pScanner,"xltDecNext");
        }
    } else {

        /* found end tag */
        switch (pScanner->curtok->tagid) {
            case TN_ATOMIC:
                *pe = SML_PE_ATOMIC_END;
                *ppContent = NULL;
                break;
            case TN_SEQUENCE:
                *pe = SML_PE_SEQUENCE_END;
                *ppContent = NULL;
                break;
            case TN_SYNC:
                *pe = SML_PE_SYNC_END;
                *ppContent = NULL;
                break;
            case TN_FINAL:
                *pe = SML_PE_FINAL;
                *ppContent = NULL;
                pDecPriv->final = 1;
                break;
            case TN_SYNCBODY:
                /* next comes the SyncML end tag, then we're done */
                if ((rc = nextToken(pDecPriv)) != SML_ERR_OK)
                    return rc;
                if ((pScanner->curtok->type == TOK_TAG_END) &&
                        (pScanner->curtok->tagid == TN_SYNCML)) {
                    *pe = SML_PE_UNDEF;
                    *ppContent = NULL;
                    pDecPriv->finished = 1;
                } else {
                    return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"xltDecNext");
                }
                break;
            default:
                return SML_DECODEERROR(SML_ERR_XLT_INVAL_PROTO_ELEM,pScanner,"xltDecNext");
        }
    }

    *ppBufPos = pScanner->getPos(pScanner);

    return SML_ERR_OK;
}

/**
 * Description see XLTDec.h header file.
 */
Ret_t
xltDecTerminate(XltDecoderPtr_t pDecoder)
{
    XltDecoderPtr_t pDecPriv;

    if (pDecoder == NULL)
        return SML_ERR_OK;

    pDecPriv = (XltDecoderPtr_t)pDecoder;
    if (pDecPriv->scanner != NULL)
        pDecPriv->scanner->destroy(pDecPriv->scanner);
    if (pDecPriv->tagstack != NULL)
        pDecPriv->tagstack->destroy(pDecPriv->tagstack);
    smlLibFree(pDecPriv);

    return SML_ERR_OK;
}


Ret_t xltDecReset(XltDecoderPtr_t pDecoder)
{
  return xltDecTerminate(pDecoder);
}

/**
 * Gets the next token from the scanner.
 * Checks if the current tag is an end tag and if so, whether the last
 * open start tag has the same tag id as the current end tag. An open start
 * tag is one which matching end tag has not been seen yet.
 * If the current tag is a start tag its tag ID will be pushed onto the
 * tag stack.
 * If the current tag is an empty tag or not a tag at all nothing will be
 * done.
 */
Ret_t
nextToken(XltDecoderPtr_t pDecoder)
{
    XltUtilStackPtr_t pTagStack;
    XltDecTokenPtr_t pToken;
    Ret_t rc;


    if ((rc = pDecoder->scanner->nextTok(pDecoder->scanner)) != SML_ERR_OK)
        return rc;

    pToken = pDecoder->scanner->curtok;
    pTagStack = pDecoder->tagstack;

    #if SYDEBUG>2
    show_token(pToken,DBG_DETAILS);
    #endif

    if (IS_START(pToken)) {
        if (pTagStack->push(pTagStack, pToken->tagid))
            return SML_ERR_UNSPECIFIC;
    } else if (IS_END(pToken)) {
        XltTagID_t lastopen;
        if (pTagStack->pop(pTagStack, &lastopen))
            return SML_ERR_UNSPECIFIC;
        if (pToken->tagid != lastopen)
            return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pDecoder->scanner,"nextToken");
    }
    return SML_ERR_OK;
}

Ret_t discardToken(XltDecoderPtr_t pDecoder)

{
    Ret_t rc;
    XltTagID_t tmp;
    if ((rc = pDecoder->scanner->pushTok(pDecoder->scanner)) != SML_ERR_OK)
        return rc;
    if ((rc = pDecoder->tagstack->pop(pDecoder->tagstack, &tmp)) != SML_ERR_OK)
        return rc;
    return SML_ERR_OK;
}

/*************************************************************************/
/* Internal Functions                                                    */
/*************************************************************************/

static SmlPcdataPtr_t
concatPCData(SmlPcdataPtr_t pDat1, const SmlPcdataPtr_t pDat2)
{
    // luz: 2006-09-07: plain wrong again. We can very well concatenate
    //      STRING, OPAQUE and CDATA, only extensions are not concatenatable
    //      For this, no extra check is needed, as implementation below already does
    //%%% if (pDat1->contentType != pDat2->contentType)
    //%%%     return NULL;

    switch (pDat1->contentType) {
        case SML_PCDATA_STRING:
            /* luz: 2005-03-07: original RTK code: plain wrong - causes crash as pDat1->content is NOT
               big enough to have data appended without re-alloc!!
               - No need to make difference between STRING and OPAQUE anyway!
            pDat1->content = (VoidPtr_t)smlLibStrcat(pDat1->content, pDat2->content);
            pDat1->length += pDat2->length;
            break;
            */
        case SML_PCDATA_CDATA:
        case SML_PCDATA_OPAQUE:
            if (pDat2->contentType==SML_PCDATA_EXTENSION)
              return NULL; // can't concat
            // luz: 2005-03-07: made sure we ALWAYS have a null byte at the end of an opaque string
            if ((pDat1->content = smlLibRealloc(pDat1->content, pDat1->length + pDat2->length + 1)) == NULL)
                return NULL;
            smlLibMemmove(((Byte_t*)pDat1->content) + pDat1->length, pDat2->content, pDat2->length);
            pDat1->length += pDat2->length;
            ((char *)pDat1->content)[pDat1->length]=0; // null byte at the end
            break;
        default:
            return NULL; // can't concat
    }
    return pDat1;
}

Ret_t
buildSyncHdr(XltDecoderPtr_t pDecoder, VoidPtr_t *ppSyncHdr)
{
    XltDecScannerPtr_t pScanner;
    SmlSyncHdrPtr_t pSyncHdr;
    Ret_t rc;
    Long_t sessionid = 0, msgid = 0, source = 0, target = 0, version = 0, proto = 0;

    /* shortcut to the scanner object */
    pScanner = pDecoder->scanner;

    /* if ppSyncHdr is not NULL we've already
       found a SyncHdr before! */
    if (*ppSyncHdr != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildSyncHdr");

    /* initialize new SmlSyncHdr */
    if ((pSyncHdr = (SmlSyncHdrPtr_t)smlLibMalloc(sizeof(SmlSyncHdr_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pSyncHdr, 0, sizeof(SmlSyncHdr_t));

    /* initialize the element type field */
    pSyncHdr->elementType = SML_PE_HEADER;

    /* empty SmlSyncHdr is possible */
    if (IS_EMPTY(pScanner->curtok)) {
        *ppSyncHdr = pSyncHdr;
        return SML_ERR_OK;
    }

    /* get next Token */
    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pSyncHdr);
        return rc;
    }

    /* parse child elements until we find a matching end tag */
    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_VERSION:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pSyncHdr->version);
                version++;
                break;
            case TN_PROTO:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pSyncHdr->proto);
                proto++;
                break;
            case TN_SESSIONID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pSyncHdr->sessionID);
                sessionid++;
                break;
            case TN_MSGID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pSyncHdr->msgID);
                msgid++;
                break;
            case TN_RESPURI:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pSyncHdr->respURI);
                break;

                /* child tags */
            case TN_TARGET:
                rc = buildTargetOrSource(pDecoder, (VoidPtr_t)&pSyncHdr->target);
                target++;
                break;
            case TN_SOURCE:
                rc = buildTargetOrSource(pDecoder, (VoidPtr_t)&pSyncHdr->source);
                source++;
                break;
            case TN_CRED:
                rc = buildCred(pDecoder, (VoidPtr_t)&pSyncHdr->cred);
                break;
            case TN_META:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pSyncHdr->meta);
                break;

                /* flags (empty tags) */
            case TN_NORESP:
                pSyncHdr->flags |= SmlNoResp_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildSyncHdr");
        }

        /* decoding of child element went ok? */
        if (rc != SML_ERR_OK) {
            smlFreeSyncHdr(pSyncHdr);

            return rc;
        }

        /* get next token */
        if ((rc = nextToken(pDecoder)) != SML_ERR_OK) {
            smlFreeSyncHdr(pSyncHdr);
            return rc;
        }
    }

    if ((sessionid == 0) || (msgid == 0) || (target == 0) || (source == 0) || (version == 0) || (proto == 0))
    {
      smlFreeSyncHdr(pSyncHdr);
      return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildSyncHdr");
    }

    *ppSyncHdr = pSyncHdr;

    return SML_ERR_OK;
}

Ret_t
buildSync(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem)
{
    XltDecScannerPtr_t pScanner;
    SmlSyncPtr_t pSync;
    Ret_t rc;
    Long_t cmdid = 0;

    /* stop decoding the Sync when we find a SyncML command */
    Byte_t break_sync = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildSync");

    /* initialize a new Sync */
    if ((pSync = (SmlSyncPtr_t)smlLibMalloc(sizeof(SmlSync_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pSync, 0, sizeof(SmlSync_t));

    /* initialize the element type field */
    pSync->elementType = SML_PE_SYNC_START;

    if (IS_EMPTY(pScanner->curtok)) {

        smlLibFree(pSync);
        return SML_ERR_OK;
    }

    /* get next token */
    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pSync);
        return rc;
    }

    /* parse child elements until we find a matching end tag
       or until we find a TN_ADD, TN_ATOMIC, etc. start tag */
    while ((pScanner->curtok->type != TOK_TAG_END) && !break_sync) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_CMDID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pSync->cmdID);
                cmdid++;
                break;
            case TN_META:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pSync->meta);
                break;
            case TN_NUMBEROFCHANGES:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pSync->noc);
                break;

                /* child tags */
            case TN_CRED:
                rc = buildCred(pDecoder, (VoidPtr_t)&pSync->cred);
                break;
            case TN_TARGET:
                rc = buildTargetOrSource(pDecoder, (VoidPtr_t)&pSync->target);
                break;
            case TN_SOURCE:
                rc = buildTargetOrSource(pDecoder, (VoidPtr_t)&pSync->source);
                break;

                /* flags */
            case TN_NORESP:
                pSync->flags |= SmlNoResp_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;

                /* quit if we find an Add, Atomic, etc.
                   element */
            case TN_ADD:
            case TN_ATOMIC:
            case TN_COPY:
            case TN_DELETE:
            case TN_SEQUENCE:
            case TN_REPLACE:
                break_sync = 1;
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildSync");
        }
        if (rc != SML_ERR_OK) {
            smlFreeSync(pSync);
            return rc;
        }
        if (!break_sync) {
            /* get next token and continue as usual */
            if ((rc = nextToken(pDecoder)) != SML_ERR_OK) {
                smlFreeSync(pSync);
                return rc;
            }
        } else {
            /* we've found a SyncML command - we need to go
               back one token and correct the tagstack */
            if ((rc = discardToken(pDecoder)) != SML_ERR_OK) {
                smlFreeSync(pSync);
                return rc;
            }
        }
    }

    if (!break_sync)  {
      if ((pScanner->curtok->tagid) != TN_SYNC)
      {
        smlFreeSync(pSync);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildSync");
      }
      else
      {
         if (pDecoder->tagstack->push(pDecoder->tagstack, pScanner->curtok->tagid))
         {
            smlFreeSync(pSync);
            return SML_ERR_UNSPECIFIC;
         }
         if ((rc = pDecoder->scanner->pushTok(pDecoder->scanner)) != SML_ERR_OK)
         {
           smlFreeSync(pSync);
           return rc;
         }
      }
    }

    *ppElem = pSync;

    return SML_ERR_OK;
}

#if (defined ATOMIC_RECEIVE || defined SEQUENCE_RECEIVE)
Ret_t
buildAtomOrSeq(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem)
{
    XltDecScannerPtr_t pScanner;
    SmlAtomicPtr_t pAoS;        /* SmlAtomicPtr_t and SequencePtr_t are pointer
                                to the same structure! */
    Ret_t rc;
    Byte_t break_aos = 0;    /* stop decoding the Atomic when we find a
                                SyncML command */
    Long_t cmdid = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildAtomOrSeq");

    if ((pAoS = (SmlAtomicPtr_t)smlLibMalloc(sizeof(SmlAtomic_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pAoS, 0, sizeof(SmlAtomic_t));

    /* initialize the element type field */
    pAoS->elementType = SML_PE_CMD_GROUP;

    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pAoS);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildAtomOrSeq");
    }

    /* get next token */
    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree (pAoS);
        return rc;
    }

    /* parse child elements until we find a matching end tag
       or until we find a TN_ADD, TN_ATOMIC, etc. start tag */
    while ((pScanner->curtok->type != TOK_TAG_END) && !break_aos) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_CMDID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pAoS->cmdID);
                cmdid++;
                break;
            case TN_META:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pAoS->meta);
                break;

                /* flags */
            case TN_NORESP:
                pAoS->flags |= SmlNoResp_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;

                /* quit if we find an Add, Atomic, etc.
                   element */
            case TN_ADD:
            case TN_REPLACE:
            case TN_DELETE:
            case TN_COPY:
            case TN_ATOMIC:
            case TN_MAP:
            case TN_SYNC:
      case TN_GET:
      case TN_ALERT:
      case TN_EXEC:
                break_aos = 1;
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildAtomOrSeq");
        }
        if (rc != SML_ERR_OK) {
            smlFreeAtomic(pAoS);
            return rc;
        }
        if (!break_aos) {
            if ((rc = nextToken(pDecoder)) != SML_ERR_OK) {
                smlFreeAtomic(pAoS);
                return rc;
            }
        } else {
            /* we've found a SyncML command - we need to go
               back one token and correct the tagstack */
            if ((rc = discardToken(pDecoder)) != SML_ERR_OK) {
                smlFreeAtomic(pAoS);
                return rc;
            }
        }
    }

    if (!break_aos) {
        /* Atomic/Sequence must contain at least one SyncML command */
        smlFreeAtomic(pAoS);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildAtomOrSeq");
    }

    if (cmdid == 0)
    {
        smlFreeAtomic(pAoS);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildAtomOrSeq");
    }

    *ppElem = pAoS;

    return SML_ERR_OK;
}
#endif

#ifdef EXEC_RECEIVE
Ret_t
buildExec(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem)
{
    XltDecScannerPtr_t pScanner;
    SmlExecPtr_t pExec;
    Ret_t rc;
    Long_t items = 0, cmdid = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildExec");

    if ((pExec = (SmlExecPtr_t)smlLibMalloc(sizeof(SmlExec_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pExec, 0, sizeof(SmlExec_t));

    /* initialize the element type field */
    pExec->elementType = SML_PE_EXEC;

    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pExec);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildExec");
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pExec);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCData */
            case TN_CMDID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pExec->cmdID);
                cmdid++;
                break;

            case TN_META:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pExec->meta);
                break;

            case TN_CORRELATOR:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pExec->correlator);
                break;

                /* child tags */
            case TN_CRED:
                rc = buildCred(pDecoder, (VoidPtr_t)&pExec->cred);
                break;

            case TN_ITEM:
                rc = appendItemList(pDecoder, &pExec->itemList);
                items++;
                break;

                /* flags */
            case TN_NORESP:
                pExec->flags |= SmlNoResp_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;

            default:
                rc =  SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildExec");
        }
        if (rc != SML_ERR_OK) {
            smlFreeExec(pExec);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeExec(pExec);
            return rc;
        }
    }

    if ((items == 0) || (cmdid == 0)) {
        smlFreeExec(pExec);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildExec");
    }

    *ppElem = pExec;

    return SML_ERR_OK;
}
#endif

Ret_t
buildGenericCmd(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem)
{
    XltDecScannerPtr_t pScanner;
    SmlGenericCmdPtr_t pGenCmd;
    Ret_t rc;
    Long_t items = 0, cmdid = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildGenericCmd");

    /* initialize a new GenericCmd */
    if ((pGenCmd = (SmlGenericCmdPtr_t)smlLibMalloc(sizeof(SmlGenericCmd_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pGenCmd, 0, sizeof(SmlGenericCmd_t));

    /* initialize the element type field */
    pGenCmd->elementType = SML_PE_GENERIC;

    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pGenCmd);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildGenericCmd");
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pGenCmd);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_CMDID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pGenCmd->cmdID);
                cmdid++;
                break;
            case TN_META:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pGenCmd->meta);
                break;

                /* child tags */
            case TN_CRED:
                rc = buildCred(pDecoder, (VoidPtr_t)&pGenCmd->cred);
                break;

                /* flags (empty tags) */
            case TN_NORESP:
                pGenCmd->flags |= SmlNoResp_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;
            case TN_ARCHIVE:
                pGenCmd->flags |= SmlArchive_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;
            case TN_SFTDEL:
                pGenCmd->flags |= SmlSftDel_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;

                /* Lists */
            case TN_ITEM:
                rc = appendItemList(pDecoder, &pGenCmd->itemList);
                items++;
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildGenericCmd");
        }
        if (rc != SML_ERR_OK) {
            smlFreeGeneric(pGenCmd);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeGeneric(pGenCmd);
            return rc;
        }
    }

    if ((items == 0) || (cmdid == 0))
    {
        smlFreeGeneric(pGenCmd);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildGenericCmd");
    }

    *ppElem = pGenCmd;

    return SML_ERR_OK;
}

Ret_t
buildAlert(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem)
{
    XltDecScannerPtr_t pScanner;
    SmlAlertPtr_t pAlert;
    Ret_t rc;
    Long_t cmdid = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildAlert");

    if ((pAlert = (SmlAlertPtr_t)smlLibMalloc(sizeof(SmlAlert_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pAlert, 0, sizeof(SmlAlert_t));

    /* initialize the element type field */
    pAlert->elementType = SML_PE_ALERT;

    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pAlert);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildAlert");
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pAlert);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_CMDID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pAlert->cmdID);
                cmdid++;
                break;
            case TN_DATA:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pAlert->data);
                break;

                /* child tags */
            case TN_CRED:
                rc = buildCred(pDecoder, (VoidPtr_t)&pAlert->cred);
                break;

                /* flags (empty tags) */
            case TN_NORESP:
                pAlert->flags |= SmlNoResp_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;

                /* Lists */
            case TN_ITEM:
                rc = appendItemList(pDecoder, &pAlert->itemList);
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildAlert");
        }
        if (rc != SML_ERR_OK) {
            smlFreeAlert(pAlert);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeAlert(pAlert);
            return rc;
        }
    }

    if (cmdid == 0)
    {
        smlFreeAlert(pAlert);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildAlert");
    }

    *ppElem = pAlert;

    return SML_ERR_OK;
}

#ifdef MAP_RECEIVE
Ret_t
buildMap(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem)
{
    XltDecScannerPtr_t pScanner;
    SmlMapPtr_t pMap;
    Ret_t rc;
    Long_t target = 0, source = 0, mapitems = 0, cmdid = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildMap");

    if ((pMap = (SmlMapPtr_t)smlLibMalloc(sizeof(SmlMap_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pMap, 0, sizeof(SmlMap_t));

    /* initialize the element type field */
    pMap->elementType = SML_PE_MAP;

    /* Source is required */
    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pMap);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildMap");
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pMap);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_CMDID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pMap->cmdID);
                cmdid++;
                break;
            case TN_META:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pMap->meta);
                break;

                /* child tags */
            case TN_CRED:
                rc = buildCred(pDecoder, (VoidPtr_t)&pMap->cred);
                break;
            case TN_SOURCE:
                rc = buildTargetOrSource(pDecoder, (VoidPtr_t)&pMap->source);
                source++;
                break;
            case TN_TARGET:
                rc = buildTargetOrSource(pDecoder, (VoidPtr_t)&pMap->target);
                target++;
                break;
#ifdef MAPITEM_RECEIVE
                /* Lists */
            case TN_MAPITEM:
                rc = appendMapItemList(pDecoder, &pMap->mapItemList);
                mapitems++;
                break;
#endif
            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildMap");
        }
        if (rc != SML_ERR_OK) {
            smlFreeMap(pMap);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeMap(pMap);
            return rc;
        }
    }

    if ((source == 0) || (mapitems == 0) || (target == 0) || (cmdid == 0)) {
        smlFreeMap(pMap);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildMap");
    }

    *ppElem = pMap;

    return SML_ERR_OK;
}
#endif

#ifdef SEARCH_RECEIVE
Ret_t
buildSearch(XltDecoderPtr_t pDecoder, VoidPtr_t *ppSearch)
{
    XltDecScannerPtr_t pScanner;
    SmlSearchPtr_t pSearch;
    Ret_t rc;
    Long_t source = 0, meta = 0, data = 0, cmdid = 0;

    pScanner = pDecoder->scanner;

    if (*ppSearch != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildSearch");

    if ((pSearch = (SmlSearchPtr_t)smlLibMalloc(sizeof(SmlSearch_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pSearch, 0, sizeof(SmlSearch_t));

    /* initialize the element type field */
    pSearch->elementType = SML_PE_SEARCH;

    /* Meta is required */
    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pSearch);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildSearch");
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pSearch);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_CMDID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pSearch->cmdID);
                cmdid++;
                break;
            case TN_LANG:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pSearch->lang);
                break;
            case TN_META:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pSearch->meta);
                meta++;
                break;
            case TN_DATA:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pSearch->data);
                data++;
                break;


                /* child tags */
            case TN_CRED:
                rc = buildCred(pDecoder, (VoidPtr_t)&pSearch->cred);
                break;
            case TN_TARGET:
                rc = buildTargetOrSource(pDecoder, (VoidPtr_t)&pSearch->target);
                break;

                /* flags */
            case TN_NORESP:
                pSearch->flags |= SmlNoResp_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;
            case TN_NORESULTS:
                pSearch->flags |= SmlNoResults_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;

                /* Lists */
            case TN_SOURCE:
                rc = appendSourceList(pDecoder, &pSearch->sourceList);
                source++;
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildSearch");
        }
        if (rc != SML_ERR_OK) {
            smlFreeSearch(pSearch);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeSearch(pSearch);
            return rc;
        }
    }

    if ((source == 0) || (meta == 0) || (data == 0) || (cmdid == 0)) {
        smlFreeSearch(pSearch);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildSearch");
    }

    *ppSearch = pSearch;

    return SML_ERR_OK;
}
#endif

Ret_t
buildPutOrGet(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem)
{
    XltDecScannerPtr_t pScanner;
    SmlGetPtr_t pGet;
    Ret_t rc;
    Long_t items = 0, cmdid = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildPutOrGet");

    if ((pGet = (SmlGetPtr_t)smlLibMalloc(sizeof(SmlGet_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pGet, 0, sizeof(SmlGet_t));

    /* initialize the element type field */
    pGet->elementType = SML_PE_PUT_GET;

    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pGet);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildPutOrGet");
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pGet);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_CMDID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pGet->cmdID);
                cmdid++;
                break;
            case TN_LANG:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pGet->lang);
                break;
            case TN_META:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pGet->meta);
                break;

                /* child tags */
            case TN_CRED:
                rc = buildCred(pDecoder, (VoidPtr_t)&pGet->cred);
                break;

                /* flags */
            case TN_NORESP:
                pGet->flags |= SmlNoResp_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;

                /* Lists */

            case TN_ITEM:
                rc = appendItemList(pDecoder, &pGet->itemList);
                items++;
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildPutOrGet");
        }
        if (rc != SML_ERR_OK) {
            smlFreeGetPut(pGet);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeGetPut(pGet);
            return rc;
        }
    }

    if ((items == 0) || (cmdid == 0))
    {
        smlFreeGetPut(pGet);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildPutOrGet");
    }

    *ppElem = pGet;

    return SML_ERR_OK;
}


Ret_t
buildRecordFieldFilter(XltDecoderPtr_t pDecoder, VoidPtr_t *ppRecordFieldFilter)
{
    XltDecScannerPtr_t pScanner;
    SmlRecordOrFieldFilterPtr_t pRecordFieldFilter;
    Long_t item = 0;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppRecordFieldFilter != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildRecordFieldFilter");

    if ((pRecordFieldFilter = (SmlRecordOrFieldFilterPtr_t)smlLibMalloc(sizeof(SmlRecordOrFieldFilter_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pRecordFieldFilter, 0, sizeof(SmlRecordOrFieldFilter_t));

    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pRecordFieldFilter);
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pRecordFieldFilter);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* Item */
            case TN_ITEM:
                rc = buildItem(pDecoder, (VoidPtr_t)&pRecordFieldFilter->item);
                item++;
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildRecordFieldFilter");
        }
        if (rc != SML_ERR_OK) {
            smlFreeRecordFieldFilterPtr(pRecordFieldFilter);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeRecordFieldFilterPtr(pRecordFieldFilter);
            return rc;
        }
    }

    if (item == 0)
    {
        smlFreeRecordFieldFilterPtr(pRecordFieldFilter);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildRecordFieldFilter");
    }

    *ppRecordFieldFilter = pRecordFieldFilter;

    return SML_ERR_OK;
}


Ret_t
buildFilter(XltDecoderPtr_t pDecoder, VoidPtr_t *ppFilter)
{
    XltDecScannerPtr_t pScanner;
    SmlFilterPtr_t pFilter;
    Long_t meta = 0;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppFilter != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildFilter");

    if ((pFilter = (SmlFilterPtr_t)smlLibMalloc(sizeof(SmlFilter_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pFilter, 0, sizeof(SmlFilter_t));

    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pFilter);
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pFilter);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_META:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pFilter->meta);
                meta++;
                break;

            /* Record or Field elements */
            case TN_FIELD:
                rc = buildRecordFieldFilter(pDecoder, (VoidPtr_t)&pFilter->field);
                break;

            case TN_RECORD:
                rc = buildRecordFieldFilter(pDecoder, (VoidPtr_t)&pFilter->record);
                break;

            /* Flags */
            case TN_FILTERTYPE:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pFilter->filtertype);
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildFilter");
        }
        if (rc != SML_ERR_OK) {
            smlFreeFilterPtr(pFilter);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeFilterPtr(pFilter);
            return rc;
        }
    }

    if (meta == 0)
    {
        smlFreeFilterPtr(pFilter);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildFilter");
    }

    *ppFilter = pFilter;

    return SML_ERR_OK;
}


Ret_t
buildTargetOrSourceParent(XltDecoderPtr_t pDecoder, VoidPtr_t *ppTargetParent)
{
    XltDecScannerPtr_t pScanner;
    SmlTargetParentPtr_t pTargetParent;
    Long_t locuri = 0;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppTargetParent != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildTargetOrSourceParent");

    if ((pTargetParent = (SmlTargetParentPtr_t)smlLibMalloc(sizeof(SmlTargetParent_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pTargetParent, 0, sizeof(SmlTargetParent_t));

    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pTargetParent);
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pTargetParent);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_LOCURI:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pTargetParent->locURI);
                locuri++;
                break;
            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildTargetOrSourceParent");
        }
        if (rc != SML_ERR_OK) {
            smlFreeSourceTargetParentPtr(pTargetParent);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeSourceTargetParentPtr(pTargetParent);
            return rc;
        }
    }

    if (locuri == 0)
    {
        smlFreeSourceTargetParentPtr(pTargetParent);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildTargetOrSourceParent");
    }

    *ppTargetParent = pTargetParent;

    return SML_ERR_OK;
}


Ret_t
buildTargetOrSource(XltDecoderPtr_t pDecoder, VoidPtr_t *ppTarget)
{
    XltDecScannerPtr_t pScanner;
    SmlTargetPtr_t pTarget;
    Long_t locuri = 0;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppTarget != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildTargetOrSourceParent");

    if ((pTarget = (SmlTargetPtr_t)smlLibMalloc(sizeof(SmlTarget_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pTarget, 0, sizeof(SmlTarget_t));

    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pTarget);
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pTarget);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_LOCURI:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pTarget->locURI);
                locuri++;
                break;
            case TN_LOCNAME:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pTarget->locName);
                break;
            /* SyncML 1.2 Filter element, %%% added 2005-08-17 by synthesis/luz */
            case TN_FILTER:
                rc = buildFilter(pDecoder, (VoidPtr_t)&pTarget->filter);
                break;
            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildTargetOrSourceParent");
        }
        if (rc != SML_ERR_OK) {
            smlFreeSourceTargetPtr(pTarget);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeSourceTargetPtr(pTarget);
            return rc;
        }
    }

    if (locuri == 0)
    {
        smlFreeSourceTargetPtr(pTarget);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildTargetOrSourceParent");
    }

    *ppTarget = pTarget;

    return SML_ERR_OK;
}

Ret_t
buildChal(XltDecoderPtr_t pDecoder, VoidPtr_t *ppChal)
{
    XltDecScannerPtr_t pScanner;
    SmlChalPtr_t pChal;
    Long_t meta = 0;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppChal != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildChal");

    if ((pChal = (SmlChalPtr_t)smlLibMalloc(sizeof(SmlChal_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pChal, 0, sizeof(SmlChal_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppChal = pChal;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pChal);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_META:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pChal->meta);
                meta++;
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildChal");
        }
        if (rc != SML_ERR_OK) {
            smlFreeChalPtr(pChal);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeChalPtr(pChal);
            return rc;
        }
    }

    if (meta == 0)
    {
        smlFreeChalPtr(pChal);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildChal");
    }

    *ppChal = pChal;

    return SML_ERR_OK;
}

Ret_t
buildCred(XltDecoderPtr_t pDecoder, VoidPtr_t *ppCred)
{
    XltDecScannerPtr_t pScanner;
    SmlCredPtr_t pCred;
    Ret_t rc;
    Long_t data = 0;

    pScanner = pDecoder->scanner;

    if (*ppCred != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildCred");

    if ((pCred = (SmlCredPtr_t)smlLibMalloc(sizeof(SmlCred_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pCred, 0, sizeof(SmlCred_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppCred = pCred;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pCred);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_DATA:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pCred->data);
                data++;
                break;
            case TN_META:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pCred->meta);
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildCred");
        }
        if (rc != SML_ERR_OK) {
            smlFreeCredPtr(pCred);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeCredPtr(pCred);
            return rc;
        }
    }

    if (data == 0)
    {
      smlFreeCredPtr(pCred);
      return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildCred");
    }

    *ppCred = pCred;

    return SML_ERR_OK;
}

Ret_t
buildItem(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem)
{
    XltDecScannerPtr_t pScanner;
    SmlItemPtr_t pItem;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildItem");

    if ((pItem = (SmlItemPtr_t)smlLibMalloc(sizeof(SmlItem_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pItem, 0, sizeof(SmlItem_t));

    /* Item might be empty */
    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pItem;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pItem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_META:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pItem->meta);
                break;
            case TN_DATA:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pItem->data);
#ifdef __USE_EXTENSIONS__
#ifdef __SML_WBXML__
                if (pItem->data && pItem->data->contentType == SML_PCDATA_OPAQUE)
                    subdtdDecodeWbxml(pDecoder, (SmlPcdataPtr_t*)&pItem->data);
#endif
#endif
                break;
                /* child tags */
            case TN_TARGET:
                rc = buildTargetOrSource(pDecoder, (VoidPtr_t)&pItem->target);
                break;
            case TN_SOURCE:
                rc = buildTargetOrSource(pDecoder, (VoidPtr_t)&pItem->source);
                break;
            case TN_TARGETPARENT:
                rc = buildTargetOrSourceParent(pDecoder, (VoidPtr_t)&pItem->targetParent);
                break;
            case TN_SOURCEPARENT:
                rc = buildTargetOrSourceParent(pDecoder, (VoidPtr_t)&pItem->sourceParent);
                break;

            /* flags */
            case TN_MOREDATA:
                pItem->flags |= SmlMoreData_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildItem");
        }
        if (rc != SML_ERR_OK) {
            smlFreeItemPtr(pItem);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeItemPtr(pItem);
            return rc;
        }
    }

    *ppElem = pItem;

    return SML_ERR_OK;
}

#ifdef MAPITEM_RECEIVE
Ret_t
buildMapItem(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem)
{
    XltDecScannerPtr_t pScanner;
    SmlMapItemPtr_t pMapItem;
    Long_t target = 0, source = 0;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildMapItem");

    if ((pMapItem = (SmlMapItemPtr_t)smlLibMalloc(sizeof(SmlMapItem_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pMapItem, 0, sizeof(SmlMapItem_t));

    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pMapItem);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildMapItem");
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pMapItem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
            /* child tags */
            case TN_TARGET:
                rc = buildTargetOrSource(pDecoder, (VoidPtr_t)&pMapItem->target);
                target++;
                break;
            case TN_SOURCE:
                rc = buildTargetOrSource(pDecoder, (VoidPtr_t)&pMapItem->source);
                source++;
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildMapItem");
        }
        if (rc != SML_ERR_OK) {
            smlFreeMapItemPtr(pMapItem);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeMapItemPtr(pMapItem);
            return rc;
        }
    }

    if ((target == 0) || (source == 0)) {
        smlFreeMapItemPtr(pMapItem);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildMapItem");
    }

    *ppElem = pMapItem;

    return SML_ERR_OK;
}

#endif

Ret_t
buildStatus(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem)
{
    XltDecScannerPtr_t pScanner;
    SmlStatusPtr_t pStatus;
    Ret_t rc;
    Long_t cmd = 0, data = 0, cmdid = 0;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildStatus");

    if ((pStatus = (SmlStatusPtr_t)smlLibMalloc(sizeof(SmlStatus_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pStatus, 0, sizeof(SmlStatus_t));

    /* initialize the element type field */
    pStatus->elementType = SML_PE_STATUS;

    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pStatus);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildStatus");
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pStatus);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCData elements */
            case TN_CMDID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pStatus->cmdID);
                cmdid++;
                break;
            case TN_MSGREF:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pStatus->msgRef);
                break;
            case TN_CMDREF:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pStatus->cmdRef);
                break;
            case TN_CMD:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pStatus->cmd);
                cmd++;
                break;
            case TN_DATA:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pStatus->data);
                data++;
                break;
            case TN_CHAL:
                rc = buildChal(pDecoder, (VoidPtr_t)&pStatus->chal);
                break;
            case TN_CRED:
                rc = buildCred(pDecoder, (VoidPtr_t)&pStatus->cred);
                break;

            /* Lists */
            case TN_ITEM:
                rc = appendItemList(pDecoder, (VoidPtr_t)&pStatus->itemList);
                break;
            case TN_TARGETREF:
                rc = appendTargetRefList(pDecoder, (VoidPtr_t)&pStatus->targetRefList);
                break;
            case TN_SOURCEREF:
                rc = appendSourceRefList(pDecoder, (VoidPtr_t)&pStatus->sourceRefList);
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildStatus");
        }
        if (rc != SML_ERR_OK) {
            smlFreeStatus(pStatus);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeStatus(pStatus);
            return rc;
        }
    }

    if ((cmd == 0) || (data == 0) || (cmdid == 0))
    {
        smlFreeStatus(pStatus);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildStatus");
    }

    *ppElem = pStatus;

    return SML_ERR_OK;
}

#ifdef RESULT_RECEIVE
Ret_t
buildResults(XltDecoderPtr_t pDecoder, VoidPtr_t *ppResults)
{
    XltDecScannerPtr_t pScanner;
    SmlResultsPtr_t pResults;
    Ret_t rc;
    Long_t cmdref = 0, items = 0, cmdid = 0;

    pScanner = pDecoder->scanner;

    if (*ppResults != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildResults");

    if ((pResults = (SmlResultsPtr_t)smlLibMalloc(sizeof(SmlResults_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pResults, 0, sizeof(SmlResults_t));

    /* initialize the element type field */
    pResults->elementType = SML_PE_RESULTS;

    if (IS_EMPTY(pScanner->curtok)) {
        smlLibFree(pResults);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildResults");
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pResults);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {

            /* PCDATA elements */
            case TN_CMDID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pResults->cmdID);
                cmdid++;
                break;
            case TN_MSGREF:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pResults->msgRef);
                break;
            case TN_CMDREF:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pResults->cmdRef);
                cmdref++;
                break;
            case TN_META:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pResults->meta);
                break;
            case TN_TARGETREF:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pResults->targetRef);
                break;
            case TN_SOURCEREF:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pResults->sourceRef);
                break;

                /* Lists */
            case TN_ITEM:
                rc = appendItemList(pDecoder, &pResults->itemList);
                items++;
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildResults");
        }
        if (rc != SML_ERR_OK) {
            smlFreeResults(pResults);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlFreeResults(pResults);
            return rc;
        }
    }

    if ((cmdref == 0) || (items == 0) || (cmdid == 0))
    {
        smlFreeResults(pResults);
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildResults");
    }

    *ppResults = pResults;

    return SML_ERR_OK;
}

#endif



/// @brief can be called to process both forms of an empty tag (<tag></tag> and <tag/>
Ret_t
buildEmptyTag(XltDecoderPtr_t pDecoder)
{
  Ret_t rc;
  // if current tag is empty (that is, ist a start-and-end like <tag/>), that's simply ok
  if (IS_EMPTY(pDecoder->scanner->curtok))
    return SML_ERR_OK;
  // otherwise, we need a next tag...
  if (((rc = nextToken(pDecoder)) != SML_ERR_OK))
    return rc; // no next tag found
  // ...and it must be a closing tag
  if (!IS_END(pDecoder->scanner->curtok))
    return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pDecoder->scanner,"buildEmptyTag");
  // yep, ok
  return SML_ERR_OK;
}


Ret_t
buildPCData(XltDecoderPtr_t pDecoder, VoidPtr_t *ppPCData)
{
    XltDecScannerPtr_t pScanner;
    SmlPcdataPtr_t pPCData = 0;
    SmlPcdataExtension_t ext;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppPCData != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildPCData_1");

    if (IS_EMPTY(pScanner->curtok)) {
        if ((pPCData = (SmlPcdataPtr_t)smlLibMalloc(sizeof(SmlPcdata_t))) == NULL)
            return SML_ERR_NOT_ENOUGH_SPACE;

        smlLibMemset(pPCData, 0, sizeof(SmlPcdata_t));

        *ppPCData = pPCData;
        return SML_ERR_OK;
    }

    pPCData = NULL;

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        if (rc == SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildPCData_2")) { /* leaks if dtd failed */
          pPCData = pScanner->curtok->pcdata;
         *ppPCData = pPCData;
      }

      return rc;
    }

    if (IS_CONTENT(pScanner->curtok)) {
        /* PCData element has a regular string or opaque content */
        while (pScanner->curtok->type == TOK_CONT) {
            if (pPCData == NULL)
                pPCData = pScanner->curtok->pcdata;
            else {
                pPCData = concatPCData(pPCData, pScanner->curtok->pcdata);
                smlLibFree(pScanner->curtok->pcdata->content);
                smlLibFree(pScanner->curtok->pcdata);

                if (pPCData == NULL)
                    return SML_DECODEERROR(SML_ERR_XLT_INVAL_PCDATA,pScanner,"buildPCData_3");
            }

            if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
                *ppPCData = pPCData;
                return rc;
            }
        }
    } else if (IS_START_OR_EMPTY(pScanner->curtok)) {
        /* PCData element contains an XML dokument that is handled by an
           extension mechanism  */
        ext = pScanner->curtok->ext;
        if ((rc = discardToken(pDecoder)) != SML_ERR_OK) return rc;
        if ((pPCData = (SmlPcdataPtr_t)smlLibMalloc(sizeof(SmlPcdata_t))) == NULL)
            return SML_ERR_NOT_ENOUGH_SPACE;
        smlLibMemset(pPCData, 0, sizeof(SmlPcdata_t));
        pPCData->contentType = SML_PCDATA_EXTENSION;
        pPCData->extension = ext;
        switch (ext) {
#ifdef __USE_METINF__
          case SML_EXT_METINF:
            if ((rc = buildMetInfMetInfCmd(pDecoder, (VoidPtr_t)&pPCData->content)) != SML_ERR_OK) {
                smlLibFree(pPCData);
                return rc;
            }
            break;
#endif
#ifdef __USE_DEVINF__
          case SML_EXT_DEVINF:

            if ((rc = buildDevInfDevInfCmd(pDecoder, (VoidPtr_t)&pPCData->content)) != SML_ERR_OK) {

            smlLibFree(pPCData);
              return rc;
            }

            /* the scanner must point to the closing PCDATA tag */
            if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
                smlLibFree(pPCData);
                return rc;
            }
            break;
#endif
          default:
            // %%% luz 2005-11-11 : added fallback to MetInf (workaround for ill-formed xml like in sync4j pda clients)
            // if undefined, try with metinf for ill-formatted xml like from sync4j palm client
            // - act like we had detected a switch in namespace to METINF
            pPCData->contentType = SML_PCDATA_EXTENSION;
            pPCData->extension = SML_EXT_METINF;
            pScanner->curtok->ext = SML_EXT_METINF;
            if ((rc = buildMetInfMetInfCmd(pDecoder, (VoidPtr_t)&pPCData->content)) != SML_ERR_OK) {
              smlLibFree(pPCData);
              return SML_DECODEERROR(SML_ERR_XLT_INVAL_EXT,pScanner,"buildPCData_4"); // if we fail parsing meta here, this is an invalid extension
            }
            break;
        } // switch

    } else if (IS_END(pScanner->curtok)) {
        /* PCData element is empty */
    } else {
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_PCDATA,pScanner,"buildPCData_5");
    }


    if (pScanner->curtok->type != TOK_TAG_END)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_PCDATA,pScanner,"buildPCData_6");

    if (pPCData == NULL) {
        if ((pPCData = (SmlPcdataPtr_t)smlLibMalloc(sizeof(SmlPcdata_t))) == NULL)
            return SML_ERR_NOT_ENOUGH_SPACE;
        smlLibMemset(pPCData, 0, sizeof(SmlPcdata_t));
    }

    *ppPCData = pPCData;

    return SML_ERR_OK;
}

Ret_t
buildPCDataList(XltDecoderPtr_t pDecoder, VoidPtr_t *ppPCData)
{
    SmlPcdataListPtr_t pPCDataList = NULL, pPrev = NULL;

    pPCDataList = (SmlPcdataListPtr_t) *ppPCData;

    /* advance to the end of the list, and create ther an empty list element */
    while (pPCDataList != NULL) {
      pPrev = pPCDataList;
      pPCDataList = pPrev->next;
    }
    if ((pPCDataList = (SmlPcdataListPtr_t)smlLibMalloc(sizeof(SmlPcdataList_t))) == NULL)
          return SML_ERR_NOT_ENOUGH_SPACE;
        smlLibMemset(pPCDataList, 0, sizeof(SmlPcdataList_t));
    if (pPrev != NULL) /* we already had some entries in the list */
      pPrev->next = pPCDataList;
    else /* nope we created a new list */
      *ppPCData = pPCDataList;
    pPCDataList->data = NULL;
    /* at this point pPCDataList should point to an valid list element */
    return buildPCData(pDecoder, (VoidPtr_t)&pPCDataList->data);
}


static Ret_t
appendItemList(XltDecoderPtr_t pDecoder, SmlItemListPtr_t *ppItemList)
{
    SmlItemListPtr_t pNewItemList;
    SmlItemListPtr_t pItemList;
    Ret_t rc;

    pItemList = *ppItemList;
    if (pItemList != NULL)
        while (pItemList->next != NULL)
            pItemList = pItemList->next;

    if ((pNewItemList = (SmlItemListPtr_t)smlLibMalloc(sizeof(SmlItemList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pNewItemList, 0, sizeof(SmlItemList_t));

    if ((rc = buildItem(pDecoder, (VoidPtr_t)&pNewItemList->item)) != SML_ERR_OK) {
        smlLibFree(pNewItemList);
        return rc;
    }

    if (pItemList == NULL)
        *ppItemList = pNewItemList;
    else
        pItemList->next = pNewItemList;

    return SML_ERR_OK;
}

#ifdef SEARCH_RECEIVE
static Ret_t
appendSourceList(XltDecoderPtr_t pDecoder, SmlSourceListPtr_t *ppSourceList)
{
    SmlSourceListPtr_t pNewSourceList;
    SmlSourceListPtr_t pSourceList;
    Ret_t rc;

    pSourceList = *ppSourceList;
    if (pSourceList != NULL)
        while (pSourceList->next != NULL)
            pSourceList = pSourceList->next;

    if ((pNewSourceList = (SmlSourceListPtr_t)smlLibMalloc(sizeof(SmlSourceList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pNewSourceList, 0, sizeof(SmlSourceList_t));

    if ((rc = buildTargetOrSource(pDecoder, (VoidPtr_t)&pNewSourceList->source)) != SML_ERR_OK) {
        smlLibFree(pNewSourceList);
        return rc;
    }

    if (pSourceList == NULL)
        *ppSourceList = pNewSourceList;
    else
        pSourceList->next = pNewSourceList;

    return SML_ERR_OK;
}
#endif /* SEARCH_RECEIVE */

#ifdef MAPITEM_RECEIVE

static Ret_t
appendMapItemList(XltDecoderPtr_t pDecoder, SmlMapItemListPtr_t *ppMapItemList)
{
    SmlMapItemListPtr_t pNewMapItemList;
    SmlMapItemListPtr_t pMapItemList;
    Ret_t rc;

    pMapItemList = *ppMapItemList;
    if (pMapItemList != NULL)
        while (pMapItemList->next != NULL)
            pMapItemList = pMapItemList->next;

    if ((pNewMapItemList = (SmlMapItemListPtr_t)smlLibMalloc(sizeof(SmlMapItemList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pNewMapItemList, 0, sizeof(SmlMapItemList_t));

    if ((rc = buildMapItem(pDecoder, (VoidPtr_t)&pNewMapItemList->mapItem)) != SML_ERR_OK) {
        smlLibFree(pNewMapItemList);
        return rc;
    }

    if (pMapItemList == NULL)
        *ppMapItemList = pNewMapItemList;
    else
        pMapItemList->next = pNewMapItemList;

    return SML_ERR_OK;
}
#endif

static Ret_t
appendTargetRefList(XltDecoderPtr_t pDecoder, SmlTargetRefListPtr_t *ppTargetRefList)
{
    SmlTargetRefListPtr_t pNewTargetRefList;
    SmlTargetRefListPtr_t pTargetRefList;
    Ret_t rc;

    pTargetRefList = *ppTargetRefList;
    if (pTargetRefList != NULL)
        while (pTargetRefList->next != NULL)
            pTargetRefList = pTargetRefList->next;

    if ((pNewTargetRefList = (SmlTargetRefListPtr_t)smlLibMalloc(sizeof(SmlTargetRefList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pNewTargetRefList, 0, sizeof(SmlTargetRefList_t));

    if ((rc = buildPCData(pDecoder, (VoidPtr_t)&pNewTargetRefList->targetRef)) != SML_ERR_OK) {
        smlFreePcdata(pNewTargetRefList->targetRef);
        smlLibFree(pNewTargetRefList);
        return rc;
    }

    if (pTargetRefList == NULL)
        *ppTargetRefList = pNewTargetRefList;
    else
        pTargetRefList->next = pNewTargetRefList;

    return SML_ERR_OK;
}

static Ret_t
appendSourceRefList(XltDecoderPtr_t pDecoder, SmlSourceRefListPtr_t *ppSourceRefList)
{
    SmlSourceRefListPtr_t pNewSourceRefList;
    SmlSourceRefListPtr_t pSourceRefList;
    Ret_t rc;

    pSourceRefList = *ppSourceRefList;
    if (pSourceRefList != NULL)
        while (pSourceRefList->next != NULL)
            pSourceRefList = pSourceRefList->next;

    if ((pNewSourceRefList = (SmlSourceRefListPtr_t)smlLibMalloc(sizeof(SmlSourceRefList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pNewSourceRefList, 0, sizeof(SmlSourceRefList_t));

    if ((rc = buildPCData(pDecoder, (VoidPtr_t)&pNewSourceRefList->sourceRef)) != SML_ERR_OK) {
        smlFreePcdata(pNewSourceRefList->sourceRef);
        smlLibFree(pNewSourceRefList);
        return rc;
    }

    if (pSourceRefList == NULL)
        *ppSourceRefList = pNewSourceRefList;
    else
        pSourceRefList->next = pNewSourceRefList;

    return SML_ERR_OK;
}

