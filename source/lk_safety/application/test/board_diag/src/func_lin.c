/*
 * func_lin.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include "string.h"
#include "board_start.h"
#undef ERPC_TYPE_DEFINITIONS
#include "board_cfg.h"
#include "Lin.h"
#include "func_can.h"

#define get_lin_timers_value(x)   ((x<get_lin_chn_table_len()) ? (x++):(x=0))
#define LIN_SEND_VAL     0x22
#define LIN_RECV_MODE

typedef enum {
    IDLE = 0,
    FULL = 1,
} LIN_STATE_e;

typedef struct {
#define MAX_CAPACITY 10
    uint8_t buf[8];
    uint8_t cnt;
    LIN_STATE_e state;
} lin_rx_t;

lin_rx_t lin_rx[] = {
    [0] = {
        .state = IDLE,
    },
    [1] = {
        .state = IDLE,
    },
    [2] = {
        .state = IDLE,
    },
    [3] = {
        .state = IDLE,
    },
};

#ifdef LIN_RECV_MODE
static Lin_PduType pdu_info[] = {
    [0] = {
        .Pid = 0x30,
        .Cs = LIN_CLASSIC_CS,
        .Dl = 8,
        .Drc = LIN_SLAVE_RESPONSE,
    },
    [1] = {
        .Pid = 0x31,
        .Cs = LIN_CLASSIC_CS,
        .Dl = 8,
        .Drc = LIN_SLAVE_RESPONSE,
    },
    [2] = {
        .Pid = 0x32,
        .Cs = LIN_CLASSIC_CS,
        .Dl = 8,
        .Drc = LIN_SLAVE_RESPONSE,
    },
    [3] = {
        .Pid = 0x33,
        .Cs = LIN_CLASSIC_CS,
        .Dl = 8,
        .Drc = LIN_SLAVE_RESPONSE,
    }
};
#endif

const lin_chn_table_t lin_chn_table[] = {
    {0x00, 0x21, 0x21},
    {0x01, 0x22, 0x22},
    {0x02, 0x23, 0x23},
    {0x03, 0x24, 0x24},
};

static bool lin_channel_to_write(uint8_t lin_send_chn, Lin_PduType *PduInfoPtr)
{
    bool ret = true;

    Lin_SendFrame(lin_send_chn, PduInfoPtr);

    return ret;
}

#ifdef LIN_RECV_MODE
static bool _lin_single_recv(board_test_exec_t *exec)
{
    bool ret = false;
    uint8_t status = CMD_PARA_ERR;
    can_cmd_t *lin_cmd = (can_cmd_t *)exec->cmd;
    uint8_t lin_chn  = lin_cmd->recv_data;

    if (lin_cmd->dev_id == g_step_case_table[LIN_SERIAL_ID].cmd_id) {
        for (uint8_t num = 0; num < ARRAY_SIZE(lin_chn_table); num++) {
            if (lin_chn != lin_chn_table[num].channel) {
                continue;
            }
            else {
                /*lin slave response*/
                if (lin_rx[lin_chn].state == FULL) {
                    uint8_t *lin_rx_buf = lin_rx[lin_chn].buf;
                    set_para_value(exec->resp[0], lin_rx_buf[0]);
                    set_para_value(exec->resp[1], lin_rx_buf[1]);
                    set_para_value(exec->resp[2], lin_rx_buf[2]);
                    set_para_value(exec->resp[3], lin_rx_buf[3]);
                    set_para_value(exec->resp[4], lin_rx_buf[4]);
                    set_para_value(exec->resp[5], lin_rx_buf[5]);
                    set_para_value(exec->resp[6], lin_rx_buf[6]);
                    set_para_value(exec->resp[7], lin_rx_buf[7]);
                    lin_rx[lin_chn].state = IDLE;
                    memset(lin_rx_buf, 0, 8);
                }
                else {

                    set_resp_err_state(exec->resp[0], CMD_PARA_ERR);
                    set_para_value(exec->resp[1], lin_chn);
                }

                ret = true;
            }
        }
    }
    else {
        set_resp_err_state(exec->resp[0], status);
        set_para_value(exec->resp[1], lin_chn);
        return ret;
    }

    if (ret != true) {
        set_resp_err_state(exec->resp[0], status);
        set_para_value(exec->resp[1], lin_chn);
        return ret;
    }

    return ret;
}

static bool _lin_peroid_recv(board_test_exec_t *exec)
{
    bool ret = false;

    for (uint8_t num = 0; num < ARRAY_SIZE(lin_chn_table); num++) {
        set_para_value(exec->cmd[0], SUBCMD_LIN);
        set_para_value(exec->cmd[1], 0);
        set_para_value(exec->cmd[2], num);
        ret = _lin_single_recv(exec);
        can_common_response(exec, PERIODIC_RESP_LIN);
        thread_sleep(300);
    }

    return ret;
}
#else
static bool _lin_single_send(board_test_exec_t *exec)
{
    bool ret = false;
    uint8_t data[8] = {0};
    uint8_t *lin_rx_buf = NULL;
    uint8_t cmdStatus = CMD_PARA_ERR;
    can_cmd_t *lin_cmd = (can_cmd_t *)exec->cmd;

    uint8_t lin_send_data = lin_cmd->standby_data1;
    uint8_t lin_send_chn  = lin_cmd->recv_data;

    Lin_PduType PduInfoPtr = {
        .Pid = lin_chn_table[lin_send_chn].Source_id,
        .Cs = LIN_CLASSIC_CS,
        .Drc = LIN_MASTER_RESPONSE,
        .Dl = 8,
        .SduPtr = data
    };

    if (lin_cmd->dev_id == g_step_case_table[LIN_SERIAL_ID].cmd_id) {

        for (uint8_t num = 0; num < ARRAY_SIZE(lin_chn_table); num++) {

            if (lin_cmd->recv_data != lin_chn_table[num].channel) {
                continue;
            }
            else {
                dprintf(debug_show_null, "lin_send_chn is %d\n", lin_send_chn);
                dprintf(debug_show_null, "lin_send_Source_id is %d\n",
                        lin_chn_table[lin_send_chn].Source_id);
                set_para_value(data[0], lin_send_chn);
                set_para_value(data[1], lin_send_data);
                lin_channel_to_write(lin_send_chn, &PduInfoPtr);
                ret = true;
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

    thread_sleep(10);

    if (Lin_GetStatus(lin_send_chn, &lin_rx_buf) != LIN_TX_OK) {
        ret = false;
        set_resp_err_state(cmdStatus, CMD_PARA_ERR);
        set_resp_err_state(exec->resp[0], cmdStatus);
        dprintf(debug_show_null, "Lin_GetStatus error\n");
        return ret;
    }
    else {
        set_para_value(cmdStatus, NORMAL_DEAL);
        dprintf(debug_show_null, "Lin_GetStatus ok\n");
    }

    set_para_value(exec->resp[0], cmdStatus);
    set_para_value(exec->resp[1], lin_send_chn);
    set_para_value(exec->resp[2], lin_send_data + 1);

    return ret;
}

static bool _lin_peroid_send(board_test_exec_t *exec)
{
    bool ret = false;

    for (uint8_t num = 0; num < ARRAY_SIZE(lin_chn_table); num++) {
        set_para_value(exec->cmd[0], SUBCMD_LIN);
        set_para_value(exec->cmd[1], 0);
        set_para_value(exec->cmd[2], num);
        set_para_value(exec->cmd[3], LIN_SEND_VAL);
        ret = _lin_single_send(exec);
        can_common_response(exec, PERIODIC_RESP_LIN);
    }

    return ret;
}
#endif

int lin_schdule_thread(void *arg)
{
    thread_sleep(1000 * 10);
    static uint8_t bus = 0;

    while (1) {
        uint8_t *lin_rx_buf = NULL;

        Lin_SendFrame(bus, &pdu_info[bus]);
        thread_sleep(50);

        uint8_t status2 = Lin_GetStatus(bus, &lin_rx_buf);

        if (status2 == LIN_RX_OK) {
            memcpy(&lin_rx[bus].buf, lin_rx_buf, 8);
            lin_rx[bus].state = FULL;
        }
        else {

            memset(&lin_rx[bus].buf, 0, 8);
        }

        if (bus == 3)
            thread_sleep(200);

        bus = (bus + 1) % 4;
    }

    return 0;
}

bool board_lin_reply_deal(board_test_exec_t *exec, board_test_state_e state)
{
    uint8_t ret = true;

    if (state == STATE_SINGLE) {
#ifdef LIN_RECV_MODE
        ret = _lin_single_recv(exec);
#else
        ret = _lin_single_send(exec);
#endif
        set_para_value(exec->board_response, can_common_response);
    }
    else if (state == STATE_PERIODIC) {
#ifdef LIN_RECV_MODE
        ret = _lin_peroid_recv(exec);
#else
        ret = _lin_peroid_send(exec);
#endif
        set_para_value(exec->board_response, NULL);
    }

    return ret;
}
