/*
 * mpu.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Cortex V7R MPU driver.
 *
 * Revision History:
 * -----------------
 */
#include <arch/arm.h>
#include <arch/arm/mpu.h>
#include <assert.h>
#include <bits.h>
#include <pow2.h>

#if ARM_WITH_MPU

/* SCTLR bits */
#define SCTLR_M         (1 << 0)    /* Enable MPU */
#define SCTLR_BR        (1 << 17)   /* Enable MPU background region */

/* RBAR bits [31:5] */
#define MPU_RBAR_MASK   (0xffffffe0)

/* RACR bits */
#define MPU_RACR_B_SHIFT    (0)     /* Bufferable */
#define MPU_RACR_C_SHIFT    (1)     /* Cacheable */
#define MPU_RACR_S_SHIFT    (2)     /* Sharable. For normal memory only. */
#define MPU_RACR_TEX_SHIFT  (3)     /* Type extension */
#define MPU_RACR_AP_SHIFT   (8)     /* Access permission */
#define MPU_RACR_XN_SHIFT   (12)    /* Execution Never */

/* RACR Access permissions */
#define RACR_AP_NO_NO   (0)     /* Privileged NO, Unprivileged NO */
#define RACR_AP_RW_NO   (1)     /* Privileged RW, Unprivileged NO */
#define RACR_AP_RW_RO   (2)     /* Privileged RW, Unprivileged RO */
#define RACR_AP_RW_RW   (3)     /* Privileged RW, Unprivileged RW */
#define RACR_AP_RO_NO   (5)     /* Privileged RO, Unprivileged NO */
#define RACR_AP_RO_RO   (6)     /* Privileged RO, Unprivileged RO */

#define MPU_RACR_CONFIG(TEX, C, B, S, AP, XN) \
    ((B << MPU_RACR_B_SHIFT)      | \
     (C << MPU_RACR_C_SHIFT)      | \
     (S << MPU_RACR_S_SHIFT)      | \
     (TEX << MPU_RACR_TEX_SHIFT)  | \
     (AP << MPU_RACR_AP_SHIFT)    | \
     (XN << MPU_RACR_XN_SHIFT))

/* Region Size and Enable Register */
#define RSR_EN           (1 << 0)   /* Enable region */
#define RSR_SIZE_SHIFT   (1)        /* Region Size = log2(size) - 1 */

static const uint32_t g_racr_config[] = {
    [MPU_REGION_STRONGORDERED] = MPU_RACR_CONFIG(
        0, 0, 0,    /* See "TEX[2:0], C and B encodings" */
        0,          /* S bit is not used for strong ordered. */
        RACR_AP_RW_RW,
        1           /* XN */
    ),

    [MPU_REGION_DEVICE] = MPU_RACR_CONFIG(
        0, 0, 1,
        0,          /* S bit is not used for device memory */
        RACR_AP_RW_RW,
        1           /* XN */
    ),

    [MPU_REGION_NORMAL] = MPU_RACR_CONFIG(
        0, 1, 1,    /* cacheable, WB, no WA */
        0,          /* Not shared. R5 L1 cache does not cache shared
                       normal regions */
        RACR_AP_RW_RW,
        0           /* non XN */
    ),

    [MPU_REGION_NORMAL_NONCACHEABLE] = MPU_RACR_CONFIG(
        1, 0, 0,    /* non-cacheable */
        0,
        RACR_AP_RW_RW,
        0           /* non XN */
    ),

    [MPU_REGION_NORMAL_RO] = MPU_RACR_CONFIG(
        0, 1, 1,    /* cacheable, WB, no WA */
        0,          /* Not shared. R5 L1 cache does not cache shared
                       normal regions */
        RACR_AP_RO_RO,
        0           /* XN */
    ),

    [MPU_REGION_NO_ACCESS] = MPU_RACR_CONFIG(
        0,
        0,
        0,
        0,
        RACR_AP_NO_NO,
        0
    ),
};

static int log2ull(uint64_t val)
{
    return sizeof(val) * 8 - 1 - __builtin_clzll(val);
}

int mpu_region_nr(void)
{
    uint32_t val = arm_read_mpuir();
    return (int)BITS_SHIFT(val, 15, 8);   /* 0, 12 or 16 */
}

/*
 * Add MPU region.
 *
 * @region Region index from 0 to mpu_region_nr - 1.
 * @base   Base address of the region. Must align to region size.
 * @size   Region size. Must be power of 2, from 32 bytes to 4GB.
 * @type   Region type.
 */
#if ENABLE_CACHE_ALIGN_CHECK
struct region_info {
    uint32_t base;
    uint32_t end;
    //mpu_region_type type;
};
struct region_info region_array[16] = {0};
int region_index = 0;
static void add_region(uint32_t base, uint32_t size)
{
    ASSERT(region_index >= 0 && region_index < 16);
    region_array[region_index].base = base;
    region_array[region_index].end = base + size;
    region_index++;
}

bool mpu_is_belong_uncache_region(uint32_t base, uint32_t size)
{
    int i;

    for (i = 0; i < region_index; i++) {
        uint32_t end = base + size;

        if (base >= region_array[i].base && end <= region_array[i].end) {
            return true;
        }
    }

    return false;
}
#endif
void mpu_add_region(int region, uint32_t base, uint64_t size,
                    mpu_region_type_e type)
{
    ASSERT(region >= 0 && region < mpu_region_nr());
    ASSERT((size & (size - 1)) == 0 &&
           size >= 32ull &&
           size <= 4ull * 1024 * 1024 * 1024);
    ASSERT(base % size == 0);
    ASSERT(type >= MPU_REGION_STRONGORDERED && type < MPU_REGION_MAX);
    dprintf(INFO, "mpu_add_region %d: base 0x%x, size 0x%llx, type %d\n",
            region, base, size, type);
    /* Calculate size field of RSR register.
     *  pow(2, sz + 1) = size
     */
    uint32_t sz = (uint32_t)log2ull(size) - 1;
    arm_write_rgnr(region);
    arm_write_rbar(base & MPU_RBAR_MASK);
    arm_write_racr(g_racr_config[type]);
    /* TODO - support sub regions? */
    arm_write_rsr((sz << RSR_SIZE_SHIFT) | RSR_EN);
#if ENABLE_CACHE_ALIGN_CHECK

    //add region in nocache list
    if (type != MPU_REGION_NORMAL) {
        add_region(base, size);
    }

#endif
}

/*
 * Enable or disable the MPU.
 *
 * MPU regions must have been configured before enabling the MPU.
 */
void mpu_enable(bool enable)
{
    uint32_t val = arm_read_sctlr();

    if (enable)
        /* Enable MPU. The default background region is always
         * enabled as well.
         */
    {
        val |= SCTLR_BR | SCTLR_M;
    }
    else {
        val &= ~SCTLR_M;
    }

    arm_write_sctlr(val);
}

#endif /* ARM_WITH_MPU */
