/*
 * sdpe_ctrl_server.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SDPE Server driver handles remote function calls, by
 * safety.
 *
 * Revision History:
 * -----------------
 */
#include <debug.h>

#include "sdpe/event.h"

extern void sdpe_start(void *route_cfg_addr, uint32_t size);
extern void sdpe_stop(void);
extern void stat_filter_enable(void *filter_config, bool en);
extern void node_loss_filter_enable(void *filter_config, bool en);
extern void pkt_ovf_filter_enable(void *filter_config, bool en);
extern void mirror_filter_enable(void *filter_config, bool en);
extern void msg_timeout_filter_enable(void *filter_config, bool en);
extern void live_cnt_filter_enable(void *filter_config, bool en);
extern void checksum_filter_enable(void *filter_config, bool en);
extern void invasion_filter_enable(void *filter_config, bool en);

void sdpe_start_routing(uint32_t route_table, uint32_t size)
{
    dprintf(INFO, "sdpe_starting_routing 0x%x\n", route_table);
    sdpe_start((void *)route_table, size);
}

void sdpe_stop_routing(void)
{
    sdpe_stop();
}

void sdpe_monitor_event(uint32_t event_id, uint8_t enable, uint32_t arg_len, uint8_t *arg)
{
    dprintf(INFO, "%s: event %d, enable %d, arg %p, arg len %d\n", __func__,
            event_id, enable, arg, arg_len);

    if (event_id == ROUTE_STAT) {
        stat_filter_enable((void *)arg, enable);
    }
    else if (event_id == ECU_NODE_MISS) {
        node_loss_filter_enable((void *)arg, enable);
    }
    else if (event_id == PKT_ID_MISSING) {
        msg_timeout_filter_enable((void *)arg, enable);
    }
    else if (event_id == LIVE_COUNTER) {
        live_cnt_filter_enable((void *)arg, enable);
    }
    else if (event_id == CHECKSUM_ERR) {
        checksum_filter_enable((void *)arg, enable);
    }
    else if (event_id == PACKET_OVERFLOW) {
        pkt_ovf_filter_enable((void *)arg, enable);
    }
    else if (event_id == BUS_MIRROR) {
        mirror_filter_enable((void *)arg, enable);
    }
    else if (event_id == INVALID_PACKET) {
        invasion_filter_enable((void *)arg, enable);
    }
}
