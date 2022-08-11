/*
 * sdpe_rpc_vlin_client.c
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

static struct vlin_instance g_vlin_client;

static int vlin_rpc_cb(void *arg, uint16_t msg_id, uint8_t *data, uint32_t len)
{
    dprintf(ALWAYS, "vlin client recv wrong msg:%d\n", msg_id);
    return -1;
}

static int vlin_construct_init_msg(Lin_ConfigType *new_config,
                                   const Lin_ConfigType *config,
                                   uint8_t *config_buf)
{
    uint32_t size;
    uint8_t *addr = config_buf;

    if (config->Count > 0) {
        size = sizeof(Lin_ControllerConfigType)*config->Count;
        if (((addr - config_buf) + size) > SDPE_RPC_VLIN_CFG_LEN) {
            return -1;
        }
        SDPE_RPC_COPY_MSG_DATA(Config, size, addr);
    }
    return 0;
}

int virLin_Init(const Lin_ConfigType *config)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_init_s *vlin_init;
    struct vlin_init_req *req;
    struct vlin_init_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    vlin_init = SDPE_RPC_GET_SHARE_BUF(struct vlin_init_s);

    req = &vlin_init->req;
    req->config.Count = config->Count;
    if (vlin_construct_init_msg(&req->config, config, req->config_buf) < 0) {
        dprintf(ALWAYS, "construct vlin init msg fail, no enough memory\n");
        return -1;
    }

    resp = &vlin_init->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_INIT);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&vlin_init, sizeof(uint32_t));

    return resp->result;
}

int virLin_CheckWakeup(uint8_t channel)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_check_wakeup_s *check_wakeup;
    struct vlin_check_wakeup_req *req;
    struct vlin_check_wakeup_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    check_wakeup = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_check_wakeup_s,
                                                SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &check_wakeup->req;
    req->channel = channel;

    resp = &check_wakeup->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_CHECK_WAKEUP);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&check_wakeup, sizeof(uint32_t));

    return resp->result;
}

int virLin_GoToSleep(uint8_t channel)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_goto_sleep_s *goto_sleep;
    struct vlin_goto_sleep_req *req;
    struct vlin_goto_sleep_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    goto_sleep = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_goto_sleep_s,
                                              SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &goto_sleep->req;
    req->channel = channel;

    resp = &goto_sleep->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_GOTO_SLEEP);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&goto_sleep, sizeof(uint32_t));

    return resp->result;
}

int virLin_GoToSleepInternal(uint8_t channel)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_goto_sleep_internal_s *goto_sleep;
    struct vlin_goto_sleep_internal_req *req;
    struct vlin_goto_sleep_internal_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    goto_sleep = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_goto_sleep_internal_s,
                                              SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &goto_sleep->req;
    req->channel = channel;

    resp = &goto_sleep->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_GOTO_SLEEP_INTERNAL);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&goto_sleep, sizeof(uint32_t));

    return resp->result;
}

int virLin_Wakeup(uint8_t channel)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_wakeup_s *wakeup;
    struct vlin_wakeup_req *req;
    struct vlin_wakeup_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    wakeup = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_wakeup_s,
                                          SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &wakeup->req;
    req->channel = channel;

    resp = &wakeup->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_WAKEUP);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&wakeup, sizeof(uint32_t));

    return resp->result;
}

int virLin_WakeupInternal(uint8_t channel)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_wakeup_internal_s *wakeup;
    struct vlin_wakeup_internal_req *req;
    struct vlin_wakeup_internal_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    wakeup = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_wakeup_internal_s,
                                          SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &wakeup->req;
    req->channel = channel;

    resp = &wakeup->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_WAKEUP_INTERNAL);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&wakeup, sizeof(uint32_t));

    return resp->result;
}

int virLin_GetStatus(uint8_t channel, uint8_t *lin_sdu)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_get_status_s *get_status;
    struct vlin_get_status_req *req;
    struct vlin_get_status_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    get_status = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_get_status_s,
                                              SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &get_status->req;
    req->channel = channel;

    resp = &get_status->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_GET_STATUS);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&get_status, sizeof(uint32_t));

    memcpy(lin_sdu, resp->pdu, SDPE_RPC_VLIN_PDU_LEN);
    return resp->result;
}

int virLin_SendFrame(uint8_t channel,
                     const Lin_PduType *pdu)
{
    struct sdpe_rpc_dev *rpc_dev = g_vlin_client.rpc_dev;
    struct vlin_send_frame_s *send_frame;
    struct vlin_send_frame_req *req;
    struct vlin_send_frame_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    send_frame = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vlin_send_frame_s,
                                              SDPE_RPC_MAX_VLIN_NUM, channel);

    req = &send_frame->req;
    req->channel = channel;
    req->pid = pdu->Pid;
    req->cs = pdu->Cs;
    req->drc = pdu->Drc;
    req->dl = pdu->Dl;
    memcpy(req->data, pdu->SduPtr, pdu->Dl);

    resp = &send_frame->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VLIN_GET_MSGID(VLIN_SEND_FRAME);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&send_frame, sizeof(uint32_t));

    return resp->result;
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

    ret = sdpe_rpc_dev_init(&g_vcan_client.priv, "VLIN_CLIENT",
                            cfg->server_id, cfg->local_addr,
                            vlin_rpc_cb, NULL,
                            SDPE_RPC_MBOX_TRNASPORT,
                            32, true, false);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc dev init fail\n");
        return ret;
    }

    ret = sdpe_rpc_dev_open(&g_vcan_client.priv);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc dev open fail\n");
        return ret;
    }

    g_vlin_client.rpc_dev = &g_vlin_client.priv;

    return ret;
#else
    sdpe_service_cb_register(SDPE_RPC_VLIN_SERVICE, vlin_rpc_cb);
    g_vlin_client.rpc_dev = sdpe_service_get_rpc_dev();
    return 0;
#endif
}
