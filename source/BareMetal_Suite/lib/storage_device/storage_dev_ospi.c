/*
 * storage_dev_ospi.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Revision History:
 * -----------------
 * 0.1, 9/18/2019 init version
 */

#include <stdint.h>
#include <stdlib.h>
#include <debug.h>
#include "storage_dev_ospi.h"
#include "mini_libc.h"

#define OSPI_BASE 0x4000000U

#ifndef EMMC_BOOT

#ifdef NOR_FLASH_SECTOR_SZ_256K
#define NOR_FLASH_SECTOR_SZ (256 * 1024)
#else
#define NOR_FLASH_SECTOR_SZ (4 * 1024)
#endif

static uint32_t virtual_block_size = 512;
struct ospi_info {
    uint8_t *base;
};

static struct ospi_info ospi_info;

int ospi_init(storage_device_t *storage_dev, uint32_t res_idx,
              void *config)
{
    ospi_info.base = (uint8_t *)OSPI_BASE;
    storage_dev->priv = &ospi_info;
    return 0;
}

int ospi_release(storage_device_t *storage_dev)
{
    return 0;
}

uint32_t ospi_get_block_size(storage_device_t *storage_dev)
{
    return virtual_block_size;
}

int ospi_read (storage_device_t *storage_dev, uint64_t src,
               uint8_t *dst, uint64_t size)
{
    struct ospi_info *info = storage_dev->priv;

    mini_memcpy_s(dst, (void *)(info->base + src), size);
    return 0;
}

uint8_t *ospi_read_ptr(storage_device_t *storage_dev, uint64_t src,
                       uint64_t size)
{
    struct ospi_info *info = storage_dev->priv;

    return (uint8_t *)(info->base + src);
}

int ospi_write(storage_device_t *storage_dev, uint64_t dst,
               const uint8_t *buf, uint64_t data_len)
{
    return -1;
}

int ospi_erase(storage_device_t *storage_dev, uint64_t dst, uint64_t len)
{
    return -1;
}

uint64_t ospi_get_capacity(storage_device_t *storage_dev)
{
    return 0;
}

uint32_t ospi_get_erase_group_size(storage_device_t *storage_dev)
{
    return NOR_FLASH_SECTOR_SZ;
}
#endif
