/*
 * Copyright (c) 2014 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

#include <stdbool.h>

#define SPIN_LOCK_INITIAL_VALUE (0)

typedef unsigned long spin_lock_t;

typedef unsigned int spin_lock_saved_state_t;
typedef unsigned int spin_lock_save_flags_t;

#if WITH_SMP
void arch_spin_lock(spin_lock_t *lock);
int arch_spin_trylock(spin_lock_t *lock);
void arch_spin_unlock(spin_lock_t *lock);
#else
static inline void arch_spin_lock(spin_lock_t *lock)
{
    *lock = 1;
}

static inline int arch_spin_trylock(spin_lock_t *lock)
{
    return 0;
}

static inline void arch_spin_unlock(spin_lock_t *lock)
{
    *lock = 0;
}
#endif

static inline void arch_spin_lock_init(spin_lock_t *lock)
{
    *lock = SPIN_LOCK_INITIAL_VALUE;
}

static inline bool arch_spin_lock_held(spin_lock_t *lock)
{
    return *lock != 0;
}
