/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    arch.h
 * @brief   header files for armv7-r arch.
 */

#ifndef __ARCH_H__
#define __ARCH_H__

#define MODE_USR    0x10
#define MODE_FIQ    0x11
#define MODE_IRQ    0x12
#define MODE_SVC    0x13
#define MODE_ABT    0x17
#define MODE_UND    0x1B
#define MODE_SYS    0x1F

#define DCACHE      0x01
#define ICACHE      0x02

#define CACHE_LINE      32
#define PAGE_SIZE 4096
#define PAGE_SIZE_SHIFT 12

#define BM_SCTLR_M  (0x01UL << 0)
#define BM_SCTLR_A  (0x01UL << 1)
#define BM_SCTLR_C  (0x01UL << 2)

#define BM_ACTLR_B1TCMPCEN  (0x01u << 27)
#define BM_ACTLR_B0TCMPCEN  (0x01u << 26)
#define BM_ACTLR_ATCMPCEN   (0x01u << 25)

#if !defined(ASSEMBLY)

#include <common_hdr.h>

#define isb()   __asm volatile("isb sy": : : "memory")
#define dsb()   __asm volatile("dsb sy": : : "memory")
#define dmb()   __asm volatile("dmb sy": : : "memory")
#define nop()   __asm volatile("nop")

void arch_enable_branch_predict(void);
void arch_invalidate_tlb(void);

U32 arch_rd_sctlr(void);
U32 arch_rd_actlr(void);
void arch_wr_actlr(U32);
void arch_wr_sctlr(U32);

U32 arch_rd_dfsr(void);
U32 arch_rd_ifsr(void);
U32 arch_rd_ifar(void);
U32 arch_rd_dfar(void);

U32 arch_rd_mpuir(void);
U32 arch_rd_mpurbar(void);
U32 arch_rd_mpurser(void);
U32 arch_rd_mpuracr(void);
U32 arch_rd_mpurgnr(void);
void arch_wr_mpurbar(U32);
void arch_wr_mpurser(U32);
void arch_wr_mpuracr(U32);
void arch_wr_mpurgnr(U32);
void arch_enable_async_abort(void);
void arch_disable_async_abort(void);

void arch_disable_cache(U32 flags);
void arch_enable_cache(U32 flags);
void arch_clean_cache_range(const void *start, U32 len);
void arch_clean_invalidate_cache_range(const void *start, U32 len);
void arch_invalidate_cache_range(const void *start, U32 len);

void *memcpy_aligned(void *dst, const void *src, size_t bytes);
void *memclr_aligned(void *dst, size_t bytes);
void fast_fifo_wr32(U32 fifo, U32 *data, U32 size);

static inline uint32_t arch_curr_cpu_num(void)
{
    return 0;
}
static inline void flush_dcache_range(uintptr_t addr, size_t size)
{
    arch_clean_invalidate_cache_range((const void *)addr, size);
}
extern void arch_enable_interrupt(void);
extern void arch_disable_interrupt(void);
#define el3_fiq_enable()    arch_enable_interrupt()

#endif
#endif
