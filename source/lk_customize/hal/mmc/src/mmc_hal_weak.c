/*
 * mmc_hal_weak.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: emmc/sd driver.
 *
 * Revision History:
 * -----------------
 * 0.1, 10/18/2019 init version
 */

#include <stdbool.h>
#include <stdint.h>
#include <platform.h>

#include "mmc_hal.h"

bool hal_mmc_creat_handle(void **handle, uint32_t mmc_res_glb_idx)
{
    return true;
}

bool hal_mmc_release_handle(void **handle) { return true; }

int hal_mmc_init(void *handle) { return 0; }

int hal_mmc_read(void *handle, mmc_address_t src, uint8_t *dst,
                 mmc_length_t length)
{
    return 0;
}

int hal_mmc_write(void *handle, mmc_address_t dst, const uint8_t *src_buf,
                  mmc_length_t length)
{
    return 0;
}

int hal_mmc_cancel(void *handle) { return 0; }

/* The erase length must multiple the device erase group unit size */
int hal_mmc_erase(void *handle, mmc_address_t dst, mmc_length_t length)
{
    return 0;
}

mmc_length_t hal_mmc_get_capacity(void *handle) { return 0; }

uint32_t hal_mmc_get_block_size(void *handle) { return 0; }

uint32_t hal_mmc_get_erase_grp_size(void *handle) { return 0; }

int hal_mmc_switch_part(void *handle, enum part_access_type part_no)
{
    return 0;
}
