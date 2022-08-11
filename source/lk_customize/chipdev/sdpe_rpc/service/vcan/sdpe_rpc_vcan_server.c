/*
 * sdpe_rpc_vcan_server.c
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

static struct vcan_instance g_vcan_server;

static int vcan_init_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_deinit_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_set_baudrate_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_set_mode_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_get_mode_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_enable_int_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_disable_int_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_check_wakeup_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_get_err_state_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_write_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_mf_write_handle(void *arg, uint8_t *data, uint32_t len);
static int vcan_mf_read_handle(void *arg, uint8_t *data, uint32_t len);

static const sdpe_rpc_usr_hdl g_vcan_server_handler[] =
{
    [VCAN_INIT]             = vcan_init_handle,
    [VCAN_DEINIT]           = vcan_deinit_handle,
    [VCAN_SET_BAUDRATE]     = vcan_set_baudrate_handle,
    [VCAN_SET_MODE]         = vcan_set_mode_handle,
    [VCAN_GET_MODE]         = vcan_get_mode_handle,
    [VCAN_ENABLE_INT]       = vcan_enable_int_handle,
    [VCAN_DISABLE_INT]      = vcan_disable_int_handle,
    [VCAN_CHECK_WAKEUP]     = vcan_check_wakeup_handle,
    [VCAN_GET_ERR_STATE]    = vcan_get_err_state_handle,
    [VCAN_WRITE]            = vcan_write_handle,
    [VCAN_MF_WRITE]         = vcan_mf_write_handle,
    [VCAN_MF_READ]          = vcan_mf_read_handle,
};

#ifdef SUPPORT_SDPE_RPC_MBUF
static void vcan_deconstruct_init_msg(Can_ConfigType *config)
{
    if (config->ctrllerCfg != NULL) {
        sdpe_rpc_free_mbuf(config->ctrllerCfg);
    }

    if (config->rxMBCfg != NULL) {
        sdpe_rpc_free_mbuf(config->rxMBCfg);
    }

    if (config->txMBCfg != NULL) {
        sdpe_rpc_free_mbuf(config->txMBCfg);
    }

    if (config->baudRateCfg != NULL) {
        sdpe_rpc_free_mbuf(config->baudRateCfg);
    }

    if (config->rxFIFOCfg != NULL) {
        if (config->rxFIFOCfg->flexcanRxFIFOCfg.filter_tab != NULL) {
            sdpe_rpc_free_mbuf(config->rxFIFOCfg->flexcanRxFIFOCfg.filter_tab);
        }

        sdpe_rpc_free_mbuf(config->rxFIFOCfg);
    }
}
#endif

static int vcan_init_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_init_s *init = (struct vcan_init_s *)data;
    struct vcan_init_req *req = &init->req;
    struct vcan_init_resp *resp = &init->resp;

    virCan_Init(&req->config);
    resp->result = 0;
    return 0;
}

static int vcan_deinit_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_deinit_s *deinit = (struct vcan_deinit_s *)data;
    struct vcan_deinit_resp *resp = &deinit->resp;

    virCan_DeInit();
    resp->result = 0;
    return 0;
}

static int vcan_set_baudrate_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_set_baudrate_s *set_baud = (struct vcan_set_baudrate_s *)data;
    struct vcan_set_baudrate_req *req = &set_baud->req;
    struct vcan_set_baudrate_resp *resp = &set_baud->resp;

    resp->result = virCan_SetBaudrate(req->controller, req->baudrate);
    return 0;
}

static int vcan_set_mode_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_set_mode_s *set_mode = (struct vcan_set_mode_s *)data;
    struct vcan_set_mode_req *req = &set_mode->req;
    struct vcan_set_mode_resp *resp = &set_mode->resp;

    resp->result = virCan_SetControllerMode(req->controller, req->mode);
    return 0;
}

static int vcan_get_mode_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_get_mode_s *get_mode = (struct vcan_get_mode_s *)data;
    struct vcan_get_mode_req *req = &get_mode->req;
    struct vcan_get_mode_resp *resp = &get_mode->resp;

    resp->result = virCan_GetControllerMode(req->controller, &resp->mode);
    return 0;
}

static int vcan_enable_int_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_enable_interrupts_s *enable_int = (struct vcan_enable_interrupts_s *)data;
    struct vcan_enable_interrupts_req *req = &enable_int->req;
    struct vcan_enable_interrupts_resp *resp = &enable_int->resp;

    virCan_EnableControllerInterrupts(req->controller);
    resp->result = 0;
    return 0;
}

static int vcan_disable_int_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_disable_interrupts_s *disable_int = (struct vcan_disable_interrupts_s *)data;
    struct vcan_disable_interrupts_req *req = &disable_int->req;
    struct vcan_disable_interrupts_resp *resp = &disable_int->resp;

    virCan_DisableControllerInterrupts(req->controller);
    resp->result = 0;
    return 0;
}

static int vcan_check_wakeup_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_check_wakeup_s *check_wakeup = (struct vcan_check_wakeup_s *)data;
    struct vcan_check_wakeup_req *req = &check_wakeup->req;
    struct vcan_check_wakeup_resp *resp = &check_wakeup->resp;

    virCan_CheckWakeup(req->controller);
    resp->result = 0;
    return 0;
}

static int vcan_get_err_state_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_get_err_state_s *get_state = (struct vcan_get_err_state_s *)data;
    struct vcan_get_err_state_req *req = &get_state->req;
    struct vcan_get_err_state_resp *resp = &get_state->resp;

    resp->result = virCan_GetControllerErrorState(req->controller, &resp->state);
    return 0;
}

static int vcan_write_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_write_s *write = (struct vcan_write_s *)data;
    struct vcan_write_req *req = &write->req;
    struct vcan_write_resp *resp = &write->resp;
    Can_PduType pdu;

#if (SDPE_RPC_DEBUG <= LK_DEBUGLEVEL)
    dprintf(SDPE_RPC_DEBUG, "pdu len:%d\n", req->length);
    for (uint8_t cnt = 0; cnt < req->length; cnt++) {
        dprintf(SDPE_RPC_DEBUG, "0x%x ", req->data[cnt]);
    }
    dprintf(SDPE_RPC_DEBUG, "\n");
#endif

    pdu.swPduHandle = req->swPduHandle;
    pdu.length = req->length;
    pdu.id = req->id;
    pdu.sdu = req->data;
    resp->result = virCan_Write(req->hth, &pdu);
    return 0;
}

static int vcan_mf_write_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_mf_write_s *mf_write = (struct vcan_mf_write_s *)data;
    struct vcan_mf_write_resp *resp = &mf_write->resp;

    vircan_MainFunction_Write();
    resp->result = 0;
    return 0;
}

static int vcan_mf_read_handle(void *arg, uint8_t *data, uint32_t len)
{
    struct vcan_mf_read_s *mf_read = (struct vcan_mf_read_s *)data;
    struct vcan_mf_read_resp *resp = &mf_read->resp;

    vircan_MainFunction_Read();
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
        dprintf(SDPE_RPC_DEBUG, "vcan server recv msg:%d\n", sub_msg_id);
        if (g_vcan_server_handler[sub_msg_id])
            return g_vcan_server_handler[sub_msg_id](arg, message_data, len);
    }
    dprintf(ALWAYS, "vcan server recv wrong msg:%d\n", msg_id);
    return -1;
}

void virCan_ControllerBusOff(uint8_t controller)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_server.rpc_dev;
    struct vcan_busoff_s *busoff;
    struct vcan_busoff_req *req;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        dprintf(ALWAYS, "controller id error:%d\n", controller);
        return;
    }

    busoff = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_busoff_s,
                                          SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &busoff->req;
    req->controller = controller;

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_BUSOFF);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&busoff, sizeof(uint32_t));
}

void virCan_SetWakeupEvent(uint8_t controller)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_server.rpc_dev;
    struct vcan_set_wakeup_event_s *set_wakeup;
    struct vcan_set_wakeup_event_req *req;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return;
    }

    if (controller >= SDPE_RPC_MAX_VCAN_NUM) {
        dprintf(ALWAYS, "controller id error:%d\n", controller);
        return;
    }

    set_wakeup = SDPE_RPC_GET_MULTI_SHARE_BUF(struct vcan_set_wakeup_event_s,
                                              SDPE_RPC_MAX_VCAN_NUM, controller);

    req = &set_wakeup->req;
    req->controller = controller;

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_SET_WAKEUP_EVENT);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&set_wakeup, sizeof(uint32_t));
}

void virCan_RxIndication(uint16_t hrh, uint32_t id, uint8_t dlc,
                         uint8_t *pdu)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_server.rpc_dev;
    struct vcan_rx_indicate_s *rx_ind;
    struct vcan_rx_indicate_req *req;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return;
    }

    rx_ind = SDPE_RPC_GET_SHARE_BUF(struct vcan_rx_indicate_s);

    req = &rx_ind->req;
    req->dlc = dlc;
    req->hrh = hrh;
    req->id = id;
    memcpy(req->data, pdu, dlc);

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_RX_INDICATIOPN);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&rx_ind, sizeof(uint32_t));
}

void virCan_TxConfirmation(uint16_t pdu_id)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_server.rpc_dev;
    struct vcan_tx_confirmation_s *tx_cnf;
    struct vcan_tx_confirmation_req *req;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return;
    }

    tx_cnf = SDPE_RPC_GET_SHARE_BUF(struct vcan_tx_confirmation_s);

    req = &tx_cnf->req;
    req->pdu_id = pdu_id;

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_TX_CONFIRMATION);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&tx_cnf, sizeof(uint32_t));
}

void virCan_ControllerWakeUp(uint8_t controller)
{
    struct sdpe_rpc_dev *rpc_dev = g_vcan_server.rpc_dev;
    struct vcan_ctrl_wakeup_s *ctrl_wakeup;
    struct vcan_ctrl_wakeup_req *req;
    uint16_t msg_id;

    if (!sdpe_rpc_is_ready()) {
        dprintf(ALWAYS, "sdpe rpc is not ready\n");
        return;
    }

    ctrl_wakeup = SDPE_RPC_GET_SHARE_BUF(struct vcan_ctrl_wakeup_s);

    req = &ctrl_wakeup->req;
    req->controller = controller;

    msg_id = SDPE_RPC_VCAN_GET_MSGID(VCAN_CTRL_WAKEUP);

    sdpe_rpc_dev_send_message(rpc_dev, msg_id, SDPE_RPC_REQ,
                              SDPE_RPC_WAIT_FOREVER,
                              (uint8_t*)&ctrl_wakeup, sizeof(uint32_t));
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

    ret = sdpe_rpc_dev_init(&g_vcan_server.priv, "VCAN_SERVER",
                            cfg->client_id, cfg->local_addr,
                            vcan_rpc_cb, NULL,
                            SDPE_RPC_MBOX_TRNASPORT,
                            32, true, false);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc dev init fail\n");
        return ret;
    }

    ret = sdpe_rpc_dev_open(&g_vcan_server.priv);
    if (ret < 0) {
        dprintf(ALWAYS, "rpc dev open fail\n");
        return ret;
    }

    g_vcan_server.rpc_dev = &g_vcan_server.priv;

    return ret;
#else
    sdpe_service_cb_register(SDPE_RPC_VCAN_SERVICE, vcan_rpc_cb);
    g_vcan_server.rpc_dev = sdpe_service_get_rpc_dev();
    return 0;
#endif
}
