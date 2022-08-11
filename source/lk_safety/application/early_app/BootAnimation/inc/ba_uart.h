#ifndef _BA_UART_H
#define _BA_UART_H
#include <uart_hal.h>
#include "hal_port.h"

void uart_config(void** handle,hal_uart_cfg_t hal_cfg);

#if defined(ENABLE_BA_UART_A)
void sidea_tx_char(void* token,void* handle,char ch);
#endif

#if defined(ENABLE_BA_UART_B)
void sideb_rx_char(void* token,void* handle,char ch);
#endif

#endif
