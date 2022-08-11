/*
 * sem_monitor.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SEM error monitor.
 *
 * Revision History:
 * -----------------
 */
#include <app.h>
#include <kernel/event.h>
#include <lib/console.h>
#include <sys_diagnosis.h>

/*
 * NOTE: GPV_SAF_BASE is different for AP and safety domain, but
 * APB_GPV_SAF_BASE is invalid in safety __regs_base.h.
 */
#define SAF_SAF_GPV_BASE    0xF4200000

static volatile bool g_enable_sem_monitor;


static void sem_clear_error(uint32_t addr)
{
    uint32_t val;

    RMWREG32(addr + 0x2c, 1, 1, 0x0);
    writel(0x3, addr + 0x30);

    RMWREG32(addr + 0x3c, 0, 16, 0x3000);   /* set bisttimeout1 reg */
    RMWREG32(addr + 0x40, 0, 8, 0xFF);      /* set bisttimeout2 reg */
    RMWREG32(addr + 0x34, 0, 1, 0x1);       /* start bist seq */

    /* polling bistdone stat */
    do {
        val = readl(addr + 0x38);
    } while (!(val & 0x1));

    RMWREG32(addr + 0x34, 1, 1, 0x1);   /* clear bistdone */
    RMWREG32(addr + 0x30, 0, 2, 0x3);   /* clear latent int */
}

static int sem_monitor(int signal,void* args)
{

    printf("SEM signal %d toggled!\n", signal);

    switch (signal) {
        case NOC_SEC____PD_SEC_mainMissionInt:
            sem_clear_error(APB_GPV_SEC_BASE);
            break;

        case NOC_MAIN_____SOC_mainMissionIntBar:
            sem_clear_error(APB_GPV_HPIA_BASE + 0x2880);
            break;

        case NOC_SAF____PD_SAF_mainMissionInt:
            sem_clear_error(SAF_SAF_GPV_BASE);
            break;

        default:
            break;
    }

    return 0;
}

static int sem_monitor_on(int argc, const cmd_args *argv)
{
    if (argc != 2) {
        dprintf(CRITICAL, "input argv error\n");
        return -1;
    }

    static bool registered = false;

    if(argv[1].i == 1)
    {
        if(!registered)
        {
            registered = true;

            sysd_register_handler(sem_monitor,NULL,3,
            NOC_SEC____PD_SEC_mainMissionInt,
            NOC_MAIN_____SOC_mainMissionIntBar,
            NOC_SAF____PD_SAF_mainMissionInt);
        }

    }

    return 0;
}

// LK console cmd
#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
STATIC_COMMAND_START
STATIC_COMMAND("sem_on", "sem_on 1 or sem_on 0", (console_cmd)&sem_monitor_on)
STATIC_COMMAND_END(sem_monitor_test);
#endif
