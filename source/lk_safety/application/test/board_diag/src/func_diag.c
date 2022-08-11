/*
 * func_diag.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include "board_start.h"
#include "func_can.h"
#include "board_cfg.h"

static bool _diag_period_read(board_test_exec_t *exec)
{
    bool ret = false;
    uint8_t cmdStatus = NORMAL_DEAL;

    set_para_value(exec->resp[0], cmdStatus);

    return ret;
}

bool board_diag_reply_deal(board_test_exec_t *exec, board_test_state_e state)
{
    bool ret = false;

    set_para_value(exec->board_response, NULL);

    if (state == STATE_PERIODIC) {
        ret = _diag_period_read(exec);
    }

    return ret;
}