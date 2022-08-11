/*
 * spi_nor_hal_weak.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: spi norflash hal driver.
 *
 * Revision History:
 * -----------------
 * 0.1, 11/10/2019 init version
 */

#include <stdint.h>
#include <stdbool.h>
#include <platform.h>

#include "spi_nor_hal.h"

bool hal_spi_nor_creat_handle(void **handle, uint32_t spi_nor_res_glb_idx)
{
    return true;
}

bool hal_spi_nor_release_handle(void **handle)
{
    return true;
}

int hal_spi_nor_init(void *handle)
{
    return 0;
}

int hal_spi_nor_read(void *handle, spi_nor_address_t src, uint8_t *dst,
                     spi_nor_length_t length)
{
    return 0;
}

int hal_spi_nor_write(void *handle, spi_nor_address_t dst,
                      const uint8_t *src_buf, spi_nor_length_t length)
{
    return 0;
}

int hal_spi_nor_cancel(void *handle)
{
    return 0;
}

/* The erase length must multiple the device erase group unit size */
int hal_spi_nor_erase(void *handle, spi_nor_address_t dst,
                      spi_nor_length_t length)
{
    return 0;
}

spi_nor_length_t hal_spi_nor_get_capacity(void *handle)
{
    return 0;
}

spi_nor_length_t hal_spi_nor_get_flash_id(void *handle)
{
    return 0;
}
