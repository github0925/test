/*
 * func_sleep_mgr.c
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

extern bool get_system_key_state(void);

static void system_sleep_exec(void)
{
    com_gpio_set_system_power_mgr(POWER_OFF);
    i2cx_gpio_set_system_power_mgr(POWER_OFF);
}

static bool system_sleep_single_cmd(board_test_exec_t *exec)
{
    bool ret = false;
    uint8_t cmdStatus = CMD_PARA_ERR;
    can_cmd_t *sleep_cmd = (can_cmd_t *)exec->cmd;

    if (sleep_cmd->dev_id != g_step_case_table[SLEEP_SERIAL_ID].cmd_id) {
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
        ret = true;
        cmdStatus = NORMAL_DEAL;
    }

    set_para_value(exec->resp[0], cmdStatus);

    return ret;
}

inline static bool native_board_eth_into_sleep_config(board_test_exec_t *exec)
{
    bool ret = false;

    can_cmd_t eth_sleep = {.dev_id = 0x3f};

    remote_test_send_req(&eth_sleep);

    if ((ret = remote_test_wait_resp(xTIME_OUT_TICKS, exec)) != true)
        goto fail;

    if ((exec->resp[0] != 0xa0) || \
            (exec->resp[1] != 0x3f))
        goto fail;

    ret = true;

fail:
    return ret;
}

inline static bool change_native_board_eth_master_slave_role(
    board_test_exec_t *exec)
{
    bool ret = false;

    can_cmd_t ethMasterSlaveRole = {
        .dev_id = 0x36,
        .route_channel_id = 0x1,
        .recv_data = 0,
        .standby_data1 = 1,
    };

    remote_test_send_req(&ethMasterSlaveRole);

    if ((ret = remote_test_wait_resp(xTIME_OUT_TICKS, exec)) != true)
        goto fail;

    if (exec->resp[0] != 0xa0)
        goto fail;

    ret = true;

fail:
    return ret;
}

static bool opposite_board_eth_into_sleep_config(board_test_exec_t *exec)
{
    bool ret = false;
    canx_opt_t canx_opt;

    set_para_value(canx_opt.can_send, (can_send_t *)canx_opt.pay_load);
    set_para_value(canx_opt.can_send->chn_num, CAN_CHN_12);
    set_para_value(canx_opt.can_send->send_data, 0x5a);
    set_para_value(canx_opt.can_send->standby_data1, 0xa5);
    set_para_value(canx_opt.resp_chn_id, CAN_CHN_12 - CAN_SER_OFFSET);
    ret = can_channel_to_write(&canx_opt, 0x612);
    return ret;
}

static bool __check_eth_inh_level(board_test_exec_t *cmd)
{
    bool     ret = false;
    uint8_t  val = HIGH;
    uint16_t cnt = 10;

    do {
        /*remote call for ap core*/
        if (remote_require_for_gpio_value(cmd) == true) {
            val = cmd->resp[2];
        }
        else {
            goto fail;
        }

        cnt--;
        thread_sleep(1);
        printf("val = %d\n", val);
    }
    while ((val == HIGH) && (cnt > 0));

    if (cnt == 0)
        goto fail;

    ret = true;
fail:

    return ret;
}

static bool check_system_into_sleep_condition(void)
{
    bool ret = false;
    board_test_exec_t cmd;
    cmd.cmd[0] = SUBCMD_GPIO_R;
    /*5072_INH*/
    cmd.cmd[1] = 0x21;
    ret = __check_eth_inh_level(&cmd);

    if (ret != true) {
        dprintf(debug_show_dg, "5072_INH ERR\n");
        goto fail;
    }

    /*5050_INH*/
    cmd.cmd[1] = 0x22;
    ret = __check_eth_inh_level(&cmd);

    if (ret != true) {
        dprintf(debug_show_dg, "5050_INH ERR\n");
        goto fail;
    }

    /*2122_P10*/
    cmd.cmd[1] = 0x25;
    ret = __check_eth_inh_level(&cmd);

    if (ret != true) {
        dprintf(debug_show_dg, "2122_P10 ERR\n");
        goto fail;
    }

    /*2122_P7*/
    cmd.cmd[1] = 0x28;
    ret =  __check_eth_inh_level(&cmd);

    if (ret != true) {
        dprintf(debug_show_dg, "2122_P7 ERR\n");
        goto fail;
    }

    /*2122_P9*/
    cmd.cmd[1] = 0x29;
    ret = __check_eth_inh_level(&cmd);

    if (ret != true) {
        dprintf(debug_show_dg, "2122_P9 ERR\n");
        goto fail;
    }

fail:
    return ret;
}

bool board_sleep_reply_deal(board_test_exec_t *exec, board_test_state_e state)
{
    bool ret = false;

    if (state == STATE_SINGLE) {
        if (system_sleep_single_cmd(exec) != true)
            goto fail;

        if ((ret = native_board_eth_into_sleep_config(exec)) != true) {
            dprintf(debug_show_dg, "native_eth_sleep err\n");
            goto fail;
        }

        if ((ret = opposite_board_eth_into_sleep_config(exec)) != true) {
            dprintf(debug_show_dg, "opposite_eth_sleep err\n");
            goto fail;
        }

#ifdef CHECK_SLEEP_CONDITION

        /*TODO:checkout eth inh pin value signal*/
        if ((ret = check_system_into_sleep_condition()) != true) {
            dprintf(debug_show_dg, "check_system_sleep err\n");
            goto fail;
        }

#endif

        if ((ret = change_native_board_eth_master_slave_role(exec)) != true) {
            dprintf(debug_show_dg, "change_native_eth_master_slave err\n");
            goto fail;
        }

        printf("change_native_board_eth_master_slave_role ok\n\n");
        system_sleep_exec();
        set_system_wakeup();
        set_system_powerdown();
    }

fail:
    set_para_value(exec->resp[0], CMD_PARA_ERR);
    set_para_value(exec->board_response, can_common_response);
    return ret;
}
