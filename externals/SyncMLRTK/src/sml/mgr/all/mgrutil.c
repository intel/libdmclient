/**
 * @file
 * some helper functions
 *
 * @target_system   all
 * @target_os       all
 * @description
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

/*************************************************************************
 *  Definitions
 *************************************************************************/

#include "syncml_tk_prefix_file.h" // %%% luz: needed for precompiled headers in eVC++

/* Include Headers */
#include <sml.h>
#include <smldtd.h>
#include <smldef.h>
#include <smlerr.h>
#include <smlmetinfdtd.h>
#include <smldevinfdtd.h>
#include "libmem.h"
#include "libstr.h"
#include "liblock.h"
#include "mgr.h"
#include "mgrutil.h"


/*************************************************************************
 *  Exported SyncML API functions
 *************************************************************************/

/**
 * Frees all allocated memory of a smlProtoElement
 *
 * @param pProtoElement (IN)
 *        Element to free
 * @return Return Code
 */
SML_API Ret_t smlFreeProtoElement(VoidPtr_t pProtoElement)
{
  if (! pProtoElement)
    return(SML_ERR_OK);

  switch (((SmlUnknownProtoElementPtr_t)pProtoElement)->elementType) {

    case SML_PE_HEADER:
      smlFreeSyncHdr((SmlSyncHdrPtr_t)pProtoElement);
      break;

    case SML_PE_SYNC_START:
      smlFreeSync((SmlSyncPtr_t)pProtoElement);
      break;

    case SML_PE_ADD:
    case SML_PE_COPY:
    case SML_PE_MOVE:
    case SML_PE_REPLACE:
    case SML_PE_DELETE:
    case SML_PE_GENERIC:
      smlFreeGeneric((SmlGenericCmdPtr_t)pProtoElement);
      break;

    case SML_PE_ALERT:
      smlFreeAlert((SmlAlertPtr_t)pProtoElement);
      break;

    case SML_PE_ATOMIC_START:
    case SML_PE_SEQUENCE_START:
    case SML_PE_CMD_GROUP:
      smlFreeAtomic((SmlAtomicPtr_t)pProtoElement);
      break;

#if (defined EXEC_SEND || defined EXEC_RECEIVE)
    case SML_PE_EXEC:
      smlFreeExec((SmlExecPtr_t)pProtoElement);
      break;
#endif

    case SML_PE_PUT:
    case SML_PE_GET:
    case SML_PE_PUT_GET:
      smlFreeGetPut((SmlPutPtr_t)pProtoElement);
      break;

    case SML_PE_MAP:
      smlFreeMap((SmlMapPtr_t)pProtoElement);
      break;

    case SML_PE_RESULTS:
      smlFreeResults((SmlResultsPtr_t)pProtoElement);
      break;

#if (defined SEARCH_SEND || defined SEARCH_RECEIVE)
    case SML_PE_SEARCH:
      smlFreeSearch((SmlSearchPtr_t)pProtoElement);
      break;
#endif
    case SML_PE_STATUS:
      smlFreeStatus((SmlStatusPtr_t)pProtoElement);
      break;

    default:
      return(SML_ERR_A_UTI_UNKNOWN_PROTO_ELEMENT);
  }

  return(SML_ERR_OK);
}


/**
 * Frees the Memory of an allocated Pcdata memory object
 *
 * @param pPcdata (IN)
 *        A Pointer to a PcData structure, which should be freed
 */
SML_API void smlFreePcdata(SmlPcdataPtr_t pPcdata)
{
  if (! pPcdata)
    return;

  if (pPcdata->contentType == SML_PCDATA_EXTENSION) {
    switch ((int)pPcdata->extension) {
#ifdef __USE_METINF__
      case SML_EXT_METINF:
        smlFreeMetinfMetinf(pPcdata->content);
        smlLibFree(pPcdata);
        break;
#endif
#ifdef __USE_DEVINF__
      case SML_EXT_DEVINF:
        smlFreeDevInfDevInf(pPcdata->content);
        smlLibFree(pPcdata);
        break;
#endif
    }
    return;
  }

  if (pPcdata->content)
    smlLibFree(pPcdata->content);

  smlLibFree(pPcdata);
}


SML_API void smlFreePcdataList(SmlPcdataListPtr_t list) {
  if (!list) return;
  smlFreePcdataList(list->next);
  smlFreePcdata(list->data);
  smlLibFree(list);
  return;
}

#ifdef __USE_METINF__
/** Subfunctions to smlFreePcdata, to freeup MetaInf DTD structures */
SML_API void smlFreeMetinfMetinf(SmlMetInfMetInfPtr_t data) {
    if (!data) return;
    smlFreePcdata(data->format);
    smlFreePcdata(data->type);
    smlFreePcdata(data->mark);
    smlFreePcdata(data->size);
    smlFreePcdata(data->version);
    smlFreePcdata(data->nextnonce);
    smlFreePcdata(data->maxmsgsize);
    /* SCTSTK - 18/03/2002, S.H. 2002-04-05 : SyncML 1.1 */
    smlFreePcdata(data->maxobjsize);
    smlFreeMetinfAnchor(data->anchor);
    smlFreeMetinfMem(data->mem);
    smlFreePcdataList(data->emi);
    smlLibFree(data);
    return;
}
SML_API void smlFreeMetinfAnchor(SmlMetInfAnchorPtr_t data) {
    if (!data) return;
    smlFreePcdata(data->last);
    smlFreePcdata(data->next);
    smlLibFree(data);
    return;
}
SML_API void smlFreeMetinfMem(SmlMetInfMemPtr_t data) {
    if (!data) return;
    /* smlFreePcdata(data->shared); %%% luz 2005-08-24: shared is a flag, not a Pcdata - */
    smlFreePcdata(data->free);
    smlFreePcdata(data->freeid);
    smlLibFree(data);
    return;
}
#endif

#ifdef __USE_DEVINF__
/** Subfunctions to smlFreePcdata, to freeup DevInf DTD structures */
SML_API void smlFreeDevInfDevInf(SmlDevInfDevInfPtr_t data) {
    if (!data) return;
    smlFreePcdata(data->verdtd);
    smlFreePcdata(data->man);
    smlFreePcdata(data->mod);
    smlFreePcdata(data->oem);
    smlFreePcdata(data->fwv);
    smlFreePcdata(data->hwv);
    smlFreePcdata(data->swv);
    smlFreePcdata(data->devid);
    smlFreePcdata(data->devtyp);
    smlFreeDevInfDatastoreList(data->datastore);
    smlFreeDevInfExtList(data->ext);
    smlFreeDevInfCtcapList(data->ctcap);
    smlLibFree(data);
}
SML_API void smlFreeDevInfDatastore(SmlDevInfDatastorePtr_t data) {
    if (!data) return;
    smlFreePcdata(data->sourceref);
    smlFreePcdata(data->displayname);
    smlFreePcdata(data->maxguidsize);
    smlFreeDevInfXmit(data->rxpref);
    smlFreeDevInfXmit(data->txpref);
    smlFreeDevInfXmitList(data->rx);
    smlFreeDevInfXmitList(data->tx);
    smlFreeDevInfDSMem(data->dsmem);
    smlFreeDevInfSynccap(data->synccap);
    smlFreeDevInfXmitList(data->filterrx);
    smlFreeDevInfCtcapList(data->ctcap);
    smlFreeDevInfFilterCapList(data->filtercap);
    smlLibFree(data);
    return;
}
SML_API void smlFreeDevInfDatastoreList(SmlDevInfDatastoreListPtr_t data) {
    if (!data) return;
    smlFreeDevInfDatastore(data->data);
    smlFreeDevInfDatastoreList(data->next);
    smlLibFree(data);
    return;
}
SML_API void smlFreeDevInfXmitList(SmlDevInfXmitListPtr_t data) {
    if (!data) return;
    smlFreeDevInfXmit(data->data);
    smlFreeDevInfXmitList(data->next);
    smlLibFree(data);
}
SML_API void smlFreeDevInfXmit(SmlDevInfXmitPtr_t data) {
    if (!data) return;
    smlFreePcdata(data->cttype);
    smlFreePcdata(data->verct);
    smlLibFree(data);
}
SML_API void smlFreeDevInfDSMem(SmlDevInfDSMemPtr_t data) {
    if (!data) return;
    // %%%luz:2003-04-28: this is now a flag! smlFreePcdata(data->shared);
    smlFreePcdata(data->maxmem);
    smlFreePcdata(data->maxid);
    smlLibFree(data);
}
SML_API void smlFreeDevInfSynccap(SmlDevInfSyncCapPtr_t data) {
    if (!data) return;
    smlFreePcdataList(data->synctype);
    smlLibFree(data);
}
SML_API void smlFreeDevInfExt(SmlDevInfExtPtr_t data) {
    if (!data) return;
    smlFreePcdata(data->xnam);
    smlFreePcdataList(data->xval);
    smlLibFree(data);
}
SML_API void smlFreeDevInfExtList(SmlDevInfExtListPtr_t data) {
    if (!data) return;
    smlFreeDevInfExt(data->data);
    smlFreeDevInfExtList(data->next);
    smlLibFree(data);
}
SML_API void smlFreeDevInfCTData(SmlDevInfCTDataPtr_t data) {
    if (!data) return;
    smlFreePcdata(data->name);
    smlFreePcdata(data->dname);
    smlFreePcdataList(data->valenum);
    smlFreePcdata(data->datatype);
    smlFreePcdata(data->maxsize);
    smlFreePcdata(data->maxoccur);
    smlLibFree(data);
}
SML_API void smlFreeDevInfCTDataProp(SmlDevInfCTDataPropPtr_t data) {
    if (!data) return;
    smlFreeDevInfCTData(data->prop);
    smlFreeDevInfCTDataList(data->param);
    smlLibFree(data);
}

SML_API void smlFreeDevInfCTDataList(SmlDevInfCTDataListPtr_t data) {
    if (!data) return;
    smlFreeDevInfCTData(data->data);
    smlFreeDevInfCTDataList(data->next);
    smlLibFree(data);
}

SML_API void smlFreeDevInfCTDataPropList(SmlDevInfCTDataPropListPtr_t data) {
    if (!data) return;
    smlFreeDevInfCTDataProp(data->data);
    smlFreeDevInfCTDataPropList(data->next);
    smlLibFree(data);
}

SML_API void smlFreeDevInfCTCap(SmlDevInfCTCapPtr_t data) {
    if (!data) return;
    smlFreePcdata(data->cttype);
    smlFreePcdata(data->verct);
    smlFreeDevInfCTDataPropList(data->prop);
    smlLibFree(data);
}

SML_API void smlFreeDevInfCtcapList(SmlDevInfCtcapListPtr_t data) {
    if (!data) return;

    smlFreeDevInfCTCap(data->data);
    smlFreeDevInfCtcapList(data->next);
    smlLibFree(data);
}

SML_API void smlFreeDevInfFilterCap(SmlDevInfFilterCapPtr_t data) {
    if (!data) return;
    smlFreePcdata(data->cttype);
    smlFreePcdata(data->verct);
    smlFreePcdataList(data->filterkeyword);
    smlFreePcdataList(data->propname);
    smlLibFree(data);
}

SML_API void smlFreeDevInfFilterCapList(SmlDevInfFilterCapListPtr_t data) {
    if (!data) return;
    smlFreeDevInfFilterCap(data->data);
    smlFreeDevInfFilterCapList(data->next);
    smlLibFree(data);
}

#endif


/*************************************************************************
 *  Exported SyncML API functions (FULL-SIZE TOOLKIT ONLY)
 *************************************************************************/

#ifndef __SML_LITE__  /* these API calls are NOT included in the Toolkit lite version */

/**
 * Return amount of unused Workspace memory
 *
 * @param id instance ID
 * @return Amount of unused Workspace memory
 */
SML_API MemSize_t smlGetFreeBuffer(InstanceID_t id)
{
  /* Definitions */
  MemSize_t  freeMem=0;

  #ifdef NOWSM
    InstanceInfoPtr_t pInstanceInfo = (InstanceInfoPtr_t)id; // ID is the instance info pointer
    if (pInstanceInfo==NULL) return 0; // no buffer if no instance there
    freeMem =
      (pInstanceInfo->instanceBuffer+pInstanceInfo->instanceBufSiz) - // end of buffer
      pInstanceInfo->writePointer; // current write pointer
  #else
    /* ask the Workspace Buffer */
    LOCKTOOLKIT("smlGetFreeBuffer");
    wsmGetFreeSize(id, &freeMem);
    RELEASETOOLKIT("smlGetFreeBuffer");
  #endif

  return (MemSize_t)freeMem;
}


/**
 * Copy a string into a Pcdata structure
 *
 * @param str (IN)
 *        Input String
 * @return A Pointer to a PcData structure
 */
SML_API SmlPcdataPtr_t smlString2Pcdata(String_t str)
{
  /* Definitions */
  SmlPcdataPtr_t pcdata;

  /* Invalid Input */
      if (! str)
    return NULL;

  /* Allocate the PcData Structure */
  pcdata = (SmlPcdataPtr_t)smlLibMalloc((MemSize_t)sizeof(SmlPcdata_t));
  if (! pcdata)
    return NULL;
      smlLibMemset (pcdata, 0, (MemSize_t)sizeof(SmlPcdata_t));

  /* Set the PcData Structure */
  pcdata->contentType = SML_PCDATA_STRING;
  pcdata->length = smlLibStrlen( str );
  pcdata->content = (VoidPtr_t)smlLibStrdup(str);

  return pcdata;
}


/**
 * Copy a Pcdata structure into a string
 *
 * @param pcdata (IN)
 *        A Pointer to a PcData structure
 * @return Input String
 */
SML_API String_t smlPcdata2String(SmlPcdataPtr_t pcdata)
{
  /* Definitions */
  String_t  str;

  /* Invalid Input */
      if (! pcdata)
    return NULL;

      /* Allocate the String */
    str = (String_t)smlLibMalloc((MemSize_t)(pcdata->length+1));
    if (str == NULL)
    return NULL;

  /* Copy the string into the allocated data structure */
      smlLibMemcpy((MemPtr_t)str, (MemPtr_t)pcdata->content, pcdata->length);
      *(str + pcdata->length) = '\0';

  return str;
}


/**
 * Duplicates a Pcdata memory object
 *
 * @param pcdata (IN)
 *        A Pointer to the original PcData structure
 * @return A Pointer to the copy of the PcData structure
 */
SML_API SmlPcdataPtr_t smlPcdataDup(SmlPcdataPtr_t pcdata)
{
  /* Definitions */
  SmlPcdataPtr_t newPcdata;

  /* Invalid Input */
  if (! pcdata)
    return NULL;

  /* Allocate the new pcdata memory object */
  newPcdata = (SmlPcdataPtr_t)smlLibMalloc((MemSize_t)sizeof(SmlPcdata_t));
  if (! newPcdata)
    return NULL;
  smlLibMemset (newPcdata, 0, (MemSize_t)sizeof(SmlPcdata_t));

  /* Set the PcData Structure */
  newPcdata->contentType = pcdata->contentType;
  newPcdata->length = pcdata->length;
  newPcdata->content =(VoidPtr_t)smlLibMalloc((MemSize_t)pcdata->length+1);
  if (newPcdata->content ==NULL)
  {
      smlLibFree(newPcdata->content);
        smlLibFree(newPcdata);
        return NULL;
  }
  smlLibMemset(newPcdata->content, 0, (MemSize_t)((pcdata->length)+1));
  smlLibMemcpy(newPcdata->content, pcdata->content, (MemSize_t)pcdata->length);

  return newPcdata;
}

#endif

SML_API void smlFreeSyncHdr(SmlSyncHdrPtr_t pSyncHdr)
{
  if (! pSyncHdr)
    return;

  smlFreePcdata(pSyncHdr->version);
  smlFreePcdata(pSyncHdr->proto);
  smlFreePcdata(pSyncHdr->sessionID);
  smlFreePcdata(pSyncHdr->msgID);
  smlFreePcdata(pSyncHdr->respURI);
  smlFreePcdata(pSyncHdr->meta);

  smlFreeSourceTargetPtr(pSyncHdr->source);
  smlFreeSourceTargetPtr(pSyncHdr->target);

  smlFreeCredPtr(pSyncHdr->cred);

  smlLibFree(pSyncHdr);
}


SML_API void smlFreeSync(SmlSyncPtr_t pSync)
{
  if (! pSync)
    return;

  smlFreePcdata(pSync->cmdID);
  smlFreePcdata(pSync->meta);
  smlFreePcdata(pSync->noc); // %%% luz 2004-09-29 added to fix mem leak

  smlFreeSourceTargetPtr(pSync->source);
  smlFreeSourceTargetPtr(pSync->target);

  smlFreeCredPtr(pSync->cred);

  smlLibFree(pSync);
}


SML_API void smlFreeGeneric(SmlGenericCmdPtr_t pGenericCmd)
{
  if (! pGenericCmd)
    return;

  smlFreePcdata(pGenericCmd->cmdID);
  smlFreePcdata(pGenericCmd->meta);

  smlFreeCredPtr(pGenericCmd->cred);

  smlFreeItemList(pGenericCmd->itemList);

  smlLibFree(pGenericCmd);
}


SML_API void smlFreeAlert(SmlAlertPtr_t pAlert)
{
  if (! pAlert)
    return;

  smlFreePcdata(pAlert->cmdID);
  smlFreePcdata(pAlert->data);

  smlFreeCredPtr(pAlert->cred);

  smlFreeItemList(pAlert->itemList);

  smlLibFree(pAlert);
}


SML_API void smlFreeAtomic(SmlAtomicPtr_t pAtomic)
{
  if (! pAtomic)
    return;

  smlFreePcdata(pAtomic->cmdID);
  smlFreePcdata(pAtomic->meta);

  smlLibFree(pAtomic);
}

#if (defined EXEC_SEND || defined EXEC_RECEIVE)

SML_API void smlFreeExec(SmlExecPtr_t pExec)
{
  if (! pExec)
    return;

  smlFreePcdata(pExec->cmdID);

  smlFreeCredPtr(pExec->cred);

  smlFreeItemPtr(pExec->item);

  smlLibFree(pExec);
}

#endif

SML_API void smlFreeGetPut(SmlPutPtr_t pGetPut)
{
  if (! pGetPut)
    return;

  smlFreePcdata(pGetPut->cmdID);
  smlFreePcdata(pGetPut->meta);
  smlFreePcdata(pGetPut->lang);

  smlFreeCredPtr(pGetPut->cred);

  smlFreeItemList(pGetPut->itemList);

  smlLibFree(pGetPut);
}


SML_API void smlFreeMap(SmlMapPtr_t pMap)
{
  if (! pMap)
    return;

  smlFreePcdata(pMap->cmdID);
  smlFreePcdata(pMap->meta);

  smlFreeCredPtr(pMap->cred);

  smlFreeSourceTargetPtr(pMap->source);
  smlFreeSourceTargetPtr(pMap->target);

  smlFreeMapItemList(pMap->mapItemList);

  smlLibFree(pMap);
}


SML_API void smlFreeResults(SmlResultsPtr_t pResults)
{
  if (! pResults)
    return;

  smlFreePcdata(pResults->cmdID);
  smlFreePcdata(pResults->msgRef);
  smlFreePcdata(pResults->cmdRef);
  smlFreePcdata(pResults->meta);
  smlFreePcdata(pResults->targetRef);
  smlFreePcdata(pResults->sourceRef);

  smlFreeItemList(pResults->itemList);

  smlLibFree(pResults);
}

#if (defined SEARCH_SEND || defined SEARCH_RECEIVE)

SML_API void smlFreeSearch(SmlSearchPtr_t pSearch)
{
  if (! pSearch)
    return;

  smlFreePcdata(pSearch->cmdID);
  smlFreePcdata(pSearch->lang);
  smlFreePcdata(pSearch->meta);
  smlFreePcdata(pSearch->data);

  smlFreeCredPtr(pSearch->cred);

  smlFreeSourceTargetPtr(pSearch->target);

  smlFreeSourceList(pSearch->sourceList);

  smlLibFree(pSearch);
}

#endif

SML_API void smlFreeStatus(SmlStatusPtr_t pStatus)
{
  if (! pStatus)
    return;

  smlFreePcdata(pStatus->cmdID);
  smlFreePcdata(pStatus->msgRef);
  smlFreePcdata(pStatus->cmdRef);
  smlFreePcdata(pStatus->cmd);
  smlFreePcdata(pStatus->data);

  smlFreeCredPtr(pStatus->cred);
  smlFreeChalPtr(pStatus->chal);

  smlFreeTargetRefList(pStatus->targetRefList);
  smlFreeSourceRefList(pStatus->sourceRefList);

  smlFreeItemList(pStatus->itemList);

  smlLibFree(pStatus);
}


SML_API void smlFreeCredPtr(SmlCredPtr_t pCred)
{
  if (! pCred)
    return;

  smlFreePcdata(pCred->meta);
  smlFreePcdata(pCred->data);

  smlLibFree(pCred);
}


SML_API void smlFreeChalPtr(SmlChalPtr_t pChal)
{
  if (! pChal)
    return;

  smlFreePcdata(pChal->meta);

  smlLibFree(pChal);
}



SML_API void smlFreeRecordFieldFilterPtr(SmlRecordOrFieldFilterPtr_t pRecordFieldFilter)
{
  if (! pRecordFieldFilter)
    return;

  smlFreeItemPtr(pRecordFieldFilter->item);
  smlLibFree(pRecordFieldFilter);
}


SML_API void smlFreeFilterPtr(SmlFilterPtr_t pFilter)
{
  if (! pFilter)
    return;

  smlFreePcdata(pFilter->meta);
  smlFreeRecordFieldFilterPtr(pFilter->field);
  smlFreeRecordFieldFilterPtr(pFilter->record);
  smlFreePcdata(pFilter->filtertype);
  smlLibFree(pFilter);
}


SML_API void smlFreeSourceTargetParentPtr(SmlSourceParentPtr_t pSourceTargetParent)
{
  if (! pSourceTargetParent)
    return;

  smlFreePcdata(pSourceTargetParent->locURI);
  smlLibFree(pSourceTargetParent);
}


SML_API void smlFreeSourceTargetPtr(SmlSourcePtr_t pSourceTarget)
{
  if (! pSourceTarget)
    return;

  smlFreePcdata(pSourceTarget->locURI);
  smlFreePcdata(pSourceTarget->locName);
  smlFreeFilterPtr(pSourceTarget->filter);

  smlLibFree(pSourceTarget);
}


SML_API void smlFreeSourceList(SmlSourceListPtr_t pSourceList)
{
  SmlSourceListPtr_t  pTmp;

  while (pSourceList) {
    pTmp = pSourceList->next;
    smlFreeSourceTargetPtr(pSourceList->source);
    smlLibFree(pSourceList);
    pSourceList = pTmp;
  }
}


SML_API void smlFreeSourceRefList(SmlSourceRefListPtr_t pSourceRefList)
{
  SmlSourceRefListPtr_t pTmp;

  while (pSourceRefList) {
    pTmp = pSourceRefList->next;
    smlFreePcdata(pSourceRefList->sourceRef);
    smlLibFree(pSourceRefList);
    pSourceRefList = pTmp;
  }
}


SML_API void smlFreeTargetRefList(SmlTargetRefListPtr_t pTargetRefList)
{
  SmlTargetRefListPtr_t pTmp;

  while (pTargetRefList) {
    pTmp = pTargetRefList->next;
    smlFreePcdata(pTargetRefList->targetRef);
    smlLibFree(pTargetRefList);
    pTargetRefList = pTmp;
  }
}


SML_API void smlFreeItemPtr(SmlItemPtr_t pItem)
{
  if (! pItem)
    return;

  smlFreePcdata(pItem->meta);
  smlFreePcdata(pItem->data);

  smlFreeSourceTargetPtr(pItem->source);
  smlFreeSourceTargetPtr(pItem->target);

  smlLibFree(pItem);
}


SML_API void smlFreeItemList(SmlItemListPtr_t pItemList)
{
  SmlItemListPtr_t  pTmp;

  while (pItemList) {
    pTmp = pItemList->next;
    smlFreeItemPtr(pItemList->item);
    smlLibFree(pItemList);
    pItemList = pTmp;
  }
}


SML_API void smlFreeMapItemPtr(SmlMapItemPtr_t pMapItem)
{
  if (! pMapItem)
    return;

  smlFreeSourceTargetPtr(pMapItem->source);
  smlFreeSourceTargetPtr(pMapItem->target);

  smlLibFree(pMapItem);
}


SML_API void smlFreeMapItemList(SmlMapItemListPtr_t pMapItemList)
{
  SmlMapItemListPtr_t pTmp;

  while (pMapItemList) {
    pTmp = pMapItemList->next;
    smlFreeMapItemPtr(pMapItemList->mapItem);
    smlLibFree(pMapItemList);
    pMapItemList = pTmp;
  }
}

#ifdef __USE_ALLOCFUNCS__
/* Helperfunctions, that allocate and preset SyncML C structs */
SML_API SmlPcdataPtr_t smlAllocPcdata() {
    SmlPcdataPtr_t p = (SmlPcdataPtr_t)smlLibMalloc(sizeof(SmlPcdata_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlPcdata_t));
    return p;
}

SML_API SmlPcdataListPtr_t smlAllocPcdataList() {
    SmlPcdataListPtr_t p = (SmlPcdataListPtr_t)smlLibMalloc(sizeof(SmlPcdataList_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlPcdataList_t));
    p->data = smlAllocPcdata();
    if (p->data == NULL) {
        smlFreePcdataList(p);
        return NULL;
    }
    return p;
}

SML_API SmlChalPtr_t smlAllocChal() {
    SmlChalPtr_t p = (SmlChalPtr_t)smlLibMalloc(sizeof(SmlChal_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlChal_t));
    p->meta = smlAllocPcdata();
    if (p->meta == NULL) {
        smlFreeChalPtr(p);
        return NULL;
    }
    return p;
}

SML_API SmlCredPtr_t smlAllocCred() {
    SmlCredPtr_t p = (SmlCredPtr_t)smlLibMalloc(sizeof(SmlCred_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlCred_t));
    p->data = smlAllocPcdata();
    if (p->data == NULL) {
        smlFreeCredPtr(p);
        return NULL;
    }
    return p;
}

SML_API SmlRecordOrFieldFilterPtr_t smlAllocRecordFieldFilter()
{
    SmlRecordOrFieldFilterPtr_t p = (SmlRecordOrFieldFilterPtr_t)smlLibMalloc(sizeof(SmlRecordOrFieldFilterPtr_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlRecordOrFieldFilterPtr_t));
    p->item = smlAllocItem();
    if (p->item == NULL) {
        smlFreeRecordFieldFilterPtr(p);
        return NULL;
    }
    return p;
}

SML_API SmlFilterPtr_t smlAllocFilter()
{
    SmlFilterPtr_t p = (SmlFilterPtr_t)smlLibMalloc(sizeof(SmlFilterPtr_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlFilterPtr_t));
    p->meta = smlAllocPcdata();
    if (p->meta == NULL) {
        smlFreeFilterPtr(p);
        return NULL;
    }
    return p;
}


SML_API SmlSourceParentPtr_t smlAllocSourceParent()
{
    SmlSourceParentPtr_t p = (SmlSourceParentPtr_t)smlLibMalloc(sizeof(SmlSourceParentPtr_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlSourceParentPtr_t));
    p->locURI = smlAllocPcdata();
    if (p->locURI == NULL) {
        smlFreeSourceTargetParentPtr(p);
        return NULL;
    }
    return p;
}


SML_API SmlTargetParentPtr_t smlAllocTargetParent()
{
    return smlAllocSourceParent();
}

SML_API SmlSourcePtr_t smlAllocSource() {
    SmlSourcePtr_t p = (SmlSourcePtr_t)smlLibMalloc(sizeof(SmlSource_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlSource_t));
    p->locURI = smlAllocPcdata();
    if (p->locURI == NULL) {
        smlFreeSourceTargetPtr(p);
        return NULL;
    }
    return p;
}


SML_API SmlTargetPtr_t smlAllocTarget() {
    return smlAllocSource();
}

SML_API SmlSourceListPtr_t smlAllocSourceList() {
    SmlSourceListPtr_t p = (SmlSourceListPtr_t)smlLibMalloc(sizeof(SmlSourceList_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlSourceList_t));
    p->source = smlAllocSource();
    if (p->source == NULL) {
        smlFreeSourceList(p);
        return NULL;
    }
    return p;
}


SML_API SmlSyncHdrPtr_t smlAllocSyncHdr() {
    SmlSyncHdrPtr_t p = (SmlSyncHdrPtr_t)smlLibMalloc(sizeof(SmlSyncHdr_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlSyncHdr_t));
    p->elementType = SML_PE_HEADER;
    p->version = smlAllocPcdata();
    if (p->version == NULL) {
        smlFreeSyncHdr(p);
        return NULL;
    }
    p->proto   = smlAllocPcdata();
    if (p->proto == NULL) {
        smlFreeSyncHdr(p);
        return NULL;
    }
    p->sessionID = smlAllocPcdata();
    if (p->sessionID == NULL) {
        smlFreeSyncHdr(p);
        return NULL;
    }
    p->msgID     = smlAllocPcdata();
    if (p->msgID == NULL) {
        smlFreeSyncHdr(p);
        return NULL;
    }
    p->target    = smlAllocTarget();
    if (p->target == NULL) {
        smlFreeSyncHdr(p);
        return NULL;
    }
    p->source    = smlAllocSource();
    if (p->source == NULL) {
        smlFreeSyncHdr(p);
        return NULL;
    }
    return p;
}

SML_API SmlItemPtr_t smlAllocItem() {
    SmlItemPtr_t p = (SmlItemPtr_t)smlLibMalloc(sizeof(SmlItem_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlItem_t));
    return p;
}

SML_API SmlItemListPtr_t smlAllocItemList() {
    SmlItemListPtr_t p = (SmlItemListPtr_t)smlLibMalloc(sizeof(SmlItemList_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlItemList_t));
    p->item = smlAllocItem();
    if (p->item == NULL) {
        smlFreeItemList(p);
        return NULL;
    }
    return p;
}

SML_API SmlGenericCmdPtr_t smlAllocGeneric() {
    SmlGenericCmdPtr_t p = (SmlGenericCmdPtr_t)smlLibMalloc(sizeof(SmlGenericCmd_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlGenericCmd_t));
    p->elementType = SML_PE_GENERIC;
    p->cmdID = smlAllocPcdata();
    if (p->cmdID == NULL) {
        smlFreeGeneric(p);
        return NULL;
    }
    p->itemList = smlAllocItemList();
    if (p->itemList == NULL) {
        smlFreeGeneric(p);
        return NULL;
    }
    return p;
}

SML_API SmlAddPtr_t smlAllocAdd() {
    SmlAddPtr_t p = smlAllocGeneric();
    if (p == NULL) return p;
    p->elementType = SML_PE_ADD;
    return p;
}

SML_API SmlCopyPtr_t smlAllocCopy() {
    SmlCopyPtr_t p = smlAllocGeneric();
    if (p == NULL) return p;
    p->elementType = SML_PE_COPY;
    return p;
}

SML_API SmlMovePtr_t smlAllocMove() {
    SmlCopyPtr_t p = smlAllocGeneric();
    if (p == NULL) return p;
    p->elementType = SML_PE_MOVE;
    return p;
}

SML_API SmlReplacePtr_t smlAllocReplace() {
    SmlReplacePtr_t p = smlAllocGeneric();
    if (p == NULL) return p;
    p->elementType = SML_PE_REPLACE;
    return p;
}

SML_API SmlDeletePtr_t smlAllocDelete() {
    SmlDeletePtr_t p = smlAllocGeneric();
    if (p == NULL) return p;
    p->elementType = SML_PE_DELETE;
    return p;
}

SML_API SmlAlertPtr_t smlAllocAlert() {
    SmlAlertPtr_t p = (SmlAlertPtr_t)smlLibMalloc(sizeof(SmlAlert_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlAlert_t));
    p->elementType = SML_PE_ALERT;
    p->cmdID = smlAllocPcdata();
    if (p->cmdID == NULL) {
        smlFreeAlert(p);
        return NULL;
    }
    p->itemList = smlAllocItemList();
    if (p->itemList == NULL) {
        smlFreeAlert(p);
        return NULL;
    }
    return p;
}

SML_API SmlAtomicPtr_t smlAllocAtomic() {
    SmlAtomicPtr_t p = (SmlAtomicPtr_t)smlLibMalloc(sizeof(SmlAtomic_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlAtomic_t));
    p->elementType = SML_PE_ATOMIC_START;
    p->cmdID = smlAllocPcdata();
    if (p->cmdID == NULL) {
        smlFreeAtomic(p);
        return NULL;
    }
    return p;
}

SML_API SmlSequencePtr_t smlAllocSequence() {
    SmlSequencePtr_t p = smlAllocAtomic();
    if (p == NULL) return NULL;
    p->elementType = SML_PE_SEQUENCE_START;
    return p;
}

SML_API SmlSyncPtr_t smlAllocSync() {
    SmlSyncPtr_t p = (SmlSyncPtr_t)smlLibMalloc(sizeof(SmlSync_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlSync_t));
    p->elementType = SML_PE_SYNC_START;
    p->cmdID = smlAllocPcdata();
    if (p->cmdID == NULL) {
        smlFreeSync(p);
        return NULL;
    }
    return p;
}

#if defined(EXEC_SEND) || defined(EXEC_RECEIVE)
SML_API SmlExecPtr_t smlAllocExec() {
    SmlExecPtr_t p = (SmlExecPtr_t)smlLibMalloc(sizeof(SmlExec_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlExec_t));
    p->elementType = SML_PE_EXEC;
    p->cmdID = smlAllocPcdata();
    if (p->cmdID == NULL) {
        smlFreeExec(p);
        return NULL;
    }
    p->item  = smlAllocItem();
    if (p->item == NULL) {
        smlFreeExec(p);
        return NULL;
    }
    return p;
}
#endif

SML_API SmlGetPtr_t smlAllocGet() {
    SmlGetPtr_t p = (SmlGetPtr_t)smlLibMalloc(sizeof(SmlGet_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlGet_t));
    p->elementType = SML_PE_GET;
    p->cmdID = smlAllocPcdata();
    if (p->cmdID == NULL) {
        smlFreeGetPut(p);
        return NULL;
    }
    p->itemList = smlAllocItemList();
    if (p->itemList == NULL) {
        smlFreeGetPut(p);
        return NULL;
    }
    return p;
}

SML_API SmlPutPtr_t smlAllocPut() {
    SmlPutPtr_t p = smlAllocGet();
    if (p == NULL) return NULL;
    p->elementType = SML_PE_PUT;
    return p;
}

SML_API SmlMapItemPtr_t smlAllocMapItem() {
    SmlMapItemPtr_t p = (SmlMapItemPtr_t)smlLibMalloc(sizeof(SmlMapItem_t));
    if (p == 0) return NULL;
    smlLibMemset(p, 0, sizeof(SmlMapItem_t));
    p->target = smlAllocTarget();
    if (p->target == NULL) {
        smlFreeMapItemPtr(p);
        return NULL;
    }
    p->source = smlAllocSource();
    if (p->source == NULL) {
        smlFreeMapItemPtr(p);
        return NULL;
    }
    return p;
}

SML_API SmlMapItemListPtr_t smlAllocMapItemList() {
    SmlMapItemListPtr_t p = (SmlMapItemListPtr_t)smlLibMalloc(sizeof(SmlMapItemList_t));
    if (p == 0) return NULL;
    smlLibMemset(p, 0, sizeof(SmlMapItemList_t));
    p->mapItem = smlAllocMapItem();
    if (p->mapItem == NULL) {
        smlFreeMapItemList(p);
        return NULL;
    }
    return p;
}

SML_API SmlMapPtr_t smlAllocMap() {
    SmlMapPtr_t p = (SmlMapPtr_t)smlLibMalloc(sizeof(SmlMap_t));
    if (p == 0) return NULL;
    smlLibMemset(p, 0, sizeof(SmlMap_t));
    p->elementType = SML_PE_MAP;
    p->cmdID = smlAllocPcdata();
    if (p->cmdID == NULL) {
        smlFreeMap(p);
        return NULL;
    }
    p->target = smlAllocTarget();
    if (p->target == NULL) {
        smlFreeMap(p);
        return NULL;
    }
    p->source = smlAllocSource();
    if (p->source == NULL) {
        smlFreeMap(p);
        return NULL;
    }
    p->mapItemList = smlAllocMapItemList();
    if (p->mapItemList == NULL) {
        smlFreeMap(p);
        return NULL;
    }
    return p;
}

SML_API SmlResultsPtr_t smlAllocResults() {
    SmlResultsPtr_t p = (SmlResultsPtr_t)smlLibMalloc(sizeof(SmlResults_t));
    if (p == 0) return NULL;
    smlLibMemset(p, 0, sizeof(SmlResults_t));
    p->elementType = SML_PE_RESULTS;
    p->cmdID = smlAllocPcdata();
    if (p->cmdID == NULL) {
        smlFreeResults(p);
        return NULL;
    }
    p->cmdRef = smlAllocPcdata();
    if (p->cmdRef == NULL) {
        smlFreeResults(p);
        return NULL;
    }
    p->itemList = smlAllocItemList();
    if (p->itemList == NULL) {
        smlFreeResults(p);
        return NULL;
    }
    return p;
}

#if (defined SEARCH_SEND || defined SEARCH_RECEIVE)

SML_API SmlSearchPtr_t smlAllocSearch() {
    SmlSearchPtr_t p = (SmlSearchPtr_t)smlLibMalloc(sizeof(SmlSearch_t));
    if (p == 0) return NULL;
    smlLibMemset(p, 0, sizeof(SmlSearch_t));
    p->elementType = SML_PE_SEARCH;
    p->cmdID = smlAllocPcdata();
    if (p->cmdID == NULL) {
        smlFreeSearch(p);
        return NULL;
    }
    p->meta  = smlAllocPcdata();
    if (p->meta == NULL) {
        smlFreeSearch(p);
        return NULL;
    }
    p->data  = smlAllocPcdata();
    if (p->data == NULL) {
        smlFreeSearch(p);
        return NULL;
    }
    p->sourceList = smlAllocSourceList();
    if (p->sourceList == NULL) {
        smlFreeSearch(p);
        return NULL;
    }
    return p;
}
#endif

SML_API SmlTargetRefListPtr_t smlAllocTargetRefList() {
    SmlTargetRefListPtr_t p = (SmlTargetRefListPtr_t)smlLibMalloc(sizeof(SmlTargetRefList_t));
    if (p == 0) return NULL;
    smlLibMemset(p, 0, sizeof(SmlTargetRefList_t));
    p->targetRef = smlAllocPcdata();
    if (p->targetRef == NULL) {
        smlFreeTargetRefList(p);
        return NULL;
    }
    return p;
}

SML_API SmlSourceRefListPtr_t smlAllocSourceRefList() {
    SmlSourceRefListPtr_t p = (SmlSourceRefListPtr_t)smlLibMalloc(sizeof(SmlSourceRefList_t));
    if (p == 0) return NULL;
    smlLibMemset(p, 0, sizeof(SmlSourceRefList_t));
    p->sourceRef = smlAllocPcdata();
    if (p->sourceRef == NULL) {
        smlFreeSourceRefList(p);
        return NULL;
    }
    return p;
}

SML_API SmlStatusPtr_t smlAllocStatus() {
    SmlStatusPtr_t p = (SmlStatusPtr_t)smlLibMalloc(sizeof(SmlStatus_t));
    if (p == 0) return NULL;
    smlLibMemset(p, 0, sizeof(SmlStatus_t));
    p->elementType = SML_PE_STATUS;
    p->cmdID = smlAllocPcdata();
    if (p->cmdID == NULL) {
        smlFreeStatus(p);
        return NULL;
    }
    p->msgRef = smlAllocPcdata();
    if (p->msgRef == NULL) {
        smlFreeStatus(p);
        return NULL;
    }
    p->cmdRef = smlAllocPcdata();
    if (p->cmdRef == NULL) {
        smlFreeStatus(p);
        return NULL;
    }
    p->cmd = smlAllocPcdata();
    if (p->cmd == NULL) {
        smlFreeStatus(p);
        return NULL;
    }
    p->data = smlAllocPcdata();
    if (p->data == NULL) {
        smlFreeStatus(p);
        return NULL;
    }
    return p;
}

SML_API SmlUnknownProtoElementPtr_t smlAllocUnknownProtoElement() {
    SmlUnknownProtoElementPtr_t p = (SmlUnknownProtoElementPtr_t)smlLibMalloc(sizeof(SmlUnknownProtoElement_t));
    if (p == 0) return NULL;
    smlLibMemset(p, 0, sizeof(SmlUnknownProtoElement_t));
    p->elementType = SML_PE_UNDEF;
    return p;
}

#ifdef __USE_METINF__
SML_API SmlMetInfMetInfPtr_t smlAllocMetInfMetInf() {
    SmlMetInfMetInfPtr_t p = (SmlMetInfMetInfPtr_t)smlLibMalloc(sizeof(SmlMetInfMetInf_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlMetInfMetInf_t));
    return p;
}

SML_API SmlMetInfAnchorPtr_t smlAllocMetInfAnchor() {
    SmlMetInfAnchorPtr_t p = (SmlMetInfAnchorPtr_t)smlLibMalloc(sizeof(SmlMetInfAnchor_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlMetInfAnchor_t));
    p->next = smlAllocPcdata();
    if (p->next == NULL) {
        smlFreeMetinfAnchor(p);
        return NULL;
    }
    return p;
}

SML_API SmlMetInfMemPtr_t smlAllocMetInfMem() {
    SmlMetInfMemPtr_t p = (SmlMetInfMemPtr_t)smlLibMalloc(sizeof(SmlMetInfMem_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlMetInfMem_t));
    p->free = smlAllocPcdata();
    if (p->free == NULL) {
        smlFreeMetinfMem(p);
        return NULL;
    }
    p->freeid = smlAllocPcdata();
    if (p->freeid == NULL) {
        smlFreeMetinfMem(p);
        return NULL;
    }
    return p;
}
#endif

#ifdef __USE_DEVINF__
SML_API SmlDevInfExtPtr_t smlAllocDevInfExt() {
    SmlDevInfExtPtr_t p = (SmlDevInfExtPtr_t)smlLibMalloc(sizeof(SmlDevInfExt_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfExt_t));
    p->xnam = smlAllocPcdata();
    if (p->xnam == NULL) {
        smlFreeDevInfExt(p);
        return NULL;
    }
    return p;
}

SML_API SmlDevInfExtListPtr_t smlAllocDevInfExtList() {
    SmlDevInfExtListPtr_t p = (SmlDevInfExtListPtr_t)smlLibMalloc(sizeof(SmlDevInfExtList_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfExtList_t));
    p->data = smlAllocDevInfExt();
    if (p->data == NULL) {
        smlFreeDevInfExtList(p);
        return NULL;
    }
    return p;
}

SML_API SmlDevInfSyncCapPtr_t smlAllocDevInfSyncCap() {
    SmlDevInfSyncCapPtr_t p = (SmlDevInfSyncCapPtr_t)smlLibMalloc(sizeof(SmlDevInfSyncCap_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfSyncCap_t));
    p->synctype = smlAllocPcdataList();
    if (p->synctype == NULL) {
        smlFreeDevInfSynccap(p);
        return NULL;
    }
    return p;
}

SML_API SmlDevInfCTDataPtr_t smlAllocDevInfCTData() {
    SmlDevInfCTDataPtr_t p = (SmlDevInfCTDataPtr_t)smlLibMalloc(sizeof(SmlDevInfCTData_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfCTData_t));
    p->name = smlAllocPcdata();
    if (p->name == NULL) {
        smlFreeDevInfCTData(p);
        return NULL;
    }
    return p;
}

SML_API SmlDevInfCTDataListPtr_t smlAllocDevInfCTDataList() {
    SmlDevInfCTDataListPtr_t p = (SmlDevInfCTDataListPtr_t)smlLibMalloc(sizeof(SmlDevInfCTDataList_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfCTDataList_t));
    p->data = smlAllocDevInfCTData();
    if (p->data == NULL) {
        smlFreeDevInfCTDataList(p);
        return NULL;
    }
    return p;
}

SML_API SmlDevInfCTDataPropPtr_t smlAllocDevInfCTDataProp() {
    SmlDevInfCTDataPropPtr_t p = (SmlDevInfCTDataPropPtr_t)smlLibMalloc(sizeof(SmlDevInfCTDataProp_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfCTDataProp_t));
    p->prop = smlAllocDevInfCTData();
    if (p->prop == NULL) {
        smlFreeDevInfCTDataProp(p);
        return NULL;
    }
    return p;
}

SML_API SmlDevInfCTDataPropListPtr_t smlAllocDevInfCTDataPropList() {
    SmlDevInfCTDataPropListPtr_t p = (SmlDevInfCTDataPropListPtr_t)smlLibMalloc(sizeof(SmlDevInfCTDataPropList_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfCTDataPropList_t));
    p->data = smlAllocDevInfCTDataProp();
    if (p->data == NULL) {
        smlFreeDevInfCTDataPropList(p);
        return NULL;
    }
    return p;
}

SML_API SmlDevInfCTCapPtr_t smlAllocDevInfCTCap() {
    SmlDevInfCTCapPtr_t p = (SmlDevInfCTCapPtr_t)smlLibMalloc(sizeof(SmlDevInfCTCap_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfCTCap_t));
    p->cttype = smlAllocPcdata();
    if (p->cttype == NULL) {
        smlFreeDevInfCTCap(p);
        return NULL;
    }
    p->prop = smlAllocDevInfCTDataPropList();
    if (p->prop == NULL) {
        smlFreeDevInfCTCap(p);
        return NULL;
    }
    return p;
}

SML_API SmlDevInfCtcapListPtr_t smlAllocDevInfCtcapList() {
    SmlDevInfCtcapListPtr_t p = (SmlDevInfCtcapListPtr_t)smlLibMalloc(sizeof(SmlDevInfCtcapList_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfCtcapList_t));
    p->data = smlAllocDevInfCTCap();
    if (p->data == NULL) {
        smlFreeDevInfCtcapList(p);
        return NULL;
    }
    return p;
}

SML_API SmlDevInfDSMemPtr_t smlAllocDevInfDSMem() {
    SmlDevInfDSMemPtr_t p = (SmlDevInfDSMemPtr_t)smlLibMalloc(sizeof(SmlDevInfDSMem_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfDSMem_t));
    return p;
}

SML_API SmlDevInfXmitPtr_t smlAllocDevInfXmit() {
    SmlDevInfXmitPtr_t p = (SmlDevInfXmitPtr_t)smlLibMalloc(sizeof(SmlDevInfXmit_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfXmit_t));
    p->cttype = smlAllocPcdata();
    if (p->cttype == NULL) {
        smlFreeDevInfXmit(p);
        return NULL;
    }
    p->verct = smlAllocPcdata();
    if (p->verct == NULL) {
        smlFreeDevInfXmit(p);
        return NULL;
    }
    return p;
}

SML_API SmlDevInfXmitListPtr_t smlAllocDevInfXmitList() {
    SmlDevInfXmitListPtr_t p = (SmlDevInfXmitListPtr_t)smlLibMalloc(sizeof(SmlDevInfXmitList_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfXmitList_t));
    p->data = smlAllocDevInfXmit();
    if (p->data == NULL) {
        smlFreeDevInfXmitList(p);
        return NULL;
    }
    return p;
}

SML_API SmlDevInfDatastorePtr_t smlAllocDevInfDatastore() {
    SmlDevInfDatastorePtr_t p = (SmlDevInfDatastorePtr_t)smlLibMalloc(sizeof(SmlDevInfDatastore_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfDatastore_t));
    p->sourceref = smlAllocPcdata();
    if (p->sourceref == NULL) {
        smlFreeDevInfDatastore(p);
        return NULL;
    }
    p->rxpref = smlAllocDevInfXmit();
    if (p->rxpref == NULL) {
        smlFreeDevInfDatastore(p);
        return NULL;
    }
    p->txpref = smlAllocDevInfXmit();
    if (p->txpref == NULL) {
        smlFreeDevInfDatastore(p);
        return NULL;
    }
    p->synccap = smlAllocDevInfSyncCap();
    if (p->synccap == NULL) {
        smlFreeDevInfDatastore(p);
        return NULL;
    }
    return p;
}

SML_API SmlDevInfDatastoreListPtr_t smlAllocDevInfDatastoreList() {
    SmlDevInfDatastoreListPtr_t p = (SmlDevInfDatastoreListPtr_t)smlLibMalloc(sizeof(SmlDevInfDatastoreList_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfDatastoreList_t));
    p->data = smlAllocDevInfDatastore();
    if (p->data == NULL) {
        smlFreeDevInfDatastoreList(p);
        return NULL;
    }
    return p;
}

SML_API SmlDevInfDevInfPtr_t smlAllocDevInfDevInf() {
    SmlDevInfDevInfPtr_t p = (SmlDevInfDevInfPtr_t)smlLibMalloc(sizeof(SmlDevInfDevInf_t));
    if (p == NULL) return NULL;
    smlLibMemset(p, 0, sizeof(SmlDevInfDevInf_t));
    p->verdtd = smlAllocPcdata();
    if (p->verdtd == NULL) {
        smlFreeDevInfDevInf(p);
        return NULL;
    }
    p->devid = smlAllocPcdata();
    if (p->devid == NULL) {
        smlFreeDevInfDevInf(p);
        return NULL;
    }
    p->devtyp = smlAllocPcdata();
    if (p->devtyp == NULL) {
        smlFreeDevInfDevInf(p);
        return NULL;
    }
    p->datastore = smlAllocDevInfDatastoreList();
    if (p->verdtd == NULL) {
        smlFreeDevInfDevInf(p);
        return NULL;
    }
    p->ctcap = smlAllocDevInfCtcapList();
    if (p->ctcap == NULL) {
        smlFreeDevInfDevInf(p);
        return NULL;
    }
    p->ext = smlAllocDevInfExtList();
    if (p->ext == NULL) {
        smlFreeDevInfDevInf(p);
        return NULL;
    }

    return p;
}

#endif // DevInf
#endif // AllocFuncs
