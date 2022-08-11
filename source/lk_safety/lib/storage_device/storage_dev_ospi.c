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
#include <assert.h>
#include <debug.h>
#include <spi_nor_hal.h>
#include "chip_res.h"
#include "storage_dev_ospi.h"

static uint32_t virtual_block_size = 512;

int ospi_init(storage_device_t *storage_dev, uint32_t res_idx,
              void *config)
{
    int ret = 0;
    void *handle;
    struct  spi_nor_handle *ospi_dev_handle;

    ret = hal_spi_nor_creat_handle(&handle, res_idx);

    if (!ret) {
        dprintf(CRITICAL, "ospi create handle failed\n");
        return -1;
    }

    ospi_dev_handle = handle;
    ospi_dev_handle->config = config;

    ret = hal_spi_nor_init(ospi_dev_handle);

    if (ret) {
        dprintf(CRITICAL, "ospi hal init failed\n");
        hal_spi_nor_release_handle(&handle);
        ospi_dev_handle = NULL;
        return -1;
    }

    storage_dev->priv = ospi_dev_handle;

    return 0;
}

int ospi_release(storage_device_t *storage_dev)
{
    if (storage_dev->priv)
        return hal_spi_nor_release_handle(&(storage_dev->priv));

    dprintf(CRITICAL, "Do opsi hal init firstly\n");
    return -1;
}

uint32_t ospi_get_block_size(storage_device_t *storage_dev)
{
    dprintf(INFO, "return virtual block size\n");
    return virtual_block_size;
}

int ospi_read(storage_device_t *storage_dev, uint64_t src, uint8_t *dst,
              uint64_t size)
{
    if (storage_dev->priv)
        return hal_spi_nor_read(storage_dev->priv, src, dst, size);

    dprintf(CRITICAL, "Do ospi hal init firstly\n");
    return -1;
}

int ospi_write(storage_device_t *storage_dev, uint64_t dst,
               const uint8_t *buf, uint64_t data_len)
{
    if (storage_dev->priv) {
        return hal_spi_nor_write(storage_dev->priv, dst, buf, data_len);
    }

    dprintf(CRITICAL, "Do ospi hal init firstly\n");
    return -1;
}

int ospi_erase(storage_device_t *storage_dev, uint64_t dst, uint64_t len)
{
    if (storage_dev->priv) {
        return hal_spi_nor_erase(storage_dev->priv, dst, len);
    }

    dprintf(CRITICAL, "Do ospi hal init firstly\n");
    return -1;
}

int ospi_cached_write(storage_device_t *storage_dev, uint64_t dst,
               const uint8_t *buf, uint64_t data_len)
{
    int ret = 0;
    uint8_t *sector_buf;
    uint32_t sect_base_addr, sect_offset, sect_boundary_lens;
    uint64_t addr = dst;
    uint64_t remaining = data_len;
    struct  spi_nor_handle *dev_handle = storage_dev->priv;
    uint32_t sector_size = dev_handle->block_size;

    sector_buf = memalign(CACHE_LINE, sector_size);
    if (sector_buf == NULL) {
        dprintf(CRITICAL, "%s: alloc memory failed\n", __FUNCTION__);
        return -1;
    }

    if (storage_dev->priv) {
        while(remaining) {
            sect_base_addr = calc_sect_base_addr(addr, sector_size);
            sect_offset = calc_sect_offset(addr, sector_size);
            sect_boundary_lens = calc_sect_boundary_lens(addr, remaining,
                                                         sector_size);
            if (sect_boundary_lens != sector_size) {
                ret = hal_spi_nor_read(storage_dev->priv, sect_base_addr,
                                       sector_buf, sector_size);
                if (ret) {
                    ret = -1;
                    dprintf(CRITICAL, "%s: read failed!\n", __FUNCTION__);
                    goto free_write_buf;
                }
            }

            memcpy(sector_buf + sect_offset, buf,  sect_boundary_lens);

            hal_spi_nor_erase(storage_dev->priv, sect_base_addr, sector_size);
            if (ret) {
                ret = -1;
                dprintf(CRITICAL, "%s: erase failed!\n", __FUNCTION__);
                goto free_write_buf;
            }

            hal_spi_nor_write(storage_dev->priv, sect_base_addr, sector_buf,
                              sector_size);
            if (ret) {
                ret = -1;
                dprintf(CRITICAL, "%s: write failed!\n", __FUNCTION__);
                goto free_write_buf;
            }

            addr += sect_boundary_lens;
            buf += sect_boundary_lens;
            remaining -= sect_boundary_lens;
        }
    } else {
        dprintf(CRITICAL, "Do ospi hal init firstly\n");
        ret = -1;
    }

free_write_buf:

    free(sector_buf);
    return ret;
}

bool ospi_need_erase(storage_device_t *storage_dev)
{
    return true;
}

uint64_t ospi_get_capacity(storage_device_t *storage_dev)
{
    if (storage_dev->priv)
        return hal_spi_nor_get_capacity(storage_dev->priv);

    dprintf(CRITICAL, "Do ospi hal init firstly\n");
    return 0;
}

uint32_t ospi_get_erase_group_size(storage_device_t *storage_dev)
{
    struct  spi_nor_handle *dev_handle = storage_dev->priv;

    if (storage_dev->priv)
        return dev_handle->block_size;
    else
        return 0;
}
