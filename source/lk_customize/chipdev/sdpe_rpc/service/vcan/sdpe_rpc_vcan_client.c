/*
 * sdpe_rpc_vcan_client.c
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
#include "sdpe_rpc_vcan.h"
#include "sdpe_ctrl_service.h"
#include "vcan_service.h"
#include "Can.h"

static struct vcan_instance g_vcan_client;

static int vcan_busoff_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_set_wakeup_event_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_rx_indication_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_tx_confirmation_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_ctrl_wakeup_handle(void *arg, uint8_t *data, uint32_t len);

static const sdpe_rpc_usr_hdl g_vcan_client_handler[] =
{
    [VCAN_BUSOFF]             = vcan_busoff_handle,
    [VCAN_SET_WAKEUP_EVENT]   = vcan_set_wakeup_event_handle,
    [VCAN_RX_INDICATIOPN]     = vcan_rx_indication_handle,
    [VCAN_TX_CONFIRMATION]    = vcan_tx_confirmation_handle,
    [VCAN_CTRL_WAKEUP]        = vcan_ctrl_wakeup_handle,
};

static mutex_t vcan_write_mutex[SDPE_RPC_MAX_HTH_NUM];

static void vcan_mutex_init(void)
{
    uint32_t i;

    for (i = 0; i < SDPE_RPC_MAX_HTH_NUM; i++) {
        mutex_init(&vcan_write_mutex[i]);
    }
}

static int vcan_busoff_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_busoff_s *busoff = (struct vcan_busoff_s *)data;
    struct vcan_busoff_req *req = &busoff->req;
    struct vcan_busoff_resp *resp = &busoff->resp;

    virCan_ControllerBusOff(req->controller);
    resp->result = 0;
    return 0;
}

static int vcan_set_wakeup_event_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_set_wakeup_event_s *set_wakeup = (struct vcan_set_wakeup_event_s *)data;
    struct vcan_set_wakeup_event_req *req = &set_wakeup->req;
    struct vcan_set_wakeup_event_resp *resp = &set_wakeup->resp;

    virCan_SetWakeupEvent(req->controller);
    resp->result = 0;
    return 0;
}

static int vcan_rx_indication_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_rx_indicate_s *rx_ind = (struct vcan_rx_indicate_s *)data;
    struct vcan_rx_indicate_req *req = &rx_ind->req;
    struct vcan_rx_indicate_resp *resp = &rx_ind->resp;

    virCan_RxIndication(req->hrh, req->id, req->dlc, req->data);
    resp->result = 0;
    return 0;
}

static int vcan_tx_confirmation_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_tx_confirmation_s *tx_cnf = (struct vcan_tx_confirmation_s *)data;
    struct vcan_tx_confirmation_req *req = &tx_cnf->req;
    struct vcan_tx_confirmation_resp *resp = &tx_cnf->resp;

    virCan_TxConfirmation(req->pdu_id);
    resp->result = 0;
    return 0;
}

static int vcan_ctrl_wakeup_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_ctrl_wakeup_s *ctrl_wakeup = (struct vcan_ctrl_wakeup_s *)data;
    struct vcan_ctrl_wakeup_req *req = &ctrl_wakeup->req;
    struct vcan_ctrl_wakeup_resp *resp = &ctrl_wakeup->resp;

    virCan_ControllerWakeUp(req->controller);
    resp->result = 0;
    return 0;
}

static int vcan_rpc_cb(void *arg, uint16_t msg_id, uint8_t *data, uint32_t len)
{
    uint8_t *message_data = (uint8_t *)(*(uint32_t*)data);
    uint8_t service_id = SDPE_RPC_GET_SERVICE_ID(msg_id);
    uint8_t sub_msg_id = SDPE_RPC_GET_MSG_ID(msg_id);

    ASSERT(service_id == SDPE_RPC_VCAN_SERVICE);

    if (sub_msg_id > VCAN_MSG_START && sub_msg_id < VCAN_MSG_END) {
        dprintf(SDPE_RPC_DEBUG, "vcan client recv msg:%d\n", sub_msg_id);
        if (g_vcan_client_handler[sub_msg_id])
            return g_vcan_client_handler[sub_msg_id](arg, message_data, len);
    }
    dprintf(ALWAYS, "vcan client recv wrong msg:%d\n", msg_id);
    return -1;
}

static int vcan_construct_init_msg(Can_ConfigType *new_config,
                                   const Can_ConfigType *config,
                                   uint8_t *config_buf)
{
    uint32_t size;
    uint8_t *addr = config_buf;

    if (config->controllerCount > 0) {
        size = sizeof(Can_ControllerConfig)*config->controllerCount;
        if (((addr - config_buf) + size) > SDPE_RPC_VCAN_CFG_LEN) {
            return -1;
        }
        SDPE_RPC_COPY_MSG_DATA(ctrllerCfg, size, addr);
    }

    if (config->rxCount > 0) {
        size = sizeof(Can_MBConfig)*config->rxCount;
        if (((addr - config_buf) + size) > SDPE_RPC_VCAN_CFG_LEN) {
            return -1;
        }
        SDPE_RPC_COPY_MSG_DATA(rxMBCfg, size, addr);
    }

    if (config->txCount > 0) {
        size = sizeof(Can_MBConfig)*config->txCount;
        if (((addr - config_buf) + size) > SDPE_RPC_VCAN_CFG_LEN) {
            return -1;
        }
        SDPE_RPC_COPY_MSG_DATA(txMBCfg, size, addr);
    }

    if (config->baudRateCfgCount > 0) {
        size = sizeof(Can_BaudRateConfig)*config->baudRateCfgCount;
        if (((addr - config_buf) + size) > SDPE_RPC_VCAN_CFG_LEN) {
            return -1;
        }
        SDPE_RPC_COPY_MSG_DATA(baudRateCfg, size, addr);
    }

    if (config->rxFifoCount > 0) {
        size = sizeof(Can_RxFIFOConfig)*config->rxFifoCount;
        if (((addr - config_buf) + size) > SDPE_RPC_VCAN_CFG_LEN) {
            return -1;
        }
        SDPE_RPC_COPY_MSG_DATA(rxFIFOCfg, size, addr);

        if (config->rxFIFOCfg->flexcanRxFIFOCfg.idFilterNum > 0) {
            size = sizeof(flexcan_rx_fifo_filter_table_t)*config->rxFIFOCfg->flexcanRxFIFOCfg.idFilterNum;
            if (((addr - config_buf) + size) > SDPE_RPC_VCAN_CFG_LEN) {
                return -1;
            }
            SDPE_RPC_COPY_MSG_DATA(rxFIFOCfg->flexcanRxFIFOCfg.filter_tab, size, addr);
        }
    }

    return 0;
}

void virCan_Init(const Can_ConfigType *config)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_init_s *vcan_init;
    struct vcan_init_req *req;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return;
    }

    vcan_init = SDPE_RPC_GET_SHARE_BUF(struct vcan_init_s);

    req = &vcan_init->req;
    memcpy(&req->config, config, sizeof(Can_ConfigType));
    if (vcan_construct_init_msg(&req->config, config, req->config_buf) < 0) {
        dprintf(ALWAYS, "construct vcan init msg fail, no enough memory\n");
        return;
    }

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_INIT);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&vcan_init, sizeof(uint32_t));
}

void virCan_DeInit(void)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_deinit_s *vcan_deinit;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return;
    }

    vcan_deinit = SDPE_RPC_GET_SHARE_BUF(struct vcan_deinit_s);

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_DEINIT);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&vcan_deinit, sizeof(uint32_t));
}

int virCan_SetBaudrate(uint8_t controller, uint16_t baudrate)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_set_baudrate_s *set_baud;
    struct vcan_set_baudrate_req *req;
    struct vcan_set_baudrate_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        dprintf(ALWAYS, "controller id error:%d\n", controller);
        return -1;
    }

    set_baud = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_set_baudrate_s,
                                            SDPE_RPC_MAX_VCAN_NUM, controller);
    req = &set_baud->req;
    req->controller = controller;
    req->baudrate = baudrate;

    resp = &set_baud->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_SET_BAUDRATE);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&set_baud, sizeof(uint32_t));

    return resp->result;
}

int virCan_SetControllerMode(uint8_t controller, uint8_t mode)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_set_mode_s *set_mode;
    struct vcan_set_mode_req *req;
    struct vcan_set_mode_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        dprintf(ALWAYS, "controller id error:%d\n", controller);
        return -1;
    }

    set_mode = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_set_mode_s,
                                            SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &set_mode->req;
    req->controller = controller;
    req->mode = mode;

    resp = &set_mode->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_SET_MODE);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&set_mode, sizeof(uint32_t));

    return resp->result;
}

int virCan_GetControllerMode(uint8_t controller, uint8_t *mode)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_get_mode_s *get_mode;
    struct vcan_get_mode_req *req;
    struct vcan_get_mode_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        dprintf(ALWAYS, "controller id error:%d\n", controller);
        return -1;
    }

    get_mode = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_get_mode_s,
                                            SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &get_mode->req;
    req->controller = controller;

    resp = &get_mode->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_GET_MODE);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&get_mode, sizeof(uint32_t));

    *mode = resp->mode;
    return resp->result;
}

int virCan_EnableControllerInterrupts(uint8_t controller)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_enable_interrupts_s *enable_int;
    struct vcan_enable_interrupts_req *req;
    struct vcan_enable_interrupts_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        dprintf(ALWAYS, "controller id error:%d\n", controller);
        return -1;
    }

    enable_int = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_enable_interrupts_s,
                                              SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &enable_int->req;
    req->controller = controller;

    resp = &enable_int->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_ENABLE_INT);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&enable_int, sizeof(uint32_t));

    return resp->result;
}

int virCan_DisableControllerInterrupts(uint8_t controller)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_disable_interrupts_s *disable_int;
    struct vcan_disable_interrupts_req *req;
    struct vcan_disable_interrupts_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        dprintf(ALWAYS, "controller id error:%d\n", controller);
        return -1;
    }

    disable_int = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_disable_interrupts_s,
                                               SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &disable_int->req;
    req->controller = controller;

    resp = &disable_int->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_DISABLE_INT);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&disable_int, sizeof(uint32_t));

    return resp->result;
}

int virCan_CheckWakeup(uint8_t controller)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_check_wakeup_s *check_wakup;
    struct vcan_check_wakeup_req *req;
    struct vcan_check_wakeup_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        dprintf(ALWAYS, "controller id error:%d\n", controller);
        return -1;
    }

    check_wakup = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_check_wakeup_s,
                                               SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &check_wakup->req;
    req->controller = controller;

    resp = &check_wakup->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_CHECK_WAKEUP);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&check_wakup, sizeof(uint32_t));

    return resp->result;
}

int virCan_GetControllerErrorState(uint8_t controller, uint8_t *state)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_get_err_state_s *get_state;
    struct vcan_get_err_state_req *req;
    struct vcan_get_err_state_resp *resp;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        dprintf(ALWAYS, "controller id error:%d\n", controller);
        return -1;
    }

    get_state = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_get_err_state_s,
                                             SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &get_state->req;
    req->controller = controller;

    resp = &get_state->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_GET_ERR_STATE);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&get_state, sizeof(uint32_t));

    *state = resp->state;
    return resp->result;
}

int virCan_Write(uint16_t hth, const Can_PduType *pdu)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_write_s *write;
    struct vcan_write_req *req;
    struct vcan_write_resp *resp;
    uint16_t msg_id;
    int result;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return -1;
    }

    if (hth >= SDPE_RPC_MAX_HTH_NUM) {
        dprintf(ALWAYS, "hth id error:%d\n", hth);
        return -1;
    }

    mutex_acquire_timeout(&vcan_write_mutex[hth], INFINITE_TIME);

    write = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_write_s,
                                         SDPE_RPC_MAX_HTH_NUM, hth);

    req = &write->req;
    req->hth = hth;
    req->length = pdu->length;
    req->swPduHandle = pdu->swPduHandle;
    req->id = pdu->id;
    memcpy(req->data, pdu->sdu, pdu->length);

    resp = &write->resp;
    resp->result = -1;

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_WRITE);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&write, sizeof(uint32_t));
    result = resp->result;

    mutex_release(&vcan_write_mutex[hth]);

    return result;
}

void vircan_MainFunction_Write(void)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_mf_write_s *vcan_mf_write;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return;
    }

    vcan_mf_write = SDPE_RPC_GET_SHARE_BUF(struct vcan_mf_write_s);

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_MF_WRITE);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&vcan_mf_write, sizeof(uint32_t));
}

void vircan_MainFunction_Read(void)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_client.rpc_dev;
    struct vcan_mf_read_s *vcan_mf_read;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return;
    }

    vcan_mf_read = SDPE_RPC_GET_SHARE_BUF(struct vcan_mf_read_s);

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_MF_READ);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&vcan_mf_read, sizeof(uint32_t));
}

int vcan_service_init(void)
{
#ifdef SUPPORT_SDPE_RPC_MULTI_TASK
    const struct sdpe_rpc_config *cfg;
    int ret;

    cfg = sdpe_rpc_find_cfg(SDPE_RPC_VCAN_SERVICE);
    if (cfg == NULL) {
        dprintf(ALWAYS, "rpc get cfg fail\n");
        return -1;
    }

    ret = sdpe_rpc_dev_init(&g_vcan_client.priv, "VCAN_CLIENT",
                            cfg->server_id, cfg->local_addr,
                            vcan_rpc_cb, NULL,
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

    g_vcan_client.rpc_dev = &g_vcan_client.priv;

    vcan_mutex_init();

    return ret;
#else
    sdpe_service_cb_register(SDPE_RPC_VCAN_SERVICE, vcan_rpc_cb);
    g_vcan_client.rpc_dev = sdpe_service_get_rpc_dev();
    vcan_mutex_init();
    return 0;
#endif
}
