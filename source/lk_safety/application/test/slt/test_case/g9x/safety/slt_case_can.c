/*
 * Copyright (c) 2019, SemiDrive, Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <app.h>
#include <lib/console.h>
#include <kernel/event.h>
#include <lk_wrapper.h>

#include "Can.h"
#include "event_groups.h"
#include <string.h>
#undef ERPC_TYPE_DEFINITIONS
#include "can_cfg.h"
#include <lib/slt_module_test.h>

#define CAN_ID       0x327
#define CAN_MSG_LEN  8

#define CONTEXT_LOCK(context) \
    spin_lock_saved_state_t __state; \
    spin_lock_irqsave(&context.lock, __state)

#define CONTEXT_UNLOCK(context) \
    spin_unlock_irqrestore(&context.lock, __state)

extern struct Can_MBConfig gCan_RxMBCfg[];
extern struct Can_MBConfig gCan_TxMBCfg[];

typedef struct {
    uint32_t canId;
    uint8_t  channel;
    uint8_t *txdata;
    uint8_t *rxdata;
    bool result;
} can_data_t;

typedef struct {
    event_t  event;
    spin_lock_t lock;
    can_data_t can_data;
    struct Can_ControllerConfig *ControllerConfig;
} can_context_t;

static can_context_t can_context;

uint8_t can_tx_data[CAN_MSG_LEN] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};
uint8_t can_rx_data[CAN_MSG_LEN] = {0};

static uint8_t can_channel_send(can_context_t *can_context)
{
    uint32_t can_id = can_context->can_data.canId;
    uint8_t  chn = can_context->can_data.channel;
    uint8_t *data = can_context->can_data.txdata;

    Can_PduType pdu = {
        .swPduHandle = 0,
        .length = 8U,
        .id = can_id,
        .sdu = data,
    };

    return Can_Write(chn, &pdu);
}

static void slt_can_msg_init(can_context_t *can_context)
{
    can_context->lock = SPIN_LOCK_INITIAL_VALUE;
    can_context->can_data.canId  = CAN_ID;
    can_context->can_data.txdata = can_tx_data;
    can_context->can_data.rxdata = can_rx_data;
    can_context->can_data.result = false;
    event_init(&can_context->event, false, EVENT_FLAG_AUTOUNSIGNAL);
}

static int can_loop_check_reslut(can_context_t *can_context)
{
    int ret = -1;

    for (uint8_t i = CAN1; i <= CAN20; i++) {
        if (can_context->can_data.rxdata[0] == can_context->can_data.txdata[0]) {
            printf("can%d pass!\n", i);
        }
        else {
            printf("can%d fail!\n", i);
            goto out;
        }
    }

    ret = 0;

out:
    return ret;
}

static int slt_can_SendFrame(can_context_t *can_context)
{
    int ret = 1;
    can_channel_send(can_context);
    event_wait(&can_context->event);
    return ret;
}

static void slt_can_init(can_context_t *can_context)
{
    slt_can_msg_init(can_context);
}

/**
 * @brief slt_diag_can_rxind callback.
 *
 * This function is called in interrupt context.
 */
void slt_diag_can_rxind(uint16_t Hrh, Can_IdType CanId,
                        uint8_t CanDlc, const uint8_t *CanSduPtr)
{
    memcpy(can_rx_data, CanSduPtr, CanDlc);
    event_signal(&can_context.event, true);
    printf("slt_diag_can rx channel is %d\n", Hrh);
}

static int slt_can_internal_ip_diagnose(can_context_t *can_context)
{
    int ret = -1;

    for (uint8_t nr = CAN1; nr <= CAN20; nr++) {
        can_context->can_data.channel = nr;
        slt_can_init(can_context);
        ret = slt_can_SendFrame(can_context);
    }

    return ret;
}

int TEST_SEC_SS_11(uint times, uint timeout, char *result_string)
{
    int ret = -1;
    slt_can_internal_ip_diagnose(&can_context);
    ret = can_loop_check_reslut(&can_context);
    return ret;
}

// test case name: module_test_sample1
// test case entry: slt_module_test_sample_hook_1
// test case level: SLT_MODULE_TEST_LEVEL_SAMPLE_1(must define in enum slt_module_test_level)
// test case parallel: test parallel with other test case
// test case time: run test times in test case, if not support set default 1
// test case timeout: run test timeout default value, must bigger than case us time, or case will be kill
// test case flag: usd user define stack size
// test case stack size: user define stack size
SLT_MODULE_TEST_HOOK_DETAIL(sec_ss_11, TEST_SEC_SS_11,
                            SLT_MODULE_TEST_LEVEL_SAMPLE_1, 0, 1, 1000, 0, 0);


