/*
* g2dlite_hal.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 04/27/2020 BI create this file
*/
#ifndef __G2DLITE_HAL_H__
#define __G2DLITE_HAL_H__
#include "__regs_base.h"
#include <irq.h>

#define G2DLITE_MAX_NUM             1

static const struct hal_g2dlite_addr2irq_t g2dlite_addr2irq_table[G2DLITE_MAX_NUM] = {
    { APB_G2D2_BASE, G2D2_G2D_IRQ_NUM},
};
#endif //__G2DLITE_HAL_H__
