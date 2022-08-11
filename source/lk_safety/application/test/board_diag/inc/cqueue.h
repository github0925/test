/*
 * cqueue.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#ifndef _CQUEUE_H
#define _CQUEUE_H

#include <__regs_base.h>
#include <lib/console.h>
#include <kernel/event.h>
#define GET_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define IS_POWER_OF_2(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))

typedef struct {
    uint8_t *dataBufer;
    uint32_t size;
    uint32_t entrance;
    uint32_t exit;
} CQ_handleTypeDef;

extern bool  cq_init(CQ_handleTypeDef *CircularQueue, uint8_t *memAdd,
                     uint16_t len);
extern bool  cq_isEmpty(CQ_handleTypeDef *CircularQueue);
extern bool  cq_isFull(CQ_handleTypeDef *CircularQueue);
extern void  cq_emptyData(CQ_handleTypeDef *CircularQueue);
extern uint32_t cq_getLength(CQ_handleTypeDef *CircularQueue);
extern uint32_t cq_getData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf,
                           uint32_t len);
extern uint32_t cq_putData(CQ_handleTypeDef *CircularQueue,
                           const uint8_t *sourceBuf, uint32_t len);

#endif