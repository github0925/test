/*
 * func_usb.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: func_release_vresion.c
 *
 * Revision History:
 * -----------------
 */
#include "board_start.h"
#include "func_can.h"
#include "remote_test.h"

#define RELEASE_VERSION_ID 0x0450

/*_release_vresion_single_read by single*/
static bool _release_vresion_single_read(board_test_exec_t *exec)
{
    bool ret = false;

    can_cmd_t *version_cmd = (can_cmd_t *)exec->cmd;

    if (version_cmd->dev_id == g_step_case_table[VERSION_SERIAL_ID].cmd_id) {
        set_para_value(exec->resp[1], (RELEASE_VERSION_ID >> 8) & 0xff);
        set_para_value(exec->resp[2],  RELEASE_VERSION_ID & 0xff);
        set_para_value(ret, true);
    }

    return ret;
}

/*board_release_version_deal function start*/
bool board_release_vresion_deal(board_test_exec_t *exec,
                                board_test_state_e state)
{
    bool ret = false;
    uint8_t cmdStatus = CMD_PARA_ERR;

    if (state == STATE_SINGLE) {
        ret = _release_vresion_single_read(exec);

        if (ret)
            set_para_value(cmdStatus, NORMAL_DEAL);

        set_para_value(exec->resp[0], cmdStatus);
        set_para_value(exec->board_response, can_common_response);
    }

    return ret;
}





