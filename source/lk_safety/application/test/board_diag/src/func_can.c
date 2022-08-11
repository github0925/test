/*
 * func_can.c
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
#include <string.h>
#include <hal_dio.h>
#include <Can.h>
#include "board_start.h"
#include "dqueue.h"
#include "cqueue.h"
#include "board_cfg.h"
#include "func_can.h"
#include "remote_test.h"

const static can_chn_table_t can_chn_table[] = {

    {0x01, 0x601, 0x701},
    {0x02, 0x602, 0x702},
    {0x03, 0x603, 0x703},
    {0x04, 0x604, 0x704},
    {0x09, 0x609, 0x709},
    {0x0a, 0x610, 0x710},
    {0x0b, 0x611, 0x711},
    {0x0c, 0x612, 0x712},
    {0x0d, 0x613, 0x713},
    {0x0e, 0x614, 0x714},
    {0x0f, 0x615, 0x715},
    {0x10, 0x616, 0x716},
    {0x11, 0x617, 0x717},
    {0x12, 0x618, 0x718},
    {0x13, 0x619, 0x719},
    {0x14, 0x620, 0x720}
};

can_chn_condition_t can_chn_condition[] = {
    {
        .channel = CAN_CHN_1,
        .result  = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_2,
        .result  = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_3,
        .result  = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_4,
        .result  = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_9,
        .result  = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_10,
        .result  = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_11,
        .result = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_12,
        .result = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_13,
        .result = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_14,
        .result = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_15,
        .result = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_16,
        .result = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_17,
        .result = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_18,
        .result = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_19,
        .result  = _INVALID,
        .can_chn_sta = _IDLE
    },
    {
        .channel = CAN_CHN_20,
        .result  = _INVALID,
        .can_chn_sta = _IDLE
    },
};
/*storing can channel value which just by DTU sending to DTU'*/
void push_can_info_into_table(can_cmd_t *can_cmd)
{
    uint8_t chn = can_cmd->route_channel_id;
    uint8_t val = can_cmd->recv_data;

    for (uint8_t num = 0; num < ARRAY_SIZE(can_chn_condition); num++) {

        if (chn == can_chn_condition[num].channel) {
            CONTEXT_LOCK(canx_app);
            set_para_value(can_chn_condition[num].can_chn_sta, _SEND);
            set_para_value(can_chn_condition[num].send_data, val);
            CONTEXT_UNLOCK(canx_app);
        }
    }
}
/*receiving DTU' can channel value and check right or not */
bool get_can_info_from_table(board_test_exec_t *exec)
{
    bool ret = false;
    can_recv_t *can_recv = (can_recv_t *)exec->cmd;
    can_resp_t *can_resp = (can_resp_t *)exec->resp;

    uint8_t chn = can_recv->chn_num;
    uint8_t val = can_recv->recv_data;

    for (uint8_t num = 0; num < ARRAY_SIZE(can_chn_condition); num++) {

        if (chn == can_chn_condition[num].channel) {

            if (can_chn_condition[num].can_chn_sta == _SEND) {

                if (val == (can_chn_condition[num].send_data + 1)) {

                    set_para_value(can_chn_condition[num].can_chn_sta, _IDLE);
                    set_para_value(can_resp->cmd_status, NORMAL_DEAL);
                    set_para_value(can_resp->route_channel_id, chn);
                    set_para_value(can_resp->send_data, val);
                    set_para_value(ret, true);
                    return ret;
                }
                else {
                    set_para_value(can_resp->cmd_status, CMD_PARA_ERR);
                    set_para_value(can_chn_condition[num].can_chn_sta, _ERR);
                    return ret;
                }
            }
            else {
                set_para_value(can_chn_condition[num].can_chn_sta, _IDLE);
                set_para_value(can_resp->cmd_status, CMD_PARA_ERR);
                return ret;
            }
        }
    }

    set_para_value(can_resp->cmd_status, CMD_PARA_ERR);

    return ret;
}

static bool opposite_dev_change_native_dev_into_sleep_config(
    board_test_exec_t *exec)
{
    bool ret = false;
    uint16_t Hrh = exec->chn;
    const uint8_t *CanSduPtr = exec->cmd;

    if (Hrh == (CAN_CHN_12 - CAN_SER_OFFSET)) {
        if ((CanSduPtr[1] == 0x5a) && (CanSduPtr[2] == 0xa5)) {
            dprintf(debug_show_null, "opposite_dev_change_native_dev\n");
            thread_sleep(1000);
            can_cmd_t eth_sleep = {.dev_id = 0x3f};
            remote_test_send_req(&eth_sleep);
            remote_test_wait_resp(xTIME_OUT_TICKS, exec);
            ret = true;
        }
    }

    return ret;
}

/*DTU' responses can channel value to DTU*/
static bool slave_resp_can_single_chn_info(uint16_t Hrh, uint8_t id_num,
        const uint8_t *CanSduPtr)
{
    bool ret = false;
    canx_opt_t canx_opt;
    set_para_value(canx_opt.can_send, (can_send_t *)canx_opt.pay_load);

    set_para_value(canx_opt.can_send->chn_num, CanSduPtr[0]);
    set_para_value(canx_opt.can_send->send_data, CanSduPtr[1] + 1);
    set_para_value(canx_opt.resp_chn_id, (uint16_t)Hrh);

    ret = can_channel_to_write(&canx_opt, can_chn_table[id_num].target_id);

    return ret;
}
/*DTU' check DTU the number of can channel*/
static uint8_t slave_resp_can_check_canid(Can_IdType CanId)
{
    uint8_t num;

    for (num = 0; num < ARRAY_SIZE(can_chn_table); num++) {

        if (CanId != can_chn_table[num].Source_id) {

            continue;
        }
        else {
            break;
        }
    }

    dprintf(debug_show_null, "slave num=%x\n", num);

    return num;
}
/*DTU' response DTU can channel value*/
bool slave_dev_resp_can_chn_info(board_test_exec_t *exec)
{
    bool ret = false;
    uint8_t id_num;
    uint16_t Hrh = exec->chn;
    Can_IdType CanId = exec->cmd_canid;
    const uint8_t *CanSduPtr = exec->cmd;

    if ((id_num = slave_resp_can_check_canid(CanId)) < ARRAY_SIZE(can_chn_table)) {
        if ((ret = opposite_dev_change_native_dev_into_sleep_config(exec)) == true)
            goto out;

        ret = slave_resp_can_single_chn_info(Hrh, id_num, CanSduPtr);
    }

out:
    return ret;
}
/*DTU check DTU' can channel return value right or not*/
static bool resp_can_single_chn_info(board_test_exec_t *exec)
{
    bool ret = false;
    uint32_t respCanID;

    if (get_board_test_context_state() == STATE_SINGLE) {
        set_para_value(respCanID, SINGLE_RESP);
    }
    else if (get_board_test_context_state() == STATE_PERIODIC) {
        set_para_value(respCanID, PERIODIC_RESP_CANFD);
    }
    else {
        return ret;
    }

    get_can_info_from_table(exec);

    can_common_response(exec, respCanID) ? (ret = true) : (ret = false);

    return ret;
}
/*DTU check DTU' can channel value*/
void can_check_back_package(board_test_exec_t *exec)
{
    resp_can_single_chn_info(exec);
}
/*cna send data by single */
static bool _can_single_send(board_test_exec_t *exec)
{
    bool ret = false;
    canx_opt_t canx_opt;
    uint8_t cmdStatus = CMD_PARA_ERR;

    can_cmd_t *can_cmd = (can_cmd_t *)exec->cmd;

    if (can_cmd->dev_id == g_step_case_table[CANFD_SERIAL_ID].cmd_id) {

        for (uint8_t num = 0; num < ARRAY_SIZE(can_chn_table); num++) {

            if (can_cmd->route_channel_id != can_chn_table[num].channel) {

                continue;
            }
            else {
                dprintf(debug_show_dg, "can_cmd->route_channel_id is %d\n",
                        can_cmd->route_channel_id);
                set_para_value(canx_opt.can_send, (can_send_t *)canx_opt.pay_load);

                set_para_value(canx_opt.can_send->chn_num, can_cmd->route_channel_id);
                set_para_value(canx_opt.can_send->send_data, can_cmd->recv_data);
                set_para_value(canx_opt.resp_chn_id,
                               can_cmd->route_channel_id - CAN_SER_OFFSET);

                push_can_info_into_table(can_cmd);

                ret = can_channel_to_write(&canx_opt, can_chn_table[num].Source_id);
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

    return ret;
}
/*cna send data by periodic*/
static bool _can_period_send(board_test_exec_t *exec)
{
    bool ret = true;
    canx_opt_t canx_opt;
    can_cmd_t can_cmd;

    set_para_value(canx_opt.can_send, (can_send_t *)canx_opt.pay_load);

    for (uint8_t num = 0; num < ARRAY_SIZE(can_chn_table); num++) {

        set_para_value(canx_opt.can_send->chn_num,
                       can_chn_table[num].channel);//send channel num
        set_para_value(canx_opt.can_send->send_data, CAN_SEND_VAL);//send data
        set_para_value(canx_opt.resp_chn_id,
                       can_chn_table[num].channel - CAN_SER_OFFSET);//send can channel

        set_para_value(can_cmd.route_channel_id, canx_opt.can_send->chn_num);
        set_para_value(can_cmd.recv_data, canx_opt.can_send->send_data);
        push_can_info_into_table(&can_cmd);

        ret = can_channel_to_write(&canx_opt, can_chn_table[num].Source_id);
        thread_sleep(15);
    }

    return ret;
}
/*can process function start*/
bool board_can_reply_deal(board_test_exec_t *exec, board_test_state_e state)
{
    bool ret = false;

    if (state == STATE_SINGLE) {
        if (_can_single_send(exec) == true) {
            set_para_value(ret, true);
            set_para_value(exec->board_response, NULL);
        }
        else {
            set_para_value(exec->board_response,
                           can_common_response);//return immediately, if an error occurs
        }
    }
    else if (state == STATE_PERIODIC) {
        ret = _can_period_send(exec);
        set_para_value(exec->board_response, NULL);
    }

    return ret;
}
