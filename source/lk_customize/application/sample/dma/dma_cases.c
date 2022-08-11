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
#include "i2s_hal.h"
/* Invoke init function for test*/
#include "app_i2s.h"
#endif

#define SIZE_BUFFERS 1024
static uint8_t src_data[ROUNDUP(SIZE_BUFFERS, CACHE_LINE)]
    __attribute__((aligned(CACHE_LINE))) = {0};
static uint8_t dst_data[ROUNDUP(SIZE_BUFFERS, CACHE_LINE)]
    __attribute__((aligned(CACHE_LINE))) = {0};

/*
 modify data to create different data pattern for testing.
*/
static void modifyData(uint8_t *src, uint8_t *dst, int words)
{
    int i;
    printf("src addr:0x%lx\n", src);
    printf("dst addr:0x%lx\n", dst);
    // Put some data in the source buffer for test
    for (i = 0; i < words; i++)
    {
        src[i] = i % 255 + 1;
        dst[i] = ~src[i]; // 255 - i % 255;
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
#ifdef ENABLE_SD_I2S
static void *i2s_sc3_handle;
static void *i2s_sc4_handle;
static i2s_sc_init_t i2s_sc3_cfg;
static i2s_sc_init_t i2s_sc4_cfg;

static void *i2s_mc1_handle;
static i2s_mc_init_t i2s_mc1_cfg;
#endif

/*case init function*/
bool init_dma_cases(void)
{
    bool ret = true;
#ifdef ENABLE_SD_I2S
    /* Here invoke i2s setting */
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

#endif
    return ret;
}
static struct dma_desc *memcpy_desc;
/* dmac irq evt handle */
void dmac_irq_evt_handle(enum dma_status status, u32 err, void *context)
{
    printf("dma irq evt: status(%d) err(%d) context(%p) \n", status, err,
           context);
    if (DMA_COMP != status)
    {
        /* terminate this channel after error. */
        hal_dma_terminate(memcpy_desc);
    }
    hal_dma_free_desc(memcpy_desc);
}

/* dma memcpy samples */
bool drv_case_memcpy(void)
{
    BEGIN_TEST;
    modifyData(src_data, dst_data, SIZE_BUFFERS);

    size_t len = 512;
    struct dma_chan *chan = hal_dma_chan_req(DMA_MEM);
    ASSERT_NOT_NULL(chan);
    memcpy_desc =
        hal_prep_dma_memcpy(chan, dst_data, src_data, len, DMA_INTERRUPT);
    /*     setup call back function. */
    memcpy_desc->dmac_irq_evt_handle = dmac_irq_evt_handle;

    hal_dma_submit(memcpy_desc);

    /*  Suggest invalidate cache before read from destination buffer.  */

    END_TEST;
}
/* dma memset samples */
bool drv_case_memset(void)
{
    BEGIN_TEST;
    u8 val = 0x5a;
    memset(src_data, val, SIZE_BUFFERS); /* clean destination buffer. */
    memset(dst_data, 1, SIZE_BUFFERS);

    size_t count = 10;
    struct dma_chan *chan = hal_dma_chan_req(DMA_MEM);
    ASSERT_NOT_NULL(chan);
    struct dma_desc *desc = hal_prep_dma_memset(chan, val, dst_data, count, 0);

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
    EXPECT_BYTES_EQ(src_data, dst_data, count, "No LLP mode")

    TEST_CASE_STATUS("Driver Memset Test");
    END_TEST;
}

#ifdef ENABLE_SD_I2S
void config_i2s_sc(int mode, int width, i2s_sc_init_t *cfg)
{
    cfg->tx_sample_resolution = width;
    cfg->rx_sample_resolution = width;
    cfg->mode = mode;
    cfg->chn_mode = I2S_SC_CHN_STEREO;
    cfg->standard = I2S_STD_PHILLIPS;
    cfg->audio_freq = 96000; // 44100
    cfg->chn_width = 7;
    cfg->func_mode = I2S_FUNC_WITH_DMA;
}

void tx_irq_evt_handle(enum dma_status status, u32 param, void *context)
{
    if (status == DMA_PENDING)
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

void rx_irq_evt_handle(enum dma_status status, u32 param, void *context)
{
    if (status == DMA_PENDING)
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

bool drv_case_i2s(void)
{
    BEGIN_TEST;
    modifyData(src_data, dst_data, SIZE_BUFFERS);
    int ret = 0;

    size_t len = 1024;

    struct dma_dev_cfg cfg_tx;
    cfg_tx.direction = DMA_MEM2DEV;
    cfg_tx.dst_addr = hal_i2s_sc_get_fifo_addr(i2s_sc3_handle);
    // cfg_tx.dst_addr = hal_i2s_sc_get_fifo_addr_4dma(i2s_sc3_handle);
    /*  This part should be bet by peri width, such as for I2S 16bits should set
     * to 2 bytes.  */
    cfg_tx.src_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
    cfg_tx.dst_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;

    cfg_tx.src_maxburst = DMA_BURST_TR_4ITEMS;
    cfg_tx.dst_maxburst = DMA_BURST_TR_4ITEMS;

    struct dma_chan *tx_chan = hal_dma_chan_req(DMA_PERI_I2S_SC3);
    ASSERT_NOT_NULL(tx_chan);
    hal_dma_dev_config(tx_chan, &cfg_tx);
    struct dma_desc *desc_tx;
    desc_tx = hal_prep_dma_dev(tx_chan, src_data, len, DMA_INTERRUPT);
    desc_tx->dmac_irq_evt_handle = tx_irq_evt_handle;
    desc_tx->context = (void *)0xA5A5;

    /* config rx channel for dev */

    struct dma_dev_cfg cfg_rx;
    cfg_rx.direction = DMA_DEV2MEM;
    cfg_rx.src_addr = hal_i2s_sc_get_fifo_addr(i2s_sc4_handle);

    /*  This part should be bet by peri width, such as for I2S 16bits should set
     * to 2 bytes. s */

    cfg_rx.src_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
    cfg_rx.dst_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;

    cfg_rx.src_maxburst = DMA_BURST_TR_4ITEMS;
    cfg_rx.dst_maxburst = DMA_BURST_TR_4ITEMS;

    struct dma_chan *rx_chan = hal_dma_chan_req(DMA_PERI_I2S_SC4);
    ASSERT_NOT_NULL(rx_chan);
    hal_dma_dev_config(rx_chan, &cfg_rx);
    struct dma_desc *desc_rx;
    desc_rx = hal_prep_dma_dev(rx_chan, dst_data, len, DMA_INTERRUPT);
    desc_rx->dmac_irq_evt_handle = rx_irq_evt_handle;
    desc_rx->context = (void *)0x5A5A;

    /* config_i2s_sc TX  */
    config_i2s_sc(I2S_SC_MOD_MASTER_TX, I2S_SAMPLE_16_BIT, &i2s_sc3_cfg);
    ret = hal_i2s_sc_config(i2s_sc3_handle, &i2s_sc3_cfg);
    if (ret)
        puts("i2s sc3 config succeed.");
    else
        puts("i2s sc3 config failed.");

    /* config_i2s_sc RX  */
    config_i2s_sc(I2S_SC_MOD_SLAVE_RX, I2S_SAMPLE_16_BIT, &i2s_sc4_cfg);
    ret = hal_i2s_sc_config(i2s_sc4_handle, &i2s_sc4_cfg);
    if (ret)
        puts("i2s sc4 config succeed.");
    else
        puts("i2s sc4 config failed.");

    ret = hal_i2s_sc_start(i2s_sc3_handle);
    ret = hal_i2s_sc_start(i2s_sc4_handle);

    hal_dma_submit(desc_tx);
    hal_dma_submit(desc_rx);

    enum dma_status ret_rx = hal_dma_sync_wait(desc_rx, 100); // 10 ms time out
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
#endif

bool run_case(int caseid)
{
    switch (caseid)
    {
    case 0:
        drv_case_memcpy();
        break;
    case 1:
        drv_case_memset();
        break;
#ifdef ENABLE_SD_I2S
    case 2:
        drv_case_i2s();
        break;
#endif
    }
    return true;
}