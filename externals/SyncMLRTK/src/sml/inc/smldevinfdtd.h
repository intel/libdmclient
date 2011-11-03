/**
 * @file
 * SyncML Device Information DTD specific type definitions
 *
 * @target_system   all
 * @target_os       all
 * @description Definition of structures representing DevInf DTD elements
 * This file reflects DevInf as specified in the document
 * http://www.openmobilealliance.org/tech/affiliates/syncml/syncml_devinf_v101_20010615.pdf
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



#ifndef _SML_DEVINFDTD_H
#define _SML_DEVINFDTD_H

/* process only if we really use DevInf DTD */
#ifdef __USE_DEVINF__

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/


#include <smldef.h>
#include <smldtd.h>


typedef struct sml_devinf_ext_s {
    SmlPcdataPtr_t      xnam;
    SmlPcdataListPtr_t  xval; /* optional */
} *SmlDevInfExtPtr_t, SmlDevInfExt_t;

typedef struct sml_devinf_extlist_s {
    SmlDevInfExtPtr_t   data;
    struct sml_devinf_extlist_s *next;
} *SmlDevInfExtListPtr_t, SmlDevInfExtList_t;

typedef struct sml_devinf_synccap_s {
    SmlPcdataListPtr_t  synctype;
} *SmlDevInfSyncCapPtr_t, SmlDevInfSyncCap_t;


typedef struct sml_devinf_ctdata_s {
    SmlPcdataPtr_t  name;
    SmlPcdataPtr_t  dname; /* optional, display name */

    SmlPcdataListPtr_t  valenum;
    SmlPcdataPtr_t  datatype;
    SmlPcdataPtr_t  maxsize; // called size before DS 1.2 (still represents devInf <Size> for DS 1.1/1.0)
    /* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
    SmlPcdataPtr_t  maxoccur;
    Flag_t          flags;  //

} *SmlDevInfCTDataPtr_t, SmlDevInfCTData_t;

typedef struct sml_devinf_ctdatalist_s {
    SmlDevInfCTDataPtr_t       data;
    struct sml_devinf_ctdatalist_s *next;
} *SmlDevInfCTDataListPtr_t, SmlDevInfCTDataList_t;

typedef struct sml_devinf_ctdataprop_s {
    SmlDevInfCTDataPtr_t    prop;
    SmlDevInfCTDataListPtr_t param;
} *SmlDevInfCTDataPropPtr_t, SmlDevInfCTDataProp_t;

typedef struct sml_devinf_ctdataproplist_s {
    SmlDevInfCTDataPropPtr_t    data;
    struct sml_devinf_ctdataproplist_s *next;
} *SmlDevInfCTDataPropListPtr_t, SmlDevInfCTDataPropList_t;

/* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
typedef struct sml_devinf_filtercap_s {
    SmlPcdataPtr_t  cttype;
    SmlPcdataPtr_t  verct;
    SmlPcdataListPtr_t filterkeyword;
    SmlPcdataListPtr_t propname;
} *SmlDevInfFilterCapPtr_t, SmlDevInfFilterCap_t;

/* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
typedef struct sml_devinf_filtercaplist_s {
    SmlDevInfFilterCapPtr_t data;
    struct sml_devinf_filtercaplist_s *next;
} *SmlDevInfFilterCapListPtr_t, SmlDevInfFilterCapList_t;

typedef struct sml_devinf_ctcap_s {
    SmlPcdataPtr_t                cttype;
    SmlPcdataPtr_t                verct;         // new for DS 1.2
    SmlDevInfCTDataPropListPtr_t  prop;
    Flag_t                        flags;         // SmlDevInfFieldLevel_f, opt.
} *SmlDevInfCTCapPtr_t, SmlDevInfCTCap_t;

typedef struct sml_devinf_ctcaplist_s {
    SmlDevInfCTCapPtr_t data;
    struct sml_devinf_ctcaplist_s *next;
} *SmlDevInfCtcapListPtr_t, SmlDevInfCtcapList_t;


typedef struct sml_devinf_dsmem_s {
    SmlPcdataPtr_t  maxmem; /* optional */
    SmlPcdataPtr_t  maxid;  /* optional */
    Flag_t  flags; /* %%% luz:2003-04-28, made sharedMem a flag, was PCData (completely wrong) */
} *SmlDevInfDSMemPtr_t, SmlDevInfDSMem_t;


typedef struct sml_devinf_xmit_s {
    SmlPcdataPtr_t  cttype;
    SmlPcdataPtr_t  verct;
} *SmlDevInfXmitPtr_t, SmlDevInfXmit_t;

typedef struct sml_devinf_xmitlist_s {
    SmlDevInfXmitPtr_t      data;
    struct sml_devinf_xmitlist_s  *next;
} *SmlDevInfXmitListPtr_t, SmlDevInfXmitList_t;

typedef struct sml_devinf_datastore_s {
    SmlPcdataPtr_t          sourceref;
    SmlPcdataPtr_t          displayname;   /* optional */
    SmlPcdataPtr_t          maxguidsize;   /* optional */
    SmlDevInfXmitPtr_t      rxpref;
    SmlDevInfXmitListPtr_t  rx;            /* optional */
    SmlDevInfXmitPtr_t      txpref;
    SmlDevInfXmitListPtr_t  tx;            /* optional */
    SmlDevInfDSMemPtr_t     dsmem;         /* optional */
    SmlDevInfSyncCapPtr_t   synccap;
    /* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
    SmlDevInfXmitListPtr_t  filterrx;      /* optional */
    SmlDevInfCtcapListPtr_t ctcap;         // opt., Datastore-local >=DS1.2 type list
    SmlDevInfFilterCapListPtr_t filtercap; // opt., filter capabilities list
    Flag_t                  flags;         // SmlDevInfHierarchical_f, opt.
} *SmlDevInfDatastorePtr_t, SmlDevInfDatastore_t;


typedef struct sml_devinf_datastorelist_s {
    SmlDevInfDatastorePtr_t data;
    struct sml_devinf_datastorelist_s *next;
} *SmlDevInfDatastoreListPtr_t, SmlDevInfDatastoreList_t;

typedef struct sml_devinf_devinf_s {
  SmlPcdataPtr_t      verdtd;
  SmlPcdataPtr_t      man; /* optional */
  SmlPcdataPtr_t      mod; /* optional */
  SmlPcdataPtr_t      oem; /* optional */
  SmlPcdataPtr_t      fwv; /* optional */
  SmlPcdataPtr_t      swv; /* optional */
  SmlPcdataPtr_t      hwv; /* optional */
  SmlPcdataPtr_t      devid;
  SmlPcdataPtr_t      devtyp;
  SmlDevInfDatastoreListPtr_t datastore;
  SmlDevInfCtcapListPtr_t     ctcap; // opt., global pre-DS 1.2 type list
  SmlDevInfExtListPtr_t       ext;
  /* SCTSTK - 18/03/2002, S.H. 2002-04-05 : SyncML 1.1 */
  Flag_t        flags;
} *SmlDevInfDevInfPtr_t, SmlDevInfDevInf_t;

#endif // __USE_DEVINF__
#endif //_SML_DEVINFDTD_H_
