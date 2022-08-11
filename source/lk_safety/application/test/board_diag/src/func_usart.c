/*
 * func_usart.c
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

static bool _uart_single_read(board_test_exec_t *exec)
{
    bool ret = false;
    uint8_t cmdStatus = CMD_PARA_ERR;
    can_cmd_t *uart_cmd = (can_cmd_t *)exec->cmd;

    if (uart_cmd->dev_id == g_step_case_table[UART_SERIAL_ID].cmd_id) {
        cmdStatus = NORMAL_DEAL;
        ret = true;
    }
    else {
        cmdStatus = CMD_PARA_ERR;
        ret = false;
    }

    set_para_value(exec->resp[0], cmdStatus);
    set_para_value(exec->resp[1], uart_cmd->route_channel_id);
    set_para_value(exec->resp[2], uart_cmd->recv_data + 1);

    return ret;
}

bool board_uart_reply_deal(board_test_exec_t *exec, board_test_state_e state)
{
    bool ret = false;

    if (state == STATE_SINGLE) {
        _uart_single_read(exec);
        set_para_value(exec->board_response, can_common_response);
        set_para_value(ret, true);
    }

    return ret;
}