/*
 * cmd_mmc.c
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SD/eMMC test.
 */
#include <stdlib.h>
#include <string.h>
#include <lib/console.h>
#include <app.h>
#include <debug.h>
#include <kernel/event.h>
#include <sdunittest.h>

#include <chip_res.h>
#include <mmc_hal.h>

#define ARRAY_NUMS(array) (sizeof(array) / sizeof(array[0]))

#define LOCAL_DEBUG_FLAG

static event_t opt_complete_event;

struct mmc_test_cfg {
    uint32_t id;
    struct mmc_cfg config;
};

static struct mmc_test_cfg mmc_cfg_data[] = {

    {.id = RES_MSHC_SD1,
     .config =
         {
             .voltage = MMC_VOL_1_8,
             .max_clk_rate = MMC_CLK_100MHZ,
             .bus_width = MMC_BUS_WIDTH_8BIT,
         }},

    {.id = RES_MSHC_SD3,
     .config =
         {
             .voltage = MMC_VOL_3_3,
             .max_clk_rate = MMC_CLK_25MHZ,
             .bus_width = MMC_BUS_WIDTH_4BIT,
         }},
};

static int mmc_test_tansfer(struct mmc_handle *dev, u32 block, u32 data,
                            u32 num, u32 async_mode)
{
    u32 ret = 0;
    u32 block_size;
    u32 erase_grp_size;

    block_size = hal_mmc_get_block_size(dev);
    erase_grp_size = hal_mmc_get_erase_grp_size(dev);

    uint8_t *mem1 =
        memalign(CACHE_LINE, ROUNDUP(block_size * (num + 1), CACHE_LINE));
    uint8_t *mem2 =
        memalign(CACHE_LINE, ROUNDUP(block_size * (num + 1), CACHE_LINE));

    memset(mem1, 0, block_size * (num + 1));
    memset(mem2, 0, block_size * (num + 1));

    uint32_t i;
    volatile uint32_t *p1 = (volatile uint32_t *)mem1;
    volatile uint32_t *p2 = (volatile uint32_t *)mem2;

    for (i = 0; i < 128 * (num + 1); i++, p1++) {
        if (data == UINT32_MAX) {
            *p1 = rand();
        }
        else {
            *p1 = data;
        }
    }

#ifdef LOCAL_DEBUG_FLAG
    /* for debug, print the last block data */
    printf("\nmmc test original data:\n");
    hexdump8(mem1 + (num - 1) * block_size, block_size);
#endif

    /* test mmc read write */
    hal_mmc_erase(dev, block * block_size,
                  ROUNDUP(num * block_size, erase_grp_size));

    hal_mmc_write(dev, block * block_size, mem1, num * block_size);
    if (async_mode)
        event_wait(&opt_complete_event);

    hal_mmc_read(dev, block * block_size, mem2, (num + 1) * block_size);
    if (async_mode)
        event_wait(&opt_complete_event);

#ifdef LOCAL_DEBUG_FLAG
    /* for debug, print the last block data */
    hexdump8(mem2 + (num - 1) * block_size, block_size);
#endif
    /* check data error */
    p1 = (volatile uint32_t *)mem1;
    for (i = 0; i < 128 * num; i++, p1++, p2++) {
        if (*p1 != *p2) {
            dprintf(CRITICAL, "\n mmc error: 0x%x - 0x%x : i=%d\n", *p1, *p2,
                    i);
            ret += 1;
            break;
        }
    }

    /* chack data exceed */
    p2 = (volatile uint32_t *)mem2 + 128 * num;
    for (i = 0; i < 128; i++, p2++) {
        if (*p2 != 0) {
            dprintf(CRITICAL, "\n mmc exceed error: 0x%x : i=%d\n", *p2, i);
            ret += 1;
            break;
        }
    }

    /* test mmc erase */
    p2 = (volatile uint32_t *)mem2;
    hal_mmc_erase(dev, block * block_size,
                  ROUNDUP(num * block_size, erase_grp_size));

    hal_mmc_read(dev, block * block_size, mem2, num * block_size);
    if (async_mode)
        event_wait(&opt_complete_event);

    for (i = 0; i < 128 * num; i++, p2++) {
        if (*p2 != 0) {
            dprintf(CRITICAL, "\n mmc erase error: 0x%x : i=%d\n", *p2, i);
            ret = 1;
            break;
        }
    }

    if (async_mode) {
        /* test mmc cancel */
        hal_mmc_write(dev, block * block_size, mem1, num * block_size);
        hal_mmc_cancel(dev);

        hal_mmc_read(dev, block * block_size, mem2, num * block_size);
        if (async_mode)
            event_wait(&opt_complete_event);

#ifdef LOCAL_DEBUG_FLAG
        /* for debug, print the last block data */
        hexdump8(mem2 + (num - 1) * block_size, block_size);
#endif
        /* check data error */
        p1 = (volatile uint32_t *)mem1;
        p2 = (volatile uint32_t *)mem2;
        for (i = 0; i < 128 * num; i++, p1++, p2++) {
            if (*p1 != *p2) {
                dprintf(INFO, "\n mmc cancel sucess: 0x%x - 0x%x : i=%d\n", *p1,
                        *p2, i);
                break;
            }
        }

        if (i == 128 * num) {
            ret += 1;
            dprintf(CRITICAL, "\n mmc cancel error: 0x%x - 0x%x : i=%d\n", *p1,
                    *p2, i);
        }
    }

    dprintf(INFO, "\n");

    free(mem1);
    free(mem2);
    return ret;
}

static bool mmc_test_sync_mode(void)
{
    dprintf(CRITICAL, "mmc sync test start!\n");
    void *handle;
    int ret = 0;
    mmc_length_t capacity;

    for (int i = 0; i < ARRAY_NUMS(mmc_cfg_data); i++) {
        dprintf(CRITICAL, "\n mmc sync test cases index: %d\n", i);

        dprintf(CRITICAL, " mmc test creat handle ...\n");
        ret = hal_mmc_creat_handle(&handle, mmc_cfg_data[i].id);
        dprintf(CRITICAL, " mmc test creat handle result: %s\n",
                ret ? "PASS" : "FAILED");
        if (!ret)
            return false;

        ((struct mmc_handle *)handle)->config = &mmc_cfg_data[i].config;
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

        dprintf(CRITICAL, "\n mmc test single block ...\n");
        ret = mmc_test_tansfer(handle, 100, UINT32_MAX, 1, 0);
        dprintf(CRITICAL, " mmc test single block result: %s\n",
                ret ? "FAILED" : "PASS");

        dprintf(CRITICAL, "\n mmc test mult block ...\n");
        ret += mmc_test_tansfer(handle, 100, UINT32_MAX, 10, 0);
        dprintf(CRITICAL, " mmc test mult block result: %s\n",
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
    }

    dprintf(CRITICAL, "\nmmc sync test end!\n");
    return true;
}

static void test_event_handle(enum mmc_opt type, enum mmc_opt_result result)
{
    dprintf(CRITICAL, " mmc opt event: type = %d, result = %d\n", type, result);
    event_signal(&opt_complete_event, false);
}

static bool mmc_test_async_mode(void)
{
    dprintf(CRITICAL, "spi_nor async test start!\n");
    void *handle;
    struct mmc_handle *mmc_handle;
    int ret = 0;
    mmc_length_t capacity;

    event_init(&opt_complete_event, false, EVENT_FLAG_AUTOUNSIGNAL);

    for (int i = 0; i < ARRAY_NUMS(mmc_cfg_data); i++) {
        dprintf(CRITICAL, "\n mmc sync test cases index: %d\n", i);

        dprintf(CRITICAL, " mmc test creat handle ...\n");
        ret = hal_mmc_creat_handle(&handle, mmc_cfg_data[i].id);
        dprintf(CRITICAL, " mmc test creat handle result: %s\n",
                ret ? "PASS" : "FAILED");
        if (!ret)
            return false;

        mmc_handle = handle;
        mmc_handle->config = &mmc_cfg_data[i].config;
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

        dprintf(CRITICAL, "\n mmc test mult block ...\n");
        ret = mmc_test_tansfer(handle, 100, UINT32_MAX, 100, 1);
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
    }

    dprintf(CRITICAL, "\nmmc async test end!\n");
    return true;
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
STATIC_COMMAND_START
STATIC_COMMAND("mmc_test_sync", "mmc sync call test case",
               (console_cmd)&mmc_test_sync_mode)
STATIC_COMMAND("mmc_test_async", "mmc async call test case",
               (console_cmd)&mmc_test_async_mode)
STATIC_COMMAND_END(mmc_test);
#endif

DEFINE_REGISTER_TEST_COMMAND(mmctest1, mmctest, mmc_test_sync)
DEFINE_REGISTER_TEST_COMMAND(mmctest2, mmctest, mmc_test_async)

APP_START(mmc_test).flags = 0 APP_END
