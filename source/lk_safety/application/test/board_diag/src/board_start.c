/*
 * board_start.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include <app.h>
#include <string.h>
#include "Can.h"
#include "board_start.h"
#include "board_init.h"
#include "board_cfg.h"
#include "func_can.h"
#include "remote_test.h"
/**
 * @brief Test execution context.
 */
typedef struct board_test_context {
    /*master_slave_mode*/
    volatile uint8_t master_slave_mode;
    /*device master or slave mode is confirmed*/
    bool confirm_flag;
    /*event init state flag*/
    bool event_init_falg;
    /* State lock */
    spin_lock_t         lock;
    /* Current test state. */
    board_test_state_e  state;
    /* Execution data. */
    board_test_exec_t   exec;
    /*copy exeution data*/
    /* Interval of periodic tests, in ms. */
    volatile uint32_t   period;
    /* Interval of periodic time reduce, in ms. */
    volatile uint32_t   time_reduce;
    /* Event to notify state change. */
    event_t             event;
} board_test_context_t;

static bool handle_handshake(board_test_exec_t *exec, board_test_state_e state);
static bool handle_mode(board_test_exec_t *exec, board_test_state_e state);
static int periodic_test_thread(void *arg);

static board_test_context_t g_board_test_context;
/*bellow are all the test cases */
const board_test_case_t g_step_case_table[] = {

    { SUBCMD_ADC,            board_adc_reply_deal,            ADC_SERIAL_ID,          SINGLE_AND_PERIOD},//adc_reply_deal
    { SUBCMD_ETH_SELF_CHECK, board_eth_self_check_deal,       ETH_SELF_CHECK_ID,      ONLY_SINGLE},//eth_self_check_deal
    { SUBCMD_GPIO_R,         board_gpio_read_reply_deal,      GPIO_SERIAL_R_ID,       SINGLE_AND_PERIOD},//gpio_reply_read_deal
    { SUBCMD_GPIO_W,         board_gpio_write_reply_deal,     GPIO_SERIAL_W_ID,       ONLY_SINGLE},//gpio_reply_write_deal
    { SUBCMD_CAPT,           board_capt_reply_deal,           CAPT_SERIAL_ID,         SINGLE_AND_PERIOD},//capt_reply_deal
    { SUBCMD_100BASE_T1,     board_eth_100base_t1_deal,       ETH_100BASE_T1_ID,      SINGLE_AND_PERIOD},//board_eth_100base_t1_deal
    { SUBCMD_1000BASE_T1,    board_eth_1000base_t1_deal,      ETH_1000BASE_T1_ID,     SINGLE_AND_PERIOD},//board_eth_1000base_t1_deal
    { SUBCMD_100BASE_Tx,     board_eth_100base_tx_deal,       ETH_100BASE_TX_ID,      ONLY_SINGLE},//board_eth_100base_tx_deal
    { SUBCMD_LIN,            board_lin_reply_deal,            LIN_SERIAL_ID,          SINGLE_AND_PERIOD},//lin_reply_deal
    { SUBCMD_CANFD,          board_can_reply_deal,            CANFD_SERIAL_ID,        SINGLE_AND_PERIOD},//can_reply_deal
    { SUBCMD_EMMC,           board_emmc_reply_deal,           EMMC_SERIAL_ID,         SINGLE_AND_PERIOD},//emmc_reply_deal
    { SUBCMD_DDR,            board_ddr_reply_deal,            DDR_SERIAL_ID,          SINGLE_AND_PERIOD},//ddr_reply_deal
    { SUBCMD_FLASH,          board_flash_reply_deal,          FLASH_SERIAL_ID,        SINGLE_AND_PERIOD},//flash_reply_deal
    { SUBCMD_UART,           board_uart_reply_deal,           UART_SERIAL_ID,         ONLY_SINGLE},//uart_reply_deal
    { SUBCMD_DIAG,           board_diag_reply_deal,           DIAG_SERIAL_ID,         ONLY_SINGLE},//diag_reply_deal
    { SUBCMD_I2C,            board_i2c_reply_deal,            I2C_SERIAL_ID,          ONLY_SINGLE},//i2c_reply_deal
    { SUBCMD_SLEEP,          board_sleep_reply_deal,          SLEEP_SERIAL_ID,        ONLY_SINGLE},//sleep_reply_deal
    { SUBCMD_POWER_OFF,      board_power_off_reply_deal,      POWER_OFF_SERIAL_ID,    ONLY_SINGLE},//power_off_reply_deal
    { SUBCMD_HANDSHAKE,      handle_handshake,                HANDSHAKE_SERIAL_ID,    ONLY_SINGLE},
    { SUBCMD_MODE,           handle_mode,                     HANDMODE_SERIAL_ID,     ONLY_SINGLE},
    { SUBCMD_PHY_INT,        write_eth_phy_gpio_deal,         ETH_PHY_INT_ID,         ONLY_SINGLE},//eth_gpio_reply_deal
    { SUBCMD_USB1,           board_usb1_reply_deal,           USB1_SERIAL_ID,         SINGLE_AND_PERIOD},//usb_gpio_reply_deal
    { SUBCMD_RELE_VERSION,   board_release_vresion_deal,      VERSION_SERIAL_ID,      ONLY_SINGLE},
    { SUBCMD_STORE_STRESS,   board_store_stress_deal,         STORE_SERIAL_ID,        ONLY_SINGLE}

};
/*set DTU or DTU' mode*/
inline static void set_master_slave_mode(MASTER_SLAVER_MODE mode)
{
    set_para_value(g_board_test_context.master_slave_mode, mode);
}
/*get DTU or DTU' mode*/
inline static uint8_t get_master_slave_mode(void)
{
    return  g_board_test_context.master_slave_mode;
}

/**
 * @brief board_diag_can_rxind callback.
 *
 * This function is called in interrupt context.
 */
void board_diag_can_rxind(uint16_t Hrh, Can_IdType CanId,
                          uint8_t CanDlc, const uint8_t *CanSduPtr)
{
    if (g_board_test_context.event_init_falg != true)
        return;

    board_test_exec_t *exec = &g_board_test_context.exec;
    BaseType_t xHigherPriorityTaskWoken = pdTRUE;

    set_para_value(exec->cmd_canid, CanId);
    memcpy(exec->cmd, CanSduPtr, CanDlc);

    if (CAN2 == Hrh) {//only for can 2 channel, we use it sending can command

        if (CanId == AP_ACK) {
            if (remote_test_resp_cb(CanSduPtr) != true)
                return;
        }
        else if (CanId == SINGLE_CMD) {
            event_signal(&exec->rx_event, true);
        }
    }
    else { //usually used in peridic test especially for multi_can test

        if (get_master_slave_mode() == XSLAVE_MODE) {
            set_para_value(exec->chn, Hrh);
            xEventGroupSetBitsFromISR(canx_app.dtu.xEventGroupHandle, CAN_SLAVE_RESP_MODE,
                                      &xHigherPriorityTaskWoken);
            return;
        }

        xEventGroupSetBitsFromISR(canx_app.dtu.xEventGroupHandle, CAN_MASTER_RESP_MODE,
                                  &xHigherPriorityTaskWoken);

    }
}
/*only for handshake*/
static bool _handshake_cmd_response(board_test_exec_t *exec)
{
    bool ret = false;
    uint8_t cmdStatus = CMD_PARA_ERR;
    can_cmd_t *handshark_cmd = (can_cmd_t *)exec->cmd;

    if (handshark_cmd->dev_id == g_step_case_table[HANDSHAKE_SERIAL_ID].cmd_id) {

        set_para_value(cmdStatus, NORMAL_DEAL);
        set_para_value(exec->resp[0], cmdStatus);
        set_para_value(exec->resp[1], 0xa5);
        set_para_value(exec->resp[2], 0x5a);
        set_para_value(ret, true);
    }
    else {
        set_para_value(exec->resp[0], cmdStatus);
        set_para_value(ret, false);
    }

    set_para_value(exec->board_response, can_common_response);

    return ret;
}

/**
 * @brief Periodic test case.
 *
 * Test all components and upload test result.
 */
static void handle_periodic_test(void)
{
    dprintf(INFO, "handle_periodic_test\n");

    board_test_exec_t *exec = &g_board_test_context.exec;

    for (uint8_t i = 0; i < ARRAY_SIZE(g_step_case_table); i++) {

        const board_test_case_t *test_case = &g_step_case_table[i];

        if ((test_case->period_type != ONLY_SINGLE) && (test_case->handler != NULL)) {

            dprintf(debug_show_null, "handle_periodic_test i=%d\n", i);
            test_case->handler(exec, g_board_test_context.state);

            if ((exec->board_response) && (g_board_test_context.state == STATE_PERIODIC)) {
                exec->board_response(exec, exec->peridic_resp_id);
            }
            else {
                memset(exec->resp, 0, sizeof(exec->resp));
            }

            thread_sleep(1);
        }
    }
}
/*only two states, the one is sigle test state, the other one is periodic test state*/
board_test_state_e get_board_test_context_state(void)
{
    return g_board_test_context.state;
}

static bool handle_handshake(board_test_exec_t *exec, board_test_state_e state)
{
    _handshake_cmd_response(exec);

    return true;
}
/*delay ms*/
void set_peridic_test_interval_time(board_test_exec_t *exec)
{
    g_board_test_context.period = (exec->cmd[4] << 24) | (exec->cmd[5] << 16) |
                                  (exec->cmd[6] << 8) | exec->cmd[7];
}
/*Periodic test interval*/
inline static uint32_t get_peridic_test_interval_time(void)
{
    return g_board_test_context.period;
}
/*Enter the periodic test interval to wait*/
static void enter_into_peridic_interval_time(void)
{
    if (g_board_test_context.time_reduce == 0)
        return;

    for (uint32_t i = 0; i < g_board_test_context.time_reduce; i++) {
        thread_sleep(1);
    }
}

void eth_self_check_for_startingup(void)
{
    static int32_t cntx = 0;
    static int32_t check_num = 0;
    static bool ethCheckRet = false;

    if (get_master_slave_mode() == XMASTER_MODE) {
        if (cntx < 300) { //ethernet init about 50S
            cntx++;
        }
        else if ((ethCheckRet != true) && (check_num < 3)) {
            board_test_exec_t cmd = {.cmd = {SUBCMD_100BASE_T1, 0x2, 0x0, 0x22, 00, 00, 00, 00}};
            ethCheckRet = board_eth_self_check_deal(&cmd, STATE_SINGLE);
            check_num++;
        }
    }
}

/*Select the single step or periodic test mode*/
static bool handle_mode(board_test_exec_t *exec, board_test_state_e state)
{
    uint8_t mode = exec->cmd[1];
    bool signal = false;

    CONTEXT_LOCK(g_board_test_context);

    switch (mode) {
        case 0x10://single test
            g_board_test_context.state = STATE_SINGLE;
            g_board_test_context.period = 0;
            break;

        case 0x11://periodic test
            g_board_test_context.state = STATE_PERIODIC;    /* ms */
            set_peridic_test_interval_time(exec);
            g_board_test_context.time_reduce = get_peridic_test_interval_time();
            signal = true;
            break;

        default:
            break;
    }

    CONTEXT_UNLOCK(g_board_test_context);

    if (signal) {
        /* Notify periodic test thread about state change. */
        event_signal(&g_board_test_context.event, false);
    }

    set_para_value(exec->resp[0], NORMAL_DEAL);
    set_para_value(exec->board_response, NULL);

    return true;
}

static bool exec_single_cmd(board_test_exec_t *exec)
{
    can_cmd_t *sub_cmd = (can_cmd_t *)exec->cmd;

    for (uint8_t i = 0; i < ARRAY_SIZE(g_step_case_table); i++) {

        const board_test_case_t *test_case = &g_step_case_table[i];

        if ((test_case->cmd_id == sub_cmd->dev_id) && (test_case->handler != NULL)) {

            test_case->handler(exec, g_board_test_context.state);

            if (exec->board_response) {
                exec->board_response(exec, SINGLE_RESP);
            }

            return true;
        }
    }

    set_para_value(exec->resp[0], CMD_PARA_ERR);
    exec->board_response(exec, SINGLE_RESP);

    return false;
}

/**
 * @brief Thread to handle periodic tests.
 */
static int periodic_test_thread(void *arg)
{
    printf("Periodic test thread start...\n\n");

    while (1) {
        board_test_state_e state;

        CONTEXT_LOCK(g_board_test_context);
        state = g_board_test_context.state;
        CONTEXT_UNLOCK(g_board_test_context);

        switch (state) {
            case STATE_SINGLE:
            case STATE_HANDSHAKE:
                /* In single step or handshake state, do nothing but wait
                 * for the state change notification to enter periodic
                 * test state.
                 */
                event_wait(&g_board_test_context.event);
                break;

            case STATE_PERIODIC:
                if (get_master_slave_mode() != XMASTER_MODE)
                    break;

                /*peridic test delay for interval*/
                g_board_test_context.time_reduce = get_peridic_test_interval_time();
                handle_periodic_test();
                enter_into_peridic_interval_time();
                break;

            default:
                break;
        }
    }

    return 0;
}
/*this function only for Multiple can roads*/
static int can_resp_test_thread(void *arg)
{
    EventBits_t uxBits;

    while (1) {
        uxBits = xEventGroupWaitBits(canx_app.dtu.xEventGroupHandle, CAN_EVENT, pdTRUE,
                                     pdFALSE, portMAX_DELAY);
        board_test_exec_t *exec = &g_board_test_context.exec;

        if (uxBits &
                MONITOR_DUT_MODE) { //this event is only for changing DTU or DTU' due to con1_19 pin state
            board_test_exec_t cmd = {.cmd = {SUBCMD_GPIO_R, 0x1A}};

            if (remote_require_for_gpio_value(&cmd) == true) {//remote call for ap core
                uint8_t chn = cmd.resp[1];
                uint8_t val = cmd.resp[2];

                if (chn == cmd.cmd[1]) {
                    if (val == HIGH) {
                        printf("MASTER_MODE\n");
                        set_master_slave_mode(XMASTER_MODE);
                    }
                    else {
                        printf("SLAVE_MODE\n");
                        set_master_slave_mode(XSLAVE_MODE);
                    }

                    g_board_test_context.confirm_flag = true;
                }
            }
        }

        if (uxBits &
                CAN_SLAVE_RESP_MODE) { // this event is only for responsing mutliple can datas, if this board is DTU'
            slave_dev_resp_can_chn_info(exec);
        }

        if (uxBits &
                CAN_MASTER_RESP_MODE) {//this event is only for checking DTU' value returned, if this board is DTU
            can_check_back_package(exec);
        }
    }

    return 0;
}

/*test case start, usually for board paraments init */
void _board_diag_start(void)
{
    printf("test thread start...\n");

    g_board_test_context.state = STATE_SINGLE;
    g_board_test_context.lock = SPIN_LOCK_INITIAL_VALUE;

    canx_app.lock = SPIN_LOCK_INITIAL_VALUE;
    canx_app.dtu.lock = SPIN_LOCK_INITIAL_VALUE;

    event_init(&g_board_test_context.event, false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&g_board_test_context.exec.rx_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&canx_app.dtu.monitor_master_slave_event, false,
               EVENT_FLAG_AUTOUNSIGNAL);

    if ((canx_app.dtu.xEventGroupHandle = xEventGroupCreate()) == NULL)
        return;

    if (remote_test_queue_creat() ==
            false) //remote road creat, usually for communication between safety core and AP core
        return;

    board_diag_module_init(&g_board_test_context.event_init_falg);
    /* Create periodic test thread. */

    thread_resume(thread_create("test-periodic", periodic_test_thread,
                                NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE));
    /* can resp procss */
    thread_resume(thread_create("can-response", can_resp_test_thread,
                                NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE));
    /* lin shchdule procss */
    thread_resume(thread_create("lin-schdule", lin_schdule_thread,
                                NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE));

    while (1) {
        board_test_exec_t *exec = &g_board_test_context.exec;

        event_wait(&exec->rx_event);

        switch (exec->cmd_canid) {
            case SINGLE_CMD:
            case SINGLE_CMD_PRIORITY:
                exec_single_cmd(exec);
                dprintf(debug_show_null, "CAN ID 0x%x\n", exec->cmd_canid);
                break;

            default:
                dprintf(debug_show_null, "Unknown CAN ID 0x%x\n", exec->cmd_canid);
                exec->resp[0] = CMD_PARA_ERR;
                break;
        }
    }
}
/*Determine if the device has entered master-slave mode*/
bool get_dev_master_slave_work_role(void)
{
    return g_board_test_context.confirm_flag;
}

/*ms*/
void monitor_periodic_interval(void)
{
    if (g_board_test_context.time_reduce > 0) {
        g_board_test_context.time_reduce--;
    }
}
/*
*case response interface
*/
bool can_common_response(board_test_exec_t *exec, uint32_t respCanID)
{
    bool ret = false;
    canx_opt_t canx_opt;

    set_para_value(canx_opt.can_resp, (can_resp_t *)canx_opt.pay_load);
    set_para_value(canx_opt.can_resp->cmd_status, exec->resp[0]);
    set_para_value(canx_opt.can_resp->route_channel_id, exec->resp[1]);
    set_para_value(canx_opt.can_resp->send_data, exec->resp[2]);
    set_para_value(canx_opt.can_resp->standby_data1, exec->resp[3]);
    set_para_value(canx_opt.can_resp->standby_data2, exec->resp[4]);
    set_para_value(canx_opt.can_resp->standby_data3, exec->resp[5]);
    set_para_value(canx_opt.can_resp->standby_data4, exec->resp[6]);
    set_para_value(canx_opt.can_resp->standby_data5, exec->resp[7]);
    set_para_value(canx_opt.resp_chn_id, CAN2);
    ret = can_channel_to_write(&canx_opt, respCanID);

    memset(canx_opt.pay_load, 0, sizeof(canx_opt.pay_load));
    memset(exec->resp, 0, sizeof(exec->resp));

    return ret;
}

int board_diag(int argc, const cmd_args *argv)
{
    _board_diag_start();
    return 0;
}

static void board_diag_entry(const struct app_descriptor *app, void *args)
{
    _board_diag_start();
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("board_diag", "Board Diag", (console_cmd)&board_diag)
STATIC_COMMAND_END(board_diag);
#endif


APP_START(board_diag)
.flags = 0,
.entry = board_diag_entry,
APP_END
