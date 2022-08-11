/*
 * dqueue.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#ifndef DQUEUE_H_
#define DQUEUE_H_

#include <__regs_base.h>
#include <lib/console.h>
#include <kernel/event.h>
#include "cqueue.h"

#define DQ_SIZE 512

#define DQ_HEAD_1 0xAA
#define DQ_HEAD_2 0x55

typedef struct {
    CQ_handleTypeDef *cqueue;
    uint8_t mem[DQ_SIZE];
    uint32_t dataInSum;
} dqueue_t;

extern uint32_t dq_getData(dqueue_t *dq, uint8_t *target);
extern uint32_t dq_pushData(dqueue_t *dq, const uint8_t *target, uint32_t size);
extern bool dq_init(dqueue_t *dq);
extern void queue_init(void);

extern dqueue_t *get_dqueue_can_handle(void);
extern CQ_handleTypeDef *get_cqueue_can_handle(void);
extern dqueue_t *get_dqueue_lin_handle(void);
extern CQ_handleTypeDef *get_cqueue_lin_handle(void);
#endif /* DQUEUE_H_ */