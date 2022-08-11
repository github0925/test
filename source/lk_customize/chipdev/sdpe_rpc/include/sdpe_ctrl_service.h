/*
 * sdpe_ctrl_service.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef _SDPE_CTRL_SERVICE_H
#define _SDPE_CTRL_SERVICE_H

#include "sdpe_rpc_framework.h"

/* SDPE CTRL SERVICE */

int sdpe_ctrl_service_init(void);
void sdpe_rpc_sync(void);
#ifndef SUPPORT_SDPE_RPC_MULTI_TASK
void sdpe_service_cb_register(uint8_t services, sdpe_rpc_usr_cb usr_cb);
struct sdpe_rpc_dev *sdpe_service_get_rpc_dev(void);
#endif
bool sdpe_rpc_is_ready(void);

void sdpe_start_routing(uint32_t route_table, uint32_t size);
void sdpe_stop_routing(void);
void sdpe_monitor_event(uint32_t event_id, uint8_t enable,
                        uint32_t arg_len, uint8_t *arg);

#endif
