/**
 * @file
 * WBXML decoder
 *
 * @target_system  all
 * @target_os      all
 * @description The WBXML scanner/tokenizer. Used by the SyncML parser.
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
/* Definitions                                                           */
/*************************************************************************/
#include "xltdevinf.h"
#include "xltdeccom.h"
#include "xlttags.h"
#include "xltutilstack.h"
#include "xltdec.h"

#include "smldtd.h"
#include "smldevinfdtd.h"
#include "smlmetinfdtd.h"
#include "mgrutil.h"

#include <libmem.h>
#include <libstr.h>

#include <smlerr.h>

#ifdef IS_END /* to avoid redefinition of this macro */
#undef IS_END
#endif

void
subdtdDecodeWbxml(XltDecoderPtr_t pDecoder,SmlPcdataPtr_t *ppPcdata);


/* WBXML version that this parser knows */
#define _MAJOR_VERSION  1
#define _MINOR_VERSION  2
#define _MAX_PARSER_MINOR_VERSION 3 // luz: WBXML 1.3 seems to work identically in the SyncML context

#define TAG_STATE 0
#define ATTRIBUTE_STATE 1

/* various checks about wbxml token */
#define HAS_ATTRIBUTES(tag) (*tag & 0x80)
#define HAS_CONTENT(tag) (*tag & 0x40)
#define IDENTITY(tag) (*tag & 0x3F)

#define IS_SWITCH(tok)  (*(tok) == 0x00)
#define IS_END(tok)     (*(tok) == 0x01)
#define IS_ENTITY(tok)  (*(tok) == 0x02)
#define IS_STR_I(tok)   (*(tok) == 0x03)
#define IS_LITERAL(tok) (IDENTITY(tok) == 0x04)
// Note: gcc cannot parse multi-line macros when file has DOS line ends
#define IS_EXT_I(tok)  ((*(tok) == 0x40) || (*(tok) == 0x41) || (*(tok) == 0x42))
#define IS_PI(tok)      (*(tok) == 0x43)
#define IS_EXT_T(tok)  ((*(tok) == 0x80) || (*(tok) == 0x81) || (*(tok) == 0x82))
#define IS_STR_T(tok)   (*(tok) == 0x83)
#define IS_EXT(tok)    ((*(tok) == 0xC0) || (*(tok) == 0xC1) || (*(tok) == 0xC2))
#define IS_OPAQUE(tok)  (*(tok) == 0xC3)
#define IS_STRING(tok) (IS_STR_I(tok) || IS_STR_T(tok))
#define IS_EXTENSION(tok) (IS_EXT_I(tok) || IS_EXT_T(tok) || IS_EXT(tok))

#define IS_ATTRIBUTE_VALUE(tok) (*(tok) & 0x80)
#define IS_ATTRIBUTE_START(tok) (~IS_ATTRIBUTE_VALUE(tok))


/** @copydoc wbxmlScannerPriv_s */
typedef struct wbxmlScannerPriv_s wbxmlScannerPriv_t, *wbxmlScannerPrivPtr_t;
/**
 * Private Interface for the WBXML scanner.
 *
 * The private scanner interface contains some additional member attributes
 * that are not listed in the public interface, e.g. a copy of the string
 * table and some other items that do not need to be known outside the
 * scanner module.
 */
struct wbxmlScannerPriv_s
{
    /* public methods */
    Ret_t (*nextTok)(XltDecScannerPtr_t);
    Ret_t (*destroy)(XltDecScannerPtr_t);
    Ret_t (*pushTok)(XltDecScannerPtr_t);
    void (*setBuf)(XltDecScannerPtr_t pScanner, const MemPtr_t pBufStart, const MemPtr_t pBufEnd);
    MemPtr_t (*getPos)(XltDecScannerPtr_t pScanner);

    /* public attributes */
    XltDecTokenPtr_t curtok;       /**< current token */
    Long_t charset;                /**< character set as specified in the
                                      WBXML header */
    String_t charsetStr;           /**< NULL */
    Long_t pubID;                  /**< document public identifier as
                                      specified in the WBXML header */
    String_t pubIDStr;             /**< pubID as a string - valid only when
                                      pubID == 0 */
    Flag_t finished;               /**< set when end of buffer is reached */

    /* private attributes */
    MemPtr_t pos;                  /**< current buffer position */
    MemPtr_t bufend;               /**< end of buffer */
    Long_t pubIDIdx;               /**< strtbl index of the string
                                      version of the pubID - valid only
                                      when pubID == 0 */

    XltUtilStackPtr_t tagstack;    /**< stack of open start tags */

    MemPtr_t strtbl;                /**< copy of the string table */
    Long_t strtbllen;               /**< length of the string table */

    Byte_t state;                   /**< tag state or attribute state */
    SmlPcdataExtension_t cptag;     /**< current codepage for tags */
    Byte_t cpattr;                  /**< current codepage for attributes */
    SmlPcdataExtension_t activeExt; /**< the active Sub DTD */
};

/* typedef for multi-byte unsigned integers as specified in the
   WAP Binary XML Content Format specification */
typedef Long_t MBINT;

/*
 * Public methods of the scanner interface.
 *
 * Description see XLTDecCom.h.
 */
static Ret_t _destroy(XltDecScannerPtr_t);
static Ret_t _nextTok(XltDecScannerPtr_t);
static Ret_t _pushTok(XltDecScannerPtr_t);
static void _setBuf(XltDecScannerPtr_t, const MemPtr_t, const MemPtr_t);
static MemPtr_t _getPos(XltDecScannerPtr_t);

/**
 * Advance the current position pointer after checking whether the end of
 * the buffer has been reached. If the end of the buffer has been reached
 * the scanner's finished flag is set.
 *
 * @return 0, if end of buffer has been reached
 *         1 otherwise
 */
static Boolean_t readBytes(wbxmlScannerPrivPtr_t pScanner, Long_t bytes);

/**
 * Decodes multi-byte integers.
 *
 * @pre pScanner->pos points to the first byte of the mb_int.
 * @post pScanner->pos points to the last byte of the mb_int.
 */
static Ret_t parseInt(wbxmlScannerPrivPtr_t pScanner, MBINT *mbi);

/**
 * wbxmlHeader, wbxmlVersion, wbxmlPublicID, wbxmlCharset
 *
 * These functions are used for decoding the WBXML document header.
 * wbxmlHeader is a short wrapper that calls the other four functions in
 * the right order to scan the header. wbxmlStrtbl makes a copy of the
 * string table.
 */
static Ret_t wbxmlHeader(wbxmlScannerPrivPtr_t pScanner);
static Ret_t wbxmlVersion(wbxmlScannerPrivPtr_t pScanner);
static Ret_t wbxmlPublicID(wbxmlScannerPrivPtr_t pScanner);
static Ret_t wbxmlCharset(wbxmlScannerPrivPtr_t pScanner);
static Ret_t wbxmlStrtbl(wbxmlScannerPrivPtr_t pScanner);

/**
 * Switch WBXML code page
 */
static Ret_t wbxmlSwitchPage(wbxmlScannerPrivPtr_t pScanner);

/**
 * wbxmlXXXToken
 *
 * Scan the document for the next valid XML/WBXML token as defined in the
 * XLTDecCom header file (e.g. TOK_TAG_START).
 *
 * @pre pScanner->pos points to the first byte of a valid WBXML
 *      element (String, Tag, etc.)
 * @post pScanner->pos points to the last byte of the WBXML element;
 *       pScanner->curtok contains type and tagid or pcdata of the token
 */
static Ret_t wbxmlStringToken(wbxmlScannerPrivPtr_t pScanner);
static Ret_t wbxmlOpaqueToken(wbxmlScannerPrivPtr_t pScanner);
static Ret_t wbxmlTagToken(wbxmlScannerPrivPtr_t pScanner);

/**
 * wbxmlSkipXXX
 *
 * WBXML extensions, entities, processing instructions and attributes are
 * not supported by this scanner. If one is found it is skipped and
 * processing continues afterwards.
 */
static Ret_t wbxmlSkipExtension(wbxmlScannerPrivPtr_t pScanner);
static Ret_t wbxmlSkipEntity(wbxmlScannerPrivPtr_t pScanner);
static Ret_t wbxmlSkipPI(wbxmlScannerPrivPtr_t);
static Ret_t wbxmlSkipAttribute(wbxmlScannerPrivPtr_t);

/*************************************************************************/
/* External Functions                                                    */
/*************************************************************************/

/*
 * Create and initialize a new WBXML scanner. Description see XLTDec.h.
 */
Ret_t
xltDecWbxmlInit(const MemPtr_t pBufEnd, MemPtr_t *ppBufPos,
        XltDecScannerPtr_t *ppScanner)
{
    wbxmlScannerPrivPtr_t pScanner;
    Ret_t rc;

    /* initialize new WBXML scanner */
    if ((pScanner = (wbxmlScannerPrivPtr_t)smlLibMalloc(sizeof(wbxmlScannerPriv_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pScanner, 0, sizeof(wbxmlScannerPriv_t));
    pScanner->bufend = pBufEnd;
    pScanner->pos = *ppBufPos;
    if ((pScanner->curtok = (XltDecTokenPtr_t)smlLibMalloc(sizeof(XltDecToken_t))) == NULL) {
        smlLibFree(pScanner);
        *ppScanner = NULL;
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    smlLibMemset(pScanner->curtok, 0, sizeof(XltDecToken_t));
    if ((rc = xltUtilCreateStack(&pScanner->tagstack, 10)) != SML_ERR_OK) {
        smlLibFree(pScanner->curtok);
        smlLibFree(pScanner);
        *ppScanner = NULL;
        return rc;
    }
    pScanner->state = TAG_STATE;

    /* point public/private methods to the right implementation */
    pScanner->nextTok = _nextTok;
    pScanner->destroy = _destroy;
    pScanner->pushTok = _pushTok;
    pScanner->setBuf = _setBuf;
    pScanner->getPos = _getPos;

    /* decode WBXML header */
    if ((rc = wbxmlHeader(pScanner)) != SML_ERR_OK) {
        pScanner->destroy((XltDecScannerPtr_t)pScanner);
        *ppScanner = NULL;
        return rc;
    }

    *ppScanner = (XltDecScannerPtr_t)pScanner;

    return SML_ERR_OK;
}

/**
 * Free memory. Description see XltDecAll.h.
 */
static Ret_t
_destroy(XltDecScannerPtr_t pScanner)
{
    wbxmlScannerPrivPtr_t pScannerPriv;

    if (pScanner == NULL)
        return SML_ERR_OK;

    pScannerPriv = (wbxmlScannerPrivPtr_t)pScanner;
    if (pScannerPriv->tagstack != NULL)
        pScannerPriv->tagstack->destroy(pScannerPriv->tagstack);
    smlLibFree(pScannerPriv->curtok);
    smlLibFree(pScannerPriv->strtbl);
    smlLibFree(pScannerPriv);

    return SML_ERR_OK;
}

/**
 * Get next token.
 */
static Ret_t
_nextTok(XltDecScannerPtr_t pScanner)
{
    wbxmlScannerPrivPtr_t pScannerPriv;
    Ret_t rc;

    pScannerPriv = (wbxmlScannerPrivPtr_t)pScanner;
    // T.K.: chanched Ptr_t to _t
    smlLibMemset(pScanner->curtok, 0, sizeof(XltDecToken_t));
    pScannerPriv->curtok->start = pScannerPriv->pos;

    /* keep going until we find a "supported" element */
    rc = SML_ERR_OK;
    while (rc == SML_ERR_OK) {
        /* skip PIs, extensions and entities... */
        if (IS_PI(pScannerPriv->pos)) {
            rc = wbxmlSkipPI(pScannerPriv);
        } else if (IS_EXTENSION(pScannerPriv->pos)) {
            rc = wbxmlSkipExtension(pScannerPriv);
        } else if (IS_ENTITY(pScannerPriv->pos)) {
            rc = wbxmlSkipEntity(pScannerPriv);

        /* ... decode strings, opaque data and tags */
        } else if (IS_STRING(pScannerPriv->pos)) {
            rc = wbxmlStringToken(pScannerPriv);
            break;
        } else if (IS_OPAQUE(pScannerPriv->pos)) {
            rc = wbxmlOpaqueToken(pScannerPriv);
            break;
        } else {
            rc = wbxmlTagToken(pScannerPriv);
            break;
        }
    }

    return rc;
}

/**
 * Reset the scanner to the starting position of the current token within
 * the buffer.
 */
static Ret_t
_pushTok(XltDecScannerPtr_t pScanner)
{
    wbxmlScannerPrivPtr_t pScannerPriv;
    XltUtilStackPtr_t pTagStack;
    XltTagID_t tagid;
    Ret_t rc = 0;

    pScannerPriv = (wbxmlScannerPrivPtr_t)pScanner;
    pTagStack = pScannerPriv->tagstack;

    if (pScannerPriv->curtok->start == NULL)
        return SML_ERR_WRONG_USAGE;

    /* reset scanner to position where tok begins */
    pScannerPriv->pos = pScannerPriv->curtok->start;

    /* correct the tag stack */
    if (pScannerPriv->curtok->type == TOK_TAG_START) {
        rc = pTagStack->pop(pTagStack, &tagid);
    } else if (pScannerPriv->curtok->type == TOK_TAG_END) {
        tagid = pScannerPriv->curtok->tagid;
        rc = pTagStack->push(pTagStack, tagid);
    }
    if (rc) return rc;

    /* invalidate curtok */
    /* T.K. Possible Error. pScannerPriv->curtok is of type XltDecToken_t NOT ...Ptr_t */
    // OrigLine:
    // smlLibMemset(pScannerPriv->curtok, 0, sizeof(XltDecTokenPtr_t));
    pScannerPriv->curtok->type = (XltTokType_t)0;

    return SML_ERR_OK;
}

static void
_setBuf(XltDecScannerPtr_t pScanner, const MemPtr_t pBufStart,
        const MemPtr_t pBufEnd)
{
    wbxmlScannerPrivPtr_t pScannerPriv = (wbxmlScannerPrivPtr_t)pScanner;
    pScannerPriv->pos = pBufStart;
    pScannerPriv->bufend = pBufEnd;
}

static MemPtr_t
_getPos(XltDecScannerPtr_t pScanner)
{
    return ((wbxmlScannerPrivPtr_t)pScanner)->pos;
}

/*************************************************************************/
/* Internal Functions                                                    */
/*************************************************************************/

/*
 * Advance the position pointer. Description see above.
 */
static Boolean_t
readBytes(wbxmlScannerPrivPtr_t pScanner, Long_t bytes)
{
    if (pScanner->pos + bytes > pScanner->bufend) {
        pScanner->finished = 1;
        return 0;
    }
    pScanner->pos += bytes;
    return 1;
}

/*
 * NOTICE: Entities, Extensions, Processing Instructions and Attributes
 * are not supported by the WBXML scanner.
 *
 * Extensions and Attributes are document-specific and are as such not used
 * by the SyncML specification.
 * The scanner will just ignore and skip over them. Neither
 * this scanner nor the parser use processing instructions so they are
 * skipped as well.
 */

/*
 * Decode the WBXML header containing version number, document public
 * identifier, character set and a string table.
 */
static Ret_t
wbxmlHeader(wbxmlScannerPrivPtr_t pScanner)
{
    Ret_t rc;

    /* decode the WBXML header */
    if ((rc = wbxmlVersion(pScanner)) != SML_ERR_OK)
        return rc;
    if ((rc = wbxmlPublicID(pScanner)) != SML_ERR_OK)
        return rc;
    if ((rc = wbxmlCharset(pScanner)) != SML_ERR_OK)
        return rc;
    if ((rc = wbxmlStrtbl(pScanner)) != SML_ERR_OK)
        return rc;
    return SML_ERR_OK;
}

/**
 * Decode WBXML version. The scanner returns an error if the major version
 * of the document differs from the major version this scanner supports or
 * if the minor version of the document is larger than the minor version
 * the scanner supports.
 */
static Ret_t
wbxmlVersion(wbxmlScannerPrivPtr_t pScanner)
{
    Byte_t major, minor;

    minor = ((Byte_t)(*pScanner->pos & 0x0F));
    major = ((Byte_t)((*pScanner->pos >> 4) + 1));

    if (major != _MAJOR_VERSION || minor > _MAX_PARSER_MINOR_VERSION)
        return SML_DECODEERROR(SML_ERR_XLT_INCOMP_WBXML_VERS,pScanner,"wbxmlVersion");

    if (!readBytes(pScanner, 1))
        return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlVersion");

    return SML_ERR_OK;
}

/**
 * Decodes WBXML Document Public Identifier.
 */
static Ret_t
wbxmlPublicID(wbxmlScannerPrivPtr_t pScanner)
{
    MBINT tmp;
    Ret_t rc;

    if (*pScanner->pos != 0) {
        /* pre-defined numeric identifier */
        if ((rc = parseInt(pScanner, &tmp)) != SML_ERR_OK)
            return rc;
        pScanner->pubID = tmp;
        pScanner->pubIDIdx = 0;
    } else {
        /* public id is given as string table entry (which we
           haven't read at this point so we'll save the reference
           for later) */
        if  (!readBytes(pScanner, 1))
            return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlPublicID");
        if ((rc = parseInt(pScanner, &tmp)) != SML_ERR_OK)
            return rc;
        pScanner->pubID = 0;
        pScanner->pubIDIdx = tmp;
    }
    if (!readBytes(pScanner, 1))
        return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlPublicID");
    return SML_ERR_OK;
}

/**
 * Decode WBXML Charset.
 */
static Ret_t
wbxmlCharset(wbxmlScannerPrivPtr_t pScanner)
{
    /* TODO: if charset iformation has to be processed
       it can be done here. For the moment only UTF-8 is used by SyncML */
    MBINT mibenum;
    Ret_t rc;

    /* charset is given as a single IANA assigned MIBEnum value */
    if ((rc = parseInt(pScanner, &mibenum)) != SML_ERR_OK)
        return rc;
    pScanner->charset = mibenum;

    if (!readBytes(pScanner, 1))
        return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlCharset");

    return SML_ERR_OK;
}

/**
 * Keep a copy of the string table.
 */
static Ret_t
wbxmlStrtbl(wbxmlScannerPrivPtr_t pScanner)
{
    MBINT len;
    Ret_t rc;

    if ((rc = parseInt(pScanner, &len)) != SML_ERR_OK)
        return rc;
    if (!readBytes(pScanner, 1))
        return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlStrtbl");
    pScanner->strtbllen = len;
    if (len > 0) {
        if (pScanner->pos + len > pScanner->bufend)
            return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlStrtbl");
        if ((pScanner->strtbl = smlLibMalloc(len)) == NULL)
        {
            return SML_ERR_NOT_ENOUGH_SPACE;
        }
        smlLibMemcpy(pScanner->strtbl, pScanner->pos, len);
        readBytes(pScanner, len);
    } else {
        pScanner->strtbl = NULL;
    }

    /* if the public ID was given as a string table reference save a
       reference to the corresponding string for later */
    if (pScanner->pubID == 0) {
        if (pScanner->pubIDIdx > pScanner->strtbllen)
            return SML_DECODEERROR(SML_ERR_XLT_INVAL_WBXML_DOC,pScanner,"wbxmlStrtbl");
        pScanner->pubIDStr = (String_t)(pScanner->strtbl + pScanner->pubIDIdx);
    }

    return SML_ERR_OK;
}

static Ret_t
parseInt(wbxmlScannerPrivPtr_t pScanner, MBINT *mbi)
{
    *mbi = 0;
    /* accumulate byte value until continuation flag (MSB) is zero */
    for (;;)  {
        *mbi = *mbi << 7;
        *mbi += *(pScanner->pos) & 0x7F;
        if (!(*pScanner->pos & 0x80)) break;
        if (!readBytes(pScanner, 1))
            return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"parseInt");
    }
    return SML_ERR_OK;
}

static Ret_t
wbxmlStringToken(wbxmlScannerPrivPtr_t pScanner)
{
    SmlPcdataPtr_t pPcdata;
    Ret_t rc;

    if ((pPcdata = (SmlPcdataPtr_t)smlLibMalloc(sizeof(SmlPcdata_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    /* copy the string into the new PCdata struct */
    if (IS_STR_I(pScanner->pos)) {
        /* inline string */
        if (!readBytes(pScanner, 1))
            return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlStringToken");
        pPcdata->extension   = SML_EXT_UNDEFINED;
        pPcdata->contentType = SML_PCDATA_STRING;
        pPcdata->length = smlLibStrlen((String_t)pScanner->pos);
        if (pScanner->pos + pPcdata->length + 1 > pScanner->bufend) {
            smlLibFree(pPcdata);
            return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlStringToken");
        }
        if ((pPcdata->content = smlLibMalloc(pPcdata->length + 1)) == NULL) {
            smlLibFree(pPcdata);
            return SML_ERR_NOT_ENOUGH_SPACE;
        }
        smlLibStrncpy(pPcdata->content, (String_t)pScanner->pos, pPcdata->length + 1);
        readBytes(pScanner, pPcdata->length + 1);

    } else {
        /* string table reference */
        MBINT offset; /* offset into string table */
        if (!readBytes(pScanner, 1))
            return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlStringToken");
        if ((rc = parseInt(pScanner, &offset)) != SML_ERR_OK) {
            smlLibFree(pPcdata);
            return rc;
        }
        if (offset >= pScanner->strtbllen) {
            smlLibFree(pPcdata);
            return SML_DECODEERROR(SML_ERR_XLT_INVAL_WBXML_DOC,pScanner,"wbxmlStringToken");
        }
        pPcdata->contentType = SML_PCDATA_STRING;
        pPcdata->length = smlLibStrlen((String_t)(pScanner->strtbl + offset));
        if ((pPcdata->content = smlLibMalloc(pPcdata->length + 1)) == NULL) {
            smlLibFree(pPcdata);
            return SML_ERR_NOT_ENOUGH_SPACE;
        }
        smlLibStrncpy(pPcdata->content, (String_t)(pScanner->strtbl + offset), pPcdata->length + 1);
        readBytes(pScanner, 1);
    }

    pScanner->curtok->pcdata = pPcdata;

    pScanner->curtok->type = TOK_CONT;

    return SML_ERR_OK;
}

static Ret_t
wbxmlOpaqueToken(wbxmlScannerPrivPtr_t pScanner)
{
    SmlPcdataPtr_t pPcdata = NULL;
    MBINT len;
    Ret_t rc;

    if (!readBytes(pScanner, 1))
        return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlOpaqueToken");

    /* a mbi indicates the length of the opaque data block that we'll
       copy into new PCdata struct */
    if ((rc = parseInt(pScanner, &len)) != SML_ERR_OK)
        return rc;
    if (!readBytes(pScanner, 1))
        return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlOpaqueToken");
    if (pScanner->pos + len > pScanner->bufend)
        return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlOpaqueToken");
    if ((pPcdata = (SmlPcdataPtr_t)smlLibMalloc(sizeof(SmlPcdata_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    pPcdata->extension   = SML_EXT_UNDEFINED;
    pPcdata->contentType = SML_PCDATA_OPAQUE;
    pPcdata->length = len;
    /* Modification 2001-07-03 by Luz %%%%%:
     * made sure that content is one null byte longer
     * than indicated opaque content, such that strings that are coded as
     * opaque (happens to be the case with Nokia 9210) can still be read
     * as C-string without need for an intermediate buffer
     */
    /* original:
    if ((pPcdata->content = smlLibMalloc(len)) == NULL) {
        smlLibFree(pPcdata);
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    */
    /* modified: */
    if ((pPcdata->content = smlLibMalloc(len+1)) == NULL) {
        smlLibFree(pPcdata);
        return SML_ERR_NOT_ENOUGH_SPACE;
    }
    ((char *)pPcdata->content)[len]=0; /* make sure there is a c-string terminator */
    /* end modification */

    smlLibMemcpy(pPcdata->content, pScanner->pos, len);
    pScanner->curtok->pcdata = pPcdata;

    readBytes(pScanner, len);

    pScanner->curtok->type = TOK_CONT;

    return SML_ERR_OK;
}

static Ret_t
wbxmlTagToken(wbxmlScannerPrivPtr_t pScanner)
{
    XltTagID_t tagid;
    Boolean_t has_cont, has_attr;
    Ret_t rc;

    if (IS_SWITCH(pScanner->pos)) {
        if ((rc = wbxmlSwitchPage(pScanner)) != SML_ERR_OK)
            return rc;
    }

    /* we have to look at the top of the tagstack to see which
       start tag an end tag belongs to */
    if (IS_END(pScanner->pos)) {
        if (!readBytes(pScanner, 1))
            return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlTagToken");
        pScanner->curtok->type = TOK_TAG_END;
        rc = pScanner->tagstack->pop(pScanner->tagstack, &tagid);
        if (rc == SML_ERR_WRONG_USAGE)
            return SML_DECODEERROR(SML_ERR_XLT_INVAL_WBXML_DOC,pScanner,"wbxmlTagToken");
        else if (rc)
            return rc;
        pScanner->curtok->tagid = tagid;
        return SML_ERR_OK;
    }


    /* look at the two MSB: does this tag have content or attributes? */

     has_cont = ((Boolean_t)(HAS_CONTENT(pScanner->pos)));
     has_attr = ((Boolean_t)(HAS_ATTRIBUTES(pScanner->pos)));


    /* look up tag ID either by string or by number */
    if (IS_LITERAL(pScanner->pos)) {
        MBINT offset; /* offset into the string table */
        if (!readBytes(pScanner, 1))
            return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlTagToken");
        if ((rc = parseInt(pScanner, &offset)) != SML_ERR_OK)
            return rc;
        if (offset > pScanner->strtbllen)
            return SML_DECODEERROR(SML_ERR_XLT_INVAL_WBXML_DOC,pScanner,"wbxmlTagToken");

        rc = (Ret_t)getTagIDByStringAndExt((String_t)(pScanner->strtbl + offset), pScanner->activeExt, &tagid);
        if ((tagid == TN_UNDEF) || (rc != SML_ERR_OK)) return rc;

    } else {
        rc = (Ret_t)getTagIDByByteAndExt((Byte_t)IDENTITY(pScanner->pos), pScanner->activeExt, &tagid);
        if ((tagid == TN_UNDEF) || (rc != SML_ERR_OK)) return rc;

    }

    /* we know everything we need to know */
    pScanner->curtok->tagid = tagid;
    pScanner->curtok->type = has_cont ? TOK_TAG_START : TOK_TAG_EMPTY;
#ifdef __USE_METINF__
    pScanner->curtok->ext = pScanner->cptag == 0 ? SML_EXT_UNDEFINED : SML_EXT_METINF;
#else
  pScanner->curtok->ext = SML_EXT_UNDEFINED;
#endif

    if (!readBytes(pScanner, 1))
        return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlTagToken");

    /* push tag onto tagstack unless this tag is empty */
    if (has_cont) {
        if ((rc = pScanner->tagstack->push(pScanner->tagstack, tagid)) != SML_ERR_OK)
            return rc;
    }

    /* skip attributes */
    if (has_attr) {
        pScanner->state = ATTRIBUTE_STATE;
        if ((rc = wbxmlSkipAttribute(pScanner)) != SML_ERR_OK)
            return rc;
        pScanner->state = TAG_STATE;
    }

    return SML_ERR_OK;
}

/*
 * Switch WBXML code page.
 */
/* T.K. 06.02.01
 * We need to enhance this as soon as we introduce
 * Sub DTD's with more than one WBXML codepage. But till then
 * there is only one case where WBXML codepages can occure, and
 * this is the MetInf Sub DTD. So in case we find a codepage switch
 * to something other than codepage zero, we set the active extension
 * to metinf.
 * In future versions the pScanner needs to be enhanced, to translate
 * codepageswitches context sensitive to the active extension.
 */
static Ret_t
wbxmlSwitchPage(wbxmlScannerPrivPtr_t pScanner)
{
    if (!readBytes(pScanner, 1))
        return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlSwitchPage");
    if (pScanner->state == TAG_STATE)
        pScanner->cptag = (SmlPcdataExtension_t)*pScanner->pos;
    else
        pScanner->cpattr = *pScanner->pos;
    readBytes(pScanner, 1);
    /* T.K. this needs to be adjusted as described above */
    if (pScanner->cpattr != 0 || pScanner->cptag != 0)
        pScanner->activeExt = SML_EXT_METINF;
    else
        pScanner->activeExt = SML_EXT_UNDEFINED;
    return SML_ERR_OK;
}


/******************************/
/* Unsupported WBXML elements */
/******************************/

/**
 * Skips entities but doesn't do anything useful yet.
 */
static Ret_t
wbxmlSkipEntity(wbxmlScannerPrivPtr_t pScanner)
{
    MBINT tmp;
    Ret_t rc;

    if (!readBytes(pScanner, 1))
        return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlSkipEntity");
    if ((rc = parseInt(pScanner, &tmp)) != SML_ERR_OK)
        return rc;
    if (!readBytes(pScanner, 1))
        return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlSkipEntity");

    return SML_ERR_OK;
}


/*
 * Decode WBXML extensions. Skips the extension but doesn't do anything
 * useful with it.
 */
static Ret_t
wbxmlSkipExtension(wbxmlScannerPrivPtr_t pScanner)
{
    MBINT tmp;
    Ret_t rc;

    if (IS_EXT(pScanner->pos)) {
        /* single byte extension token */
        if (!readBytes(pScanner, 1))
          return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlSkipExtension");
    } else if (IS_EXT_I(pScanner->pos)) {
        /* inline string extension token */
        if (!readBytes(pScanner, 1))
            return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlSkipExtension");
        if (!readBytes(pScanner, smlLibStrlen((String_t)pScanner->pos) + 1))
            return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlSkipExtension");
    } else {
        /* inline integer extension token */
        if (!readBytes(pScanner, 1))
            return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlSkipExtension");
        if ((rc = parseInt(pScanner, &tmp)) != SML_ERR_OK)
            return rc;
        if (!readBytes(pScanner, tmp + 1))
            return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlSkipExtension");
    }
    return SML_ERR_OK;
}

/*
 * Handle XML processing instructions. PIs are not supported but the
 * scanner recognizes and skips over them.
 */
static Ret_t
wbxmlSkipPI(wbxmlScannerPrivPtr_t pScanner)
{
    /* PIs are just like tag attributes with a special PI token instead
     * of the attribute start token */
    return wbxmlSkipAttribute(pScanner);
}

/*
 * Handle attributes. Attributes are not supported but the
 * scanner recognizes and skips over them.
 */
static Ret_t
wbxmlSkipAttribute(wbxmlScannerPrivPtr_t pScanner)
{
    XltDecTokenPtr_t oldtok;
    MBINT tmp;
    Ret_t rc = 0;

    /* skipping attributes shouldn't change the current token so we
       make a copy... */
    if ((oldtok = (XltDecTokenPtr_t)smlLibMalloc(sizeof(XltDecToken_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemcpy(oldtok, pScanner->curtok, sizeof(XltDecToken_t));

    /* ... skip until attribute end tag... */
    while (!IS_END(pScanner->pos)) {
        if (IS_STRING(pScanner->pos)) {
            rc = wbxmlStringToken(pScanner);
            /* avoid memory leak due to this ugly workaround of
               skipping attributes */
            smlLibFree(pScanner->curtok->pcdata);
        } else if (IS_EXTENSION(pScanner->pos)) {
            rc = wbxmlSkipExtension(pScanner);
        } else if (IS_ENTITY(pScanner->pos)) {
            rc = wbxmlSkipEntity(pScanner);
        } else if (IS_OPAQUE(pScanner->pos)) {
            rc = wbxmlOpaqueToken(pScanner);
            /* avoid memory leak due to this ugly workaround of
               skipping attributes */
            smlLibFree(pScanner->curtok->pcdata);
        } else if (IS_LITERAL(pScanner->pos)) {
            if (!readBytes(pScanner, 1))
                return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlSkipAttribute");
            rc = parseInt(pScanner, &tmp);
            if (!readBytes(pScanner, 1))
                return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlSkipAttribute");
        } else if (IS_SWITCH(pScanner->pos)) {
            rc = wbxmlSwitchPage(pScanner);
        } else {
            if (!readBytes(pScanner, 1))
                return SML_DECODEERROR(SML_ERR_XLT_END_OF_BUFFER,pScanner,"wbxmlSkipAttribute");
        }

        if (rc != SML_ERR_OK) {
            smlLibFree(oldtok);
            return rc;
        }

    }
    /* ... then skip the end tag itself... */
    readBytes(pScanner, 1);

    /* ... and finaly restore our copy of curtok */
    smlLibMemcpy(pScanner->curtok, oldtok, sizeof(XltDecToken_t));
    smlLibFree(oldtok);

    return SML_ERR_OK;
}

#ifdef __USE_EXTENSIONS__
/*
 * This function tries to decode an inlined WBXML document inside
 * an PCDATA element.
 * In case of failing to decode it the PCDATA element isn't changed
 * at all.
 */

void
subdtdDecodeWbxml(XltDecoderPtr_t pDecoder,SmlPcdataPtr_t *ppPcdata) {
    Ret_t                 _err         = SML_ERR_OK;
    MemPtr_t              pSubBuf      = NULL;
    SmlPcdataPtr_t        pSubPcdata   = NULL;
    XltDecoderPtr_t       pSubDecoder  = NULL;
#ifdef __USE_DEVINF__
    wbxmlScannerPrivPtr_t pScannerPriv = NULL;
#endif

    /* some sanity checks at first */

    if (*ppPcdata == NULL) {
      if (pDecoder) /* use this rare case to remove warning */
      {
      }
      return;
    }

    if ((*ppPcdata)->contentType != SML_PCDATA_OPAQUE) return;

    // now create a sub buffer
    pSubBuf = (MemPtr_t)smlLibMalloc((*ppPcdata)->length);
    if (pSubBuf == NULL) return;
    smlLibMemset(pSubBuf, 0x00, (*ppPcdata)->length);
    smlLibMemmove(pSubBuf, (*ppPcdata)->content, (*ppPcdata)->length);

    /* ok looks fine sofar - now lets decode the rest */
    /* now lets create a decoder, but without parsing the SyncML
     * start tags (because it's not there) and skip the XML
     * part as we don't need it.
     */
    pSubDecoder = (XltDecoderPtr_t)smlLibMalloc(sizeof(XltDecoder_t));
    if (pSubDecoder == NULL) {
        smlLibFree(pSubBuf);
        return;
    }
    pSubDecoder->finished = 0;
    pSubDecoder->final    = 0;
    pSubDecoder->scanner  = NULL;
    if (xltUtilCreateStack(&pSubDecoder->tagstack, 10) != SML_ERR_OK) {
        smlLibFree(pSubDecoder);
        smlLibFree(pSubBuf);
        return;
    }
    if (xltDecWbxmlInit(pSubBuf+(*ppPcdata)->length,&pSubBuf, &pSubDecoder->scanner) != SML_ERR_OK) {
        xltDecTerminate(pSubDecoder);
        smlLibFree(pSubBuf);
        return;
    }
    pSubDecoder->charset    = pSubDecoder->scanner->charset;
    pSubDecoder->charsetStr = NULL;

    pSubPcdata = (SmlPcdataPtr_t)smlLibMalloc(sizeof(SmlPcdata_t));
    if (pSubPcdata == NULL) {
        xltDecTerminate(pSubDecoder);
        smlLibFree(pSubPcdata);
        smlLibFree(pSubBuf);
        return;
    }
    /* T.K.
     * In the future we need to check the WBXML stringtable and
     * switch into the right Sub DTD. But sofar only DevInf is
     * supported so we can save time and space
     */
    /* T.K.
     * To prevent buffer corruption when __USE_DEVINF__ is not used
     * we initialize _err with any errorcode != OK, and this way
     * force the function to exit without modifying the ppPcdata
     */
    _err = SML_ERR_UNSPECIFIC;
#ifdef __USE_DEVINF__
    pSubPcdata->contentType   = SML_PCDATA_EXTENSION;
    pSubPcdata->extension     = SML_EXT_DEVINF;
    pSubPcdata->length        = 0;
    pSubPcdata->content       = NULL;

    pScannerPriv = (wbxmlScannerPrivPtr_t)pSubDecoder->scanner;
    pScannerPriv->activeExt  = SML_EXT_DEVINF;
    pScannerPriv->cpattr     = 0;
    pScannerPriv->cptag      = (SmlPcdataExtension_t)0;
    smlLibMemset(pScannerPriv->curtok, 0,sizeof(XltDecToken_t));

    _err = buildDevInfDevInfCmd(pSubDecoder, (VoidPtr_t)&pSubPcdata->content);
#endif

    if (_err != SML_ERR_OK) {
        xltDecTerminate(pSubDecoder);
        smlLibFree(pSubPcdata);
        smlLibFree(pSubBuf);
        return;
    }

    /* parsing is done, now lets anchor it within the original PCDATA element */
    smlFreePcdata(*ppPcdata);
    *ppPcdata = pSubPcdata;

    /* we are done */
    xltDecTerminate(pSubDecoder);
    smlLibFree(pSubBuf);

    return;
}

#endif
#endif
