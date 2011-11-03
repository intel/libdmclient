// prefix file, used to include global defines with compilers
// that don't allow global switches on the command line or
// the makefile.
// Also useful in some environments with precompiled headers like eVC++

// Global RTK options added by luz@synthesis.ch:

// - if NOWSM is defined, the entire complicated and thread-unsafe workspace manager
//   is completely bypassed.
//   The API remains the same, however the InstanceID_t type is no longer
//   a wsm handle, but a direct pointer to the InstanceInfo_t record.
//   The InstanceInfo_t record has a different layout, as it now includes
//   the sml buffer management.
//   As there is no global buffer management any more with NOWSM defined,
//   sml instances as returned by smlInitInstance() are now completely
//   independent and may be used by different threads.
//   NOTE 1: defining NOWSM makes the global functions smlInit, smlSetSyncMLOptions
//   and smlTerminate unavailable in statically linked case. For dynamic linking,
//   these functions are still available but are implemented as dummies.
//   NOTE 2: defining NOWSM requires external implementation of the
//   following functions:
//   - smlLibVprintf and smlLibPrint
//   - alternatively, one can #define TRACE_TO_STDOUT and implement localOutput
//     instead.
#define NOWSM 1
//#define TRACE_TO_STDOUT 1

// - causes that the opaque data type gets
//   correctly tagged in XML payload with "<![CDATA["
//#define PCDATA_OPAQUE_AS_CDATA 1

/* eof */
