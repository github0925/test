/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    arch.h
 * @brief   header files for host.
 */

#ifndef __ARCH_H__
#define __ARCH_H__

#include <common_hdr.h>

#define DCACHE      0x01
#define ICACHE      0x02
#define CACHE_LINE  64

#define BM_SCTLR_M  (0x01UL << 0)
#define BM_SCTLR_A  (0x01UL << 1)
#define BM_SCTLR_C  (0x01UL << 2)

#define dsb()
#define isb()
#define dmb()
#define nop()

void arch_enable_branch_predict(void);
void arch_invalidate_tlb(void);

U32 arch_rd_sctlr(void);
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

void arch_disable_cache(U32 flags);
void arch_enable_cache(U32 flags);
void clean_cache_range(const void *start, U32 len);
void clean_invalidate_cache_range(const void *start, U32 len);
void invalidate_cache_range(const void *start, U32 len);

void fast_fifo_wr32(U32 fifo, U32 *data, U32 size);

#endif
