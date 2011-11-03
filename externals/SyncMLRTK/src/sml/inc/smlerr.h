/**
 * @file
 * ErrorLibrary
 *
 * @target_system   all
 * @target_os       all
 * @description Definition of the used Error Codes
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


#ifndef _SML_ERR_H
  #define _SML_ERR_H



/*************************************************************************
 *  Definitions
 *************************************************************************/



/*
 * No error, success code
 */
#define SML_ERR_OK               0x00      /**< OK  */

#define SML_ERR_CONTINUE         0x01      /**< OK, but processing of message not finished yet (smlProcessData(NEXT_COMMAND) case) */


/*
 * SyncML Common Error Codes
 */

// general errors
#define SML_ERR_UNSPECIFIC       0x10      /**< unspecific error */
#define SML_ERR_NOT_ENOUGH_SPACE 0x11      /**< not enough memory to perform this operation */
#define SML_ERR_WRONG_USAGE      0x13      /**< function was called in wrong context */

// wrong parameters
#define SML_ERR_WRONG_PARAM      0x20      /**< wrong parameter */
#define SML_ERR_INVALID_SIZE     0x21      /**< param has an invalid size */
#define SML_ERR_INVALID_HANDLE   0x22      /**< if handle is invalid/unknown */
#define SML_ERR_INVALID_OPTIONS  0x23      /**< unkown or unallowed options */


/*
 * SyncML Mgr Error Codes
 */
#define SML_ERR_A_MGR_ERROR                 0x1001   /**< a template */
#define SML_ERR_MGR_INVALID_INSTANCE_INFO   0x1002   /**< a invalid Instance Info structure is used */
#define SML_ERR_COMMAND_NOT_HANDLED         0x1003   /**< no callback function is available to handle this command */
#define SML_ERR_ALREADY_INITIALIZED         0x1004   /**< Mgr allready initialized */


/*
 * SyncML Xlt Error Codes
 */
#define SML_ERR_XLT_MISSING_CONT            0x2001   /**< required field content missing */
#define SML_ERR_XLT_BUF_ERR                 0x2002   /**< Buffer too small  */
#define SML_ERR_XLT_INVAL_PCDATA_TYPE       0x2003   /**< Invalid (WBXML) Element Type (STR_I etc.) */
#define SML_ERR_XLT_INVAL_LIST_TYPE         0x2004   /**< Invalid List Type (COL_LIST etc.) */
#define SML_ERR_XLT_INVAL_TAG_TYPE          0x2005   /**< Invalid Tag Type (TT_BEG etc.) */
#define SML_ERR_XLT_ENC_UNK                 0x2007   /**< Unknown Encoding (WBXML, XML) */
#define SML_ERR_XLT_INVAL_PROTO_ELEM        0x2008   /**< Invalid Protocol Element (ADD, Delete, ...) */
#define SML_ERR_MISSING_LIST_ELEM           0x2009   /**< Missing Content of List Elements  */
#define SML_ERR_XLT_INCOMP_WBXML_VERS       0x200A   /**< Incompatible WBXML Content Format Version */
#define SML_ERR_XLT_INVAL_SYNCML_DOC        0x200B   /**< Document does not conform to SyncML DTD  */
#define SML_ERR_XLT_INVAL_PCDATA            0x200C   /**< Invalid PCData elem (e.g. not encoded as OPAQUE data) */
#define SML_ERR_XLT_TOKENIZER_ERROR         0x200D   /**< Unspecified tokenizer error */
#define SML_ERR_XLT_INVAL_WBXML_DOC         0x200E   /**< Document does not conform to WBXML specification */
#define SML_ERR_XLT_WBXML_UKN_TOK           0x200F   /**< Document contains unknown WBXML token */
#define SML_ERR_XLT_MISSING_END_TAG         0x2010   /**< Non-empty start tag without matching end tag */
#define SML_ERR_XLT_INVALID_CODEPAGE        0x2011   /**< WBXML document uses unspecified code page */
#define SML_ERR_XLT_END_OF_BUFFER           0x2012   /**< End of buffer reached */
#define SML_ERR_XLT_INVAL_XML_DOC           0x2013   /**< Document does not conform to XML 1.0 specification */
#define SML_ERR_XLT_XML_UKN_TAG             0x2014   /**< Document contains unknown XML tag */
#define SML_ERR_XLT_INVAL_PUB_IDENT         0x2015   /**< Invalid Public Identifier */
#define SML_ERR_XLT_INVAL_EXT               0x2016   /**< Invalid Codepage Extension */
#define SML_ERR_XLT_NO_MATCHING_CODEPAGE    0x2017   /**< No matching Codepage could be found */
#define SML_ERR_XLT_INVAL_INPUT_DATA        0x2018   /**< Data missing in input structure */


/*
 * SyncML Wsm Error Codes
 */
#define SML_ERR_WSM_BUF_TABLE_FULL          0x3001   /**< no more empty entries in buffer table available */

/*
 * SyncML Util Error Codes
 */
#define SML_ERR_A_UTI_UNKNOWN_PROTO_ELEMENT 0x7001


#endif

