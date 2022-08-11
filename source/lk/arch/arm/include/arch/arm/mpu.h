/*
 * mpu.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Cortex V7R MPU driver.
 *
 * Revision History:
 * -----------------
 */
#ifndef _ARCH_ARM_MPU_H
#define _ARCH_ARM_MPU_H

#ifndef ARM_ISA_ARMV7
#error "MPU code for ARMv7 only"
#endif

/*
 * Memory types supported by the MPU driver.
 */
typedef enum mpu_region_type {
    /* Strong ordered
     *  - non-bufferable, non-cachable, shareable, RW, XN
     */
    MPU_REGION_STRONGORDERED,

    /* Device memory
     *  - bufferable, non-cachable, shareable, RW, XN
     */
    MPU_REGION_DEVICE,

    /* Normal memory
     *  - bufferable, outer and inner write-back & write allocate,
     *    shareble, RW, non XN
     */
    MPU_REGION_NORMAL,

    /* Normal non-cacheable memory
     *  - bufferable, outer and inner non-cacheable, shareble, RW,
     *    non XN
     */
    MPU_REGION_NORMAL_NONCACHEABLE,

    /* Not accessable.
     */
    MPU_REGION_NO_ACCESS,

    MPU_REGION_MAX,
} mpu_region_type_e;

#ifndef ASSEMBLY

#include <compiler.h>
#include <stdbool.h>
#include <sys/types.h>

__BEGIN_CDECLS

int mpu_region_nr(void);
void mpu_add_region(int region, uint32_t base, uint32_t size,
                    mpu_region_type_e type);
void mpu_enable(bool enable);
bool mpu_is_belong_uncache_region(uint32_t base, uint32_t size);
__END_CDECLS

#endif /* ASSEMBLY */

#endif