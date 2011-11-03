/**
 * @file
 * DeviceInf DTD related functions for the en-/decoder
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
#ifdef __USE_DEVINF__

#include "smldevinfdtd.h"
#include "smlmetinfdtd.h"
#include "xlttags.h"
#include "xltdevinf.h"
#include "xlttagtbl.h"
#include "xltenc.h"
#include "xltencwbxml.h"

#include <libstr.h>
#include <smlerr.h>
#include <smldtd.h>
#include <libmem.h>
#include <libutil.h>
#include <mgrutil.h>

/* decoder callbacks */
Ret_t buildDevInfDevInfContent(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem);
Ret_t buildDevInfCtcap(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem, Boolean_t datastoreLocal);


Ret_t buildDevInfDevInfCmd(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfDevInfPtr_t pElem = NULL;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfDevInfCmd");

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pElem);
        return rc;
    }

    switch (pScanner->curtok->tagid) {
        case TN_DEVINF_DEVINF:
            rc = buildDevInfDevInfContent(pDecoder, (VoidPtr_t)&pElem);
            break;
        default:
            rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfDevInfCmd");
    }
    if (rc != SML_ERR_OK) {
        smlLibFree(pElem);
        return rc;
    }
    *ppElem = pElem;

    return SML_ERR_OK;
}

Ret_t buildDevInfDevInfContent(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfDevInfPtr_t pElem;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfDevInfContent");

    if ((pElem = (SmlDevInfDevInfPtr_t)smlLibMalloc(sizeof(SmlDevInfDevInf_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pElem, 0, sizeof(SmlDevInfDevInf_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pElem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
            case TN_DEVINF_VERDTD:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->verdtd);
                break;
            case TN_DEVINF_MAN:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->man);
                break;
            case TN_DEVINF_MOD:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->mod);
                break;
            case TN_DEVINF_OEM:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->oem);
                break;
            case TN_DEVINF_FWV:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->fwv);
                break;
            case TN_DEVINF_SWV:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->swv);
                break;
            case TN_DEVINF_HWV:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->hwv);
                break;
            case TN_DEVINF_DEVID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->devid);
                break;
            case TN_DEVINF_DEVTYP:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->devtyp);
                break;
            case TN_DEVINF_DATASTORE:
                rc = buildDevInfDataStoreList(pDecoder, (VoidPtr_t)&pElem->datastore);
                break;
            case TN_DEVINF_CTCAP:
                rc = buildDevInfCtcap(pDecoder, (VoidPtr_t)&pElem->ctcap,FALSE);  // CTCap which is global in devInf itself (pre-DS 1.2)
                break;
            case TN_DEVINF_EXT:
                rc = buildDevInfExtList(pDecoder, (VoidPtr_t)&pElem->ext);
                break;
            /* SCTSTK - 18/03/2002 S.H. 2002-04-05 : SyncML 1.1 */
            case TN_DEVINF_UTC:
                pElem->flags |= SmlDevInfUTC_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;
            case TN_DEVINF_NOFM:
                pElem->flags |= SmlDevInfNOfM_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;
            case TN_DEVINF_LARGEOBJECT:
                pElem->flags |= SmlDevInfLargeObject_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;
            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfDevInfContent");
        }
        if (rc != SML_ERR_OK) {
            smlLibFree(pElem);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlLibFree(pElem);
            return rc;
        }
    }
    *ppElem = pElem;

    return SML_ERR_OK;
}


static Ret_t buildDevInfFilterCapList(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem)
{
  SmlDevInfFilterCapListPtr_t  *ppListElem   = NULL;
  SmlDevInfFilterCapListPtr_t  pFiltercap    = NULL;
  SmlPcdataListPtr_t           *ppList       = NULL;
  SmlPcdataListPtr_t           pList         = NULL;
  XltDecScannerPtr_t           pScanner      = pDecoder->scanner;

  Ret_t rc;

  // if empty, that's ok and treated as NOP:  <FilterCap/>
  if (IS_EMPTY(pScanner->curtok)) {
    return SML_ERR_OK;
  }
  // get next token
  if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
    return rc;
  }
  // get pointer to where we'd insert the next list element
  ppListElem   = (SmlDevInfFilterCapListPtr_t *) ppElem;
  while (*ppListElem!=NULL) {
    ppListElem = &((*ppListElem)->next);
  }
  // create new list element now
  pFiltercap = (SmlDevInfFilterCapListPtr_t)smlLibMalloc(sizeof(SmlDevInfFilterCapList_t));
  if (pFiltercap == NULL) return SML_ERR_NOT_ENOUGH_SPACE;
  smlLibMemset(pFiltercap, 0, sizeof(SmlDevInfFilterCapList_t));
  pFiltercap->data = (SmlDevInfFilterCapPtr_t)smlLibMalloc(sizeof(SmlDevInfFilterCap_t));
  if (pFiltercap->data == NULL) {
    smlFreeDevInfFilterCapList(pFiltercap);
    return SML_ERR_NOT_ENOUGH_SPACE;
  }
  smlLibMemset(pFiltercap->data, 0, sizeof(SmlDevInfFilterCap_t));
  // scan until we hit end of enclosure (</FilterCap>)
  while (pScanner->curtok->type != TOK_TAG_END) {
    // process tags
    switch (pScanner->curtok->tagid) {
      /* PCDATA elements */
      case TN_DEVINF_CTTYPE:
        rc = buildPCData(pDecoder, (VoidPtr_t)&pFiltercap->data->cttype);
        break;
      case TN_DEVINF_VERCT:
        rc = buildPCData(pDecoder, (VoidPtr_t)&pFiltercap->data->verct);
        break;
      case TN_DEVINF_FILTERKEYWORD:
        ppList = &pFiltercap->data->filterkeyword;
        goto keywordOrPropname;
      case TN_DEVINF_PROPNAME:
        ppList = &pFiltercap->data->propname;
        goto keywordOrPropname;
      keywordOrPropname:
        // find last existing valenum
        while (*ppList!=NULL)
          ppList=&((*ppList)->next);
        // create new list element
        pList = (SmlPcdataListPtr_t)smlLibMalloc(sizeof(SmlPcdataList_t));
        if (pList == NULL) return SML_ERR_NOT_ENOUGH_SPACE;
        smlLibMemset(pList, 0, sizeof(SmlPcdataList_t));
        // insert contents
        rc = buildPCData(pDecoder, (VoidPtr_t)&pList->data);
        if (rc==SML_ERR_OK) {
          // ok, add element to list
          *ppList = pList;
          ppList = &(pList->next);
        }
        else
          smlFreePcdataList(pList); // invalid, get rid of it
        break;
      default:
        rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfFilterCapList");
    }
    if (rc != SML_ERR_OK) {
      smlFreeDevInfFilterCapList(pFiltercap);
      return rc;
    }
    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
      smlFreeDevInfFilterCapList(pFiltercap);
      return rc;
    }
  } /* while we do not see an end tag */
  // link into list
  *ppListElem = pFiltercap;
  // set elem link to where we must link in next element
  ppListElem = &(pFiltercap->next);
  // done ok
  return SML_ERR_OK;
}


Ret_t buildDevInfDataStoreCmd(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfDatastorePtr_t pElem;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfDataStoreCmd");

    if ((pElem = (SmlDevInfDatastorePtr_t)smlLibMalloc(sizeof(SmlDevInfDatastore_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pElem, 0, sizeof(SmlDevInfDatastore_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pElem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
            /* PCDATA elements */
            case TN_DEVINF_SOURCEREF:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->sourceref);
                break;
            case TN_DEVINF_DISPLAYNAME:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->displayname);
                break;
            case TN_DEVINF_MAXGUIDSIZE:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->maxguidsize);
                break;
            /* other elements */
            case TN_DEVINF_RXPREF:
                if (pElem->rxpref!=NULL) {
                  // we already have a RxPref - simply ignore further ones
                  // (in DS 1.2, some implementations send an other rx-pref for the folder object apart from the main type)
                  SmlDevInfXmitPtr_t dummyP = NULL;
                  rc = buildDevInfXmitCmd(pDecoder, (VoidPtr_t)&dummyP);
                  smlFreeDevInfXmit(dummyP); // simply discard after parsing
                }
                else
                  rc = buildDevInfXmitCmd(pDecoder, (VoidPtr_t)&pElem->rxpref);
                break;
            case TN_DEVINF_TXPREF:
                if (pElem->txpref!=NULL) {
                  // we already have a txpref - simply ignore further ones
                  // (in DS 1.2, some implementations send an other tx-pref for the folder object apart from the main type)
                  SmlDevInfXmitPtr_t dummyP = NULL;
                  rc = buildDevInfXmitCmd(pDecoder, (VoidPtr_t)&dummyP);
                  smlFreeDevInfXmit(dummyP); // simply discard after parsing
                }
                else
                  rc = buildDevInfXmitCmd(pDecoder, (VoidPtr_t)&pElem->txpref);
                break;
            case TN_DEVINF_RX:
                rc = buildDevInfXmitList(pDecoder, (VoidPtr_t)&pElem->rx);
                break;
            case TN_DEVINF_TX:
                rc = buildDevInfXmitList(pDecoder, (VoidPtr_t)&pElem->tx);
                break;
            case TN_DEVINF_CTCAP:
                rc = buildDevInfCtcap(pDecoder, (VoidPtr_t)&pElem->ctcap,TRUE); // CTCap which is local to datastore (DS 1.2 style)
                break;
            case TN_DEVINF_DSMEM:
                rc = buildDevInfDSMemCmd(pDecoder, (VoidPtr_t)&pElem->dsmem);
                break;
            case TN_DEVINF_SYNCCAP:
                rc = buildDevInfSyncCapCmd(pDecoder, (VoidPtr_t)&pElem->synccap);
                break;



            /* SyncML DS 1.2, Synthesis/luz 2005-08-25 */
            case TN_DEVINF_FILTERCAP:
                rc = buildDevInfFilterCapList(pDecoder, (VoidPtr_t)&pElem->filtercap);
                break;
            case TN_DEVINF_FILTERRX:
                rc = buildDevInfXmitList(pDecoder, (VoidPtr_t)&pElem->filterrx);
                break;
            case TN_DEVINF_HIERARCHICAL:
                pElem->flags |= SmlDevInfHierarchical_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;
            case TN_DEVINF_FIELDLEVEL:
                pElem->flags |= SmlDevInfFieldLevel_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;

            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfDataStoreCmd");
        }
        if (rc != SML_ERR_OK) {
            smlLibFree(pElem);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlLibFree(pElem);
            return rc;
        }
    }
    *ppElem = pElem;

    return SML_ERR_OK;
}

Ret_t buildDevInfXmitCmd(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfXmitPtr_t pXmit;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfXmitCmd");

    if ((pXmit = (SmlDevInfXmitPtr_t)smlLibMalloc(sizeof(SmlDevInfXmit_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pXmit, 0, sizeof(SmlDevInfXmit_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pXmit;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pXmit);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
            /* PCDATA elements */
            case TN_DEVINF_CTTYPE:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pXmit->cttype);
                break;
            case TN_DEVINF_VERCT:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pXmit->verct);
                break;
            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfXmitCmd");
        }
        if (rc != SML_ERR_OK) {
            smlLibFree(pXmit);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlLibFree(pXmit);
            return rc;
        }
    }
    *ppElem = pXmit;

    return SML_ERR_OK;
}

Ret_t buildDevInfXmitList(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    SmlDevInfXmitListPtr_t pElem = NULL, pPrev = NULL;

    pElem = (SmlDevInfXmitListPtr_t) *ppElem;

    /* advance to the end of the list, and create ther an empty list element */
    while (pElem != NULL) {
      pPrev = pElem;
      pElem = pPrev->next;
    }
    if ((pElem = (SmlDevInfXmitListPtr_t)smlLibMalloc(sizeof(SmlDevInfXmitList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pElem, 0, sizeof(SmlDevInfXmitList_t));
    if (pPrev != NULL) /* we already had some entries in the list */
      pPrev->next = pElem;
    else /* nope we created a new list */
      *ppElem = pElem;
    pElem->data = NULL;
    /* at this point pElem should point to an valid list element */
    return buildDevInfXmitCmd(pDecoder, (VoidPtr_t)&pElem->data);
}

Ret_t buildDevInfDataStoreList(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    SmlDevInfDatastoreListPtr_t pElem = NULL, pPrev = NULL;

    pElem = (SmlDevInfDatastoreListPtr_t) *ppElem;

    /* advance to the end of the list, and create ther an empty list element */
    while (pElem != NULL) {
      pPrev = pElem;
      pElem = pPrev->next;
    }
    if ((pElem = (SmlDevInfDatastoreListPtr_t)smlLibMalloc(sizeof(SmlDevInfDatastoreList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pElem, 0, sizeof(SmlDevInfDatastoreList_t));
    if (pPrev != NULL) /* we already had some entries in the list */
      pPrev->next = pElem;
    else /* nope we created a new list */
      *ppElem = pElem;
    pElem->data = NULL;
    /* at this point pElem should point to an valid list element */
    return buildDevInfDataStoreCmd(pDecoder, (VoidPtr_t)&pElem->data);
}

Ret_t buildDevInfExtList(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    SmlDevInfExtListPtr_t pElem = NULL, pPrev = NULL;

    pElem = (SmlDevInfExtListPtr_t) *ppElem;

    /* advance to the end of the list, and create ther an empty list element */
    while (pElem != NULL) {
      pPrev = pElem;
      pElem = pPrev->next;
    }
    if ((pElem = (SmlDevInfExtListPtr_t)smlLibMalloc(sizeof(SmlDevInfExtList_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pElem, 0, sizeof(SmlDevInfExtList_t));
    if (pPrev != NULL) /* we already had some entries in the list */
      pPrev->next = pElem;
    else /* nope we created a new list */
      *ppElem = pElem;
    pElem->data = NULL;
    /* at this point pElem should point to an valid list element */
    return buildDevInfExtCmd(pDecoder, (VoidPtr_t)&pElem->data);
}


/// @brief scans common elements of Properties and PropParams
/// @return SML_ERR_XLT_INVAL_PROTO_ELEM if unknown tag is encountered
static Ret_t scanPropOrParamElement(XltDecoderPtr_t pDecoder, SmlDevInfCTDataPtr_t pPropOrParam)
{
  SmlPcdataListPtr_t *ppValenums = &(pPropOrParam->valenum);
  SmlPcdataListPtr_t pValenum;
  XltDecScannerPtr_t pScanner = pDecoder->scanner;
  Ret_t rc;

  // process tags
  switch (pScanner->curtok->tagid) {
    case TN_DEVINF_DISPLAYNAME:
      rc = buildPCData(pDecoder, (VoidPtr_t)&pPropOrParam->dname);
      break;
    case TN_DEVINF_DATATYPE:
      rc = buildPCData(pDecoder, (VoidPtr_t)&pPropOrParam->datatype);
      break;
    case TN_DEVINF_MAXSIZE:
    case TN_DEVINF_SIZE:
      rc = buildPCData(pDecoder, (VoidPtr_t)&pPropOrParam->maxsize);
      break;
    case TN_DEVINF_MAXOCCUR:
      rc = buildPCData(pDecoder, (VoidPtr_t)&pPropOrParam->maxoccur);
      break;
    /* Flags */
    case TN_DEVINF_NOTRUNCATE:
      pPropOrParam->flags |= SmlDevInfNoTruncate_f;
      rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
      break;
    /* Valenum* */
    case TN_DEVINF_VALENUM:
      // find last existing valenum
      while (*ppValenums!=NULL)
        ppValenums=&((*ppValenums)->next);
      // create new list element
      pValenum = (SmlPcdataListPtr_t)smlLibMalloc(sizeof(SmlPcdataList_t));
      if (pValenum == NULL) return SML_ERR_NOT_ENOUGH_SPACE;
      smlLibMemset(pValenum, 0, sizeof(SmlPcdataList_t));
      // insert contents
      rc = buildPCData(pDecoder, (VoidPtr_t)&pValenum->data);
      if (rc==SML_ERR_OK) {
        // ok, add element to list
        *ppValenums = pValenum;
        ppValenums = &(pValenum->next);
      }
      else
        smlFreePcdataList(pValenum); // invalid, get rid of it
      break;
    default:
      rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_PROTO_ELEM,pScanner,"scanPropOrParamElement");
  }
  return rc;
}


// @brief scan a single <PropParam></PropParam> bracket, or a <1.2 type list where a new <PropName> implies end of current PropParam and start of a new one
static Ret_t buildDevInfPropParam(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem, int datastoreLocal) {
    SmlDevInfCTDataListPtr_t      *ppListElem   = NULL;
    SmlDevInfCTDataListPtr_t      pParam        = NULL;
    Boolean_t                     newElement    = FALSE;
    XltDecScannerPtr_t            pScanner      = pDecoder->scanner;
    Ret_t rc;

    if (IS_EMPTY(pScanner->curtok)) {
        return SML_ERR_OK;
    }

    if (datastoreLocal) {
      // DS 1.2: we have no initial tag to process right away, get next tag now
      if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        return rc;
      }
      newElement=TRUE; // new <property> has already started here
    }

    // get pointer to where we'd insert the next list element
    ppListElem   = (SmlDevInfCTDataListPtr_t *)ppElem;
    while (*ppListElem!=NULL) {
      ppListElem = &((*ppListElem)->next);
    }
    pParam=NULL;
    // now ppListElem points to a SmlDevInfCTDataListPtr_t which is NULL and
    // can be overwritten with the link to a new element

    // scan until we hit end of enclosure (</PropParam>) or an unknown other tag
    while (pScanner->curtok->type != TOK_TAG_END) {
      if (!datastoreLocal && pScanner->curtok->tagid==TN_DEVINF_PARAMNAME) {
        // DS 1.1-style CTCap, seeing <ParamName> implies new element
        newElement=TRUE;
      }
      // create new element if needed
      if (newElement) {
        newElement=FALSE;
        pParam = (SmlDevInfCTDataListPtr_t)smlLibMalloc(sizeof(SmlDevInfCTDataList_t));
        if (pParam == NULL) return SML_ERR_NOT_ENOUGH_SPACE;
        smlLibMemset(pParam, 0, sizeof(SmlDevInfCTDataList_t));
        pParam->data = (SmlDevInfCTDataPtr_t)smlLibMalloc(sizeof(SmlDevInfCTData_t));
        if (pParam->data == NULL) {
          smlFreeDevInfCTDataList(pParam);
          return SML_ERR_NOT_ENOUGH_SPACE;
        }
        smlLibMemset(pParam->data, 0, sizeof(SmlDevInfCTData_t));
        // link into list
        *ppListElem = pParam;
        // set elem link to where we must link in next element
        ppListElem = &(pParam->next);
      }
      // now, if we don't have a pParam, this is an invalid <DS 1.2 devinf
      if (pParam==NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfPropParam");

      // process tags
      switch (pScanner->curtok->tagid) {
        /* PCDATA */
        case TN_DEVINF_PARAMNAME:
          rc = buildPCData(pDecoder, (VoidPtr_t)&pParam->data->name);
          break;
        default:
          rc = scanPropOrParamElement(pDecoder, pParam->data);
          if (rc==SML_ERR_XLT_INVAL_PROTO_ELEM && !datastoreLocal)
            return SML_ERR_OK; // parser signals unknown tag, this is ok for pre-1.2
          break;
      }
      if (rc != SML_ERR_OK)
        return rc;
      if (((rc = nextToken(pDecoder)) != SML_ERR_OK))
        return rc;
    } /* while we do not see an end tag */
    return SML_ERR_OK;
}


// @brief scan a single <Property></Property> bracket, or a <1.2 type list where a new <PropName> implies end of current property and start of a new one
static Ret_t buildDevInfProperty(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem, int datastoreLocal) {
    SmlDevInfCTDataPropListPtr_t  *ppListElem   = NULL;
    SmlDevInfCTDataPropListPtr_t  pProp         = NULL;
    Boolean_t                     newElement    = FALSE;
    XltDecScannerPtr_t            pScanner      = pDecoder->scanner;
    Ret_t rc;

    /* Do not return immediately for <1.2 style, the outer loop ends only meeting an end tag
     * which will lead to an infinite loop*/
    if (datastoreLocal && IS_EMPTY(pScanner->curtok)) {
        return SML_ERR_OK;
    }

    if (datastoreLocal) {
      // DS 1.2: we have no initial tag to process right away, get next tag now
      if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        return rc;
      }
      newElement=TRUE; // new <property> has already started here
    }

    // get pointer to where we'd insert the next list element
    ppListElem   = (SmlDevInfCTDataPropListPtr_t *) ppElem;
    while (*ppListElem!=NULL) {
      ppListElem = &((*ppListElem)->next);
    }
    pProp=NULL;
    // now ppListElem points to a SmlDevInfCTDataPropListPtr_t which is NULL and
    // can be overwritten with the link to a new element

    // scan until we hit end of enclosure (</Property>) or an unknown other tag
    while (pScanner->curtok->type != TOK_TAG_END) {
      if (!datastoreLocal && pScanner->curtok->tagid==TN_DEVINF_PROPNAME) {
        // DS 1.1-style CTCap, seeing <PropName> implies new element
        newElement=TRUE;
      }
      // create new element if needed
      if (newElement) {
        newElement=FALSE;
        pProp = (SmlDevInfCTDataPropListPtr_t)smlLibMalloc(sizeof(SmlDevInfCTDataPropList_t));
        if (pProp == NULL) return SML_ERR_NOT_ENOUGH_SPACE;
        smlLibMemset(pProp, 0, sizeof(SmlDevInfCTDataPropList_t));
        pProp->data = (SmlDevInfCTDataPropPtr_t)smlLibMalloc(sizeof(SmlDevInfCTDataProp_t));
        if (pProp->data == NULL) {
          smlFreeDevInfCTDataPropList(pProp);
          return SML_ERR_NOT_ENOUGH_SPACE;
        }
        smlLibMemset(pProp->data, 0, sizeof(SmlDevInfCTDataProp_t));
        pProp->data->prop = (SmlDevInfCTDataPtr_t)smlLibMalloc(sizeof(SmlDevInfCTData_t));
        if (pProp->data->prop == NULL) {
          smlFreeDevInfCTDataPropList(pProp);
          return SML_ERR_NOT_ENOUGH_SPACE;
        }
        smlLibMemset(pProp->data->prop, 0, sizeof(SmlDevInfCTData_t));
        // link into list
        *ppListElem = pProp;
        // set elem link to where we must link in next element
        ppListElem = &(pProp->next);
      }
      // now, if we don't have a pProp, this is an invalid <DS 1.2 devinf
      if (pProp==NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfProperty");

      // process tags
      switch (pScanner->curtok->tagid) {
        case TN_DEVINF_PROPPARAM:
          // DS 1.2 case: only </PropParam> ends the param building process
          rc = buildDevInfPropParam(pDecoder, (VoidPtr_t)&pProp->data->param,datastoreLocal);
          break;
        case TN_DEVINF_PARAMNAME:
          // <DS 1.2 case: current token TN_DEVINF_PARAMNAME is processed by builder
          if (datastoreLocal) {
          	// found <ParamName> directly within <Property> (instead of enclosed in a <PropParam>) - Nokia 2630 style
            // -> do not reject it but try to process in DS 1.1 style even if this is a DS 1.2 devInf
            rc = buildDevInfPropParam(pDecoder, (VoidPtr_t)&pProp->data->param,FALSE); // force DS 1.1 style
            if (rc==SML_ERR_OK)
              continue; // re-evaluate current tag (tag that caused buildDevInfPropParam() to end, either next <PropName>, unknown or closing </CTCap>
          }
          else {
            rc = buildDevInfPropParam(pDecoder, (VoidPtr_t)&pProp->data->param,datastoreLocal);
            if (rc==SML_ERR_OK)
              continue; // re-evaluate current tag (tag that caused buildDevInfPropParam() to end, either next <PropName>, unknown or closing </CTCap>
          }
          break;

        /* PCDATA */
        case TN_DEVINF_PROPNAME:
          rc = buildPCData(pDecoder, (VoidPtr_t)&pProp->data->prop->name);
          break;

        default:
          rc = scanPropOrParamElement(pDecoder, pProp->data->prop);
          if (rc==SML_ERR_XLT_INVAL_PROTO_ELEM && !datastoreLocal)
            return SML_ERR_OK; // parser signals unknown tag, this is ok for pre-1.2
      }
      if (rc != SML_ERR_OK)
        return rc;
      if (((rc = nextToken(pDecoder)) != SML_ERR_OK))
        return rc;
    } /* while we do not see an end tag */
    return SML_ERR_OK;
}



// @brief scan a single <ctcap></ctcap> bracket, which may contain serveral ctcaps for <DS 1.2, and only one single ctcap for >=DS 1.2
Ret_t buildDevInfCtcap(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem, Boolean_t datastoreLocal)
{
    SmlDevInfCtcapListPtr_t       *ppListElem   = NULL;
    SmlDevInfCtcapListPtr_t       pCtcap        = NULL;
    Boolean_t                     newElement    = FALSE;
    XltDecScannerPtr_t            pScanner      = pDecoder->scanner;
    Ret_t rc;

    if (IS_EMPTY(pScanner->curtok)) {
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        return rc;
    }

    // get pointer to where we'd insert the next list element
    ppListElem   = (SmlDevInfCtcapListPtr_t *)ppElem;
    while (*ppListElem!=NULL) {
      ppListElem = &((*ppListElem)->next);
    }
    pCtcap=NULL;
    // now ppListElem points to a SmlDevInfCtcapListPtr_t which is NULL and
    // can be overwritten with the link to a new element

    // datastore local CTCap is DS 1.2 or later, and implies that every type description has its own <CTCap></CTCap>
    // So starting a CTCap implies creating a new element
    newElement=datastoreLocal;

    while (pScanner->curtok->type != TOK_TAG_END) {
        if (!datastoreLocal && pScanner->curtok->tagid==TN_DEVINF_CTTYPE) {
          // DS 1.1-style CTCap, seeing CTTYPE implies new element
          newElement=TRUE;
        }
        // create new element if needed
        if (newElement) {
          newElement=FALSE;
          pCtcap = (SmlDevInfCtcapListPtr_t)smlLibMalloc(sizeof(SmlDevInfCtcapList_t));
          if (pCtcap == NULL) return SML_ERR_NOT_ENOUGH_SPACE;
          smlLibMemset(pCtcap, 0, sizeof(SmlDevInfCtcapList_t));
          pCtcap->data = (SmlDevInfCTCapPtr_t)smlLibMalloc(sizeof(SmlDevInfCTCap_t));
          if (pCtcap->data == NULL) {
            smlFreeDevInfCtcapList(pCtcap);
            return SML_ERR_NOT_ENOUGH_SPACE;
          }
          smlLibMemset(pCtcap->data, 0, sizeof(SmlDevInfCTCap_t));
          // link into list
          *ppListElem = pCtcap;
          // set elem link to where we must link in next element
          ppListElem = &(pCtcap->next);
        }
        // now, if we don't have a pCtcap, this is an invalid <DS 1.2 devinf where another tag than <CTType> follows <CTCap>
        if (pCtcap==NULL)
          return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfCtcap");
        // process tags
        switch (pScanner->curtok->tagid) {
          case TN_DEVINF_CTTYPE:
            rc = buildPCData(pDecoder, (VoidPtr_t)&pCtcap->data->cttype);
            break;
          case TN_DEVINF_VERCT:
            rc = buildPCData(pDecoder, (VoidPtr_t)&pCtcap->data->verct);
            break;
          case TN_DEVINF_PROPERTY:
            // DS 1.2 case: only </Property> ends the property building process, next token must be read first
            // If there is a Property tag, let's take it as DS 1.2 Property Decoding even if the CTCAP is globally
            // This is found from some Nokia phones (eg. N900, which will send a DevInf v1.2 but the CTCAP
            // was not inside the datastore as DevInf v1.1)
            rc = buildDevInfProperty(pDecoder, (VoidPtr_t)&pCtcap->data->prop,TRUE);
            break;
          case TN_DEVINF_PROPNAME:
            // <DS 1.2 case: current token TN_DEVINF_PROPNAME is processed by builder, next occurence of TN_DEVINF_PROPNAME ends property as well
            if (datastoreLocal)
              rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfCtcap"); // <PropName> must be inside <Property>
            else {
              rc = buildDevInfProperty(pDecoder, (VoidPtr_t)&pCtcap->data->prop,datastoreLocal);
              if (rc==SML_ERR_OK)
                continue; // re-evaluate current tag (tag that caused buildDevInfProperty() to end, either unknown or closing </CTCap>
                          // this means do not return SML_ERR_OK unless this is an unknow tag or closing </CTCcap>, otherwise it will trigger an infinite loop
            }
            break;

        default:
            rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfCtcap");
        }
        if (rc != SML_ERR_OK) {
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            return rc;
        }
    } /* eof while */
    return SML_ERR_OK;
}


Ret_t buildDevInfDSMemCmd(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfDSMemPtr_t pElem;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfDSMemCmd");

    if ((pElem = (SmlDevInfDSMemPtr_t)smlLibMalloc(sizeof(SmlDevInfDSMem_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pElem, 0, sizeof(SmlDevInfDSMem_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pElem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
            /* PCDATA elements */
            case TN_DEVINF_SHAREDMEM:
                // %%% luz:2003-04-28: made work as a flag
                pElem->flags |= SmlDevInfSharedMem_f;
                rc = buildEmptyTag(pDecoder); // allow for <tag></tag> instead of <tag/>
                break;
            case TN_DEVINF_MAXMEM:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->maxmem);
                break;
            case TN_DEVINF_MAXID:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->maxid);
                break;
            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfDSMemCmd");
        }
        if (rc != SML_ERR_OK) {
            smlLibFree(pElem);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlLibFree(pElem);
            return rc;
        }
    }
    *ppElem = pElem;

    return SML_ERR_OK;
}

Ret_t buildDevInfCTCapCmd(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfCTCapPtr_t pElem;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfCTCapCmd");

    if ((pElem = (SmlDevInfCTCapPtr_t)smlLibMalloc(sizeof(SmlDevInfCTCap_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pElem, 0, sizeof(SmlDevInfCTCap_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pElem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
            case TN_DEVINF_CTTYPE:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->cttype);
                break;
            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfCTCapCmd");
        }
        if (rc != SML_ERR_OK) {
            smlLibFree(pElem);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlLibFree(pElem);
            return rc;
        }
    }
    *ppElem = pElem;

    return SML_ERR_OK;
}

Ret_t buildDevInfSyncCapCmd(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfSyncCapPtr_t pElem;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfSyncCapCmd");

    if ((pElem = (SmlDevInfSyncCapPtr_t)smlLibMalloc(sizeof(SmlDevInfSyncCap_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pElem, 0, sizeof(SmlDevInfSyncCap_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pElem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
            case TN_DEVINF_SYNCTYPE:
                rc = buildPCDataList(pDecoder, (VoidPtr_t)&pElem->synctype);
                break;
            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfSyncCapCmd");
        }
        if (rc != SML_ERR_OK) {
            smlLibFree(pElem);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlLibFree(pElem);
            return rc;
        }
    }
    *ppElem = pElem;

    return SML_ERR_OK;
}

Ret_t buildDevInfExtCmd(XltDecoderPtr_t pDecoder, VoidPtr_t *ppElem) {
    XltDecScannerPtr_t pScanner;
    SmlDevInfExtPtr_t pElem;
    Ret_t rc;

    pScanner = pDecoder->scanner;

    if (*ppElem != NULL)
        return SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfExtCmd");

    if ((pElem = (SmlDevInfExtPtr_t)smlLibMalloc(sizeof(SmlDevInfExt_t))) == NULL)
        return SML_ERR_NOT_ENOUGH_SPACE;
    smlLibMemset(pElem, 0, sizeof(SmlDevInfExt_t));

    if (IS_EMPTY(pScanner->curtok)) {
        *ppElem = pElem;
        return SML_ERR_OK;
    }

    if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
        smlLibFree(pElem);
        return rc;
    }

    while (pScanner->curtok->type != TOK_TAG_END) {
        switch (pScanner->curtok->tagid) {
            case TN_DEVINF_XNAM:
                rc = buildPCData(pDecoder, (VoidPtr_t)&pElem->xnam);
                break;
            case TN_DEVINF_XVAL:
                rc = buildPCDataList(pDecoder, (VoidPtr_t)&pElem->xval);
                break;
            default:
                rc = SML_DECODEERROR(SML_ERR_XLT_INVAL_SYNCML_DOC,pScanner,"buildDevInfExtCmd");
        }
        if (rc != SML_ERR_OK) {
            smlLibFree(pElem);
            return rc;
        }
        if (((rc = nextToken(pDecoder)) != SML_ERR_OK)) {
            smlLibFree(pElem);
            return rc;
        }
    }
    *ppElem = pElem;

    return SML_ERR_OK;
}


/* see xltenc.c:XltEncBlock for description of parameters */
Ret_t devinfEncBlock(XltTagID_t tagId, XltRO_t reqOptFlag, const VoidPtr_t pContent, SmlEncoding_t enc, BufferMgmtPtr_t pBufMgr, SmlPcdataExtension_t attFlag) {
  //Return variable
  Ret_t _err;
  SmlPcdataListPtr_t           pList     = NULL;
  SmlDevInfDatastoreListPtr_t  dsList    = NULL;
  SmlDevInfCtcapListPtr_t      ctList    = NULL;
  SmlDevInfExtListPtr_t        exList    = NULL;
  SmlDevInfXmitListPtr_t       xmList    = NULL;
  SmlDevInfFilterCapListPtr_t  fcapList  = NULL;
  SmlDevInfCTDataPropListPtr_t propList  = NULL;
  SmlDevInfCTDataListPtr_t     paramList = NULL;
  XltTagID_t                   nameTagID = TN_UNDEF;
  SmlDevInfCTDataPtr_t         pPropOrParam = NULL;


    //Check if pContent of a required field is missing
  if ((reqOptFlag == REQUIRED) && (pContent == NULL))
    return SML_ERR_XLT_MISSING_CONT;
  //Check if pContent of a optional field is missing -> if yes we are done
  else if (pContent == NULL)
    return SML_ERR_OK;

  //Generate the commands -> see DTD
  switch (tagId) {
    case TN_DEVINF_EXT:
      if ((_err = xltGenerateTag(TN_DEVINF_EXT, TT_BEG, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_XNAM, REQUIRED, ((SmlDevInfExtPtr_t) pContent)->xnam, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      pList = ((SmlDevInfExtPtr_t)pContent)->xval;
          while (pList != NULL) {
              if ((_err = devinfEncBlock(TN_DEVINF_XVAL, OPTIONAL, pList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
              pList = pList->next;
          };
      if ((_err = xltGenerateTag(TN_DEVINF_EXT, TT_END, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      break;
    case TN_DEVINF_SYNCCAP:
      if ((_err = xltGenerateTag(TN_DEVINF_SYNCCAP, TT_BEG, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      pList = ((SmlDevInfSyncCapPtr_t)pContent)->synctype;
          while (pList != NULL) {
              if ((_err = devinfEncBlock(TN_DEVINF_SYNCTYPE, OPTIONAL, pList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
              pList = pList->next;
          };
      if ((_err = xltGenerateTag(TN_DEVINF_SYNCCAP, TT_END, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      break;
    case TN_DEVINF_SHAREDMEM:
      //set the flag in the (WB)XML document if the flag is in the pContent
      if ((*((Flag_t *) pContent)) & (SmlDevInfSharedMem_f))
          if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      break;
    // %%% luz:2003-04-28 added missing 1.1 devinf tags here
    case TN_DEVINF_UTC:
      //set the flag in the (WB)XML document if the flag is in the pContent
      if ((*((Flag_t *) pContent)) & (SmlDevInfUTC_f)) {
        if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      }
      break;
    case TN_DEVINF_NOFM:
      //set the flag in the (WB)XML document if the flag is in the pContent
      if ((*((Flag_t *) pContent)) & (SmlDevInfNOfM_f)) {
        if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      }
      break;
    case TN_DEVINF_LARGEOBJECT:
      //set the flag in the (WB)XML document if the flag is in the pContent
      if ((*((Flag_t *) pContent)) & (SmlDevInfLargeObject_f)) {
        if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      }
      break;

    /* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
    case TN_DEVINF_FIELDLEVEL:
      //set the flag in the (WB)XML document if the flag is in the pContent
      if ((*((Flag_t *) pContent)) & (SmlDevInfFieldLevel_f)) {
        if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      }
      break;
    case TN_DEVINF_HIERARCHICAL:
      //set the flag in the (WB)XML document if the flag is in the pContent
      if ((*((Flag_t *) pContent)) & (SmlDevInfHierarchical_f)) {
        if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      }
      break;
    case TN_DEVINF_NOTRUNCATE:
      //set the flag in the (WB)XML document if the flag is in the pContent
      if ((*((Flag_t *) pContent)) & (SmlDevInfNoTruncate_f)) {
        if ((_err = xltGenerateTag(tagId, TT_ALL, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      }
      break;

    case TN_DEVINF_PROPERTY:
      // pContent is SmlDevInfCTDataPropPtr_t
      paramList = ((SmlDevInfCTDataPropPtr_t)pContent)->param; // get param list
      // modify pContent
      pPropOrParam = ((SmlDevInfCTDataPropPtr_t)pContent)->prop; // common property/propparam description record
      // set the tag to be used for name
      nameTagID=TN_DEVINF_PROPNAME; // is a property, use <PropName>
      goto param_or_property;
    case TN_DEVINF_PROPPARAM:
      // pContent is SmlDevInfCTDataPtr_t (common data structure for <Property> and <PropParam>)
      paramList=NULL;
      // set the tag to be used for name
      nameTagID=TN_DEVINF_PARAMNAME; // is a propparam, use <ParamName>
      // set content
      pPropOrParam = (SmlDevInfCTDataPtr_t)pContent;
      goto param_or_property;
    param_or_property:
      // - pPropOrParam is SmlDevInfCTDataPtr_t (common data structure for <Property> and <PropParam>)
      // - paramList is either NULL (for params, which do not have params themselves) or SmlDevInfCTDataListPtr_t (for properties)
      // DS 1.2 or higher : open <Property>/<PropParam> to properly group all property related elements
      if (pBufMgr->vers>=SML_VERS_1_2) {
        if ((_err = xltGenerateTag(tagId, TT_BEG, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      }
      /* -- Propname or ParamName */
      if ((_err = devinfEncBlock(nameTagID, REQUIRED, pPropOrParam->name, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      /* -- (ValEnum+ | (Datatype, Size?))? */
      //%%% luz 2007-08-14 empty datatype is allowed as well (as this is what e.g. Nokia E90 sends, and when using SML to translate back to XML, we'd failed here before)
      if (pPropOrParam->valenum != NULL && pPropOrParam->datatype != NULL && pPropOrParam->datatype->length!=0)
        return SML_ERR_XLT_INVAL_INPUT_DATA;
      if (pPropOrParam->valenum != NULL) {
          // ValEnum+
          pList = pPropOrParam->valenum;
          while (pList != NULL) {
              if ((_err = devinfEncBlock(TN_DEVINF_VALENUM, REQUIRED, pList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
              pList = pList->next;
          };
      } else if (pPropOrParam->datatype != NULL) {
          // Datatype?
          if ((_err = devinfEncBlock(TN_DEVINF_DATATYPE, REQUIRED, pPropOrParam->datatype, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      }
      /* (Max)Size? (note, in <1.2, the tag was called "Size", in >=1.2, the tag is called "MaxSize" - but both have the same WBXML token) */
      if ((_err = devinfEncBlock(pBufMgr->vers>=SML_VERS_1_2 ? TN_DEVINF_MAXSIZE : TN_DEVINF_SIZE, OPTIONAL, pPropOrParam->maxsize, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      /* -- DS 1.2: MaxOccur?, is valid for Properties only, not for PropParams */
      if ((_err = devinfEncBlock(TN_DEVINF_MAXOCCUR, OPTIONAL, pPropOrParam->maxoccur, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      /* -- DS 1.2: (NoTruncate?), is valid for Properties only, not for PropParams */
      if ((_err = devinfEncBlock(TN_DEVINF_NOTRUNCATE, OPTIONAL, &(pPropOrParam->flags), enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;

      /* -- DisplayName ? */
      if ((_err = devinfEncBlock(TN_DEVINF_DISPLAYNAME, OPTIONAL, pPropOrParam->dname, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;

      /* -- now the paramList */
      while (paramList != NULL) {
          /* Synthesis/luz 2005-08-24 : separated generation of <PropParam> element into own case */
          if ((_err = devinfEncBlock(TN_DEVINF_PROPPARAM, REQUIRED, paramList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
          paramList = paramList->next;
      }
      // DS 1.2 or higher : close </Property>/</PropParam> encapsulation
      if (pBufMgr->vers>=SML_VERS_1_2) {
        if ((_err = xltGenerateTag(tagId, TT_END, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      }
      break;

    case TN_DEVINF_CTCAP:
      ctList = ((SmlDevInfCtcapListPtr_t)pContent);
      if (ctList == NULL)
        break; // empty CTCap - is ok, do not output anything
      // before DS 1.2 : all types are encapsulated in a single CTCap
      if (pBufMgr->vers<SML_VERS_1_2) {
        if ((_err = xltGenerateTag(TN_DEVINF_CTCAP, TT_BEG, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      }
      while (ctList != NULL) {
          if (ctList->data == NULL) return SML_ERR_XLT_INVAL_INPUT_DATA;
          // DS 1.2 or higher : every content type is separately enclosed in a CTCap
          if (pBufMgr->vers>=SML_VERS_1_2) {
            if ((_err = xltGenerateTag(TN_DEVINF_CTCAP, TT_BEG, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
          }
          if ((_err = devinfEncBlock(TN_DEVINF_CTTYPE, REQUIRED, ctList->data->cttype, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
          /* SyncML DS 1.2, Synthesis/luz 2005-09-28, required for DS 1.2 only, therefore OPTIONAL here */
          if ((_err = devinfEncBlock(TN_DEVINF_VERCT, OPTIONAL, ctList->data->verct, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
          /* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
          if ((_err = devinfEncBlock(TN_DEVINF_FIELDLEVEL, OPTIONAL, &(ctList->data->flags), enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;

          /* now the propList */
          // %%% luz 2002-11-27: made property list optional (e.g. text/message of P800 has none)
          propList = ctList->data->prop;
          // %%% original: if (propList == NULL) return SML_ERR_XLT_INVAL_INPUT_DATA;
          while (propList != NULL) {
              if (propList->data == NULL) return SML_ERR_XLT_INVAL_INPUT_DATA;
              if (propList->data->prop == NULL) return SML_ERR_XLT_INVAL_INPUT_DATA;
              /* Synthesis/luz 2005-08-24 : separated generation of <property> element into own case */
              if ((_err = devinfEncBlock(TN_DEVINF_PROPERTY, REQUIRED, propList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
              propList = propList->next;
          }
          // DS 1.2 or higher : every content type is separately enclosed in a CTCap
          if (pBufMgr->vers>=SML_VERS_1_2) {
            if ((_err = xltGenerateTag(TN_DEVINF_CTCAP, TT_END, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
          }
          /* eof propList */
          ctList = ctList->next;
      };
      // before DS 1.2 : all types are encapsulated in a single CTCap
      if (pBufMgr->vers<SML_VERS_1_2) {
        if ((_err = xltGenerateTag(TN_DEVINF_CTCAP, TT_END, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      }
      break;

    case TN_DEVINF_DSMEM:
      if ((_err = xltGenerateTag(TN_DEVINF_DSMEM, TT_BEG, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_SHAREDMEM, OPTIONAL, &(((SmlDevInfDSMemPtr_t) pContent)->flags), enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_MAXMEM,    OPTIONAL, ((SmlDevInfDSMemPtr_t) pContent)->maxmem, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_MAXID,     OPTIONAL, ((SmlDevInfDSMemPtr_t) pContent)->maxid,  enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = xltGenerateTag(TN_DEVINF_DSMEM, TT_END, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      break;

    // special case, the following 5 have the same structure, only the tag name differs
    case TN_DEVINF_RX:
    case TN_DEVINF_TX:
    case TN_DEVINF_RXPREF:
    case TN_DEVINF_TXPREF:
    case TN_DEVINF_FILTERRX: /* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
      if ((_err = xltGenerateTag(tagId, TT_BEG, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_CTTYPE, REQUIRED, ((SmlDevInfXmitPtr_t) pContent)->cttype, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_VERCT,  REQUIRED, ((SmlDevInfXmitPtr_t) pContent)->verct,  enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = xltGenerateTag(tagId, TT_END, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      break;

    /* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
    case TN_DEVINF_FILTERCAP:
      if ((_err = xltGenerateTag(tagId, TT_BEG, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_CTTYPE, REQUIRED, ((SmlDevInfFilterCapPtr_t) pContent)->cttype, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_VERCT,  REQUIRED, ((SmlDevInfFilterCapPtr_t) pContent)->verct,  enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      pList = ((SmlDevInfFilterCapPtr_t) pContent)->filterkeyword;
      while (pList != NULL) {
        if ((_err = devinfEncBlock(TN_DEVINF_FILTERKEYWORD, REQUIRED, pList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
        pList=pList->next;
      }
      pList = ((SmlDevInfFilterCapPtr_t) pContent)->propname;
      while (pList != NULL) {
        if ((_err = devinfEncBlock(TN_DEVINF_PROPNAME, REQUIRED, pList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
        pList=pList->next;
      }
      if ((_err = xltGenerateTag(tagId, TT_END, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      break;


    case TN_DEVINF_DATASTORE:
      if ((_err = xltGenerateTag(TN_DEVINF_DATASTORE, TT_BEG, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_SOURCEREF,    REQUIRED, ((SmlDevInfDatastorePtr_t) pContent)->sourceref,    enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_DISPLAYNAME,  OPTIONAL, ((SmlDevInfDatastorePtr_t) pContent)->displayname,  enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_MAXGUIDSIZE,  OPTIONAL, ((SmlDevInfDatastorePtr_t) pContent)->maxguidsize,  enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_RXPREF,       REQUIRED, ((SmlDevInfDatastorePtr_t) pContent)->rxpref,       enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      xmList = ((SmlDevInfDatastorePtr_t)pContent)->rx;
      while (xmList != NULL) {
          if ((_err = devinfEncBlock(TN_DEVINF_RX, OPTIONAL, xmList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
          xmList = xmList->next;
      };
      if ((_err = devinfEncBlock(TN_DEVINF_TXPREF,       REQUIRED, ((SmlDevInfDatastorePtr_t) pContent)->txpref,       enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      xmList = ((SmlDevInfDatastorePtr_t)pContent)->tx;
      while (xmList != NULL) {
          if ((_err = devinfEncBlock(TN_DEVINF_TX, OPTIONAL, xmList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
          xmList = xmList->next;
      };
      /* For SyncML DS 1.2, CTCaps are local to datastore, no longer global in DevInf */
      if ((_err = devinfEncBlock(TN_DEVINF_CTCAP,        OPTIONAL, ((SmlDevInfDatastorePtr_t) pContent)->ctcap,        enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;

      if ((_err = devinfEncBlock(TN_DEVINF_DSMEM,        OPTIONAL, ((SmlDevInfDatastorePtr_t) pContent)->dsmem,        enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      /* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
      if ((_err = devinfEncBlock(TN_DEVINF_HIERARCHICAL, OPTIONAL, &(((SmlDevInfDatastorePtr_t) pContent)->flags),     enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;

      if ((_err = devinfEncBlock(TN_DEVINF_SYNCCAP,      REQUIRED, ((SmlDevInfDatastorePtr_t) pContent)->synccap,      enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;

      /* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
      xmList = ((SmlDevInfDatastorePtr_t)pContent)->filterrx;
      while (xmList != NULL) {
          if ((_err = devinfEncBlock(TN_DEVINF_FILTERRX, OPTIONAL, xmList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
          xmList = xmList->next;
      };
      /* SyncML DS 1.2, Synthesis/luz 2005-08-25 */
      fcapList = ((SmlDevInfDatastorePtr_t)pContent)->filtercap;
      while (fcapList != NULL) {
          if ((_err = devinfEncBlock(TN_DEVINF_FILTERCAP, OPTIONAL, fcapList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
          fcapList = fcapList->next;
      };

      if ((_err = xltGenerateTag(TN_DEVINF_DATASTORE, TT_END, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      break;

    case TN_DEVINF_DEVINF:
      if ((_err = xltGenerateTag(TN_DEVINF_DEVINF, TT_BEG, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_VERDTD,  REQUIRED, ((SmlDevInfDevInfPtr_t) pContent)->verdtd,  enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_MAN,     OPTIONAL, ((SmlDevInfDevInfPtr_t) pContent)->man,     enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_MOD,     OPTIONAL, ((SmlDevInfDevInfPtr_t) pContent)->mod,     enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_OEM,     OPTIONAL, ((SmlDevInfDevInfPtr_t) pContent)->oem,     enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_FWV,     OPTIONAL, ((SmlDevInfDevInfPtr_t) pContent)->fwv,     enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_SWV,     OPTIONAL, ((SmlDevInfDevInfPtr_t) pContent)->swv,     enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_HWV,     OPTIONAL, ((SmlDevInfDevInfPtr_t) pContent)->hwv,     enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_DEVID,   REQUIRED, ((SmlDevInfDevInfPtr_t) pContent)->devid,   enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_DEVTYP,  REQUIRED, ((SmlDevInfDevInfPtr_t) pContent)->devtyp,  enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_UTC,     OPTIONAL, &(((SmlDevInfDevInfPtr_t) pContent)->flags),     enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_NOFM,    OPTIONAL, &(((SmlDevInfDevInfPtr_t) pContent)->flags),     enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      if ((_err = devinfEncBlock(TN_DEVINF_LARGEOBJECT, OPTIONAL, &(((SmlDevInfDevInfPtr_t) pContent)->flags), enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      // According to SyncML standard, the dsList must not be empty. However as some implementations send such devInf,
      // and the RTK is also used to convert WBXML to XML for human readable message dumps, we now allow
      // generating such devInf (for the XML log output). 
      dsList = ((SmlDevInfDevInfPtr_t)pContent)->datastore;
      //if (dsList == NULL) return SML_ERR_XLT_MISSING_CONT;
      if(dsList != NULL) {
          if ((_err = devinfEncBlock(TN_DEVINF_DATASTORE, REQUIRED, dsList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
          dsList = dsList->next;
          while (dsList != NULL) {
              if ((_err = devinfEncBlock(TN_DEVINF_DATASTORE, OPTIONAL, dsList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
              dsList = dsList->next;
          };
      }
      // for pre DS 1.2, CTCaps are global (i.e at the devInf level):
      if ((_err = devinfEncBlock(TN_DEVINF_CTCAP, OPTIONAL, ((SmlDevInfDevInfPtr_t)pContent)->ctcap, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      // extensions
      exList = ((SmlDevInfDevInfPtr_t)pContent)->ext;
      while (exList != NULL) {
          if ((_err = devinfEncBlock(TN_DEVINF_EXT, OPTIONAL, exList->data, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
          exList = exList->next;
      };

      if ((_err = xltGenerateTag(TN_DEVINF_DEVINF, TT_END, enc, pBufMgr, SML_EXT_DEVINF)) != SML_ERR_OK) return _err;
      break;

    default: { // all leaf nodes (PCDATA#)
      return xltEncPcdata(tagId, reqOptFlag, pContent, enc, pBufMgr, attFlag);
    } //* eof default statement from switch tagid
  } // eof switch tagid
  return SML_ERR_OK;
}
#endif /* __USE_DEVINF__ */
