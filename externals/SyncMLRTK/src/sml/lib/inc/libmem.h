/**
 * @file
 * Library for Memory Functions
 *
 * @target_system   ALL
 * @target_os       ALL
 * @description Header for the implementation of common memory handling functions
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



#ifndef _LIB_MEM_H
#define _LIB_MEM_H



/*************************************************************************
 *  Definitions
 *************************************************************************/

#include "define.h"

#include <stdlib.h>
#include <smldef.h>
#ifdef __ANSI_C__
#include <string.h>
#endif
#ifdef __PALM_OS__
#include <MemoryMgr.h>
#endif


/*************************************************************************
 *  External Functions for all Toolkit versions
 *************************************************************************/


#ifdef __PALM_OS__  /* we use #define to reduce heap usage */
 void *smlLibRealloc (VoidPtr_t objectP, MemSize_t constSize) LIB_FUNC;
 void smlLibFree (void* objectP) LIB_FUNC;
 #define  smlLibMemset(pObject,value,count)   ((void)MemSet((VoidPtr_t)pObject,(MemSize_t)count,(int)value))
 #define  smlLibMemcpy(pTarget,pSource,count)   (MemMove(pTarget,(VoidPtr_t)pSource,count) ? pTarget : pTarget)
 #define  smlLibMemmove(pTarget,pSource,count)    (MemMove(pTarget,(VoidPtr_t)pSource,(MemSize_t)count) ? pTarget : pTarget)
 #define  smlLibMemcmp(pTarget,pSource,count)   (MemCmp((VoidPtr_t)pTarget,(VoidPtr_t)pSource,(MemSize_t)count))
 #define  smlLibMalloc(size)    ((VoidPtr_t)MemPtrNew((MemSize_t)size))
 #define  smlLibMemsize(pObject)    ((MemSize_t)MemPtrSize((VoidPtr_t)pObject))
#else
  SML_API_DEF void  *smlLibRealloc(void *pObject, MemSize_t size) LIB_FUNC;
  SML_API_DEF void  smlLibFree(void *pObject) LIB_FUNC;
  SML_API_DEF void  *smlLibMemset(void *pObject, int value, MemSize_t count) LIB_FUNC;
  SML_API_DEF void  *smlLibMemcpy(void *pTarget, const void *pSource, MemSize_t count) LIB_FUNC;
  SML_API_DEF void  *smlLibMemmove(void *pTarget, const void *pSource, MemSize_t count) LIB_FUNC;
  SML_API_DEF int   smlLibMemcmp(const void *pTarget, const void *pSource, MemSize_t count) LIB_FUNC;
  // original:  SML_API_DEF void  *smlLibMalloc(MemSize_t size) LIB_FUNC;
  #define smlLibMalloc(m) malloc(m)
#endif


#endif

