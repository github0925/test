/*
 * sdpe_rpc_ctrl_client.c
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
#include "sdpe_rpc_ctrl.h"
#include "sdpe_ctrl_service.h"

static struct sdpe_ctrl_instance g_sdpe_ctrl_client;

#ifndef SUPPORT_SDPE_RPC_MULTI_TASK
static sdpe_rpc_usr_cb g_sdpe_service_handler[SDPE_RPC_MAX_SERVICE];
#endif

static int sdpe_rpc_sync_cnf_handle(void *arg, uint8_t *data, uint32_t len);
static int sdpe_rpc_event_cb_handle(void *arg, uint8_t *data, uint32_t len);

extern void sdpe_event_cb(uint32_t event_id, uint32_t arg);

static const sdpe_rpc_usr_hdl g_sdpe_ctrl_handler[] =
{
    [SDPE_CTRL_SYNC_CNF] = sdpe_rpc_sync_cnf_handle,
    [SDPE_CTRL_EVENT_CB] = sdpe_rpc_event_cb_handle,
};

static int sdpe_ctrl_rpc_cb(void *arg, uint16_t msg_id, uint8_t *data, uint32_t len)
{
    uint8_t *message_data = (uint8_t *)(*(uint32_t*)data);
    uint8_t service_id = SDPE_RPC_GET_SERVICE_ID(msg_id);
    uint8_t sub_msg_id = SDPE_RPC_GET_MSG_ID(msg_id);

    ASSERT(service_id < SDPE_RPC_MAX_SERVICE);

#ifndef SUPPORT_SDPE_RPC_MULTI_TASK
    if (service_id != SDPE_RPC_SDPE_SERVICE) {
        sdpe_rpc_usr_cb cb = g_sdpe_service_handler[service_id];
        if (cb) {
            return cb(arg, msg_id, data, len);
        }
    }
#endif

    if (sub_msg_id > SDPE_CTRL_MSG_START && sub_msg_id < SDPE_CTRL_MSG_END) {
        if (g_sdpe_ctrl_handler[sub_msg_id])
            return g_sdpe_ctrl_handler[sub_msg_id](arg, message_data, len);
    }
    dprintf(ALWAYS, "sdpe ctrl recv wrong msg:%d\n", msg_id);
    return -1;
}

static int sdpe_rpc_sync_cnf_handle(void *arg, uint8_t *data, uint32_t len)
{
    g_sdpe_ctrl_client.ready = true;
    return 0;
}

static int sdpe_rpc_event_cb_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct sdpe_ctrl_event_cb *event_cb = (struct sdpe_ctrl_event_cb *)data;
    struct sdpe_ctrl_event_cb_req *req = &event_cb->req;
    struct sdpe_ctrl_event_cb_resp *resp = &event_cb->resp;

    sdpe_event_cb(req->event_id, req->arg);
    resp->result = 0;
    return 0;
}

void sdpe_rpc_sync(void)
{
    struct sdpe_rpc_dev *rpc_dev = &g_sdpe_ctrl_client.rpc_dev;
    uint16_t msg_id;

    msg_id = SDPE_RPC_CTRL_GET_MSGID(SDPE_CTRL_SYNC_REQ);

    while (!g_sdpe_ctrl_client.ready) {

        sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_IND,
                                  SDPE_RPC_WAIT_FOREVER,
                                  NULL, 0);

        thread_sleep(1);
    }
}

void sdpe_start_routing(uint32_t route_table, uint32_t size)
{
    struct sdpe_rpc_dev *rpc_dev = &g_sdpe_ctrl_client.rpc_dev;
    struct sdpe_ctrl_start_route *start_route;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return;
    }

    start_route = SDPE_RPC_GET_SHARE_BUF(struct sdpe_ctrl_start_route);
    start_route->req.route_table_addr = route_table;
    start_route->req.route_table_size = size;

    msg_id = SDPE_RPC_CTRL_GET_MSGID(SDPE_CTRL_START_ROUTE);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&start_route, sizeof(uint32_t));
}

void sdpe_stop_routing(void)
{
    struct sdpe_rpc_dev *rpc_dev = &g_sdpe_ctrl_client.rpc_dev;
    struct sdpe_ctrl_stop_route *stop_route;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return;
    }

    stop_route = SDPE_RPC_GET_SHARE_BUF(struct sdpe_ctrl_stop_route);

    msg_id = SDPE_RPC_CTRL_GET_MSGID(SDPE_CTRL_STOP_ROUTE);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&stop_route, sizeof(uint32_t));
}

void sdpe_monitor_event(uint32_t event_id, uint8_t enable,
                        uint32_t arg_len, uint8_t *arg)
{
    struct sdpe_rpc_dev *rpc_dev = &g_sdpe_ctrl_client.rpc_dev;
    struct sdpe_ctrl_set_monitor *set_monitor;
    struct sdpe_ctrl_set_monitor_req *req;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return;
    }

    ASSERT(arg_len <= SDPE_RPC_EVENT_ARG_LEN);

    set_monitor = SDPE_RPC_GET_SHARE_BUF(struct sdpe_ctrl_set_monitor);
    req = &set_monitor->req;
    req->event_id = event_id;
    req->enable_flag = enable;
    req->arg_len = arg_len;
    memcpy(req->arg, arg, arg_len);

    msg_id = SDPE_RPC_CTRL_GET_MSGID(SDPE_CTRL_MONITOR);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&set_monitor, sizeof(uint32_t));
}

#ifndef SUPPORT_SDPE_RPC_MULTI_TASK
void sdpe_service_cb_register(uint8_t services, sdpe_rpc_usr_cb usr_cb)
{
    g_sdpe_service_handler[services] = usr_cb;
}

struct sdpe_rpc_dev *sdpe_service_get_rpc_dev(void)
{
    return &g_sdpe_ctrl_client.rpc_dev;
}
#endif

bool sdpe_rpc_is_ready(void)
{
    return g_sdpe_ctrl_client.ready;
}

int sdpe_ctrl_service_init(void)
{
    const struct sdpe_rpc_config *cfg;
    int ret;

    cfg = sdpe_rpc_find_cfg(SDPE_RPC_SDPE_SERVICE);
    if (cfg == NULL) {
        dprintf(ALWAYS, "rpc get cfg fail\n");
        return -1;
    }

    ret = sdpe_rpc_dev_init(&g_sdpe_ctrl_client.rpc_dev, "SDPE_CTRL_CLIENT",
                            cfg->server_id, cfg->local_addr,
                            sdpe_ctrl_rpc_cb, NULL,
                            SDPE_RPC_MBOX_TRNASPORT,
                            32, true, false);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc dev init fail\n");
        return ret;
    }

    ret = sdpe_rpc_dev_open(&g_sdpe_ctrl_client.rpc_dev);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc dev open fail\n");
        return ret;
    }

    return ret;
}
