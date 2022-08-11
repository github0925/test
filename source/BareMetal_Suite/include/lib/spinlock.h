#ifndef _SPIN_LOCK_H_
#define _SPIN_LOCK_H_

#include "arch/spinlock.h"

typedef   spin_lock_t   spinlock_t;

void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);

#define spin_lock       arch_spin_lock
#define spin_unlock     arch_spin_unlock

#endif  /* ifndef _SPIN_LOCK_H_ */

