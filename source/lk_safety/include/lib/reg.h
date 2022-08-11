/*
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 */

#ifndef _LIB_REG_H
#define _LIB_REG_H

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#if WITH_KERNEL_VM
# define v2p(va)        (paddr_t)(vaddr_to_paddr(va))
# define p2v(pa)        (vaddr_t)(paddr_to_kvaddr(pa))
#else
# define v2p(va)        (paddr_t)(va)
# define p2v(pa)        (vaddr_t)(pa)
#endif

#define _vaddr(pa)      p2v(pa)
#define _paddr(va)      v2p(va)
#define _ioaddr(pa)     _vaddr(pa)
#endif /* _LIB_REG_H */
