/********************************************************
 *      Copyright(c) 2019   Semidrive  Semiconductor    *
 *      All rights reserved.                            *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#include "uart.h"

extern const uart_ops_t uart_ctl;

static uint32_t uart_enabled = 0;

int32_t uart_init(module_e m, uart_cfg_t *cfg)
{
    uintptr_t b = soc_get_module_base(m);

    int32_t res = uart_ctl.init(b, cfg);

    uart_enabled |= (0x01u << (m - UART1));

    return res;
}

int32_t uart_tx(module_e m, uint8_t *data, size_t sz)
{
    uintptr_t b = soc_get_module_base(m);
    return uart_ctl.uart_tx(b, data, sz);
}

int32_t uart_rx(module_e m, uint8_t *data, size_t sz)
{
    uintptr_t b = soc_get_module_base(m);
    return uart_ctl.uart_rx(b, data, sz);
}

bool uart_is_enabled(module_e m)
{
    return !!(uart_enabled & (0x01u << (m - UART1)));
}
