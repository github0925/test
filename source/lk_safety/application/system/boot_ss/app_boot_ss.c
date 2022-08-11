/*
 * Copyright (c) 2019 Semidrive Inc.
 *
 */
#include <assert.h>
#include <lib/console.h>
#include <lk/init.h>
#include <stdlib.h>

#include "clkgen_hal.h"
#include "cpu_hal.h"
#include "dcf.h"
#include "image_cfg.h"
#include "lib/reg.h"
#include "lib/reboot.h"
#include "lib/sdrv_common_reg.h"
#include "reboot_service.h"
#ifdef ENABLE_RTC
#include "rtc_drv.h"
#endif
#include "str.h"

#define BOOT_REASON_DETECT_LEVEL (LK_INIT_LEVEL_PLATFORM + 1)
#define BOOT_AP_LEVEL            (BOOT_REASON_DETECT_LEVEL + 1)

extern void ospi_handover_entry(void);

static void dcf(uint32_t level)
{
    dcf_init();
#if CONFIG_USE_SYS_PROPERTY
    system_property_set(DMP_ID_DDR_STATUS, 0);
    system_property_set(DMP_ID_PORT_STATUS, 0);
#ifndef WITH_APPLICATION_EARLY_APP
    system_property_set(DMP_ID_DC_STATUS, 1);
#endif
    system_property_set(DMP_ID_HANDOVER_STATUS, 1);
    system_property_set(DMP_ID_PLL_CLK_STATUS, 1);
#endif
}

static void boot_sec(uint32_t level)
{
    rb_arg arg = {0};
    arg.entry = IRAM2_BASE;
    arg.flags |= RB_COLD;
    reboot_module(RB_SEC_M, RB_RB_OPC, &arg);
}

static void boot_mp(uint32_t level)
{
#ifdef SDPE_MEMBASE
    rb_arg arg;
    arg.entry = SDPE_MEMBASE;
    reboot_module(RB_MP_M, RB_RB_OPC, &arg);
#endif
}

static void boot_ap2(uint32_t level)
{
    rb_arg arg;

    if (is_str_resume(STR_AP2)) {
        arg.entry = get_str_resume_entry(STR_AP2);
    }
    else {
#ifdef AP2_PRELOADER_MEMBASE
        arg.entry = AP2_PRELOADER_MEMBASE;
#elif PLATFORM_V9TS_B
        arg.entry = AP1_PRELOADER_MEMBASE;
#endif
    }

    reboot_module(RB_AP2_M, RB_RB_OPC, &arg);
}

static void boot_ap1(uint32_t level)
{
#ifdef AP1_PRELOADER_MEMBASE
    rb_arg arg;

    if (is_str_resume(STR_AP1))
        arg.entry = get_str_resume_entry(STR_AP1);
    else
        arg.entry = AP1_PRELOADER_MEMBASE;

    reboot_module(RB_AP1_M, RB_RB_OPC, &arg);
#else
    panic("no entry for ap1!\n");
#endif
}

int ap2_boot(int argc, const cmd_args *argv)
{
    boot_ap2(0);
    return 0;
}

int ap1_boot(int argc, const cmd_args *argv)
{
    boot_ap1(0);
    return 0;
}

int j2mp(int argc, const cmd_args *argv)
{
    bool ret;
    uint64_t entry;
    void *cpu_handle = NULL;
    entry = strtoul(argv[1].str, NULL, 16);
    ret = hal_cpu_create_handle(&cpu_handle);
    ASSERT(ret);
    ret = hal_cpu_boot(cpu_handle, CPU_ID_MP, entry);
    ASSERT(ret);
    hal_cpu_release_handle(cpu_handle);
    return 0;
}

static void boot_ap(uint32_t level)
{
#if PLATFORM_G9X || PLATFORM_V9F || PLATFORM_V9TS_B || PLATFORM_BF200 || PLATFORM_D9LITE
    boot_ap2(0);
#else
#if CONFIG_DCF_HAS_AP2

    if (is_str_resume(STR_AP2))
        boot_ap2(0);

#endif
    boot_ap1(0);
#endif
}

#ifdef ENABLE_RTC
static int rtc_switch_osc_thread(void *arg)
{
    /* For rtc_xtal 32KHz, the max locked time may be
     * 1s, so needs to poll the status and switch clock source in a thread.
     * */
    while (!rtc_switch_osc(false)) {
        thread_sleep(10);
    }

    return 0;
}
#endif

static void rtc_check(uint32_t level)
{
#ifdef ENABLE_RTC
    thread_t *t = thread_create("rtc-check-thread", rtc_switch_osc_thread,
                                NULL,
                                DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_detach_and_resume(t);
#endif
}

static int reboot_reason_detect(void)
{
    reboot_args_t reboot_args;
    reboot_args.val = sdrv_common_reg_get_u32(SDRV_REG_BOOTREASON);

    if (reboot_args.args.reason == HALT_REASON_SW_UPDATE) {
        /* Never return, it will poll boot reason "sw reset" and reboot after SW updated */
        ospi_handover_entry();
    }

    return 0;
}

LK_INIT_HOOK(boot_sec, boot_sec, LK_INIT_LEVEL_THREADING)
#if defined(PLATFORM_G9X)||defined(PLATFORM_G9Q)
LK_INIT_HOOK(boot_mp, boot_mp, LK_INIT_LEVEL_PLATFORM)
#endif
LK_INIT_HOOK(dcf, dcf, LK_INIT_LEVEL_PLATFORM)
LK_INIT_HOOK(rtc_check, rtc_check, LK_INIT_LEVEL_PLATFORM)
LK_INIT_HOOK(boot_ap, boot_ap, BOOT_AP_LEVEL)
LK_INIT_HOOK(reboot_reason_detect, (lk_init_hook)reboot_reason_detect,
             BOOT_REASON_DETECT_LEVEL);

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START STATIC_COMMAND("ap2", "boot ap2",
                                    (console_cmd)&ap2_boot)
STATIC_COMMAND("ap1", "boot ap1", (console_cmd)&ap1_boot)
STATIC_COMMAND("j2mp", "j2mp", (console_cmd)&j2mp)
STATIC_COMMAND_END(boot_ss);
#endif

