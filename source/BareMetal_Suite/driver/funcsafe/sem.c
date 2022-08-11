/********************************************************
 *      Copyright(c) 2019   Semidrive  Semiconductor    *
 *      All rights reserved.                            *
 ********************************************************/

#include <common_hdr.h>
#include "sem.h"

void sem_ei_glb_en(uintptr_t b)
{
    U32 v = readl(b + COMMON_SET_OFF);
    v |= BM_COMMON_SET_EI_GLB_EN;
    writel(v, b + COMMON_SET_OFF);
}

#if defined(SEM_APIs_sem_ei_glb_dis)
void sem_ei_glb_dis(uintptr_t b) __RAM_FUNC__;
void sem_ei_glb_dis(uintptr_t b)
{
    U32 v = readl(b + COMMON_SET_OFF);
    v &= ~BM_COMMON_SET_EI_GLB_EN;
    writel(v, b + COMMON_SET_OFF);
}
#endif

bool sem_input_status(uintptr_t b, U32 id) __RAM_FUNC__;
bool sem_input_status(uintptr_t b, U32 id)
{
    U32 v = readl(b + INT_STATUS_OFF(id / 32));
    return (0 != (v & (0x01U << (id % 32))));
}
