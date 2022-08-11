/*
 * mmc_hal.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: emmc/sd hal driver.
 *
 * Revision History:
 * -----------------
 * 0.1, 10/18/2019 init version
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <debug.h>
#include <platform.h>
#include <platform/interrupts.h>
#include <trace.h>

#include <irq.h>
#include <mmc_sdhci.h>
#include <res.h>
#include <scr_hal.h>

#include "mmc_hal.h"

#define LOCAL_TRACE 0

#define MMC_BLOCK_SIZE (512)

#ifndef MMC_LOCAL_BUF_SIZE
#define MMC_LOCAL_BUF_SIZE (512)
#endif

#define MMC_MAX_XFER_SIZE (0x1000000)


static spin_lock_t mmc_spin_lock = SPIN_LOCK_INITIAL_VALUE;

static struct mmc_handle s_mmc_handle[DEFAULT_MMC_MAX_NUM] = {0};

static struct mmc_device s_mmc_dev[DEFAULT_MMC_MAX_NUM] = {0};

struct mmc_priv_res g_mmc_priv_res[DEFAULT_MMC_MAX_NUM] = {
    {0, RES_MSHC_SD1, MSHC1_INTR_NUM, MSHC1_WAKEUP_INTR_NUM,
     RES_MODULE_RST_SEC_MSHC1, RES_IP_SLICE_SEC_EMMC1, RES_GATING_EN_SEC_EMMC1,
     SCR_SEC__RW__mshc1_ddr_mode},
    {1, RES_MSHC_SD2, MSHC2_INTR_NUM, MSHC2_WAKEUP_INTR_NUM,
     RES_MODULE_RST_SEC_MSHC2, RES_IP_SLICE_SEC_EMMC2, RES_GATING_EN_SEC_EMMC2,
     SCR_SEC__RW__mshc2_ddr_mode},
    {2, RES_MSHC_SD3, MSHC3_INTR_NUM, MSHC3_WAKEUP_INTR_NUM,
     RES_MODULE_RST_SEC_MSHC3, RES_IP_SLICE_SEC_EMMC3, RES_GATING_EN_SEC_EMMC3,
     SCR_SEC__RW__mshc3_ddr_mode},
    {3, RES_MSHC_SD4, MSHC4_INTR_NUM, MSHC4_WAKEUP_INTR_NUM,
     RES_MODULE_RST_SEC_MSHC4, RES_IP_SLICE_SEC_EMMC4, RES_GATING_EN_SEC_EMMC4,
     SCR_SEC__RW__mshc4_ddr_mode},
};

static uint8_t local_buf[MMC_LOCAL_BUF_SIZE] __ALIGNED(CACHE_LINE);

static inline struct mmc_priv_res *get_priv_res(struct mmc_priv_res *res,
                                                uint32_t idx)
{
    for (int i = 0; i < DEFAULT_MMC_MAX_NUM; i++) {
        if (res[i].res_glb_idx == idx)
            return &res[i];
    }
    return NULL;
}

bool hal_mmc_creat_handle(void **handle, uint32_t mmc_res_glb_idx)
{
    bool ret;
    int res_id;
    addr_t apb_paddr;
    int32_t apb_offset;
    struct mmc_priv_res *priv_res;
    struct mmc_handle *p_handle;
    spin_lock_saved_state_t states;

    res_id = res_get_info_by_id(mmc_res_glb_idx, &apb_paddr, &apb_offset);
    if (res_id < 0) {
        *handle = NULL;
        return false;
    }
    dprintf(INFO, "res_id = %d, apb_paddr = 0x%lx, apb_offset = 0x%x \n",
            res_id, apb_paddr, apb_offset);

    priv_res = get_priv_res(g_mmc_priv_res, mmc_res_glb_idx);

    if (priv_res) {
        p_handle = &s_mmc_handle[priv_res->slot];
        spin_lock_irqsave(&mmc_spin_lock, states);
        if (p_handle->apb_base) {
            ret = false;
        }
        else {
            memset(p_handle, 0, sizeof(struct mmc_handle));
            p_handle->priv_res = priv_res;
            p_handle->apb_base = apb_paddr;
            p_handle->priv_data = &s_mmc_dev[priv_res->slot];
            memset(p_handle->priv_data, 0, sizeof(struct mmc_device));
            *handle = p_handle;
            ret = true;
        }
        spin_unlock_irqrestore(&mmc_spin_lock, states);
    }
    else {
        ret = false;
    }

    return ret;
}

bool hal_mmc_release_handle(void **handle)
{
    ASSERT(handle);
    struct mmc_handle *mmc = *handle;
    spin_lock_saved_state_t states;

    spin_lock_irqsave(&mmc_spin_lock, states);
    mmc->apb_base = 0;
    mask_interrupt(mmc->priv_res->int_irq);
    *handle = NULL;
    spin_unlock_irqrestore(&mmc_spin_lock, states);

    return true;
}

int hal_mmc_init(void *handle)
{
    ASSERT(handle);
    struct mmc_handle *mmc = handle;
    struct mmc_device *dev = mmc->priv_data;
    struct mmc_config_data *mmc_cfg = &dev->config;

    mmc_cfg->slot = mmc->priv_res->slot;
    mmc_cfg->irq = mmc->priv_res->int_irq;
    mmc_cfg->sdhc_base = mmc->apb_base;
    mmc_cfg->max_clk_rate = mmc->config->max_clk_rate;
    mmc_cfg->bus_width = mmc->config->bus_width;
    mmc_cfg->voltage = mmc->config->voltage;
    mmc_cfg->hs200_support = mmc->config->hs200_support;
    mmc_cfg->hs400_support = mmc->config->hs400_support;

    if (mmc_platform_init(dev)) {
        dprintf(CRITICAL, "platform init failed!\n");
        return -1;
    }

    dev->host.parent = mmc;

    if (!mmc->async_mode && mmc->event_handle) {
        dprintf(CRITICAL,
                "parameter error, async mode is %s and event hand is %s !\n",
                mmc->async_mode ? "true" : "false",
                mmc->event_handle ? "not null" : "null");
        return -1;
    }
    dev->host.async_mode = mmc->async_mode;

    mmc->block_size = MMC_BLOCK_SIZE;

    if (dev->card.type < MMC_TYPE_MMCHC) {
        dprintf(INFO, "\n sd card au_size %d, num_aus %d\n",
                dev->card.ssr.au_size, dev->card.ssr.num_aus);
        mmc->erase_grp_size =
            dev->card.ssr.au_size * dev->card.ssr.num_aus * MMC_BLOCK_SIZE;
    }
    else {
        dprintf(INFO, "\n mmc card erase_size %d, erase_mult %d\n",
                dev->card.csd.erase_grp_size, dev->card.csd.erase_grp_mult);
        mmc->erase_grp_size = (dev->card.csd.erase_grp_size + 1) *
                              (dev->card.csd.erase_grp_mult + 1) *
                              MMC_BLOCK_SIZE;
    }

    return 0;
}

static inline void *calc_adma_boundary(addr_t input, addr_t boundary)
{
    return  (void *)((input + boundary) & ~(boundary - 1));
}

int hal_mmc_read(void *handle, mmc_address_t src, uint8_t *dst,
                 mmc_length_t length)
{
    LTRACEF("handle %p, src %lld, dst %p, len %lld\n", handle, src, dst,
            length);
    ASSERT(handle);
    ASSERT(IS_ALIGNED(src, MMC_BLOCK_SIZE));
    ASSERT(IS_ALIGNED(length, MMC_BLOCK_SIZE));

    struct mmc_handle *mmc = handle;
    struct mmc_device *dev = mmc->priv_data;
    mmc_length_t remaining = length;
    uint32_t read_len;
    uint8_t *buf = dst;
    uint8_t use_local_buf = 0;
    mmc_length_t max_read_length = MMC_MAX_XFER_SIZE;

    if (!IS_ALIGNED(dst, CACHE_LINE)) {
        max_read_length = MMC_LOCAL_BUF_SIZE;
        use_local_buf = 1;
    }

    while (remaining) {
        if (use_local_buf)
            buf = local_buf;

        read_len = MIN(remaining, max_read_length);
        if (mmc_sdhci_read(dev, buf, src / MMC_BLOCK_SIZE,
                            read_len / MMC_BLOCK_SIZE)) {
            return -1;
        }

        if (buf != dst)
            memcpy(dst, buf, read_len);
        else
            buf += read_len;

        remaining -= read_len;
        dst += read_len;
        src += read_len;
    }
    return 0;
}

int hal_mmc_write(void *handle, mmc_address_t dst, const uint8_t *src_buf,
                  mmc_length_t length)
{
    LTRACEF("handle %p, dst %lld, len %lld, buf %p\n", handle, dst, length,
            src_buf);
    ASSERT(handle);
    ASSERT(IS_ALIGNED(dst, MMC_BLOCK_SIZE));
    ASSERT(IS_ALIGNED(length, MMC_BLOCK_SIZE));

    struct mmc_handle *mmc = handle;
    struct mmc_device *dev = mmc->priv_data;
    mmc_length_t remaining = length;
    uint32_t write_len;
    uint8_t *buf;
    uint8_t use_local_buf = 0;
    mmc_length_t max_write_length = MMC_MAX_XFER_SIZE;

    if (!IS_ALIGNED(src_buf, CACHE_LINE)) {
        max_write_length = MMC_LOCAL_BUF_SIZE;
        use_local_buf = 1;
    }

    while (remaining) {
        buf = (uint8_t *)src_buf;
        write_len = MIN(remaining, max_write_length);

        if (use_local_buf) {
            buf = local_buf;
            memcpy(buf, src_buf, write_len);
        }

        if (mmc_sdhci_write(dev, buf, dst / MMC_BLOCK_SIZE,
                            write_len / MMC_BLOCK_SIZE)) {
            return -1;
        }
        remaining -= write_len;
        src_buf += write_len;
        dst += write_len;
    }
    return 0;
}

/* The erase length must multiple the device erase group unit size */
int hal_mmc_erase(void *handle, mmc_address_t dst, mmc_length_t length)
{
    LTRACEF("handle %p, dst %lld, len %lld\n", handle, dst, length);
    ASSERT(handle);
    struct mmc_handle *mmc = handle;
    /* The erase address and size need erase group size aligned */
    ASSERT(DIV_ROUND_UP(dst, mmc->erase_grp_size) == dst / mmc->erase_grp_size);
    ASSERT(DIV_ROUND_UP(length, mmc->erase_grp_size) ==
                 length / mmc->erase_grp_size);

    struct mmc_device *dev = mmc->priv_data;

    return mmc_sdhci_erase(dev, dst / MMC_BLOCK_SIZE, length);
}

int hal_mmc_cancel(void *handle)
{
    LTRACEF("handle %p\n", handle);
    ASSERT(handle);

    struct mmc_handle *mmc = handle;
    struct mmc_device *dev = mmc->priv_data;

    return mmc_sdhci_cancel(dev);
}

mmc_length_t hal_mmc_get_capacity(void *handle)
{
    ASSERT(handle);
    struct mmc_handle *mmc = handle;
    struct mmc_device *dev = mmc->priv_data;

    if (dev) {
        return dev->card.capacity;
    }
    else {
        return -1;
    }
}

uint32_t hal_mmc_get_block_size(void *handle)
{
    ASSERT(handle);
    struct mmc_handle *mmc = handle;

    if (handle) {
        return mmc->block_size;
    }
    else {
        return -1;
    }
}

uint32_t hal_mmc_get_erase_grp_size(void *handle)
{
    ASSERT(handle);
    struct mmc_handle *mmc = handle;

    if (handle) {
        return mmc->erase_grp_size;
    }
    else {
        return -1;
    }
}

int hal_mmc_switch_part(void *handle, enum part_access_type part_no)
{
    LTRACEF("handle %p, part_no %d\n", handle, part_no);
    ASSERT(handle);
    struct mmc_handle *mmc = handle;
    struct mmc_device *dev = mmc->priv_data;

    return mmc_sdhci_switch_part(dev, part_no);
}
