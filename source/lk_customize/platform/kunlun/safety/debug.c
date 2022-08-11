/*
 * Copyright (c) 2015 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <debug.h>
#include <arch/ops.h>
#include <stdarg.h>
#include <platform.h>
#include <dev/uart.h>
#ifdef SUPPORT_VIRT_UART
#include <dev/vuart.h>
#endif

void _dputc(char c)
{
    int port = uart_get_current_port();
    /* save the uart log to DDR buffer */
#ifdef SUPPORT_VIRT_UART

    if (vuart_is_available()) {
        if (c == '\n') {
            vuart_putc('\r');
        }

        vuart_putc(c);
    }

#endif

    if (c == '\n') {
        uart_putc(port, '\r');
    }

    uart_putc(port, c);
}

int dgetc(char *c, bool wait)
{
    int _c;
    int port = uart_get_current_port();
    /* input char from virt console or physical console */
#ifdef SUPPORT_VIRT_CONSOLE

    if ((_c = vuart_getc(port, wait)) < 0) {
        return -1;
    }

#else

    if ((_c = uart_getc(port, wait)) < 0) {
        return -1;
    }

#endif
    *c = _c;
    return 0;
}

void platform_halt(platform_halt_action suggested_action,
                   platform_halt_reason reason)
{
    arch_disable_ints();

    for (;;);
}

uint32_t debug_cycle_count(void)
{
    PANIC_UNIMPLEMENTED;
}

void platform_dputc(char c)
{
    _dputc(c);
}

int platform_dgetc(char *c, bool wait)
{
    return dgetc(c, wait);
}
