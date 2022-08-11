
/*
 * func_i2c.c
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
#include "tca9539.h"
#include "board_cfg.h"
#include "func_i2c.h"
#include "i2c_hal.h"
#include "func_can.h"
#include <stdio.h>

extern const domain_res_t g_iomuxc_res;

#define get_i2c_write_level(x) ((x->standby_data1 & 0x80) ? 1 : 0)
#define get_i2c_read_write_mode(x) ((x & 0x80) ? 1 : 0)

#define PortConf_PIN_GPIO_E10 1
#define PortConf_PIN_GPIO_E11 1
/*I2C_3*/
static const Port_PinModeType PIN_GPIO_A4_I2C3_CLK = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT2),
};

static const Port_PinModeType PIN_GPIO_A5_I2C3_SDA  = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT2),
};
/*I2C_9*/
static const Port_PinModeType PIN_GPIO_D0_I2C9_CLK = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT1),
};

static const Port_PinModeType PIN_GPIO_D1_I2C9_SDA  = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT1),
};
#if 0
static i2c_dev_desc_t x5p49_desc = {
    0x0, 0x01, I2C_CHN_5, NULL, NULL //clock control
};
#endif
static i2c_dev_desc_t pf8200_desc = {
    0x1, 0x02, I2C_CHN_4, NULL, match_dev_type_pf8200 //pmic  control
};

static i2c_dev_desc_t tca9539_desc[] = {
    {0x0, 0x03, I2C_CHN_9, NULL, match_dev_type_tca9539}, //extern gpio  u3001   read    //prefor add 1, back reduce 1
    {0x1, 0x04, I2C_CHN_3, NULL, match_dev_type_tca9539}  //extern gpio  u3103   write   //prefor add 1, back reduce 1
};

i2c_chn_table_t i2c_chn_table;

struct tca9539_device *pd[ARRAY_SIZE(tca9539_desc)];

static bool i2c_dev_creat_handle(void **handle, uint8_t chn)
{
    bool ret = false;

    switch (chn) {
        case I2C_CHN_4:
            ret = hal_i2c_creat_handle(handle, RES_I2C_I2C4);
            break;

        case I2C_CHN_5:
            ret = hal_i2c_creat_handle(handle, RES_I2C_I2C5);
            break;

        default:
            break;
    }

    return ret;
}

void i2c_channel_init(void)
{
    void *port_handle;

    hal_port_creat_handle(&port_handle, g_iomuxc_res.res_id[0]);

    if (port_handle) {
        /* I2C3 */
        hal_port_set_pin_mode(port_handle, PortConf_PIN_GPIO_A4, PIN_GPIO_A4_I2C3_CLK);
        hal_port_set_pin_mode(port_handle, PortConf_PIN_GPIO_A5, PIN_GPIO_A5_I2C3_SDA);
        /* I2C9 */
        hal_port_set_pin_mode(port_handle, PortConf_PIN_GPIO_D0, PIN_GPIO_D0_I2C9_CLK);
        hal_port_set_pin_mode(port_handle, PortConf_PIN_GPIO_D1, PIN_GPIO_D1_I2C9_SDA);
        hal_port_release_handle(&port_handle);
    }
    else {
        ASSERT(false);
    }

    for (uint8_t num = 0; num < ARRAY_SIZE(tca9539_desc); num++) {
        pd[num] = tca9539_init(tca9539_desc[num].channel, 0x74);
        set_para_value(tca9539_desc[num].handle, pd[num]->i2c_handle);
    }

    if (i2c_dev_creat_handle(&pf8200_desc.handle, pf8200_desc.channel) != true) {
        ASSERT(false);
    }

    //if (i2c_dev_creat_handle(&x5p49_desc.handle, x5p49_desc.channel) != true) {
    //  ASSERT(false);
    //}
    /*U3001*/
    i2c_channel_to_read(0x0, 1);
    i2c_channel_to_read(0x0, 2);
    i2c_channel_to_read(0x0, 3);
    i2c_channel_to_read(0x0, 4);
    i2c_channel_to_read(0x0, 5);
    i2c_channel_to_read(0x0, 6);
    i2c_channel_to_read(0x0, 7);
    i2c_channel_to_read(0x0, 8);

    i2c_channel_to_read(0x0, 9);
    i2c_channel_to_read(0x0, 10);
    i2c_channel_to_read(0x0, 11);
    i2c_channel_to_read(0x0, 12);
    i2c_channel_to_read(0x0, 13);
    i2c_channel_to_read(0x0, 14);
    i2c_channel_to_read(0x0, 15);
    i2c_channel_to_read(0x0, 16);

    /*U3103*/
    i2c_channel_to_read(0x1, 1);
    i2c_channel_to_read(0x1, 2);
    i2c_channel_to_write(0x1, 3, HIGH);
    i2c_channel_to_write(0x1, 4, HIGH);
    i2c_channel_to_read(0x1, 5);
    i2c_channel_to_read(0x1, 6);
    i2c_channel_to_write(0x1, 7, HIGH);
    i2c_channel_to_write(0x1, 8, HIGH);

    i2c_channel_to_read(0x1, 9);
    i2c_channel_to_read(0x1, 10);
    i2c_channel_to_write(0x1, 11, HIGH);
    i2c_channel_to_write(0x1, 12, HIGH);
    i2c_channel_to_read(0x1, 13);
    i2c_channel_to_read(0x1, 14);
    i2c_channel_to_write(0x1, 15, HIGH);
    i2c_channel_to_write(0x1, 16, HIGH);

    //set_para_value(i2c_chn_table.i2c_dev_desc[0], &x5p49_desc);
    set_para_value(i2c_chn_table.i2c_dev_desc[1], &pf8200_desc);
    set_para_value(i2c_chn_table.i2c_dev_desc[2], &tca9539_desc[0]);
    set_para_value(i2c_chn_table.i2c_dev_desc[3], &tca9539_desc[1]);
}

uint8_t i2c_channel_to_read(uint8_t chn, uint8_t pin)
{
    uint8_t val = 0;

    if (pd[chn] == NULL) {
        dprintf(debug_show_dg, "no i2c_channel_handle");
        return 0;
    }
    else {
        pd[chn]->ops.input_enable(pd[chn], pin);
        val = pd[chn]->ops.input_val(pd[chn], pin);
    }

    return val;
}

bool i2c_channel_to_write(uint8_t chn, uint8_t pin, uint8_t val)
{
    bool ret = false;
    dprintf(debug_show_dg, "i2c_channel_to_write():\n");

    if (pd[chn] == NULL) {
        dprintf(debug_show_dg, "no i2c_channel_handle\n");
        return ret;
    }
    else {
        ret = true;
    }

    if ((pin >= 1) && (pin <= 16)) {
        ret = true;
    }
    else {
        return ret;
    }

    dprintf(debug_show_dg, "i2c_channel_to_write():chn=%d pin=%d, val=%d\n", chn,
            pin, val);
    pd[chn]->ops.output_enable(pd[chn], pin);
    pd[chn]->ops.output_val(pd[chn], pin, val);

    return ret;
}

static bool _i2c_single_opt(board_test_exec_t *exec, TCA9539_MODE mode)
{
    bool ret = false;
    CMD_STATUS cmdStatus = CMD_PARA_ERR;
    can_cmd_t *i2c_cmd = (can_cmd_t *)exec->cmd;
    i2cx_dev_t i2cx_dev;

    dprintf(debug_show_dg, "i2c into %d function\n", mode);

    if (i2c_cmd->dev_id == g_step_case_table[I2C_SERIAL_ID].cmd_id) {

        for (uint8_t num = 0; num < I2C_CHANNEL_NUM; num++) {

            if (i2c_cmd->route_channel_id != i2c_chn_table.i2c_dev_desc[num]->pin_num) {
                continue;
            }
            else {
                set_para_value(i2cx_dev.mode, mode);
                set_para_value(i2cx_dev.num, i2c_chn_table.i2c_dev_desc[num]->order);
                ret = i2c_chn_table.i2c_dev_desc[num]->dev_type(exec, i2cx_dev);//match_device
                break;
            }
        }
    }
    else {
        set_resp_err_state(exec->resp[0], cmdStatus);
        return ret;
    }

    if (ret != true) {
        set_resp_err_state(exec->resp[0], cmdStatus);
        return ret;
    }
    else {
        set_resp_err_state(cmdStatus, NORMAL_DEAL);
    }

    set_para_value(exec->resp[0], cmdStatus);
    set_para_value(exec->resp[1], i2c_cmd->route_channel_id);

    return ret;
}
/*i2c process function start*/
bool board_i2c_reply_deal(board_test_exec_t *exec, board_test_state_e state)
{
    bool ret = false;
    uint8_t write_read_flg = exec->cmd[3];

    if (state == STATE_SINGLE) {

        ret = get_i2c_read_write_mode(write_read_flg) ? _i2c_single_opt(
                  exec, READ) : _i2c_single_opt(exec, WRITE);

        set_para_value(exec->board_response, can_common_response);
    }

    return ret;
}
