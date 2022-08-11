#include "ba_uart.h"
#include "stdlib.h"
#include <uart_hal.h>
#include <target_res.h>
#include "hal_port.h"
#include "res.h"
#include "chip_res.h"
#include <ckgen_init.h>
#include "string.h"
#include "container.h"

#define RX_COUNT_SIDEB  30
#define TX_COUNT_SIDEA  10

hal_uart_port_cfg_t ba_uargcfg = {
    .sclk = CKGEN_UART_SAF,
    .baud = 115200,
    .data_bits = UART_HAL_CHAR_8BITS,
    .stop_bits = UART_HAL_STOP_1BIT,
    .parity = UART_HAL_NO_PARITY,
    .loopback_enable = false
};
hal_uart_fifo_cfg_t ba_uartfifo = {
    .fifo_enable = false,
    .rx_trigger = UART_HAL_RX_FIFO_CHAR_1,
    .tx_trigger = UART_HAL_TX_FIFO_EMPTY
};

void uart_config(void** handle,hal_uart_cfg_t hal_cfg)
{
    hal_uart_creat_handle(handle,RES_UART_UART1);

    memcpy(&hal_cfg.port_cfg,&ba_uargcfg,sizeof(hal_uart_port_cfg_t));
    memcpy(&hal_cfg.fifo_cfg,&ba_uartfifo,sizeof(hal_uart_fifo_cfg_t));
    hal_uart_init(*handle,&hal_cfg);
}

#if defined(ENABLE_BA_UART_A)
void sidea_tx_char(void* token,void* handle,char ch)
{
    char rxdata = 0;
    uint32_t irx = TX_COUNT_SIDEA;
    while(rxdata != ch)
    {
        hal_uart_putc(handle,ch);
        thread_sleep(10);
        hal_uart_getc(handle,&rxdata);
        if(irx-- == 0)
            break;
        if(token_getstatus(token) == TOKEN_ABNORMAL)
            break;
    }
}
#endif

#if defined(ENABLE_BA_UART_B)
void sideb_rx_char(void* token,void* handle,char ch)
{
    char rxdata = 0;
    uint32_t itx = RX_COUNT_SIDEB;
    hal_uart_getc(handle,&rxdata);
    while(rxdata != ch)
    {
        thread_sleep(10);
        hal_uart_getc(handle,&rxdata);
        if(itx-- == 0)
            break;
        if(token_getstatus(token) == TOKEN_ABNORMAL)
            break;
    }
    hal_uart_putc(handle,ch);
}
#endif
