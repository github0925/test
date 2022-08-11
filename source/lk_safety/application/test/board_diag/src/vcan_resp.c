/*
 * vcan_resp.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include <stdbool.h>
#include <string.h>
#include "dqueue.h"
#include "cqueue.h"
#include "board_start.h"
#include "board_cfg.h"
#include "func_can.h"

canx_opt_t canx_app;
/*standing off*/
void push_can_info_into_queue(uint16_t Hrh, uint8_t CanSduPtr)
{
    uint8_t info[2];
    dqueue_t *dqueue;

    set_para_value(info[0], Hrh);
    set_para_value(info[1], CanSduPtr);

    dqueue = get_dqueue_can_handle();

    dq_pushData(dqueue, info, sizeof(info));
}
/*standing off*/
uint8_t get_can_info_chn_from_queue(uint8_t *pbuf)
{
    uint8_t len;
    dqueue_t *dqueue;
    CONTEXT_LOCK(canx_app);
    dqueue = get_dqueue_can_handle();
    CONTEXT_UNLOCK(canx_app);

    len = dq_getData(dqueue, pbuf);

    return len;
}
/*standing off*/
void clean_can_chn_queue_info(void)
{
    CQ_handleTypeDef *cqueue;

    cqueue = get_cqueue_can_handle();

    cq_emptyData(cqueue);
}
/*board data is sent out by a can channel, here we use can_2 channel*/
bool can_channel_to_write(canx_opt_t *canx_opt, uint32_t canId)
{
    bool ret = false;

    Can_PduType pdu = {
        .swPduHandle = 0,
        .length = 8U,
        .id = canId,
        .sdu = canx_opt->pay_load
    };

    if (Can_Write(canx_opt->resp_chn_id, &pdu) != E_NOT_OK)
        ret = true;

    return ret;
}