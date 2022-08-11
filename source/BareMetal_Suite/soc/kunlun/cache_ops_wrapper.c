/********************************************************
 *  Copyright(c) 2019   Semidrive       *
 *  All Right Reserved.
 *******************************************************/

#include <types_def.h>
#include <arch.h>
#include <soc.h>

#if defined(ARCH_armv7_r)
/** Note: we don't handle the case that part of the range in tcm, part not.
 *  Callers in ROM code will make sure the scenario above will never happen.
 */
#define IS_CACHE_MAINTAINANCE_NEEDED(s, l) \
    (((uintptr_t)(-1UL) - (uintptr_t)(s) > (uintptr_t)(l)) && \
    (((uintptr_t)(s) > (uintptr_t)TCM_END) || \
     ((uintptr_t)(s) + (uintptr_t)(l) < TCM_BASE)) \
     && !FUSE_DCACHE_DISABLE())

void clean_cache_range(const void *start, U32 len)
{
    if (IS_CACHE_MAINTAINANCE_NEEDED(start, len)) {
        arch_clean_cache_range(start, len);
    }
}

void clean_invalidate_cache_range(const void *start, U32 len)
{
    if (IS_CACHE_MAINTAINANCE_NEEDED(start, len)) {
        arch_clean_invalidate_cache_range(start, len);
    }
}

void invalidate_cache_range(const void *start, U32 len)
{
    if (IS_CACHE_MAINTAINANCE_NEEDED(start, len)) {
        arch_invalidate_cache_range(start, len);
    }
}

#endif
