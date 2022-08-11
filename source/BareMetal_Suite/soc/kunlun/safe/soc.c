/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <soc.h>
#include <pll/sc_ss_pfpll.h>
#include <clk/ckgen.h>
#include <pll/sc_ss_pfpll_reg.h>
#include <mpu/mpu.h>
#include <testbench/testbench.h>
#include <analog/analog.h>
#include <scr/scr.h>
#include <arch.h>
#include <rstgen/rstgen.h>
#include <srv_timer/srv_timer.h>
#include <mailbox/mailbox.h>
#include <wdog/wdog.h>
#include <gpio/gpio.h>
#include "str.h"
extern void soc_ospi_pin_cfg(void *para);
extern void soc_uart1_pin_cfg(void *para);
extern void soc_uart3_pin_cfg(void *para);
extern void soc_uart4_pin_cfg(void *para);
extern void soc_mpu_pre_setup(void);
extern void soc_mpu_update(void);
extern void soc_i2c_pin_cfg(module_e m, void *para);

void soc_early_init(void)
{
#if defined(DEBUG_ENABLE) && defined(ARCH_armv7_r)
    arch_enable_async_abort();
    U32 v = arch_rd_sctlr();
    v |= BM_SCTLR_A;
    arch_wr_sctlr(v);
    dsb();
    isb();
#endif
}

void soc_pre_boot(void)
{
}

void soc_init(void)
{
    if (TRUE == mpu_is_enabled()) {
        mpu_disable();
    }

    soc_mpu_pre_setup();
    soc_mpu_update();
    mpu_enable();
    DBG("MPU enabled.\n");
}

void soc_clock_init(void)
{
    DBG("%s entered\n", __FUNCTION__);
    osc_out_sel_e sel = SEL_XTAL1;
    U64 tk_tmot = tmr_tick() + SOC_us_TO_TICK(XTAL_LOCK_TIMEOUT_us);

    while ((!ana_is_xtal_ready(APB_XTAL_SAFETY_BASE))
            && (tmr_tick() < tk_tmot));

    if (tmr_tick() >= tk_tmot) {
        DBG("%s: Opps, xtal not locked in-time.\n", __FUNCTION__);
        sel = SEL_OSC;
    }

    DBG("%s: OSC24MHz source selected as %s\n", __FUNCTION__,
        sel == SEL_XTAL1 ? "XTAL1" : "RC OSC");
    ana_osc24_output_sel(FSRC_SAF, sel);
    /* 1800MHz/2 = 900MHz */
    /* div_a/b/c/d = 1800/(4/6/8/10) */
    sc_pfpll_program(APB_PLL1_BASE, PLL_FBDIV(SAFE_DIV(225)), PLL_REFDIV(3),
                     PLL_FRAC(0), PLL_POSTDIV1(2), DIV_ABCD(3, 5, 7, 9));
    /* 24/2*133/2 = 800MHz */
    /* div_a/b/c/d = 1600/(4/6/8/10) = 400/266/200/207.9*/
    sc_pfpll_program(APB_PLL2_BASE, PLL_FBDIV(SAFE_DIV(133)), PLL_REFDIV(2),
                     PLL_FRAC(5592405), PLL_POSTDIV1(2), DIV_ABCD(3, 5, 7, 9));
    /* Safe Plat, from PLL1 */
    ckgen_bus_slice_cfg(APB_CKGEN_SAF_BASE, SLICE_ID(0), PATH_B,
                        SLICE_SRC(4), PREDIV(0));
    /* switch to path B */
    ckgen_bus_slice_switch(APB_CKGEN_SAF_BASE, SLICE_ID(0));
}

void soc_config_clk(module_e m, clk_freq_e freq)
{
    osc_out_sel_e sel = SEL_XTAL1;
    U64 tk_tmot = 0;
#if DO_NOC_INIT
    U32 gasket_addr = 0, v = 0;
#endif
    U32 idiv = 133, postdiv = 2;

    switch (m) {
        case CRYPTO_ENG1:
            /* this clock slice not be used in HW */
            /*
            ckgen_ip_slice_cfg(APB_CKGEN_SAF_BASE, SLICE_ID(0),
                SLICE_SRC(5), PREDIV(1), POSTDIV(0));
            */
            break;

        case OSPI_CTRL1:
            break;

        case TIMER1:
            /* timer clk sourced from 24MHz */
            /*
            ckgen_ip_slice_cfg(APB_CKGEN_SAF_BASE, SLICE_ID(12),
                SLICE_SRC(4), PREDIV(0), POSTDIV(0));
            */
            break;
#if defined(CFG_DRV_UART)

        case UART1:
        case UART2:
        case UART3:
        case UART4:
#if defined(TC_z1)
            ckgen_ip_slice_cfg(APB_CKGEN_SAF_BASE, SLICE_ID(3),
                               SLICE_SRC(4), PREDIV(0), POSTDIV(0));       /* 160MHz */
#else
            ckgen_ip_slice_cfg(APB_CKGEN_SAF_BASE, SLICE_ID(3),             /* 80MHz */
                               SLICE_SRC(4), PREDIV(1), POSTDIV(0));
#endif
            break;
#endif

        case DDR_SS:
            str_save_ddr_freq(freq);
            tk_tmot = tmr_tick() + SOC_us_TO_TICK(XTAL_LOCK_TIMEOUT_us);

            while ((!scr_bit_get(APB_SCR_SEC_BASE, RO,
                                 SCR_SEC_XTAL_SAF_XTAL_RDY_RO_START_BIT))
                    && (tmr_tick() < tk_tmot));

            if (tmr_tick() >= tk_tmot) {
                DBG("%s: Opps, xtal not locked in-time.\n", __FUNCTION__);
                sel = SEL_OSC;
            }

            DBG("%s: HIS/HPI/DDR PLL OSC24MHz source selected as %s\n", __FUNCTION__,
                sel == SEL_XTAL1 ? "XTAL1" : "RC OSC");
            /* Workaround:
             * Unfortunately, if DDR PLL initialized by Safety, there is high chance security
             * ROM may hang when it setup HPI PLL.
             */
#if DO_NOC_INIT

            if (!sc_pfpll_is_enabled(
                        APB_PLL_HPI_BASE)) {   /* as a flag that HPI/HSI clock setup */
                /* Program HPI PLL: 237 * 8 = 1896MHz */
                /* PLL output 948*2MHz, divided by 2 in CKGEN */
                DBG("%s: to program HPI PLL...\n", __FUNCTION__);
                ana_osc24_output_sel(FSRC_HPI, sel);
                sc_pfpll_program(APB_PLL_HPI_BASE, PLL_FBDIV(SAFE_DIV(237)), PLL_REFDIV(3),
                                 PLL_FRAC(0), PLL_POSTDIV1(1), DIV_ABCD(3, 5, 7, 9));
                gasket_addr = APB_CKGEN_SOC_BASE
                              + SOC_CKGEN_REG_MAP(CKGEN_BUS_SLICE_GASKET_OFF(SLICE_ID(2)));
                v = readl(gasket_addr);
                /* DIV_M - unconnected
                 * DIV_N - APB MUX clock.  950/6MHz
                 * DIV_P - unconnected
                 * DIV_Q - MPC_DDR pclk.   950/6MHz
                 * */
                v &= ~(FM_CKGEN_BUS_SLICE_GASKET_N_DIV_NUM
                       | FM_CKGEN_BUS_SLICE_GASKET_Q_DIV_NUM);
                v |= FV_CKGEN_BUS_SLICE_GASKET_N_DIV_NUM(5)
                     | FV_CKGEN_BUS_SLICE_GASKET_Q_DIV_NUM(5);    /* DIV6 */
                writel(v, gasket_addr);

                /* until the div really takes effect.
                * It needs 8 clks + several apb_clks to sync */
                while (readl(gasket_addr) & (BM_CKGEN_BUS_SLICE_GASKET_DIV_N_BUSY
                                             | BM_CKGEN_BUS_SLICE_GASKET_DIV_Q_BUSY));

                /* HPI_NOC, from PLL_HPI */
                ckgen_bus_slice_cfg(APB_CKGEN_SOC_BASE, SLICE_ID(2), PATH_B,
                                    SLICE_SRC(4), PREDIV(1));

                while (!(readl(APB_PLL3_BASE + PLL_CTRL_OFF) & BM_PLL_CTRL_LOCK));

                ckgen_bus_slice_switch(APB_CKGEN_SOC_BASE, SLICE_ID(2));
                /* HPI600 */
                ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(51),
                                   SLICE_SRC(4), PREDIV(0), POSTDIV(0));
                /* HPI800 */
                ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(52),
                                   SLICE_SRC(4), PREDIV(0), POSTDIV(0));
                /* 1> Program CKGEN_SOC slice to get proper clk_ckgen freqency.*/
                /* src = 24MHz, hpi.fsrefclk output */
                ckgen_bus_slice_cfg(APB_CKGEN_SOC_BASE, SLICE_ID(3), PATH_B,
                                    SLICE_SRC(1), PREDIV(0));
                ckgen_bus_slice_switch(APB_CKGEN_SOC_BASE, SLICE_ID(3));
            }

#endif
#if defined(CFG_USB) || defined(CFG_PCIE)

            if ((m == USB_CTRL1 || m == PCIE1X || m == PCIE2X)
                    && (!sc_pfpll_is_enabled(
                            APB_PLL_HIS_BASE))) {   /* as a flag that HSI clock setup */
                /* 2> Release SS reset if needed */
                /* HIS 'SS' rst connected to main rst. No need to release again */
                /* 3> Sel clk_sel[0]/[1] to 0 (default value) */
                /* 4> Program PLL to get proper frequency */
                ana_osc24_output_sel(FSRC_HIS, sel);
                /* 1000MHz */
                sc_pfpll_program(APB_PLL_HIS_BASE, PLL_FBDIV(SAFE_DIV(250)), PLL_REFDIV(3),
                                 PLL_FRAC(0), PLL_POSTDIV1(2), DIV_ABCD(3, 5, 7, 9));
                /* DIV_M:    1000MHz    For test
                 * DIV_N:    100MHz     PCIPHY RefClk
                 * DIV_P:    500        AXI Clock   (HIS bus clock, max 600MHz)
                 * DIV_Q:    250        APB Clock
                 */
                ckgen_uuu_slice_cfg(APB_CKGEN_SOC_BASE, UUU_ID(10),
                                    UUU_SEL_PLL, DIV_MNPQ(0, 9, 1, 3));
            }

#endif

            if (m == DDR_SS && (!sc_pfpll_is_enabled(APB_PLL_DDR_BASE))) {
                ana_osc24_output_sel(FSRC_DDR, sel);

                if (freq == 3)
                    idiv = 266;
                else if (freq == 2 || freq == 0) {
                    idiv = 200; /* 3200 */

                    if (freq == 0) {    /* 1600 */
                        postdiv = 4;
                    }
                }
                else if (freq == 1) {
                    idiv = 133;     /* 2133 */
                } else if (freq == 5) {
				idiv = 150;
                postdiv = 2; /* 2400 */
                } else if ((freq < 4400) && (freq > 1600)) {
                    postdiv = 2;
                    idiv = ((freq / 4 + 4) * 8) / 8 / 4;
                }
                else if ((freq >= 800) && (freq <= 1600)) {
                    postdiv = 4;
                    idiv = ((freq / 4 + 4) * 8 / 8) / 2;
                }
                else {
                    DBG("Opps, invalid DDR data rate\n");
                    assert(0);
                }

                freq = idiv * 8 / postdiv;
                DBG("DDR PLL: freq=%dMHz, fbdiv=%d, postdiv=%d\n", freq, idiv, postdiv);
                ckgen_uuu_slice_cfg(APB_CKGEN_SOC_BASE, UUU_ID(9),
                                    UUU_SEL_CKGEN_SOC, DIV_MNPQ(0, 1, 3, 0));
                sc_pfpll_program(APB_PLL_DDR_BASE, PLL_FBDIV(SAFE_DIV(idiv)),
                                 PLL_REFDIV(3),
                                 PLL_FRAC(0), PLL_POSTDIV1(postdiv), DIV_ABCD(3, 5, 0, 0));
                ckgen_uuu_slice_cfg(APB_CKGEN_SOC_BASE, UUU_ID(9),
                                    UUU_SEL_PLL, DIV_MNPQ(0, 1, 3, 0));
            }

            if (m == USB_CTRL1) {
                ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_USB1_INDEX);
                ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_USB1_REF_INDEX);
            }
            else if (m == PCIE1X) {
                ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_PCIEX1_INDEX);
            }
            else if (m == PCIE2X) {
                ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_PCIEX2_INDEX);
            }

            if ((m == PCIE1X) || (m == PCIE2X)) {
                ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_PCIEPHY_INDEX);
                ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_PCIE_REF_INDEX);
            }

            break;

        case I2C1:
        case I2C3:
        case I2C4:
            ckgen_ip_slice_cfg(APB_CKGEN_SAF_BASE,
                               SLICE_ID(1),             /* 133.3MHz */
                               SLICE_SRC(5), PREDIV(1), POSTDIV(0));
            break;

        case I2C10:
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(4),
                               SLICE_SRC(4), PREDIV(1), POSTDIV(0));       /* 60MHz */
            break;

        default:
            DBG("%s:Opps, invalid module %d\n", __FUNCTION__, m);
            TB_ERROR_THEN_STOP();
            break;
    }
}

void soc_pin_cfg(module_e m, void *para)
{
    switch (m) {
#if defined(CFG_DRV_UART)

        case UART1:
            soc_uart1_pin_cfg(para);
            break;

        case UART3:
            soc_uart3_pin_cfg(para);
            break;

        case UART4:
            soc_uart4_pin_cfg(para);
            break;
#endif
#if defined(CFG_DRV_I2C)

        case I2C1:
        case I2C3:
        case I2C4:
        case I2C10:
            soc_i2c_pin_cfg(m, para);
            break;
#endif

        default:
            DBG("%s:Opps, invalid module %d\n", __FUNCTION__, m);
            break;
    }
}

U32 soc_get_bt_pin(void)
{
    U32 v = scr_bits_rd(APB_SCR_SAF_BASE, RO, 0, 4);
    return v;
}

U32 soc_get_core_id(void)
{
    return CORE2;
}

void soc_remap_to_zero(U32 addr)
{
    /* actually, the remapping been enabled by a register bit in ROMC. */
    addr = addr >> 12;
    scr_bits_wr(APB_SCR_SAF_BASE, L31,
                SCR_SAF_REMAP_CR5_SAF_AR_ADDR_OFFSET_19_0_L31_START_BIT,
                20, addr);
    scr_bit_set(APB_SCR_SAF_BASE, L31,
                SCR_SAF_REMAP_CR5_SAF_AR_REMAP_OVRD_EN_L31_START_BIT);
    dsb();
    isb();
}

U32 soc_get_reset_source(void)
{
    return rg_get_reset_source(APB_RSTGEN_SAF_BASE);
}

U32 soc_rd_storage_reg(U8 type, U32 id)
{
    U32 v = 0;

    if (RST_CYCLE_PERSIST == type) {
        v = rg_rd_gpr(APB_RSTGEN_SAF_BASE, id);
    }

    return v;
}

void soc_wr_storage_reg(U8 type, U32 id, U32 v)
{
    if (RST_CYCLE_PERSIST == type) {
        rg_wr_gpr(APB_RSTGEN_SAF_BASE, id, v);
    }
}

void soc_wdog_reset_en(void)
{
}

void soc_gpio_mux(module_e ctrl, U32 io)
{
    /* Safety ROM will not touch pins in Security domain. And furtunately,
     * pads in Safety domain defaulty mux-ed to GPIO1 */
}

bool soc_wdt_is_hw_en(U32 base)
{
    return APB_WDT1_BASE == base ? (1 == FUSE_WDT1_DEFUALT_EN()) : false;
}
uintptr_t soc_to_dma_address(uintptr_t cpu_addr)
{
    uintptr_t dma_addr = 0;

    if ((cpu_addr >= TCM_BASE) && (cpu_addr <= TCM_END)) {
        dma_addr = (cpu_addr - TCM_BASE) + TCM_SYSTEM_ADDR;
    }
    else if (cpu_addr > CR5_DDR_BASE) {
        dma_addr = cpu_addr + 0x10000000;
    }
    else {
        dma_addr = cpu_addr;
    }

    return dma_addr;
}

void __attribute__((weak)) irq_handler(void)
{
}
