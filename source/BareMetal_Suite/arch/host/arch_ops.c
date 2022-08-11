/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    arch.h
 * @brief
 */

#include "arch.h"

#if defined(SIM_AS_kunlun)
extern U32 sctlr;
extern U32 mpuir;
extern U32 mpurgnr;
extern U32 mpu_rgn_regs[16][3];

U32 arch_rd_sctlr(void)
{
    return sctlr;
}

void arch_wr_sctlr(U32 val)
{
    sctlr = val;
}

U32 arch_rd_mpuir(void)
{
    return (16UL << 8);
}

U32 arch_rd_mpurbar(void)
{
    return mpu_rgn_regs[mpurgnr][0];
}

U32 arch_rd_mpurser(void)
{
    return mpu_rgn_regs[mpurgnr][1];
}

U32 arch_rd_mpuracr(void)
{
    return mpu_rgn_regs[mpurgnr][2];
}

U32 arch_rd_mpurgnr(void)
{
    return mpurgnr;
}

void arch_wr_mpurbar(U32 val)
{
    mpu_rgn_regs[mpurgnr][0] = val;
}

void arch_wr_mpurser(U32 val)
{
    mpu_rgn_regs[mpurgnr][1] = val;
}

void arch_wr_mpuracr(U32 val)
{
    mpu_rgn_regs[mpurgnr][2] = val;
}

void arch_wr_mpurgnr(U32 val)
{
    mpurgnr = val;
}
#endif

void arch_disable_cache(U32 flags)
{
}

void arch_enable_cache(U32 flags)
{
}

void clean_cache_range(const void *start, U32 len)
{
}

void clean_invalidate_cache_range(const void *start, U32 len)
{
}

void invalidate_cache_range(const void *start, U32 len)
{
}

void fast_fifo_wr32(U32 fifo, U32 *data, U32 size)
{
    for (int i = 0; i < size; i++, data++) {
        U32 v = readl(fifo);
        writel(v, data);
    }
}
