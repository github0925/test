/*
 * func_emmc.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include "string.h"
#include "remote_test.h"
#include "board_start.h"
#include "func_can.h"

/*emmc read uses remote call, which function in ap core*/
static bool _emmc_single_read(board_test_exec_t *exec)
{
    bool ret = false;
    can_cmd_t *emmc_cmd = (can_cmd_t *)exec->cmd;

    if (emmc_cmd->dev_id == g_step_case_table[EMMC_SERIAL_ID].cmd_id) {
        set_para_value(emmc_cmd->route_channel_id, READ_OPS);

        remote_test_send_req(emmc_cmd);

        if ((ret = remote_test_wait_resp(xTIME_OUT_TICKS, exec)) == true) {
            set_para_value(exec->resp[0], NORMAL_DEAL);
        }
        else {
            set_para_value(exec->resp[0], CMD_PARA_ERR);
        }
    }

    return ret;
}
/*emmc read by periodic*/
static bool _emmc_period_read(board_test_exec_t *exec)
{
    bool ret = false;
    set_para_value(exec->cmd[0], SUBCMD_EMMC);
    ret = _emmc_single_read(exec);
    set_para_value(exec->peridic_resp_id, PERIODIC_RESP_EMMC);

    return ret;
}
/*emmc process function start*/
bool board_emmc_reply_deal(board_test_exec_t *exec, board_test_state_e state)
{
    bool ret = false;

    if (state == STATE_SINGLE) {
        ret = _emmc_single_read(exec);
    }
    else if (state == STATE_PERIODIC) {
        ret = _emmc_period_read(exec);
    }

    set_para_value(exec->board_response, can_common_response);

    return ret;
}