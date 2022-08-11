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
#include <stdint.h>
#include <stdlib.h>
#include <compiler.h>
#include <debug.h>
#include <mmc/mmc_sdhci.h>

#include "storage_dev_mmc.h"

#ifdef EMMC_BOOT

#define MMC_BLOCK_SIZE (512)
#define MMC_MAX_XFER_SIZE (0x1000000)

#define round_boundary(value, boundary)     \
    ((__typeof__(value))((boundary) - 1))

#define round_up(value, boundary)       \
    ((((value) - 1) | round_boundary(value, boundary)) + 1)

#define round_down(value, boundary)     \
    ((value) & ~round_boundary(value, boundary))

static uint8_t buf_algined[MMC_BLOCK_SIZE] __ALIGNED(MMC_BLOCK_SIZE);

#if SDCARD_BOOT
static struct mmc_device mmc_dev = {
	.config = {
		.slot = 2,
		.irq = 146,
		.sdhc_base = 0xf41a0000,
		.max_clk_rate = 50000000,
		.bus_width = 1,
		.voltage = 7,
		.hs200_support = 0,
		.hs400_support = 0,
	}
};
#else
static struct mmc_device mmc_dev = {
    .config = {
        .slot = 0,
        .irq = 142,
        .sdhc_base = 0xf4180000,
        .max_clk_rate = 200000000,
        .bus_width = 2,
        .voltage = 5,
        .hs200_support = 0,
        .hs400_support = 0,
    }
};
#endif

/* 64KB for zero erase size */
#define ZERO_ERASE_SIZE_DEFAULT  (512)
uint8_t zero_buf[512] __ALIGNED(512) = {0};

int mmc_init(storage_device_t *storage_dev, uint32_t res_idex,
             void *config)
{
    struct mmc_device *mmc = &mmc_dev;
    struct mmc_config_data *mmc_cfg = &mmc->config;

    if (config)
        memcpy(mmc_cfg, config, sizeof(struct mmc_config_data));

    if (mmc_platform_init(mmc)) {
        FATAL("mmc platform init failed!\n");
        return -1;
    }

    storage_dev->priv = mmc;
    return 0;
}

int mmc_release(storage_device_t *storage_dev)
{
    if (storage_dev->priv) {
        storage_dev->priv = NULL;
        return 0;
    }

    FATAL("Do mmc hal init firstly\n");
    return -1;
}

uint32_t mmc_get_block_size(storage_device_t *storage_dev)
{
    if (storage_dev->priv) {
        return  MMC_BLOCK_SIZE;
    }

    FATAL("Do mmc hal init firstly\n");
    return 0;
}

int mmc_read(storage_device_t *storage_dev, uint64_t src, uint8_t *dst,
             uint64_t size)
{
    struct mmc_device *dev = storage_dev->priv;
    uint64_t remaining = size;
    uint32_t read_len;
    uint8_t *buf = dst;
    uint64_t unaligned = 0;

    assert(IS_ALIGNED(dst, CACHE_LINE));
    assert(IS_ALIGNED(size, MMC_BLOCK_SIZE));

    if (storage_dev->priv == NULL) {
        FATAL("Do mmc hal init firstly\n");
        return -1;
    }

    unaligned = ROUNDUP(src, (uint64_t)MMC_BLOCK_SIZE) - src;
    src = ROUNDDOWN(src, (uint64_t)MMC_BLOCK_SIZE);

    if (unaligned) {
        if (mmc_sdhci_read(dev, buf_algined, src / MMC_BLOCK_SIZE, 1)) {
            return -1;
        }

        src += MMC_BLOCK_SIZE;
    }

    while (remaining) {
        read_len = MIN(remaining, MMC_MAX_XFER_SIZE);

        if (mmc_sdhci_read(dev, buf, src / MMC_BLOCK_SIZE,
                           read_len / MMC_BLOCK_SIZE)) {
            return -1;
        }

        buf += read_len;
        remaining -= read_len;
        src += read_len;
    }

    if (unaligned) {
        memmove(dst + unaligned, dst, size - unaligned);
        memcpy(dst, buf_algined + MMC_BLOCK_SIZE - unaligned, unaligned);
    }

    return 0;
}

int mmc_write(storage_device_t *storage_dev, uint64_t dst,
              const uint8_t *src_buf, uint64_t data_len)
{
    assert(IS_ALIGNED(src_buf, MMC_BLOCK_SIZE));
    assert(IS_ALIGNED(dst, MMC_BLOCK_SIZE));
    assert(IS_ALIGNED(data_len, MMC_BLOCK_SIZE));

    if (storage_dev->priv == NULL) {
        FATAL("Do mmc hal init firstly\n");
        return -1;
    };

    struct mmc_device *dev = storage_dev->priv;

    uint64_t remaining = data_len;

    uint32_t write_len;

    uint8_t *buf = (uint8_t *)src_buf;

    while (remaining) {
        write_len = MIN(remaining, MMC_MAX_XFER_SIZE);

        if (mmc_sdhci_write(dev, buf, dst / MMC_BLOCK_SIZE,
                            write_len / MMC_BLOCK_SIZE)) {
            return -1;
        }

        remaining -= write_len;
        buf += write_len;
        dst += write_len;
    }

    return 0;
}

int mmc_switch_part(storage_device_t *storage_dev, uint32_t part_no)
{
    if (storage_dev->priv) {
        struct mmc_device *dev = storage_dev->priv;

        return mmc_sdhci_switch_part(dev, part_no);
    }

    FATAL("Do mmc hal init firstly\n");
    return -1;
}

/* if the size < erase group size, we write zero in mmc instead of erase */
int mmc_write_zero(storage_device_t *storage_dev, uint64_t dst, uint32_t count,
                   uint32_t blz)
{
    if (storage_dev->priv == NULL) {
        FATAL("Do mmc hal init firstly\n");
        return -1;
    };

    int ret = -1;

    uint8_t *zero = NULL;

    uint64_t zero_size = 0;

    uint64_t zero_buf_size = 0;

    zero_size = blz * count;

    zero_buf_size = blz * count;

    struct mmc_device *dev = storage_dev->priv;

    if (!count)
        return 0;

    if (zero_buf_size > ZERO_ERASE_SIZE_DEFAULT)
        zero_buf_size = round_down(ZERO_ERASE_SIZE_DEFAULT, (uint64_t)blz);

    zero = zero_buf;

    for (uint32_t i = 0; i < zero_size / zero_buf_size; i++) {

        ret = mmc_sdhci_write(dev, zero, dst, zero_buf_size);
        dst += zero_buf_size;

        if (ret) {
            FATAL("%s write failure\n", __func__);
            return -1;
        }
    }

    return 0;
}

uint32_t mmc_get_erase_group_size(storage_device_t *storage_dev)
{
    if (storage_dev->priv == NULL) {
        FATAL("Do mmc hal init firstly\n");
        return -1;
    };

    struct mmc_device *dev = storage_dev->priv;

    if (dev->card.type < MMC_TYPE_MMCHC) {
        INFO("\n sd card au_size %d, num_aus %d\n",
             dev->card.ssr.au_size, dev->card.ssr.num_aus);
        dev->erase_grp_size =
            dev->card.ssr.au_size * dev->card.ssr.num_aus * MMC_BLOCK_SIZE;
    }
    else {
        INFO("\n mmc card erase_size %d, erase_mult %d\n",
             dev->card.csd.erase_grp_size, dev->card.csd.erase_grp_mult);
        dev->erase_grp_size = (dev->card.csd.erase_grp_size + 1) *
                              (dev->card.csd.erase_grp_mult + 1) *
                              MMC_BLOCK_SIZE;
    }

    return dev->erase_grp_size;
}

int mmc_erase(storage_device_t *storage_dev, uint64_t dst, uint64_t len)
{
    int ret = 0;
    uint32_t blz = 0;
    uint64_t count = 0;
    uint32_t erase_grp_sz = 0;
    struct mmc_device *dev = storage_dev->priv;

    if (!storage_dev->priv) {
        FATAL("Do mmc hal init firstly\n");
        return -1;
    }

    blz = MMC_BLOCK_SIZE;
    erase_grp_sz = mmc_get_erase_group_size(storage_dev);

    if (dst % blz || len % blz || erase_grp_sz % blz || !len) {
        FATAL("%s dst:%llu len:%llu not aligned, block size:%u erase grp size:%u!\n",
              __func__, dst, len, blz, erase_grp_sz);
        return -1;
    }

    while (len) {

        if (len < erase_grp_sz) {
            count = len / blz;
            ret |=  mmc_write_zero(storage_dev, dst, count, blz);
            INFO("%s %d dst:%llu, len:%llu\n", __func__, __LINE__, dst, count * blz);
            break;
        }
        else if (dst % erase_grp_sz) {
            count = (round_up(dst,  erase_grp_sz) - dst) / blz;
            ret |= mmc_write_zero(storage_dev, dst, count, blz);
            len -= count * blz;
            dst += count * blz;
            INFO("%s %d dst:%llu, len:%llu\n", __func__, __LINE__, dst, count * blz);
        }
        else {
            count = len / erase_grp_sz;
            ret |= mmc_sdhci_erase(dev, dst / MMC_BLOCK_SIZE, count * erase_grp_sz);

            dst += count * erase_grp_sz;
            len -= count * erase_grp_sz;
            INFO("%s %d dst:%llu, len:%llu\n", __func__, __LINE__, dst,
                 count * erase_grp_sz);
        }
    }

    return ret;
}

uint64_t mmc_get_capacity(storage_device_t *storage_dev)
{
    if (storage_dev->priv) {
        struct mmc_device *dev = storage_dev->priv;

        return dev->card.capacity;
    }

    return 0;
}
#endif
