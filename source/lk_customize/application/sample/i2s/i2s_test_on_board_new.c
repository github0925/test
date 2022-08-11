//*****************************************************************************
//
// i2s_test_on_board_new.c
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include "dma_hal.h"
#include "i2s_hal.h"
#include "i2c_hal.h"
#include "res.h"
#include "wav.h"
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <platform.h>
#include <platform/debug.h>
#include <lib/console.h>
#include <string.h>
#include <audio_common.h>
#include <audio_test.h>
#include "tlv320.h"
#include <kernel/event.h>

#define PERIOD_SIZE 2048
#define TEST_DURATION (10000)

static void *i2s_handle;
static struct dma_desc *desc_tx, *desc_rx;
static struct audio_codec_dev *tlv;
const static struct au_codec_dev_ctrl_interface *tlv_drv;
static lk_time_t start_ts;
static event_t e_record_end;
static uint8_t cyclic_buffer[ROUNDUP(PERIOD_SIZE,
                                     CACHE_LINE)]__attribute__((aligned(CACHE_LINE)));

static void init_tlv320_codec(pcm_params_t pcm)
{
    tlv = sdrv_tlv320_get_dev(2);
    tlv_drv = sdrv_tlv320_get_controller_interface();

    hal_i2c_creat_handle(&tlv->i2c_handle, tlv->i2c_res_num);

    tlv_drv->start_up(tlv);
    tlv_drv->set_format(*tlv, pcm);
    tlv_drv->set_hw_params(*tlv, pcm);
}

static void trigger_tlv320_codec(int cmd)
{
    tlv_drv->trigger(*tlv, cmd);
}

static void disable_tlv320_codec(struct audio_codec_dev *dev)
{
    tlv_drv->trigger(*dev, SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP);
    tlv_drv->trigger(*dev, SD_AUDIO_PCM_TRIGGER_CAPTURE_STOP);
    tlv_drv->shutdown(*dev);
}

static void playback_irq_evt_handle(enum dma_status status, u32 err,
                                    void *context)
{
    printf("dma irq evt: status(%d) err(%d) context(%p) \n", status, err,
           context);

    if (DMA_COMP != status) {
        /* terminate this channel after error. */
        hal_dma_terminate(desc_tx);
        trigger_tlv320_codec(SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP);
    }
}

static void record_cyclic_irq_evt_handle(enum dma_status status, u32 err,
        void *context)
{
    // printf("dma irq evt: status(%d) err(%d) context(%p) \n", status, err, context);

    if ((current_time() - start_ts) > TEST_DURATION) {
        printf("sc record stopped ts(%d)\n", current_time());
        hal_dma_terminate(desc_rx);
        trigger_tlv320_codec(SD_AUDIO_PCM_TRIGGER_CAPTURE_STOP);
        event_signal(&e_record_end, false);
    }
}

void sd_audio_trigger_action(u32 casenum)
{
    uint32_t dir;

    if (!hal_sd_i2s_sc_create_handle(&i2s_handle, RES_I2S_SC_I2S_SC8))
        printf("error:i2s resource get failed.\n");

    pcm_params_t pcm_info = sd_audio_get_test_params(casenum);
    pcm_info.slot_width = SD_AUDIO_SLOT_WIDTH_16BITS;
    dir = pcm_info.mode & SD_AUDIO_DIR_MODE_ENABLE;
    hal_sd_i2s_sc_start_up(i2s_handle, pcm_info);
    hal_sd_i2s_sc_set_format(i2s_handle);
    hal_sd_i2s_sc_set_hw_params(i2s_handle);
    init_tlv320_codec(pcm_info);
    start_ts = current_time();
    event_init(&e_record_end, false, EVENT_FLAG_AUTOUNSIGNAL);

    if (dir == SD_AUDIO_DIR_MODE_TRANSMIT) {
        trigger_tlv320_codec(SD_AUDIO_PCM_TRIGGER_PLAYBACK_START);
        desc_tx = hal_i2s_trigger_dma_tr_start(i2s_handle, DMA_PERI_I2S_SC8,
                                               wav_data,
                                               sizeof wav_data, playback_irq_evt_handle,
                                               SD_AUDIO_PCM_TRIGGER_PLAYBACK_START);
        hal_dma_sync_wait(desc_tx, -1);
        hal_sd_i2s_sc_trigger(i2s_handle, SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP);
        disable_tlv320_codec(tlv);
        hal_dma_free_desc(desc_tx);
        hal_sd_i2s_sc_shutdown(i2s_handle);
        hal_sd_i2s_sc_release_handle(i2s_handle);
        hal_i2c_release_handle(tlv->i2c_handle);
    }
    else if (dir == SD_AUDIO_DIR_MODE_RECEIVE) {
        trigger_tlv320_codec(SD_AUDIO_PCM_TRIGGER_CAPTURE_START);
        desc_rx = hal_i2s_trigger_dma_tr_start(i2s_handle, DMA_MEM, cyclic_buffer,
                                               PERIOD_SIZE,
                                               record_cyclic_irq_evt_handle,
                                               SD_AUDIO_PCM_TRIGGER_CAPTURE_START);
        event_wait_timeout(&e_record_end, -1);
        hal_sd_i2s_sc_trigger(i2s_handle, SD_AUDIO_PCM_TRIGGER_CAPTURE_STOP);
        disable_tlv320_codec(tlv);
        hal_dma_free_desc(desc_rx);
        hal_sd_i2s_sc_shutdown(i2s_handle);
        hal_sd_i2s_sc_release_handle(i2s_handle);
        hal_i2c_release_handle(tlv->i2c_handle);
    }
    else if (dir == SD_AUDIO_DIR_MODE_FULL_DUPLEX) {
        trigger_tlv320_codec(SD_AUDIO_PCM_TRIGGER_PLAYBACK_START);
        trigger_tlv320_codec(SD_AUDIO_PCM_TRIGGER_CAPTURE_START);
        desc_tx = hal_i2s_trigger_dma_tr_start(i2s_handle, DMA_PERI_I2S_SC8,
                                               wav_data,
                                               sizeof wav_data, playback_irq_evt_handle,
                                               SD_AUDIO_PCM_TRIGGER_PLAYBACK_START);
        desc_rx = hal_i2s_trigger_dma_tr_start(i2s_handle, DMA_MEM, cyclic_buffer,
                                               PERIOD_SIZE,
                                               record_cyclic_irq_evt_handle,
                                               SD_AUDIO_PCM_TRIGGER_CAPTURE_START);
        hal_dma_sync_wait(desc_tx, -1);
        event_wait_timeout(&e_record_end, -1);
        hal_sd_i2s_sc_trigger(i2s_handle, SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP);
        hal_sd_i2s_sc_trigger(i2s_handle, SD_AUDIO_PCM_TRIGGER_CAPTURE_STOP);

        disable_tlv320_codec(tlv);
        hal_dma_free_desc(desc_tx);
        hal_dma_free_desc(desc_rx);
        hal_sd_i2s_sc_release_handle(i2s_handle);
        hal_i2c_release_handle(tlv->i2c_handle);
    }

}

void audio_test_start(int argc, const cmd_args *argv)
{
    static u32 casenum = 0;

    if (argc < 2) {
        sd_audio_test_usage();
        return;
    }

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i].str, "-help")) {
            sd_audio_test_usage();
        }
        else if (!strcmp(argv[i].str, "-n") && i + 1 < argc) {
            casenum = argv[++i].i - 1;

            if (casenum < 960 && casenum >= 0) {
                sd_audio_show_test_info_by_casenum(casenum);
            }
            else {
                printf("error: casenum out of range.\n");
                sd_audio_test_usage();
            }
        }

        if (!strcmp(argv[i].str, "-r")) {
            if (casenum < 960 && casenum >= 0) {
                sd_audio_trigger_action(casenum);
            }
            else {
                printf("error: casenum out of range.\n");
                sd_audio_test_usage();
            }
        }
    }
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("audio_test", "test audio parameters.",
               (console_cmd)&audio_test_start)
STATIC_COMMAND_END(i2s_onboard_test);
#endif

APP_START(i2s_test_on_board)
.flags = 0
         APP_END