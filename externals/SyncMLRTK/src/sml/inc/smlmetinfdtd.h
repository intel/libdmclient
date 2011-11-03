/**
 * @file
 * SyncML Meta Information DTD specific type definitions
 *
 * @target_system   all
 * @target_os       all
 * @description Definition of structures representing MetInf DTD elements
 */


/**************************************************************************/
/* @note                                                                  */
/* These definitions are based on the MetInf DTD dated from Aug, 29th, 00 */
/**************************************************************************/



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

#ifndef _SML_METINFDTD_H
#define _SML_METINFDTD_H

/* process only if we really use MetInf DTD */
#ifdef __USE_METINF__

/*************************************************************************/
/*  Definitions                                                          */
/*************************************************************************/


#include <smldef.h>
#include <smldtd.h>


typedef struct sml_metinf_anchor_s {
  SmlPcdataPtr_t  last; /* optional */
  SmlPcdataPtr_t  next;
} *SmlMetInfAnchorPtr_t, SmlMetInfAnchor_t;

typedef struct sml_metinf_mem_s {
  /* %%% luz 2005-08-24 :this definition was plain wrong - shared is a flag, not Pcdata!
  SmlPcdataPtr_t  shared; */
  SmlPcdataPtr_t  free;
  SmlPcdataPtr_t  freeid;
  /* Added by Synthesis/luz 2005-08-24, was wrong in toolkit up to now! */
  Flag_t          flags;    // SharedMem flag (SmlMetInfSharedMem_f)
} *SmlMetInfMemPtr_t, SmlMetInfMem_t;

typedef struct sml_metinf_metinf_s {
  SmlPcdataPtr_t      format;    /* opt. */
  SmlPcdataPtr_t      type;      /* opt. */
  SmlPcdataPtr_t      mark;      /* opt. */
  SmlPcdataPtr_t      size;      /* opt. */
  SmlPcdataPtr_t      nextnonce; /* opt. */
  SmlPcdataPtr_t      version;
  SmlPcdataPtr_t      maxmsgsize; /* optional */
  /* SCTSTK - 18/03/2002, S.H. 2002-04-05 : SyncML 1.1 */
  SmlPcdataPtr_t      maxobjsize; /* optional */
  SmlMetInfMemPtr_t   mem;  /* optional */
  SmlPcdataListPtr_t  emi; /* optional */
  SmlMetInfAnchorPtr_t anchor; /* opt. */
  /* SyncML DS 1.2, Synthesis/luz 2005-08-24 */
  Flag_t              flags;      // FieldLevel flag (SmlMetInfFieldLevel_f)
} *SmlMetInfMetInfPtr_t, SmlMetInfMetInf_t;

#endif    /* __USE_METINF__ */
#endif    /* _SML_METINFDTD_H */
