/*
 * sdpe_rpc_l2.c
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sdpe_rpc_transport.h"
#include "sdpe_rpc_l2.h"

#define SF                              0xF0U                           /* Single frame */
#define MF(nr)                          ((nr) & 0x0FU)                  /* Multiple frame, nr: frame number */
#define EF(nr)                          (((nr) & 0x0FU) | 0x10U)        /* Ending frame */
#define IS_SF(buf)                      ((buf)[0] == SF)                /* Is single frame? */
#define IS_EF(buf)                      (((buf)[0] & 0xF0U) == 0x10U)   /* Is ending frame? */
#define GET_FRAME_NUM(buf)              ((buf)[0] & 0x0FU)
#define FC_SIZE                         1U

int sdpe_rpc_l2_init(struct sdpe_rpc_l2_instance *rpc_l2,
                     struct sdpe_rpc_transport_dev *transport)
{
    int ret = -1;

    rpc_l2->rx_buffer = malloc(transport->mtu);
    if (rpc_l2->rx_buffer) {
        rpc_l2->transport = transport;
        rpc_l2->rx_buf_len = transport->mtu;
        rpc_l2->rx_offset = 0;
        rpc_l2->rx_sn = 0;
        ret = 0;
    }

    return ret;
}

int sdpe_rpc_l2_uninit(struct sdpe_rpc_l2_instance *rpc_l2)
{
    if (rpc_l2->rx_buffer)
        free(rpc_l2->rx_buffer);

    return 0;
}

int sdpe_rpc_l2_send(struct sdpe_rpc_l2_instance *rpc_l2,
                     uint8_t *head, uint8_t head_len,
                     uint8_t sdu_num, va_list arg)
{
    uint8_t *sdu = NULL;
    uint8_t *pdu = NULL;
    uint32_t sdu_offset = 0;
    uint32_t pdu_offset = 0;
    uint32_t sdu_len;
    uint32_t pdu_len;
    uint32_t copy_len;
    uint8_t frame_cnt = 0;
    uint8_t sdu_cnt;

    pdu = sdpe_rpc_transport_tx_alloc(rpc_l2->transport, &pdu_len,
                                      SDPE_RPC_WAIT_FOREVER);
    if (!pdu) {
        dprintf(WARN, "%s: allocates tx buffer failed\n", __func__);
        return -1;
    }
    pdu_offset = FC_SIZE;

    memcpy(&pdu[pdu_offset], head, head_len);
    pdu_offset += head_len;

    for (sdu_cnt = sdu_num; sdu_cnt > 0; sdu_cnt--) {
        sdu = va_arg(arg, uint8_t *);
        sdu_len = va_arg(arg, uint32_t);
        sdu_offset = 0;

        while (sdu_offset < sdu_len) {

            if (!pdu) {
                pdu = sdpe_rpc_transport_tx_alloc(rpc_l2->transport, &pdu_len,
                                                  SDPE_RPC_WAIT_FOREVER);
                if (!pdu) {
                    dprintf(WARN, "%s: allocates tx buffer failed\n", __func__);
                    return -1;
                }
                pdu_offset = FC_SIZE;
            }

            /* copy sdu data to pdu */

            copy_len = MIN((sdu_len - sdu_offset), (pdu_len - pdu_offset));
            memcpy(&pdu[pdu_offset], &sdu[sdu_offset], copy_len);
            sdu_offset += copy_len;
            pdu_offset += copy_len;

            /* finish pdu or no data to trans, send pdu */

            bool last_end = (sdu_offset == sdu_len) && (sdu_cnt == 1);

            if (pdu_offset == pdu_len || last_end) {

                /* fill pdu head */

                if (last_end) {
                    if (frame_cnt == 0) {
                        *pdu = SF;
                    }
                    else {
                        *pdu = EF(frame_cnt);
                    }
                }
                else {
                    *pdu = MF(frame_cnt++);
                }

                sdpe_rpc_transport_send_nocopy(rpc_l2->transport, pdu,
                                               pdu_offset);
                pdu = NULL;
            }
        }
    }

    return 0;
}

static void sdpe_rpc_l2_restart_recv(struct sdpe_rpc_l2_instance *rpc_l2)
{
    rpc_l2->rx_offset = 0;
    rpc_l2->rx_sn = 0;
}

int sdpe_rpc_l2_recv(struct sdpe_rpc_l2_instance *rpc_l2,
                     uint8_t *data, uint32_t *data_len)
{
    uint8_t sn;
    uint16_t payload;
    uint32_t recv_len;

    while (1) {

        recv_len = rpc_l2->rx_buf_len;

        if (sdpe_rpc_transport_recv(rpc_l2->transport, rpc_l2->rx_buffer,
                                    &recv_len, true) < 0) {
            dprintf(WARN, "%s: rpc transport recv error...\n", __func__);
            return -1;
        }

        payload = recv_len - FC_SIZE;

        if ((payload + rpc_l2->rx_offset) > *data_len) {
            dprintf(WARN, "%s: recv frame len error...\n", __func__);
            sdpe_rpc_l2_restart_recv(rpc_l2);
            return -1;
        }

        sn = GET_FRAME_NUM(rpc_l2->rx_buffer);

        if (IS_SF(rpc_l2->rx_buffer) || sn == 0) {
            sdpe_rpc_l2_restart_recv(rpc_l2);
        }
        else if (sn == (rpc_l2->rx_sn + 1)) {
            rpc_l2->rx_sn = sn;
        }
        else {
            dprintf(WARN, "%s: recv frame discontinuity...\n", __func__);
            sdpe_rpc_l2_restart_recv(rpc_l2);
            return -1;
        }

        memcpy(&data[rpc_l2->rx_offset], rpc_l2->rx_buffer + FC_SIZE, payload);
        rpc_l2->rx_offset += payload;

        if (IS_SF(rpc_l2->rx_buffer) || IS_EF(rpc_l2->rx_buffer)) {
            *data_len = rpc_l2->rx_offset;
            sdpe_rpc_l2_restart_recv(rpc_l2);
            return 0;
        }
    }
}

