#include <assert.h>
#include <lk/init.h>
#include <platform.h>
#include <sys_diagnosis.h>
#include <res.h>
#include <chip_res.h>
#include <lib/reboot.h>
#include <lib/sdrv_common_reg.h>
#include <lib/console.h>
#if WITH_FRAMEWORK_SERVICE_PROPERTY
#include <dcf.h>
#include <dcf_common.h>
#endif
#include "str.h"

#define RSTGEN_GENERAL_REG(n) ((0x50 + (n)*4) << 10)

#define WAKEUP_SOURCE_NULL  0x0
#define WAKEUP_SOURCE_RTC1  0x1
#define WAKEUP_SOURCE_RTC2  0x2
#define WAKEUP_SOURCE_EXT1  0x4
#define WAKEUP_SOURCE_EXT2  0x8

enum rb_state_t {
    rbStateObserved = 1,
    rbStateConfRecv,
    rbStateDone,
    rbStateFin,
};

#define PROP_MASK(x)   (0xF << (x-1)*4)
#define PROP_STATE_GET(x,v) (uint32_t)( (v & PROP_MASK(x)) >> (x-1)*4 )
#define PROP_STATE_SET(x,v,s) (uint32_t)( (v & ~PROP_MASK(x)) | s << (x-1)*4)

void ospi_handover_entry(void);
void soc_global_rst(void);
void target_quiesce(void);

#if !STATIC_HANDOVER

#if WITH_LIB_REBOOT
#include "reboot_service.h"
#endif

#if SUPPORT_STR_MODE
#include "ddr_enter_self.bin.h"
void reset_safety_cr5(uint32_t entry);
#endif
#include <rtc_drv.h>
#include <pmu_hal.h>

static int32_t pmu_config_wakeup_source(bool en, uint8_t source)
{
    int32_t ret = 0;
    void *handle = NULL;

    if (WAKEUP_SOURCE_NULL == source) {
        printf("invalid wakeup source\n");
        return -1;
    }

    ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);

    if (ret != 0) {
        printf("get handle fail\n");
        return ret;
    }

    ret = hal_pmu_init(handle);

    if (ret != 0) {
        printf("pmu init fail\n");
        return ret;
    }

    if (source == WAKEUP_SOURCE_RTC1) {
        ret = hal_pmu_set_internal_wakeup_enable(handle, 0, en);
    }
    else if (source == WAKEUP_SOURCE_RTC2) {
        ret = hal_pmu_set_internal_wakeup_enable(handle, 1, en);
    }
    else if (source == WAKEUP_SOURCE_EXT1) {
        ret = hal_pmu_set_external_wakeup_enable(handle, 0, en);
    }
    else if (source == WAKEUP_SOURCE_EXT2) {
        ret = hal_pmu_set_external_wakeup_enable(handle, 1, en);
    }

    if (ret != 0) {
        printf("pmu set internal wakeup enable fail\n");
        return ret;
    }

    ret = hal_pmu_exit(handle);

    if (ret != 0) {
        printf("pmu exit fail\n");
        return ret;
    }

    ret = hal_pmu_release_handle(handle);

    if (ret != 0) {
        printf("release handle fail\n");
        return ret;
    }

    return ret;
}

static int32_t pmu_shut_down(void)
{
    int32_t ret = 0;
    void *handle = NULL;
    ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);

    if (ret != 0) {
        printf("get handle fail\n");
        return ret;
    }

    ret = hal_pmu_init(handle);

    if (ret != 0) {
        printf("pmu init fail\n");
        return ret;
    }

    ret = hal_pmu_powerdown(handle);

    if (ret != 0) {
        printf("pmu set pd fail\n");
        return ret;
    }

    ret = hal_pmu_exit(handle);

    if (ret != 0) {
        printf("pmu exit fail\n");
        return ret;
    }

    ret = hal_pmu_release_handle(handle);

    if (ret != 0) {
        printf("release handle fail\n");
        return ret;
    }

    return ret;
}

static void config_rtc(uint32_t rtc_base, uint32_t expired_s)
{
    if (expired_s < 1) expired_s = 1;

    rtc_update_cmp_value(rtc_base, S2RtcTick(expired_s));
    rtc_wakeup_enable(rtc_base, true, false, true);
}

static void rb_state_wait(enum rb_state_t state, int src)
{
#if WITH_FRAMEWORK_SERVICE_PROPERTY
    int val = 0;

    do {
        system_property_get(DMP_ID_REBOOT_STATUS, &val);
    } while (PROP_STATE_GET(src, val) < state);

#endif
}

static void rb_state_update(enum rb_state_t state, int src)
{
#if WITH_FRAMEWORK_SERVICE_PROPERTY
    int val = 0;
    system_property_get(DMP_ID_REBOOT_STATUS, &val);
    val = PROP_STATE_SET(src, val, state);
    system_property_set(DMP_ID_REBOOT_STATUS, val);
#endif
}

static int hard_code_wdt_clear(uint32_t resid)
{
    addr_t addr;
    int32_t idx;
    res_get_info_by_id(resid, &addr, &idx);

    if (!addr) {
        printf("wrong wdt res:0x%x\n", resid);
        ASSERT(addr);
    }

    printf("clear wdt %u\n", resid);
    //below part should be replaced by driver or hal.
    uint32_t val = readl(addr + 0x24);
    writel((val | (1 << 8)), addr + 0x24);
    return 0;
}

static int module_sw_reboot(int signal, void *args)
{
#if WITH_LIB_REBOOT
    rb_arg arg = {0};
    rb_module_e module = RB_MAX_M;

    if (args)
        arg.entry = (addr_t)args;

#endif
    uint32_t signal_src = 5;
    uint32_t wdt_res_id = RES_WATCHDOG_WDT5;

    switch (signal) {
        case WDT5____ovflow_int:
            signal_src = 5;
#if WITH_LIB_REBOOT
            module = RB_AP1_M;
#endif
            wdt_res_id = RES_WATCHDOG_WDT5;
            printf("Observed ap1 wdt reboot\n");
            break;

        case WDT6____ovflow_int:
            signal_src = 6;
#if WITH_LIB_REBOOT
            module = RB_AP2_M;
#endif
            wdt_res_id = RES_WATCHDOG_WDT6;
            printf("Observed ap2 wdt reboot\n");
            break;

        case WDT4____ovflow_int:
            signal_src = 4;
#if WITH_LIB_REBOOT
            module = RB_MP_M;
#endif
            wdt_res_id = RES_WATCHDOG_WDT4;
            printf("Observed mp wdt reboot\n");
            break;

        default:
            printf("unhandle reboot signal!\n");
            return 0;
    }

    rb_state_update(rbStateConfRecv, signal_src);
    rb_state_update(rbStateDone, signal_src);
    //rb_state_wait(rbStateFin,signal_src);//wait here to avoid redunant callback
    hard_code_wdt_clear(wdt_res_id);
#if WITH_LIB_REBOOT
    reboot_module(module, RB_RB_OPC, &arg);
#endif
    rb_state_update(rbStateObserved, signal_src);
    printf("domain reboot probe fin.\n");
    return 0;
}

static void configure_wakeup(reboot_args_t reboot_args)
{
    if ((reboot_args.args.source & WAKEUP_SOURCE_RTC1) &&
            reboot_args.args.para) {
        sdrv_common_reg_set_u32(0, SDRV_REG_BOOTREASON);
        config_rtc(APB_RTC1_BASE, reboot_args.args.para);
        pmu_config_wakeup_source(true, WAKEUP_SOURCE_RTC1);
    }

    if (reboot_args.args.source & WAKEUP_SOURCE_EXT1) {
        sdrv_common_reg_set_u32(0, SDRV_REG_BOOTREASON);
        pmu_config_wakeup_source(true, WAKEUP_SOURCE_EXT1);
        printf("Non-implemented ext1 config handler\n");
    }

    if (reboot_args.args.source & WAKEUP_SOURCE_EXT2) {
        sdrv_common_reg_set_u32(0, SDRV_REG_BOOTREASON);
        pmu_config_wakeup_source(true, WAKEUP_SOURCE_EXT2);
        printf("Non-implemented ext2 config handler\n");
    }
}


#if SUPPORT_STR_MODE
static void str_reboot_or_shutdown(int signal, reboot_args_t reboot_args,
                                   int global_reset_src)
{
    if (WDT5____ovflow_int == signal) {//AP1
        arch_invalidate_cache_range(STR_AP1_TO_SAF, CACHE_LINE);

        if (is_str_enter(STR_AP1)) { // str mode
            str_vote(STR_AP1);
            sem_map_signal(SEM1, signal, SEM_INTR_CPU, false);
        }
        else {
            if (global_reset_src == signal) {
                configure_wakeup(reboot_args);
                pmu_shut_down();
            }
        }
    }
    else if (WDT6____ovflow_int == signal) {  //AP2
        arch_invalidate_cache_range(STR_AP2_TO_SAF, CACHE_LINE);

        if (is_str_enter(STR_AP2)) { // str mode
            str_vote(STR_AP2);
            sem_map_signal(SEM1, signal, SEM_INTR_CPU, false);
        }
        else {
            if (global_reset_src == signal) {
                configure_wakeup(reboot_args);
                pmu_shut_down();
            }
        }
    }
    else {
        printf("str only support AP1/AP2 now\n");
    }

    if (check_str_all()) {
        configure_wakeup(reboot_args);
#if CONFIG_DCF_HAS_AP1
        set_str_flag_to_rtc(STR_AP1);
#endif
#if CONFIG_DCF_HAS_AP2
        set_str_flag_to_rtc(STR_AP2);
#endif
        str_clr_vote();

        if (reboot_args.args.source ==
                0) { //no wakeupsrc so direct resume from here.
            module_sw_reboot(WDT5____ovflow_int,
                             (void *)get_str_resume_entry(STR_AP1));
            sem_map_signal(SEM1, WDT5____ovflow_int, SEM_INTR_CPU, true);
#if CONFIG_DCF_HAS_AP2
            module_sw_reboot(WDT6____ovflow_int,
                             (void *)get_str_resume_entry(STR_AP2));
            sem_map_signal(SEM1, WDT6____ovflow_int, SEM_INTR_CPU, true);
#endif
        }
        else {  //resume by press wakeup src
            power_config_before_str();
            memcpy((void *)0x00100000, (void *)ddr_enter_self_image,
                   sizeof(ddr_enter_self_image));
            arch_clean_cache_range((addr_t)0x00100000,
                                   ROUNDUP(sizeof(ddr_enter_self_image), CACHE_LINE));
            reset_safety_cr5(0x00100000);

            while (1);
        }
    }
}
#endif

static int reboot_routine(int signal, void *args)
{
    reboot_args_t reboot_args;
    int global_reset_src;
    reboot_args.val = sdrv_common_reg_get_u32(SDRV_REG_BOOTREASON);
    printf("get reboot args:0x%x reson=%d signal:%d\n", reboot_args.val,
           reboot_args.args.reason, signal);
    /* Global resets are triggered only from AP. */
#if PLATFORM_G9X || PLATFORM_V9F || PLATFORM_V9TS_B || PLATFORM_D9LITE
    global_reset_src = WDT6____ovflow_int;
#else
    global_reset_src = WDT5____ovflow_int;
#endif

    switch (reboot_args.args.reason) {
        case HALT_REASON_SW_RESET:
            if (global_reset_src == signal)
                clr_str_flag(STR_AP1);
            else
                clr_str_flag(STR_AP2);

            /* clear boot reason. */
            sdrv_common_reg_set_u32(0, SDRV_REG_BOOTREASON);
            /* Module reset. */
            module_sw_reboot(signal, args);
            break;

        case HALT_REASON_SW_RECOVERY:
            /* For recovery mode */
            module_sw_reboot(signal, args);
            break;

        case HALT_REASON_POR: {
#if SUPPORT_STR_MODE
            str_reboot_or_shutdown(signal, reboot_args, global_reset_src);
#else

            /* System power down. */
            if (global_reset_src == signal) {
                configure_wakeup(reboot_args);
                pmu_shut_down();
            }

#endif
            break;
        }

        case HALT_REASON_SW_GLOBAL_POR:
        case HALT_REASON_SW_UPDATE:
            if (global_reset_src == signal) {
                clr_str_flag(STR_AP1);
                target_quiesce();
                soc_global_rst();
            }

            break;

        case HALT_REASON_UNKNOWN:
        default:
            printf("Ignored unknown reboot reason: 0x%x!\n",
                   reboot_args.args.reason);
            break;
    }

    return 0;
}

static int reboot_bootloader_test(int argc, const cmd_args *argv)
{
    reboot_args_t reboot_args = {0};
    reboot_args.args.reason = HALT_REASON_SW_UPDATE;
    sdrv_common_reg_set_u32(reboot_args.val, SDRV_REG_BOOTREASON);
    soc_global_rst();
    return 0;
}

#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START STATIC_COMMAND("rb-bl", "reboot to bootloader\n",
                                    (console_cmd)&reboot_bootloader_test)
STATIC_COMMAND_END(rb_bl);

#endif

#endif /* STATIC HANDOVER */

SYSD_CALL(reboot_probe)
{
#if STATIC_HANDOVER
    ospi_handover_entry();
#else
    reboot_args_t reboot_args;
    reboot_args.val = sdrv_common_reg_get_u32(SDRV_REG_BOOTREASON);
    rb_state_update(rbStateObserved, 4); /* MP wdg */
    sysd_register_daemon_handler(reboot_routine, NULL, WDT4____ovflow_int);
    rb_state_update(rbStateObserved, 5); /* AP1 wdg */
    sysd_register_daemon_handler(reboot_routine, NULL, WDT5____ovflow_int);
    rb_state_update(rbStateObserved, 6); /* AP2 wdg */
    sysd_register_daemon_handler(reboot_routine, NULL, WDT6____ovflow_int);

    if (reboot_args.args.reason == HALT_REASON_SW_UPDATE) {
        ospi_handover_entry();
    }

#endif
}
