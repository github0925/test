/*
* Copyright (c) 2019 Semidrive Semiconductor Inc.
* All rights reserved.
*
* Description: common function for arm
*
*/

#include <common/arm/platform.h>
#include <arch/arm/mpu.h>
#include <__regs_base.h>

int platform_mpu_r5_common(int region)
{
#if ARM_WITH_MPU
    extern addr_t _nocacheable_start;
    extern addr_t _nocacheable_end;

    if ((&_nocacheable_end - &_nocacheable_start) > 0) {
        mpu_add_region(region++, (addr_t)&_nocacheable_start,
                       &_nocacheable_end - &_nocacheable_start, MPU_REGION_NORMAL_NONCACHEABLE);
    }

    mpu_add_region(region++, APB_CKGEN_SEC_BASE, 0x04000000, MPU_REGION_DEVICE);

#endif
    return region;
}
