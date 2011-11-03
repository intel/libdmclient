/**
 * @file
 * List of SyncML Instances
 *
 * @target_system   all
 * @target_os       all
 * @description This module handles an element list of type InstanceInfo. Each
 * element is identified by the InstanceID. There are functions provided
 * to add, find and remove InstanceInfo elements.
 * This file is private to the core module. The InstanceInfo list is
 * used by the Modules MGR, MGRCmdDispatcher, MGRCmdBuilder
 * and MGRInstanceMgr.
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
#include <smlerr.h>
#include "libmem.h"
#include "liblock.h"
#include "mgr.h"

#ifndef __SML_LITE__  /* Only ONE instance is supported in the Toolkit lite version */

#ifndef NOWSM /* only need if we are using workspace manager */


/* Used external functions */
SyncMLInfoPtr_t mgrGetSyncMLAnchor(void);
InstanceInfoPtr_t mgrGetInstanceListAnchor(void);
void mgrSetInstanceListAnchor(InstanceInfoPtr_t newListAnchor);


/* SyncML internal function prototypes */
Ret_t addInfo(InstanceInfoPtr_t pInfo);
InstanceInfoPtr_t findInfo(InstanceID_t id);
Ret_t removeInfo(InstanceID_t id);


/* Private function prototypes */






/*************************************************************************
 *  SyncML internal functions
 *************************************************************************/


/**
 * Adds a new element to the list
 *
 * @param pInfo (IN)
 *        pointer to the structure to be be added to list
 * @return Return value,\n
 *         SML_ERR_OK if element was added successfully
 */
Ret_t addInfo(InstanceInfoPtr_t pInfo)
{

  if (pInfo!=NULL)
    {
    InstanceInfoPtr_t _pTmp;

    LOCKTOOLKIT("addInfo");
    /* Remember old beginning of the list */
    _pTmp=mgrGetInstanceListAnchor();

    /* insert element immediately after anchor */
    mgrSetInstanceListAnchor(pInfo);      // anchor of list points now to new info element
    pInfo->nextInfo=_pTmp;                // Next info element is the prior first one.
    RELEASETOOLKIT("addInfo");
    return SML_ERR_OK;

    } else {                              // Invalid InstanceInfo pointer was used (NULL)

    return SML_ERR_MGR_INVALID_INSTANCE_INFO;
    }

}





/**
 * Searches an element with the given InstanceID in the list
 *
 * @param id (IN)
 *        ID of the InstanceInfo structure to be retrieved
 * @return Pointer to the InstanceInfo structure with the given ID\n
 *         NULL, if no InstanceInfo with the given ID has been found
 */
InstanceInfoPtr_t findInfo(InstanceID_t id)
{

  InstanceInfoPtr_t _pTmp;                // A helper pointer

  /* go through the list until end */
  LOCKTOOLKIT("findInfo");
  for (_pTmp=mgrGetInstanceListAnchor(); _pTmp!=NULL; _pTmp=_pTmp->nextInfo)
  {
    if (_pTmp->id == id) {
      RELEASETOOLKIT("findInfo");
      return _pTmp;                       // STOP, we've found the info, return!
    }
  }
  RELEASETOOLKIT("findInfo");
  return NULL;                            // Info was not found, return NULL

}





/**
 * Removes an element with the given InstanceID from the list
 *
 * @param id (IN)
 *        ID of the InstanceInfo structure to be removed
 * @return Return value,\n
 *         SML_ERR_OK if element was removed successfully
 */
Ret_t removeInfo(InstanceID_t id)
{

  InstanceInfoPtr_t _pTmp;               // A helper pointer
  InstanceInfoPtr_t _pRemember;          // A helper pointer


  LOCKTOOLKIT("removeInfo");
  /* Remember current anchor */
  _pRemember=mgrGetInstanceListAnchor();

  /* special check, if list is empty */
  if (_pRemember==NULL ) {
    RELEASETOOLKIT("removeInfo");
    return SML_ERR_MGR_INVALID_INSTANCE_INFO;
  }

  /* special check, if first element should be removed */
  if (_pRemember->id == id)
    {
    // It's the first element, update anchor!
    mgrSetInstanceListAnchor(_pRemember->nextInfo);
    //freeInfo(_pRemember); // Delete structure, free memory
    RELEASETOOLKIT("removeInfo");
    return SML_ERR_OK;                    // return
    }


  /* go through the list until end */
  for (_pTmp=_pRemember->nextInfo; _pTmp!=NULL; _pTmp=_pTmp->nextInfo)
    {
    if (_pTmp->id == id)                  // STOP, we've found the info
      {
      _pRemember->nextInfo=_pTmp->nextInfo;
      //freeInfo(_pTmp);  // Delete structure, free memory
      RELEASETOOLKIT("removeInfo");
      return SML_ERR_OK;                  // return

      } else {

      _pRemember=_pTmp;                   // update helper pointer
      }
    }

  RELEASETOOLKIT("removeInfo");
  return SML_ERR_MGR_INVALID_INSTANCE_INFO;  // Info wasn't found

}




#endif // !defined(NOWSM)


#endif
