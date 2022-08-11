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
#include <platform.h>

#include <chip_res.h>
#include <spi_nor_hal.h>

#define ARRAY_NUMS(array) (sizeof(array) / sizeof(array[0]))

#define LOCAL_DEBUG_FLAG

#define SPINOR_OPT_READ 1
#define SPINOR_OPT_WRITE 2

//#define USE_HEAP_MEMORY_TEST

static event_t opt_complete_event;

struct spi_nor_test_cfg {
    uint32_t id;
    struct spi_nor_cfg config;
};

static struct spi_nor_test_cfg spi_nor_cfg_data[] = {
    { .id = RES_OSPI_REG_OSPI1,
      .config =
          {
              .cs = SPI_NOR_CS0,
              .bus_clk = SPI_NOR_CLK_50MHZ,
              .octal_ddr_en = 0,
          } },

    { .id = RES_OSPI_REG_OSPI1,
      .config =
          {
              .cs = SPI_NOR_CS0,
              .bus_clk = SPI_NOR_CLK_50MHZ,
              .octal_ddr_en = 1,
          } },
};

static int spi_nor_test_tansfer(struct spi_nor_handle *dev, u64 addr, u32 data,
                                u64 len, u32 async_mode)
{
    u32 ret = 0;
    u32 erase_block_size;

    erase_block_size = dev->block_size;
#ifdef USE_HEAP_MEMORY_TEST
    uint8_t *mem1 = memalign(CACHE_LINE, ROUNDUP(len + 32, CACHE_LINE));
    uint8_t *mem2 = memalign(CACHE_LINE, ROUNDUP(len + 32, CACHE_LINE));
    memset(mem1, 0, len + 32);
    memset(mem2, 0, len + 32);
#else
    uint8_t *mem1 = (uint8_t *)0x30100000;
    uint8_t *mem2 = (uint8_t *)0x31800000;
#endif

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
                      ROUNDUP(len + 32, erase_block_size) + erase_block_size);

    hal_spi_nor_read(dev, addr, mem2, len);
    if (async_mode)
        event_wait(&opt_complete_event);

    /* check data error */
    p2 = mem2;

#ifdef LOCAL_DEBUG_FLAG
    /* for debug, print the first 32 bytes data */
    hexdump8(mem2, 32);
#endif

    hal_spi_nor_write(dev, addr, mem1, len);
    if (async_mode)
        event_wait(&opt_complete_event);

    hal_spi_nor_read(dev, addr, mem2, len+32);
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
            printf("write data:\n");
            hexdump8((const void *)p3, 32);
            printf("read data:\n");
            hexdump8((const void *)p2, 32);
            uint32_t aaa = 0;
            hal_spi_nor_read(dev, ROUNDDOWN(addr + i, 4), mem2, 32);
            printf("read again:\n");
            hexdump8(mem2, 32);
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
                      ROUNDUP(len + 32, erase_block_size) + erase_block_size);

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
        thread_sleep(1);
        hal_spi_nor_cancel(dev);
        if (async_mode)
            event_wait(&opt_complete_event);

        hal_spi_nor_read(dev, addr, mem2, len);
        thread_sleep(1);
        hal_spi_nor_cancel(dev);
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
                dprintf(CRITICAL, "\n spi_nor cancel sucess: 0x%x - 0x%x : i=%d\n",
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
#ifdef USE_HEAP_MEMORY_TEST
    free(mem1);
    free(mem2);
#endif
    return ret;
}

static int spi_nor_test_performance(struct spi_nor_handle *dev, u64 addr,
                                    u32 data, u32 len, u32 count)
{
    u32 ret = 0;
    u32 erase_block_size;
    lk_time_t time;

    erase_block_size = dev->block_size;
#ifdef USE_HEAP_MEMORY_TEST
    uint8_t *mem1 = memalign(CACHE_LINE, ROUNDUP(len, CACHE_LINE));
    uint8_t *mem2 = memalign(CACHE_LINE, ROUNDUP(len, CACHE_LINE));
    memset(mem1, 0, len);
    memset(mem2, 0, len);
#else
    uint8_t *mem1 = (uint8_t *)0x30100000;
    uint8_t *mem2 = (uint8_t *)0x31800000;
#endif

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
                      ROUNDUP(len + 32, erase_block_size) + erase_block_size);

    hal_spi_nor_read(dev, addr, mem2, len);

    /* check data error */
    p2 = mem2;

#ifdef LOCAL_DEBUG_FLAG
    /* for debug, print the first 32 bytes data */
    hexdump8(mem2, 32);
#endif


    hal_spi_nor_write(dev, addr, mem1, len);

    time = current_time();
    for (int i = 0; i < count; i++) {
        hal_spi_nor_read(dev, addr, mem2, len);
    }
    time = current_time() - time;

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
            printf("write data:\n");
            hexdump8((const void *)p3, 32);
            printf("read data:\n");
            hexdump8((const void *)p2, 32);
            uint32_t aaa = 0;
            hal_spi_nor_read(dev, ROUNDDOWN(addr + i, 4), mem2, 32);
            printf("read again:\n");
            hexdump8(mem2, 32);
            ret += 1;
            break;
        }
    }

    dprintf(CRITICAL, "\n spi_nor read data: len(0x%x), times(%d), use time(%d ms), speed = %d MB/s\n",
                    len, count, time, len * count / 1000 / time);

    dprintf(CRITICAL, "\n");
#ifdef USE_HEAP_MEMORY_TEST
    free(mem1);
    free(mem2);
#endif
    return ret;
}

static int spi_nor_test_rw(struct spi_nor_handle *dev, u64 addr,
                                    u32 len, u32 data, u32 flag)
{
    u32 ret = 0;

#ifdef USE_HEAP_MEMORY_TEST
    uint8_t *mem1 = memalign(CACHE_LINE, ROUNDUP(len, CACHE_LINE));
    memset(mem1, 0, len);
#else
    uint8_t *mem1 = (uint8_t *)0x30100000;
#endif

    uint32_t i;
    volatile uint32_t *p1 = (volatile uint32_t *)mem1;

    if (flag == SPINOR_OPT_WRITE) {
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
        ret = hal_spi_nor_write(dev, addr, mem1, len);
    }
    else if (flag == SPINOR_OPT_READ) {
        ret = hal_spi_nor_read(dev, addr, mem1, len);
#ifdef LOCAL_DEBUG_FLAG
        /* for debug, print the first 32 bytes data */
        hexdump8(mem1, 32);
#endif
    }

    dprintf(CRITICAL, "\n");
#ifdef USE_HEAP_MEMORY_TEST
    free(mem1);
#endif
    return ret;
}

static bool spi_nor_erase_cmd(int argc, const cmd_args *argv)
{
    void *handle;
    int ret;
    spi_nor_length_t capacity;
    struct spi_nor_handle *spi_nor_handle;
    u64 addr = 0;
    u32 len = 0x10000;
    u32 erase_block_size;

    switch (argc) {
        case 3:
            len = atoui(argv[2].str);
        case 2:
            addr= atoul(argv[1].str);
        default:
            break;
    }

    dprintf(CRITICAL, "spi_nor erase test start!\n");


    dprintf(CRITICAL, " spi_nor test creat handle ...\n");
    ret = hal_spi_nor_creat_handle(&handle, spi_nor_cfg_data[0].id);
    dprintf(CRITICAL, " spi_nor test creat handle result: %s\n",
            ret ? "PASS" : "FAILED");
    if (!ret)
        return false;

    spi_nor_handle = handle;
    spi_nor_handle->config = &spi_nor_cfg_data[0].config;

    dprintf(CRITICAL, " spi_nor test init spi_nor device ...\n");
    ret = hal_spi_nor_init(handle);
    dprintf(CRITICAL, " spi_nor test init spi_nor device result: %s\n",
            ret ? "FAILED" : "PASS");
    if (ret) {
        ret = 1;
        goto erase_release_handle;
    }
    dprintf(CRITICAL, " spi_nor get capacity ...\n");
    capacity = hal_spi_nor_get_capacity(handle);
    dprintf(CRITICAL, " spi_nor get capacity = %llu result: %s\n", capacity,
            capacity ? "PASS" : "FAILED");
    if (!capacity) {
        ret = 1;
        goto erase_release_handle;
    }

    dprintf(CRITICAL, "\n spi_nor test erase: addr = 0x%llx, len = 0x%x\n", addr, len);
    ret += hal_spi_nor_erase(spi_nor_handle, addr, len);
    dprintf(CRITICAL, " spi_nor test erase result: %s\n",
            ret ? "FAILED" : "PASS");

erase_release_handle:
    if (!hal_spi_nor_release_handle(&handle)) {
        dprintf(CRITICAL, " spi_nor test release handle failed!");
        return false;
    }
    if (ret)
        return false;
    dprintf(CRITICAL, "\nspi_nor erase test pass!\n");
    return true;
}

static bool spi_nor_write_cmd(int argc, const cmd_args *argv)
{
    void *handle;
    int ret;
    spi_nor_length_t capacity;
    struct spi_nor_handle *spi_nor_handle;
    u64 addr = 0;
    u32 len = 0x10000;
    u32 data = UINT32_MAX;

    switch (argc) {
        case 4:
            data = atoui(argv[3].str);
        case 3:
            len = atoui(argv[2].str);
        case 2:
            addr= atoul(argv[1].str);
        default:
            break;
    }

    dprintf(CRITICAL, "spi_nor write test start!\n");

    dprintf(CRITICAL, " spi_nor test creat handle ...\n");
    ret = hal_spi_nor_creat_handle(&handle, spi_nor_cfg_data[0].id);
    dprintf(CRITICAL, " spi_nor test creat handle result: %s\n",
            ret ? "PASS" : "FAILED");
    if (!ret)
        return false;

    spi_nor_handle = handle;
    spi_nor_handle->config = &spi_nor_cfg_data[0].config;

    dprintf(CRITICAL, " spi_nor test init spi_nor device ...\n");
    ret = hal_spi_nor_init(handle);
    dprintf(CRITICAL, " spi_nor test init spi_nor device result: %s\n",
            ret ? "FAILED" : "PASS");
    if (ret) {
        ret = 1;
        goto write_release_handle;
    }
    dprintf(CRITICAL, " spi_nor get capacity ...\n");
    capacity = hal_spi_nor_get_capacity(handle);
    dprintf(CRITICAL, " spi_nor get capacity = %llu result: %s\n", capacity,
            capacity ? "PASS" : "FAILED");
    if (!capacity) {
        ret = 1;
        goto write_release_handle;
    }

    dprintf(CRITICAL, "\n spi_nor test write: addr = 0x%llx, len = 0x%x\n", addr, len);
    ret += spi_nor_test_rw( spi_nor_handle, addr, len, data, SPINOR_OPT_WRITE);
    dprintf(CRITICAL, " spi_nor test write result: %s\n",
            ret ? "FAILED" : "PASS");

write_release_handle:
    if (!hal_spi_nor_release_handle(&handle)) {
        dprintf(CRITICAL, " spi_nor test release handle failed!");
        return false;
    }
    if (ret)
        return false;


    dprintf(CRITICAL, "\nspi_nor write test pass!\n");
    return true;
}


static bool spi_nor_read_cmd(int argc, const cmd_args *argv)
{
    void *handle;
    int ret;
    spi_nor_length_t capacity;
    struct spi_nor_handle *spi_nor_handle;
    u64 addr = 0;
    u32 len = 0x10000;
    u32 data = UINT32_MAX;

    switch (argc) {
        case 3:
            len = atoui(argv[2].str);
        case 2:
            addr= atoul(argv[1].str);
        default:
            break;
    }

    dprintf(CRITICAL, "spi_nor read test start!\n");

    dprintf(CRITICAL, " spi_nor test creat handle ...\n");
    ret = hal_spi_nor_creat_handle(&handle, spi_nor_cfg_data[0].id);
    dprintf(CRITICAL, " spi_nor test creat handle result: %s\n",
            ret ? "PASS" : "FAILED");
    if (!ret)
        return false;

    spi_nor_handle = handle;
    spi_nor_handle->config = &spi_nor_cfg_data[0].config;

    dprintf(CRITICAL, " spi_nor test init spi_nor device ...\n");
    ret = hal_spi_nor_init(handle);
    dprintf(CRITICAL, " spi_nor test init spi_nor device result: %s\n",
            ret ? "FAILED" : "PASS");
    if (ret) {
        ret = 1;
        goto read_release_handle;
    }
    dprintf(CRITICAL, " spi_nor get capacity ...\n");
    capacity = hal_spi_nor_get_capacity(handle);
    dprintf(CRITICAL, " spi_nor get capacity = %llu result: %s\n", capacity,
            capacity ? "PASS" : "FAILED");
    if (!capacity) {
        ret = 1;
        goto read_release_handle;
    }

    dprintf(CRITICAL, "\n spi_nor test read: addr = 0x%llx, len = 0x%x\n", addr, len);
    ret += spi_nor_test_rw( spi_nor_handle, addr, len, data, SPINOR_OPT_READ);
    dprintf(CRITICAL, " spi_nor test read result: %s\n",
            ret ? "FAILED" : "PASS");

read_release_handle:
    if (!hal_spi_nor_release_handle(&handle)) {
        dprintf(CRITICAL, " spi_nor test release handle failed!");
        return false;
    }
    if (ret)
        return false;

    dprintf(CRITICAL, "\nspi_nor read test pass!\n");
    return true;
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
        ret += spi_nor_test_tansfer(handle, 0x1000, UINT32_MAX, 0x1000, 0);
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
        ret += spi_nor_test_tansfer(handle, 0x1000000, UINT32_MAX, 0x80000, 1);
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

static bool spi_nor_stress_cmd(int argc, const cmd_args *argv)
{
    void *handle;
    int ret;
    spi_nor_length_t capacity;
    struct spi_nor_handle *spi_nor_handle;
    u64 times = 1;
    u64 addr = 0x1000;
    u32 data = UINT32_MAX;
    u64 len = 0x400;
    u32 async_mode = 0;

    switch (argc) {
        case 5:
            data = atoui(argv[4].str);
        case 4:
            times = atoul(argv[3].str);
        case 3:
            async_mode = atoui(argv[2].str);
        case 2:
            addr= atoul(argv[1].str);
        default:
            break;
    }

    dprintf(CRITICAL, "spi_nor stress test start!\n");

    event_init(&opt_complete_event, false, EVENT_FLAG_AUTOUNSIGNAL);

    while (times) {
        len = rand() & 0xfffff;
        dprintf(CRITICAL, "\n spi_nor stress test remaining times:%lld len:%lld\n", times, len);
        times--;

        for (int i = 0; i < ARRAY_NUMS(spi_nor_cfg_data); i++) {
            dprintf(CRITICAL, " spi_nor sync test cases index: %d\n", i);

            dprintf(CRITICAL, " spi_nor test creat handle ...\n");
            ret = hal_spi_nor_creat_handle(&handle, spi_nor_cfg_data[i].id);
            dprintf(CRITICAL, " spi_nor test creat handle result: %s\n",
                    ret ? "PASS" : "FAILED");
            if (!ret)
                return false;

            spi_nor_handle = handle;
            spi_nor_handle->config = &spi_nor_cfg_data[i].config;
            if (async_mode) {
                spi_nor_handle->async_mode = 1;
                spi_nor_handle->event_handle = test_event_handle;
                dprintf(CRITICAL, " spi_nor test async mode.\n");
            }

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
            ret += spi_nor_test_tansfer(handle, addr, data, len * 4, async_mode);
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
    }

    dprintf(CRITICAL, "\nspi_nor stress test pass!\n");
    return true;
}

static bool spi_nor_performance_cmd(int argc, const cmd_args *argv)
{
    void *handle;
    int ret;
    spi_nor_length_t capacity;
    struct spi_nor_handle *spi_nor_handle;
    u64 times = 10;
    u64 addr = 0x1000000;
    u32 data = UINT32_MAX;
    u32 len = 0x1000000;

    switch (argc) {
        case 5:
            data = atoui(argv[4].str);
        case 4:
            times = atoul(argv[3].str);
        case 3:
            len = atoui(argv[2].str);
        case 2:
            addr= atoul(argv[1].str);
        default:
            break;
    }

    dprintf(CRITICAL, "spi_nor performance test start!\n");

    for (int i = 0; i < ARRAY_NUMS(spi_nor_cfg_data); i++) {
        dprintf(CRITICAL, " spi_nor sync test cases index: %d\n", i);

        dprintf(CRITICAL, " spi_nor test creat handle ...\n");
        ret = hal_spi_nor_creat_handle(&handle, spi_nor_cfg_data[i].id);
        dprintf(CRITICAL, " spi_nor test creat handle result: %s\n",
                ret ? "PASS" : "FAILED");
        if (!ret)
            return false;

        spi_nor_handle = handle;
        spi_nor_handle->config = &spi_nor_cfg_data[i].config;

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
        ret += spi_nor_test_performance(handle, addr, data, len, times);
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

    dprintf(CRITICAL, "\nspi_nor performances test pass!\n");
    return true;
}


#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
STATIC_COMMAND_START
STATIC_COMMAND("spi_nor_test_erase", "spi_nor erase test case: Usage: spi_nor_test_erase [ADDR] [SIZE]\n",
                (console_cmd)&spi_nor_erase_cmd)
STATIC_COMMAND("spi_nor_test_write", "spi_nor write test case: Usage: spi_nor_test_write [ADDR] [SIZE]\n",
                (console_cmd)&spi_nor_write_cmd)
STATIC_COMMAND("spi_nor_test_read", "spi_nor read test case: Usage: spi_nor_test_read [ADDR] [SIZE]\n",
                (console_cmd)&spi_nor_read_cmd)
STATIC_COMMAND("spi_nor_test_sync", "spi_nor sync call test case",
               (console_cmd)&spi_nor_test_sync_mode)
STATIC_COMMAND("spi_nor_test_async", "spi_nor async call test case",
               (console_cmd)&spi_nor_test_async_mode)
STATIC_COMMAND("spi_nor_test_stress", "spi_nor stress test case\n Usage: spi_nor_test_stress [ADDR] [ASYNC] [TIMES]\n"
                "ADDR is flash address; ASYNC is a bool value which mean if use async mode; TIMES is test times\n",
               (console_cmd)&spi_nor_stress_cmd)
STATIC_COMMAND("spi_nor_test_performance", "spi_nor performance test case\n Usage: spi_nor_test_performance [ADDR] [SIZE] [TIMES]\n"
                "ADDR is flash address; SIZE is a int value which mean one time read data size; TIMES is test times\n",
               (console_cmd)&spi_nor_performance_cmd)
STATIC_COMMAND_END(spi_nor_test);
#endif

//DEFINE_REGISTER_TEST_COMMAND(spi_nortest1, spi_nortest, spi_nor_test_async)
DEFINE_REGISTER_TEST_COMMAND(spi_nortest2, spi_nortest, spi_nor_test_sync)
DEFINE_REGISTER_TEST_COMMAND(spi_nortest3, spi_nortest, spi_nor_test_stress)
DEFINE_REGISTER_TEST_COMMAND(spi_nortest4, spi_nortest, spi_nor_test_performance)

APP_START(spi_nor_test)
.flags = 0,
//.stack_size = 0x4000,
APP_END
