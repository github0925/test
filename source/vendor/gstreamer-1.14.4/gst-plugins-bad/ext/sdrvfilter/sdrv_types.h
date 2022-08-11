#ifndef __SDRV_TYPES_H__
#define __SDRV_TYPES_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <asm/types.h>
#include <asm/posix_types.h>

typedef uint64_t u64;
typedef int64_t s64;

typedef __u32 u32;
typedef __s32 s32;

typedef __u16 u16;
typedef __s16 s16;

typedef __u8  u8;
typedef __s8  s8;

typedef u64 dma_addr_t;
#endif
