/*
 * gic.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef _GIC_H
#define _GIC_H

#include "__regs_base.h"
#include "target_res.h"

#define GICBASE(n)  (GIC2_BASE)
#define GICD_OFFSET (0x1000)
#define GICC_OFFSET (0x2000)

#define MAX_INT         MAX_INT_NUM

#endif /* _GIC_H */
