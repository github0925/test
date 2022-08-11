/*
 * Copyright (c) 2018, Linaro Limited. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	lk/cache.h
 * @brief	LK cache operation primitives for libmetal.
 */

#ifndef __METAL_CACHE__H__
#error "Include metal/cache.h instead of metal/lk/cache.h"
#endif

#include <arch/ops.h>

#ifndef __METAL_LK_CACHE__H__
#define __METAL_LK_CACHE__H__

#ifdef __cplusplus
extern "C" {
#endif

static inline void __metal_cache_flush(void *addr, unsigned int len)
{
	arch_clean_cache_range(addr, len);
}

static inline void __metal_cache_invalidate(void *addr, unsigned int len)
{
	arch_invalidate_cache_range(addr, len);
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_LK_CACHE__H__ */
