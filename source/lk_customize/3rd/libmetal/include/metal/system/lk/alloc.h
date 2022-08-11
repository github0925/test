/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	freertos/alloc.c
 * @brief	FreeRTOS libmetal memory allocattion definitions.
 */

#ifndef __METAL_ALLOC__H__
#error "Include metal/alloc.h instead of metal/freertos/alloc.h"
#endif

#ifndef __METAL_LK_ALLOC__H__
#define __METAL_LK_ALLOC__H__

#include <lib/heap.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void *metal_allocate_memory(unsigned int size)
{
	return (malloc(size));
}

static inline void metal_free_memory(void *ptr)
{
	free(ptr);
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_LK_ALLOC__H__ */
