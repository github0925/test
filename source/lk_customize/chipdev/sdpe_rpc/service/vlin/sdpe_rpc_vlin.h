/*
 * sdpe_rpc_vlin.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _VLIN_H_
#define _VLIN_H_

#include "Lin.h"
#include "sdpe_rpc_common.h"

#define SDPE_RPC_VLIN_CFG_LEN       128
#define SDPE_RPC_VLIN_PDU_LEN       8
#define SDPE_RPC_MAX_VLIN_NUM       4

#define SDPE_RPC_VLIN_GET_MSGID(id) \
    SDPE_RPC_CREATE_MSGID(SDPE_RPC_VLIN_SERVICE, id)

enum vlin_cb_types_e
{
    VLIN_MSG_START = 0,
    VLIN_INIT,
    VLIN_SEND_FRAME,
    VLIN_CHECK_WAKEUP,
    VLIN_GOTO_SLEEP,
    VLIN_GOTO_SLEEP_INTERNAL,
    VLIN_WAKEUP,
    VLIN_WAKEUP_INTERNAL,
    VLIN_GET_STATUS,
    VLIN_MSG_END
};

typedef struct vlin_instance {
    struct sdpe_rpc_dev *rpc_dev;
#ifdef SUPPORT_SDPE_RPC_MULTI_TASK
    struct sdpe_rpc_dev priv;
#endif
} vlin_instance_t;

SDPE_RPC_PACKED_BEGIN
struct vlin_init_req {
    struct Lin_ConfigType config;
    uint8_t config_buf[SDPE_RPC_VLIN_CFG_LEN];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_init_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_init_s {
    struct vlin_init_req req;
    struct vlin_init_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_send_frame_req {
    uint8_t channel;
    uint8_t pid;
    uint8_t cs;
    uint8_t drc;
    uint8_t dl;
    uint8_t reserved[3];
    uint8_t data[SDPE_RPC_VLIN_PDU_LEN];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_send_frame_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_send_frame_s {
    struct vlin_send_frame_req req;
    struct vlin_send_frame_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_check_wakeup_req {
    uint8_t channel;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_check_wakeup_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_check_wakeup_s {
    struct vlin_check_wakeup_req req;
    struct vlin_check_wakeup_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_goto_sleep_req {
    uint8_t channel;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_goto_sleep_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_goto_sleep_s {
    struct vlin_goto_sleep_req req;
    struct vlin_goto_sleep_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_goto_sleep_internal_req {
    uint8_t channel;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_goto_sleep_internal_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_goto_sleep_internal_s {
    struct vlin_goto_sleep_internal_req req;
    struct vlin_goto_sleep_internal_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_wakeup_req {
    uint8_t channel;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_wakeup_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_wakeup_s {
    struct vlin_wakeup_req req;
    struct vlin_wakeup_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_wakeup_internal_req {
    uint8_t channel;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_wakeup_internal_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_wakeup_internal_s {
    struct vlin_wakeup_internal_req req;
    struct vlin_wakeup_internal_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_get_status_req {
    uint8_t channel;
    uint8_t reserved[3];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_get_status_resp {
    int result;
    uint8_t pdu[SDPE_RPC_VLIN_PDU_LEN];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct vlin_get_status_s {
    struct vlin_get_status_req req;
    struct vlin_get_status_resp resp;
} SDPE_RPC_PACKED_END;

#endif
