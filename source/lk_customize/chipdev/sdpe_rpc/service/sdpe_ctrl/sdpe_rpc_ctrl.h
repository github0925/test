/*
 * sdpe_rpc_ctrl.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _SDPE_CTRL_H_
#define _SDPE_CTRL_H_

#include "sdpe_rpc_common.h"

#define SDPE_RPC_CTRL_GET_MSGID(id) \
    SDPE_RPC_CREATE_MSGID(SDPE_RPC_SDPE_SERVICE, id)

#define SDPE_RPC_EVENT_ARG_LEN      300

enum sdpe_ctrl_cb_types_e
{
    SDPE_CTRL_MSG_START = 0,
    SDPE_CTRL_SYNC_REQ,
    SDPE_CTRL_SYNC_CNF,
    SDPE_CTRL_START_ROUTE,
    SDPE_CTRL_STOP_ROUTE,
    SDPE_CTRL_MONITOR,
    SDPE_CTRL_EVENT_CB,
    SDPE_CTRL_MSG_END
};

typedef struct sdpe_ctrl_instance {
    struct sdpe_rpc_dev rpc_dev;
    bool ready;
} sdpe_ctrl_instance_t;

SDPE_RPC_PACKED_BEGIN
struct sdpe_ctrl_start_route_req {
    uint32_t route_table_addr;
    uint32_t route_table_size;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct sdpe_ctrl_start_route_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct sdpe_ctrl_start_route {
    struct sdpe_ctrl_start_route_req req;
    struct sdpe_ctrl_start_route_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct sdpe_ctrl_stop_route_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct sdpe_ctrl_stop_route {
    struct sdpe_ctrl_stop_route_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct sdpe_ctrl_set_monitor_req {
    uint32_t event_id;
    uint32_t enable_flag;
    uint32_t arg_len;
    uint8_t arg[SDPE_RPC_EVENT_ARG_LEN];
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct sdpe_ctrl_set_monitor_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct sdpe_ctrl_set_monitor {
    struct sdpe_ctrl_set_monitor_req req;
    struct sdpe_ctrl_set_monitor_resp resp;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct sdpe_ctrl_event_cb_req {
    uint32_t event_id;
    uint32_t arg;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct sdpe_ctrl_event_cb_resp {
    int result;
} SDPE_RPC_PACKED_END;

SDPE_RPC_PACKED_BEGIN
struct sdpe_ctrl_event_cb {
    struct sdpe_ctrl_event_cb_req req;
    struct sdpe_ctrl_event_cb_resp resp;
} SDPE_RPC_PACKED_END;

#endif
