#include <common_hdr.h>
#include <soc.h>
#include <pll/sc_ss_pfpll.h>
#include <clk/ckgen.h>
#include <pll/sc_ss_pfpll_reg.h>
#include <mpu/mpu.h>
#include <testbench/testbench.h>
#include <scr/scr.h>
#include <arch.h>
#include <rstgen/rstgen.h>
#include <analog/analog.h>
#include <mailbox/mailbox.h>
#include <srv_timer/srv_timer.h>
#include <clk/ckgen_reg.h>
#include <gpio/gpio.h>
#include <funcsafe/sem.h>
#include "plat_interrupt.h"
#include "soc_def.h"

void __attribute__((weak)) do_irq(void)
{

}

void platform_irq(void)
{
    do_irq();
}

void platform_fiq(void)
{
#ifdef SHELL_USE_USB
    plat_interrupt_hdlr();
#endif
}

void soc_pin_cfg(module_e m, void *para)
{

}

void soc_config_clk(module_e m, clk_freq_e freq)
{
    osc_out_sel_e sel = SEL_XTAL1;
    U64 tk_tmot = 0;
    (void)freq;
    U32 gasket_addr = 0, v = 0;
    switch (m) {
        case USB_CTRL1:
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

            if (!sc_pfpll_is_enabled(APB_PLL_HPI_BASE)) {   /* as a flag that HPI/HSI clock setup */
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

            if ((m == USB_CTRL1 || m == PCIE1X || m == PCIE2X)
                && (!sc_pfpll_is_enabled(APB_PLL_HIS_BASE))) {   /* as a flag that HSI clock setup */
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
            break;
            default:
            break;
    }
}

uintptr_t soc_to_dma_address(uintptr_t cpu_addr)
{
    return cpu_addr;
}

bool soc_wdt_is_hw_en(U32 base)
{
    return APB_WDT1_BASE == base ? (1 == FUSE_WDT1_DEFUALT_EN()) : false;
}
