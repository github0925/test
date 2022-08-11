/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    send_char.c
 * @brief   send_char function which be called by mini_printf
 */

#include <common_hdr.h>
#include <testbench/testbench.h>
#include <soc.h>
#include <uart/uart.h>

extern module_e tty;

void send_char(U8 c)
{
#if defined(CFG_DRV_UART)

    if (uart_is_enabled(tty)) {
        uart_tx(tty, &c, 1);
    }

#elif defined(CFG_TB_SYS_TB_CTRL_EN)
    tb_putchar(c);
#endif
}
