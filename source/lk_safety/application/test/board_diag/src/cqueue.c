/*
 * cqueue.c
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
#include "string.h"
/*standing off*/
bool cq_init(CQ_handleTypeDef *circular_queue, uint8_t *memAdd, uint16_t len)
{
    bool ret = false;
    circular_queue->size = len;

    if (!IS_POWER_OF_2(circular_queue->size)) {
        return ret;
    }

    if (memAdd == NULL) {
        return ret;
    }
    else {
        ret = true;
    }

    circular_queue->dataBufer = memAdd;

    memset(circular_queue->dataBufer, 0, len);
    circular_queue->entrance = circular_queue->exit = 0;

    return ret;
}
/*standing off*/
bool cq_isEmpty(CQ_handleTypeDef *circular_queue)
{
    if (circular_queue->entrance == circular_queue->exit)
        return true;
    else
        return false;
}
/*standing off*/
bool cq_isFull(CQ_handleTypeDef *circular_queue)
{
    if ((circular_queue->entrance - circular_queue->exit) == circular_queue->size)
        return true;
    else
        return false;
}
/*standing off*/
uint32_t cq_getLength(CQ_handleTypeDef *circular_queue)
{

    return (circular_queue->entrance - circular_queue->exit);
}
/*standing off*/
void cq_emptyData(CQ_handleTypeDef *circular_queue)
{

    circular_queue->entrance = circular_queue->exit = 0;
    memset(circular_queue->dataBufer, 0, circular_queue->size);
}
/*standing off*/
uint32_t cq_putData(CQ_handleTypeDef *circular_queue, const uint8_t *sourceBuf,
                    uint32_t len)
{
    uint32_t size = 0;

    len = GET_MIN(len, circular_queue->size - circular_queue->entrance +
                  circular_queue->exit);

    size = GET_MIN(len, circular_queue->size - (circular_queue->entrance &
                   (circular_queue->size - 1)));
    memcpy(circular_queue->dataBufer + (circular_queue->entrance &
                                        (circular_queue->size - 1)), sourceBuf, size);
    memcpy(circular_queue->dataBufer, sourceBuf + size, len - size);

    circular_queue->entrance += len;

    return len;
}
/*standing off*/
uint32_t cq_getData(CQ_handleTypeDef *circular_queue, uint8_t *targetBuf,
                    uint32_t len)
{
    uint32_t size = 0;

    len = GET_MIN(len, circular_queue->entrance - circular_queue->exit);

    size = GET_MIN(len, circular_queue->size - (circular_queue->exit &
                   (circular_queue->size - 1)));
    memcpy(targetBuf, circular_queue->dataBufer + (circular_queue->exit &
            (circular_queue->size - 1)), size);
    memcpy(targetBuf + size, circular_queue->dataBufer, len - size);

    circular_queue->exit += len;

    return len;
}