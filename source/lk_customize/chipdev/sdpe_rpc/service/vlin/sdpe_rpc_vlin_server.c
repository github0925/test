/*
 * sdpe_rpc_vlin_server.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <kernel/event.h>
#include <kernel/thread.h>
#include <string.h>

#include "sdpe_rpc_cfg.h"
#include "sdpe_rpc_framework.h"
#include "sdpe_rpc_mbuf.h"
#include "sdpe_rpc_vlin.h"
#include "sdpe_ctrl_service.h"
#include "vlin_service.h"

static struct vlin_instance g_vlin_server;

static int vlin_init_handle(void *arg, uint8_t *data, uint32_t len);
static int vlin_send_frame_handle(void *arg, uint8_t *data, uint32_t len);
static int vlin_check_wakeup_handle(void *arg, uint8_t *data, uint32_t len);
static int vlin_goto_sleep_handle(void *arg, uint8_t *data, uint32_t len);
static int vlin_goto_sleep_internal_handle(void *arg, uint8_t *data, uint32_t len);
static int vlin_wakeup_handle(void *arg, uint8_t *data, uint32_t len);
static int vlin_wakeup_internal_handle(void *arg, uint8_t *data, uint32_t len);
static int vlin_get_status_handle(void *arg, uint8_t *data, uint32_t len);

extern Std_ReturnType virLinIf_SendFrame(uint8 Channel, const Lin_PduType *PduInfoPtr);

static const sdpe_rpc_usr_hdl g_vlin_server_handler[] =
{
    [VLIN_INIT]                 = vlin_init_handle,
    [VLIN_SEND_FRAME]           = vlin_send_frame_handle,
    [VLIN_CHECK_WAKEUP]         = vlin_check_wakeup_handle,
    [VLIN_GOTO_SLEEP]           = vlin_goto_sleep_handle,
    [VLIN_GOTO_SLEEP_INTERNAL]  = vlin_goto_sleep_internal_handle,
    [VLIN_WAKEUP]               = vlin_wakeup_handle,
    [VLIN_WAKEUP_INTERNAL]      = vlin_wakeup_internal_handle,
    [VLIN_GET_STATUS]           = vlin_get_status_handle,
};

#ifdef SUPPORT_SDPE_RPC_MBUF
static void vlin_deconstruct_init_msg(Lin_ConfigType *config)
{
    if (config->Config != NULL) {
        sdpe_rpc_free_mbuf(config->Config);
    }
}
#endif

static int vlin_init_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vlin_init_s *init = (struct vlin_init_s*)data;
    struct vlin_init_req *req = &init->req;
    struct vlin_init_resp *resp = &init->resp;

    virLin_Init(&req->config);
    resp->result = 0;
    return 0;
}

static int vlin_check_wakeup_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vlin_check_wakeup_s *check_wakeup = (struct vlin_check_wakeup_s*)data;
    struct vlin_check_wakeup_req *req = &check_wakeup->req;
    struct vlin_check_wakeup_resp *resp = &check_wakeup->resp;

    resp->result = virLin_CheckWakeup(req->channel);
    return 0;
}

static int vlin_goto_sleep_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vlin_goto_sleep_s *goto_sleep = (struct vlin_goto_sleep_s*)data;
    struct vlin_goto_sleep_req *req = &goto_sleep->req;
    struct vlin_goto_sleep_resp *resp = &goto_sleep->resp;

    resp->result = virLin_GoToSleep(req->channel);
    return 0;
}

static int vlin_goto_sleep_internal_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vlin_goto_sleep_internal_s *goto_sleep = (struct vlin_goto_sleep_internal_s*)data;
    struct vlin_goto_sleep_internal_req *req = &goto_sleep->req;
    struct vlin_goto_sleep_internal_resp *resp = &goto_sleep->resp;

    resp->result = virLin_GoToSleepInternal(req->channel);
    return 0;
}

static int vlin_wakeup_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vlin_wakeup_s *wakeup = (struct vlin_wakeup_s*)data;
    struct vlin_wakeup_req *req = &wakeup->req;
    struct vlin_wakeup_resp *resp = &wakeup->resp;

    resp->result = virLin_Wakeup(req->channel);
    return 0;
}

static int vlin_wakeup_internal_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vlin_wakeup_internal_s *wakeup = (struct vlin_wakeup_internal_s*)data;
    struct vlin_wakeup_internal_req *req = &wakeup->req;
    struct vlin_wakeup_internal_resp *resp = &wakeup->resp;

    resp->result = virLin_WakeupInternal(req->channel);
    return 0;
}

static int vlin_get_status_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vlin_get_status_s *get_status = (struct vlin_get_status_s*)data;
    struct vlin_get_status_req *req = &get_status->req;
    struct vlin_get_status_resp *resp = &get_status->resp;

    resp->result = virLin_GetStatus(req->channel, resp->pdu);
    return 0;
}

static int vlin_send_frame_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vlin_send_frame_s *send_frame = (struct vlin_send_frame_s*)data;
    struct vlin_send_frame_req *req = &send_frame->req;
    struct vlin_send_frame_resp *resp = &send_frame->resp;
    Lin_PduType pdu;

#if (SDPE_RPC_DEBUG <= LK_DEBUGLEVEL)
    dprintf(SDPE_RPC_DEBUG,
            "%s() channel:%d, Pid: %d, Cs: %d, Drc: %d, Dl: %d, Sdu: ",
            __func__, req->channel, req->pid, req->cs,
            req->drc, req->dl);

    for (uint32_t i = 0; i < req->dl; i++) {
        dprintf(SDPE_RPC_DEBUG, "%d ", req->data[i]);
    }
    dprintf(SDPE_RPC_DEBUG, "\n");
#endif

    pdu.Pid = req->pid;
    pdu.Cs = req->cs;
    pdu.Drc = req->drc;
    pdu.Dl = req->dl;
    pdu.SduPtr = req->data;

    resp->result = virLin_SendFrame(req->channel, &pdu);
    return 0;
}

static int vlin_rpc_cb(void *arg, uint16_t msg_id, uint8_t *data, uint32_t len)
{
    uint8_t *message_data = (uint8_t *)(*(uint32_t*)data);
    uint8_t service_id = SDPE_RPC_GET_SERVICE_ID(msg_id);
    uint8_t sub_msg_id = SDPE_RPC_GET_MSG_ID(msg_id);

    ASSERT(service_id == SDPE_RPC_VLIN_SERVICE);

    if (sub_msg_id > VLIN_MSG_START && sub_msg_id < VLIN_MSG_END) {
        dprintf(SDPE_RPC_DEBUG, "vlin server recv msg:%d\n", sub_msg_id);
        if (g_vlin_server_handler[sub_msg_id])
            return g_vlin_server_handler[sub_msg_id](arg, message_data, len);
    }
    dprintf(ALWAYS, "vlin server recv wrong msg:%d\n", msg_id);
    return -1;
}

int vlin_service_init(void)
{
#ifdef SUPPORT_SDPE_RPC_MULTI_TASK
    const struct sdpe_rpc_config *cfg;
    int ret;

    cfg = sdpe_rpc_find_cfg(SDPE_RPC_VLIN_SERVICE);
    if (cfg == NULL) {
        dprintf(ALWAYS, "rpc get cfg fail\n");
        return -1;
    }

    ret = sdpe_rpc_dev_init(&g_vlin_server.priv, "VLIN_SERVER",
                            cfg->client_id, cfg->local_addr,
                            vlin_rpc_cb, NULL,
                            SDPE_RPC_MBOX_TRNASPORT,
                            32, true, false);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc dev init fail\n");
        return ret;
    }

    ret = sdpe_rpc_dev_open(&g_vlin_server.priv);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc dev open fail\n");
        return ret;
    }

    g_vlin_server.rpc_dev = &g_vlin_server.priv;

    return ret;
#else
    sdpe_service_cb_register(SDPE_RPC_VLIN_SERVICE, vlin_rpc_cb);
    g_vlin_server.rpc_dev = sdpe_service_get_rpc_dev();
    return 0;
#endif
}

