/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	lk/sys.h
 * @brief	LK system primitives for libmetal.
 */

#ifndef __METAL_SYS__H__
#error "Include metal/sys.h instead of metal/lk/sys.h"
#endif

#ifndef __METAL_LK_SYS__H__
#define __METAL_LK_SYS__H__


#ifdef __cplusplus
extern "C" {
#endif

#ifndef METAL_MAX_DEVICE_REGIONS
#define METAL_MAX_DEVICE_REGIONS 1
#endif

/** Structure for FreeRTOS libmetal runtime state. */
struct metal_state {

	/** Common (system independent) data. */
	struct metal_common_state common;
};

#ifdef METAL_INTERNAL

/**
 * @brief restore interrupts to state before disable_global_interrupt()
 */
inline static
void sys_irq_restore_enable(unsigned int flags)
{
    return;
}

/**
 * @brief disable all interrupts
 */
inline static
unsigned int sys_irq_save_disable(void)
{
    return 0;
}

#endif /* METAL_INTERNAL */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_LK_SYS__H__ */
