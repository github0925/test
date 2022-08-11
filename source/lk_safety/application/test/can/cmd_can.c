/*
*  cmd_can.c
*
* Copyright (c) 2020 Semidrive Semiconductor.
* All rights reserved.
*
* Description: can samplecode.
*
* Revision History:
* -----------------
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <kernel/thread.h>

#undef ERPC_TYPE_DEFINITIONS
#include "Can.h"
#include "can_cfg.h"

#include "hal_port.h"
#include "hal_dio.h"
#include "res.h"
#include "chip_res.h"

struct can_status_info {
    uint64_t write_cnt;
    uint64_t rx_ind_cnt;
};

extern const Can_ConfigType gCan_Config;

extern struct Can_ControllerConfig gCan_CtrllerConfig[];
extern struct Can_MBConfig gCan_RxMBCfg[];
extern struct Can_MBConfig gCan_TxMBCfg[];

static bool can_test_ind_flag = 0;

static struct can_status_info g_can_status[NUM_OF_HTHS];

#define CAN_ADD_WRITE_INFO(hth)    g_can_status[hth].write_cnt++
#define CAN_ADD_RX_IND_INFO(hrh)   g_can_status[hrh].rx_ind_cnt++

#ifdef V9F_A02_REF
extern const domain_res_t g_gpio_res;
static const Port_PinType TJA1043_EN_PIN = PortConf_PIN_RGMII1_RXD1;
static const Port_PinModeType TJA1043_EN_PIN_MODE = {
    ((uint32_t)PORT_PAD_POE__ENABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_UP ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};

void can_switch_port_setup(void) {
    void *port_handle = NULL;
    void *dio_handle = NULL;

    hal_port_creat_handle(&port_handle, RES_PAD_CONTROL_SAF_JTAG_TMS);
    if (port_handle) {
        hal_port_set_pin_mode(port_handle, TJA1043_EN_PIN, TJA1043_EN_PIN_MODE);
    } else {
        printf("port get handle failed!\n");
    }

    hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);
    if (dio_handle) {
        hal_dio_write_channel(dio_handle, TJA1043_EN_PIN, 1);
    } else {
        printf("dio get handle failed!\n");
    }

    hal_dio_release_handle(&dio_handle);
    hal_port_release_handle(port_handle);
}
#endif

int can_init_test(int argc, const cmd_args *argv)
{
#ifdef V9F_A02_REF
    can_switch_port_setup();
#endif

#ifdef SDPE
    for (size_t i = 0; i < gCan_Config.controllerCount; i++) {
        Can_ConfigType Can_Config = {
            .controllerCount = 1,
            .ctrllerCfg = &gCan_CtrllerConfig[i],
        };
        for (size_t j = 0; j < gCan_Config.rxCount; j++) {
            if (gCan_RxMBCfg[j].controllerId ==
                gCan_CtrllerConfig[i].controllerId) {
                Can_Config.rxCount++;
                if (!Can_Config.rxMBCfg) {
                    Can_Config.rxMBCfg = &gCan_RxMBCfg[j];
                }
            }
        }
        for (size_t j = 0; j < gCan_Config.txCount; j++) {
            if (gCan_TxMBCfg[j].controllerId ==
                gCan_CtrllerConfig[i].controllerId) {
                Can_Config.txCount++;
                if (!Can_Config.txMBCfg) {
                    Can_Config.txMBCfg = &gCan_TxMBCfg[j];
                }
            }
        }
        printf("Can_Init %d\n", i);
        Can_Init(&Can_Config);
    }

    /* Send configuration paramenter ending message. */
    Can_ConfigType ending = {
        .controllerCount = 0
    };
    Can_Init(&ending);
#else
    Can_Init(&gCan_Config);
#endif

    for (size_t i = 0; i < CAN_CTRL_CONFIG_CNT; i++) {
        printf("Can_SetControllerMode %d\n", i);
        Can_SetControllerMode(i, CAN_CS_STARTED);
    }

    printf("%s() Result: %d\n", __func__, 0);
    return 0;
}

int can_deinit_test(int argc, const cmd_args *argv)
{
    Can_DeInit();
    printf("%s() Result: %d\n", __func__, 0);
    return 0;
}

int can_set_baudrate_test(int argc, const cmd_args *argv)
{
    Std_ReturnType ret;

    if (argc != 3) {
        printf("argc must be 3\n");
        return -1;
    }

    ret = Can_SetBaudrate(argv[1].u, argv[2].u);
    printf("%s() Result: %d\n", __func__, ret);
    return 0;
}

int can_set_mode_test(int argc, const cmd_args *argv)
{
    Std_ReturnType ret;

    if (argc != 3) {
        printf("argc must be 3\n");
        return -1;
    }

    ret = Can_SetControllerMode(argv[1].u, argv[2].u);
    printf("%s() Result: %d\n", __func__, ret);
    return 0;
}

int can_dis_int_test(int argc, const cmd_args *argv)
{
    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    Can_DisableControllerInterrupts(argv[1].u);
    printf("%s() Result: %d\n", __func__, 0);
    return 0;
}

int can_en_int_test(int argc, const cmd_args *argv)
{
    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    Can_EnableControllerInterrupts(argv[1].u);
    printf("%s() Result: %d\n", __func__, 0);
    return 0;
}

int can_check_wakeup_test(int argc, const cmd_args *argv)
{
    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    Can_CheckWakeup(argv[1].u);
    printf("%s() Result: %d\n", __func__, 0);
    return 0;
}

int can_get_error_state_test(int argc, const cmd_args *argv)
{
    static Can_ErrorStateType ErrorState;
    Std_ReturnType ret;

    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    ret = Can_GetControllerErrorState(argv[1].u, &ErrorState);
    printf("%s() Result: %d, ErrorState: %d\n", __func__,
           ret, ErrorState);
    return 0;
}

int can_get_mode_test(int argc, const cmd_args *argv)
{
    static Can_ControllerStateType ControllerMode;
    Std_ReturnType ret;

    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    ret = Can_GetControllerMode(argv[1].u, &ControllerMode);
    printf("%s() Result: %d, ControllerMode: %d\n", __func__,
           ret, ControllerMode);
    return 0;
}

int can_write_test(int argc, const cmd_args *argv)
{
    static uint8_t Data[64] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                                10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                                20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                                30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                                40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
                                50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
                                60, 61, 62, 63
                              };
    uint16_t hth;
    static Can_PduType PduInfo;
    static uint16_t swPduHandle = 0;
    Std_ReturnType ret;
    uint32_t id;
    uint8_t length;

    if (argc < 2 || argc > 4) {
        printf("argc must between 2~4\n");
        return -1;
    }

    hth = argv[1].u;

    if (argc == 2) {
        id = MAKE_STANDARD_CAN_ID(0x327);
        length = 8U;
    }
    else if (argc == 3){
        id = MAKE_STANDARD_CAN_ID(argv[2].u);
        length = 8U;
    }
    else {
        id = MAKE_STANDARD_CAN_ID(argv[2].u);
        length = argv[3].u;
    }

    PduInfo.swPduHandle = swPduHandle;
    PduInfo.length = length;
    PduInfo.id = id;
    PduInfo.sdu = Data;

    lk_bigtime_t timer_stamp_start = current_time_hires();
    ret = Can_Write(hth, &PduInfo);
    lk_bigtime_t timer_stamp_end = current_time_hires();
    printf("%s() Result: %d\n", __func__, ret);
    printf("%s() timer stamp start: %lld\n", __func__, timer_stamp_start);
    printf("%s() timer stamp start: %lld\n", __func__, timer_stamp_end);

    swPduHandle++;
    for(uint16_t i = 0; i < 64; i++) {
        Data[i] += 1;
    }

    CAN_ADD_WRITE_INFO(hth);

    return 0;
}

static int can_perf_test_entry(void *arg)
{
    cmd_args *argv = (cmd_args*)arg;
    uint16_t hth;
    uint8_t DataLen;
    int32_t test_count, count;
    uint16_t swPduHandle = 0;
    uint32_t send_period;
    bool send_flag;
    lk_bigtime_t timer_stamp_start, timer_stamp_end, timer_cost;
    static Can_PduType CanPdu;
    static uint8_t Data[64] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                                10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                                20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                                30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
                                40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
                                50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
                                60, 61, 62, 63
                              };

    hth = argv[1].u;
    DataLen = argv[2].u;
    send_period = argv[3].u;
    test_count = argv[4].i;
    count = test_count;

    CanPdu.length = DataLen;
    CanPdu.id = MAKE_STANDARD_CAN_ID(0x327);
    CanPdu.sdu = Data;

    timer_stamp_start = current_time_hires();

    do {
        CanPdu.swPduHandle = swPduHandle++;
        Can_Write(hth, &CanPdu);
        CAN_ADD_WRITE_INFO(hth);
        thread_sleep(send_period);
        if (count > 0)
            count--;
        if (test_count == -1 || count > 0)
            send_flag = true;
        else
            send_flag = false;
    } while (send_flag);

    timer_stamp_end = current_time_hires();

    timer_cost = timer_stamp_end - timer_stamp_start;

    printf("avg time:%lld us\n", timer_cost / test_count);
    printf("throughput = %.2f MB/s\n", (float)(test_count * DataLen) / timer_cost);

    free(argv);

    return 0;
}

static int can_perf_test(int argc, const cmd_args *argv)
{
    struct thread *perf_test_thread;
    cmd_args *param;

    if (argc != 5) {
        printf("argc must 5\n");
        return -1;
    }

    param = malloc(sizeof(cmd_args) * argc);
    if (!param) {
        printf("perf test malloc fail\n");
        return -1;
    }

    memcpy(param, argv, sizeof(cmd_args) * argc);

    perf_test_thread = thread_create("can_perf_test", can_perf_test_entry,
                                     (void*)param, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(perf_test_thread);

    return 0;
}

static int can_indication_ctrl(int argc, const cmd_args *argv)
{
    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    can_test_ind_flag = argv[1].b;
    printf("%s() Result: %d, ind_flag: %d\n",
           __func__, 0, can_test_ind_flag);

    return 0;
}

static int can_status_report_print(void)
{
    struct can_status_info *info = g_can_status;
    uint32_t cnt;
    bool flag = false;

    printf("\n===can status report===\n");
    for (cnt = 0; cnt < NUM_OF_HTHS; cnt++, info++) {
        if (info->write_cnt != 0 || info->rx_ind_cnt != 0) {
            printf("can id %d:\n", cnt);
            printf("\twrite_cnt:%lld\n", info->write_cnt);
            printf("\trx_ind_cnt:%lld\n", info->rx_ind_cnt);
            flag = true;
        }
    }

    if (!flag)
        printf("no status info\n");

    return 0;
}

static int can_status_report_handle(void *arg)
{
    uint32_t report_period = (uint32)arg;

    while (true) {
        can_status_report_print();
        thread_sleep(report_period);
    }

    return 0;
}

static int can_status_report(int argc, const cmd_args *argv)
{
    uint32_t report_period;
    struct thread *time_thread;

    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    report_period = argv[1].u;

    time_thread = thread_create("status_report", can_status_report_handle,
                                (void*)report_period, LOW_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(time_thread);

    return 0;
}

static int can_polling_rx(int argc, const cmd_args *argv)
{
    Can_MainFunction_Read();

    return 0;
}

static int can_polling_tx(int argc, const cmd_args *argv)
{
    Can_MainFunction_Write();

    return 0;
}

static void can_usage(void)
{
    printf("can init");
    printf("\t\t:can init test\n");

    printf("can deinit");
    printf("\t\t:can deinit test\n");

    printf("can set_baudrate");
    printf("\t:can set baudrate test [controller] [baudrate]\n");

    printf("can set_mode");
    printf("\t\t:can set mode test [controller] [mode]\n");

    printf("can get_mode");
    printf("\t\t:can get mode test [controller]\n");

    printf("can dis_int");
    printf("\t\t:can disalbe int test [controller]\n");

    printf("can en_int");
    printf("\t\t:can enable int test [controller]\n");

    printf("can check_wakeup");
    printf("\t:can check wakeup test [controller]\n");

    printf("can get_state");
    printf("\t\t:can get error state test [controller]\n");

    printf("can write");
    printf("\t\t:can write test [hth] [prot_id] [data_len]\n");

    printf("can perf");
    printf("\t\t:can perf test [hth] [data_len] [send_period] [test_count]\n");

    printf("can ind_ctrl");
    printf("\t\t:enable or disable indication [enalbe - 1/disable - 0]\n");

    printf("can status_report");
    printf("\t:enable or disable status report [report_period]\n");

    printf("can polling_rx");
    printf("\t\t:rx polling\n");

    printf("can polling_tx");
    printf("\t\t:tx polling\n");
}

int do_can_cmd(int argc, const cmd_args *argv)
{
    if (argc < 2) {
        can_usage();
        return 0;
    }

    if (!strcmp(argv[1].str, "init")) {
        can_init_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "deinit")) {
        can_deinit_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "set_baudrate")) {
        can_set_baudrate_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "set_mode")) {
        can_set_mode_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "get_mode")) {
        can_get_mode_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "dis_int")) {
        can_dis_int_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "en_int")) {
        can_en_int_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "check_wakeup")) {
        can_check_wakeup_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "get_state")) {
        can_get_error_state_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "write")) {
        can_write_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "perf")) {
        can_perf_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "ind_ctrl")) {
        can_indication_ctrl(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "status_report")) {
        can_status_report(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "polling_rx")) {
        can_polling_rx(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "polling_tx")) {
        can_polling_tx(argc-1, &argv[1]);
    }
    else
    {
        printf("error cmd\n");
    }

    return 0;
}

void can_rx_indication_test(uint16_t hrh, uint32_t can_id,
                            uint8_t dlc, uint8_t *sdu)
{
    if (can_test_ind_flag)
        printf("\n%s() Hrh: %d, CanId: 0x%X, CanDlc: %d\n", \
               __func__, hrh, can_id, dlc);

    CAN_ADD_RX_IND_INFO(hrh);
}

void can_tx_confirmation_test(uint16_t pdu_id)
{
    if (can_test_ind_flag)
        printf("\n%s() canTxPduId: %d\n", __func__, pdu_id);
}

#include <lib/console.h>

STATIC_COMMAND_START
STATIC_COMMAND("can", "can test", (console_cmd)&do_can_cmd)
STATIC_COMMAND_END(cmd_can);

APP_START(cmd_can)
.flags = 0,
APP_END
