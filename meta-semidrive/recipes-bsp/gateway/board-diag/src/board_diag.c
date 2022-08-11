/*
 * board_diag.c
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <linux/rpmsg.h>
#include <sys/ioctl.h>

#include "board_diag.h"
#include "debug.h"
#include "func_gpio.h"
#include "func_emmc.h"
#include "func_eth.h"
#include "gpio-ops.h"
#include "rpmsg_ops.h"
#include "sw_timer.h"
#include "func_usb.h"
#include "func_capt.h"
#include "socket.h"

#define PACKET_CANID_OFFSET             0
#define PACKET_FRL_OFFSET               4
#define PACKET_FRD_OFFSET               8
#define PACKET_HEAD_LEN                 12

struct eth_msg_head {
    uint16_t msg_id;
    uint16_t msg_type;
    uint32_t cookie;
} __attribute__((packed));

#define SDPE_MSG_HEAD                   sizeof(struct eth_msg_head)

#define TEST_PACKET_MAX_SIZE            (512 + SDPE_MSG_HEAD)

#define RPMSG_DEV_NAME                  "/dev/sdpe-eth"

#define SDPE_RPC_ETH1_SERVICE           3
#define SDPE_RPC_ETH2_SERVICE           4
#define ETH_SEND_FRAME_MSG_ID           1

#define SDPE_RPC_REQ                    0
#define SDPE_RPC_RSP                    1
#define SDPE_RPC_IND                    2

#define CONTEXT_LOCK(context) \
    pthread_mutex_lock(&(context.mutex)); \

#define CONTEXT_UNLOCK(context) \
    pthread_mutex_unlock(&(context.mutex)); \

#define CONTEXT_SIGNAL(context) \
    pthread_cond_signal(&(context.cond)); \

#define CONTEXT_WAIT(context) \
    pthread_cond_wait(&(context.cond), &(context.mutex)); \

/**
 * @brief Test execution context.
 */
typedef struct test_context {

    /* State lock */
    pthread_mutex_t mutex;

    /* Event to notify state change. */
    pthread_mutex_t cond_mutex;
    pthread_cond_t cond;

    /* Current test state. */
    test_state_e state;

    /* Execution data. */
    test_exec_t exec;

    /* Interval of periodic tests, in ms. */
    int period;

} test_context_t;

const char *ping_ip_addr = NULL;
static void init_device(void);
static void deinit_device(void);
static bool response_to_rpmsg(int ep_fd, uint8_t *buf, size_t len);
static bool handle_handshake(test_exec_t *exec, test_state_e state);
static bool handle_mode(test_exec_t *exec, test_state_e state);
static void handle_periodic_test(void);

static int rpmsg_fd = -1;
static int service_id = SDPE_RPC_ETH1_SERVICE;

static test_context_t g_test_context;
const test_case_t g_step_case_table[] = {

    { SUBCMD_ADC,            NULL,                  ADC_SERIAL_ID,          ONLY_SINGLE},//adc_reply_deal
    { SUBCMD_GPIO_R,         gpio_read_reply_deal,  GPIO_SERIAL_R_ID,       ONLY_SINGLE},//gpio_reply_deal
    { SUBCMD_GPIO_W,         gpio_write_reply_deal, GPIO_SERIAL_W_ID,       ONLY_SINGLE},//gpio_reply_deal
    { SUBCMD_CAPT,           capt_reply_deal,       CAPT_SERIAL_ID,         ONLY_SINGLE},//capt_reply_deal
    { SUBCMD_LIN,            NULL,                  LIN_SERIAL_ID,          ONLY_SINGLE}, //lin_reply_deal
    { SUBCMD_CANFD,          NULL,                  CANFD_SERIAL_ID,        ONLY_SINGLE}, //can_reply_deal
    { SUBCMD_EMMC,           emmc_reply_deal,       EMMC_SERIAL_ID,         ONLY_SINGLE}, //emmc_reply_deal
    { SUBCMD_DDR,            NULL,                  DDR_SERIAL_ID,          ONLY_SINGLE},//ddr_reply_deal
    { SUBCMD_FLASH,          NULL,                  FLASH_SERIAL_ID,        ONLY_SINGLE},//flash_reply_deal
    { SUBCMD_UART,           NULL,                  UART_SERIAL_ID,         ONLY_SINGLE},//uart_reply_deal
    { SUBCMD_DIAG,           NULL,                  DIAG_SERIAL_ID,         ONLY_SINGLE},//diag_reply_deal
    { SUBCMD_I2C,            NULL,                  I2C_SERIAL_ID,          ONLY_SINGLE},//i2c_reply_deal
    { SUBCMD_SLEEP,          eth_enable_wakesrc,    SLEEP_SERIAL_ID,        ONLY_SINGLE},//sleep_reply_deal
    { SUBCMD_POWER_OFF,      eth_enable_wakesrc,    POWER_OFF_SERIAL_ID,    ONLY_SINGLE},//power_off_reply_deal
    { SUBCMD_100BASE_T1,     eth_100base_t1,        ETH_100BASE_T1_ID,      ONLY_SINGLE},//power_off_reply_deal
    { SUBCMD_1000BASE_T1,    eth_1000base_t1,       ETH_1000BASE_T1_ID,     ONLY_SINGLE},//power_off_reply_deal
    { SUBCMD_100BASE_TX,     eth_100base_tx,        ETH_100BASE_TX_ID,      ONLY_SINGLE},//power_off_reply_deal
    { SUBCMD_ETH_INT,        eth_int_ctrl,          ETH_INT_SERIAL_ID,      ONLY_SINGLE},//power_off_reply_deal
    { SUBCMD_USB1,           usb1_reply_deal,       USB1_SERIAL_ID,         ONLY_SINGLE},//usb1_reply_deal
    { SUBCMD_HANDSHAKE,      handle_handshake,      HANDSHAKE_SERIAL_ID,    ONLY_SINGLE},
    { SUBCMD_MODE,           handle_mode,           HANDMODE_SERIAL_ID,     ONLY_SINGLE},
    { SUBCMD_RELE_VERSION,   NULL,                  VERSION_SERIAL_ID,      ONLY_SINGLE},
    { SUBCMD_STORE_STRESS,   emmc_reply_deal,       STORE_SERIAL_ID,        ONLY_SINGLE}
};

static bool handle_mode(test_exec_t *exec, test_state_e state)
{
    uint8_t mode;
    bool signal = false;

    mode = exec->cmd[1];

    switch (mode) {
        case 0:
            g_test_context.state = STATE_SINGLE;
            break;

        case 1:
            g_test_context.state = STATE_PERIODIC;
            g_test_context.period = 1000;    /* ms */
            signal = true;
            break;

        case 2:
            g_test_context.state = STATE_PERIODIC;
            g_test_context.period = 10;    /* ms */
            signal = true;
            break;

        default:
            break;
    }

    exec->resp[0] = NORMAL_DEAL;

    //clean_can_multi_chn_info();

    if (signal) {
        /* Notify periodic test thread about state change. */
        CONTEXT_SIGNAL(g_test_context);
    }

    return true;
}

uint8_t common_response(test_exec_t *exec, uint32_t respCanID)
{
    uint8_t ret = false;
    test_resp_packet_t packet;
    test_resp_t *can_resp = (test_resp_t *)exec->resp;

    packet.canid = htonl(respCanID);
    packet.data_len = htonl(sizeof(test_resp_t));
    memcpy(&packet.resp, can_resp, sizeof(test_resp_t));

    response_to_rpmsg(rpmsg_fd, (uint8_t *)&packet,
                      sizeof(test_resp_packet_t)) ? (ret = false) : (ret = true);

    return ret;
}

/**
 * @brief Periodic test case.
 *
 * Test all components and upload test result.
 */
static void handle_periodic_test(void)
{
    test_exec_t *exec = &g_test_context.exec;
    const test_case_t *test_case = &g_step_case_table[0];

    for (uint8_t i = 0; i < ARRAY_SIZE(g_step_case_table); i++, test_case++) {

        if ((test_case->period_type != ONLY_SINGLE)
                && (test_case->handler != NULL)) {
            test_case->handler(exec, g_test_context.state);
            usleep(50000);
        }
    }
}

static bool handshake_cmd_response(test_exec_t *exec)
{
    bool ret = false;
    test_resp_t *handshake_resp = (test_resp_t *)exec->resp;

    if (exec->cmd[0] == g_step_case_table[HANDSHAKE_SERIAL_ID].cmd_id) {
        handshake_resp->response_channel_id = 1;
        ret = true;
    }
    else {
        ret = false;
    }

    common_response(exec, SINGLE_RESP);
    return ret;
}

static bool handle_handshake(test_exec_t *exec, test_state_e state)
{
    g_test_context.state = STATE_HANDSHAKE;

    exec->resp[0] = NORMAL_DEAL;
    exec->resp[1] = 0xa5;
    exec->resp[2] = 0x5a;

    handshake_cmd_response(exec);

    return true;
}

static bool exec_single_cmd(test_exec_t *exec)
{
    bool ret = false;
    bool found = false;
    uint8_t sub_cmd = 0;
    const test_case_t *test_case  = g_step_case_table;

    CONTEXT_LOCK(g_test_context);
    memcpy(&g_test_context.exec, exec, sizeof(test_exec_t));
    sub_cmd = exec->cmd[0];

    for (uint8_t i = 0; i < ARRAY_SIZE(g_step_case_table); i++, test_case++) {

        if ((test_case->cmd_id == sub_cmd) && (test_case->handler != NULL)) {
            ret = test_case->handler(&g_test_context.exec, g_test_context.state);
            found = true;
            break;
        }
    }

    if (!found) {
        ERROR("cann't handle test case, sub_cmd:%u\n", sub_cmd);
    }

    CONTEXT_UNLOCK(g_test_context);
    return ret;
}

static void process_test_packet(test_exec_t *exec)
{
    if (!exec)
        return;

    switch (exec->cmd_canid) {
        case SINGLE_CMD:
            exec_single_cmd(exec);
            DBG("CAN ID 0x%x\n", exec->cmd_canid);
            break;

        default:
            ERROR("Unknown CAN ID 0x%x\n", exec->cmd_canid);
            exec->resp[0] = CMD_PARA_ERR;
            common_response(exec, SINGLE_RESP);
            break;
    }
}

static void eth_fill_msg_head(uint8_t *data, int service_id)
{
    struct eth_msg_head *head = (struct eth_msg_head *)data;

    head->msg_id = (service_id << 8) + ETH_SEND_FRAME_MSG_ID;
    head->msg_type = SDPE_RPC_IND;
}

static bool response_to_rpmsg(int ep_fd, uint8_t *buf, size_t len)
{
    int bytes_sent;
    bool ret = false;
    uint8_t *s_payload;

    if (ep_fd < 0) {
        ERROR("ERROR: rpmsg endpoint fd:%d\n", ep_fd);
        return false;
    }

    if (len > (TEST_PACKET_MAX_SIZE - SDPE_MSG_HEAD)) {
        ERROR("ERROR: response data len error:%ld\n", len);
        return false;
    }

    s_payload = (uint8_t *)malloc(TEST_PACKET_MAX_SIZE);

    if (!s_payload) {
        ERROR("ERROR: Failed to allocate memory for s_payload.\n");
        goto out;
    }

    eth_fill_msg_head(s_payload, service_id);
    memcpy(s_payload + SDPE_MSG_HEAD, buf, len);
    bytes_sent = write(rpmsg_fd, s_payload, len + SDPE_MSG_HEAD);

    if (bytes_sent < 0) {
        ERROR("\nERROR: Send RPMsg failed\n");
        goto out;
    }

    ret = true;
out:

    if (s_payload)
        free(s_payload);

    return ret;
}

/**
 * @brief Thread to handle periodic tests.
 */
static void *periodic_test_thread(void *arg)
{
    int period;
    test_state_e state;
    DBG("Periodic test thread start...\n\n");

    while (1) {

        CONTEXT_LOCK(g_test_context);
        state = g_test_context.state;

        while (state != STATE_PERIODIC) {
            /* In single step or handshake state, do nothing but wait
             * for the state change notification to enter periodic
             * test state.
             */
            DBG("wait for periodic test cmd\n\n");
            CONTEXT_WAIT(g_test_context);
            state = g_test_context.state;
        }

        period = g_test_context.period;

        DBG("start periodic test\n\n");
        /* here, enter period test mode */
        handle_periodic_test();

        CONTEXT_UNLOCK(g_test_context);

        usleep(period * 1000);
    }

    return 0;
}

static void main_loop(int ep_fd)
{
    int bytes_rcvd;
    int frame_len = 0;
    test_exec_t test_exec;
    uint8_t *test_packet = NULL;

    test_packet = (uint8_t *)malloc(TEST_PACKET_MAX_SIZE);

    if (!test_packet) {
        ERROR("ERROR: Failed to allocate memory for test_packet.\n");
        goto exit;
    }

    while (1) {
        bytes_rcvd = read(ep_fd, test_packet, TEST_PACKET_MAX_SIZE);

        if (bytes_rcvd <= 0) {
            continue;
        }

        //hexdump8(test_packet, bytes_rcvd);

        uint8_t *payload = test_packet + SDPE_MSG_HEAD;
        payload += PACKET_HEAD_LEN;

        test_exec.cmd_canid = ntohl(*(uint32_t *)(payload + PACKET_CANID_OFFSET));
        frame_len = ntohl(*(uint32_t *)(payload + PACKET_FRL_OFFSET));
        memcpy(&test_exec.cmd, payload + PACKET_FRD_OFFSET, frame_len);
        process_test_packet(&test_exec);
    }

exit:

    if (test_packet)
        free(test_packet);
}

static void context_init(test_context_t *context)
{
    if (!context)
        return;

    pthread_mutex_init(&context->mutex, NULL);
    pthread_cond_init(&context->cond, NULL);

    context->state = STATE_SINGLE;
    memset(&context->exec, 0x0, sizeof(test_exec_t));
    context->period = 0;
}

static void init_device(void)
{
    gpio_init();
    timer_init();
}

static void deinit_device(void)
{
    gpio_deinit();
}

int main(int argc, char *argv[])
{
    int opt;
    int rpmsg_ept = 1024;
    pthread_t period_thread_id;
    char rpmsg_dev_name[32];
    int eth_id = 0;

    while ((opt = getopt(argc, argv, "e:p:")) != -1) {
        switch (opt) {
            case 'e':
                rpmsg_ept = atoi(optarg);
                DBG("getopt e for rpmsg endpoint %d\n", rpmsg_ept);
                break;

            case 'p':
                ping_ip_addr = optarg;
                break;

            default:
                DBG("getopt return unsupported option: -%c\n", opt);
                break;
        }
    }

    context_init(&g_test_context);

    if (rpmsg_ept == 1024) {
        eth_id = 0;
        service_id = SDPE_RPC_ETH1_SERVICE;
    }
    else {
        eth_id = 1;
        service_id = SDPE_RPC_ETH2_SERVICE;
    }

    sprintf(rpmsg_dev_name, "%s%d", RPMSG_DEV_NAME, eth_id);

    rpmsg_fd = open(rpmsg_dev_name, O_RDWR);

    if (rpmsg_fd < 0) {
        perror("Failed to open rpmsg device.");
        return -1;
    }

    init_device();
    pthread_create(&period_thread_id, NULL, periodic_test_thread, NULL);
    create_socket_ops();
    main_loop(rpmsg_fd);

    pthread_join(period_thread_id, NULL);

    deinit_device();

    close(rpmsg_fd);
    return 0;
}
