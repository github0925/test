/*
 * func_usb.c
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
#include "remote_test.h"

/*usb read by single*/
static bool _usb1_single_read(board_test_exec_t *exec)
{
    bool ret = false;

    can_cmd_t *usb_cmd = (can_cmd_t *)exec->cmd;

    if (usb_cmd->dev_id == g_step_case_table[USB1_SERIAL_ID].cmd_id) {

        remote_test_send_req(usb_cmd);

        if (remote_test_wait_resp(xTIME_OUT_TICKS, exec) == true) {
            if (exec->resp[0] == NORMAL_DEAL) {
                set_para_value(ret, true);
            }
        }
    }

    return ret;
}

/*usb read by periodic*/
static bool _usb1_period_read(board_test_exec_t *exec)
{
    bool ret = false;
    set_para_value(exec->cmd[0], SUBCMD_USB1);
    ret = _usb1_single_read(exec);
    set_para_value(exec->peridic_resp_id, PERIODIC_RESP_USB1);

    return ret;
}

/*usb process function start*/
bool board_usb1_reply_deal(board_test_exec_t *exec, board_test_state_e state)
{
    bool ret = false;
    uint8_t cmdStatus = CMD_PARA_ERR;

    if (state == STATE_PERIODIC) {
        ret = _usb1_period_read(exec);

        if (ret)
            set_para_value(cmdStatus, NORMAL_DEAL);

        set_para_value(exec->resp[0], cmdStatus);
        set_para_value(exec->board_response, can_common_response);
    }

    return ret;
}
