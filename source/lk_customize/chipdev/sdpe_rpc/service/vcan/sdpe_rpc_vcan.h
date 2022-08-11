/*
 * sdpe_rpc_vcan.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _VCAN_H_
#define _VCAN_H_

#include "Can.h"
#include "sdpe_rpc_common.h"

#define SDPE_RPC_VCAN_CFG_LEN       5120
#define SDPE_RPC_VCAN_PDU_LEN       64
#define SDPE_RPC_MAX_VCAN_NUM       20
#define SDPE_RPC_MAX_HTH_NUM        140

#define SDPE_RPC_VCAN_GET_MSGID(id) \
    SDPE_RPC_CREATE_MSGID(SDPE_RPC_VCAN_SERVICE, id)

enum vcan_cb_types_e
{
    VCAN_MSG_START = 0,
    VCAN_INIT,
    VCAN_DEINIT,
    VCAN_SET_BAUDRATE,
    VCAN_SET_MODE,
    VCAN_GET_MODE,
    VCAN_ENABLE_INT,
    VCAN_DISABLE_INT,
    VCAN_CHECK_WAKEUP,
    VCAN_GET_ERR_STATE,
    VCAN_WRITE,
    VCAN_MF_WRITE,
    VCAN_MF_READ,
    VCAN_BUSOFF,
    VCAN_SET_WAKEUP_EVENT,
    VCAN_RX_INDICATIOPN,
    VCAN_TX_CONFIRMATION,
    VCAN_CTRL_WAKEUP,
    VCAN_MSG_END
};

typedef struct vcan_instance {
    struct sdpe_rpc_dev *rpc_dev;
#ifdef SUPPORT_SDPE_RPC_MULTI_TASK
    struct sdpe_rpc_dev priv;
#endif
} vcan_instance_t;

SDPE_RPC_PACKED_BEGIN
struct vcan_init_req {
    Can_ConfigType config;
    uint8_t config_buf[SDPE_RPC_VCAN_CFG_LEN];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_init_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_init_s {
    struct vcan_init_req req;
    struct vcan_init_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_deinit_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_deinit_s {
    struct vcan_deinit_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_set_baudrate_req {
    uint8_t controller;
    uint8_t reserved;
    uint16_t baudrate;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_set_baudrate_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_set_baudrate_s {
    struct vcan_set_baudrate_req req;
    struct vcan_set_baudrate_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_set_mode_req {
    uint8_t controller;
    uint8_t mode;
    uint8_t reserved[2];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_set_mode_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_set_mode_s {
    struct vcan_set_mode_req req;
    struct vcan_set_mode_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_get_mode_req {
    uint8_t controller;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_get_mode_resp {
    int result;
    uint8_t mode;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_get_mode_s {
    struct vcan_get_mode_req req;
    struct vcan_get_mode_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_enable_interrupts_req {
    uint8_t controller;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_enable_interrupts_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_enable_interrupts_s {
    struct vcan_enable_interrupts_req req;
    struct vcan_enable_interrupts_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_disable_interrupts_req {
    uint8_t controller;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_disable_interrupts_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_disable_interrupts_s {
    struct vcan_disable_interrupts_req req;
    struct vcan_disable_interrupts_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_check_wakeup_req {
    uint8_t controller;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_check_wakeup_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_check_wakeup_s {
    struct vcan_check_wakeup_req req;
    struct vcan_check_wakeup_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_get_err_state_req {
    uint8_t controller;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_get_err_state_resp {
    int result;
    uint8_t state;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_get_err_state_s {
    struct vcan_get_err_state_req req;
    struct vcan_get_err_state_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_write_req {
    uint16_t hth;
    uint8_t  length;
    uint8_t  reserved[3];
    uint16_t swPduHandle;
    uint32_t id;
    uint8_t  data[SDPE_RPC_VCAN_PDU_LEN];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_write_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_write_s {
    struct vcan_write_req req;
    struct vcan_write_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_mf_write_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_mf_write_s {
    struct vcan_mf_write_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_mf_read_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_mf_read_s {
    struct vcan_mf_read_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_busoff_req {
    uint8_t controller;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_busoff_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_busoff_s {
    struct vcan_busoff_req req;
    struct vcan_busoff_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_set_wakeup_event_req {
    uint8_t controller;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_set_wakeup_event_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_set_wakeup_event_s {
    struct vcan_set_wakeup_event_req req;
    struct vcan_set_wakeup_event_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_rx_indicate_req {
    uint8_t  dlc;
    uint8_t  reserved;
    uint16_t hrh;
    uint32_t id;
    uint8_t  data[SDPE_RPC_VCAN_PDU_LEN];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_rx_indicate_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_rx_indicate_s {
    struct vcan_rx_indicate_req req;
    struct vcan_rx_indicate_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_tx_confirmation_req {
    uint16_t pdu_id;
    uint8_t reserved[2];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_tx_confirmation_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_tx_confirmation_s {
    struct vcan_tx_confirmation_req req;
    struct vcan_tx_confirmation_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_ctrl_wakeup_req {
    uint8_t controller;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_ctrl_wakeup_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vcan_ctrl_wakeup_s {
    struct vcan_ctrl_wakeup_req req;
    struct vcan_ctrl_wakeup_resp resp;
} SDPE_RPC_PACKED_END;

#endif
