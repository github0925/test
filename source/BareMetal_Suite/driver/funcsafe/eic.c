/********************************************************
 *      Copyright(c) 2019   Semidrive  Semiconductor    *
 *      All rights reserved.                            *
 ********************************************************/

#include "eic.h"

#define EIC_EN_OFF(n)   (0x100 + (n) * 4)
#define EIC_BIT_OFF(n)  (0x200 + (n) * 4)

void eic_inj_enable(uintptr_t b, U32 id) __RAM_FUNC__;
void eic_inj_enable(uintptr_t b, U32 id)
{
    U32 v = readl(b + EIC_EN_OFF(id / 32));
    v |= (0x01 << (id % 32));
    writel(v, b + EIC_EN_OFF(id / 32));
}

void eic_inj_disable(uintptr_t b, U32 id) __RAM_FUNC__;
void eic_inj_disable(uintptr_t b, U32 id)
{
    U32 v = readl(b + EIC_EN_OFF(id / 32));
    v &= ~(0x01 << (id % 32));
    writel(v, b + EIC_EN_OFF(id / 32));
}

#if defined(CFG_FS_EIC_API_eic_set_bit)
void eic_set_bit(uintptr_t b, U32 bp) __RAM_FUNC__;
void eic_set_bit(uintptr_t b, U32 bp)
{
    U32 v = readl(b + EIC_BIT_OFF(bp / 32));
    v |= (0x01 << (bp % 32));
    writel(v, b + EIC_BIT_OFF(bp / 32));
}
#endif

#if defined(CFG_FS_EIC_API_eic_clr_bit)
void eic_clr_bit(uintptr_t b, U32 bp) __RAM_FUNC__;
void eic_clr_bit(uintptr_t b, U32 bp)
{
    U32 v = readl(b + EIC_BIT_OFF(bp / 32));
    v &= ~(0x01 << (bp % 32));
    writel(v, b + EIC_BIT_OFF(bp / 32));
}
#endif
