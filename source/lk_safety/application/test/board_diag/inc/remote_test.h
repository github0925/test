/*
 * ap_safety_mail.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#ifndef __REMOTE_TEST_H_
#define __REMOTE_TEST_H_

#include "board_cfg.h"
#include "func_can.h"
#include "board_start.h"

typedef struct {
#define EMMC_TIME_OUT_TICKS     32000000
#define DDR_TIME_OUT_TICKS      5000
#define FLASH_TIME_OUT_TICKS    600000

#define xTIME_OUT_TICKS         5000
#define AP_TO_SAFETY_MESSG_LEN  8
    spin_lock_t lock;
    bool xmutex_flg;
    bool xqueue_flg;
    SemaphoreHandle_t xMutex;
    xQueueHandle MsgQueue;
    can_resp_t MsgBuf;
    CAN_CHN_STATE ap_resp_state;
} ap_safety_mail_t;

extern bool remote_require_for_gpio_value(board_test_exec_t *exec);
extern bool remote_test_queue_creat(void);
extern bool remote_test_send_req(can_cmd_t *cmdx);
extern bool remote_test_wait_resp(uint32_t time_out, board_test_exec_t *exec);
extern bool remote_test_resp_cb(const uint8_t *CanSduPtr);
#endif