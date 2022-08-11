/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    mem_libc.c
 * @brief   secure version of mem_xxx routines
 */

#include <common_hdr.h>
#include <arch.h>
#include <limits.h>

#if (ULONG_MAX == 4294967295UL)
#define CPU_REG_SZ  4
typedef uint32_t cpu_reg_t;
#elif (ULONG_MAX == 18446744073709551615ULL)
#define CPU_REG_SZ  8
typedef uint64_t cpu_reg_t;
#endif

void *mini_memcpy_s(void *dst, const void *src, size_t size)
{
    U32 align_src = (uintptr_t)src % CPU_REG_SZ;
    U32 align_dst = (uintptr_t)dst % CPU_REG_SZ;
    size_t sz_saved = size;
    U8 *dst_p = (U8 *)dst;
    const U8 *src_p = (const U8 *)src;

    assert((NULL != dst_p));    /* copy from 0 is allowed */
    assert(((uintptr_t)(-1ULL) - (uintptr_t)dst_p) > (uintptr_t)size);
    assert(((uintptr_t)(-1ULL) - (uintptr_t)src_p) > (uintptr_t)size);

    if (align_src != align_dst) {
        for (; size > 0; size--, dst_p++, src_p++) {
            *dst_p = *src_p;
        }
    } else {
        size_t bytes;

        if (0 != align_src) {
            bytes = size > (CPU_REG_SZ - align_dst) ? (CPU_REG_SZ - align_dst) : size;

            for (size_t i = 0; i < bytes ; i++, size--, dst_p++, src_p++) {
                *dst_p = *src_p;
            }
        }

        if (size > CPU_REG_SZ) {
            bytes = size / CPU_REG_SZ * CPU_REG_SZ;
#if defined(MEMCPY_ASM_OPTIMIZED)
            memcpy_aligned(dst_p, src_p, bytes);
            dst_p += bytes;
            src_p += bytes;
#else
            cpu_reg_t *l_src_p = (cpu_reg_t *)src_p;
            cpu_reg_t *l_dst_p = (cpu_reg_t *)dst_p;

            for (size_t i = 0; i < bytes / sizeof(cpu_reg_t);
                 i++, src_p += sizeof(cpu_reg_t), dst_p += sizeof(cpu_reg_t)) {
                *l_dst_p++ = *l_src_p++;
            }

#endif
            size -= bytes;
        }

        for (; size > 0; size--, dst_p++, src_p++) {
            *dst_p = *src_p;
        }
    }

    assert((0 == size) && (dst == (dst_p - sz_saved))
           && (src  == (src_p - sz_saved)));

    return dst;
}

void mini_memclr_s(void *dst, size_t size)
{
    U32 align = (uintptr_t)dst % CPU_REG_SZ;
    size_t bytes;
    U8 *dst_p = (U8 *)dst;
    size_t sz_saved = size;

    assert(NULL != dst_p);
    assert(((uintptr_t)(-1ULL) - (uintptr_t)dst_p) > (uintptr_t)size);

    if (align != 0) {
        bytes = size > (CPU_REG_SZ - align) ? (CPU_REG_SZ - align) : size;

        for (; bytes > 0; bytes--, size--, dst_p++) {
            *dst_p = 0;
        }
    }

    if (size > CPU_REG_SZ) {
        bytes = size / CPU_REG_SZ * CPU_REG_SZ;
#if defined(MEMCLR_ASM_OPTIMIZED)
        memclr_aligned(dst_p, bytes);
        size -= bytes;
        dst_p += bytes;
#else
        cpu_reg_t *l_dst_p = (cpu_reg_t *)dst_p;

        for (; bytes > 0;
             bytes -= sizeof(cpu_reg_t), size -= sizeof(cpu_reg_t),
             dst_p += sizeof(cpu_reg_t), l_dst_p++) {
            *l_dst_p = 0;
        }

#endif
    }

    for (; size > 0; size--, dst_p++) {
        *dst_p = 0;
    }

    assert((0 == size) && ((U8 *)dst + sz_saved == dst_p));
}

/*
 * memcmp is not heavily used in this project, so the char by char comparision is
 * acceptable here.
 */
S32 mini_memcmp_s(const void *mem1, const void *mem2, size_t size)
{
    U32 res = 0;
    const U8 *mem1_p = (const U8 *)mem1, *mem2_p = (const U8 *)mem2;
    size_t sz_saved = size;

    assert((NULL != mem1_p) && (NULL != mem2_p));
    assert(((uintptr_t)(-1ULL) - (uintptr_t)mem1_p) > (uintptr_t)size);
    assert(((uintptr_t)(-1ULL) - (uintptr_t)mem2_p) > (uintptr_t)size);

    for (; size > 0; size--, mem1_p++, mem2_p++) {
        res |= (*mem1_p != *mem2_p);
    }

    assert((0 == size) && (mem1 == mem1_p - sz_saved)
           && (mem2 == mem2_p - sz_saved));

    return res;
}

void *mini_memset_s(void *dst, int val, size_t size)
{
    U8 *dst_p = (U8 *)dst;
    size_t sz_saved = size;

    assert(NULL != dst_p);
    assert(((uintptr_t)(-1ULL) - (uintptr_t)dst_p) > (uintptr_t)size);

    for (; size > 0; size--, dst_p++) {
        *dst_p = (U8)val;
    }

    assert((0 == size) && (dst == dst_p - sz_saved));

    return dst;
}

void *mini_memmove_s(void *dst, const void *src, size_t size)
{
    U8 *dst_p = (U8 *)dst;
    const U8 *src_p = (const U8 *) src;
    int i = 0;

    assert((NULL != dst_p));    /* move from 0 is allowed */

    assert(((uintptr_t)(-1ULL) - (uintptr_t)dst_p) > (uintptr_t)size);
    assert(((uintptr_t)(-1ULL) - (uintptr_t)src_p) > (uintptr_t)size);

    U32 dis = src_p > dst_p ? (U32)(uintptr_t)(src_p - dst_p) :
              (U32)(uintptr_t)(dst_p - src_p);

    if ((0 == dis) || (0 == size)) {
        /* do nothing */
    } else if ( (dis >= size) || (src_p > dst_p)) {
        mini_memcpy_s(dst_p, src_p, size);
    } else {
        /* from end */
        src_p += size;
        dst_p += size;

        for (i = size; i > 0; i--) {
            dst_p--;
            src_p--;
            *dst_p = *src_p;
        }

        assert((dst_p == (U8 *)dst) && (src_p == (U8 *)src));
    }

    return dst;
}

#if defined(NO_STDLIB)
void *memcpy (void *dst, const void *src, size_t sz)
{
    mini_memcpy_s(dst, src, sz);

    return dst;
}

void *memset(void *dst, int val, size_t sz)
{
    return mini_memset_s(dst, (U8)val, sz);
}
#endif

void *mini_mem_rvscpy_s(void *dst, const void *src, size_t sz)
{
    const U8 *p_src = (const U8 *)src + sz - 1;
    U8 *p_dst = (U8 *)dst;
    size_t sz_saved = sz;

    assert((NULL != dst) && (sz > 0));

    while (sz) {
        *p_dst++ = *p_src--;
        sz--;
    }

    assert((0 == sz)
           && ((p_src + 1) == (U8 *)src)
           && (p_dst == ((U8 *)dst + sz_saved)));

    return dst;
}

/* memory reverse */
void *mini_mem_rvs_s(void *mem, size_t sz)
{
    size_t n = (sz + 1) / 2;
    U8 *begin = (U8 *)mem;
    U8 *end = (U8 *) (mem) + sz - 1;

    assert((NULL != mem) && (sz > 0));

    while (n) {
        U8 tmp = *begin;
        *begin = *end;
        *end = tmp;
        begin++;
        end--;
        n--;
    }

    assert(0 == n);

    return mem;
}
