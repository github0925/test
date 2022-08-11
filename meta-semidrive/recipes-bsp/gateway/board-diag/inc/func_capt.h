/*
 * func_capt.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef FUNC_CAPT_H_
#define FUNC_CAPT_H_
#include "board_diag.h"

struct pwm_capture {
    uint16_t period;
    uint16_t duty_cycle;
};

extern bool capt_reply_deal(test_exec_t *exec, test_state_e state);

#endif

