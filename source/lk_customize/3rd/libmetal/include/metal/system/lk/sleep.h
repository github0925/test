/*
 * Copyright (c) 2018, Linaro Limited. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	lk/sleep.h
 * @brief	LK sleep primitives for libmetal.
 */

#ifndef __METAL_SLEEP__H__
#error "Include metal/sleep.h instead of metal/lk/sleep.h"
#endif

#ifndef __METAL_LK_SLEEP__H__
#define __METAL_LK_SLEEP__H__

#include <debug.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int __metal_sleep_usec(unsigned int usec)
{
	spin(usec);
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_LK_SLEEP__H__ */
