/*
 * func_capt.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "cfg.h"
#include "board_diag.h"
#include "debug.h"
#include "func_capt.h"

const static char *pwm_export_chmod =
    "chmod 777 /sys/class/pwm/pwmchip0/export";

const static char *pwm_echo = "echo 0 > /sys/class/pwm/pwmchip0/export";

const static char *pwm_capture_chmod =
    "chmod 777 /sys/class/pwm/pwmchip0/pwm0/capture";

#define MAX_LEN  32
uint16_t period_buf[MAX_LEN];
uint16_t duty_buf[MAX_LEN];

bool pwm_capture_ops(test_exec_t *exec)
{
    bool ret = false;
    int capt_id = -1;
    int cnt = 0;
    volatile uint8_t row = 0;
    volatile uint8_t column = 0;
    static uint16_t sum = 0;
    static bool pwm_echo_flg = false;
    char buf[32] = {'\0'};
    struct pwm_capture result = {0};
    test_cmd_t *capt = (test_cmd_t *)exec->cmd;
    char square_wave[][16] = {{'\0'}, {'\0'}};

    if (pwm_echo_flg != true) {

        if (system(pwm_export_chmod) < 0) {
            ERROR("chmod export fail\n");
            goto out;
        }

        if (system(pwm_echo) < 0) {
            ERROR("echo export fail\n");
            goto out;
        }

        if (system(pwm_capture_chmod) < 0) {
            ERROR("chmod capture fail\n");
            goto out;
        }

        set_para_value(pwm_echo_flg, true);
    }

    capt_id = open("/sys/class/pwm/pwmchip0/pwm0/capture", O_RDWR);

    if (capt_id < 0) {
        ERROR("open pwm_capture fail\n");
        goto out;
    }

    cnt = read(capt_id, buf, sizeof(buf));

    if (cnt < 0 ) {
        ERROR("read pwm_capture fail\n");
        goto out;
    }

    for (uint8_t i = 0; i < cnt; i++) {
        if ((buf[i] != '\n') && (row < 2)) {
            if (buf[i] != ' ') {
                square_wave[row][column] = buf[i];
                column ++;
            }
            else {
                square_wave[row][column] = '\n';
                column = 0;
                row++;
            }
        }
    }

    result.period = atoi(square_wave[0]);
    result.duty_cycle = atoi(square_wave[1]);
    filter_move_step(sum, MAX_LEN, result.period, period_buf);
    filter_move_step(sum, MAX_LEN, result.duty_cycle, duty_buf);
    result.period = anti_inter_filter_algo(sum, period_buf);
    result.duty_cycle = anti_inter_filter_algo(sum, duty_buf);

    if (sum < MAX_LEN)
        sum++;

    set_para_value(ret, true);
out:

    if (capt_id >= 0)
        close(capt_id);

    /*channel*/
    set_para_value(exec->resp[1], (uint8_t)capt->route_channel_id);
    set_para_value(exec->resp[2], (uint8_t)(result.period >> 8) & 0xff);
    set_para_value(exec->resp[3], (uint8_t)(result.period & 0xff));
    set_para_value(exec->resp[4], (uint8_t)(result.duty_cycle >> 8) & 0xff);
    set_para_value(exec->resp[5], (uint8_t)(result.duty_cycle & 0xff));
    return ret;
}

bool capt_reply_deal(test_exec_t *exec, test_state_e state)
{
    bool ret = false;
    uint32_t respCanID;
    CMD_STATUS cmdStatus = CMD_PARA_ERR;

    if (state == STATE_SINGLE) {
        respCanID = SINGLE_RESP;
        ret = pwm_capture_ops(exec);
    }
    else {
        return ret;
    }

    if (ret)
        set_para_value(cmdStatus, NORMAL_DEAL);

    set_para_value(exec->resp[0], cmdStatus);

    common_response(exec, respCanID) ? (ret = true) : (ret = false);
    return ret;
}
