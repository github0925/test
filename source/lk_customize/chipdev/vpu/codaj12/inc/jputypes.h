//------------------------------------------------------------------------------
// File: jputypes.h
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef _JPU_TYPES_H_
#define _JPU_TYPES_H_

#include <stdint.h>

typedef uint8_t             Uint8;
typedef uint16_t            Uint16;
typedef uint32_t            Uint32;
typedef uint64_t            Uint64;
typedef int8_t              Int8;
typedef int16_t             Int16;
typedef int32_t             Int32;
typedef int64_t             Int64;
typedef uint32_t            PhysicalAddress;
typedef unsigned char       BYTE;
typedef int32_t             BOOL;

#ifndef NULL
#define NULL    0
#endif

#ifndef TRUE
#define TRUE                        1
#endif /* TRUE */

#define STATIC              static

#ifndef FALSE
#define FALSE                       0
#endif /* FALSE */

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P)          \
    /*lint -save -e527 -e530 */ \
{ \
    (P) = (P); \
} \
    /*lint -restore */
#endif

#endif    /* _JPU_TYPES_H_ */
