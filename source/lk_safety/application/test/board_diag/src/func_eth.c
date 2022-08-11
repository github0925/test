#include "string.h"
#include "func_eth.h"
#include "board_start.h"
#include "remote_test.h"
#include "func_can.h"

/*
*100base_t1 paraments configure
*/
const static eth_chn_table_t eth_100base_t1_table[] = {
    {0x01, 0x11},//5072_port_1
    {0x02, 0x12},//5072_port_2
    {0x03, 0x13},//5072_port_3
    {0x04, 0x14},//5072_port_4
    {0x05, 0x15},//5072_port_5
    {0x06, 0x16},//5072_port_6
    {0x07, 0x01},//5050_port_1
    {0x08, 0x02},//5050_port_2
    {0x09, 0x03},//5050_port_3
    {0x0a, 0x04},//5050_port_4
    {0x0b, 0x05},//5050_port_5
};
/*
*1000base_t1 paraments configure
*/
const static eth_chn_table_t eth_1000base_t1_table[] = {
    {0x01, 0x06},//88Q2122_88Q5050_P567_RGMII
    {0x02, 0x1a},//88Q2122_88Q5072_P10_SGMII
    {0x03, 0x19},//88Q2122_88Q5072_P9_SGMII
    {0x04, 0x07},//88Q2122_88Q5050_P7_SGMII
};
/*
*100base_tx paraments configure
*/
const static eth_chn_table_t eth_100base_tx_table[] = {
    {0x01, 0x17},//5072_Port7差分对信号P_0,5072_Port7差分对信号N_0
};

switch_error_arg_t switch_error_arg[ETH_MAX_CHN];

static int eth_filtering_error_successful(board_test_exec_t *exec, bool retval)
{
    int ret = -1;
    uint8_t state = exec->resp[0];
    uint8_t nr = exec->resp[1] -
                 1; //Channel alignment, because eth channel start  from 1.

    if (ETH_MAX_CHN <= nr)
        goto out;

    if ((retval == true) && (state == NORMAL_DEAL)) {
        switch_error_arg[nr].time_val = ETH_MAX_ERROR_TIME;
        ret = 0;
    }
    else {
        if (switch_error_arg[nr].time_val > 0) {
            ret = 1;
        }
    }

out:
    return ret;
}

void _eth_xbase_t1_channel_sequence_for_periodic_test(board_test_exec_t *exec,
        uint8_t chn, uint8_t dev_id)
{
    if (dev_id == SUBCMD_100BASE_T1) {
        set_para_value(exec->resp[1], chn);
    }
    else if (dev_id == SUBCMD_1000BASE_T1) {
        set_para_value(exec->resp[1], ARRAY_SIZE(eth_100base_t1_table) + chn);
    }
    else if (dev_id == SUBCMD_100BASE_Tx) {
        set_para_value(exec->resp[1],
                       ARRAY_SIZE(eth_100base_t1_table) + ARRAY_SIZE(eth_1000base_t1_table) + chn);
    }
}
/*ethernet read users remote call, which function is in ap core*/
static bool _eth_xbase_t1_single_send(board_test_exec_t *exec,
                                      eth_pdev_t *eth_dev)
{
    bool ret = false;
    uint16_t result = 0;
    uint8_t cmdStatus = CMD_PARA_ERR;
    can_cmd_t *eth_cmd = (can_cmd_t *)exec->cmd;

    if (eth_cmd->dev_id == g_step_case_table[eth_dev->rate_type_id].cmd_id) {

        for (uint8_t num = 0; num < eth_dev->eth_chn_table_len; num++) {

            if (eth_cmd->route_channel_id != eth_dev->eth_chn_table[num].chn_num) {
                continue;
            }
            else {
                remote_test_send_req(eth_cmd);

                if (remote_test_wait_resp(xTIME_OUT_TICKS, exec) == true) {
                    set_para_value(cmdStatus, NORMAL_DEAL);
                    ret = true;
                    return ret;
                }
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

    /*clean eth receive buf*/
    set_para_value(exec->resp[0], cmdStatus);
    set_para_value(exec->resp[1], eth_cmd->route_channel_id);
    set_para_value(exec->resp[2], (uint8_t)((result >> 8) & 0xff));
    set_para_value(exec->resp[3], (uint8_t)(result & 0xff));

    return ret;
}
/*ethernet send by periodic*/
static bool _eth_xbase_t1_peridic_send(board_test_exec_t *exec,
                                       CASE_SERIAL_ID case_id, uint8_t dev_id, uint8_t table_len,
                                       const eth_chn_table_t *eth_table)
{
    bool ret = false;
    eth_pdev_t eth_pdev;

    set_para_value(eth_pdev.rate_type_id, case_id);
    set_para_value(eth_pdev.eth_chn_table, eth_table);
    set_para_value(eth_pdev.eth_chn_table_len, table_len);

    for (uint8_t num = 0; num < table_len; num++) {
        set_para_value(exec->cmd[0], dev_id);//set dev_id
        set_para_value(exec->cmd[1], eth_table[num].chn_num);//set route_channel_id
        set_para_value(exec->cmd[2], 0);//set mormal test mode
        set_para_value(exec->cmd[3], 0x22);//valid data in mormal mode
        set_para_value(exec->cmd[4],
                       XMASTER_MODE);//set mormal test in default master mode
        ret = _eth_xbase_t1_single_send(exec, &eth_pdev);
        _eth_xbase_t1_channel_sequence_for_periodic_test(exec, eth_table[num].chn_num,
                dev_id);//Channel arrangement for peridic

        if (eth_filtering_error_successful(exec, ret) > 0) {
            set_resp_err_state(exec->resp[0], NORMAL_DEAL);
            set_para_value(exec->resp[2], 0x23);
        }

        can_common_response(exec, PERIODIC_RESP_ETH);
    }

    return ret;
}
/*ethernet 100 base T1 function*/
bool board_eth_100base_t1_deal(board_test_exec_t *exec,
                               board_test_state_e state)
{
    uint8_t ret = false;

    if (state == STATE_SINGLE) {/*ethernet 100 base T1 signal test*/
        eth_pdev_t eth_pdev;
        set_para_value(eth_pdev.rate_type_id, ETH_100BASE_T1_ID);
        set_para_value(eth_pdev.eth_chn_table, eth_100base_t1_table);
        set_para_value(eth_pdev.eth_chn_table_len, ARRAY_SIZE(eth_100base_t1_table));

        ret = _eth_xbase_t1_single_send(exec, &eth_pdev);
        set_para_value(exec->board_response, can_common_response);
    }
    else if (state == STATE_PERIODIC) { /*ethernet 100 base T1 peridic test*/
        ret = _eth_xbase_t1_peridic_send(exec, ETH_100BASE_T1_ID, SUBCMD_100BASE_T1,
                                         ARRAY_SIZE(eth_100base_t1_table), eth_100base_t1_table);

        set_para_value(exec->board_response, NULL);
    }

    return ret;
}
/*ethernet 1000 base T1 function*/
bool board_eth_1000base_t1_deal(board_test_exec_t *exec,
                                board_test_state_e state)
{
    uint8_t ret = false;

    if (state == STATE_SINGLE) {/*ethernet 1000 base T1 signal test*/
        eth_pdev_t eth_pdev;

        set_para_value(eth_pdev.rate_type_id, ETH_1000BASE_T1_ID);
        set_para_value(eth_pdev.eth_chn_table, eth_1000base_t1_table);
        set_para_value(eth_pdev.eth_chn_table_len, ARRAY_SIZE(eth_1000base_t1_table));

        ret = _eth_xbase_t1_single_send(exec, &eth_pdev);
        set_para_value(exec->board_response, can_common_response);
    }
    else if (state == STATE_PERIODIC) { /*ethernet 1000 base T1 peridic test*/
        ret = _eth_xbase_t1_peridic_send(exec, ETH_1000BASE_T1_ID, SUBCMD_1000BASE_T1,
                                         ARRAY_SIZE(eth_1000base_t1_table), eth_1000base_t1_table);
        set_para_value(exec->board_response, NULL);
    }

    return ret;
}
/*ethernet TX function*/
bool board_eth_100base_tx_deal(board_test_exec_t *exec,
                               board_test_state_e state)
{
    uint8_t ret = false;

    if (state == STATE_SINGLE) {/*ethernet 1000 base Tx signal test*/
        eth_pdev_t eth_pdev;

        set_para_value(eth_pdev.rate_type_id, ETH_100BASE_TX_ID);
        set_para_value(eth_pdev.eth_chn_table, eth_100base_tx_table);
        set_para_value(eth_pdev.eth_chn_table_len, ARRAY_SIZE(eth_100base_tx_table));

        ret = _eth_xbase_t1_single_send(exec, &eth_pdev);
        set_para_value(exec->board_response, can_common_response);
    }
    else if (state == STATE_PERIODIC) { /*ethernet 100 base Tx peridic test*/
        ret = _eth_xbase_t1_peridic_send(exec, ETH_100BASE_TX_ID, SUBCMD_100BASE_Tx,
                                         ARRAY_SIZE(eth_100base_tx_table), eth_100base_tx_table);
        set_para_value(exec->board_response, NULL);
    }

    return ret;
}
