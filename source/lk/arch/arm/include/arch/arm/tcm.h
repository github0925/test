/*
 * tcm.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Cortex V7R TCM driver.
 *
 * Revision History:
 * -----------------
 */
#ifndef _ARCH_ARM_TCM_H
#define _ARCH_ARM_TCM_H

#ifndef ASSEMBLY

#include <compiler.h>
#include <stdbool.h>
#include <sys/types.h>

__BEGIN_CDECLS

void tcm_get_size(size_t *atcm_size, size_t *btcm_size);
void tcm_enable(uint32_t atcm_base, uint32_t btcm_base, bool enable_ecc);

__END_CDECLS

#endif /* ASSEMBLY */

#endif /* _ARCH_ARM_TCM_H */
