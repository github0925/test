/*
 * sdpe_rpc_ctrl_server.c
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

static struct sdpe_ctrl_instance g_sdpe_ctrl_server;

#ifndef SUPPORT_SDPE_RPC_MULTI_TASK
static sdpe_rpc_usr_cb g_sdpe_service_handler[SDPE_RPC_MAX_SERVICE];
#endif

static int sdpe_rpc_sync_handle(void *arg, uint8_t *data, uint32_t len);
static int sdpe_start_routing_handle(void *arg, uint8_t *data, uint32_t len);
static int sdpe_stop_routing_handle(void *arg, uint8_t *data, uint32_t len);
static int sdpe_monitor_event_handle(void *arg, uint8_t *data, uint32_t len);

static const sdpe_rpc_usr_hdl g_sdpe_ctrl_handler[] =
{
    [SDPE_CTRL_SYNC_REQ]    = sdpe_rpc_sync_handle,
    [SDPE_CTRL_START_ROUTE] = sdpe_start_routing_handle,
    [SDPE_CTRL_STOP_ROUTE]  = sdpe_stop_routing_handle,
    [SDPE_CTRL_MONITOR]     = sdpe_monitor_event_handle,
};

extern void sdpe_start(void *route_cfg_addr);

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

static int sdpe_rpc_sync_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct sdpe_rpc_dev *rpc_dev = &g_sdpe_ctrl_server.rpc_dev;
    uint16_t msg_id;

    msg_id = SDPE_RPC_CTRL_GET_MSGID(SDPE_CTRL_SYNC_CNF);

    return sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_IND,
                                     SDPE_RPC_WAIT_FOREVER, NULL, 0);
}

static int sdpe_start_routing_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct sdpe_ctrl_start_route *start_route = (struct sdpe_ctrl_start_route *)data;
    struct sdpe_ctrl_start_route_req *req = &start_route->req;
    struct sdpe_ctrl_start_route_resp *resp = &start_route->resp;

    sdpe_start_routing(req->route_table_addr, req->route_table_size);
    resp->result = 0;
    return 0;
}

static int sdpe_stop_routing_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct sdpe_ctrl_stop_route *stop_route = (struct sdpe_ctrl_stop_route *)data;
    struct sdpe_ctrl_stop_route_resp *resp = &stop_route->resp;

    sdpe_stop_routing();
    resp->result = 0;
    return 0;
}

static int sdpe_monitor_event_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct sdpe_ctrl_set_monitor *set_monitor = (struct sdpe_ctrl_set_monitor *)data;
    struct sdpe_ctrl_set_monitor_req *req = &set_monitor->req;
    struct sdpe_ctrl_set_monitor_resp *resp = &set_monitor->resp;

    sdpe_monitor_event(req->event_id, req->enable_flag,
                       req->arg_len, req->arg);
    resp->result = 0;
    return 0;
}

void sdpe_event_cb(uint32_t event_id, uint32_t arg)
{
    struct sdpe_rpc_dev *rpc_dev = &g_sdpe_ctrl_server.rpc_dev;
    struct sdpe_ctrl_event_cb *event_cb;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return;
    }

    event_cb = SDPE_RPC_GET_SHARE_BUF(struct sdpe_ctrl_event_cb);
    event_cb->req.event_id = event_id;
    event_cb->req.arg = arg;

    msg_id = SDPE_RPC_CTRL_GET_MSGID(SDPE_CTRL_EVENT_CB);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&event_cb, sizeof(uint32_t));
}

#ifndef SUPPORT_SDPE_RPC_MULTI_TASK
void sdpe_service_cb_register(uint8_t services, sdpe_rpc_usr_cb usr_cb)
{
    g_sdpe_service_handler[services] = usr_cb;
}

struct sdpe_rpc_dev *sdpe_service_get_rpc_dev(void)
{
    return &g_sdpe_ctrl_server.rpc_dev;
}
#endif

bool sdpe_rpc_is_ready(void)
{
    return g_sdpe_ctrl_server.ready;
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

    ret = sdpe_rpc_dev_init(&g_sdpe_ctrl_server.rpc_dev, "SDPE_CTRL_SERVER",
                            cfg->client_id, cfg->local_addr,
                            sdpe_ctrl_rpc_cb, NULL,
                            SDPE_RPC_MBOX_TRNASPORT,
                            32, true, false);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc dev init fail\n");
        return ret;
    }

    ret = sdpe_rpc_dev_open(&g_sdpe_ctrl_server.rpc_dev);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc dev open fail\n");
        return ret;
    }

    g_sdpe_ctrl_server.ready = true;

    return ret;
}

