
/*
 * Lin_PBCfg.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: LIN configurations.
 *
 * Revision History:
 * -----------------
 * 011, 11/23/2019 Xidong Du    Implement this file.
 */
#include "Lin.h"
#include "chip_res.h"

static const Lin_ControllerConfigType lin_controller_config[LIN_IFC_CHN_MAX] = {
#ifdef SCI1_USED
    [LIN_IFC_SCI1] = {
        .hrdChannel = RES_UART_UART7,
        .sclk = 80000000,
        .baud = 19200,
        .data_bits = UART_HAL_CHAR_8BITS,
        .stop_bits = UART_HAL_STOP_1BIT,
        .parity = UART_HAL_NO_PARITY,
        .loopback_enable = false,
        .fifo_enable = true,
        .rx_trigger = UART_HAL_RX_FIFO_CHAR_1,
        .tx_trigger = UART_HAL_TX_FIFO_EMPTY
    },
#endif

#ifdef SCI2_USED
    [LIN_IFC_SCI2] = {
#ifdef V9F_A02_REF
        .hrdChannel = RES_UART_UART2,
#else
        .hrdChannel = RES_UART_UART8,
#endif
        .sclk = 80000000,
        .baud = 19200,
        .data_bits = UART_HAL_CHAR_8BITS,
        .stop_bits = UART_HAL_STOP_1BIT,
        .parity = UART_HAL_NO_PARITY,
        .loopback_enable = false,
        .fifo_enable = true,
        .rx_trigger = UART_HAL_RX_FIFO_CHAR_1,
        .tx_trigger = UART_HAL_TX_FIFO_EMPTY
    },
#endif

#ifdef SCI3_USED
    [LIN_IFC_SCI3] = {
        .hrdChannel = RES_UART_UART3,
        .sclk = 80000000,
        .baud = 19200,
        .data_bits = UART_HAL_CHAR_8BITS,
        .stop_bits = UART_HAL_STOP_1BIT,
        .parity = UART_HAL_NO_PARITY,
        .loopback_enable = false,
        .fifo_enable = true,
        .rx_trigger = UART_HAL_RX_FIFO_CHAR_1,
        .tx_trigger = UART_HAL_TX_FIFO_EMPTY
    },
#endif

#ifdef SCI4_USED
    [LIN_IFC_SCI4] = {
        .hrdChannel = RES_UART_UART4,
        .sclk = 80000000,
        .baud = 19200,
        .data_bits = UART_HAL_CHAR_8BITS,
        .stop_bits = UART_HAL_STOP_1BIT,
        .parity = UART_HAL_NO_PARITY,
        .loopback_enable = false,
        .fifo_enable = true,
        .rx_trigger = UART_HAL_RX_FIFO_CHAR_1,
        .tx_trigger = UART_HAL_TX_FIFO_EMPTY
    },
#endif
};

const Lin_ConfigType lin_config = {
    .Count = LIN_IFC_CHN_MAX,
    .Config = (Lin_ControllerConfigType *)lin_controller_config,
};
