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

void soc_i2c_pin_cfg(module_e m, void *para) __attribute__((weak));
void soc_i2c_pin_cfg(module_e m, void *para)
{

}

extern void soc_uart_pin_cfg(module_e m, void *para);
extern void soc_mpu_pre_setup(void);
extern void soc_mpu_update(void);
extern void soc_fw_protect_iram(U32 en_did);
extern void ce_enable_sec_vio(U32 base, uint32_t msk);

void soc_early_init(void)
{
#if defined(DEBUG_ENABLE) && defined(ARCH_armv7_r)\
    && !defined(CFG_RAM_BUILD)
    /* TODO: figure out async abort thrown once arch_enable_async_abort
     * called on RAM build */
    arch_enable_async_abort();
    U32 v = arch_rd_sctlr();
    v |= BM_SCTLR_A;
    arch_wr_sctlr(v);
    dsb();
    isb();
#endif
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

void soc_config_clk(module_e m, clk_freq_e freq)
{
    osc_out_sel_e sel = SEL_XTAL1;
    U64 tk_tmot = 0;
    U32 gasket_addr = 0, v = 0, idiv = 133, postdiv = 2;

    switch (m) {
#if defined(CFG_BT_DEV_SDMMC)

    case SD_MMC_CTRL1:
        if (SDMMC_IDENT == freq) {
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(9),
                               SLICE_SRC(4), PREDIV(7), POSTDIV(63));
            /* src=sec_ss.fsrefclk output, output: 24MHz/2/12 = 1MHz */
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(50),
                               SLICE_SRC(2), PREDIV(1), POSTDIV(11));
        } else if (SDMMC_25MHz == freq) {
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(9),
                               SLICE_SRC(4), PREDIV(3), POSTDIV(1));
        } else if (SDMMC_50MHz == freq) {
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(9),
                               SLICE_SRC(4), PREDIV(1), POSTDIV(1));
        }

        break;

    case SD_MMC_CTRL2:
        if (SDMMC_IDENT == freq) {
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(10),
                               SLICE_SRC(4), PREDIV(7), POSTDIV(63));
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(50),
                               SLICE_SRC(2), PREDIV(1), POSTDIV(11));
        } else if (SDMMC_25MHz == freq) {
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(10),
                               SLICE_SRC(4), PREDIV(3), POSTDIV(1));
        } else if (SDMMC_50MHz == freq) {
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(10),
                               SLICE_SRC(4), PREDIV(1), POSTDIV(1));
        }

        break;

    case SD_MMC_CTRL3:
        if (SDMMC_IDENT == freq) {
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(11),
                               SLICE_SRC(4), PREDIV(7), POSTDIV(63));
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(50),
                               SLICE_SRC(2), PREDIV(1), POSTDIV(11));
        } else if (SDMMC_25MHz == freq) {
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(11),
                               SLICE_SRC(4), PREDIV(3), POSTDIV(1));
        }

        break;
#endif

    case CRYPTO_ENG2:
        /* this clock slice not be used in HW */
        break;

    case TIMER3:
        /* timer clk sourced from 24MHz */
        break;

    case USB_CTRL1:
    case PCIE1X:
    case PCIE2X:
    case DDR_SS:
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

        if (m == DDR_SS) {
            ana_osc24_output_sel(FSRC_DDR, sel);

            if (freq == 3)
                idiv = 266; /* 4266 */
            else if (freq == 2 || freq == 0) {
                idiv = 200; /* 3200 */

                if (freq == 0) {    /* 1600 */
                    postdiv = 4;
                }
            } else if (freq == 4){
                idiv = 100;
                postdiv = 4; /* 800 */
            }else if (freq == 1) {
                idiv = 133;     /* 2133 */
            } else if (freq == 5) {
                idiv = 150;
				postdiv = 2;
            } else if ((freq < 4400) && (freq > 1600)) {
                postdiv = 2;
                idiv = ((freq / 4 + 4) * 8) / 8 / 4;
            } else if ((freq >= 800) && (freq <= 1600)) {
                postdiv = 4;
                idiv = ((freq / 4 + 4) * 8 / 8) / 2;
            } else {
                DBG("Opps, invalid DDR data rate\n");
                assert(0);
            }

            freq = idiv * 8 / postdiv;
            DBG("DDR PLL: freq=%dMHz, fbdiv=%d, postdiv=%d\n", freq, idiv, postdiv);
            sc_pfpll_program(APB_PLL_DDR_BASE, PLL_FBDIV(SAFE_DIV(idiv)), PLL_REFDIV(3),
                             PLL_FRAC(0), PLL_POSTDIV1(postdiv), DIV_ABCD(3, 5, 0, 0));
            ckgen_uuu_slice_cfg(APB_CKGEN_SOC_BASE, UUU_ID(9),
                                UUU_SEL_PLL, DIV_MNPQ(0, 1, 3, 0));
        }

        if (m == USB_CTRL1) {
            ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_USB1_INDEX);
            ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_USB1_REF_INDEX);
        } else if (m == PCIE1X) {
            ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_PCIEX1_INDEX);
        } else if (m == PCIE2X) {
            ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_PCIEX2_INDEX);
        }

        if ((m == PCIE1X) || (m == PCIE2X)) {
            ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_PCIEPHY_INDEX);
            ckgen_cg_en(APB_CKGEN_SOC_BASE, CKGEN_SOC_CKGATE_PCIE_REF_INDEX);
        }

        break;
#if defined(CFG_BT_DEV_SPINOR)

    case OSPI_CTRL2:

        /* There is a div2 in ospi wrapper to divide ckgen clk then feed
         * into ospi as refclk, if phy_mode not asserted.
         * If phy_mode asserted, the div2 will be bypass-ed */
        if (FREQ_SAFE == freq) {    /* 12.5MHz, non-phy mode */
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(21),
                               SLICE_SRC(7), PREDIV(0), POSTDIV(0));
            ospi_set_clk_div(soc_get_module_base(m), 32);
        } else if (SPI_FREQ0 == freq) {  /* 25MHz, phy mode */
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(21),
                               SLICE_SRC(7), PREDIV(3), POSTDIV(1));
            ospi_set_clk_div(soc_get_module_base(m), 2);
        } else if (SPI_FREQ1 == freq) { /* 50MHz, phy mode */
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(21),
                               SLICE_SRC(7), PREDIV(1), POSTDIV(1));
            ospi_set_clk_div(soc_get_module_base(m), 2);
        } else if (SPI_FREQ2 == freq) { /* 100MHz, phy mode */
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(21),
                               SLICE_SRC(7), PREDIV(1), POSTDIV(0));
            ospi_set_clk_div(soc_get_module_base(m), 2);
        } else if (SPI_FREQ3 == freq) { /* 133MHz, phy mode */
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(21),
                               SLICE_SRC(5), PREDIV(0), POSTDIV(0)); /* src = 266MHz */
            ospi_set_clk_div(soc_get_module_base(m), 2);
        } else if (freq == SPI_FREQ0_DIV) { /* 25MHz, non-phy mode */
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(21),
                               SLICE_SRC(7), PREDIV(1), POSTDIV(0));
            ospi_set_clk_div(soc_get_module_base(m), 8);
        } else if (freq == SPI_FREQ1_DIV) { /* 50MHz, non-phy mode */
            ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(21),
                               SLICE_SRC(7), PREDIV(0), POSTDIV(0));
            ospi_set_clk_div(soc_get_module_base(m), 8);
        }

        break;
#endif
#if defined(CFG_DRV_UART)

    case UART9:
    case UART11:
    case UART13:
    case UART15:
#if defined(TC_z1)
        ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(7),
                           SLICE_SRC(4), PREDIV(0), POSTDIV(0));       /* 120MHz */
#else
        ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(7),
                           SLICE_SRC(4), PREDIV(1), POSTDIV(0));       /* 60MHz */
#endif
        break;

    case UART10:
    case UART12:
    case UART14:
    case UART16:
#if defined(TC_z1)
        ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(8),
                           SLICE_SRC(4), PREDIV(0), POSTDIV(0));       /* 120MHz */
#else
        ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(8),
                           SLICE_SRC(4), PREDIV(1), POSTDIV(0));       /* 60MHz */
#endif
        break;
#endif
    case I2C10:
        ckgen_ip_slice_cfg(APB_CKGEN_SEC_BASE, SLICE_ID(4),
                           SLICE_SRC(4), PREDIV(1), POSTDIV(0));       /* 60MHz */
        break;

    default:
        DBG("%s: Opps, invalid module %d\n", __FUNCTION__, m);
        TB_ERROR_THEN_STOP();
        break;
    }
}

void soc_pin_cfg(module_e m, void *para)
{
    switch (m) {
    case SD_MMC_CTRL1:
        break;

    case SD_MMC_CTRL2:
        break;

    case SD_MMC_CTRL3:
        break;

    case OSPI_CTRL2:
        break;

    case UART1...UART20:
        soc_uart_pin_cfg(m, para);
        break;

    case PCIE1X:
        break;

    case PCIE2X:
        break;

    case I2C10:
        soc_i2c_pin_cfg(m, para);
        break;

    default:
        DBG("%s:Opps, Invaid module %d\n", __FUNCTION__, m);
        break;
    }
}

U32 soc_get_bt_pin(void)
{
#if defined(TC_zebu)    /*zebu can not force boot pins */
    U32 v = BT_PIN_eMMC1;
#else
    U32 v = scr_bits_rd(APB_SCR_SEC_BASE, RO, 0, 4);
#endif

    return v;
}

U32 soc_get_core_id(void)
{
    return CORE1;
}

void soc_remap_to_zero(U32 addr)
{
    /* actually, the remapping been enabled by a register bit in ROMC. */
    addr = addr >> 12;
    scr_bits_wr(APB_SCR_SEC_BASE, L31,
                SCR_SEC_REMAP_CR5_SEC_AR_ADDR_OFFSET_19_0_L31_START_BIT, 20, addr);
    scr_bit_set(APB_SCR_SEC_BASE, L31,
                SCR_SEC_REMAP_CR5_SEC_AR_REMAP_OVRD_EN_L31_START_BIT);

    dsb();
    isb();
}

U32 soc_get_reset_source(void)
{
    return rg_get_reset_source(APB_RSTGEN_SEC_BASE);;
}

U32 soc_rd_storage_reg(U8 type, U32 id)
{
    U32 v = 0;

    if (RST_CYCLE_PERSIST == type) {
        v = rg_rd_gpr(APB_RSTGEN_SEC_BASE, id);
    }

    return v;
}

void soc_wr_storage_reg(U8 type, U32 id, U32 v)
{
    if (RST_CYCLE_PERSIST == type) {
        rg_wr_gpr(APB_RSTGEN_SEC_BASE, id, v);
    }
}

void soc_wdog_reset_en(void)
{
    /* set 'wdog_reset_enable[1]' (for wdt3) in rstgen.
     * 'wdog_reset_enable[0]' (for wdt1) asserted by default in safety */
    rg_glb_reset_en(APB_RSTGEN_SEC_BASE, 0x10);
}

uintptr_t soc_to_dma_address(uintptr_t cpu_addr)
{
    uintptr_t dma_addr = 0;

    if ((cpu_addr >= TCM_BASE) && (cpu_addr <= TCM_END)) {
        dma_addr = (cpu_addr - TCM_BASE) + TCM_SYSTEM_ADDR;
    } else if (cpu_addr > CR5_DDR_BASE){
        dma_addr = cpu_addr + 0x10000000;
    }else{
        dma_addr = cpu_addr;
    }

    return dma_addr;
}

/*
 * #1 Set 'XN' to RAMs (except RAM_FUNC)
 * #2 Mark non-available/non-enabled RAMs (say DDR) as 'Invalid' in case CPU
 *    speculativly fetch code/data from there
 *
 * Upper 2GB not mapped thus its attribute is default Strong Ordered or Device.
 */
static const mpu_region_desc_t mpu_rgn_list[] = {
    {0, MPU_RGN_2G, MPU_ATTR_nSHARED_DEVICE | MPU_PRIV_FULL_ACCESS},
    {ATCM_BASE, MPU_RGN_64K, MPU_ATTR_WBWA | MPU_PRIV_FULL_ACCESS | BM_MPU_XN},
    {BTCM_BASE, MPU_RGN_64K, MPU_ATTR_WBWA | MPU_PRIV_FULL_ACCESS | BM_MPU_XN},
    {
        IRAM1_BASE, MPU_RGN_1M, MPU_ATTR_WBWA | MPU_PRIV_FULL_ACCESS
#if !defined(CFG_RAM_BUILD)
        | BM_MPU_XN
#endif
    },
    {0, 0, 0}
};

void soc_mpu_pre_setup(void)
{
    mpu_setup_regions(mpu_rgn_list);
}

void soc_mpu_update(void)
{
#if defined(CFG_RAM_FUNC)
    U32 id = ARRAY_SZ(mpu_rgn_list) - 1;
    mpu_region_desc_t rg;
    rg.base = (U32)(uintptr_t)__ram_func_start;
    rg.attribute = MPU_ATTR_WB_NWA | MPU_PRIV_READ_ONLY;
    rg.size = MPU_RGN_4K;
    mpu_region_cfg(id, &rg);
#endif
}


void irq_handler(void)
{
#ifdef SHELL_USE_USB
    plat_interrupt_hdlr();
#endif
}



void ROMC_SetStickyBit(uint32_t pos)
{
    uint32_t v = readl(APB_ROMC2_BASE + 0x34);
    v |= (0x01UL << (pos % 32u));
    writel(v, APB_ROMC2_BASE + 0x34);
}


void sec_soc_vector_init(void)
{
    if (0u == rg_rd_gpr(APB_RSTGEN_SEC_BASE, 1)) {
        soc_remap_to_zero(APP_VECTOR_TBL);
        ROMC_SetStickyBit(0);
        rg_wr_gpr(APB_RSTGEN_SEC_BASE, 1, 0x52454d50u);
        arch_disable_cache(ICACHE | DCACHE);
        rg_core_reset(APB_RSTGEN_SEC_BASE, RSTGEN_SEC_CORE_RST_B_CR5_SEC_INDEX);
    } else {
        rg_wr_gpr(APB_RSTGEN_SEC_BASE, 1, 0u);
    }
}

bool soc_wdt_is_hw_en(U32 base)
{
    return APB_WDT1_BASE == base ? (1 == FUSE_WDT1_DEFUALT_EN()) : false;
}