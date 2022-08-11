/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */
#include <app.h>
#include <arch.h>
#include <dcf.h>
#include <dcf_common.h>
#include <debug.h>
#include <err.h>
#include <heap.h>
#include <lib/bytes.h>
#include <lib/console.h>
#include <lib/reg.h>
#include <platform.h>
#include <platform/debug.h>
#include <res_loader.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "audio_rpc_svc.h"

#include "am_api.h"
#include "chip_res.h"
#include "dma_hal.h"
#include "i2s_hal.h"
#include "ipcc_rpc.h"
#include "rpmsg_env.h"
#include "worker.h"
#if ENABLE_SD_DMA
#include "dma_hal.h"
#endif
#include "res.h"

#define AUDIO_RPC_SERVICE_LOG CRITICAL
#define GET_ENUM_NAME(x) (#x)

static struct audio_server g_audio_server;

void start_worker(void *pargs, uint32_t path)
{
    struct audio_rpc_cmd *ctl = (struct audio_rpc_cmd *)pargs;
    dprintf(AUDIO_RPC_SERVICE_LOG, "%s , path_name: %d, path_vol: %d\n",
            __func__, ctl->msg.recv_msg.val0, ctl->msg.recv_msg.val1);
    start_path(ctl->msg.recv_msg.val0, ctl->msg.recv_msg.val1);
}

void stop_worker(void *pargs, uint32_t path) {  stop_path(path); }

void set_vol_worker(void *pargs, uint32_t path)
{
    struct audio_rpc_cmd *ctl = (struct audio_rpc_cmd *)pargs;
    set_path_vol(ctl->msg.recv_msg.val0, ctl->msg.recv_msg.val1);
}

void mute_worker(void *pargs, uint32_t path)
{
    struct audio_rpc_cmd *ctl = (struct audio_rpc_cmd *)pargs;
    mute_path(ctl->msg.recv_msg.val0, ctl->msg.recv_msg.val1);
}

void audio_play_worker(void *pargs, uint32_t enable)
{
#if ENABLE_AUDIO_AGENT
    struct audio_rpc_cmd *ctl = (struct audio_rpc_cmd *)pargs;
    uint32_t res_id = ctl->msg.recv_msg.val0;
    uint32_t prop = ctl->msg.recv_msg.val1;
    au_agent_params_t agent_params;
    dprintf(AUDIO_RPC_SERVICE_LOG, "%s , res_id: %d, prop: %d, play: %d\n",
            __func__, res_id, prop, enable);
    if (enable) {
        au_agent_open(&g_audio_server.audio_handle, 0);
        agent_params.op_code = AU_AGENT_OP_START;
        if (res_id == 0) {
            strncpy(agent_params.path, TU_PATH, AU_AGENT_PATH_SIZE);
        } else {
            strncpy(agent_params.path, WA_PATH, AU_AGENT_PATH_SIZE);
        }
        agent_params.priority = 1;
        au_agent_operation(g_audio_server.audio_handle, &agent_params);
    } else {
        agent_params.op_code = AU_AGENT_OP_STOP;
        au_agent_operation(g_audio_server.audio_handle, &agent_params);
        au_agent_close(&g_audio_server.audio_handle);
    }
#endif
}

static rpc_call_result_t audio_server_callback(rpc_server_handle_t hserver,
                                               rpc_call_request_t *request)
{
    rpc_call_result_t result = {
        0,
    };
    struct audio_rpc_cmd *ctl = (struct audio_rpc_cmd *)&request->param[0];
    struct audio_rpc_result *r = (struct audio_rpc_result *)&result.result[0];
    struct audio_server *server;

    result.ack = request->cmd;
    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        return result;
    }

    server = &g_audio_server;
    mutex_acquire(&server->mutex);

    switch (ctl->op) {
    case OP_PCM_CAPTURE_CTL:
        dprintf(AUDIO_RPC_SERVICE_LOG, "rpc cmd from audio service: %s.\n",
                GET_ENUM_NAME(I2S_SVR_OP_PCM_PLAYBACK_CTL));
        r->msg.data[0] = 1;
        r->msg.data[1] = 2;
        r->msg.data[2] = 3;
        r->msg.data[3] = 4;
        break;
    case OP_START:
        dprintf(AUDIO_RPC_SERVICE_LOG,
                "rpc cmd from audio service: %s. val0: %d , vol: %d\n",
                GET_ENUM_NAME(OP_START), ctl->msg.recv_msg.val0,
                ctl->msg.recv_msg.val1);
        call_worker(start_worker, ctl, 0);
        break;
    case OP_STOP:
        dprintf(AUDIO_RPC_SERVICE_LOG, "rpc cmd from audio service: %s, val0: %d \n",
                GET_ENUM_NAME(OP_STOP), ctl->msg.recv_msg.val0);
        call_worker(stop_worker, NULL, ctl->msg.recv_msg.val0);
        break;
    case OP_SETVOL:
        dprintf(AUDIO_RPC_SERVICE_LOG,
                "rpc cmd from audio service: %s, val0: %d, val1: %d\n",
                GET_ENUM_NAME(OP_SETVOL), ctl->msg.recv_msg.val0,
                ctl->msg.recv_msg.val1);

        call_worker(set_vol_worker, ctl, 0);
        break;
    case OP_MUTE:
        dprintf(AUDIO_RPC_SERVICE_LOG, "rpc cmd from audio service: %s.\n",
                GET_ENUM_NAME(OP_MUTE));
        call_worker(mute_worker, ctl, 0);
        break;
    case OP_SWITCH:
        dprintf(AUDIO_RPC_SERVICE_LOG, "rpc cmd from audio service: %s.\n",
                GET_ENUM_NAME(OP_SWITCH));
        switch_path(ctl->msg.recv_msg.val0, ctl->msg.recv_msg.val1);
        break;
    case OP_PLAY_AGENT:
        dprintf(
            AUDIO_RPC_SERVICE_LOG,
            "rpc cmd from audio service: %s.data0: %d, val0: %d, val1: %d\n",
            GET_ENUM_NAME(OP_PLAY_AGENT), ctl->msg.recv_msg.val0,
            ctl->msg.recv_msg.val1);
        if (ctl->msg.recv_msg.val1 == 0) // start playback
        {
            call_worker(audio_play_worker, ctl, 1);
        } else { // stop playback
            call_worker(audio_play_worker, ctl, 0);
        }
        break;
    default:
        dprintf(AUDIO_RPC_SERVICE_LOG, "unknown cmd: (%d)\n", ctl->op);
        break;
    }
    // TODO: Use semaphore implement synchronous calls
    result.retcode = 0;
    mutex_release(&server->mutex);
    dprintf(AUDIO_RPC_SERVICE_LOG, "<%s> exited.\n", __func__);
    return result;
}

int safety_audio_notify_agent(u8 *data, u16 len)
{
    return dcf_do_notify(g_audio_server.notifier, data, len);
}

int audio_svr_add_notifier(struct audio_server *server)
{
    dprintf(AUDIO_RPC_SERVICE_LOG, "<%s>audio server add notifier\n", __func__);
    struct dcf_notifier_attribute attr;
    dcf_notify_attr_init(&attr, server->rproc, server->mbox_addr);
    server->notifier = dcf_create_notifier(&attr);

    if (!server->notifier) {
        dprintf(AUDIO_RPC_SERVICE_LOG,
                "<%s>audio server create notify failed.\n", __func__);
        return -1;
    }

    return 0;
}

void audio_rpc_service_init(void)
{
    dprintf(AUDIO_RPC_SERVICE_LOG, "<%s>start audio rpc service init\n",
            __func__);
    struct audio_server *server;

    rpc_server_impl_t devfunc[] = {
        {MOD_RPC_REQ_AUDIO_SERVICE, audio_server_callback, IPCC_RPC_NO_FLAGS},
    };
    server = &g_audio_server;

    if (server->initialized)
        return;

    server->rproc = DP_CA_AP1;
    server->mbox_addr = AUDIO_SERVICE_MBOX_ADDR;

    mutex_init(&server->mutex);
    audio_svr_add_notifier(server);

    dprintf(AUDIO_RPC_SERVICE_LOG, "<%s> start audio rpc service.\n", __func__);
    start_ipcc_rpc_service(devfunc, ARRAY_SIZE(devfunc));
    server->initialized = true;
    start_worker_service();
    dprintf(AUDIO_RPC_SERVICE_LOG, "SUCCESS\n");
}

void audio_rpc_service_entry(const struct app_descriptor *app, void *args)
{
    audio_rpc_service_init();
}

/* test notify func: send designated value. */
void audio_server_send_notify(const struct app_descriptor *app, void *args)
{
    u8 data[10] = {10, 9, 8, 7, 6};
    u16 len = 10;

    /*print notify content */
    dprintf(AUDIO_RPC_SERVICE_LOG,
            "audio server send data length: %d to android\n", len);
    safety_audio_notify_agent(data, len);
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
/* STATIC_COMMAND("au_svr", "test audio server",
               (console_cmd)&i2s_rpc_service_init) */
STATIC_COMMAND("send_nf", "send audio server notify to android",
               (console_cmd)&audio_server_send_notify)
STATIC_COMMAND_END(audio_service_test);
#endif

APP_START(i2s_rpc).entry = (app_entry)audio_rpc_service_entry,
    .stack_size = 8192, APP_END
