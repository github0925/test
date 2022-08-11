/*
*  cmd_lin.c
*
* Copyright (c) 2020 Semidrive Semiconductor.
* All rights reserved.
*
* Description: cmd_lin samplecode.
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
#include "Lin.h"

#include "hal_port.h"
#include "hal_dio.h"
#include "res.h"
#include "chip_res.h"

#ifdef SCI4_USED
#define LIN_MAX_CHANNEL_ID      LIN_IFC_SCI4
#else
#define LIN_MAX_CHANNEL_ID      LIN_IFC_SCI3
#endif

extern const Lin_ConfigType lin_config;

#ifdef V9F_A02_REF
static const Port_PinType PIN_GPIO_A6_UART2_TX = PortConf_PIN_GPIO_A6;
static const Port_PinModeType MODE_GPIO_A6_UART2_TX = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_ALT1),
};

static const Port_PinType PIN_GPIO_A7_UART2_RX = PortConf_PIN_GPIO_A7;
static const Port_PinModeType MODE_GPIO_A7_UART2_RX = {
    ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__IN | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_OPENDRAIN | PORT_PIN_MODE_ALT1),
};

static const Port_PinType TJA1020_EN_PIN = PortConf_PIN_RGMII1_TXC;
static const Port_PinModeType TJA1020_EN_PIN_MODE = {
    ((uint32_t)PORT_PAD_POE__ENABLE | PORT_PAD_IS__OUT | PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_PULL_UP ),
    ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN  | PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
};

extern const domain_res_t g_gpio_res;

void lin_switch_port_setup(void)
{
    void *port_handle = NULL;
    void *dio_handle = NULL;
    hal_port_creat_handle(&port_handle, RES_PAD_CONTROL_SAF_JTAG_TMS);
    if (port_handle) {
        hal_port_set_pin_mode(port_handle, TJA1020_EN_PIN, TJA1020_EN_PIN_MODE);
        hal_port_set_pin_mode(port_handle, PIN_GPIO_A6_UART2_TX, MODE_GPIO_A6_UART2_TX);
        hal_port_set_pin_mode(port_handle, PIN_GPIO_A7_UART2_RX, MODE_GPIO_A7_UART2_RX);
    } else {
        printf("port get handle failed!\n");
    }

    hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);
    if (dio_handle) {
        hal_dio_write_channel(dio_handle, TJA1020_EN_PIN, 1);
    } else {
        printf("dio get handle failed!\n");
    }
    hal_dio_release_handle(&dio_handle);
    hal_port_release_handle(port_handle);
}
#endif

int lin_init_test(int argc, const cmd_args *argv)
{
#ifdef V9F_A02_REF
    lin_switch_port_setup();
#endif
    Lin_Init(&lin_config);
    printf("%s() Result: %d\n", __func__, 0);
    return 0;
}

static int lin_send_frame_test(int argc, const cmd_args *argv)
{
    static uint8_t Data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    static Lin_PduType PduInfo;
    Std_ReturnType ret;
    uint8_t Pid;
    uint8_t channel;

    if (argc < 2 || argc > 3) {
        printf("argc must between 2~3\n");
        return -1;
    }

    channel = argv[1].u;
    if (channel > LIN_MAX_CHANNEL_ID) {
        printf("channel id error:%d\n", channel);
        return -1;
    }

    if (argc == 2) {
        Pid = 0x21;
    }
    else {
        Pid = argv[2].u;
    }

    PduInfo.Pid = Pid;
    PduInfo.Cs = LIN_CLASSIC_CS;
    PduInfo.Drc = LIN_MASTER_RESPONSE;
    PduInfo.Dl = 8;
    PduInfo.SduPtr = Data;

    ret = Lin_SendFrame(channel, &PduInfo);
    printf("%s() Result: %d\n", __func__, ret);

    for (uint8_t i = 0; i < 8; i++) {
        Data[i] += 1;
    }

    return 0;
}

int lin_check_wakeup_test(int argc, const cmd_args *argv)
{
    Std_ReturnType ret;
    uint8_t channel;

    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    channel = argv[1].u;
    if (channel > LIN_MAX_CHANNEL_ID) {
        printf("channel id error:%d\n", channel);
        return -1;
    }

    ret = Lin_CheckWakeup(channel);
    printf("%s() Result: %d\n", __func__, ret);

    return 0;
}

int lin_goto_sleep_test(int argc, const cmd_args *argv)
{
    Std_ReturnType ret;
    uint8_t channel;

    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    channel = argv[1].u;
    if (channel > LIN_MAX_CHANNEL_ID) {
        printf("channel id error:%d\n", channel);
        return -1;
    }

    ret = Lin_GoToSleep(channel);
    printf("%s() Result: %d\n", __func__, ret);

    return 0;
}

int lin_goto_sleep_internel_test(int argc, const cmd_args *argv)
{
    Std_ReturnType ret;
    uint8_t channel;

    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    channel = argv[1].u;
    if (channel > LIN_MAX_CHANNEL_ID) {
        printf("channel id error:%d\n", channel);
        return -1;
    }

    ret = Lin_GoToSleepInternal(channel);
    printf("%s() Result: %d\n", __func__, ret);

    return 0;
}

int lin_wakeup_test(int argc, const cmd_args *argv)
{
    Std_ReturnType ret;
    uint8_t channel;

    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    channel = argv[1].u;
    if (channel > LIN_MAX_CHANNEL_ID) {
        printf("channel id error:%d\n", channel);
        return -1;
    }

    ret = Lin_Wakeup(channel);
    printf("%s() Result: %d\n", __func__, ret);

    return 0;
}

int lin_wakeup_internel_test(int argc, const cmd_args *argv)
{
    Std_ReturnType ret;
    uint8_t channel;

    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    channel = argv[1].u;
    if (channel > LIN_MAX_CHANNEL_ID) {
        printf("channel id error:%d\n", channel);
        return -1;
    }

    ret = Lin_WakeupInternal(channel);
    printf("%s() Result: %d\n", __func__, ret);

    return 0;
}

int lin_get_status_test(int argc, const cmd_args *argv)
{
    uint8_t *SduPtr = NULL;
    Lin_StatusType ret;
    uint8_t channel;

    if (argc != 2) {
        printf("argc must be 2\n");
        return -1;
    }

    channel = argv[1].u;
    if (channel > LIN_MAX_CHANNEL_ID) {
        printf("channel id error:%d\n", channel);
        return -1;
    }

    ret = Lin_GetStatus(channel, &SduPtr);
    printf("%s() Result: %d, sdu: ", __func__, ret);

    for (uint8_t i = 0; i < 8; i++) {
        printf("0x%x ", SduPtr[i]);
    }
    printf("\n");

    return 0;
}

static void lin_usage(void)
{
    printf("lin init");
    printf("\t\t:lin init test\n");

    printf("lin send_frame");
    printf("\t\t:lin send frame test [channel] [prot_id]\n");

    printf("lin check_wakeup");
    printf("\t:lin check_wakeup test [channel]\n");

    printf("lin goto_sleep");
    printf("\t\t:lin goto sleep test [channel]\n");

    printf("lin goto_sleep_internel");
    printf("\t:lin goto sleep internel test [channel]\n");

    printf("lin wakeup");
    printf("\t\t:lin wakeup test [channel]\n");

    printf("lin wakeup_internel");
    printf("\t:lin wakeup internel test [channel]\n");

    printf("lin get_status");
    printf("\t\t:lin get status test [channel]\n");
}

int do_lin_cmd(int argc, const cmd_args *argv)
{
    if (argc < 2) {
        lin_usage();
        return 0;
    }

    if (!strcmp(argv[1].str, "init")) {
        lin_init_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "send_frame")) {
        lin_send_frame_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "check_wakeup")) {
        lin_check_wakeup_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "goto_sleep")) {
        lin_goto_sleep_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "goto_sleep_internel")) {
        lin_goto_sleep_internel_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "wakeup")) {
        lin_wakeup_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "wakeup_internel")) {
        lin_wakeup_internel_test(argc-1, &argv[1]);
    }
    else if (!strcmp(argv[1].str, "get_status")) {
        lin_get_status_test(argc-1, &argv[1]);
    }
    else
    {
        printf("error cmd\n");
    }

    return 0;
}

#include <lib/console.h>

STATIC_COMMAND_START
STATIC_COMMAND("lin", "lin test", (console_cmd)&do_lin_cmd)
STATIC_COMMAND_END(cmd_lin);

APP_START(cmd_lin)
.flags = 0,
APP_END
