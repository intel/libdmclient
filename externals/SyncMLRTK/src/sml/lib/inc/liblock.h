/**
 * @file
 * Library for Thread Locking Functions
 *
 * @target_system   ALL
 * @target_os       ALL
 * @description thread-locking library, RTK addition by luz@synthesis.ch
 */

#ifndef LIBLOCK_H
#define LIBLOCK_H

#include "define.h"

#ifdef __DEBUG_LOCKS
// needs debug global and DebugPrintf()
extern void ThreadDebugPrintf(const char *text,...);
#define DEBUGPRINTF(m) { ThreadDebugPrintf m; }
#endif

#if defined(__MAKE_THREADSAFE) && !defined(NOWSM)
  // thread safety measures are required only when working with WSM
  #ifdef _WIN32
    #ifdef __DEBUG_LOCKS
      // we need TryEnterCriticalSection for debug
      #define _WIN32_WINNT 0x0400
    #endif
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #undef WIN32_LEAN_AND_MEAN
    // remove problematic windows defines (OPAQUE in wingdi.h, OPTIONAL in ????)
    #undef OPAQUE
    #undef OPTIONAL

    #include <stdio.h>

    #ifdef __DEBUG_LOCKS
      /* - functions that also document lock usage */
      void _ToolKitLockInit(const char *msg);
      void _ToolKitLockFree(const char *msg);
      void _LockToolKit(const char *msg);
      void _ReleaseToolKit(const char *msg);
      #define TOOLKITLOCK_INIT(m) _ToolKitLockInit(m);
      #define TOOLKITLOCK_FREE(m) _ToolKitLockFree(m);
      #define LOCKTOOLKIT(m) _LockToolKit(m);
      #define RELEASETOOLKIT(m) _ReleaseToolKit(m);
    #else
      /* - simple macros to use the lock */
      #define TOOLKITLOCK_INIT(m) InitializeCriticalSection(&gSmlLock);
      #define TOOLKITLOCK_FREE(m) DeleteCriticalSection(&gSmlLock);
      #define LOCKTOOLKIT(m) EnterCriticalSection(&gSmlLock);
      #define RELEASETOOLKIT(m) LeaveCriticalSection(&gSmlLock);
    #endif
  #else
    #error "liblock.h: unsupported platform"
  #endif
#else
  /* just NOP */
  #define TOOLKITLOCK_INIT(m)
  #define TOOLKITLOCK_FREE(m)
  #define LOCKTOOLKIT(m)
  #define RELEASETOOLKIT(m)
#endif

#endif // LIBLOCK_H

// globals declarations may not be omitted on second include!
#ifdef __MAKE_THREADSAFE
  #ifdef _WIN32
    /* define the required things */
    #ifndef _IMPLEMENTS_LOCK_GLOBALS
    /* - the lock itself, as global variable */
    extern CRITICAL_SECTION gSmlLock;
    #else
    CRITICAL_SECTION gSmlLock;
    #endif
  #endif
#endif


/* eof */
