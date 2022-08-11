/*
 * sdpe_rpc_doip_server.c
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
#include "doip_service.h"

#include "sdpe_rpc_doip.h"

static struct vdoip_instance g_vdoip_server;

extern int doip_data_request(struct sdpe_doip_addr *t_addr, uint8_t *t_data, uint32_t t_len);

static int vdoip_data_request_handle(void *arg, uint8_t *data, uint32_t len);

static const sdpe_rpc_usr_hdl g_vdoip_server_handler[] =
{
    [VDOIP_DATA_REQ] = vdoip_data_request_handle,
};

static int vdoip_rpc_cb(void *arg, uint16_t msg_id, uint8_t *data, uint32_t len)
{
    uint8_t service_id = SDPE_RPC_GET_SERVICE_ID(msg_id);
    uint8_t sub_msg_id = SDPE_RPC_GET_MSG_ID(msg_id);

    ASSERT(service_id == SDPE_RPC_VDOIP_SERVICE);

    if (sub_msg_id > VDOIP_MSG_START && sub_msg_id < VDOIP_MSG_END) {
        dprintf(SDPE_RPC_DEBUG, "doip server recv msg:%d\n", sub_msg_id);
        if (g_vdoip_server_handler[sub_msg_id])
            return g_vdoip_server_handler[sub_msg_id](arg, data, len);
    }
    dprintf(ALWAYS, "doip server recv wrong msg:%d\n", msg_id);
    return -1;
}

static int vdoip_data_request_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vdoip_data_req_s *data_req= (struct vdoip_data_req_s *)data;

    ASSERT(data != NULL);

    return doip_data_request(&data_req->t_addr, data_req->t_data, data_req->t_len);
}

int vdoip_data_confirm(struct sdpe_doip_addr *addr, uint32_t result)
{
    struct vdoip_data_cnf_s cnf;
    uint16_t msg_id;

    msg_id = SDPE_RPC_DOIP_GET_MSGID(VDOIP_DATA_CNF);

    memcpy(&cnf.t_addr, addr, sizeof(struct sdpe_doip_addr));
    cnf.t_result = result;

    return sdpe_rpc_dev_send_message(&g_vdoip_server.priv, msg_id,
                                     SDPE_RPC_IND, SDPE_RPC_WAIT_FOREVER, 1,
                                     &cnf, sizeof(struct vdoip_data_cnf_s));
}

int vdoip_data_som_ind(struct sdpe_doip_addr *addr, uint32_t t_len)
{
    struct vdoip_data_som_ind_s som_ind;
    uint16_t msg_id;

    msg_id = SDPE_RPC_DOIP_GET_MSGID(VDOIP_DATA_SOM_IND);

    memcpy(&som_ind.t_addr, addr, sizeof(struct sdpe_doip_addr));
    som_ind.t_len = t_len;

    return sdpe_rpc_dev_send_message(&g_vdoip_server.priv, msg_id,
                                     SDPE_RPC_IND, SDPE_RPC_WAIT_FOREVER, 1,
                                     &som_ind, sizeof(struct vdoip_data_som_ind_s));
}

int vdoip_data_ind(struct sdpe_doip_addr *addr, uint32_t t_result,
                   uint32_t t_len, uint8_t *data)
{
    struct vdoip_data_ind_s ind;
    uint16_t msg_id;

    msg_id = SDPE_RPC_DOIP_GET_MSGID(VDOIP_DATA_IND);

    memcpy(&ind.t_addr, addr, sizeof(struct sdpe_doip_addr));
    ind.t_result = t_result;
    ind.t_len = t_len;

    if (t_len == 0) {
        return sdpe_rpc_dev_send_message(&g_vdoip_server.priv, msg_id,
                                         SDPE_RPC_IND, SDPE_RPC_WAIT_FOREVER, 1,
                                         &ind, sizeof(struct vdoip_data_ind_s));
    }
    else {
        return sdpe_rpc_dev_send_message(&g_vdoip_server.priv, msg_id,
                                         SDPE_RPC_IND, SDPE_RPC_WAIT_FOREVER, 2,
                                         &ind, sizeof(struct vdoip_data_ind_s),
                                         data, t_len);
    }
}

int vdoip_service_open(void)
{
    int ret;

    ret = sdpe_rpc_dev_open(&g_vdoip_server.priv);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc transport open fail\n");
        return ret;
    }

    return 0;
}

int vdoip_service_close(void)
{
    return sdpe_rpc_dev_close(&g_vdoip_server.priv);
}

int vdoip_service_init(void)
{
    const struct sdpe_rpc_config *cfg;
    int ret;

    cfg = sdpe_rpc_find_cfg(SDPE_RPC_VDOIP_SERVICE);
    if (cfg == NULL) {
        dprintf(ALWAYS, "rpc get cfg fail\n");
        return -1;
    }

    ret = sdpe_rpc_dev_init(&g_vdoip_server.priv, "sdpe-doip",
                            cfg->client_id, cfg->local_addr,
                            vdoip_rpc_cb, NULL,
                            SDPE_RPC_RPMSG_TRNASPORT,
                            SDPE_RPC_DOIP_PACKET_LEN,
                            false, true);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc dev init fail\n");
        return ret;
    }

    return ret;
}
