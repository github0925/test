/*
 * app_mmc.c
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SD/eMMC sample.
 */
#include <stdlib.h>
#include <string.h>
#include <lib/console.h>
#include <app.h>
#include <debug.h>
#include <kernel/event.h>

#include <chip_res.h>
#include <mmc_hal.h>

static event_t opt_complete_event;

struct mmc_test_cfg {
    uint32_t id;
    struct mmc_cfg config;
};

static struct mmc_test_cfg mmc_cfg_data = {
    .id = RES_MSHC_SD1,
    .config =
        {
            .voltage = MMC_VOL_1_8,
            .max_clk_rate = MMC_CLK_100MHZ,
            .bus_width = MMC_BUS_WIDTH_8BIT,
        },
};

static int mmc_test_tansfer(struct mmc_handle *dev, u32 block, u32 data,
                            u32 num, u32 async_mode, u32 test_cancel)
{
    u32 ret = 0;
    u32 block_size;
    u32 erase_grp_size;

    block_size = hal_mmc_get_block_size(dev);
    erase_grp_size = hal_mmc_get_erase_grp_size(dev);

    uint8_t *mem1 = memalign(CACHE_LINE, ROUNDUP(block_size * num, CACHE_LINE));
    uint8_t *mem2 = memalign(CACHE_LINE, ROUNDUP(block_size * num, CACHE_LINE));

    memset(mem1, 0, block_size * num);
    memset(mem2, 0, block_size * num);

    uint32_t i;
    volatile uint32_t *p1 = (volatile uint32_t *)mem1;
    for (i = 0; i < 128 * num; i++, p1++) {
        if (data == UINT32_MAX) {
            *p1 = rand();
        }
        else {
            *p1 = data;
        }
    }

    /* for debug, print the last block data */
    printf("\nmmc test original data, the last block data:\n");
    hexdump8(mem1 + (num - 1) * block_size, block_size);

    /* test mmc read write */
    ret = hal_mmc_erase(dev, block * block_size,
                        ROUNDUP(num * block_size, erase_grp_size));
    if (ret) {
        printf("%s: erase error, ret = %d\n", __FUNCTION__, ret);
        goto tansfer_out;
    }

    ret = hal_mmc_write(dev, block * block_size, mem1, num * block_size);
    if (ret) {
        printf("%s: write error, ret = %d\n", __FUNCTION__, ret);
        goto tansfer_out;
    }
    if (async_mode) {
        if (test_cancel) {
            ret = hal_mmc_cancel(dev);
            printf(
                "%s: cancel the the previous read/write operation, ret = %d\n",
                __FUNCTION__, ret);
        }
        else {
            event_wait(&opt_complete_event);
            printf("%s: write complete\n", __FUNCTION__);
        }
    }

    ret = hal_mmc_read(dev, block * block_size, mem2, num * block_size);
    if (ret) {
        printf("%s: read error, ret = %d\n", __FUNCTION__, ret);
        goto tansfer_out;
    }
    if (async_mode) {
        event_wait(&opt_complete_event);
        printf("%s: read complete\n", __FUNCTION__);
    }

    /* for debug, print the last block data */
    hexdump8(mem2 + (num - 1) * block_size, block_size);

tansfer_out:
    dprintf(INFO, "\n");
    free(mem1);
    free(mem2);
    return ret;
}

static bool mmc_sync_mode_sample(void)
{
    dprintf(CRITICAL, "mmc sync sample start!\n");
    void *handle;
    int ret = 0;
    mmc_length_t capacity;

    dprintf(CRITICAL, " mmc test creat handle ...\n");
    ret = hal_mmc_creat_handle(&handle, mmc_cfg_data.id);
    dprintf(CRITICAL, " mmc test creat handle result: %s\n",
            ret ? "PASS" : "FAILED");
    if (!ret)
        return false;

    ((struct mmc_handle *)handle)->config = &mmc_cfg_data.config;
    dprintf(CRITICAL, " mmc test init mmc device ...\n");
    ret = hal_mmc_init(handle);
    dprintf(CRITICAL, " mmc test init mmc device result: %s\n",
            ret ? "FAILED" : "PASS");
    if (ret) {
        ret = 1;
        goto sync_release_handle;
    }

    dprintf(CRITICAL, " mmc get capacity ...\n");
    capacity = hal_mmc_get_capacity(handle);
    dprintf(CRITICAL, " mmc get capacity = %llu result: %s\n", capacity,
            capacity ? "PASS" : "FAILED");
    if (!capacity) {
        ret = 1;
        goto sync_release_handle;
    }

    dprintf(CRITICAL, " mmc switch partiion to boot1 ...\n");
    ret = hal_mmc_switch_part(handle, PART_ACCESS_BOOT1);
    dprintf(CRITICAL, " mmc switch partiion to boot1 result: %s\n",
            ret ? "FAILED" : "PASS");
    if (ret) {
        ret = 1;
        goto sync_release_handle;
    }

    dprintf(CRITICAL, "\n mmc test transfer ...\n");
    ret = mmc_test_tansfer(handle, 100, UINT32_MAX, 10, 0, 0);
    dprintf(CRITICAL, " mmc test transfer result: %s\n",
            ret ? "FAILED" : "PASS");
    if (ret) {
        ret = 1;
        goto sync_release_handle;
    }

sync_release_handle:
    dprintf(CRITICAL, " mmc test release handle ...\n");
    ret += hal_mmc_release_handle(&handle);
    dprintf(CRITICAL, " mmc test release handle result: %s\n",
            ret ? "PASS" : "FAILED");

    if (ret != 1)
        return false;

    dprintf(CRITICAL, "\nmmc sync test end!\n");
    return true;
}

static void test_event_handle(enum mmc_opt type, enum mmc_opt_result result)
{
    dprintf(CRITICAL, " mmc opt event: type = %d, result = %d\n", type, result);
    event_signal(&opt_complete_event, false);
}

static bool mmc_async_mode_sample(void)
{
    dprintf(CRITICAL, "mmc async test start!\n");
    void *handle;
    struct mmc_handle *mmc_handle;
    int ret = 0;
    mmc_length_t capacity;

    event_init(&opt_complete_event, false, EVENT_FLAG_AUTOUNSIGNAL);

    dprintf(CRITICAL, " mmc test creat handle ...\n");
    ret = hal_mmc_creat_handle(&handle, mmc_cfg_data.id);
    dprintf(CRITICAL, " mmc test creat handle result: %s\n",
            ret ? "PASS" : "FAILED");
    if (!ret)
        return false;

    mmc_handle = handle;
    mmc_handle->config = &mmc_cfg_data.config;
    mmc_handle->async_mode = 1;
    mmc_handle->event_handle = test_event_handle;

    dprintf(CRITICAL, " mmc test init mmc device ...\n");
    ret = hal_mmc_init(handle);
    dprintf(CRITICAL, " mmc test init mmc device result: %s\n",
            ret ? "FAILED" : "PASS");
    if (ret) {
        ret = 1;
        goto async_release_handle;
    }

    dprintf(CRITICAL, " mmc get capacity ...\n");
    capacity = hal_mmc_get_capacity(handle);
    dprintf(CRITICAL, " mmc get capacity = %llu result: %s\n", capacity,
            capacity ? "PASS" : "FAILED");
    if (!capacity) {
        ret = 1;
        goto async_release_handle;
    }

    dprintf(CRITICAL, " mmc switch partiion to boot2 ...\n");
    ret = hal_mmc_switch_part(handle, PART_ACCESS_BOOT2);
    dprintf(CRITICAL, " mmc switch partiion to boot2 result: %s\n",
            ret ? "FAILED" : "PASS");
    if (ret) {
        ret = 1;
        goto async_release_handle;
    }

    dprintf(CRITICAL, "\n mmc test transfer ...\n");
    ret = mmc_test_tansfer(handle, 100, UINT32_MAX, 10, 1, 0);
    dprintf(CRITICAL, " mmc test transfer result: %s\n",
            ret ? "FAILED" : "PASS");

    /* Only async mode support cancel api */
    dprintf(CRITICAL, "\n mmc test transfer cancel\n");
    ret += mmc_test_tansfer(handle, 100, UINT32_MAX, 100, 1, 1);
    dprintf(CRITICAL, " mmc test mult block result: %s\n",
            ret ? "FAILED" : "PASS");

    if (ret) {
        ret = 1;
        goto async_release_handle;
    }

async_release_handle:
    dprintf(CRITICAL, " mmc test release handle ...\n");
    ret += hal_mmc_release_handle(&handle);
    dprintf(CRITICAL, " mmc test release handle result: %s\n",
            ret ? "PASS" : "FAILED");
    if (ret != 1)
        return false;

    dprintf(CRITICAL, "\nmmc async test end!\n");
    return true;
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
STATIC_COMMAND_START
STATIC_COMMAND("mmc_sync", "mmc sync call sample",
               (console_cmd)&mmc_sync_mode_sample)
STATIC_COMMAND("mmc_async", "mmc sync call sample",
               (console_cmd)&mmc_async_mode_sample)
STATIC_COMMAND_END(mmc_sample);
#endif

APP_START(mmc_sample).flags = 0 APP_END
