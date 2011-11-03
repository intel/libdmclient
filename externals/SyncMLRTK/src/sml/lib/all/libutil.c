/**
 * @file
 * Library for IO Functions
 *
 * @target_system   ALL
 * @target_os       ALL
 * @description Utility I/O  functions
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

#ifndef NOWSM
// Note: in NOWSM case, these routines must be implemented in the
//       calling main code.

#include <smldef.h>
#include <smlerr.h>

#ifdef __ANSI_C__
#include <stdarg.h>
#include <stdio.h>
#endif

#ifdef __PALM_OS__
#include <PalmOS.h>
#include <unix_stdarg.h>
#endif

#ifdef __EPOC_OS__
#include <stdarg.h>
#include <stdio.h>
#endif

#include "define.h"
#include "libmem.h"
#include "mgr.h"
#include "libutil.h"


// size of output buffer
#define BUFFERSIZE 128


/* Used external functions */
extern SyncMLInfoPtr_t mgrGetSyncMLAnchor();


/*************************************************************************
 *  External Functions
 *************************************************************************/


/**
 * Assembles a formatted textstring out of a list of argument. This string is
 * passed to a callback function, which is implementated by the application
 * for output to the user
 *
 * @param text (IN)
 *        printf like format text string with multiple
 *        arguments to be formatted as specified\n
 *        Supported are:
 *        \%d, \%i, \%u, \%x, \%s, \%c\n
 *        +, -, \<space\>, *, \<number\>, h, l, L
 */
SML_API void smlLibPrint(const char *text, ...)
{
    char    pBuffer[BUFFERSIZE];
    va_list   args;
    SyncMLInfoPtr_t pSyncMLInfo;

  if ((pSyncMLInfo = mgrGetSyncMLAnchor()) != NULL) {

    pBuffer[0] = '\0';

    va_start(args, text);

    // assemble the text string out of the single arguments
    #ifdef __ANSI_C__
    vsprintf(pBuffer, text, args);
    #endif

    #ifdef __PALM_OS__
    StrVPrintF(pBuffer, text, args);
    #endif

    #ifdef __EPOC_OS__
    vsprintf(pBuffer, text, args);
    #endif

    va_end(args);

    // use the application callback function,
    // which implements the output.
    if (pSyncMLInfo &&
            pSyncMLInfo->syncmlOptions &&
            pSyncMLInfo->syncmlOptions->defaultPrintFunc)
      ((smlPrintFunc)(pSyncMLInfo->syncmlOptions->defaultPrintFunc))(pBuffer);
  }
}

SML_API void  smlLibVprintf(const char *format, va_list args)
{
    char    pBuffer[BUFFERSIZE];
    SyncMLInfoPtr_t pSyncMLInfo;

  if ((pSyncMLInfo = mgrGetSyncMLAnchor()) != NULL) {

    pBuffer[0] = '\0';

    // assemble the text string out of the single arguments
    #ifdef __ANSI_C__
    vsprintf(pBuffer, format, args);
    #endif

    #ifdef __PALM_OS__
    StrVPrintF(pBuffer, format, args);
    #endif

    #ifdef __EPOC_OS__
    vsprintf(pBuffer, format, args);
    #endif

    // use the application callback function,
    // which implements the output.
    if (pSyncMLInfo &&
            pSyncMLInfo->syncmlOptions &&
            pSyncMLInfo->syncmlOptions->defaultPrintFunc)
      ((smlPrintFunc)(pSyncMLInfo->syncmlOptions->defaultPrintFunc))(pBuffer);
  }
}

#endif // !defined(NOWSM)
