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
#include "wdg_hal.h"
#include "chip_res.h"
#include "lib/reg.h"
#include "lib/sdrv_common_reg.h"
#ifdef SUPPORT_VIRT_UART
#include <dev/vuart.h>
#endif

#ifndef RSTGEN_GENERAL_REG
#define RSTGEN_GENERAL_REG(n) ((0x50 + (n)*4) << 10)
#endif

#define RSTGEN_BOOT_REASON_REG (APB_RSTGEN_SEC_BASE + RSTGEN_GENERAL_REG(4))

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
/*RSTGEN_BOOT_REASON_REG:bit 0~3:bootreason, bit 4~7:wakeup source, bit 8~31: params */
static void config_bootreason(platform_halt_reason reason)
{
    RMWREG32(_ioaddr(RSTGEN_BOOT_REASON_REG), 0, 4, reason);
    return;
}
platform_halt_reason get_bootreason(void)
{
    platform_halt_reason reason;
    uint32_t v = readl(_ioaddr(RSTGEN_BOOT_REASON_REG));
    reason = v & 0xf;
    return reason;
}

void config_wakeupsrc(int wksrc, unsigned long data)
{
    RMWREG32(_ioaddr(RSTGEN_BOOT_REASON_REG), 4, 28,
             (wksrc & 0xf) | ((data & 0xffffff) << 4));
    return;
}

void get_wakeupsrc(int *wksrc, unsigned long *data)
{
    uint32_t v = readl(_ioaddr(RSTGEN_BOOT_REASON_REG));

    if (wksrc) {
        *wksrc = (v >> 4) & 0xf;
    }

    if (data) {
        *data = (v >> 8) & 0xffffff;
    }

    return;
}

void platform_halt(platform_halt_action suggested_action,
                   platform_halt_reason reason)
{
    void *watchdog_handle = NULL;
    bool ret;
    wdg_app_config_t wdg_app_cfg;

    switch (suggested_action) {
        case HALT_ACTION_HALT:
            arch_disable_ints();

            for (;;);

            break;

        case HALT_ACTION_REBOOT:
        case HALT_ACTION_SHUTDOWN:
            hal_wdg_creat_handle(&watchdog_handle, RES_WATCHDOG_WDT3);

            if (watchdog_handle) {
                wdg_app_cfg.workMode = wdg_mode1;
                ret = hal_wdg_init(watchdog_handle, &wdg_app_cfg);

                if (ret) {
                    hal_wdg_set_timeout(watchdog_handle, 100);
                    hal_wdg_enable_interrupts(watchdog_handle);
                    /* set boot reason */
                    config_bootreason(reason);
                    /* clean up system status notice reg */
                    sdrv_common_reg_set_u32(SDRV_REG_STATUS, 0);
                    hal_wdg_enable(watchdog_handle);
                    arch_disable_ints();

                    for (;;);
                }
            }
            else {
                hal_wdg_deinit(watchdog_handle);
                hal_wdg_release_handle(watchdog_handle);
                printf("reboot/power off failed\n");
            }

        default:
            arch_disable_ints();

            for (;;);
    }
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

