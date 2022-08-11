/*
 * spi_nor_hal.c
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

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <debug.h>
#include <platform.h>
#include <platform/interrupts.h>
#include <trace.h>

#include "rstgen_hal.h"
#include "chip_res.h"
#include "clkgen_hal.h"
#include "irq.h"
#include "scr_hal.h"

#include "spi_nor_hal.h"
#include "spi_nor.h"

#undef LOCAL_TRACE
#define LOCAL_TRACE 0

#ifndef SPINOR_LOCAL_BUF_SIZE
#define SPINOR_LOCAL_BUF_SIZE (256)
#endif

spin_lock_t spi_nor_spin_lock = SPIN_LOCK_INITIAL_VALUE;

static struct spi_nor_handle s_spi_nor_handle[DEFAULT_SPI_NOR_MAX_NUM] = { 0 };

static struct spi_nor s_spi_nor_dev[DEFAULT_SPI_NOR_MAX_NUM] = { 0 };

static struct spi_nor_priv_res s_spi_nor_priv_res[DEFAULT_SPI_NOR_MAX_NUM] = {
    { 0, OSPI1_O_INTERRUPT_NUM, RES_OSPI_REG_OSPI1, RES_OSPI_OSPI1,
      RES_MODULE_RST_SAF_OSPI1, RES_IP_SLICE_SAF_OSPI1},
    { 1, OSPI2_O_INTERRUPT_NUM, RES_OSPI_REG_OSPI2, RES_OSPI_OSPI2,
      RES_MODULE_RST_SEC_OSPI2, RES_IP_SLICE_SEC_OSPI2},
};

static uint8_t local_buf[SPINOR_LOCAL_BUF_SIZE] __ALIGNED(CACHE_LINE);

static bool scr_write_signal(const scr_signal_t signal, uint32_t val)
{
    scr_handle_t handle;

    handle = hal_scr_create_handle(signal);

    if (handle) {
        bool ret = hal_scr_set(handle, val);
        if (!ret)
            dprintf(CRITICAL, "spinor set scr signal failed!\n");
        hal_scr_delete_handle(handle);
        return ret;
    }
    else {
        dprintf(CRITICAL, "spinor create scr handle failed!\n");
        return false;
    }
}

static void ospi_scr_remap(u32 slot, bool enable, u32 offset, u32 low, u32 up)
{
    if (slot == 1) {
        if (enable) {
            scr_write_signal(SCR_SAFETY__L31__remap_ospi1_imagd_2nd_offset_19_0,
                             offset);
            scr_write_signal(
                SCR_SAFETY__L31__remap_ospi1_imagd_2nd_low_limit_19_0, low);
            scr_write_signal(
                SCR_SAFETY__L31__remap_ospi1_imagd_2nd_up_limit_19_0, up);
        }
        scr_write_signal(SCR_SAFETY__L31__remap_ospi1_remap_en, enable);
    }
    else if (slot == 2) {
        if (enable) {
            scr_write_signal(SCR_SEC__L31__remap_ospi2_imagd_2nd_offset_19_0,
                             offset);
            scr_write_signal(SCR_SEC__L31__remap_ospi2_imagd_2nd_low_limit_19_0,
                             low);
            scr_write_signal(SCR_SEC__L31__remap_ospi2_imagd_2nd_up_limit_19_0,
                             up);
        }
        scr_write_signal(SCR_SEC__L31__remap_ospi2_remap_en, enable);
    }
}

static bool model_reset(uint32_t res_id)
{
    bool ret = true;
    static void *g_handle;
    ret = hal_rstgen_creat_handle(&g_handle, RES_GLOBAL_RST_SEC_RST_EN);

    if (!ret) {
        return -1;
    }

    /*get handle ok and enable rstgen is true*/
    ret = hal_rstgen_init(g_handle);

    if (ret) {
        ret = hal_rstgen_module_reset(g_handle, res_id);
    }

    ret &= hal_rstgen_release_handle(g_handle);

    return ret;
}

static inline uint32_t get_priv_res(struct spi_nor_priv_res *res, uint32_t idx)
{
    for (int i = 0; i < DEFAULT_SPI_NOR_MAX_NUM; i++) {
        if (res[i].res_reg_glb_idx == idx)
            return res[i].slot;
    }
    return -1;
}

bool hal_spi_nor_creat_handle(void **handle, uint32_t spi_nor_res_glb_idx)
{
    bool ret;
    struct spi_nor_handle *p_handle;
    int res_id;
    uint32_t slot;
    addr_t apb_paddr;
    addr_t ahb_paddr;
    int32_t apb_offset;
    int32_t ahb_offset;
    spin_lock_saved_state_t states;

    res_id = res_get_info_by_id(spi_nor_res_glb_idx, &apb_paddr, &apb_offset);
    if (res_id < 0) {
        *handle = NULL;
        return false;
    }
    dprintf(INFO, "res_id = %d, apb_paddr = 0x%lx, apb_offset = 0x%x \n",
            res_id, apb_paddr, apb_offset);

    slot = get_priv_res(s_spi_nor_priv_res, spi_nor_res_glb_idx);

    res_id = res_get_info_by_id(s_spi_nor_priv_res[slot].res_glb_idx,
                                &ahb_paddr, &ahb_offset);
    if (res_id < 0) {
        *handle = NULL;
        return false;
    }
    dprintf(INFO, "res_id = %d, ahb_paddr = 0x%lx, ahb_offset = 0x%x \n",
            res_id, ahb_paddr, ahb_offset);

    p_handle = &s_spi_nor_handle[slot];
    spin_lock_irqsave(&spi_nor_spin_lock, states);
    if (p_handle->apb_base) {
        ret = false;
    }
    else {
        memset(p_handle, 0, sizeof(struct spi_nor_handle));
        p_handle->id = slot + 1;
        p_handle->apb_base = apb_paddr;
        p_handle->ahb_base = ahb_paddr;
        p_handle->irq = s_spi_nor_priv_res[slot].irq;
        p_handle->priv_data = &s_spi_nor_dev[slot];
        *handle = p_handle;
        ret = true;
    }
    spin_unlock_irqrestore(&spi_nor_spin_lock, states);

    return ret;
}

bool hal_spi_nor_release_handle(void **handle)
{
    ASSERT(handle);
    struct spi_nor_handle *spi_nor = *handle;
    struct spi_nor *dev = spi_nor->priv_data;
    spin_lock_saved_state_t states;

    spi_nor_deinit(dev);
    if (spi_nor->irq)
        mask_interrupt(spi_nor->irq);

    spin_lock_irqsave(&spi_nor_spin_lock, states);
    spi_nor->apb_base = 0;
    *handle = NULL;
    spin_unlock_irqrestore(&spi_nor_spin_lock, states);

    return true;
}

static uint32_t spi_nor_get_refclk(struct spi_nor_handle *handle)
{
    uint32_t ref_clock;
    uint32_t res_glb_idx = s_spi_nor_priv_res[handle->id - 1].ckgen_res_glb_idx;
    void *ckgen_handle;

    if (!hal_clock_creat_handle(&ckgen_handle)) {
        dprintf(CRITICAL, "%s: clkgen creat handle failed\n", __FUNCTION__);
        return 0;
    }

    ref_clock = hal_clock_ipclk_get(ckgen_handle, res_glb_idx,
                                      mon_ref_clk_24M, 0);
    dprintf(CRITICAL, "%s: ref_clock = %d\n", __FUNCTION__, ref_clock);
    /*release clock handle*/
    hal_clock_release_handle(ckgen_handle);

    return ref_clock;
}

int hal_spi_nor_init(void *handle)
{
    ASSERT(handle);
    struct spi_nor_handle *spi_nor = handle;
    struct spi_nor *dev = spi_nor->priv_data;
    struct spi_nor_config *spi_nor_cfg = &dev->config_data;

    spi_nor_cfg->octal_ddr_en = spi_nor->config->octal_ddr_en;
    spi_nor_cfg->cs = spi_nor->config->cs;
    spi_nor_cfg->bus_clk = spi_nor->config->bus_clk;
    spi_nor_cfg->id = spi_nor->id;
    spi_nor_cfg->apb_base = spi_nor->apb_base;
    spi_nor_cfg->ahb_base = spi_nor->ahb_base;
    spi_nor_cfg->irq = spi_nor->irq;
    spi_nor_cfg->clk = 300000000;

    if (spi_nor_init(dev)) {
        dprintf(CRITICAL, "norflash init failed!\n");
        return -1;
    }

    dev->parent = spi_nor;

    if (!spi_nor->async_mode && spi_nor->event_handle) {
        dprintf(CRITICAL,
                "parameter error, async mode is %s and event hand is %s !\n",
                spi_nor->async_mode ? "true" : "false",
                spi_nor->event_handle ? "not null" : "null");
        return -1;
    }
    dev->async_mode = spi_nor->async_mode;

    spi_nor->block_size = dev->block_size;

    return 0;
}

inline int hal_spi_nor_read(void *handle, spi_nor_address_t src, uint8_t *dst,
                            spi_nor_length_t length)
{
    LTRACEF("handle %p, src %lld, dst %p, len %lld\n", handle, src, dst,
            length);
    ASSERT(handle);
    struct spi_nor_handle *spi_nor = handle;
    struct spi_nor *dev = spi_nor->priv_data;
    uint32_t read_len;

    /* the memory address not aligned with cache line size */
    if (!IS_ALIGNED(dst, CACHE_LINE)) {
        read_len = ROUNDUP((addr_t)dst, CACHE_LINE) - (addr_t)dst;
        read_len = MIN(read_len, length);

        if (spi_nor_read(dev, src, local_buf, ROUNDUP(read_len, 4)))
            return -1;

        memcpy(dst, local_buf, read_len);
        length -= read_len;
        dst += read_len;
        src += read_len;
    }

    if (length) {
        read_len = ROUNDDOWN(length, CACHE_LINE);
        if (read_len) {
            if (spi_nor_read(dev, src, dst, read_len))
                return -1;

            length -= read_len;
            dst += read_len;
            src += read_len;
        }
    }

    if (length) {
        read_len = length;
        if (spi_nor_read(dev, src, local_buf, ROUNDUP(read_len, 4)))
            return -1;
        memcpy(dst, local_buf, read_len);
        length -= read_len;
    }

    assert(!length);
    return 0;
}

inline int hal_spi_nor_write(void *handle, spi_nor_address_t dst,
                             const uint8_t *src_buf, spi_nor_length_t length)
{
    LTRACEF("handle %p, dst %lld, len %lld, buf %p\n", handle, dst, length,
            src_buf);
    ASSERT(handle);

    struct spi_nor_handle *spi_nor = handle;
    struct spi_nor *dev = spi_nor->priv_data;
    uint32_t write_len;
    uint8_t *buf = (uint8_t *)src_buf;
    uint8_t use_local_buf = 0;
    spi_nor_length_t max_write_length = length;
    spi_nor_length_t remaining = length;

    if (!IS_ALIGNED(src_buf, CACHE_LINE)) {
        max_write_length = SPINOR_LOCAL_BUF_SIZE;
        use_local_buf = 1;
    }

    while (remaining) {
        write_len = MIN(remaining, max_write_length);

        if (use_local_buf) {
            buf = local_buf;
            memcpy(buf, src_buf, write_len);
        }

        if (spi_nor_write(dev, dst, buf, write_len))
            return -1;

        remaining -= write_len;
        src_buf += write_len;
        dst += write_len;
    }

    return 0;
}

/* The erase address and length must aligned with block size */
int hal_spi_nor_erase(void *handle, spi_nor_address_t dst,
                      spi_nor_length_t length)
{
    LTRACEF("handle %p, dst %lld, len %lld\n", handle, dst, length);
    ASSERT(handle);
    struct spi_nor_handle *spi_nor = handle;
    ASSERT(IS_ALIGNED(dst, spi_nor->block_size));
    ASSERT(IS_ALIGNED(length, spi_nor->block_size));

    struct spi_nor *dev = spi_nor->priv_data;

    return spi_nor_erase(dev, dst, length);
}

int hal_spi_nor_cancel(void *handle)
{
    LTRACEF("handle %p\n", handle);
    ASSERT(handle);

    struct spi_nor_handle *spi_nor = handle;
    struct spi_nor *dev = spi_nor->priv_data;

    return spi_nor_cancel(dev);
}

spi_nor_length_t hal_spi_nor_get_capacity(void *handle)
{
    ASSERT(handle);
    struct spi_nor_handle *spi_nor = handle;
    struct spi_nor *dev = spi_nor->priv_data;

    return spi_nor_get_capacity(dev);
}

spi_nor_length_t hal_spi_nor_get_flash_id(void *handle)
{
    ASSERT(handle);
    struct spi_nor_handle *spi_nor = handle;
    struct spi_nor *dev = spi_nor->priv_data;

    return spi_nor_get_flash_id(dev);
}

void hal_spi_nor_reset_slave(void *handle)
{
    spi_nor_reset_slave(((struct spi_nor_handle*)handle)->apb_base);
}
