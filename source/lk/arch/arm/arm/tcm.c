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
#include <arch/arm.h>
#include <arch/arm/tcm.h>
#include <assert.h>
#include <bits.h>
#include <stdlib.h>

#if ARM_WITH_TCM

#define ATCMPCEN        (1ul << 25)
#define B0TCMPCEN       (1ul << 26)
#define B1TCMPCEN       (1ul << 27)

/**
 * @brief Clear the TCM.
 */
static void tcm_clear(uint32_t base, size_t size)
{
    uint32_t a = 0;
    uint32_t addr;

    /* Each store must be 64 bits aligned. */
    for (addr = base; addr < base + size; addr += 8) {
        __asm__ volatile(
            "STRD %0, %1, [%2]" : : "r" (a), "r" (a), "r" (addr)
        );
    }
}

/**
 * @brief Get TCM size in bytes.
 */
void tcm_get_size(size_t *atcm_size, size_t *btcm_size)
{
    ASSERT(atcm_size && btcm_size);

    uint32_t asize = BITS_SHIFT(arm_read_atcmrgn(), 6, 2);
    uint32_t bsize = BITS_SHIFT(arm_read_btcmrgn(), 6, 2);

    *atcm_size = 1 << (asize + 9);
    *btcm_size = 1 << (bsize + 9);
}

/**
 * @brief Enable the TCM.
 */
void tcm_enable(uint32_t atcm_base, uint32_t btcm_base, bool enable_ecc)
{
    size_t      atcm_size, btcm_size;
    uint32_t    actlr = arm_read_actlr();

    /* Disable ECC by default */
    actlr &= ~(ATCMPCEN | B0TCMPCEN | B1TCMPCEN);
    arm_write_actlr(actlr);

    /* Get TCM size. */
    tcm_get_size(&atcm_size, &btcm_size);

    /* Update TCM base address. */
    if (atcm_size != 0) {
        ASSERT(IS_ALIGNED(atcm_base, atcm_size));
        arm_write_atcmrgn(atcm_base | 1);
    }

    if (btcm_size != 0) {
        ASSERT(IS_ALIGNED(btcm_base, btcm_size));
        arm_write_btcmrgn(btcm_base | 1);
    }

    if (enable_ecc) {
        /* Write to the TCM in 64-bit aligned quantities to initialize
         * ECC codes.
         */
        tcm_clear(atcm_base, atcm_size);
        tcm_clear(btcm_base, btcm_size);

        /* Enable ECC.  */
        actlr |= ATCMPCEN | B0TCMPCEN | B1TCMPCEN;
        arm_write_actlr(actlr);
    }
}

#endif /* ARM_WITH_TCM */
