/*
 * dqueue.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include "cqueue.h"
#include "dqueue.h"
#include "string.h"
#include "board_cfg.h"
/*standing off*/
CQ_handleTypeDef cqueue_can;
dqueue_t dqueue_can;
/*standing off*/
CQ_handleTypeDef cqueue_lin;
dqueue_t dqueue_lin;
/*standing off*/
dqueue_t *get_dqueue_can_handle(void)
{
    return &dqueue_can;
}
/*standing off*/
CQ_handleTypeDef *get_cqueue_can_handle(void)
{
    return &cqueue_can;
}
/*standing off*/
dqueue_t *get_dqueue_lin_handle(void)
{
    return &dqueue_lin;
}
/*standing off*/
CQ_handleTypeDef *get_cqueue_lin_handle(void)
{
    return &cqueue_lin;
}
/*standing off*/
static uint32_t cq_putInf(dqueue_t *dq, uint32_t len)
{
    uint32_t size;
    uint8_t info[3];

    info[0] = DQ_HEAD_1;
    info[1] = DQ_HEAD_2;
    info[2] = len;

    size = cq_putData(dq->cqueue, info, sizeof(info));
    return size;
}
/*standing off*/
uint32_t dq_getData(dqueue_t *dq, uint8_t *target)
{
    uint8_t info[3];
    uint32_t len = 0;

    if (cq_isEmpty(dq->cqueue) == true) {
        dprintf(debug_show_dg, "cq_isEmpty\n");
        return 0;
    }

    if (cq_getData(dq->cqueue, info, sizeof(info)) == 0) {
        dprintf(debug_show_dg, "cq_getData is 0\n");
        return 0;
    }

    if ((info[0] == DQ_HEAD_1) && (info[1] == DQ_HEAD_2)) {
        dprintf(debug_show_dg, "cq_getData success\n");
        len = cq_getData(dq->cqueue, target, info[2]);
        return len;
    }
    else {
        return 0;
    }
}
/*standing off*/
uint32_t dq_pushData(dqueue_t *dq, const uint8_t *target, uint32_t size)
{
    uint32_t len;

    if (cq_isFull(dq->cqueue) == true) {
        dprintf(debug_show_dg, "cq_isFull");
        return 0;
    }

    if (cq_putInf(dq, size) == 0) {
        dprintf(debug_show_dg, "putInf size is 0");
        return 0;
    }

    len = (cq_putData(dq->cqueue, target, size));

    return len;
}
/*standing off*/
bool dq_init(dqueue_t *dq)
{
    bool ret = false;

    ret = cq_init(dq->cqueue, dq->mem, sizeof(dq->mem));
    return ret;
}
/*standing off*/
void queue_init(void)
{
    dqueue_can.cqueue = &cqueue_can;
    dq_init(&dqueue_can);
}