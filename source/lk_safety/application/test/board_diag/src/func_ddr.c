/*
 * func_ddr.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include <stdbool.h>
#include "string.h"
#include "board_start.h"
#include "func_can.h"
#include "board_cfg.h"
#include "crc.h"

#define DDR_LEN  1024
static uint8_t pt[DDR_LEN];
static uint8_t temp[DDR_LEN];

static bool ddr_write(uint8_t *pt, uint32_t len)
{
    bool ret = false;

    if (pt == NULL)
        return ret;

    if (len == 0)
        return ret;

    ret = true;

    for (uint32_t i = 0; i < len; i++) {
        *(pt + i) = i;

    }

    return ret;
}

static bool ddr_read(uint8_t *st, uint8_t *dt, uint32_t len)
{
    bool ret = false;

    if (st == NULL || dt == NULL)
        return ret;

    if (len == 0)
        return ret;

    ret = true;

    for (uint32_t i = 0; i < len; i++) {
        *(dt + i) = *(st + i);
    }

    return ret;
}

/*ddr into check function*/
bool ddr_check_value_result(uint8_t *pt, uint8_t *temp, uint32_t len)
{
    bool ret = false;
    uint16_t crc_read, crc_wrire;

    if (ddr_write(pt, len) == true) {
        crc_wrire = crcchks(pt, len);
    }
    else
        return ret;

    if (ddr_read(pt, temp, len) == true) {
        crc_read = crcchks(temp, len);
    }
    else
        return ret;

    if (crc_wrire == crc_read) {
        set_para_value(ret, true);
    }

    return ret;
}
/*ddr read by single*/
static bool _ddr_single_read(board_test_exec_t *exec)
{
    bool ret = false;

    if (exec->cmd[0] == g_step_case_table[DDR_SERIAL_ID].cmd_id) {

        ret = ddr_check_value_result(pt, temp, DDR_LEN);
    }

    if (ret == true)
        set_para_value(exec->resp[0], NORMAL_DEAL);
    else
        set_para_value(exec->resp[0], CMD_PARA_ERR);

    return ret;
}
/*ddr read by periodic*/
static bool _ddr_period_read(board_test_exec_t *exec)
{
    bool ret = false;
    set_para_value(exec->cmd[0], SUBCMD_DDR);
    ret = _ddr_single_read(exec);
    set_para_value(exec->peridic_resp_id, PERIODIC_RESP_DDR);

    return ret;
}
/*ddr process function start*/
bool board_ddr_reply_deal(board_test_exec_t *exec, board_test_state_e state)
{
    bool ret = false;

    if (state == STATE_SINGLE) {
        ret = _ddr_single_read(exec);
    }
    else if (state == STATE_PERIODIC) {
        ret = _ddr_period_read(exec);
    }

    set_para_value(exec->board_response, can_common_response);

    return ret;
}
