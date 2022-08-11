//*****************************************************************************
//
// cmd_spdif.c
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include <app.h>
#include <spdif_hal.h>
#include <chip_res.h>
#include <wav_16LE.h>
#include <wav_24LE.h>
#include <stdio.h>
#include <dma_hal.h>

static void *spdif_tx;
static spdif_cfg_info_t cfg_tx;

static void dmac_irq_evt_handle(enum dma_status status, u32 err,
                                void *context)
{
    printf("dma irq evt: status(%d) err(%d) context(%p) \n", status, err,
           context);
}

void spdif_demo(void)
{
    hal_spdif_create_handle(&spdif_tx, RES_SPDIF_SPDIF2);
    hal_spdif_init(spdif_tx);

    cfg_tx.tr_mode = SPDIF_TRANSMITTER;
    cfg_tx.tsample_rate = 44100;
    cfg_tx.transfer_mode = SPDIF_TR_WITH_DMA;
    cfg_tx.resolution = SPDIF_FORMAT_16BITS;
    cfg_tx.ch_mode = SPDIF_STEREO;
    cfg_tx.preamble_delay = 10;
    cfg_tx.parity = SPDIF_ENABLE;
    cfg_tx.aempty_threshold = SPDIF_FIFO_THRESHOLD_DEFAULT;
    cfg_tx.afull_threshold = SPDIF_FIFO_THRESHOLD_DEFAULT;

    hal_spdif_config(spdif_tx, &cfg_tx);
    hal_spdif_show_reg_config(spdif_tx);

    // dma cfg
    size_t len = sizeof wav_data;
    struct dma_chan *tx_ch = hal_dma_chan_req(DMA_PERI_SPDIF2);

    if (tx_ch == NULL)
        return;

    struct dma_dev_cfg dma_cfg_tx;
    dma_cfg_tx.direction = DMA_MEM2DEV;
    dma_cfg_tx.dst_addr = hal_spdif_get_fifo_addr(spdif_tx);
    dma_cfg_tx.src_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    dma_cfg_tx.dst_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    dma_cfg_tx.src_maxburst = DMA_BURST_TR_16ITEMS;
    dma_cfg_tx.dst_maxburst = DMA_BURST_TR_16ITEMS;
    hal_dma_dev_config(tx_ch, &dma_cfg_tx);
    struct dma_desc *desc_tx;
    desc_tx = hal_prep_dma_dev(tx_ch, (void *)wav_data1, len, DMA_INTERRUPT);
    desc_tx->dmac_irq_evt_handle = dmac_irq_evt_handle;
    desc_tx->context = (void *)0xa5a5;

    hal_spdif_start(spdif_tx);
    hal_dma_submit(desc_tx);

    enum dma_status ret = hal_dma_sync_wait(desc_tx, 10000);

    if (DMA_COMP != ret) {
        hal_dma_terminate(desc_tx);
    }

    printf("dma transfer completed with ret(%d) \n", ret);
    hal_dma_free_desc(desc_tx);
    hal_spdif_stop(spdif_tx);
    hal_spdif_release_handle(spdif_tx);
}

void spdif_play_int(void)
{
    hal_spdif_create_handle(&spdif_tx, RES_SPDIF_SPDIF2);
    hal_spdif_init(spdif_tx);

    cfg_tx.tr_mode = SPDIF_TRANSMITTER;
    cfg_tx.tsample_rate = 44100;
    cfg_tx.transfer_mode = SPDIF_TR_WITH_INT;
    cfg_tx.resolution = SPDIF_FORMAT_16BITS;
    cfg_tx.ch_mode = SPDIF_STEREO;
    cfg_tx.preamble_delay = 0;
    cfg_tx.parity = SPDIF_ENABLE;
    cfg_tx.ptx_buffer = (void *)wav_data;
    cfg_tx.tx_count = sizeof wav_data;
    cfg_tx.aempty_threshold = SPDIF_FIFO_THRESHOLD_DEFAULT;
    cfg_tx.afull_threshold = SPDIF_FIFO_THRESHOLD_DEFAULT;

    hal_spdif_config(spdif_tx, &cfg_tx);
    hal_spdif_show_reg_config(spdif_tx);
    hal_spdif_start(spdif_tx);

    hal_spdif_wait_tx_comp_intmode(spdif_tx, 10000);
    hal_spdif_stop(spdif_tx);
    hal_spdif_release_handle(spdif_tx);
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>

STATIC_COMMAND_START
STATIC_COMMAND("spdif_pb_dma", "spdif playback with dma",
               (console_cmd)&spdif_demo)
STATIC_COMMAND("spdif_pb_int", "spdif playback with int",
               (console_cmd)&spdif_play_int)
STATIC_COMMAND_END(spdif);
#endif

APP_START(spdif)
.flags = 0,
APP_END
