/**
 * @file
 * Library for Thread Locking Functions
 *
 * @target_system   ALL
 * @target_os       ALL
 * @description thread-locking library, RTK addition by luz@synthesis.ch
 */

#include "syncml_tk_prefix_file.h" // %%% luz: needed for precompiled headers in eVC++

#define _IMPLEMENTS_LOCK_GLOBALS
#include "liblock.h"
#undef _IMPLEMENTS_LOCK_GLOBALS


#ifdef __DEBUG_LOCKS

static short gLockNest;


void _ToolKitLockInit(const char *msg)
{
  gLockNest=0;
  InitializeCriticalSection(&gSmlLock);
  DEBUGPRINTF(("Toolkitlock initialized from '%s'",msg))
} // _ToolKitLockInit


void _ToolKitLockFree(const char *msg) {
  DeleteCriticalSection(&gSmlLock);
  DEBUGPRINTF(("Toolkitlock freed from '%s'",msg));
} // _ToolKitLockFree


void _LockToolKit(const char *msg)
{
  short sec=0;
  int k;

  if (!TryEnterCriticalSection(&gSmlLock)) {
    // could not enter
    for (k=0; k<10; k++) {
      Sleep(100); // wait 0.1 sec
      if (TryEnterCriticalSection(&gSmlLock)) goto entered;
    }
    // could not enter after 1 sec
    DEBUGPRINTF(("Toolkitlock not free after 1 sec trying..."))
    do {
      Sleep(30000); // wait 30 secs
      sec+=30;
      if (TryEnterCriticalSection(&gSmlLock)) goto entered;
      DEBUGPRINTF(("Toolkitlock not free after %d sec trying...",sec))
    } while (1);
  }
entered:
  #if __DEBUG_LOCKS>1
  DEBUGPRINTF(("Toolkitlock entered from '%s'",msg))
  #endif
  ;
} // _LockToolKit


void _ReleaseToolKit(const char *msg)
{
  LeaveCriticalSection(&gSmlLock);
  #if __DEBUG_LOCKS>1
  DEBUGPRINTF(("Toolkitlock released from '%s'",msg))
  #endif
} // _ReleaseToolKit


#endif

/* eof */
