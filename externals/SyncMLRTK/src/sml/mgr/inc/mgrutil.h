/**
 * @file
 * SyncML API for freeing SyncML C structures
 *
 * @target_system   all
 * @target_os       all
 * @description Definitions for internal use within the SyncML implementation
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



#ifndef _MGR_UTIL_H
  #define _MGR_UTIL_H


/* Prototypes of exported SyncML API functions */
SML_API Ret_t smlFreeProtoElement(VoidPtr_t pProtoElement) MGR_FUNC;
SML_API void smlFreePcdata(SmlPcdataPtr_t pPcdata) MGR_FUNC;
SML_API void smlFreePcdataList(SmlPcdataListPtr_t list) MGR_FUNC;

SML_API void smlFreeSyncHdr(SmlSyncHdrPtr_t pSyncHdr) MGR_FUNC;
SML_API void smlFreeSync(SmlSyncPtr_t pSync) MGR_FUNC;
SML_API void smlFreeGeneric(SmlGenericCmdPtr_t pGenericCmd) MGR_FUNC;
SML_API void smlFreeAlert(SmlAlertPtr_t pAlert) MGR_FUNC;
SML_API void smlFreeAtomic(SmlAtomicPtr_t pAtomic) MGR_FUNC;
#if (defined EXEC_SEND || defined EXEC_RECEIVE)
  SML_API void smlFreeExec(SmlExecPtr_t pExec) MGR_FUNC;
#endif
SML_API void smlFreeGetPut(SmlPutPtr_t pGetPut) MGR_FUNC;
SML_API void smlFreeMap(SmlMapPtr_t pMap) MGR_FUNC;
SML_API void smlFreeResults(SmlResultsPtr_t pResults) MGR_FUNC;
#if (defined SEARCH_SEND || defined SEARCH_RECEIVE)
  SML_API void smlFreeSearch(SmlSearchPtr_t pSearch) MGR_FUNC;
#endif
SML_API void smlFreeStatus(SmlStatusPtr_t pStatus) MGR_FUNC;
SML_API void smlFreeCredPtr(SmlCredPtr_t pCred) MGR_FUNC;
SML_API void smlFreeChalPtr(SmlChalPtr_t pChal) MGR_FUNC;
SML_API void smlFreeRecordFieldFilterPtr(SmlRecordOrFieldFilterPtr_t pRecordFieldFilter) MGR_FUNC;
SML_API void smlFreeFilterPtr(SmlFilterPtr_t pFilter) MGR_FUNC;
SML_API void smlFreeSourceTargetParentPtr(SmlSourceParentPtr_t pSourceTargetParent) MGR_FUNC;
SML_API void smlFreeSourceTargetPtr(SmlSourcePtr_t pSourceTarget) MGR_FUNC;
SML_API void smlFreeSourceList(SmlSourceListPtr_t pSourceList) MGR_FUNC;
SML_API void smlFreeSourceRefList(SmlSourceRefListPtr_t pSourceRefList) MGR_FUNC;
SML_API void smlFreeTargetRefList(SmlTargetRefListPtr_t pTargetRefList) MGR_FUNC;
SML_API void smlFreeItemPtr(SmlItemPtr_t pItem) MGR_FUNC;
SML_API void smlFreeItemList(SmlItemListPtr_t pItemList) MGR_FUNC;
SML_API void smlFreeMapItemPtr(SmlMapItemPtr_t pMapItem) MGR_FUNC;
SML_API void smlFreeMapItemList(SmlMapItemListPtr_t pMapItemList) MGR_FUNC;

#ifdef __USE_METINF__
SML_API void smlFreeMetinfAnchor(SmlMetInfAnchorPtr_t data) MGR_FUNC;
SML_API void smlFreeMetinfMem(SmlMetInfMemPtr_t data) MGR_FUNC;
SML_API void smlFreeMetinfMetinf(SmlMetInfMetInfPtr_t data) MGR_FUNC;
#endif
#ifdef __USE_DEVINF__
SML_API void smlFreeDevInfDatastore(SmlDevInfDatastorePtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfDatastoreList(SmlDevInfDatastoreListPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfXmitList(SmlDevInfXmitListPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfXmit(SmlDevInfXmitPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfDSMem(SmlDevInfDSMemPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfSynccap(SmlDevInfSyncCapPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfExt(SmlDevInfExtPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfExtList(SmlDevInfExtListPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfCTData(SmlDevInfCTDataPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfCTDataList(SmlDevInfCTDataListPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfCTDataProp(SmlDevInfCTDataPropPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfCTDataPropList(SmlDevInfCTDataPropListPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfCTCap(SmlDevInfCTCapPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfCtcapList(SmlDevInfCtcapListPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfFilterCap(SmlDevInfFilterCapPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfFilterCapList(SmlDevInfFilterCapListPtr_t data) MGR_FUNC;
SML_API void smlFreeDevInfDevInf(SmlDevInfDevInfPtr_t data) MGR_FUNC;
#endif
#ifndef __SML_LITE__  /* these API calls are NOT included in the Toolkit lite version */
SML_API String_t smlPcdata2String( SmlPcdataPtr_t pcdata ) MGR_FUNC;
SML_API SmlPcdataPtr_t smlString2Pcdata( String_t str ) MGR_FUNC;
SML_API SmlPcdataPtr_t smlPcdataDup(SmlPcdataPtr_t pcdata) MGR_FUNC;
SML_API MemSize_t smlGetFreeBuffer(InstanceID_t id) MGR_FUNC;
#endif

#ifdef __USE_ALLOCFUNCS__
SML_API SmlPcdataPtr_t smlAllocPcdata() MGR_FUNC;
SML_API SmlPcdataListPtr_t smlAllocPcdataList() MGR_FUNC;
SML_API SmlChalPtr_t smlAllocChal() MGR_FUNC;
SML_API SmlCredPtr_t smlAllocCred() MGR_FUNC;
SML_API SmlRecordOrFieldFilterPtr_t smlAllocRecordFieldFilter() MGR_FUNC;
SML_API SmlFilterPtr_t smlAllocFilter() MGR_FUNC;
SML_API SmlSourceParentPtr_t smlAllocSourceParent() MGR_FUNC;
SML_API SmlTargetParentPtr_t smlAllocTargetParent() MGR_FUNC;
SML_API SmlSourcePtr_t smlAllocSource() MGR_FUNC;
SML_API SmlTargetPtr_t smlAllocTarget() MGR_FUNC;
SML_API SmlSourceListPtr_t smlAllocSourceList() MGR_FUNC;
SML_API SmlSyncHdrPtr_t smlAllocSyncHdr() MGR_FUNC;
SML_API SmlItemPtr_t smlAllocItem() MGR_FUNC;
SML_API SmlItemListPtr_t smlAllocItemList() MGR_FUNC;
SML_API SmlGenericCmdPtr_t smlAllocGeneric() MGR_FUNC;
SML_API SmlAddPtr_t smlAllocAdd() MGR_FUNC;
SML_API SmlCopyPtr_t smlAllocCopy() MGR_FUNC;
SML_API SmlMovePtr_t smlAllocMove() MGR_FUNC;
SML_API SmlReplacePtr_t smlAllocReplace() MGR_FUNC;
SML_API SmlDeletePtr_t smlAllocDelete() MGR_FUNC;
SML_API SmlAlertPtr_t smlAllocAlert() MGR_FUNC;
SML_API SmlAtomicPtr_t smlAllocAtomic() MGR_FUNC;
SML_API SmlSequencePtr_t smlAllocSequence() MGR_FUNC;
SML_API SmlSyncPtr_t smlAllocSync() MGR_FUNC;
SML_API SmlExecPtr_t smlAllocExec() MGR_FUNC;
SML_API SmlGetPtr_t smlAllocGet() MGR_FUNC;
SML_API SmlPutPtr_t smlAllocPut() MGR_FUNC;
SML_API SmlMapItemPtr_t smlAllocMapItem() MGR_FUNC;
SML_API SmlMapItemListPtr_t smlAllocMapItemList() MGR_FUNC;
SML_API SmlMapPtr_t smlAllocMap() MGR_FUNC;
SML_API SmlResultsPtr_t smlAllocResults() MGR_FUNC;
SML_API SmlSearchPtr_t smlAllocSearch() MGR_FUNC;
SML_API SmlTargetRefListPtr_t smlAllocTargetRefList() MGR_FUNC;
SML_API SmlSourceRefListPtr_t smlAllocSourceRefList() MGR_FUNC;
SML_API SmlStatusPtr_t smlAllocStatus() MGR_FUNC;
SML_API SmlUnknownProtoElementPtr_t smlAllocUnknownProtoElement() MGR_FUNC;
#ifdef __USE_METINF__
SML_API SmlMetInfMetInfPtr_t smlAllocMetInfMetInf() MGR_FUNC;
SML_API SmlMetInfAnchorPtr_t smlAllocMetInfAnchor() MGR_FUNC;
SML_API SmlMetInfMemPtr_t smlAllocMetInfMem() MGR_FUNC;
#endif // MetInf

#ifdef __USE_DEVINF__
SML_API SmlDevInfExtPtr_t smlAllocDevInfExt() MGR_FUNC;
SML_API SmlDevInfExtListPtr_t smlAllocDevInfExtList() MGR_FUNC;
SML_API SmlDevInfSyncCapPtr_t smlAllocDevInfSyncCap() MGR_FUNC;
SML_API SmlDevInfCTDataPtr_t smlAllocDevInfCTData() MGR_FUNC;
SML_API SmlDevInfCTDataListPtr_t smlAllocDevInfCTDataList() MGR_FUNC;
SML_API SmlDevInfCTDataPropPtr_t smlAllocDevInfCTDataProp() MGR_FUNC;
SML_API SmlDevInfCTDataPropListPtr_t smlAllocDevInfCTDataPropList() MGR_FUNC;
SML_API SmlDevInfCTCapPtr_t smlAllocDevInfCTCap() MGR_FUNC;
SML_API SmlDevInfCtcapListPtr_t smlAllocDevInfCtcapList() MGR_FUNC;
SML_API SmlDevInfDSMemPtr_t smlAllocDevInfDSMem() MGR_FUNC;
SML_API SmlDevInfXmitPtr_t smlAllocDevInfXmit() MGR_FUNC;
SML_API SmlDevInfXmitListPtr_t smlAllocDevInfXmitList() MGR_FUNC;
SML_API SmlDevInfDatastorePtr_t smlAllocDevInfDatastore() MGR_FUNC;
SML_API SmlDevInfDatastoreListPtr_t smlAllocDevInfDatastoreList() MGR_FUNC;
SML_API SmlDevInfDevInfPtr_t smlAllocDevInfDevInf() MGR_FUNC;
#endif // DevInf
#endif // AllocFuncs
#endif // MgrUtil.h
