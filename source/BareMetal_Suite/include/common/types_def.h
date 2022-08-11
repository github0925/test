/********************************************************
 *  Copyright(c) 2018   Semidrive       *
 *******************************************************/

/**
 * @file    types_def.h
 * @brief   header file for common types
 */

#ifndef __TYPES_DEF_H__
#define __TYPES_DEF_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef unsigned long long U64;
typedef unsigned int    U32;
typedef unsigned short  U16;
typedef unsigned char   U8;
typedef long long S64;
typedef int    S32;
typedef short  S16;
typedef char   S8;

typedef U8    BOOL;

#define ARRAY_SZ(x) (sizeof(x)/sizeof(x[0]))

#ifndef TRUE
#define TRUE    1UL
#endif
#ifndef FALSE
#define FALSE   0UL
#endif
#ifndef NULL
#define NULL    (void*) (uintptr_t)0UL
#endif

#define U32_MAX     0xFFFFFFFFU

typedef void (*fv_v) (void);

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

typedef unsigned long long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

typedef int status_t;

typedef uintptr_t addr_t;
typedef uintptr_t vaddr_t;
typedef uintptr_t paddr_t;

typedef enum {
    ATB_SUCCESS = 0x55,
    ATB_FAIL = 0xAA,
    ATB_YES = 0x55,
    ATB_NO = 0xAA,

} atb_status_e;

typedef uint32_t mutex_t;

#endif // __TYPES_DEF_H__
