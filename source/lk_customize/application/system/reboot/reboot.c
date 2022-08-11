/*
 * reboot.c
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: reboot implement.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <platform.h>
#include <platform/debug.h>
#include <lib/console.h>
#include <lk/init.h>
#include "boot.h"
#include <lib/reg.h>
#include <reg.h>
#include "__regs_base.h"

static const char *reason_fastboot = "fastboot";
static const char *reason_shutdown = "shutdown";

static int reboot_cmd(int argc, const cmd_args *argv)
{
    platform_halt_reason reason = HALT_REASON_SW_RESET;
    platform_halt_action action = HALT_ACTION_REBOOT;

    if (argc == 2) {
        if (!strcmp(argv[1].str, reason_fastboot)) {
            reason = HALT_REASON_SW_UPDATE;
        }

        if (!strcmp(argv[1].str, reason_shutdown)) {
            reason = HALT_REASON_POR;
            action = HALT_ACTION_SHUTDOWN;
        }
    }

    platform_halt(action, reason);
    return 0;
}

static int poweroff_cmd(int argc, const cmd_args *argv)
{
    platform_halt(HALT_ACTION_SHUTDOWN, HALT_REASON_POR);
    return 0;
}
void config_wakeupsrc(int wksrc, unsigned long data);
platform_halt_reason get_bootreason(void);
void get_wakeupsrc(int *wksrc, unsigned long *data);

static int setwksrc_cmd(int argc, const cmd_args *argv)
{
    int wksrc = 0;
    unsigned long sec = 0;

    if (argc < 2) {
        printf("set wakeup source: src(bit 0:rtc0, bit 1:rtc1, bit 2:ext 0; bit 3:ext 1) param(delay seconds for rtc)\n");
        return 0;
    }

    wksrc = argv[1].i;
    sec = argv[2].u;
    config_wakeupsrc(wksrc, sec);
    return 0;
}

static inline int is_sem1_handoverred(void)
{
    uint32_t pin = boot_get_pin();

    if (pin >= 9 && pin <= 13) {
        return 1;
    }

    return 0;
}

void sem_config(uint level)
{
    if (is_sem1_handoverred()) {
        int wksrc;
        unsigned long data;
        platform_halt_reason reason;
        addr_t saf_rstgen = _ioaddr(APB_RSTGEN_SAF_BASE);
        /* Sem global reset enable */
        writel(0xa, saf_rstgen);
        /* Release sem module reset */
        writel(2, saf_rstgen + 0x49000);
        writel(3, saf_rstgen + 0x49000);

        /* Waiting for sem module release done */
        while (!(readl(saf_rstgen + 0x49000) & 0x40000000));

        /* wdt1 ~ wdt6 overflow event */
        addr_t sem_base = _ioaddr(APB_SEM1_BASE);
        writel(0x49248, sem_base + 0x2a4);
        /* overflow int enable */
        writel(0x808, sem_base + 0x8);
        reason = get_bootreason();
        get_wakeupsrc(&wksrc, &data);
        printf("get reboot reason %d, wakeupsrc %d, data %ld\n", reason, wksrc,
               data);

        switch (reason) {
            case HALT_REASON_POR: //powerdown
                //soc_config_wakeup_from_rtc(0);
                //soc_powerdown();
                break;

            case HALT_REASON_SW_RESET: //reboot
                //soc_config_wakeup_from_rtc(1);
                //config_rtc();
                //soc_powerdown();
                break;

            default:
                break;
        }
    }
}

LK_INIT_HOOK(sem_config, sem_config, LK_INIT_LEVEL_PLATFORM_EARLY);

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START STATIC_COMMAND("reboot", "soft reset",
                                    (console_cmd)&reboot_cmd)
STATIC_COMMAND("poweroff", "powerdown", (console_cmd)&poweroff_cmd)
STATIC_COMMAND("setwksrc",
               "set wakeup source: src(bit 0:rtc0, bit 1:rtc1, bit 2:ext 0; bit 3:ext 1) param(delay seconds for rtc)",
               (console_cmd)&setwksrc_cmd)
STATIC_COMMAND_END(power_cmd);
#endif
