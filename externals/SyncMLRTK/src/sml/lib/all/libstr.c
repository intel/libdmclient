/**
 * @file
 * Library for String Functions
 *
 * @target_system   ALL
 * @target_os       ALL
 * @description implementation of common string-handling functions
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

#include <smldef.h>
#include "libstr.h"
#include "libmem.h"
#ifdef __ANSI_C__
#include <string.h>
#endif
#ifdef __PALM_OS__
#include <StringMgr.h>
#endif



/*************************************************************************
 *  External Functions  for all TOOLKIT Versions
 *************************************************************************/


/**
 * Duplicates the String "constStringP".
 * Returns a pointer to the new copy of "constStringP".
 *
 * @param constStringP (IN)
 *        string which is duplicated
 * @return pointer to the new copy,\n
 *         NULL if no copy could be allocated
 */
SML_API String_t smlLibStrdup (const char *constStringP)
{
  String_t _new_str;

  // allocate memory for new copy
  _new_str = (String_t)smlLibMalloc(smlLibStrlen(constStringP) + 1);

  // Copy the string into the new memory
  if (_new_str != NULL)
    smlLibStrcpy(_new_str, constStringP);

  return _new_str;
}


#ifndef __PALM_OS__
/* If not Palm OS we use the Standard ANSI C functions */
SML_API String_t smlLibStrcpy(const char *pTarget, const char *pSource) {
  return strcpy((char *)pTarget, (char *)pSource);
}
SML_API String_t smlLibStrncpy(const char *pTarget, const char *pSource, int count){
  return strncpy((char *)pTarget, (char *)pSource, count);
}
SML_API String_t smlLibStrcat(const char *pTarget, const char *pSource){
  return strcat((char *)pTarget, (char *)pSource);
}
SML_API int smlLibStrcmp(const char *pTarget, const char *pSource){
  return strcmp((char *)pTarget, (char *)pSource);
}
SML_API int smlLibStrncmp(const char *pTarget, const char *pSource, int count){
  return strncmp((char *)pTarget, (char *)pSource, count);
}
SML_API int smlLibStrnCasecmp(const char *pTarget, const char *pSource, int count){
  return strncasecmp((char *)pTarget, (char *)pSource, count);
}
SML_API String_t smlLibStrchr(const char *pString, char character){
  return strchr((char *)pString, character);
}
SML_API int smlLibStrlen(const char *pString){
  return strlen((char *)pString);
}


#endif



/*************************************************************************
 *  Additional External Functions  for Full Size TOOLKIT ONLY
 *************************************************************************/

#ifndef __SML_LITE__  /* these API calls are NOT included in the Toolkit lite version */
#ifndef __PALM_OS__  /* we use #define to reduce heap usage */
SML_API String_t smlLibStrncat(const char *pTarget, const char *pSource, int count){
  return strncat((char *)pTarget, (char *)pSource, count);
}
SML_API String_t smlLibStrstr(const char *pString, const char *pSubString){
  return strstr((char *)pString, (char *)pSubString);
}
#endif
#endif
