/*
 * Copyright (c) 2019 Semidrive Inc.
 *
 * Safety LIN scheduler Sample Application.
 */
#include <app.h>
#include <lib/console.h>
#include <stdio.h>
#include <string.h>

#include "Lin.h"
#ifdef COM
#include "com_cbk.h"
#include "com_cfg.h"

struct lin_sched {
    frame_desc_t    *frame;
    uint8_t         cs_type;
    bool            is_tx;
    int             delay;
};

static struct lin_sched g_lin_sched[] = {
    [0] = {
        .frame = &lin_mcp_1,
        .cs_type = LIN_CLASSIC_CS,
        .is_tx = false,
        .delay = 50,
    },
    [1] = {
        .frame = &gw_atc_1,
        .cs_type = LIN_CLASSIC_CS,
        .is_tx = true,
        .delay = 50,
    },
};

static void lin_sched(const struct app_descriptor *app, void *args)
{
    size_t frame_nr = sizeof(g_lin_sched) / sizeof(g_lin_sched[0]);

    /* Wait for RPC initializing completed. */
    thread_sleep(3000);

    for (int i = 0; ; i++) {
        int idx = i % frame_nr;
        struct lin_sched *sched = &g_lin_sched[idx];
        uint8_t channel = sched->frame->bus_id;
        uint8_t *rx_data;

        Lin_PduType pdu_info = {
            .Pid = sched->frame->prot_id,
            .Cs = sched->cs_type,
            .Dl = sched->frame->len,
            .SduPtr = sched->frame->data
        };
        pdu_info.Drc = (sched->is_tx) ? LIN_MASTER_RESPONSE : LIN_SLAVE_RESPONSE;

        uint8_t ret = Lin_SendFrame(channel, &pdu_info);
        dprintf(INFO, "Lin_SendFrame: ret %d, Pid 0x%x, bus_id %d, cs 0x%x\n",
                ret, pdu_info.Pid, channel, pdu_info.Cs);

        thread_sleep(sched->delay);
        uint8_t status = Lin_GetStatus(channel, &rx_data);
        dprintf(INFO, "virLin_GetStatus: status %d,"
                "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                status, rx_data[0], rx_data[1], rx_data[2], rx_data[3],
                rx_data[4], rx_data[5], rx_data[6], rx_data[7]);
        if (!sched->is_tx) {
            com_rx_frame(1, channel, pdu_info.Pid, rx_data);
        }
    }
}

APP_START(lin_sched)
.entry = lin_sched,
APP_END
#endif
