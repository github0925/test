/*
 * dma_cases.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * dma cases function head file
 *
 * Revision History:
 * -----------------
 * 0.1, 10/19/2019 yishao init version
 */
#include <app.h>
#include <arch.h>
#include <arch/ops.h>
#include <debug.h>
#include <kernel/debug.h>
#include <kernel/thread.h>
#include <lib/console.h>
#include <lib/reg.h>
#include <platform.h>
#include <reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unittest.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#include "dma_hal.h"
#include "res.h"
#ifdef ENABLE_SD_I2S
#include "app_i2s.h"
#include "i2s_hal.h"
#endif
#define SIZE_BUFFERS 2048 + 16
static uint8_t src_data[ROUNDUP(SIZE_BUFFERS, CACHE_LINE)]
    __attribute__((aligned(CACHE_LINE))) = {0};
static uint8_t dst_data[ROUNDUP(SIZE_BUFFERS, CACHE_LINE)]
    __attribute__((aligned(CACHE_LINE))) = {0};

typedef struct
{
    char name[50];
    u32 param_index;
    u32 flag; // 0: for
} dma_tst_case_t;

dma_tst_case_t dma_tst_cases[] = {
    //{Case name ,  tst type, param index, flag}
    {"[1.0 DMA memcpy drv tst 1]", 0, 0},
    {"[1.1 DMA memcpy drv tst 2]", 1, 0},
    {"[1.2 DMA memcpy drv tst 3]", 2, 0},
    {"[1.3 DMA memcpy drv tst 4]", 3, 0},
    {"[1.4 DMA memcpy drv tst 5]", 4, 0},
    {"[1.5 DMA memcpy drv tst 6]", 5, 0},
    {"[1.6 DMA memcpy drv tst 7]", 6, 0},
    {"[1.7 DMA memcpy 1 blk tst 8]", 7, 0},
    {"[1.7 DMA memcpy 2 blk tst 9]", 8, 0},
    {"[1.7 DMA memcpy performance tst 10]", 9, 0},
    {"[2.1 DMA memset drv tst 1]", 10, 0},
    {"[2.2 DMA memset drv tst 2]", 11, 0},
    {"[2.3 DMA memset drv tst 3]", 12, 0},
    {"[2.4 DMA memset drv tst 4]", 13, 0},
#ifdef ENABLE_SD_I2S
    {"[3.1 DMA hal_prep_dma_dev 8bits drv tst]", 14, 0},
    {"[3.2 DMA hal_prep_dma_dev 16bits drv tst]", 15, 0},
    {"[3.3 DMA hal_prep_dma_dev 32bits drv tst]", 16, 0},
    {"[4.1 DMA hal_prep_dma_cyclic 8bits drv tst]", 17, 0},
    {"[4.2 DMA hal_prep_dma_cyclic 16bits drv tst]", 18, 0},
    {"[4.3 DMA hal_prep_dma_cyclic 32bits drv tst]", 19, 0},
#endif
};

#define TST_CASES_NUM (sizeof(dma_tst_cases) / sizeof(dma_tst_cases[0]))
/*  list all of dma test cases */
void tst_cfg_list(void)
{
    for (int i = 0; i < TST_CASES_NUM; i++)
    {
        printf("Case %-20d: %s \n", i, dma_tst_cases[i].name);
    }
}
/*
 modify data to create different data pattern for testing.
*/
static void modifyData(uint8_t *src, uint8_t *dst, int words)
{
    int i;
    printf("src addr:0x%lx dst addr:0x%lx words:0x%lx\n", src, dst, words);
    /* Put some data in the source buffer for test */
    for (i = 0; i < words; i++)
    {
        src[i] = i % 255 + 1;
        dst[i] = ~src[i]; /* 255 - i % 255; */
    }
}

/** Simple data verification
 * deprecated function
 */
static bool verifyData(uint32_t *src, uint32_t *dst, int words)
{
    int i, errIdx = -1;

    for (i = 0; ((i < words) && (errIdx == -1)); i++)
    {
        if (src[i] != dst[i])
        {
            errIdx = i;
        }
    }

    if (errIdx != -1)
    {
        return false;
    }
    return true;
}

/* dmac irq evt handle */
void tst_irq_evt_handle(enum dma_status status, u32 err, void *context)
{
    printf("dma irq evt: status(%d) err(%d) context(%p) \n", status, err,
           context);
}

/* dma memcpy test cases */
bool drv_tst_dma_memcpy(u32 case_id)
{
    BEGIN_TEST;
    uint8_t *src_ptr = src_data, *dst_ptr = dst_data;
    size_t len = 512;
    switch (case_id)
    {
    case 0:
        src_ptr = src_data;
        dst_ptr = dst_data;
        len = 512;
        break;
    case 1:
        src_ptr = src_data;
        dst_ptr = dst_data + 1;
        len = 511;
        break;
    case 2:
        src_ptr = src_data + 1;
        dst_ptr = dst_data;
        len = 511;
        break;
    case 3:
        src_ptr = src_data;
        dst_ptr = dst_data + 2;
        len = 513;
        break;
    case 4:
        src_ptr = src_data + 2;
        dst_ptr = dst_data;
        len = 513;
        break;
    case 5:
        src_ptr = src_data;
        dst_ptr = dst_data + 3;
        len = 511;
        break;
    case 6:
        src_ptr = src_data + 3;
        dst_ptr = dst_data;
        len = 513;
        break;

    case 7:
        src_ptr = (uint8_t *)_vaddr(0x50000000);
        dst_ptr = (uint8_t *)_vaddr(0x60000000);
        len = 0x10000 * 4;
        break;

    case 8:
        src_ptr = (uint8_t *)_vaddr(0x50000000);
        dst_ptr = (uint8_t *)_vaddr(0x60000000);
        len = 0x10000 * 8;
        break;

    case 9:
        src_ptr = (uint8_t *)_vaddr(0x50000000);
        dst_ptr = (uint8_t *)_vaddr(0x60000000);
        len = 0x1000000;
        break;
    }
    modifyData(src_ptr, dst_ptr, len + 16);

    struct dma_chan *chan = hal_dma_chan_req(DMA_MEM);
    ASSERT_NOT_NULL(chan);
    struct dma_desc *desc =
        hal_prep_dma_memcpy(chan, dst_ptr, src_ptr, len, DMA_INTERRUPT);
    /*     setup call back function. */
    desc->dmac_irq_evt_handle = tst_irq_evt_handle;
    lk_time_t start = current_time();
    hal_dma_submit(desc);
    enum dma_status ret = hal_dma_sync_wait(desc, 2000); // 10 ms time out
    if (DMA_COMP != ret)
    {
        /* terminate this channel after error. */
        hal_dma_terminate(desc);
    }
    lk_time_t stop = current_time();
    printf("transfer completed with ret(%d) \n", ret);
    /*     free desc and it will free dma channel. */
    hal_dma_free_desc(desc);
    /*  Suggest invalidate cache before read.*/
    arch_invalidate_cache_range((addr_t)dst_ptr, len);

    if (len <= SIZE_BUFFERS)
    {
        EXPECT_BYTES_EQ(src_ptr, dst_ptr, len, "LLP Mode")
        EXPECT_BYTES_NE(src_ptr + len, dst_ptr + len, 16, "LLP mode");
    }
    else
    {
        if (stop > start)
        {
            printf("WARNING: start %lu stop %lu KBps %lu \n", start, stop, len / (stop - start));
        }
        EXPECT_BYTES_EQ(src_ptr + len - 1024, dst_ptr + len - 1024, 1024, "LLP Mode Last")
    }
    TEST_CASE_STATUS("Driver Memcpy Test");
    END_TEST;
}
/* dma memset test cases */
bool drv_tst_dma_memset(u32 case_id)
{
    BEGIN_TEST;
    uint8_t *dst_ptr = dst_data;
    size_t count = 10;
    switch (case_id)
    {
    case 0:
        dst_ptr = dst_data;
        count = 10;
        break;
    case 1:
        dst_ptr = dst_data + 1;
        count = 12;
        break;
    case 2:
        dst_ptr = dst_data + 2;
        count = 13;
        break;
    case 3:
        dst_ptr = dst_data + 3;
        count = 14;
        break;
    }
    u8 val = 0x5a;

    memset(src_data, val, SIZE_BUFFERS); /* clean destination buffer. */
    memset(dst_data, ~val, SIZE_BUFFERS);

    struct dma_chan *chan = hal_dma_chan_req(DMA_MEM);
    ASSERT_NOT_NULL(chan);
    struct dma_desc *desc =
        hal_prep_dma_memset(chan, val, dst_ptr, count, DMA_INTERRUPT);
    /* setup call back function. */
    desc->dmac_irq_evt_handle = tst_irq_evt_handle;

    hal_dma_submit(desc);
    enum dma_status ret = hal_dma_sync_wait(desc, 20); // 10 ms time out
    if (DMA_COMP != ret)
    {
        /* terminate this channel after error. */
        hal_dma_terminate(desc);
    }

    /* free desc and it will free dma channel. */
    hal_dma_free_desc(desc);

    printf("transfer completed with ret(%d) \n", ret);

    /*  Suggest invalidate cache before read. */
    arch_invalidate_cache_range((addr_t)dst_ptr, count);
    EXPECT_BYTES_EQ(src_data, dst_ptr, count, "LLP mode")
    EXPECT_BYTES_NE(src_data, dst_ptr + count, 8, "LLP mode");

    TEST_CASE_STATUS("Driver Memset Test");
    END_TEST;
}
#ifdef ENABLE_SD_I2S
static void *i2s_sc3_handle;
static void *i2s_sc4_handle;
static i2s_sc_init_t i2s_sc3_cfg;
static i2s_sc_init_t i2s_sc4_cfg;

static void *i2s_mc1_handle;
static i2s_mc_init_t i2s_mc1_cfg;
/*case init function*/
bool init_sc_34(void)
{
    bool ret = true;
    i2s_early_init();
    ret = hal_i2s_sc_create_handle(&i2s_sc3_handle, RES_I2S_SC_I2S_SC3);
    if (ret)
        puts("created sc 3 handle succeed.");
    else
        puts("create  sc 3 handle failed.");

    ret = hal_i2s_sc_init(i2s_sc3_handle);
    if (ret)
        puts("init sc 3 handle succeed.");
    else
        puts("init sc 3 handle failed.");

    ret = hal_i2s_sc_create_handle(&i2s_sc4_handle, RES_I2S_SC_I2S_SC4);
    if (ret)
        puts("created sc 4 handle succeed.");
    else
        puts("create  sc 4 handle failed.");

    ret = hal_i2s_sc_init(i2s_sc4_handle);
    if (ret)
        puts("init sc 4 handle succeed.");
    else
        puts("init sc 4 handle failed.");

    return true;
}

void tst_config_i2s_sc(int mode, int width, i2s_sc_init_t *cfg)
{
    cfg->tx_sample_resolution = width;
    cfg->rx_sample_resolution = width;
    cfg->mode = mode;
    cfg->chn_mode = I2S_SC_CHN_STEREO;
    cfg->standard = I2S_STD_PHILLIPS;
    cfg->audio_freq = 44100;
    cfg->chn_width = 7;
    cfg->func_mode = I2S_FUNC_WITH_DMA;
}

void tst_tx_irq_evt_handle(enum dma_status status, u32 param, void *context)
{
    if (status != DMA_ERR)
    {
        printf("tx status (%d) addr(0x%x) context(%p) \n", status, param,
               context);
    }
    else
    {
        printf("tx status (%d) err(0x%x) context(%p) \n", status, param,
               context);
    }
}

void tst_rx_irq_evt_handle(enum dma_status status, u32 param, void *context)
{
    if (status != DMA_ERR)
    {
        printf("rx status (%d) addr(0x%x) context(%p) \n", status, param,
               context);
    }
    else
    {
        printf("rx status (%d) err(0x%x) context(%p) \n", status, param,
               context);
    }
}

bool drv_tst_i2s_sc_dev(uint bytewidth)
{
    BEGIN_TEST;
    modifyData(src_data, dst_data, SIZE_BUFFERS);
    // init_sc_34();
    int ret = 0;

    size_t len = 1024;

    struct dma_dev_cfg cfg_tx;
    cfg_tx.direction = DMA_MEM2DEV;
    cfg_tx.dst_addr = hal_i2s_sc_get_fifo_addr(i2s_sc3_handle);
    // cfg_tx.dst_addr = hal_i2s_sc_get_fifo_addr_4dma(i2s_sc3_handle);
    /*  This part should be bet by peri width, such as for I2S 16bits should set
     * to 1,2,4 bytes.  */
    if (1 == bytewidth)
    {
        cfg_tx.src_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
        cfg_tx.dst_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
    }
    else if (2 == bytewidth)
    {
        cfg_tx.src_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
        cfg_tx.dst_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    }
    else
    {
        cfg_tx.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        cfg_tx.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
    }

    cfg_tx.src_maxburst = DMA_BURST_TR_4ITEMS;
    cfg_tx.dst_maxburst = DMA_BURST_TR_4ITEMS;

    struct dma_chan *tx_chan = hal_dma_chan_req(DMA_PERI_I2S_SC3);
    ASSERT_NOT_NULL(tx_chan);
    hal_dma_dev_config(tx_chan, &cfg_tx);
    struct dma_desc *desc_tx;
    desc_tx = hal_prep_dma_dev(tx_chan, src_data, len, DMA_INTERRUPT);
    desc_tx->dmac_irq_evt_handle = tst_tx_irq_evt_handle;
    desc_tx->context = (void *)0xA5A5;

    /* config rx channel for dev */

    struct dma_dev_cfg cfg_rx;
    cfg_rx.direction = DMA_DEV2MEM;
    cfg_rx.src_addr = hal_i2s_sc_get_fifo_addr(i2s_sc4_handle);

    /*  This part should be bet by peri width, such as for I2S 16bits should set
     * to 2 bytes. s */

    if (1 == bytewidth)
    {
        cfg_rx.src_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
        cfg_rx.dst_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
    }
    else if (2 == bytewidth)
    {
        cfg_rx.src_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
        cfg_rx.dst_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    }
    else
    {
        cfg_rx.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        cfg_rx.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
    }

    cfg_rx.src_maxburst = DMA_BURST_TR_4ITEMS;
    cfg_rx.dst_maxburst = DMA_BURST_TR_4ITEMS;

    struct dma_chan *rx_chan = hal_dma_chan_req(DMA_PERI_I2S_SC4);
    ASSERT_NOT_NULL(rx_chan);
    hal_dma_dev_config(rx_chan, &cfg_rx);
    struct dma_desc *desc_rx;
    desc_rx = hal_prep_dma_dev(rx_chan, dst_data, len, DMA_INTERRUPT);
    desc_rx->dmac_irq_evt_handle = tst_rx_irq_evt_handle;
    desc_rx->context = (void *)0x5A5A;

    /* config_i2s_sc TX  */
    if (1 == bytewidth)
    {
        tst_config_i2s_sc(I2S_SC_MOD_MASTER_TX, I2S_SAMPLE_16_BIT,
                          &i2s_sc3_cfg);
    }
    else if (2 == bytewidth)
    {
        tst_config_i2s_sc(I2S_SC_MOD_MASTER_TX, I2S_SAMPLE_32_BIT,
                          &i2s_sc3_cfg);
    }
    else
    {
        tst_config_i2s_sc(I2S_SC_MOD_MASTER_TX, I2S_SAMPLE_8_BIT,
                          &i2s_sc3_cfg);
    }

    /* config_i2s_sc RX  */
    if (1 == bytewidth)
    {
        tst_config_i2s_sc(I2S_SC_MOD_SLAVE_RX, I2S_SAMPLE_16_BIT,
                          &i2s_sc4_cfg);
    }
    else if (2 == bytewidth)
    {
        tst_config_i2s_sc(I2S_SC_MOD_SLAVE_RX, I2S_SAMPLE_32_BIT,
                          &i2s_sc4_cfg);
    }
    else
    {
        tst_config_i2s_sc(I2S_SC_MOD_SLAVE_RX, I2S_SAMPLE_8_BIT,
                          &i2s_sc4_cfg);
    }
    ret = hal_i2s_sc_config(i2s_sc3_handle, &i2s_sc3_cfg);
    /*    if (ret)
        puts("i2s sc3 config succeed.");
    else
        puts("i2s sc3 config failed."); */
    ret = hal_i2s_sc_config(i2s_sc4_handle, &i2s_sc4_cfg);
    /*     if (ret)
        puts("i2s sc4 config succeed.");
    else
        puts("i2s sc4 config failed."); */

    ret = hal_i2s_sc_start(i2s_sc3_handle);
    ret = hal_i2s_sc_start(i2s_sc4_handle);
    hal_dma_submit(desc_tx);
    hal_dma_submit(desc_rx);

    enum dma_status ret_rx = hal_dma_sync_wait(desc_rx, 200); // 10 ms time out
    if (DMA_COMP != ret_rx)
    {
        hal_dma_terminate(desc_tx);
        hal_dma_terminate(desc_rx);
    }
    printf("transfer completed with ret(%d) \n", ret);

    hal_dma_free_desc(desc_tx);
    hal_dma_free_desc(desc_rx);

    ret = hal_i2s_sc_stop(i2s_sc3_handle);
    ret = hal_i2s_sc_stop(i2s_sc4_handle);
    EXPECT_BYTES_EQ(src_data, dst_data, len, "No LLP mode")
    EXPECT_BYTES_NE(src_data + len, dst_data + len, 16, "No LLP mode");
    TEST_CASE_STATUS("Driver dma dev Test");
    END_TEST;
}

void tst_tx_irq_evt_cyclic_handle(enum dma_status status, u32 param,
                                  void *context)
{
    if (status != DMA_ERR)
    {
        printf("tx status (%d) addr(0x%x) context(%p) \n", status, param,
               context);
    }
    else
    {
        printf("tx status (%d) err(0x%x) context(%p) \n", status, param,
               context);
    }
}
static int dma_xfer_count = 0;
struct dma_desc *desc_tx_cyclic;
struct dma_desc *desc_rx_cyclic;
void tst_rx_irq_evt_cyclic_handle(enum dma_status status, u32 param,
                                  void *context)
{
    if (status != DMA_ERR)
    {

        dma_xfer_count++;
        printf("rx status (%d) addr(0x%x) context(%p) count(0x%x) \n", status,
               param, context, dma_xfer_count);
        if (dma_xfer_count > 3)
        {
            hal_dma_terminate(desc_tx_cyclic);
            hal_dma_terminate(desc_rx_cyclic);
        }
    }
    else
    {
        printf("rx status (%d) err(0x%x) context(%p) \n", status, param,
               context);
    }
}

bool drv_tst_i2s_sc_cyclic(uint bytewidth)
{
    BEGIN_TEST;
    modifyData(src_data, dst_data, SIZE_BUFFERS);
    //  init_sc_34();
    int ret = 0;
    dma_xfer_count = 0;
    size_t len = 1024;

    struct dma_dev_cfg cfg_tx;
    cfg_tx.direction = DMA_MEM2DEV;
    cfg_tx.dst_addr = hal_i2s_sc_get_fifo_addr(i2s_sc3_handle);
    // cfg_tx.dst_addr = hal_i2s_sc_get_fifo_addr_4dma(i2s_sc3_handle);
    /*  This part should be bet by peri width, such as for I2S 16bits should set
     * to 1,2,4 bytes.  */
    if (1 == bytewidth)
    {
        cfg_tx.src_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
        cfg_tx.dst_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
    }
    else if (2 == bytewidth)
    {
        cfg_tx.src_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
        cfg_tx.dst_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    }
    else
    {
        cfg_tx.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        cfg_tx.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
    }

    cfg_tx.src_maxburst = DMA_BURST_TR_4ITEMS;
    cfg_tx.dst_maxburst = DMA_BURST_TR_4ITEMS;

    struct dma_chan *tx_chan = hal_dma_chan_req(DMA_PERI_I2S_SC3);
    ASSERT_NOT_NULL(tx_chan);
    hal_dma_dev_config(tx_chan, &cfg_tx);

    desc_tx_cyclic =
        hal_prep_dma_cyclic(tx_chan, src_data, len, len / 4, DMA_INTERRUPT);
    desc_tx_cyclic->dmac_irq_evt_handle = tst_tx_irq_evt_cyclic_handle;
    desc_tx_cyclic->context = (void *)0xA5A5;

    /* config rx channel for dev */

    struct dma_dev_cfg cfg_rx;
    cfg_rx.direction = DMA_DEV2MEM;
    cfg_rx.src_addr = hal_i2s_sc_get_fifo_addr(i2s_sc4_handle);

    /*  This part should be bet by peri width, such as for I2S 16bits should set
     * to 2 bytes. s */

    if (1 == bytewidth)
    {
        cfg_rx.src_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
        cfg_rx.dst_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
    }
    else if (2 == bytewidth)
    {
        cfg_rx.src_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
        cfg_rx.dst_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    }
    else
    {
        cfg_rx.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        cfg_rx.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
    }

    cfg_rx.src_maxburst = DMA_BURST_TR_4ITEMS;
    cfg_rx.dst_maxburst = DMA_BURST_TR_4ITEMS;

    struct dma_chan *rx_chan = hal_dma_chan_req(DMA_PERI_I2S_SC4);
    ASSERT_NOT_NULL(rx_chan);
    hal_dma_dev_config(rx_chan, &cfg_rx);

    desc_rx_cyclic =
        hal_prep_dma_cyclic(rx_chan, dst_data, len, len / 4, DMA_INTERRUPT);
    desc_rx_cyclic->dmac_irq_evt_handle = tst_rx_irq_evt_cyclic_handle;
    desc_rx_cyclic->context = (void *)0x5A5A;

    /* config_i2s_sc TX  */
    if (1 == bytewidth)
    {
        tst_config_i2s_sc(I2S_SC_MOD_MASTER_TX, I2S_SAMPLE_16_BIT,
                          &i2s_sc3_cfg);
    }
    else if (2 == bytewidth)
    {
        tst_config_i2s_sc(I2S_SC_MOD_MASTER_TX, I2S_SAMPLE_32_BIT,
                          &i2s_sc3_cfg);
    }
    else
    {
        tst_config_i2s_sc(I2S_SC_MOD_MASTER_TX, I2S_SAMPLE_8_BIT,
                          &i2s_sc3_cfg);
    }

    /* config_i2s_sc RX  */
    if (1 == bytewidth)
    {
        tst_config_i2s_sc(I2S_SC_MOD_SLAVE_RX, I2S_SAMPLE_16_BIT,
                          &i2s_sc4_cfg);
    }
    else if (2 == bytewidth)
    {
        tst_config_i2s_sc(I2S_SC_MOD_SLAVE_RX, I2S_SAMPLE_32_BIT,
                          &i2s_sc4_cfg);
    }
    else
    {
        tst_config_i2s_sc(I2S_SC_MOD_SLAVE_RX, I2S_SAMPLE_8_BIT,
                          &i2s_sc4_cfg);
    }
    ret = hal_i2s_sc_config(i2s_sc3_handle, &i2s_sc3_cfg);
    /*    if (ret)
        puts("i2s sc3 config succeed.");
    else
        puts("i2s sc3 config failed."); */

    ret = hal_i2s_sc_config(i2s_sc4_handle, &i2s_sc4_cfg);
    /*     if (ret)
        puts("i2s sc4 config succeed.");
    else
        puts("i2s sc4 config failed."); */

    ret = hal_i2s_sc_start(i2s_sc3_handle);
    ret = hal_i2s_sc_start(i2s_sc4_handle);
    hal_dma_submit(desc_tx_cyclic);
    hal_dma_submit(desc_rx_cyclic);

    enum dma_status ret_rx =
        hal_dma_sync_wait(desc_rx_cyclic, 200); // 10 ms time out
    if (DMA_COMP != ret_rx)
    {
        hal_dma_terminate(desc_tx_cyclic);
        hal_dma_terminate(desc_rx_cyclic);
    }
    printf("transfer completed with ret(%d) \n", ret);
    hal_dma_free_desc(desc_tx_cyclic);
    hal_dma_free_desc(desc_rx_cyclic);
    ret = hal_i2s_sc_stop(i2s_sc3_handle);
    ret = hal_i2s_sc_stop(i2s_sc4_handle);

    EXPECT_BYTES_EQ(src_data, dst_data, len, "No LLP mode")
    EXPECT_BYTES_NE(src_data + len, dst_data + len, 16, "No LLP mode");
    TEST_CASE_STATUS("Driver dma dev Test");
    END_TEST;
}
#endif

bool drv_tst_memcpy_ch15(u32 ch)
{
    BEGIN_TEST;
    modifyData(src_data, dst_data, SIZE_BUFFERS);

    size_t len = 512;
    struct dma_chan *chan = hal_dma_chan_req_with_ch(ch);
    ASSERT_NOT_NULL(chan);
    struct dma_desc *desc =
        hal_prep_dma_memcpy(chan, dst_data, src_data, len, DMA_INTERRUPT);
    /*     setup call back function. */
    desc->dmac_irq_evt_handle = tst_irq_evt_handle;

    hal_dma_submit(desc);
    enum dma_status ret = hal_dma_sync_wait(desc, 20); // 10 ms time out
    if (DMA_COMP != ret)
    {
        /* terminate this channel after error. */
        hal_dma_terminate(desc);
    }
    printf("transfer completed with ret(%d) \n", ret);
    /*     free desc and it will free dma channel. */
    hal_dma_free_desc(desc);

    EXPECT_BYTES_EQ(src_data, dst_data, len, "No LLP mode")
    EXPECT_BYTES_NE(src_data + len, dst_data + len, 16, "No LLP mode");
    TEST_CASE_STATUS("Driver Memcpy Test");
    END_TEST;
}

int dma_tst(int argc, const cmd_args *argv)
{
    if (argc != 2)
    {
        printf("dma test argc:%d: number error!\n", argc);
        return -1;
    }
    int tst_id = argv[1].i;
    if ((tst_id < 0) || (tst_id >= TST_CASES_NUM))
    {
        printf("dma test id error:%d !\n", tst_id);
        return -1;
    }
    printf("Case %-20d %s \n", tst_id, dma_tst_cases[tst_id].name);
    switch (tst_id)
    {
    case 0:
        drv_tst_dma_memcpy(0);
        break;

    case 1:
        drv_tst_dma_memcpy(1);
        break;

    case 2:
        drv_tst_dma_memcpy(2);
        break;

    case 3:
        drv_tst_dma_memcpy(3);
        break;

    case 4:
        drv_tst_dma_memcpy(4);
        break;

    case 5:
        drv_tst_dma_memcpy(5);
        break;

    case 6:
        drv_tst_dma_memcpy(6);
        break;

    case 7:
        printf("This case is running in DDR 0x5000 0000 ~ 0x6000 0000 \n");
        drv_tst_dma_memcpy(7);
        break;

    case 8:
        printf("This case is running in DDR 0x5000 0000 ~ 0x6000 0000 \n");
        drv_tst_dma_memcpy(8);
        break;

    case 9:
        printf("This case is running in DDR 0x5000 0000 ~ 0x6000 0000 \n");
        drv_tst_dma_memcpy(9);
        break;

    case 10:
        drv_tst_dma_memset(0);
        break;

    case 11:
        drv_tst_dma_memset(1);
        break;

    case 12:
        drv_tst_dma_memset(2);
        break;

    case 13:
        drv_tst_dma_memset(3);
        break;

#ifdef ENABLE_SD_I2S
    case 11:
        /* 8 bit width set to 0 */
        drv_tst_i2s_sc_dev(0);
        break;

    case 12:
        /* 16 bit width set to 0 */
        drv_tst_i2s_sc_dev(1);
        break;

    case 13:
        /* 32 bit width set to 0 */
        drv_tst_i2s_sc_dev(2);
        break;

    case 14:
        /* 8 bit width set to 0 */
        drv_tst_i2s_sc_cyclic(0);
        break;

    case 15:
        /* 16 bit width set to 0 */
        drv_tst_i2s_sc_cyclic(1);
        break;

    case 16:
        /* 32 bit width set to 0 */
        drv_tst_i2s_sc_cyclic(2);
        break;
#endif

    default:
        /*Memcpy with special channel*/
        drv_tst_memcpy_ch15(15);
        break;
    }
    return 0;
}
/* #define STATIC_COMMAND_FUNC(func_name) \
    STATIC_COMMAND("#func_name, "test dma " #func_name, (console_cmd)&func_name)
 */

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("dma_test_list", "list dma test cases",
               (console_cmd)&tst_cfg_list)
#ifdef ENABLE_SD_I2S
STATIC_COMMAND("dma_init", "init sc", (console_cmd)&init_sc_34)
#endif
STATIC_COMMAND("dma_test", "do dma test e.g dma_test 0", (console_cmd)&dma_tst)
STATIC_COMMAND_END(dmatest);
#endif
APP_START(dma_test).flags = 0 APP_END
