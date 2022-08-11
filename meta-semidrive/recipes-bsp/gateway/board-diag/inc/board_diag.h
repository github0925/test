/*
 * board_diag.h
 *
 * Copyright (c) 2020 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef _BOARD_DIAG_H
#define _BOARD_DIAG_H

#include "sdrv_types.h"
/*
 * Single step commands share the same cmd & resp CAN ID. They're
 * differentiated by data payload byte 0.
 */
#define SINGLE_CMD                0x326
#define SINGLE_RESP               0x327

/* Single execution sub-commands. */
#define SUBCMD_DIAG               0X0
#define SUBCMD_ADC                0x2
#define SUBCMD_GPIO_R             0x4
#define SUBCMD_GPIO_W             0x6
#define SUBCMD_CAPT               0x7
#define SUBCMD_LIN                0x35
#define SUBCMD_100BASE_T1         0x36
#define SUBCMD_1000BASE_T1        0x37
#define SUBCMD_100BASE_TX         0x38
#define SUBCMD_CANFD              0x39
#define SUBCMD_EMMC               0x3a
#define SUBCMD_DDR                0x3b
#define SUBCMD_FLASH              0x3c
#define SUBCMD_UART               0x3d
#define SUBCMD_I2C                0x3e
#define SUBCMD_SLEEP              0x3f
#define SUBCMD_POWER_OFF          0X40
#define SUBCMD_ETH_INT            0X41
#define SUBCMD_USB1               0X42
#define SUBCMD_RELE_VERSION       0x50
#define SUBCMD_STORE_STRESS       0x60

/* Handshake and mode setting sub-commands. */
#define SUBCMD_HANDSHAKE          0x1
#define SUBCMD_MODE               0xc

/* CAN ID of periodic responses. */
#define PERIODIC_RESP_ADC         0x65
#define PERIODIC_RESP_GPIO        0x66
#define PERIODIC_RESP_CAPT        0x67
#define PERIODIC_RESP_DIAG        0x68
#define PERIODIC_RESP_RESERVED    0x69
#define PERIODIC_RESP_CANFD       0x6a
#define PERIODIC_RESP_LIN         0x6b
#define PERIODIC_RESP_DDR         0x6d
#define PERIODIC_RESP_EMMC        0x6e
#define PERIODIC_RESP_FLASH       0x6f

#define TEST_CMD_LEN        (8)
#define TEST_RESP_LEN       (8)

#define set_para_value(x, y)     (x=y)
#define get_gpio_legal_value_(x) (x)?(x=1):(x=0)
#define ARRAY_SIZE(x)  ((sizeof(x) / sizeof(x[0])))

typedef enum {
    ADC_SERIAL_ID = 0,
    GPIO_SERIAL_R_ID,
    GPIO_SERIAL_W_ID,
    CAPT_SERIAL_ID,
    LIN_SERIAL_ID,
    CANFD_SERIAL_ID,
    EMMC_SERIAL_ID,
    DDR_SERIAL_ID,
    FLASH_SERIAL_ID,
    UART_SERIAL_ID,
    DIAG_SERIAL_ID,
    I2C_SERIAL_ID,
    SLEEP_SERIAL_ID,
    POWER_OFF_SERIAL_ID,
    ETH_100BASE_T1_ID,
    ETH_1000BASE_T1_ID,
    ETH_100BASE_TX_ID,
    ETH_INT_SERIAL_ID,
    USB1_SERIAL_ID,
    HANDSHAKE_SERIAL_ID,
    HANDMODE_SERIAL_ID,
    VERSION_SERIAL_ID,
    STORE_SERIAL_ID
} CASE_SERIAL_ID;

typedef enum {
    ONLY_SINGLE,
    ONLY_PERIOD,
    SINGLE_AND_PERIOD,
} PERIOD_TYPE;

/**
 * @brief Test application state machine.
 */
typedef enum test_state {
    /* Handling single step test commands. */
    STATE_SINGLE,

    /* Handshake received, but test mode is not configured. */
    STATE_HANDSHAKE,

    /* Test mode is configured. Upload data periodically. */
    STATE_PERIODIC,
} test_state_e;

/**
 * @brief Test execution data.
 */
typedef struct {
    uint32_t            cmd_canid;
    uint8_t             cmd[TEST_CMD_LEN];

    uint32_t            resp_canid;
    uint8_t             resp[TEST_RESP_LEN];

} test_exec_t;

/**
 * @brief Test case handler.
 */
typedef bool (* test_handler)(test_exec_t *exec, test_state_e state);

/**
 * @brief cmdStatus
 */

typedef enum {
    NORMAL_DEAL = 0XA0,
    CMD_PARA_ERR = 0XB0,
    CMD_PARA_NUM_ERR = 0XC0
} CMD_STATUS;

/**
 * @brief Test case.
 */
typedef struct test_case {
    uint32_t     cmd_id;
    test_handler handler;
    int32_t      serial_id;
    uint8_t      period_type;
} test_case_t;

typedef struct {
    uint8_t dev_id;
    uint8_t route_channel_id;
    uint8_t recv_data;
    uint8_t standby_data1;
    uint8_t standby_data2;
    uint8_t standby_data3;
    uint8_t standby_data4;
    uint8_t standby_data5;
} test_cmd_t;

typedef struct {
    uint8_t cmd_status;
    uint8_t route_channel_id;
    uint8_t send_data;
    uint8_t standby_data1;
    uint8_t standby_data2;
    uint8_t standby_data3;
    uint8_t standby_data4;
    uint8_t response_channel_id;
} test_resp_t;

typedef struct {
    uint32_t canid;
    uint32_t data_len;
    test_resp_t resp;
} test_resp_packet_t;

extern const char *ping_ip_addr;
extern uint8_t common_response(test_exec_t *exec, uint32_t respCanID);
extern uint16_t anti_inter_filter_algo(uint16_t smootnum, uint16_t *arraydata);
extern void filter_move_step(uint16_t sum, uint16_t len, uint16_t val,
                      uint16_t *arraydata);
#endif /* _TEST_H */
