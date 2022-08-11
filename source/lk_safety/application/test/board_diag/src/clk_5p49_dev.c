/*
 * clk_5945_dev.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: clk_5945_dev.c
 *
 * Revision History:
 * -----------------
 */
#include "board_start.h"
#include "func_i2c.h"
#include <stdio.h>
#include <string.h>
#include "i2c_hal.h"
#include "board_cfg.h"
#include "func_can.h"

#define IDT_5P49V60_SLAVE_ID 0x6a

typedef struct {
    uint8_t num;
    uint8_t reg;
    uint8_t val;
} clk_5949_reg_t;

static clk_5949_reg_t clk_5949_reg[] = {
    {0x0, 0x31, 0x81},//Output Divider 2 Control Register Settings
    {0x1, 0x3a, 0x04},//Output Divider 2 Spread Modulation Rate Configuring Register
    {0x2, 0x3e, 0xe0},//Output Divider 2 Integer Part
    {0x3, 0x62, 0xbb},//Clock2 Output
    {0x4, 0x63, 0x01},//Clock2 Output Configuration
    {0x5, 0x41, 0x81},//Output Divider 3 Control Register Settings
    {0x6, 0x4a, 0x04},//Output Divider 3 Spread Modulation Rate Configuring Register
    {0x7, 0x4e, 0xe0},//Output Divider 3 Integer Part
    {0x8, 0x64, 0xbb},//Clock3 Output
    {0x9, 0x65, 0x01}//Clock3 Output Configuration
};

static bool clk_5p49v60_read_reg(clk_5949_reg_t *clk_5949_reg, void *handle)
{
    bool ret = false;

    ret = hal_i2c_read_reg_data(handle, IDT_5P49V60_SLAVE_ID, &clk_5949_reg->reg, 1,
                                &clk_5949_reg->val, 1);

    return ret;
}

static bool monitor_clock_frequency(board_test_exec_t *exec, void *handle)
{
    bool ret = false;
    can_cmd_t *x5p49_cmd = (can_cmd_t *)exec->cmd;

    for (uint8_t num = 0; num < ARRAY_SIZE(clk_5949_reg); num++) {

        if (x5p49_cmd->recv_data != clk_5949_reg[num].num) {
            continue;
        }
        else {
            ret = clk_5p49v60_read_reg(&clk_5949_reg[num], handle);
            set_para_value(exec->resp[2], clk_5949_reg[num].val);
        }
    }

    return ret;
}

static bool clk_5949_dev_opt(board_test_exec_t *exec, i2cx_dev_t i2cx_dev)
{
    bool ret = false;
    void *handle = i2c_chn_table.i2c_dev_desc[i2cx_dev.num]->handle;

    ret = monitor_clock_frequency(exec, handle);

    return ret;
}

bool match_dev_type_5949(board_test_exec_t *exec, i2cx_dev_t i2cx_dev)
{
    bool ret = false;

    ret = clk_5949_dev_opt(exec, i2cx_dev);

    return ret;
}