/*
 * sdpe_rpc_eth_server.c
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
#include "eth_service.h"

#include "sdpe_rpc_eth.h"

static struct veth_instance g_veth_server[SDPE_RPC_MAX_ETH_NUM];

static int veth_send_frame_handle(void *arg, uint8_t *data, uint32_t len);

static const sdpe_rpc_usr_hdl g_veth_server_handler[] =
{
    [VETH_SEND_FRAME] = veth_send_frame_handle,
};

static int veth_rpc_cb(void *arg, uint16_t msg_id, uint8_t *data, uint32_t len)
{
    struct veth_instance *eth_server = (struct veth_instance *)arg;
    uint8_t service_id = SDPE_RPC_GET_SERVICE_ID(msg_id);
    uint8_t sub_msg_id = SDPE_RPC_GET_MSG_ID(msg_id);

    ASSERT(eth_server != NULL);
    ASSERT(service_id == eth_server->service_id);

    if (sub_msg_id > VETH_MSG_START && sub_msg_id < VETH_MSG_END) {
        dprintf(SDPE_RPC_DEBUG, "eth server recv msg:%d\n", sub_msg_id);
        if (g_veth_server_handler[sub_msg_id])
            return g_veth_server_handler[sub_msg_id](arg, data, len);
    }
    dprintf(ALWAYS, "eth server recv wrong msg:%d\n", msg_id);
    return -1;
}

static int veth_send_frame_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct veth_instance *eth_server = (struct veth_instance *)arg;
    int ret = 0;

    ASSERT(eth_server != NULL);

    if (eth_server->usr_cb) {
        ret = eth_server->usr_cb(eth_server->arg, data, len);
    }

    return ret;
}

static int veth_get_eth_service_id(int eth_id)
{
    int rpc_service_id = -1;

    if (eth_id == 0) {
        rpc_service_id = SDPE_RPC_VETH1_SERVICE;
    }
    else if (eth_id == 1){
        rpc_service_id = SDPE_RPC_VETH2_SERVICE;
    }

    return rpc_service_id;
}

int veth_send_frame(uint8_t eth_id, uint8_t *data, uint32_t len)
{
    struct veth_instance *eth_server;
    uint16_t msg_id;

    ASSERT(eth_id < SDPE_RPC_MAX_ETH_NUM);

    eth_server = &g_veth_server[eth_id];

    msg_id = SDPE_RPC_CREATE_MSGID(veth_get_eth_service_id(eth_id),
                                   VETH_SEND_FRAME);

    return sdpe_rpc_dev_send_message(&eth_server->priv, msg_id, SDPE_RPC_IND,
                                     SDPE_RPC_WAIT_FOREVER, 1, data, len);
}

int veth_service_open(uint8_t eth_id, veth_usr_cb usr_cb, void *arg)
{
    struct veth_instance *eth_server;
    int ret;

    ASSERT(eth_id < SDPE_RPC_MAX_ETH_NUM);

    eth_server = &g_veth_server[eth_id];
    ret = sdpe_rpc_dev_open(&eth_server->priv);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc transport open fail\n");
        return ret;
    }

    eth_server->usr_cb = usr_cb;
    eth_server->arg = arg;

    return 0;
}

int veth_service_close(uint8_t eth_id)
{
    struct veth_instance *eth_server;

    ASSERT(eth_id < SDPE_RPC_MAX_ETH_NUM);

    eth_server = &g_veth_server[eth_id];
    return sdpe_rpc_dev_close(&eth_server->priv);
}

int veth_service_init(uint8_t eth_id)
{
    const struct sdpe_rpc_config *cfg;
    struct veth_instance *eth_server;
    int eth_service_id;
    int ret;

    ASSERT(eth_id < SDPE_RPC_MAX_ETH_NUM);

    eth_service_id = veth_get_eth_service_id(eth_id);
    if (eth_service_id == -1) {
        dprintf(ALWAYS, "error eth_id:%d\n", eth_id);
        return -1;
    }

    cfg = sdpe_rpc_find_cfg(eth_service_id);
    if (cfg == NULL) {
        dprintf(ALWAYS, "rpc get cfg fail\n");
        return -1;
    }

    eth_server = &g_veth_server[eth_id];

    char name[32];
    sprintf(name, "sdpe-eth%d", eth_id);
    ret = sdpe_rpc_dev_init(&eth_server->priv, name,
                            cfg->client_id, cfg->local_addr,
                            veth_rpc_cb, eth_server,
                            SDPE_RPC_RPMSG_TRNASPORT,
                            SDPE_RPC_ETH_PACKET_LEN,
                            false, true);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc dev init fail\n");
        return ret;
    }

    eth_server->service_id = eth_service_id;

    return ret;
}
