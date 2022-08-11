#include <assert.h>
#include <reg.h>
#include <debug.h>
#include "__regs_base.h"
#include "pmu_hal.h"
#include "tca9539.h"
#if SUPPORT_BOARDINFO
#include "boardinfo_hwid_usr.h"
#endif

#define IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR (APB_IOMUXC_RTC_BASE)
#define REG_AP_APB_IOMUX_CTRL_RTC_PIN_MUX_CONFIG_SYS_WAKEUP0 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x214<<0))
#define REG_AP_APB_IOMUX_CTRL_RTC_IO_PAD_CONFIG_SYS_WAKEUP0 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + (0x18<<0))
#define REG_AP_APB_IOMUX_CTRL_RTC_PIN_MUX_CONFIG_SYS_WAKEUP1 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + 0x218)
#define REG_AP_APB_IOMUX_CTRL_RTC_IO_PAD_CONFIG_SYS_WAKEUP1 (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + 0x1C)

#define RTC_IO_PAD_CONFIG_SYS_PWR_ON   (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + 0x14)
#define RTC_IO_PAD_CONFIG_SYS_CTRL0    (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + 0x20)
#define RTC_IO_PAD_CONFIG_SYS_CTRL1    (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + 0x24)
#define RTC_IO_PAD_CONFIG_SYS_CTRL2    (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + 0x28)
#define RTC_IO_PAD_CONFIG_SYS_CTRL3    (IOMUX_CTRL_RTC_APB_AB0_BASE_ADDR + 0x2C)

static void can_lin_trcv_sleep(void)
{
    int board_ver = 2;
    struct tca9539_device *pd;

#if SUPPORT_BOARDINFO
    board_ver = get_part_id(PART_BOARD_ID_MIN);
#endif

    pd = tca9539_init(3, 0x74);
    ASSERT(pd);

    if (board_ver == 2) {
        pd->ops.output_enable(pd, 11);
        pd->ops.output_val(pd, 11, 0);  /* CAN1_NTSB */

        /* Wait for transceiver sleeping. */
        spin(100U);
    }
    else if (board_ver == 3) {
        pd->ops.output_enable(pd, 11);
        pd->ops.output_val(pd, 11, 0);  /* CAN1_NTSB */

        pd->ops.output_enable(pd, 1);
        pd->ops.output_val(pd, 1, 0);   /* LIN0_EN */
        pd->ops.output_enable(pd, 6);
        pd->ops.output_val(pd, 6, 0);   /* LIN1_EN */

        /* Wait for transceiver sleeping. */
        spin(1000U);
    }
    else {
        dprintf(ALWAYS, "Unknown board, may sleep failed\n");
    }

    tca9539_deinit(pd);
}

static void wake_pin_en(void)
{
    int addr;
    int wdata;
    //PAD_NAME: *****SYS_WAKEUP0****
    //MUX_NAME: *****PMU_WAKEUP0****
    //config pad oe bit 4 and pad mux bit 0,1,2
    addr = REG_AP_APB_IOMUX_CTRL_RTC_PIN_MUX_CONFIG_SYS_WAKEUP0;
    wdata = 1;
    wdata = ((wdata & 0xffffffef) | (1 << 4));
    writel(wdata, addr);
    //config select bit 0,1,2,3
    //no select define in register h file

    /* Wakeup1 */
    writel(wdata, REG_AP_APB_IOMUX_CTRL_RTC_PIN_MUX_CONFIG_SYS_WAKEUP1);
}

static void wake_pin_pull_down(void)
{
    int addr;
    int wdata;

    addr = REG_AP_APB_IOMUX_CTRL_RTC_IO_PAD_CONFIG_SYS_WAKEUP0;
    wdata = readl(addr);
    wdata = ((wdata & 0xfffffffc) | 1 );
    writel(wdata, addr);

    addr = REG_AP_APB_IOMUX_CTRL_RTC_IO_PAD_CONFIG_SYS_WAKEUP1;
    wdata = readl(addr);
    wdata = ((wdata & 0xfffffffc) | 1 );
    writel(wdata, addr);
}

static void rtc_domain_pin_config(void)
{
    uint32_t reg_addr[5] = {RTC_IO_PAD_CONFIG_SYS_PWR_ON,
                            RTC_IO_PAD_CONFIG_SYS_CTRL0,
                            RTC_IO_PAD_CONFIG_SYS_CTRL1,
                            RTC_IO_PAD_CONFIG_SYS_CTRL2,
                            RTC_IO_PAD_CONFIG_SYS_CTRL3};
    int addr, wdata;

    /* Disable internal pull-up and pull-down. */
    for (size_t i = 0; i < sizeof(reg_addr) / sizeof(reg_addr[0]);
         i++) {
        addr = reg_addr[i];
        wdata = readl(addr);
        wdata &= 0xFFFFFFFE;
        writel(wdata, addr);
    }
}

static void disable_osc_and_por(void)
{
    /* Oscillator setting from register. */
    RMWREG32(APB_RC_RTC_BASE, 14, 1, 0);
    /* select xtal_32k */
    RMWREG32(APB_RC_RTC_BASE + 4, 0, 1, 1);
    /* disable osc (rc_rtc) */
    RMWREG32(APB_RC_RTC_BASE, 0, 1, 0);
    /* disable POR */
    RMWREG32(APB_PMU_BASE + 0xC, 9, 1, 1);
}

static void en_ewkp_and_power_down(bool en_wk0, uint8_t wk0_level,
                                   bool en_wk1, uint8_t wk1_level)
{
    void *handle;

    /*
     * Enable Wakeup0 and Wakeup1 pin.
     */
    ASSERT(!hal_pmu_creat_handle(&handle, RES_PMU_PMU));
    ASSERT(!hal_pmu_init(handle));
    /* Wakeup0 high level valid. */
    ASSERT(!hal_pmu_set_external_wakeup_polarity(handle, 0, wk0_level));
    ASSERT(!hal_pmu_set_external_wakeup_enable(handle, 0, en_wk0));
    /* Wakeup1 high levle valid. */
    ASSERT(!hal_pmu_set_external_wakeup_polarity(handle, 1, wk1_level));
    ASSERT(!hal_pmu_set_external_wakeup_enable(handle, 1, en_wk1));

    /* Sleep. */
    ASSERT(!hal_pmu_powerdown(handle));

    ASSERT(!hal_pmu_exit(handle));
    ASSERT(!hal_pmu_release_handle(handle));
}

static void board_sleep(bool en_wk0, uint8_t wk0_level,
                        bool en_wk1, uint8_t wk1_level)
{
    can_lin_trcv_sleep();
    rtc_domain_pin_config();
    disable_osc_and_por();
    wake_pin_en();
    wake_pin_pull_down();
    en_ewkp_and_power_down(en_wk0, wk0_level, en_wk1, wk1_level);
}

#if WITH_LIB_CONSOLE
#include <lib/console.h>

static int sleep(int argc, const cmd_args *argv)
{
    if (argc < 5) {
        printf("Usage: low_power <en_wk0> <wk0_level> <en_wk1> <wk1_level>");
        return 1;
    }

    board_sleep(argv[1].u, argv[2].u, argv[3].u, argv[4].u);

    return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("low_power", "G9 ref board sleep", sleep)
STATIC_COMMAND_END(sleep);
#endif
