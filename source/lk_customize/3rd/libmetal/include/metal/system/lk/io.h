/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	lk/io.h
 * @brief	LK specific io definitions.
 */

#ifndef __METAL_IO__H__
#error "Include metal/io.h instead of metal/lk/io.h"
#endif

#ifndef __METAL_LK_IO__H__
#define __METAL_LK_IO__H__

#include <stdlib.h>
#include <lib/reg.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef METAL_INTERNAL

/**
 * @brief memory mapping for an I/O region
 */
void metal_sys_io_mem_map(struct metal_io_region *io);

/**
 * @brief memory mapping
 */
inline static
void *metal_machine_io_mem_map(void *va, metal_phys_addr_t pa,
			       size_t size, unsigned int flags)
{
    (void)size;
    (void)flags;
    va = _ioaddr((paddr_t)pa);
    return va;
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* __METAL_LK_IO__H__ */
