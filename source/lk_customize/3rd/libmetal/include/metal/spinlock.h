/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * @file	spinlock.h
 * @brief	Spinlock primitives for libmetal.
 */

#ifndef __METAL_SPINLOCK__H__
#define __METAL_SPINLOCK__H__

#include <kernel/spinlock.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \defgroup spinlock Spinlock Interfaces
 *  @{ */
struct metal_spinlock {
	spin_lock_t v;
};

/** Static metal spinlock initialization. */
#define METAL_SPINLOCK_INIT		{ATOMIC_FLAG_INIT}

/**
 * @brief	Initialize a libmetal spinlock.
 * @param[in]	slock	Spinlock to initialize.
 */
static inline void metal_spinlock_init(struct metal_spinlock *slock)
{
	spin_lock_init(&slock->v);
}

/**
 * @brief	Acquire a spinlock.
 * @param[in]	slock   Spinlock to acquire.
 * @see metal_spinlock_release
 */
static inline void metal_spinlock_acquire(struct metal_spinlock *slock)
{
	spin_lock(&slock->v);
}

/**
 * @brief	Release a previously acquired spinlock.
 * @param[in]	slock	Spinlock to release.
 * @see metal_spinlock_acquire
 */
static inline void metal_spinlock_release(struct metal_spinlock *slock)
{
	spin_unlock(&slock->v);
}

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_SPINLOCK__H__ */
