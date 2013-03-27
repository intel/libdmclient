/**
 * @file
 * Compiler Flag Definition File
 *
 * @target_system   win
 * @target_os       win
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

/**
 * File for Windows Specific Compiler Flags
 */

#ifndef _DEFINE_H
  #define _DEFINE_H
#define __ANSI_C__


/* thread safety (added by luz@synthesis.ch, 2001-10-29) */
/* Note: moved define of this to target_options.h of every target */
//#undef __MAKE_THREADSAFE

/* enable Alloc helpers */
#define __USE_ALLOCFUNCS__

/* do we need WBXML (binary XML) processing ? */
#define __SML_WBXML__
/* do we need the capability to decode plain text tokens in WBXML? */
#define __SML_WBXML_TEXTTOKENS__
/* do we need XML processing ? */
#define __SML_XML__
/* are we using a 'light' toolkit ? */
//#define __SML_LITE__
/* do we use Sub DTD extensions ? */
#define __USE_EXTENSIONS__
/* do we need Metainformation DTD parsing ? */
#define __USE_METINF__
/* do we use Device Info DTD ? */
//#define __USE_DEVINF__

/* which of the following optional commands should be included ? */

//#define ADD_SEND
//#define ATOMIC_SEND
#define ATOMIC_RECEIVE
//#define COPY_SEND
#define COPY_RECEIVE
//#define EXEC_SEND
#define EXEC_RECEIVE
//#define GET_SEND
//#define MAP_RECEIVE
//#define MAPITEM_RECEIVE
//#define RESULT_RECEIVE
//#define SEARCH_SEND
//#define SEARCH_RECEIVE
//#define SEQUENCE_SEND
#define SEQUENCE_RECEIVE


/* TK: to improve interoperability and handling we
 * switched to using .def files instead of compiler
 * specific per function definitions. As long as we only
 * use C this is the easiest and cleanes way
 */

#define SML_API
#define SML_API_DEF
#define XPT_API
#define XPT_API_DEF

/* Multi segment macro for Palm OS */
#define LIB_FUNC
#define MGR_FUNC
#define WSM_FUNC
#define XLT_FUNC

/* TK: Old, now obsolete code follows here */
#ifdef USE_OLD_DEFINES
/*
When building the DLL code with GNU, you should define BUILDING_DLL so that
the variables/functions are exported correctly. When using the DLL,
do NOT define BUILDING_DLL, and then the variables/functions will be
imported correctly.

You need to be using egcs-1.1.1 or newer.

Building the DLL:
 - define BUILDING_DLL, which defines SML_API __attribute__((dllexport))
Building the client code:
 - DO NOT define BUILDING_DLL, which defines SML_API to be one __attribute__((dllimport))
*/


#if __GNUC__ || __MSVC__ || __MSVCPP__ || _WIN32_WCE
  /* define this, if you want to link the toolkit static */
  #if __LINK_TOOLKIT_STATIC__ || __linux__
    #define SML_API
    #define SML_API_DEF
    #define XPT_API
    #define XPT_API_DEF
  #else
    #if BUILDING_DLL
      #define SML_API   __declspec (dllexport)
      #define SML_API_DEF   __declspec (dllexport)
      #define XPT_API   __declspec (dllexport)
      #define XPT_API_DEF   __declspec (dllexport)
    #else /* Not BUILDING_DLL */
      #define SML_API   __declspec (dllimport)
      #define SML_API_DEF   __declspec (dllimport)
      #define XPT_API   __declspec (dllimport)
      #define XPT_API_DEF   __declspec (dllimport)
    #endif /* Not BUILDING_DLL */
  #endif
#else
  #if __IBMC__ || __IBMCPP__
    #define SML_API_DEF   __stdcall
    #define SML_API   __stdcall _Export
    #define XPT_API_DEF   __stdcall
    #define XPT_API   __stdcall _Export
  #else
        #if WIN32
        #define SML_API     __declspec (dllexport)
        #define SML_API_DEF __declspec (dllexport)
        #define XPT_API     #error
        #define XPT_API_DEF #error
        #else
        #define SML_API_DEF __stdcall
    #define SML_API     __stdcall
        #define XPT_API_DEF __stdcall
    #define XPT_API     __stdcall
  #endif
#endif
#endif
#endif // USE_OLD_DEFINES

#endif
