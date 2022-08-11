/*
 * Copyright (c) 2019, SemiDrive, Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "dma_hal.h"
#include "i2s_hal.h"
#include "i2c_hal.h"
#include "res.h"
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <platform.h>
#include <platform/debug.h>
#include <lib/console.h>
#include <string.h>
#include <kernel/event.h>
#include <lib/slt_module_test.h>

#define TEST_DURATION (10000)
typedef void(*hal_spi_int_func_cbk)(enum dma_status status, u32 err,
                                    void *context);
static bool i2s1_result = false;
static bool i2s2_result = false;

static char wav_data[] = {
    0xff, 0x10, 0xff, 0x10, 0xea, 0xb, 0xea, 0xb,
    0xf9, 0x07, 0xf9, 0x07, 0xb4, 0x8, 0xb4, 0x8
};

typedef enum {
    I2S1_DMA = DMA_PERI_I2S_SC1,
    I2S2_DMA = DMA_PERI_I2S_SC2,
} i2s_dma_e;

typedef enum {
    I2S1_REG = RES_I2S_SC_I2S_SC1,
    I2S2_REG = RES_I2S_SC_I2S_SC2,
    I2S_MAX  = 2
} i2s_reg_e;

typedef struct {
    bool *result;
    uint8_t channel;
    int resid;
    int dmaid;
    uint32_t irq_num;
    void *handle;
    hal_spi_int_func_cbk cbk;
} i2s_device_t;

typedef struct {
    mutex_t mutex;
    i2s_device_t *dev;
} i2s_contex_t;

static inline void slt_i2s1_ovf_cbk(enum dma_status status, u32 err,
                                    void *context)
{
    i2s1_result = true;
    dprintf(ALWAYS, "slt i2s1 ovf cbk sucessfully!\n");
}

static inline void slt_i2s2_ovf_cbk(enum dma_status status, u32 err,
                                    void *context)
{
    i2s2_result = true;
    dprintf(ALWAYS, "slt i2s2 ovf cbk sucessfully!\n");
}

static int slt_i2s_checkout_result(i2s_contex_t *i2s_contex)
{
    int ret = -1;
    mutex_acquire(&i2s_contex->mutex);
    ret =  *i2s_contex->dev->result ? 0 : -1;
    mutex_release(&i2s_contex->mutex);

    return ret;
}

static int slt_i2s_internal_ip_diagnose(i2s_contex_t *i2s_contex)
{
    int ret = -1;
    struct dma_desc *desc_tx = hal_i2s_trigger_dma_tr_start(i2s_contex->dev->handle,
                               i2s_contex->dev->dmaid,
                               wav_data,
                               sizeof wav_data, i2s_contex->dev->cbk,
                               SD_AUDIO_PCM_TRIGGER_PLAYBACK_START);

    if (desc_tx == NULL) {
        dprintf(ALWAYS, "hal i2s%d trigger dma transmit start fail\n",
                i2s_contex->dev->channel);
        goto out;
    }

    hal_dma_sync_wait(desc_tx, -1);
    hal_dma_free_desc(desc_tx);
    ret = 1;
out:

    return ret;
}

int TEST_SAFE_SS_10(uint times, uint timeout, char *result_string)
{
    int ret = -1;
    i2s_contex_t i2s_contex;
    i2s_device_t i2s_dev[I2S_MAX] = {
        [0] = {
            .result = &i2s1_result,
            .channel = 1,
            .resid = I2S1_REG,
            .dmaid = I2S1_DMA,
            .cbk = slt_i2s1_ovf_cbk,
        },
        [1] = {
            .result = &i2s2_result,
            .channel = 2,
            .resid = I2S2_REG,
            .dmaid = I2S2_DMA,
            .cbk = slt_i2s2_ovf_cbk,
        }
    };

    mutex_init(&i2s_contex.mutex);

    for (uint8_t idx = 0; idx < I2S_MAX; idx++) {
        if (!hal_sd_i2s_sc_create_handle(&i2s_dev[idx].handle, i2s_dev[idx].resid)) {
            printf("i2s%d sc create_handle failed.\n", idx);
            goto out;
        }

        pcm_params_t pcm_info;
        pcm_info.mode = SD_AUDIO_TRANSFER_CODEC_MASTER |
                        SD_AUDIO_DIR_MODE_TRANSMIT |
                        SD_AUDIO_CHANNEL_NUM_TX_STEREO | SD_AUDIO_TRANSFER_WITH_DMA;
        pcm_info.sample_rate = SD_AUDIO_SR_48000;
        pcm_info.resolution = SD_AUDIO_SAMPLE_WIDTH_16BITS;
        pcm_info.standard = SD_AUDIO_I2S_STANDARD_PHILLIPS;
        pcm_info.slot_width = SD_AUDIO_SLOT_WIDTH_32BITS;

        hal_sd_i2s_sc_start_up(i2s_dev[idx].handle, pcm_info);
        hal_sd_i2s_sc_set_format(i2s_dev[idx].handle);
        hal_sd_i2s_sc_set_hw_params(i2s_dev[idx].handle);
    }

    for (uint8_t idx = 0; idx < I2S_MAX; idx++) {
        i2s_contex.dev = &i2s_dev[idx];

        if (slt_i2s_internal_ip_diagnose(&i2s_contex) < 0) {
            dprintf(ALWAYS, "slt i2s%d ip diagnose fail\n", i2s_contex.dev->channel);
            goto out;
        }
    }

    for (uint8_t idx = 0; idx < I2S_MAX; idx++) {
        i2s_contex.dev = &i2s_dev[idx];

        if (slt_i2s_checkout_result(&i2s_contex) < 0) {
            dprintf(ALWAYS, "slt i2s%d checkout result fail\n", i2s_contex.dev->channel);
            goto out;
        }
    }

    for (uint8_t idx = 0; idx < I2S_MAX; idx++) {
        hal_sd_i2s_sc_trigger(i2s_dev[idx].handle, SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP);
        hal_sd_i2s_sc_shutdown(i2s_dev[idx].handle);
        hal_sd_i2s_sc_release_handle(i2s_dev[idx].handle);
        hal_i2c_release_handle(i2s_dev[idx].handle);
    }

    ret = 0;
out:
    i2s1_result = false;
    i2s2_result = false;
    return ret;
}

SLT_MODULE_TEST_HOOK_DETAIL(safe_ss_10, TEST_SAFE_SS_10,
                            SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 500, 0, 0);

