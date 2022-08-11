/*
 * storage_dev_mmc.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: emmc/sd driver.
 *
 * Revision History:
 * -----------------
 * 0.1, 9/18/2019 init version
 */
#include <arch/defines.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <debug.h>
#include "mmc_hal.h"

#include "storage_dev_mmc.h"

/* 64KB for zero erase size */
#define ZERO_ERASE_SIZE_DEFAULT  (0x010000)
int mmc_init(storage_device_t *storage_dev, uint32_t res_idex,
             void *config)
{
    int ret = 0;
    void *handle;
    struct mmc_handle *mmc_dev_handle;

    ret = hal_mmc_creat_handle(&handle, res_idex);

    if (!ret) {
        dprintf(CRITICAL, "mmc create handle failed\n");
        return -1;
    }

    mmc_dev_handle = handle;
    mmc_dev_handle->config = config;

    printf("mmc_dev_handle  %p\n", mmc_dev_handle);
    ret = hal_mmc_init(mmc_dev_handle);

    if (ret) {
        dprintf(CRITICAL, "mmc hal init failed\n");
        hal_mmc_release_handle(handle);
        mmc_dev_handle = NULL;
        return -1;
    }

    storage_dev->priv = mmc_dev_handle;
    return 0;
}

int mmc_release(storage_device_t *storage_dev)
{
    if (storage_dev->priv)
        return hal_mmc_release_handle(&(storage_dev->priv));

    dprintf(CRITICAL, "Do mmc hal init firstly\n");
    return -1;
}

uint32_t mmc_get_block_size(storage_device_t *storage_dev)
{
    if (storage_dev->priv) {
        return  hal_mmc_get_block_size(storage_dev->priv);
    }

    dprintf(CRITICAL, "Do mmc hal init firstly\n");
    return 0;
}

int mmc_read(storage_device_t *storage_dev, uint64_t src, uint8_t *dst,
             uint64_t size)
{
    if (storage_dev->priv)
        return hal_mmc_read(storage_dev->priv, src, dst, size);

    dprintf(CRITICAL, "Do mmc hal init firstly\n");
    return -1;
}

int mmc_write(storage_device_t *storage_dev, uint64_t dst,
              const uint8_t *buf, uint64_t data_len)
{
    if (storage_dev->priv)
        return hal_mmc_write(storage_dev->priv, dst, buf, data_len);

    dprintf(CRITICAL, "Do mmc hal init firstly\n");
    return -1;
}

int mmc_switch_part(storage_device_t *storage_dev, uint32_t part_no)
{
    if (storage_dev->priv)
        return hal_mmc_switch_part(storage_dev->priv, part_no);

    dprintf(CRITICAL, "Do mmc hal init firstly\n");
    return -1;
}

/* if the size < erase group size, we write zero in mmc instead of erase */
static int mmc_write_zero(storage_device_t *storage_dev, uint64_t dst,
                          uint32_t count, uint32_t blz)
{
    int ret = -1;
    uint8_t *zero = NULL;
    uint64_t zero_size = 0;
    uint64_t zero_buf_size = 0;

    zero_size = blz * count;
    zero_buf_size = blz * count;

    if (!count)
        return 0;

    if (zero_buf_size > ZERO_ERASE_SIZE_DEFAULT)
        zero_buf_size = round_down(ZERO_ERASE_SIZE_DEFAULT, (uint64_t)blz);

    zero = (uint8_t *)memalign(blz,  zero_buf_size);

    if (!zero) {
        dprintf(CRITICAL, "%s malloc failure\n", __func__);
        goto end;
    }

    memset((void *)zero, 0x0, zero_buf_size);

    for (uint32_t i = 0; i < zero_size / zero_buf_size; i++) {
        ret = hal_mmc_write(storage_dev->priv, dst, zero, zero_buf_size);
        dst += zero_buf_size;

        if (ret) {
            dprintf(CRITICAL, "%s write failure\n", __func__);
            goto end;
        }
    }

    if (zero_size % zero_buf_size)
        ret = hal_mmc_write(storage_dev->priv, dst, zero, zero_size % zero_buf_size);

end:

    if (zero)
        free(zero);

    return ret;
}

int mmc_erase(storage_device_t *storage_dev, uint64_t dst, uint64_t len)
{
    int ret = 0;
    uint32_t blz = 0;
    uint64_t count = 0;
    uint32_t erase_grp_sz = 0;

    if (!storage_dev->priv) {
        dprintf(CRITICAL, "Do mmc hal init firstly\n");
        return -1;
    }

    blz = hal_mmc_get_block_size(storage_dev->priv);
    erase_grp_sz = hal_mmc_get_erase_grp_size(storage_dev->priv);

    dprintf(INFO, "erase grp:%u dst:%llu, len:%llu\n", erase_grp_sz, dst, len);

    if (dst % blz || len % blz || erase_grp_sz % blz || !len) {
        dprintf(CRITICAL,
                "%s dst:%llu len:%llu not aligned, block size:%u erase grp size:%u!\n",
                __func__, dst, len, blz, erase_grp_sz);
        return -1;
    }

    while (len) {

        if (len < erase_grp_sz) {
            count = len / blz;
            ret |=  mmc_write_zero(storage_dev, dst, count, blz);
            dprintf(INFO, "%s %d dst:%llu, len:%llu\n", __func__, __LINE__, dst,
                    count * blz);
            break;
        }
        else if (dst % erase_grp_sz) {
            count = (round_up(dst,  erase_grp_sz) - dst) / blz;
            ret |= mmc_write_zero(storage_dev, dst, count, blz);
            len -= count * blz;
            dst += count * blz;
            dprintf(INFO, "%s %d dst:%llu, len:%llu\n", __func__, __LINE__, dst,
                    count * blz);
        }
        else {
            count = len / erase_grp_sz;
            ret |= hal_mmc_erase(storage_dev->priv, dst, count * erase_grp_sz);

            dst += count * erase_grp_sz;
            len -= count * erase_grp_sz;
            dprintf(INFO, "%s %d dst:%llu, len:%llu\n", __func__, __LINE__, dst,
                    count * erase_grp_sz);
        }
    }

    return ret;
}

uint64_t mmc_get_capacity(storage_device_t *storage_dev)
{
    if (storage_dev->priv)
        return hal_mmc_get_capacity(storage_dev->priv);

    return 0;
}

uint32_t mmc_get_erase_group_size(storage_device_t *storage_dev)
{
    if (storage_dev->priv) {
        return hal_mmc_get_erase_grp_size(storage_dev->priv);
    }
    else {
        return 0;
    }
}
