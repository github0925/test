/*
 * cmd_spi_nor.c
 *
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: spi nor test.
 */
#include <stdlib.h>
#include <string.h>
#include <lib/console.h>
#include <app.h>
#include <debug.h>
#include <kernel/event.h>
#include <sdunittest.h>

#include <chip_res.h>
#include <spi_nor_hal.h>

#define ARRAY_NUMS(array) (sizeof(array) / sizeof(array[0]))

#define LOCAL_DEBUG_FLAG

static event_t opt_complete_event;

struct spi_nor_test_cfg {
    uint32_t id;
    struct spi_nor_cfg config;
};

static struct spi_nor_test_cfg spi_nor_cfg_data[] = {
    { .id = RES_OSPI_REG_OSPI2,
      .config =
          {
              .cs = SPI_NOR_CS0,
              .bus_clk = SPI_NOR_CLK_100MHZ,
              .octal_ddr_en = false,
          } },
};

static int spi_nor_test_tansfer(struct spi_nor_handle *dev, u64 addr, u32 data,
                                u64 len, u32 async_mode)
{
    u32 ret = 0;
    u32 erase_block_size;

    erase_block_size = dev->block_size;

    uint8_t *mem1 = memalign(CACHE_LINE, ROUNDUP(len + 32, CACHE_LINE));
    uint8_t *mem2 = memalign(CACHE_LINE, ROUNDUP(len + 32, CACHE_LINE));

    memset(mem1, 0, len + 32);
    memset(mem2, 0, len + 32);

    uint32_t i;
    volatile uint32_t *p1 = (volatile uint32_t *)mem1;
    volatile uint8_t *p2 = (volatile uint8_t *)mem2;
    volatile uint8_t *p3 = (volatile uint8_t *)mem1;

    for (i = 0; i < len / 4; i++, p1++) {
        if (data == UINT32_MAX) {
            *p1 = rand();
        }
        else {
            *p1 = data;
        }
    }

#ifdef LOCAL_DEBUG_FLAG
    /* for debug, print the first 32 bytes data */
    printf("\nspi_nor test original data:\n");
    hexdump8(mem1, MIN(len, 32));
#endif

    /* test spi_nor read write */
    hal_spi_nor_erase(dev, ROUNDDOWN(addr, erase_block_size),
                      ROUNDUP(len, erase_block_size) + erase_block_size * 0x10);

    hal_spi_nor_write(dev, addr, mem1, len);
    if (async_mode)
        event_wait(&opt_complete_event);

    hal_spi_nor_read(dev, addr, mem2, len + 32);
    if (async_mode)
        event_wait(&opt_complete_event);

#ifdef LOCAL_DEBUG_FLAG
    /* for debug, print the first 32 bytes data */
    hexdump8(mem2, MIN(len, 32));
#endif
    /* check data error */
    p2 = mem2;
    for (i = 0; i < len; i++, p2++, p3++) {
        if (*p2 != *p3) {
            dprintf(CRITICAL, "\n spi_nor data error: 0x%x - 0x%x : i=%d\n",
                    *p3, *p2, i);
            ret += 1;
            break;
        }
    }

    /* chack data exceed */
    p2 = mem2 + len;
    for (i = 0; i < 32; i++, p2++) {
        if (*p2 != 0xff) {
            dprintf(CRITICAL, "\n spi_nor exceed error: 0x%x : i=%d\n", *p2, i);
            ret += 1;
            break;
        }
    }

    /* test spi_nor erase */
    hal_spi_nor_erase(dev, ROUNDDOWN(addr, erase_block_size),
                      ROUNDUP(len, erase_block_size) + erase_block_size * 0x10);

    hal_spi_nor_read(dev, addr, mem2, len);
    if (async_mode)
        event_wait(&opt_complete_event);

    /* check data error */
    p2 = mem2;

#ifdef LOCAL_DEBUG_FLAG
    /* for debug, print the first 32 bytes data */
    hexdump8(mem2, 32);
#endif
    for (i = 0; i < len; i++, p2++) {
        if (*p2 != 0xff) {
            dprintf(CRITICAL, "\n spi_nor erase error: 0x%x : i=%d\n", *p2, i);
            ret += 1;
            break;
        }
    }

    if (async_mode) {
        /* test spi_nor cancel */
        hal_spi_nor_write(dev, addr, mem1, len);
        hal_spi_nor_cancel(dev);
        if (async_mode)
            event_wait(&opt_complete_event);

        hal_spi_nor_read(dev, addr, mem2, len);
        if (async_mode)
            event_wait(&opt_complete_event);

#ifdef LOCAL_DEBUG_FLAG
        /* for debug, print the first 32 bytes data */
        hexdump8(mem2, MIN(len, 32));
#endif
        /* check data error */
        p2 = mem2;
        p3 = mem1;
        for (i = 0; i < len; i++, p2++, p3++) {
            if (*p2 != *p3) {
                dprintf(INFO, "\n spi_nor cancel sucess: 0x%x - 0x%x : i=%d\n",
                        *p3, *p2, i);
                break;
            }
        }

        if (i == len) {
            ret += 1;
            dprintf(CRITICAL, "\n spi_nor cancel error: 0x%x - 0x%x : i=%d\n",
                    *p3, *p2, i);
        }
    }

    dprintf(CRITICAL, "\n");

    free(mem1);
    free(mem2);
    return ret;
}

static bool spi_nor_test_sync_mode(void)
{
    dprintf(CRITICAL, "spi_nor sync test start!\n");
    void *handle;
    int ret;
    spi_nor_length_t capacity;

    for (int i = 0; i < ARRAY_NUMS(spi_nor_cfg_data); i++) {
        dprintf(CRITICAL, "\n spi_nor sync test cases index: %d\n", i);

        dprintf(CRITICAL, " spi_nor test creat handle ...\n");
        ret = hal_spi_nor_creat_handle(&handle, spi_nor_cfg_data[i].id);
        dprintf(CRITICAL, " spi_nor test creat handle result: %s\n",
                ret ? "PASS" : "FAILED");
        if (!ret)
            return false;

        ((struct spi_nor_handle *)handle)->config = &spi_nor_cfg_data[i].config;
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

        dprintf(CRITICAL, "\n spi_nor test transfer ...\n");
        ret += spi_nor_test_tansfer(handle, 100, UINT32_MAX, 0x10000, 0);
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
    }

    dprintf(CRITICAL, "\nspi_nor sync test end!\n");
    return true;
}
static void test_event_handle(enum spi_nor_opt type,
                              enum spi_nor_opt_result result)
{
    dprintf(CRITICAL, " spi_nor opt event: type = %d, result = %d\n", type,
            result);
    event_signal(&opt_complete_event, false);
}

static bool spi_nor_test_async_mode(void)
{
    dprintf(CRITICAL, "spi_nor async test start!\n");
    void *handle;
    struct spi_nor_handle *spi_nor_handle;
    int ret;
    spi_nor_length_t capacity;

    event_init(&opt_complete_event, false, EVENT_FLAG_AUTOUNSIGNAL);

    for (int i = 0; i < ARRAY_NUMS(spi_nor_cfg_data); i++) {
        dprintf(CRITICAL, "\n spi_nor async test cases index: %d\n", i);

        dprintf(CRITICAL, " spi_nor test creat handle ...\n");
        ret = hal_spi_nor_creat_handle(&handle, spi_nor_cfg_data[i].id);
        dprintf(CRITICAL, " spi_nor test creat handle result: %s\n",
                ret ? "PASS" : "FAILED");
        if (!ret) {
            ret = 1;
            return false;
        }

        spi_nor_handle = handle;
        spi_nor_handle->config = &spi_nor_cfg_data[i].config;
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
        ret += spi_nor_test_tansfer(handle, 100, UINT32_MAX, 0x10000, 1);
        dprintf(CRITICAL, " spi_nor test transfer result: %s\n",
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
    }

    dprintf(CRITICAL, "\nspi_nor async test end!\n");
    return true;
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
STATIC_COMMAND_START
STATIC_COMMAND("spi_nor_test_sync", "spi_nor sync call test case",
               (console_cmd)&spi_nor_test_sync_mode)
STATIC_COMMAND("spi_nor_test_async", "spi_nor async call test case",
               (console_cmd)&spi_nor_test_async_mode)
STATIC_COMMAND_END(spi_nor_test);
#endif

DEFINE_REGISTER_TEST_COMMAND(spi_nortest1, spi_nortest, spi_nor_test_async)
DEFINE_REGISTER_TEST_COMMAND(spi_nortest2, spi_nortest, spi_nor_test_sync)

APP_START(spi_nor_test).flags = 0 APP_END
