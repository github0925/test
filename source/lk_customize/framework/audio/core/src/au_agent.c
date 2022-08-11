/*
* Copyright (c) 2021 Semidrive Semiconductor, Inc.
* audio agent for remote
*/

//#include "animation_config.h"
#include "chip_res.h"
#include "dma_hal.h"
#include <lk_wrapper.h>
#include <app.h>
#include <container.h>
#include <debug.h>
#include <heap.h>
#include <lib/console.h>
#include <platform.h>
#include <platform/debug.h>
#include <res_loader.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boardinfo_hwid_usr.h"
#include "au_agent.h"
#include "sd_audio.h"
#include "dma_hal.h"
#include "i2s_hal.h"



#define DMA_MAX (65536)
#define DMA_PERIOD_LEN (1200)
#define I2S_RES RES_I2S_SC_I2S_SC1
#define I2S_DMA_CHAN DMA_PERI_I2S_SC1
#define AGENT_TIMEOUT 1000
#define AM_AGENT_DBG_PRT 2

static au_agent_t au_agent;

static int au_agent_do_action(au_agent_t *agent);

void audio_agent_task(token_handle_t token)
{
    // struct sd_hwid_usr hwid = get_fullhw_id();
    au_agent_t *agent = (au_agent_t *)token;
    dprintf(AM_AGENT_DBG_PRT, "%s: ####audio agent running #####\n", __func__);
    for (;;) {
        event_wait(&agent->agent_start_event);
        dprintf(AM_AGENT_DBG_PRT, "%s: audioagent rec op_code =%x\n", __func__,
                agent->op_code);
        au_agent_do_action(agent);
        event_signal(&agent->agent_done_event, false);
    }
}

bool au_agent_init(void)
{
    bool ret = true;
    au_agent_t *agent = &au_agent;
    memset(agent, 0, sizeof(au_agent_t));
    event_init(&agent->agent_start_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&agent->agent_done_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    agent->status = AU_AGENT_ST_IDLE;
    agent->pcm_data = NULL;
    agent->i2s_handle = NULL;
    agent->thread =
        thread_create("audio_agent", (thread_start_routine)audio_agent_task,
                      (void *)agent, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    if (agent->thread) {
        thread_detach_and_resume(agent->thread);
    } else {
        ret = false;
        dprintf(0, "%s: ####agent task creat faild #####\n", __func__);
    }
    return ret;
}

bool au_agent_open(au_agent_handle_t *handle, u_int32_t res_id)
{
    bool ret = true;
    au_agent_t *agent = &au_agent;
    *handle = (au_agent_handle_t)agent;
    return true;
}
bool au_agent_close(au_agent_handle_t handle)
{
    handle = NULL;
    return true;
}

bool au_agent_operation(au_agent_handle_t handle,
                        au_agent_params_t *agent_params)
{
    int ret = 0;
    bool result = true;
    if ((!handle) || (!agent_params)) {
        dprintf(0, "%s: error handle or agent_params is null\n", __func__);
        result = false;
        return result;
    }
    au_agent_t *agent = (au_agent_t *)handle;
    agent->op_code = agent_params->op_code;
    agent->params = agent_params;
    event_signal(&agent->agent_start_event, false);
    ret = event_wait_timeout(&agent->agent_done_event, AGENT_TIMEOUT);
    if (ERR_TIMED_OUT == ret) {
        dprintf(0, "%s: agent timeout error\n", __func__);
        result = false;
    }
    if (agent->result_error) {
        dprintf(0, "%s: error:result_error=%d\n", __func__,
                agent->result_error);
        result = false;
    }
    return result;
}

static void agent_playback_irq_evt_handle(enum dma_status status, u32 err,
                                          void *context)
{
    dprintf(AM_AGENT_DBG_PRT,
            "dma irq evt: status(%d) err(0x%x) context(%p) \n", status, err,
            context);
    au_agent_t *agent = (au_agent_t *)context;
    if (DMA_COMP != status) {
        /* terminate this channel after error. */
        hal_dma_terminate(agent->desc_tx);
    }
}

static int pcm_prepare(au_agent_t *agent)
{
    int ret = 0;
    int size = 0;
    au_agent_params_t *agent_params;
    dprintf(AM_AGENT_DBG_PRT, "%s:\n", __func__);
    if (AU_AGENT_ST_IDLE != agent->status) {
        dprintf(0, "%s: error:agent need be IDLE,shuold do stop it  now=%d\n",
                __func__, agent->status);
        ret = -1;
        return ret;
    }
    agent_params = agent->params;
    strncpy(agent->path, agent_params->path, AU_AGENT_PATH_SIZE);
    agent->priority = agent_params->priority;
    size = res_size(agent->path);
    if (size < 0) {
        ret = -1;
        dprintf(0, "%s: error:agent->pcm_data is NULL \n", __func__);
        return ret;
    }
    agent->pcm_size = ROUNDUP(size, 32);
    agent->pcm_data = (uint8_t *)memalign(32, agent->pcm_size);
    dprintf(0, "%s: error:agent->pcm_size %d \n", __func__, agent->pcm_size);
    if (!agent->pcm_data) {
        ret = -1;
        dprintf(0, "%s: error:agent->pcm_data is NULL \n", __func__);
        return ret;
    }
    ret = res_load(agent->path, agent->pcm_data, agent->pcm_size, 0);
    if (ret < 0) {
        dprintf(0, "can't load audio res!\n");
        free(agent->pcm_data);
        agent->pcm_data = NULL;
        ret = -1;
        return ret;
    }
    agent->status = AU_AGENT_ST_PREPARE;
    return ret;
}
static int pcm_realse(au_agent_t *agent)
{
    int ret = 0;
    dprintf(AM_AGENT_DBG_PRT, "%s:\n", __func__);
    if (AU_AGENT_ST_STOP != agent->status) {
        dprintf(0, "%s: error:agent need be in AGENT_ST_STOP  now=%d\n",
                __func__, agent->status);
        ret = -1;
        return ret;
    }
    if (agent->pcm_data) {
        free(agent->pcm_data);
    }
    agent->pcm_data = NULL;
    agent->status = AU_AGENT_ST_IDLE;
    return ret;
}
static int pcm_reset(au_agent_t *agent)
{
    int ret = 0;
    dprintf(AM_AGENT_DBG_PRT, "%s:\n", __func__);
    if (agent->pcm_data) {
        free(agent->pcm_data);
    }
    agent->pcm_data = NULL;
    agent->status = AU_AGENT_ST_IDLE;
    return ret;
}
au_agent_status pcm_getstatus(void)
{
    au_agent_t *agent = &au_agent;
    return agent->status;
}

static int pcm_start(au_agent_t *agent)
{
    int ret = 0;
    pcm_params_t pcm_info;
    dprintf(AM_AGENT_DBG_PRT, "%s:\n", __func__);
    if (AU_AGENT_ST_PREPARE != agent->status) {
        dprintf(0, "%s: error:agent need be in AGENT_ST_PREPARE  now is =%d\n",
                __func__, agent->status);
        ret = -1;
        return ret;
    }
    if (!hal_sd_i2s_sc_create_handle(&agent->i2s_handle, RES_I2S_SC_I2S_SC1)) {
        dprintf(0, "error:i2s resource get failed.\n");
        ret = -1;
        goto ERR_HANDLE2;
    }
    pcm_info.mode = SD_AUDIO_TRANSFER_CODEC_MASTER |
                    SD_AUDIO_DIR_MODE_TRANSMIT |
                    SD_AUDIO_CHANNEL_NUM_TX_STEREO | SD_AUDIO_TRANSFER_WITH_DMA;
    pcm_info.sample_rate = SD_AUDIO_SR_48000;
    pcm_info.resolution = SD_AUDIO_SAMPLE_WIDTH_16BITS;
    pcm_info.standard = SD_AUDIO_I2S_STANDARD_PHILLIPS;
    pcm_info.slot_width = SD_AUDIO_SLOT_WIDTH_32BITS;
    hal_sd_i2s_sc_start_up(agent->i2s_handle, pcm_info);
    hal_sd_i2s_sc_set_format(agent->i2s_handle);
    hal_sd_i2s_sc_set_hw_params(agent->i2s_handle);
    // config dma channel
    struct dma_dev_cfg dma_cfg;
    dma_cfg.direction = DMA_MEM2DEV;
    dma_cfg.src_addr = 0;
    dma_cfg.dst_addr = hal_sd_i2s_sc_get_fifo_addr(agent->i2s_handle);
    dma_cfg.src_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
    dma_cfg.dst_addr_width = DMA_DEV_BUSWIDTH_2_BYTES;
    dma_cfg.src_maxburst = DMA_BURST_TR_32ITEMS;
    dma_cfg.dst_maxburst = DMA_BURST_TR_32ITEMS;
    agent->dma_chan = hal_dma_chan_req(DMA_PERI_I2S_SC1);
    hal_dma_dev_config(agent->dma_chan, &dma_cfg);
    if (agent->pcm_size <= DMA_MAX) {
        agent->period_count = 2;
        agent->period_len = agent->pcm_size / 2;
    } else {
        agent->period_count = agent->pcm_size / DMA_PERIOD_LEN;
        // drop some data at tail.
        /*         if (agent->pcm_size % DMA_PERIOD_LEN) {
                    agent->period_count++;
                } */
        agent->period_len = DMA_PERIOD_LEN;
    }
    agent->desc_tx =
        hal_prep_dma_cyclic(agent->dma_chan, (void *)agent->pcm_data,
                            agent->period_count * agent->period_len,
                            agent->period_len, DMA_INTERRUPT);
    if (agent->desc_tx == NULL) {
        ret = -1;
        dprintf(0, "error:hal_i2s_trigger_dma_tr_start failed.\n");
        goto ERR_HANDLE1;
    }
    agent->desc_tx->dmac_irq_evt_handle = agent_playback_irq_evt_handle;
    agent->desc_tx->context = (void *)agent;
    hal_sd_i2s_sc_trigger(agent->i2s_handle,
                          SD_AUDIO_PCM_TRIGGER_PLAYBACK_START);
    hal_dma_submit(agent->desc_tx);
    agent->status = AU_AGENT_ST_RUNNING;
    return ret;
ERR_HANDLE1:
    if (agent->i2s_handle) {
        hal_sd_i2s_sc_trigger(agent->i2s_handle,
                              SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP);
        hal_sd_i2s_sc_shutdown(agent->i2s_handle);
        hal_sd_i2s_sc_release_handle(agent->i2s_handle);
    }
    if (agent->desc_tx) {
        hal_dma_free_desc(agent->desc_tx);
        agent->desc_tx = NULL;
    }
ERR_HANDLE2:
    pcm_reset(agent);
    return ret;
}
static int pcm_stop(au_agent_t *agent)
{
    int ret = 0;
    dprintf(AM_AGENT_DBG_PRT, "%s:\n", __func__);
    if (AU_AGENT_ST_RUNNING != agent->status) {
        dprintf(0, "%s: error:agent need be in AGENT_ST_RUNNING  now=%d\n",
                __func__, agent->status);
        ret = -1;
        return ret;
    }
    if (agent->i2s_handle)
        hal_sd_i2s_sc_trigger(agent->i2s_handle,
                              SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP);
    if (agent->desc_tx) {
        hal_dma_terminate(agent->desc_tx);
        hal_dma_free_desc(agent->desc_tx);
    }
    if (agent->i2s_handle) {
        hal_sd_i2s_sc_shutdown(agent->i2s_handle);
        hal_sd_i2s_sc_release_handle(agent->i2s_handle);
    }
    agent->i2s_handle = NULL;
    agent->desc_tx = NULL;
    agent->status = AU_AGENT_ST_STOP;
    return ret;
}

static int au_agent_do_action(au_agent_t *agent)
{
    int ret = 0;
    dprintf(AM_AGENT_DBG_PRT, "%s:\n", __func__);
    switch (agent->op_code) {
    case AU_AGENT_OP_START:
        ret = pcm_prepare(agent);
        ret = pcm_start(agent);
        break;
    case AU_AGENT_OP_STOP:
        ret = pcm_stop(agent);
        ret = pcm_realse(agent);
        break;
    default:
        dprintf(0, "%s: error: not support op_code=%d\n", __func__,
                agent->op_code);
        ret = -1;
    }
    agent->params = NULL;
    agent->result_error = ret;
    return ret;
}
