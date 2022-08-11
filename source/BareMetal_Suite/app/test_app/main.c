/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <mini_libc/mini_libc.h>
#include <service.h>
#include <soc.h>
#include <arch.h>
#include "uart/uart.h"
#include "shell/shell.h"
#if defined(BOARD)
#include "board.h"
#endif

/* workaround: if .data section is empty, objcopy not work properly */
char *prod_str = "test app";
const char *cpu_str = "Saf";
#if defined(BOARD_x9_ref)
const char *board_str = "BOARD_x9_ref";
#elif defined(BOARD_g9_ref)
const char *board_str = "BOARD_g9_ref";
#else
const char *board_str = "BOARD_not_specified";
#endif

module_e tty = TTY_UART;

int __main(int argc, char *argv[])
{
#if defined(DEBUG_ENABLE) || defined(INFO_LEVEL)
    soc_deassert_reset(TTY_UART);
    soc_pin_cfg(TTY_UART, NULL);
    soc_config_clk(TTY_UART, UART_FREQ1);
    uart_cfg_t uart_cfg;
    memclr(&uart_cfg, sizeof(uart_cfg));
    uart_cfg.parity = UART_PARITY_NONE;
    uart_cfg.stop = STOP_1BIT;
    uart_cfg.baud_rate = 115200u;
    uart_init(TTY_UART, &uart_cfg);
#endif
#if defined(BOARD)
    board_setup(0, 0, 0, 0);
#endif

    tmr_enable();

    INFO("\n\n%s: %s, built on %s at %s\n\n", cpu_str, prod_str, __DATE__, __TIME__);
#if defined(BOARD)
    INFO("Board: %s\n", board_str);
#endif

    shell_loop();

    return 0;
}
