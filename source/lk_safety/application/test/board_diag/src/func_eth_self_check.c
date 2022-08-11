/*
 * func_eth_self_check.c
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
#include "func_eth.h"
#include "board_start.h"
#include "remote_test.h"
#include "func_can.h"

/*ethernet self check*/
static bool _eth_self_check(board_test_exec_t *exec)
{
    bool ret = false;

    can_cmd_t *eth_check_cmd = (can_cmd_t *)exec->cmd;

    if (eth_check_cmd->dev_id == g_step_case_table[ETH_100BASE_T1_ID].cmd_id) {

        remote_test_send_req(eth_check_cmd);

        if (remote_test_wait_resp(xTIME_OUT_TICKS, exec) == true) {
            if (exec->resp[0] == NORMAL_DEAL) {
                set_para_value(ret, true);
            }
        }
    }

    return ret;
}

/*ethernet self check function process*/
bool board_eth_self_check_deal(board_test_exec_t *exec,
                               board_test_state_e state)
{
    bool ret = false;
    uint8_t cmdStatus = CMD_PARA_ERR;

    if (state == STATE_SINGLE) {
        ret = _eth_self_check(exec);

        if (ret)
            set_para_value(cmdStatus, NORMAL_DEAL);

        memset(exec->resp, 0, sizeof(exec->resp));
        set_para_value(exec->resp[0], SUBCMD_ETH_SELF_CHECK);
        set_para_value(exec->resp[1], cmdStatus);
        can_common_response(exec, SINGLE_RESP);
    }

    return ret;
}
