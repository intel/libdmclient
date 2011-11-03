/**
 * @file
 * Library for Memory Functions
 *
 * @target_system   ALL
 * @target_os       ALL
 * @description Header for the implementation of common memory handling
 * functions
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
#include "libmem.h"
#ifdef __PALM_OS__
#include "MemoryMgr.h"
#endif

#ifdef MEMORY_PROFILING
  // %%% luz 2002-10-02
  #include "profiling.h"
#endif

/*************************************************************************
 *  External Functions  for all TOOLKIT Versions
 *************************************************************************/


/**
 * Deallocates the memory of object "pObject", which has been allocated
 * previously.
 * If "pObject" is a NULL pointer nothing happens.
 * If "pObject" is a pointer to memory which has not been allocated
 * previouly, the behaviour is undefined.
 * The contents of the deallocated memory object is destroyed.
 */
SML_API void smlLibFree(void *pObject)
{
  if (! pObject) return;
  #ifdef __PALM_OS__
    MemPtrFree(pObject);
  #else
    // %%% luz 2002-10-02
    #ifdef MEMORY_PROFILING
    sysync_free(pObject);
    #else
    free(pObject);
    #endif
  #endif
}


/**
 * Changes size of preallocated space for memory object "pObject"
 * to the new size specified by "constSize".
 *
 * If the new size is larger than the old size, the old contents
 * is not changed. Additionally space is added at the the end of
 * "pObject". The new allocated space is not initialized
 * to any special value.
 * If the new size is smaller than the old size, the unused space
 * is discarded.
 *
 * If "pObject" is a NULL pointer, this function behaves just like
 * smlLibMalloc().
 * If "pObject" does not point to a previously allocated memory area,
 * the behavior is undefined.
 * If "constSize" is 0, a NULL pointer is returned and the space
 * which "pObject" points to is freed up.
 *
 * Returns a pointer to the first byte of the resized object.
 * If no new memory could be allocated, a NULL Pointer is returned
 * without changing the memory object "pObject" (Nothing happens to the content).
 *
 * @param pObject (IN/OUT)
 *        memory object, which size should be changed
 * @param constSize (IN)
 *        new size the memory object shall use
 * @return void pointer to memory object, which size has been be changed\n
 *         NULL, if not successfull or if constSize==0
 */
SML_API void *smlLibRealloc(void *pObject, MemSize_t constSize)
{
#ifdef __PALM_OS__
  VoidPtr_t _new_object;
  MemSize_t   _old_size;

    // It's a malloc!
    if (pObject == NULL)
        return smlLibMalloc(constSize);

    _old_size = MemPtrSize(pObject);
    if (constSize <= _old_size) {
        // make it smaller
        MemPtrResize(pObject, constSize);
        _new_object = pObject;
    } else {
        // maker it larger (we need to allocate new memory somewhere else)
        _new_object = smlLibMalloc(constSize);
        if (_new_object != NULL) {
            smlLibMemmove(_new_object, pObject, _old_size);
            smlLibFree(pObject);
        }
    }

  return _new_object;
#else
  // %%% luz 2002-10-02
  #ifdef MEMORY_PROFILING
  return sysync_realloc(pObject, constSize);
  #else
  return realloc(pObject, constSize);
  #endif
#endif
}


#ifndef __PALM_OS__
/* If not Palm OS we use the Standard ANSI C functions */
SML_API void *smlLibMemset(void *pObject, int value, MemSize_t count){
  return memset(pObject, value, count);
}
SML_API void *smlLibMemcpy(void *pTarget, const void *pSource, MemSize_t count){
  return memcpy(pTarget, pSource, count);
}
SML_API void *smlLibMemmove(void *pTarget, const void *pSource, MemSize_t count){
  return memmove(pTarget, pSource, count);
}
SML_API int smlLibMemcmp(const void *pTarget, const void *pSource, MemSize_t count){
  return memcmp(pTarget, pSource, count);
}
#ifndef smlLibMalloc
// define only if not already defined as a macro
SML_API void *smlLibMalloc(MemSize_t size) {
  // %%% luz 2002-10-02
  #ifdef MEMORY_PROFILING
  return (void *)sysync_malloc(size);
  #else
  return (void *)malloc(size);
  #endif
}
#endif

#endif



