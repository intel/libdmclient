/**
 * @file
 * Managing SyncML
 *
 * @target_system   all
 * @target_os       all
 * @description Core Module managing the life-cycle of a syncML Process itself
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
#include <smldef.h>
#include <smlerr.h>

#if defined(NOWSM) && !__LINK_TOOLKIT_STATIC__
// we need dummies of these as they are listed in the SyncML.def file
SML_API_DEF Ret_t smlInit(SmlOptionsPtr_t pOptions) { return SML_ERR_OK; }
SML_API_DEF Ret_t smlSetSyncMLOptions (SmlOptionsPtr_t pOptions) { return SML_ERR_OK; }
SML_API_DEF Ret_t smlTerminate(void) { return SML_ERR_OK; }
#endif

#ifndef NOWSM

#include "libmem.h"
#include "liblock.h"
#include "wsm.h"
#include "mgr.h"

#ifdef __EPOC_OS__
#include "core_globals_epoc.h"
#endif

/* Prototypes of exported SyncML API functions */
SML_API Ret_t smlInit(SmlOptionsPtr_t pOptions);
SML_API Ret_t smlSetSyncMLOptions (SmlOptionsPtr_t pOptions);
SML_API Ret_t smlTerminate(void);

/* SyncML internal function prototypes */
InstanceInfoPtr_t mgrGetInstanceListAnchor(void);
void mgrSetInstanceListAnchor(InstanceInfoPtr_t newListAnchor);
SyncMLInfoPtr_t mgrGetSyncMLAnchor(void);


/**
 * Anchor of the global syncML info structure
 */
#ifndef __EPOC_OS__
static SyncMLInfoPtr_t     pGlobalAnchor=NULL; // this global pointer is used to access ALL globals within syncml
#endif                                         // This is the ONLY global varible of SyncML!

#ifdef __EPOC_OS__
#define pGlobalAnchor TheCoreGlobalsEpoc()->pGlobalAnchor
#endif

/*************************************************************************
 *  Exported SyncML API functions
 *************************************************************************/




/**
 * Initializes the SyncML Reference Tookit. This is required, before any
 * other function can be used.
 *
 * @param pCoreOptions (IN)
 *        options to be applied for the toolkit
 * @return Return Code
 */
SML_API Ret_t smlInit(SmlOptionsPtr_t pCoreOptions)
{

  /* ---- Definitions --- */
  WsmOptions_t*      pWorkspaceOptions;
  Ret_t              rc;


  /* --- check, if SyncML has already been initialized --- */
  if (pGlobalAnchor!=NULL) return SML_ERR_ALREADY_INITIALIZED;

  /* --- Check pOptions, which have been passed by the application --- */
  if (!pCoreOptions)
    return SML_ERR_WRONG_USAGE;


  /* --- Create a SyncML info memory object to store all globals --- */
  TOOLKITLOCK_INIT("smlInit");
  pGlobalAnchor = (SyncMLInfoPtr_t)smlLibMalloc((MemSize_t)sizeof(SyncMLInfo_t));
  if (pGlobalAnchor==NULL) return SML_ERR_NOT_ENOUGH_SPACE;
  smlLibMemset(pGlobalAnchor,0,(MemSize_t)sizeof(SyncMLInfo_t));


  /* --- Set SyncML settings and options  --- */
  pGlobalAnchor->instanceListAnchor = NULL;  // no instance exists at the beginning
  rc = smlSetSyncMLOptions (pCoreOptions);   // store the options in the global structure
  if (rc!=SML_ERR_OK){
     smlLibFree(pGlobalAnchor);
     pGlobalAnchor = NULL;
     return rc;
  }

  pGlobalAnchor->tokTbl = (TokenInfoPtr_t)smlLibMalloc(sizeof(TokenInfo_t));
  if (pGlobalAnchor->tokTbl == NULL)
  {
   smlLibFree(pGlobalAnchor);
   return SML_ERR_NOT_ENOUGH_SPACE;
   }
  smlLibMemset(pGlobalAnchor->tokTbl, 0, sizeof(TokenInfo_t));
  /* --- Init all modules ---*/

  /* Init Workspace Module */
  pWorkspaceOptions=(WsmOptions_t*)smlLibMalloc((MemSize_t)sizeof(WsmOptions_t)); // create workspace options
  if (pWorkspaceOptions == NULL) {
      smlLibFree(pGlobalAnchor->syncmlOptions);
      smlLibFree(pGlobalAnchor->tokTbl);
      smlLibFree(pGlobalAnchor);
      pGlobalAnchor = NULL;
      return SML_ERR_NOT_ENOUGH_SPACE;
  }

  smlLibMemset(pWorkspaceOptions,0,(MemSize_t)sizeof(WsmOptions_t));
  pWorkspaceOptions->maxAvailMem=(MemSize_t)pGlobalAnchor->syncmlOptions->maxWorkspaceAvailMem;

  rc = wsmInit (pWorkspaceOptions);
  if (rc!=SML_ERR_OK){
    smlLibFree(pGlobalAnchor->syncmlOptions);
    smlLibFree(pGlobalAnchor->tokTbl);
    smlLibFree(pGlobalAnchor);
    pGlobalAnchor = NULL;
    smlLibFree(pWorkspaceOptions);
    return rc;
  }
  smlLibFree(pWorkspaceOptions);
  return SML_ERR_OK;
}


/**
 * Terminate SyncML. Frees all memory and other ressources used by
 * SyncML. This function must be called when terminating SyncML
 *
 * @pre All instances must have been terminated
 * @return Return Code
 */
SML_API Ret_t smlTerminate(void) {
  // Have all Instances been terminated?
  if (pGlobalAnchor->instanceListAnchor!=NULL)
    return SML_ERR_WRONG_USAGE;

  /* --- Make sure, the workspace is destroyed --*/
  LOCKTOOLKIT("smlTerminate");
  wsmTerminate();

  /* --- Free the global structure --*/
  smlLibFree(pGlobalAnchor->tokTbl->SyncML);
  smlLibFree(pGlobalAnchor->tokTbl->MetInf);
  smlLibFree(pGlobalAnchor->tokTbl->DevInf);
  smlLibFree(pGlobalAnchor->tokTbl);
  smlLibFree(pGlobalAnchor->syncmlOptions);
  smlLibFree(pGlobalAnchor);
  pGlobalAnchor=NULL;

  TOOLKITLOCK_FREE("smlTerminate");

  return SML_ERR_OK;

}



/**
 * Change the option settings for syncML
 *
 * @param pCoreOptions (IN)
 *        options to be applied for the toolkit
 * @return Return Code
 */
SML_API Ret_t smlSetSyncMLOptions(SmlOptionsPtr_t pCoreOptions) {


  /* ---- Definitions --- */
  SmlOptionsPtr_t pCoreOptionsCopy;


  /* --- Check pOptions, which have been passed by the application --- */
  if (!pCoreOptions)
    return SML_ERR_WRONG_USAGE;


  /* --- free SyncML options --- */
  smlLibFree(pGlobalAnchor->syncmlOptions);
  pGlobalAnchor->syncmlOptions = NULL;
  /* --- Use a copy of pCoreOptions --- */
  pCoreOptionsCopy =  (SmlOptionsPtr_t)smlLibMalloc((MemSize_t)sizeof(SmlOptions_t));
  if (pCoreOptionsCopy==NULL) return SML_ERR_NOT_ENOUGH_SPACE;
  smlLibMemcpy(pCoreOptionsCopy,pCoreOptions,(MemSize_t)sizeof(SmlOptions_t));


  /* --- set new SyncML options --- */
  pGlobalAnchor->syncmlOptions  = pCoreOptionsCopy;  // set the options,passed from the application

  return SML_ERR_OK;

}

/*************************************************************************
 *  SyncML internal functions
 *************************************************************************/
/**
 * Retrieves a pointer to the structure holding all global informations
 * within SyncML
 *
 * @return Pointer to the pGlobalAnchor
 */
SyncMLInfoPtr_t mgrGetSyncMLAnchor(void)
{
  return pGlobalAnchor;
}

/**
 * Retrieves a pointer to the list holding all instance informations
 *
 * @return Pointer to the pInstanceListAnchor
 */
InstanceInfoPtr_t mgrGetInstanceListAnchor(void)
{
  return pGlobalAnchor->instanceListAnchor;
}

/**
 * Set the pointer to the list holding all instance informations
 *
 * @param newListAnchor (IN)
 *        pointer to the pInstanceListAnchor
 */
void mgrSetInstanceListAnchor(InstanceInfoPtr_t newListAnchor)
{
  pGlobalAnchor->instanceListAnchor=newListAnchor;
}


#endif // !defined(NOWSM)
