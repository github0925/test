/********************************************************
 *      Copyright(c) 2019   Semidrive  Semiconductor    *
 *      All rights reserved.                            *
 ********************************************************/

#ifndef __SEM_H__
#define __SEM_H__

#include <common_hdr.h>
#include "sem_hw.h"

#define COMMON_SET_OFF  0
#define BM_COMMON_SET_EI_GLB_EN     (0x01U << 16)
#define INT_STATUS_OFF(n)       (0x80U + 4 * (n))

void sem_ei_glb_en(uintptr_t b);
void sem_ei_glb_dis(uintptr_t b);
bool sem_input_status(uintptr_t b, U32 id);

#endif  /* __SEM_H__ */
