/********************************************************
 *      Copyright(c) 2019   Semidrive  Semiconductor    *
 *      All rights reserved.                            *
 ********************************************************/

#ifndef __EIC_H__
#define __EIC_H__

#include <common_hdr.h>

void eic_inj_enable(uintptr_t b, U32 id);
void eic_inj_disable(uintptr_t b, U32 id);
void eic_set_bit(uintptr_t b, U32 bp);
void eic_clr_bit(uintptr_t b, U32 bp);

#endif
