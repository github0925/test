/*
 * board_start.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#ifndef _BOARD_DIAG_H
#define _BOARD_DIAG_H

#include <lib/console.h>
#include <kernel/event.h>
#include <lk_wrapper.h>
#include "Can.h"
#include "event_groups.h"

struct board_test_exec;
/*
 * Single step commands share the same cmd & resp CAN ID. They're
 * differentiated by data payload byte 0.
 */
#define SINGLE_CMD_PRIORITY       0x3
#define SINGLE_CMD                0x326
#define SINGLE_RESP               0x327
#define AP_ACK                    0x327

typedef enum {
    ADC_SERIAL_ID = 0,
    ETH_SELF_CHECK_ID,
    GPIO_SERIAL_R_ID,
    GPIO_SERIAL_W_ID,
    CAPT_SERIAL_ID,
    ETH_100BASE_T1_ID,
    ETH_1000BASE_T1_ID,
    ETH_100BASE_TX_ID,
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
    HANDSHAKE_SERIAL_ID,
    HANDMODE_SERIAL_ID,
    ETH_PHY_INT_ID,
    USB1_SERIAL_ID,
    VERSION_SERIAL_ID,
    STORE_SERIAL_ID
} CASE_SERIAL_ID;

typedef enum {
    ONLY_SINGLE,
    ONLY_PERIOD,
    SINGLE_AND_PERIOD,
    NONE
} PERIOD_TYPE;
/* Single execution sub-commands. */
#define SUBCMD_DIAG               0X0
#define SUBCMD_ETH_SELF_CHECK     0x1
#define SUBCMD_ADC                0x2
#define SUBCMD_GPIO_R             0x4
#define SUBCMD_GPIO_W             0x6
#define SUBCMD_CAPT               0x7
#define SUBCMD_LIN                0x35
#define SUBCMD_100BASE_T1         0x36
#define SUBCMD_1000BASE_T1        0x37
#define SUBCMD_100BASE_Tx         0x38
#define SUBCMD_CANFD              0x39
#define SUBCMD_EMMC               0x3a
#define SUBCMD_DDR                0x3b
#define SUBCMD_FLASH              0x3c
#define SUBCMD_UART               0x3d
#define SUBCMD_I2C                0x3e
#define SUBCMD_SLEEP              0x3f
#define SUBCMD_POWER_OFF          0X40
#define SUBCMD_PHY_INT            0x41
#define SUBCMD_USB1               0x42
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
#define PERIODIC_RESP_ETH         0x6c
#define PERIODIC_RESP_DDR         0x6d
#define PERIODIC_RESP_EMMC        0x6e
#define PERIODIC_RESP_FLASH       0x6f
#define PERIODIC_RESP_USB1        0x70

#define BOARD_TEST_CMD_LEN          (8)
#define BOARD_TEST_RESP_LEN         (8)

typedef enum board_test_state {
    /* Handling single step test commands. */
    STATE_SINGLE,

    /* Handshake received, but test mode is not configured. */
    STATE_HANDSHAKE,

    /* Test mode is configured. Upload data periodically. */
    STATE_PERIODIC,
} board_test_state_e;

/**
 * @brief Test execution data.
 */
struct board_test;

struct board_test {
    uint32_t            peridic_resp_id;
    uint32_t            cmd_canid;
    uint8_t             cmd[BOARD_TEST_CMD_LEN];
    uint16_t            chn;
    uint32_t            resp_canid;
    uint8_t             resp[BOARD_TEST_RESP_LEN];
    event_t             rx_event;
    bool (*board_response)(struct board_test *exec, uint32_t canId);
};

typedef struct board_test board_test_exec_t;

/**
 * @brief Test case handler.
 */
typedef bool (*board_test_handler)(board_test_exec_t *exec,
                                   board_test_state_e state);

/**
 * @brief Test application state machine.
 */

/**
 * @brief cmdStatus
 */

typedef enum {
    NORMAL_DEAL      = 0XA0,
    CMD_PARA_ERR     = 0XB0,
    CMD_PARA_NUM_ERR = 0XC0
} CMD_STATUS;

/**
 * @brief master or slaver mode
 */

typedef enum {
    XMASTER_MODE = 0X0,
    XSLAVE_MODE  = 0X1
} MASTER_SLAVER_MODE;

/**
 * @brief Test case.
 */
typedef struct board_test_case {
    uint32_t                cmd_id;
    board_test_handler      handler;
    int32_t                 serial_id;
    uint8_t                 period_type;
} board_test_case_t;

extern const board_test_case_t g_step_case_table[];

extern bool board_adc_reply_deal(board_test_exec_t *exec,
                                 board_test_state_e state);
extern bool board_eth_self_check_deal(board_test_exec_t *exec,
                                      board_test_state_e state);
extern bool board_flash_reply_deal(board_test_exec_t *exec,
                                   board_test_state_e state);
extern bool board_emmc_reply_deal(board_test_exec_t *exec,
                                  board_test_state_e state);
extern bool board_ddr_reply_deal(board_test_exec_t *exec,
                                 board_test_state_e state);
extern bool board_eth_100base_t1_deal(board_test_exec_t *exec,
                                      board_test_state_e state);
extern bool board_eth_1000base_t1_deal(board_test_exec_t *exec,
                                       board_test_state_e state);
extern bool board_eth_100base_tx_deal(board_test_exec_t *exec,
                                      board_test_state_e state);
extern bool board_can_reply_deal(board_test_exec_t *exec,
                                 board_test_state_e state);
extern bool board_gpio_read_reply_deal(board_test_exec_t *exec,
                                       board_test_state_e state);
extern bool board_gpio_write_reply_deal(board_test_exec_t *exec,
                                        board_test_state_e state);
extern bool board_capt_reply_deal(board_test_exec_t *exec,
                                  board_test_state_e state);
extern bool board_lin_reply_deal(board_test_exec_t *exec,
                                 board_test_state_e state);
extern bool board_uart_reply_deal(board_test_exec_t *exec,
                                  board_test_state_e state);
extern bool board_diag_reply_deal(board_test_exec_t *exec,
                                  board_test_state_e state);
extern bool board_i2c_reply_deal(board_test_exec_t *exec,
                                 board_test_state_e state);
extern bool board_sleep_reply_deal(board_test_exec_t *exec,
                                   board_test_state_e state);
extern bool board_power_off_reply_deal(board_test_exec_t *exec,
                                       board_test_state_e state);
extern bool write_eth_phy_gpio_deal(board_test_exec_t *exec,
                                    board_test_state_e state);
extern bool board_usb1_reply_deal(board_test_exec_t *exec,
                                  board_test_state_e state);

extern bool board_release_vresion_deal(board_test_exec_t *exec,
                                       board_test_state_e state);

extern bool board_store_stress_deal(board_test_exec_t *exec,
                                    board_test_state_e state);

extern int sdpe_test_main(int argc, const cmd_args *argv);
extern void _board_test_main(void);
extern board_test_state_e get_board_test_context_state(void);
extern bool can_common_response(board_test_exec_t *exec, uint32_t respCanID);
extern void set_peridic_test_interval_time(board_test_exec_t *exec);
extern void monitor_periodic_interval(void);
extern bool get_dev_master_slave_work_role(void);
extern int lin_schdule_thread(void *arg);
extern void eth_self_check_for_startingup(void);
extern void spread_spectrum_ops(void);
#endif /* BOARD_DIAG */
