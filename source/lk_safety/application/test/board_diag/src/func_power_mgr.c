/*
 * func_power_mgr.c
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
#include "board_cfg.h"
#include "func_can.h"
#include "func_dio.h"
#include "func_i2c.h"
#include "func_power.h"
#include "remote_test.h"
#ifdef PWOER_OFF
/*TODO:if power off is different from sleep mode*/
extern bool get_system_key_state(void);
static void system_power_off_exec(void)
{
    com_gpio_set_system_power_mgr(POWER_OFF);
    i2cx_gpio_set_system_power_mgr(POWER_OFF);
}

static bool system_power_off_single_cmd(board_test_exec_t *exec)
{
    bool ret = false;
    uint8_t cmdStatus = CMD_PARA_ERR;
    can_cmd_t *power_off_cmd = (can_cmd_t *)exec->cmd;

    if (power_off_cmd->dev_id != g_step_case_table[POWER_OFF_SERIAL_ID].cmd_id) {
        set_resp_err_state(exec->resp[0], cmdStatus);
        dprintf(debug_show_dg, "not find dev_id\n");
        return ret;
    }

    if (get_system_key_state() == ON_LINE) {
        set_resp_err_state(exec->resp[0], cmdStatus);
        dprintf(debug_show_dg, "get_system_kl15_state on line\n");
        return ret;
    }
    else {
        set_para_value(ret, true);
        set_para_value(cmdStatus, NORMAL_DEAL);
    }

    set_para_value(exec->resp[0], cmdStatus);

    return ret;
}
#endif

bool board_power_off_reply_deal(board_test_exec_t *exec,
                                board_test_state_e state)
{
    bool ret = false;

    if (state == STATE_SINGLE) {
        exec->cmd[0] = SUBCMD_SLEEP;
        ret = board_sleep_reply_deal(exec, state);
    }

    return ret;
}
