/*
 * ioexpend_tca9539_dev.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: tca9539_dev
 *
 * Revision History:
 * -----------------
 */
#include "board_cfg.h"
#include "board_start.h"
#include "func_i2c.h"
#include "func_can.h"

static int8_t tca9535_port_offset_correct(uint8_t pn)
{
    if ((pn >= 0) && (pn <= 7)) {
        return (pn + 1);
    }
    else if ((pn >= 10) && (pn <= 17)) {
        return (pn - 1);
    }

    return -1;
}

static bool read_tca9539(board_test_exec_t *exec, i2cx_dev_t i2cx_dev)
{
    bool ret = false;
    uint8_t result;
    can_cmd_t *i2c_cmd = (can_cmd_t *)exec->cmd;
    uint8_t port_num = i2c_cmd->standby_data4;

    if (port_num <= taca9535_pin_max) {

        if (tca9535_port_offset_correct(port_num) == -1)
            return ret;

        port_num = tca9535_port_offset_correct(port_num);

        result = i2c_channel_to_read(i2cx_dev.num, port_num);

        dprintf(debug_show_dg, "return result=0x%x\n", result);

        if ((result != 0) && (result != 1))
            return ret;

        set_para_value(exec->resp[2], result);
        set_para_value(exec->resp[1], i2c_cmd->standby_data3);
        set_para_value(ret, true);
    }

    return ret;
}

static bool write_tca9539(board_test_exec_t *exec, i2cx_dev_t i2cx_dev)
{
    bool ret = false;
    can_cmd_t *i2c_cmd = (can_cmd_t *)exec->cmd;
    uint8_t port_num = i2c_cmd->standby_data4;
    uint8_t result = i2c_cmd->standby_data2;

    if ((result != 0) && (result != 1))
        return ret;

    if (port_num <= taca9535_pin_max) {

        if (tca9535_port_offset_correct(port_num) == -1)
            return ret;

        port_num = tca9535_port_offset_correct(port_num);
        i2c_channel_to_write(i2cx_dev.num, port_num, result);
        set_para_value(exec->resp[1], i2c_cmd->standby_data3);
        set_para_value(exec->resp[2], result);
        set_para_value(ret, true);
    }

    return ret;
}

bool match_dev_type_tca9539(board_test_exec_t *exec, i2cx_dev_t i2cx_dev)
{
    bool ret = false;

    if (i2cx_dev.mode == READ)
        ret = read_tca9539(exec, i2cx_dev);
    else if (i2cx_dev.mode == WRITE)
        ret = write_tca9539(exec, i2cx_dev);

    return ret;
}