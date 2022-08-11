/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    soc.c
 * @brief   soc specific code
 */

#include <common_hdr.h>
#include <fuse_ctrl/fuse_ctrl.h>
#include <soc.h>
#include <arch.h>
#include <testbench/testbench.h>
#include <scr/scr.h>
#include <rstgen/rstgen.h>
#include <mpu/mpu.h>
#include <wdog/wdog.h>
#include <hw_rng/hw_rng.h>

U32 soc_read_fuse(U32 id) __attribute__((weak));
U32 soc_read_fuse(U32 id)
{
    return fuse_read(id);
}

U32 soc_sense_fuse(U32 id) __attribute__((weak));
U32 soc_sense_fuse(U32 id)
{
    uint32_t v = 0;
    fuse_sense(id, &v);
    return v;
}

void soc_deassert_reset(module_e m) __WEAK__;
void soc_deassert_reset(module_e m)
{
    switch (m) {
        case SD_MMC_CTRL1:
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_MSHC1_INDEX, 1);
            /* once pin mux-ed to phy owner (mshc1/2), phy reset will be taken over
             * by msch register, otherwise by scr */
            /*scr_bit_set(APB_SCR_SEC_BASE, RW,
                    SCR_SEC_MSHC1_PHY_RESETB_SCR_RW_START_BIT);*/
            break;

        case SD_MMC_CTRL2:
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_MSHC2_INDEX, 1);
            /*scr_bit_set(APB_SCR_SEC_BASE, RW,
                    SCR_SEC_MSHC2_PHY_RESETB_SCR_RW_START_BIT);*/
            break;

        case SD_MMC_CTRL3:
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_MSHC3_INDEX, 1);
            /* Once pin muxed to SD3, MSHC2 phy reset taken over by MSHC3 reg.*/
            /*scr_bit_set(APB_SCR_SEC_BASE, RW,
                    SCR_SEC_MSHC2_PHY_RESETB_SCR_RW_START_BIT);*/
            break;
#if defined(CFG_BT_DEV_SDMMC_CTRL4)

        case SD_MMC_CTRL4:
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_MSHC4_INDEX, 1);
            break;
#endif

        case OSPI_CTRL2:
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_OSPI2_INDEX, 1);
            break;

        case USB_CTRL1:
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_USB_SS_I_USB1_POR_RST_B_INDEX, 1);
            break;

        case PCIE1X:
        case PCIE2X:
            if (PCIE1X == m) {
                rg_module_reset(APB_RSTGEN_SEC_BASE,
                                RSTGEN_SEC_MODULE_RST_B_PCIE_SS_I_PCIEX1_PRESETN_INDEX, 1);
            }
            else {
                rg_module_reset(APB_RSTGEN_SEC_BASE,
                                RSTGEN_SEC_MODULE_RST_B_PCIE_SS_I_PCIEX2_PRESETN_INDEX, 1);
            }

            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_PCIE_SS_I_PHY_PRESETN_INDEX, 1);
            break;

        case CRYPTO_ENG2:
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_CE2_INDEX, 1);
            break;

        case UART1...UART16:
            break;

        case OSPI_CTRL1:
            rg_module_reset(APB_RSTGEN_SAF_BASE,
                            RSTGEN_SAF_MODULE_RST_B_OSPI1_INDEX, 1);
            break;

        case CRYPTO_ENG1:
        case I2C3:
            break;

        case DDR_SS:
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_DDR_SS_RST_N_INDEX, 1);
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_DDR_SS_SW_APB_RSTN_INDEX, 1);
            break;

        case GIC:
#if defined(TGT_ap)
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_GIC4_INDEX, 1);
#elif defined(TGT_sec)
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_GIC2_INDEX, 1);
#endif
            break;

        default:
            DBG("%s: Opps, invalid module %d\n", __FUNCTION__, m);
            TB_ERROR_THEN_STOP();
            break;
    }
}

void soc_assert_reset(module_e m) __WEAK__;
void soc_assert_reset(module_e m)
{
    switch (m) {
        case SYSTEM:
#if defined(TGT_safe)
            rg_glb_reset_en(APB_RSTGEN_SAF_BASE, 1);    // self sw reset enable
            rg_glb_self_reset(APB_RSTGEN_SAF_BASE, 1);
#elif defined(TGT_sec)
            rg_glb_reset_en(APB_RSTGEN_SEC_BASE, 1);    // self sw reset enable
            rg_glb_self_reset(APB_RSTGEN_SEC_BASE, 1);
#endif
            break;

        case USB_CTRL1:
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_USB_SS_I_USB1_POR_RST_B_INDEX, 0);
            break;

        case DDR_SS:
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_DDR_SS_SW_AXI_RSTN_INDEX, 1);
            rg_module_reset(APB_RSTGEN_SEC_BASE,
                            RSTGEN_SEC_MODULE_RST_B_DDR_SS_SW_DDR_CORE_RSTN_INDEX, 1);
            break;

        default:
            TB_ERROR_THEN_STOP();
            break;
    }
}

void system_reset(void)
{
    dsb();
    isb();
    soc_assert_reset(SYSTEM);

    while (1);
}

#if defined(CFG_SOC_API_soc_en_isolation)
void soc_en_isolation(module_e m)
{
    switch (m) {
        case USB_CTRL1:
            rg_en_isolation(APB_RSTGEN_SEC_BASE, 1);
            break;

        default:
            DBG("%s: Opps, module %d not supported.\n", __FUNCTION__, m);
            break;
    }
}
#endif


void soc_dis_isolation(module_e m)
{
    /*
     * sec_ss_rstgen_sec_iso_b[0]  - pcie_phy_iso_b
     * sec_ss_rstgen_sec_iso_b[1]  - usb_phy_iso_b
     * sec_ss_rstgen_sec_iso_b_2   - CPU1/2
     * sec_ss_rstgen_sec_iso_b_3   - GPU1/2
     * sec_ss_rstgen_sec_iso_b_4   - DDR
     */
    switch (m) {
        case USB_CTRL1:
            rg_dis_isolation(APB_RSTGEN_SEC_BASE, 1);
            break;

        case PCIE1X:
        case PCIE2X:
            rg_dis_isolation(APB_RSTGEN_SEC_BASE, 0);
            break;

        case DDR_SS:
            rg_dis_isolation(APB_RSTGEN_SEC_BASE, 4);
            break;

        case CPU1_SS:
        case CPU2_SS:
            rg_dis_isolation(APB_RSTGEN_SEC_BASE, 2);
            break;

        default:
            DBG("%s: Opps, module %d not supported.\n", __FUNCTION__, m);
            break;
    }
}

life_cycle_e soc_get_lc(void)
{
    life_cycle_e lc = LC_PROD;

    if (scr_bit_get(SCR_BASE, RO, SCR_SAF_EFUSEC_FA_CFG_0_RO_START_BIT)) {
        lc = LC_FAIL;
    }
    else if (scr_bit_get(SCR_BASE, RO,
                         SCR_SAF_EFUSEC_MISC_CFG_7_RO_START_BIT)) {
        lc = LC_PROD;
    }
    else if (scr_bit_get(SCR_BASE, RO,
                         SCR_SAF_EFUSEC_MANU_CFG_88_RO_START_BIT)) {
        lc = LC_DEV;
    }
    else {
        lc = LC_ATE;
    }

    return lc;
}

uintptr_t soc_get_module_base(module_e m) __attribute__((weak));
uintptr_t soc_get_module_base(module_e m)
{
    return m == GPIO1 ? APB_GPIO1_BASE :
           m == GPIO2 ? APB_GPIO2_BASE :
           m == GPIO3 ? APB_GPIO3_BASE :
           m == GPIO4 ? APB_GPIO4_BASE :
           m == GPIO5 ? APB_GPIO5_BASE :
           m == TIMER1 ? APB_TIMER1_BASE :
           m == TIMER2 ? APB_TIMER2_BASE :
           m == TIMER3 ? APB_TIMER3_BASE :
           m == TIMER4 ? APB_TIMER4_BASE :
           m == TIMER5 ? APB_TIMER5_BASE :
           m == UART1 ? APB_UART1_BASE :
           m == UART2 ? APB_UART2_BASE :
           m == UART3 ? APB_UART3_BASE :
           m == UART4 ? APB_UART4_BASE :
           m == UART5 ? APB_UART5_BASE :
           m == UART6 ? APB_UART6_BASE :
           m == UART7 ? APB_UART7_BASE :
           m == UART8 ? APB_UART8_BASE :
           m == UART9 ? APB_UART9_BASE :
           m == UART10 ? APB_UART10_BASE :
           m == UART11 ? APB_UART11_BASE :
           m == UART12 ? APB_UART12_BASE :
           m == UART13 ? APB_UART13_BASE :
           m == UART14 ? APB_UART14_BASE :
           m == UART15 ? APB_UART15_BASE :
           m == UART16 ? APB_UART16_BASE :
           m == CANFD1 ? APB_CAN1_BASE :
           m == CANFD2 ? APB_CAN2_BASE :
           m == CANFD3 ? APB_CAN3_BASE :
           m == CANFD4 ? APB_CAN4_BASE :
           m == CANFD5 ? APB_CAN5_BASE :
           m == CANFD6 ? APB_CAN6_BASE :
           m == CANFD7 ? APB_CAN7_BASE :
           m == CANFD8 ? APB_CAN8_BASE :
           m == PWM1 ? APB_PWM1_BASE :
           m == PWM2 ? APB_PWM2_BASE :
           m == PWM3 ? APB_PWM3_BASE :
           m == PWM4 ? APB_PWM4_BASE :
           m == PWM5 ? APB_PWM5_BASE :
           m == PWM6 ? APB_PWM6_BASE :
           m == PWM7 ? APB_PWM7_BASE :
           m == PWM8 ? APB_PWM8_BASE :
           m == I2C1 ? APB_I2C1_BASE :
           m == I2C2 ? APB_I2C2_BASE :
           m == I2C3 ? APB_I2C3_BASE :
           m == I2C4 ? APB_I2C4_BASE :
           m == I2C5 ? APB_I2C5_BASE :
           m == I2C6 ? APB_I2C6_BASE :
           m == I2C7 ? APB_I2C7_BASE :
           m == I2C8 ? APB_I2C8_BASE :
           m == I2C9 ? APB_I2C9_BASE :
           m == I2C10 ? APB_I2C10_BASE :
           m == I2C11 ? APB_I2C11_BASE :
           m == I2C12 ? APB_I2C12_BASE :
           m == I2C13 ? APB_I2C13_BASE :
           m == I2C14 ? APB_I2C14_BASE :
           m == I2C15 ? APB_I2C15_BASE :
           m == I2C16 ? APB_I2C16_BASE :
           m == DMA1 ? DMA1_BASE :
           m == DMA2 ? DMA2_BASE :
           m == DMA3 ? DMA3_BASE :
           m == DMA4 ? DMA4_BASE :
           m == DMA5 ? DMA5_BASE :
           m == DMA6 ? DMA6_BASE :
           m == DMA7 ? DMA7_BASE :
           m == DMA8 ? DMA8_BASE :
           m == MAILBOX ? MB_REG_BASE :
           m == OSPI1 ? APB_OSPI1_BASE :
           m == OSPI2 ? APB_OSPI2_BASE :
           m == FUSECTRL ? APB_EFUSEC_BASE :
           m == WDOG1 ? APB_WDT1_BASE :
           m == WDOG2 ? APB_WDT2_BASE :
           m == WDOG3 ? APB_WDT3_BASE :
           m == WDOG4 ? APB_WDT4_BASE :
           m == WDOG5 ? APB_WDT5_BASE :
           m == WDOG6 ? APB_WDT6_BASE :
           m == WDOG7 ? APB_WDT7_BASE :
           m == WDOG8 ? APB_WDT8_BASE :
           m == USB_CTRL1 ? APB_USB1_BASE :
#if defined(TGT_ap)
           m == CE2 ? APB_CE2_VCE1_BASE :
#else
           m == CE2 ? CE2_REG_BASE :
           m == CE1 ? APB_CE1_REG_BASE :
#endif
           0ul;
}

U32 fuse_get_cores(void)
{
    uint32_t n = 0;
    uint32_t v = (fuse_read(0x2b / 4) >> 25) & 0x07u;

    if (v >= 6) {
        v = 6;
    }

    n = 6 - v;

    if (0 == n) {
        if (!(fuse_read(0x2c / 4) & (0x01u << 3))) {
            n = 1 | (0x01u << 8);
        }
    }

    return n;
}

void soc_get_rand(uint8_t *rng, size_t sz)
{
    hrng_get_rnd(rng, (int32_t)sz);
}
