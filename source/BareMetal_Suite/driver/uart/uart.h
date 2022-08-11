/********************************************************
 *      Copyright(c) 2019   Semidrive  Semiconductor    *
 *      All rights reserved.                            *
 ********************************************************/
#ifndef __UART_H__
#define __UART_H__

#include <soc_def.h>

typedef enum {
    RS232,
    RS422,
    RS485,
} protocol_e;

typedef enum {
    UART_PARITY_EVEN,
    UART_PARITY_ODD,
    UART_PARITY_NONE,
} parity_e;

typedef enum {
    STOP_1BIT,
    STOP_1P5BIT,
    STOP_2BIT,
} uart_stop_e;

typedef enum {
    FLOW_CTRL_HW,
    FLOW_CTRL_CHAR,
    FLOW_CTRL_NONE,
} flow_ctrl_e;

typedef struct {
    protocol_e protocol;
    parity_e parity;
    uart_stop_e stop;
    flow_ctrl_e flow_ctrl;
    U8 data_bits;
    U32 baud_rate;
} uart_cfg_t;

typedef struct {
    int32_t (*init)(uintptr_t b, uart_cfg_t *cfg);
    int32_t (*uart_tx)(uintptr_t b, uint8_t *data, size_t sz);
    int32_t (*uart_rx)(uintptr_t b, uint8_t *data, size_t sz);
} uart_ops_t;

int32_t uart_init(module_e m, uart_cfg_t *cfg);
int32_t uart_tx(module_e m, uint8_t *data, size_t sz);
int32_t uart_rx(module_e m, uint8_t *data, size_t sz);
bool uart_is_enabled(module_e m);

#endif  /* __UART_H__ */
