/*
 * func_gpio.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <gpiod.h>
#include <stdio.h>
#include <string.h>

#include "board_diag.h"
#include "debug.h"
#include "cfg.h"
#include "gpio-ops.h"

#define END_OF_TABLE(x) (x->gpio_Pin == PIN_NUM_INVALID && x->pin_num == PIN_NUM_INVALID)

/*
 * @return true if the gpio has been found, false if not found
 */
static bool gpio_single_read(test_exec_t *exec)
{
    bool found = false;
    bool opt_ret = false;
    uint8_t result = 0;
    CMD_STATUS cmdStatus = CMD_PARA_ERR;
    test_cmd_t *gpio_cmd = (test_cmd_t *)exec->cmd;
    const gpio_value_table_t *gpio = gpio_read_value_table;


    for (; !END_OF_TABLE(gpio); gpio++) {

        if (gpio_cmd->route_channel_id != gpio->pin_num
                || gpio->gpio_Pin == PIN_NUM_INVALID) {
            continue;
        }

        opt_ret = gpio_read(gpio->gpio_Pin, &result);

        cmdStatus = opt_ret ? NORMAL_DEAL : CMD_PARA_ERR;

        found = true;
        break;
    }

    if (found && !opt_ret) {
        set_para_value(exec->resp[0], cmdStatus);
        return found;
    }

    set_para_value(exec->resp[0], cmdStatus);
    set_para_value(exec->resp[1], gpio_cmd->route_channel_id);
    set_para_value(exec->resp[2], result);

    return found;
}

static bool gpio_period_read(test_exec_t *exec)
{
    bool found = false;
    uint8_t val = 0;
    uint32_t result = 0;
    const gpio_value_table_t *gpio = gpio_read_value_table;
    CMD_STATUS cmdStatus = NORMAL_DEAL;

    for (uint8_t num = 0; !END_OF_TABLE(gpio); num++, gpio++) {

        DBG("num:%u gpio:%p pin:%u\n", num, gpio, gpio->pin_num);

        if (gpio->gpio_Pin == PIN_NUM_INVALID) {
            continue;
        }
        else {
            gpio_read(gpio->gpio_Pin, &val);
            val ? (result |=  (1 << num)) : (result &= ~(1 << num));
            found = true;
        }
    }

    set_para_value(exec->resp[0], cmdStatus);
    set_para_value(exec->resp[1], result & 0xff);
    set_para_value(exec->resp[2], (result >> 8) & 0xff);
    set_para_value(exec->resp[3], (result >> 16) & 0xff);
    set_para_value(exec->resp[4], (result >> 24) & 0xff);

    return found;
}

/*
 * @return true if the gpio has been found, false if not found
 */
static bool gpio_single_write(test_exec_t *exec)
{
    bool found = false;
    bool opt_ret = false;
    const gpio_value_table_t *gpio = gpio_write_value_table;
    CMD_STATUS cmdStatus = CMD_PARA_ERR;

    test_cmd_t *gpio_cmd = (test_cmd_t *)exec->cmd;

    memset(exec->resp, 0, sizeof(exec->resp));

    for (; !END_OF_TABLE(gpio); gpio++) {

        if (gpio_cmd->route_channel_id != gpio->pin_num
                || gpio->gpio_Pin == PIN_NUM_INVALID) {
            continue;
        }
        else {
            DBG("pin:%u  val:%u\n",
                gpio_cmd->route_channel_id, gpio_cmd->recv_data);
            opt_ret = gpio_write(gpio->gpio_Pin,
                                 gpio_cmd->recv_data);

            if (opt_ret)
                cmdStatus = NORMAL_DEAL;

            found = true;
            break;
        }
    }

    if (found && !opt_ret) {
        set_para_value(exec->resp[0], cmdStatus);
        return found;
    }

    set_para_value(exec->resp[0], cmdStatus);
    set_para_value(exec->resp[1], gpio_cmd->route_channel_id);
    set_para_value(exec->resp[2], gpio_cmd->recv_data);

    return found;
}

/*
 * gpio read
 */
bool gpio_read_reply_deal(test_exec_t *exec, test_state_e state)
{
    bool found = false;
    uint8_t ret = false;
    uint32_t respCanID;

    if (state == STATE_SINGLE) {
        respCanID = SINGLE_RESP;
        found = gpio_single_read(exec);
    }
    else if (state == STATE_PERIODIC) {
        respCanID = PERIODIC_RESP_GPIO;
        found = gpio_period_read(exec);
    }
    else {
        return ret;
    }

    if (found)
        common_response(exec, respCanID) ? (ret = true) : (ret = false);

    return ret;
}

/*
 * gpio set
 */
bool gpio_write_reply_deal(test_exec_t *exec, test_state_e state)
{
    bool found = false;
    uint8_t ret = false;
    uint32_t respCanID;

    if (state == STATE_SINGLE) {
        respCanID = SINGLE_RESP;
        found = gpio_single_write(exec);
    }
    else {
        return ret;
    }

    DBG("found:%d\n", found);

    if (found)
        common_response(exec, respCanID) ? (ret = true) : (ret = false);

    return ret;
}
