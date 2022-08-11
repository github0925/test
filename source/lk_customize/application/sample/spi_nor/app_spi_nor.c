/*
 * app_spi_nor.c
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: spi nor sample.
 */
#include <stdlib.h>
#include <string.h>
#include <lib/console.h>
#include <app.h>
#include <debug.h>
#include <kernel/event.h>

#include <chip_res.h>
#include <spi_nor_hal.h>

static event_t opt_complete_event;

struct spi_nor_test_cfg {
    uint32_t id;
    struct spi_nor_cfg config;
};

static struct spi_nor_test_cfg spi_nor_cfg_data = {
    .id = RES_OSPI_REG_OSPI2,
    .config =
        {
            .cs = SPI_NOR_CS0,
            .bus_clk = SPI_NOR_CLK_100MHZ,
            .octal_ddr_en = false,
        },
};

static int spi_nor_test_tansfer(struct spi_nor_handle *dev, u64 addr, u32 data,
                                u64 len, u32 async_mode, u32 test_cancel)
{
    u32 ret = 0;
    u32 erase_block_size;

    erase_block_size = dev->block_size;

    uint8_t *mem1 = memalign(CACHE_LINE, ROUNDUP(len, CACHE_LINE));
    uint8_t *mem2 = memalign(CACHE_LINE, ROUNDUP(len, CACHE_LINE));

    memset(mem1, 0, len);
    memset(mem2, 0, len);

    uint32_t i;
    volatile uint32_t *p1 = (volatile uint32_t *)mem1;
    for (i = 0; i < len / 4; i++, p1++) {
        if (data == UINT32_MAX) {
            *p1 = rand();
        }
        else {
            *p1 = data;
        }
    }

    /* for debug, print the first 32 bytes data */
    printf("\nspi_nor test original data:\n");
    hexdump8(mem1, MIN(len, 32));

    /* test spi_nor read write */
    ret = hal_spi_nor_erase(dev, ROUNDDOWN(addr, erase_block_size),
                            ROUNDUP(len, erase_block_size) +
                                erase_block_size * 0x10);
    if (ret) {
        printf("%s: erase error, ret = %d\n", __FUNCTION__, ret);
        goto tansfer_out;
    }

    ret = hal_spi_nor_write(dev, addr, mem1, len);
    if (ret) {
        printf("%s: write error, ret = %d\n", __FUNCTION__, ret);
        goto tansfer_out;
    }
    if (async_mode) {
        if (test_cancel) {
            ret = hal_spi_nor_cancel(dev);
            printf(
                "%s: cancel the the previous read/write operation, ret = %d\n",
                __FUNCTION__, ret);
        }
        event_wait(&opt_complete_event);
        printf("%s: write complete\n", __FUNCTION__);
    }

    hal_spi_nor_read(dev, addr, mem2, len);
    if (ret) {
        printf("%s: write error, ret = %d\n", __FUNCTION__, ret);
        goto tansfer_out;
    }
    if (async_mode) {
        event_wait(&opt_complete_event);
        printf("%s: read complete\n", __FUNCTION__);
    }

    /* for debug, print the first 32 bytes data */
    hexdump8(mem2, MIN(len, 32));

tansfer_out:
    dprintf(CRITICAL, "\n");
    free(mem1);
    free(mem2);
    return ret;
}

static bool spi_nor_sync_mode_sample(void)
{
    dprintf(CRITICAL, "spi_nor sync sample start!\n");
    void *handle;
    int ret;
    spi_nor_length_t capacity;

    dprintf(CRITICAL, " spi_nor test creat handle ...\n");
    ret = hal_spi_nor_creat_handle(&handle, spi_nor_cfg_data.id);
    dprintf(CRITICAL, " spi_nor test creat handle result: %s\n",
            ret ? "PASS" : "FAILED");
    if (!ret)
        return false;

    ((struct spi_nor_handle *)handle)->config = &spi_nor_cfg_data.config;
    dprintf(CRITICAL, " spi_nor test init spi_nor device ...\n");
    ret = hal_spi_nor_init(handle);
    dprintf(CRITICAL, " spi_nor test init spi_nor device result: %s\n",
            ret ? "FAILED" : "PASS");
    if (ret) {
        ret = 1;
        goto sync_release_handle;
    }
    dprintf(CRITICAL, " spi_nor get capacity ...\n");
    capacity = hal_spi_nor_get_capacity(handle);
    dprintf(CRITICAL, " spi_nor get capacity = %llu result: %s\n", capacity,
            capacity ? "PASS" : "FAILED");
    if (!capacity) {
        ret = 1;
        goto sync_release_handle;
    }

    /* Only async mode support cancel api */
    dprintf(CRITICAL, "\n spi_nor test transfer ...\n");
    ret += spi_nor_test_tansfer(handle, 100, UINT32_MAX, 0x10000, 0, 0);
    dprintf(CRITICAL, " spi_nor test transfer result: %s\n",
            ret ? "FAILED" : "PASS");

    if (ret) {
        ret = 1;
        goto sync_release_handle;
    }
sync_release_handle:
    dprintf(CRITICAL, " spi_nor test release handle ...\n");
    ret += hal_spi_nor_release_handle(&handle);
    dprintf(CRITICAL, " spi_nor test release handle result: %s\n",
            ret ? "PASS" : "FAILED");

    if (ret != 1)
        return false;

    dprintf(CRITICAL, "\nspi_nor sync sample end!\n");
    return true;
}

static void test_event_handle(enum spi_nor_opt type,
                              enum spi_nor_opt_result result)
{
    dprintf(CRITICAL, " spi_nor opt event: type = %d, result = %d\n", type,
            result);
    event_signal(&opt_complete_event, false);
}

static bool spi_nor_async_mode_sample(void)
{
    dprintf(CRITICAL, "spi_nor async sample start!\n");
    void *handle;
    struct spi_nor_handle *spi_nor_handle;
    int ret;
    spi_nor_length_t capacity;

    event_init(&opt_complete_event, false, EVENT_FLAG_AUTOUNSIGNAL);

    dprintf(CRITICAL, " spi_nor test creat handle ...\n");
    ret = hal_spi_nor_creat_handle(&handle, spi_nor_cfg_data.id);
    dprintf(CRITICAL, " spi_nor test creat handle result: %s\n",
            ret ? "PASS" : "FAILED");
    if (!ret) {
        ret = 1;
        return false;
    }

    spi_nor_handle = handle;
    spi_nor_handle->config = &spi_nor_cfg_data.config;
    spi_nor_handle->async_mode = 1;
    spi_nor_handle->event_handle = test_event_handle;

    dprintf(CRITICAL, " spi_nor test init spi_nor device ...\n");
    ret = hal_spi_nor_init(handle);
    dprintf(CRITICAL, " spi_nor test init spi_nor device result: %s\n",
            ret ? "FAILED" : "PASS");
    if (ret) {
        ret = 1;
        goto async_release_handle;
    };

    dprintf(CRITICAL, " spi_nor get capacity ...\n");
    capacity = hal_spi_nor_get_capacity(handle);
    dprintf(CRITICAL, " spi_nor get capacity = %llu result: %s\n", capacity,
            capacity ? "PASS" : "FAILED");
    if (!capacity) {
        ret = 1;
        goto async_release_handle;
    }

    dprintf(CRITICAL, "\n spi_nor test transfer ...\n");
    ret += spi_nor_test_tansfer(handle, 100, UINT32_MAX, 0x10000, 1, 0);
    dprintf(CRITICAL, " spi_nor test tansfer: result: %s\n",
            ret ? "FAILED" : "PASS");

    /* Only async mode support cancel api */
    dprintf(CRITICAL, "\n spi_nor test transfer cancel\n");
    ret += spi_nor_test_tansfer(handle, 100, UINT32_MAX, 0x10000, 1, 1);
    dprintf(CRITICAL, " spi_nor test transfer cancel result: %s\n",
            ret ? "FAILED" : "PASS");

    if (ret) {
        ret = 1;
        goto async_release_handle;
    }
async_release_handle:
    dprintf(CRITICAL, " spi_nor test release handle ...\n");
    ret += hal_spi_nor_release_handle(&handle);
    dprintf(CRITICAL, " spi_nor test release handle result: %s\n",
            ret ? "PASS" : "FAILED");
    if (ret != 1)
        return false;

    dprintf(CRITICAL, "\nspi_nor async sample end!\n");
    return true;
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
STATIC_COMMAND_START
STATIC_COMMAND("spi_nor_sync", "spi_nor sync call sample",
               (console_cmd)&spi_nor_sync_mode_sample)
STATIC_COMMAND("spi_nor_async", "spi_nor async call sample",
               (console_cmd)&spi_nor_async_mode_sample)
STATIC_COMMAND_END(spi_nor_sample);
#endif

APP_START(spi_nor_sample).flags = 0 APP_END
