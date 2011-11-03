/**
 * @file
 * XLT Decoder Stack
 *
 * @target_system  all
 * @target_os      all
 * @description A simple array-based stack implementation.
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

/*************************************************************************/
/* Definitions                                                           */
/*************************************************************************/

#include "syncml_tk_prefix_file.h" // %%% luz: needed for precompiled headers in eVC++

#include "xltdeccom.h"
#include "xltutilstack.h"

#include <smlerr.h>

#include <libmem.h>

struct ArrayStack_s;
typedef struct ArrayStack_s *ArrayStackPtr_t, ArrayStack_t;
struct ArrayStack_s
{
    /* public */
    Ret_t (*top)(const XltUtilStackPtr_t, XltUtilStackItem_t *);
    Ret_t (*pop)(XltUtilStackPtr_t, XltUtilStackItem_t *);
    Ret_t (*push)(XltUtilStackPtr_t, const XltUtilStackItem_t);
    Ret_t (*destroy)(XltUtilStackPtr_t);

    /* private */
    Long_t topidx;          // index of the top of the stack
    Long_t size;            // size of the stack (multiple of chunksize)
    Long_t chunksize;       // size of memory chunks allocated at a time
    XltUtilStackItem_t *array;     // the stack itself
};

static Ret_t _top(const XltUtilStackPtr_t, XltUtilStackItem_t *);
static Ret_t _pop(XltUtilStackPtr_t, XltUtilStackItem_t *);
static Ret_t _push(XltUtilStackPtr_t, const XltUtilStackItem_t);
static Ret_t _destroy(XltUtilStackPtr_t);

/*************************************************************************/
/* External Functions                                                    */
/*************************************************************************/

Ret_t
xltUtilCreateStack(XltUtilStackPtr_t *ppStack, const Long_t size)
{
    ArrayStackPtr_t pStack;

    if (size <= 0)
        return SML_ERR_WRONG_PARAM;
        if ((pStack = (ArrayStackPtr_t)smlLibMalloc(sizeof(ArrayStack_t))) == NULL) {
    *ppStack = NULL;
           return SML_ERR_NOT_ENOUGH_SPACE;
  }

    pStack->top = _top;
    pStack->pop = _pop;
    pStack->push = _push;
    pStack->destroy = _destroy;
    pStack->topidx = -1;
    pStack->size = size;
    pStack->chunksize = size;
    pStack->array = NULL;
    if ((pStack->array = (XltUtilStackItem_t*)smlLibMalloc(size * sizeof(XltUtilStackItem_t))) == NULL) {
    *ppStack = NULL;
    smlLibFree(pStack);
        return SML_ERR_NOT_ENOUGH_SPACE;
  }

    *ppStack = (XltUtilStackPtr_t)pStack;



    return SML_ERR_OK;
}

/*************************************************************************/
/* Internal Functions                                                    */
/*************************************************************************/

static Ret_t
_top(const XltUtilStackPtr_t pStack, XltUtilStackItem_t *itemPtr)
{
    ArrayStackPtr_t pStackPriv = (ArrayStackPtr_t)pStack;

    if (pStackPriv->topidx == -1)
        return SML_ERR_WRONG_USAGE;

    *itemPtr = pStackPriv->array[pStackPriv->topidx];

    return SML_ERR_OK;
}

static Ret_t
_pop(XltUtilStackPtr_t pStack, XltUtilStackItem_t *itemPtr)
{
    ArrayStackPtr_t pStackPriv = (ArrayStackPtr_t)pStack;
    XltUtilStackItem_t item;

    if (pStackPriv->topidx == -1)
        return SML_ERR_WRONG_USAGE;

    item = pStackPriv->array[pStackPriv->topidx];
    pStackPriv->topidx--;

    if ((pStackPriv->topidx >= 0) &&
            (pStackPriv->topidx < pStackPriv->size - pStackPriv->chunksize)) {
        Long_t newsize;
        XltUtilStackItem_t *newarray;

        newsize = pStackPriv->size - pStackPriv->chunksize;
        if ((newarray = (XltUtilStackItem_t*)smlLibRealloc(pStackPriv->array,
                        newsize * sizeof(XltUtilStackItem_t))) != NULL) {
            pStackPriv->size = newsize;
            pStackPriv->array = newarray;
        } else {
            return SML_ERR_NOT_ENOUGH_SPACE;
        }
    }

    *itemPtr = item;

    return SML_ERR_OK;
}

static Ret_t
_push(XltUtilStackPtr_t pStack, const XltUtilStackItem_t item)
{
    ArrayStackPtr_t pStackPriv = (ArrayStackPtr_t)pStack;

    if (pStackPriv->topidx == pStackPriv->size - 1) {
        Long_t newsize;
        XltUtilStackItem_t *newarray;

        newsize = pStackPriv->size + pStackPriv->chunksize;
        if ((newarray = (XltUtilStackItem_t*)smlLibRealloc(pStackPriv->array,
                        newsize * sizeof(XltUtilStackItem_t))) != NULL) {
            pStackPriv->size = newsize;
            pStackPriv->array = newarray;
        } else {
            return SML_ERR_NOT_ENOUGH_SPACE;
        }
    }

    pStackPriv->topidx++;
    pStackPriv->array[pStackPriv->topidx] = item;

    return SML_ERR_OK;
}

static Ret_t
_destroy(XltUtilStackPtr_t pStack)
{
    ArrayStackPtr_t pStackPriv;

    if (pStack == NULL)
        return SML_ERR_OK;

    pStackPriv = (ArrayStackPtr_t)pStack;

    smlLibFree(pStackPriv->array);
    smlLibFree(pStackPriv);
    return SML_ERR_OK;
}
