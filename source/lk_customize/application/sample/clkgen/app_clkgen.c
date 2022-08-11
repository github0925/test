/*
* app_clkgen.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement the app to auto jump to kernel.
*
* Revision History:
* -----------------
*/
#include <app.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <platform.h>
#include <lib/console.h>
#include <arch.h>
#include "clkgen_hal.h"
#include "chip_res.h"
#include "ckgen_cfg.h"

/****************globale res manage id to clkgen res index****************/
/*fsrefclk global resource index*/
const uint32_t g_RefclkResGlbResIdx[] = {
    RES_SCR_L16_SAF_FSREFCLK_SAF,
    RES_SCR_L16_SEC_FSREFCLK_SEC,
    RES_SCR_L16_SEC_FSREFCLK_HPI,
    RES_SCR_L16_SEC_FSREFCLK_CPU1,
    RES_SCR_L16_SEC_FSREFCLK_CPU2,
    RES_SCR_L16_SEC_FSREFCLK_GPU1,
    RES_SCR_L16_SEC_FSREFCLK_GPU2,
    RES_SCR_L16_SEC_FSREFCLK_HIS,
    RES_SCR_L16_SEC_FSREFCLK_VPU,
    RES_SCR_L16_SEC_FSREFCLK_VSN,
    RES_SCR_L16_SEC_FSREFCLK_DISP,
    RES_SCR_L16_SEC_FSREFCLK_DDR_SS,
};

/*safety ip bus slice and gating global resource index*/
const uint32_t g_SafIpSliceGlbResIdx[] = {
    RES_IP_SLICE_SAF_CE1,
    RES_IP_SLICE_SAF_I2C_SAF,
    RES_IP_SLICE_SAF_SPI_SAF,
    RES_IP_SLICE_SAF_UART_SAF,
    RES_IP_SLICE_SAF_I2S_MCLK1,
    RES_IP_SLICE_SAF_I2S_SC1,
    RES_IP_SLICE_SAF_I2S_SC2,
    RES_IP_SLICE_SAF_ENET1_TX,
    RES_IP_SLICE_SAF_ENET1_RMII,
    RES_IP_SLICE_SAF_ENET1_PHY_REF,
    RES_IP_SLICE_SAF_ENET1_TIMER_SEC,
    RES_IP_SLICE_SAF_OSPI1,
    RES_IP_SLICE_SAF_TIMER1,
    RES_IP_SLICE_SAF_TIMER2,
    RES_IP_SLICE_SAF_PWM1,
    RES_IP_SLICE_SAF_PWM2,
    RES_IP_SLICE_SAF_CAN_1_TO_4,
};

const uint32_t g_SafBusSliceGlbResIdx[] = {
    RES_BUS_SLICE_SAF_SAF_PLAT_CTL,
};

const uint32_t g_SafGatingGlbResIdx[] = {
    RES_GATING_EN_SAF_SAF_PLAT,
    RES_GATING_EN_SAF_SAF_PLAT_NOC,
    RES_GATING_EN_SAF_NOC_MAINB,
    RES_GATING_EN_SAF_MAC_RDC,
    RES_GATING_EN_SAF_RTC_PCLK,
    RES_GATING_EN_SAF_GIC1,
    RES_GATING_EN_SAF_DMA1,
    RES_GATING_EN_SAF_ENTE1_TX,
    RES_GATING_EN_SAF_ENET1_PHY_REF,
    RES_GATING_EN_SAF_ENET1_TIMER_SEC,
    RES_GATING_EN_SAF_OSPI1,
    RES_GATING_EN_SAF_IRAM1,
    RES_GATING_EN_SAF_ROMC1,
    RES_GATING_EN_SAF_EFUSEC,
    RES_GATING_EN_SAF_CE1,
    RES_GATING_EN_SAF_I2C1,
    RES_GATING_EN_SAF_I2C2,
    RES_GATING_EN_SAF_I2C3,
    RES_GATING_EN_SAF_I2C4,
    RES_GATING_EN_SAF_SPI1,
    RES_GATING_EN_SAF_SPI2,
    RES_GATING_EN_SAF_SPI3,
    RES_GATING_EN_SAF_SPI4,
    RES_GATING_EN_SAF_UART1,
    RES_GATING_EN_SAF_UART2,
    RES_GATING_EN_SAF_UART3,
    RES_GATING_EN_SAF_UART4,
    RES_GATING_EN_SAF_UART5,
    RES_GATING_EN_SAF_UART6,
    RES_GATING_EN_SAF_UART7,
    RES_GATING_EN_SAF_UART8,
    RES_GATING_EN_SAF_I2S_SC1,
    RES_GATING_EN_SAF_I2S_SC2,
    RES_GATING_EN_SAF_TBU11,
    RES_GATING_EN_SAF_TBU12,
    RES_GATING_EN_SAF_TBU13,
    RES_GATING_EN_SAF_TIMER1,
    RES_GATING_EN_SAF_TIMER2,
    RES_GATING_EN_SAF_PWM1,
    RES_GATING_EN_SAF_PWM2,
    RES_GATING_EN_SAF_CAN1,
    RES_GATING_EN_SAF_CAN2,
    RES_GATING_EN_SAF_CAN3,
    RES_GATING_EN_SAF_CAN4,
    RES_GATING_EN_SAF_GPIO1,
    RES_GATING_EN_SAF_SCR_SAF,
    RES_GATING_EN_SAF_PVT_SENS_SAF,
    RES_GATING_EN_SAF_PLL1,
    RES_GATING_EN_SAF_PLL2,
    RES_GATING_EN_SAF_CKGEN_SAF,
    RES_GATING_EN_SAF_RSTGEN_SAF,
    RES_GATING_EN_SAF_SEM1,
    RES_GATING_EN_SAF_SEM2,
    RES_GATING_EN_SAF_EIC_SAF,
    RES_GATING_EN_SAF_RPC_SAF,
    RES_GATING_EN_SAF_XTAL_SAF,
    RES_GATING_EN_SAF_BIPC_ENET1,
};

/*display ip bus slice and gating global resource index*/
const uint32_t g_DispIpSliceGlbResIdx[] = {
    RES_IP_SLICE_DISP_MIPI_CSI1_PIX,
    RES_IP_SLICE_DISP_MIPI_CSI2_PIX,
    RES_IP_SLICE_DISP_MIPI_CSI3_PIX,
    RES_IP_SLICE_DISP_DP1,
    RES_IP_SLICE_DISP_DP2,
    RES_IP_SLICE_DISP_DP3,
    RES_IP_SLICE_DISP_DC5,
    RES_IP_SLICE_DISP_DC1,
    RES_IP_SLICE_DISP_DC2,
    RES_IP_SLICE_DISP_DC3,
    RES_IP_SLICE_DISP_DC4,
    RES_IP_SLICE_DISP_SPARE1,
    RES_IP_SLICE_DISP_SPARE2,
    RES_IP_SLICE_DISP_EXT_AUD1,
    RES_IP_SLICE_DISP_EXT_AUD2,
    RES_IP_SLICE_DISP_EXT_AUD3,
    RES_IP_SLICE_DISP_EXT_AUD4,
};

const uint32_t g_DispBusSliceGlbResIdx[] = {
    RES_BUS_SLICE_DISP_DISP_BUS_CTL,
};

const uint32_t g_DispGatingGlbResIdx[] = {
    RES_GATING_EN_DISP_DISP_BUS,
    RES_GATING_EN_DISP_LVDS_CLK_1,
    RES_GATING_EN_DISP_LVDS_CLK_2,
    RES_GATING_EN_DISP_IRAM5,
    RES_GATING_EN_DISP_XTAL_AP,
    RES_GATING_EN_DISP_PLL_LVDS1,
    RES_GATING_EN_DISP_PLL_LVDS2,
    RES_GATING_EN_DISP_PLL_LVDS3,
    RES_GATING_EN_DISP_PLL_LVDS4,
    RES_GATING_EN_DISP_CKGEN_DISP,
    RES_GATING_EN_DISP_DC1,
    RES_GATING_EN_DISP_DC2,
    RES_GATING_EN_DISP_DC3,
    RES_GATING_EN_DISP_DC4,
    RES_GATING_EN_DISP_DC5,
    RES_GATING_EN_DISP_DP1,
    RES_GATING_EN_DISP_DP2,
    RES_GATING_EN_DISP_DP3,
    RES_GATING_EN_DISP_SPARE1,
    RES_GATING_EN_DISP_SPARE2,
    RES_GATING_EN_DISP_MIPI_DSI1,
    RES_GATING_EN_DISP_MIPI_DSI2,
    RES_GATING_EN_DISP_LVDS_SS,
    RES_GATING_EN_DISP_MIPI_CSI1_PIX_CSI,
    RES_GATING_EN_DISP_MIPI_CSI2_PIX_CSI,
    RES_GATING_EN_DISP_MIPI_CSI3_PIX_SCI,
    RES_GATING_EN_DISP_MIPI_CSI1_PIX,
    RES_GATING_EN_DISP_MIPI_CSI2_PIX,
    RES_GATING_EN_DISP_MIPI_CSI3_PIX,
};

/*secure ip bus core slice and gating global resource index*/
const uint32_t g_SecIpSliceGlbResIdx[] = {
    RES_IP_SLICE_SEC_CE2,
    RES_IP_SLICE_SEC_AUD_DSP1,
    RES_IP_SLICE_SEC_I2C_SEC0,
    RES_IP_SLICE_SEC_I2C_SEC1,
    RES_IP_SLICE_SEC_SPI_SEC0,
    RES_IP_SLICE_SEC_SPI_SEC1,
    RES_IP_SLICE_SEC_UART_SEC0,
    RES_IP_SLICE_SEC_UART_SEC1,
    RES_IP_SLICE_SEC_EMMC1,
    RES_IP_SLICE_SEC_EMMC2,
    RES_IP_SLICE_SEC_EMMC3,
    RES_IP_SLICE_SEC_EMMC4,
    RES_IP_SLICE_SEC_ENET2_TX,
    RES_IP_SLICE_SEC_ENET2_RMII,
    RES_IP_SLICE_SEC_ENET2_PHY_REF,
    RES_IP_SLICE_SEC_ENET2_TIMER_SEC,
    RES_IP_SLICE_SEC_SPDIF1,
    RES_IP_SLICE_SEC_SPDIF2,
    RES_IP_SLICE_SEC_SPDIF3,
    RES_IP_SLICE_SEC_SPDIF4,
    RES_IP_SLICE_SEC_OSPI2,
    RES_IP_SLICE_SEC_TIMER3,
    RES_IP_SLICE_SEC_TIMER4,
    RES_IP_SLICE_SEC_TIMER5,
    RES_IP_SLICE_SEC_TIMER6,
    RES_IP_SLICE_SEC_TIMER7,
    RES_IP_SLICE_SEC_TIMER8,
    RES_IP_SLICE_SEC_PWM3,
    RES_IP_SLICE_SEC_PWM4,
    RES_IP_SLICE_SEC_PWM5,
    RES_IP_SLICE_SEC_PWM6,
    RES_IP_SLICE_SEC_PWM7,
    RES_IP_SLICE_SEC_PWM8,
    RES_IP_SLICE_SEC_I2S_MCLK2,
    RES_IP_SLICE_SEC_I2S_MCLK3,
    RES_IP_SLICE_SEC_I2S_MC1,
    RES_IP_SLICE_SEC_I2S_MC2,
    RES_IP_SLICE_SEC_I2S_SC3,
    RES_IP_SLICE_SEC_I2S_SC4,
    RES_IP_SLICE_SEC_I2S_SC5,
    RES_IP_SLICE_SEC_I2S_SC6,
    RES_IP_SLICE_SEC_I2S_SC7,
    RES_IP_SLICE_SEC_I2S_SC8,
    RES_IP_SLICE_SEC_CSI_MCLK1,
    RES_IP_SLICE_SEC_CSI_MCLK2,
    RES_IP_SLICE_SEC_GIC4_GIC5,
    RES_IP_SLICE_SEC_CAN5_CAN20,
    RES_IP_SLICE_SEC_TRACE,
    RES_IP_SLICE_SEC_SYS_CNT,
    RES_IP_SLICE_SEC_MSHC_TIMER,
    RES_IP_SLICE_SEC_HPI_CLK600,
    RES_IP_SLICE_SEC_HPI_CLK800,
};

const uint32_t g_SecBusSliceGlbResIdx[] = {
    RES_BUS_SLICE_SEC_SEC_PLAT_CTL,
};

const uint32_t g_SecCoreSliceGlbResIdx[] = {
    RES_CORE_SLICE_SEC_MP_PLAT,
};

const uint32_t g_SecGatingGlbResIdx[] = {
    RES_GATING_EN_SEC_SEC_PLAT,
    RES_GATING_EN_SEC_MP_PLAT,
    RES_GATING_EN_SEC_AUD_DSP1,
    RES_GATING_EN_SEC_CE2,
    RES_GATING_EN_SEC_GIC2,
    RES_GATING_EN_SEC_GIC3,
    RES_GATING_EN_SEC_EMMC1,
    RES_GATING_EN_SEC_MSHC_TIMER,
    RES_GATING_EN_SEC_EMMC2,
    RES_GATING_EN_SEC_EMMC3,
    RES_GATING_EN_SEC_EMMC4,
    RES_GATING_EN_SEC_ENET2_TX,
    RES_GATING_EN_SEC_ENET2_TIMER_SEC,
    RES_GATING_EN_SEC_OSPI2,
    RES_GATING_EN_SEC_MB,
    RES_GATING_EN_SEC_IRAM2,
    RES_GATING_EN_SEC_IRAM3,
    RES_GATING_EN_SEC_IRAM4,
    RES_GATING_EN_SEC_ROMC2,
    RES_GATING_EN_SEC_I2C5,
    RES_GATING_EN_SEC_I2C6,
    RES_GATING_EN_SEC_I2C7,
    RES_GATING_EN_SEC_I2C8,
    RES_GATING_EN_SEC_I2C9,
    RES_GATING_EN_SEC_I2C10,
    RES_GATING_EN_SEC_I2C11,
    RES_GATING_EN_SEC_I2C12,
    RES_GATING_EN_SEC_I2C13,
    RES_GATING_EN_SEC_I2C14,
    RES_GATING_EN_SEC_I2C15,
    RES_GATING_EN_SEC_I2C16,
    RES_GATING_EN_SEC_SPI5,
    RES_GATING_EN_SEC_SPI6,
    RES_GATING_EN_SEC_SPI7,
    RES_GATING_EN_SEC_SPI8,
    RES_GATING_EN_SEC_UART9,
    RES_GATING_EN_SEC_UART10,
    RES_GATING_EN_SEC_UART11,
    RES_GATING_EN_SEC_UART12,
    RES_GATING_EN_SEC_UART13,
    RES_GATING_EN_SEC_UART14,
    RES_GATING_EN_SEC_SPDIF1,
    RES_GATING_EN_SEC_SPDIF2,
    RES_GATING_EN_SEC_SPDIF3,
    RES_GATING_EN_SEC_SPDIF4,
    RES_GATING_EN_SEC_TIMER3,
    RES_GATING_EN_SEC_TIMER4,
    RES_GATING_EN_SEC_TIMER5,
    RES_GATING_EN_SEC_TIMER6,
    RES_GATING_EN_SEC_TIMER7,
    RES_GATING_EN_SEC_TIMER8,
    RES_GATING_EN_SEC_PWM3,
    RES_GATING_EN_SEC_PWM4,
    RES_GATING_EN_SEC_PWM5,
    RES_GATING_EN_SEC_PWM6,
    RES_GATING_EN_SEC_PWM7,
    RES_GATING_EN_SEC_PWM8,
    RES_GATING_EN_SEC_I2S_MC1,
    RES_GATING_EN_SEC_I2S_MC2,
    RES_GATING_EN_SEC_I2S_SC3,
    RES_GATING_EN_SEC_I2S_SC4,
    RES_GATING_EN_SEC_I2S_SC5,
    RES_GATING_EN_SEC_I2S_SC6,
    RES_GATING_EN_SEC_I2S_SC7,
    RES_GATING_EN_SEC_I2S_SC8,
    RES_GATING_EN_SEC_CANFD5,
    RES_GATING_EN_SEC_CANFD6,
    RES_GATING_EN_SEC_CANFD7,
    RES_GATING_EN_SEC_CANFD8,
    RES_GATING_EN_SEC_TRACE,
    RES_GATING_EN_SEC_SYS_CNT,
    RES_GATING_EN_SEC_TBU14,
    RES_GATING_EN_SEC_TBU15,
    RES_GATING_EN_SEC_GPIO2,
    RES_GATING_EN_SEC_GPIO3,
    RES_GATING_EN_SEC_GPIO4,
    RES_GATING_EN_SEC_GPIO5,
    RES_GATING_EN_SEC_WDT3,
    RES_GATING_EN_SEC_WDT4,
    RES_GATING_EN_SEC_WDT5,
    RES_GATING_EN_SEC_WDT6,
    RES_GATING_EN_SEC_WDT7,
    RES_GATING_EN_SEC_WDT8,
    RES_GATING_EN_SEC_PLL3,
    RES_GATING_EN_SEC_PLL4,
    RES_GATING_EN_SEC_PLL5,
    RES_GATING_EN_SEC_PLL6,
    RES_GATING_EN_SEC_PLL7,
    RES_GATING_EN_SEC_CANFD9,
    RES_GATING_EN_SEC_CANFD10,
    RES_GATING_EN_SEC_CANFD11,
    RES_GATING_EN_SEC_CANFD12,
    RES_GATING_EN_SEC_CANFD15,
    RES_GATING_EN_SEC_CANFD16,
    RES_GATING_EN_SEC_CANFD17,
    RES_GATING_EN_SEC_CANFD18,
    RES_GATING_EN_SEC_CANFD19,
    RES_GATING_EN_SEC_CANFD20,
};

/*soc ip slice and gating global resource index*/
const uint32_t g_SocIpSliceGlbResIdx[] = {
    RES_IP_SLICE_SOC_VPU1,
    RES_IP_SLICE_SOC_MJPEG,
};

const uint32_t g_SocBusSliceGlbResIdx[] = {
    RES_BUS_SLICE_SOC_VPU_BUS_CTL,
    RES_BUS_SLICE_SOC_VSN_BUS_CTL,
    RES_BUS_SLICE_SOC_NOC_BUS_CLOCK_CTL,
    RES_BUS_SLICE_SOC_HIS_BUS_CTL,
};

const uint32_t g_SocCoreSliceGlbResIdx[] = {
    RES_CORE_SLICE_SOC_CPU1A,
    RES_CORE_SLICE_SOC_CPU1B,
    RES_CORE_SLICE_SOC_CPU2,
    RES_CORE_SLICE_SOC_GPU1,
    RES_CORE_SLICE_SOC_GPU2,
    RES_CORE_SLICE_SOC_DDR,
};

/*uuu wrapper global resource index*/
const uint32_t g_UuuSliceGlbResIdx[]= {
    RES_UUU_WRAP_SOC_CPU1A,
    RES_UUU_WRAP_SOC_CPU1B,
    RES_UUU_WRAP_SOC_CPU2,
    RES_UUU_WRAP_SOC_GPU1,
    RES_UUU_WRAP_SOC_GPU2,
    RES_UUU_WRAP_SOC_VPU1,
    RES_UUU_WRAP_SOC_MJPEG,
    RES_UUU_WRAP_SOC_VPU_BUS,
    RES_UUU_WRAP_SOC_VSN,
    RES_UUU_WRAP_SOC_DDR,
    RES_UUU_WRAP_SOC_HIS_BUS,
};

const uint32_t g_SocGatingGlbResIdx[] = {
    RES_GATING_EN_SOC_TCU,
    RES_GATING_EN_SOC_TBU0,
    RES_GATING_EN_SOC_TBU1,
    RES_GATING_EN_SOC_TBU2,
    RES_GATING_EN_SOC_TBU3,
    RES_GATING_EN_SOC_TBU4,
    RES_GATING_EN_SOC_TBU5,
    RES_GATING_EN_SOC_TBU6,
    RES_GATING_EN_SOC_TBU8,
    RES_GATING_EN_SOC_TBU9,
    RES_GATING_EN_SOC_TBU10,
    RES_GATING_EN_SOC_SCR4K_SID,
    RES_GATING_EN_SOC_SCR4K_SSID,
    RES_GATING_EN_SOC_BIPC_DDR,
    RES_GATING_EN_SOC_EIC_HPI,
    RES_GATING_EN_SOC_GIC4_GIC5,
    RES_GATING_EN_SOC_GIC4,
    RES_GATING_EN_SOC_GIC5,
    RES_GATING_EN_SOC_CPU1A_2_PLL_CPU1A_PLL_CPU1B_PCLK,
    RES_GATING_EN_SOC_CPU2_0,
    RES_GATING_EN_SOC_CPU2_1,
    RES_GATING_EN_SOC_CPU2_PCLK_ATCLK_GICCLK,
    RES_GATING_EN_SOC_CPU2_PLL_CPU2_PCLK,
    RES_GATING_EN_SOC_GPU1_0,
    RES_GATING_EN_SOC_GPU1_1,
    RES_GATING_EN_SOC_GPU1_2,
    RES_GATING_EN_SOC_GPU2_0,
    RES_GATING_EN_SOC_GPU2_1,
    RES_GATING_EN_SOC_GPU2_2,
    RES_GATING_EN_SOC_VPU1,
    RES_GATING_EN_SOC_VPU_BUS_0_VPU1_ACLK,
    RES_GATING_EN_SOC_VPU_BUS_1_VPU1_PCLK,
    RES_GATING_EN_SOC_VPU_BUS_0_VPU2_ACLK,
    RES_GATING_EN_SOC_VPU_BUS_1_VPU2_PCLK,
    RES_GATING_EN_SOC_MJPEG,
    RES_GATING_EN_SOC_VPU_BUS_0_MJPEG_ACLK,
    RES_GATING_EN_SOC_VPU_BUS_1_MJPEG_PCLK,
    RES_GATING_EN_SOC_VPU_BUS_1_PLL_VPU,
    RES_GATING_EN_SOC_VSN_BUS_0_VDSP_CLK,
    RES_GATING_EN_SOC_VSN_BUS_1_NOC_VSP,
    RES_GATING_EN_SOC_VSN_BUS_1_PLL_VSN,
    RES_GATING_EN_SOC_VSN_BUS_0_BIPC_VDSP_ACLK,
    RES_GATING_EN_SOC_VSN_BUS_1_BIPC_VDSP_PCLK,
    RES_GATING_EN_SOC_VSN_BUS_1_EIC_VSN,
    RES_GATING_EN_SOC_HIS_BUS_2_NOC_HIS_MAINCLK,
    RES_GATING_EN_SOC_HIS_BUS_3_NOC_HIS_PERCLK,
    RES_GATING_EN_SOC_HIS_BUS_2_PCIE2_MSTR_ACLK,
    RES_GATING_EN_SOC_HIS_BUS_3_PCIE2_PLK,
    RES_GATING_EN_SOC_HIS_BUS_2_PCIE1_MSTR_ACLK,
    RES_GATING_EN_SOC_HIS_BUS_3_PCIE1_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_3_PCIE_PHY_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_1,
    RES_GATING_EN_SOC_HIS_BUS_2_USB1_XM_ACLK,
    RES_GATING_EN_SOC_HIS_BUS_3_USB1_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_3_USB1_CTRL_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_3_USB1_PHY_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_3_USB2_CTRL_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_3_USB2_PHY_PCLK,
    RES_GATING_EN_SOC_HIS_BUS_2_USB2_XM_ACLK,
    RES_GATING_EN_SOC_HIS_BUS_3_USB2_PCLK,
    RES_GATING_EN_SOC_CPU1B_0,
    RES_GATING_EN_SOC_CPU1A_2_PCLK_ATCLK_GICCLK,
    RES_GATING_EN_SOC_CPU1A_1,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK0,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK1,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK2,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK3,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK4,
    RES_GATING_EN_SOC_CPU1A_0_CORE_CLK5,
};

#if SAF_SYSTEM_CFG
static int cmd_clkgen(int argc, const cmd_args *argv)
{

    bool ret = true;
    uint32_t get_clk = 0;
    uint16_t glb_res_idx_size = 0;
    static void *g_saf_handle;
    hal_saf_clock_set_default();
    ret = hal_clock_creat_handle(&g_saf_handle);
    if (!ret) {
        printf("cmd_clkgen creat handle failed\n");
        return -1;
    }

    /*init start*/
    glb_res_idx_size = sizeof(g_RefclkResGlbResIdx)/sizeof(g_RefclkResGlbResIdx[0]);
    for (uint8_t i = 0; i< glb_res_idx_size; i++) {
        ret = hal_clock_osc_init(g_saf_handle,g_RefclkResGlbResIdx[i],xtal_saf_24M,true);
    }
#if 0
    glb_res_idx_size = sizeof(g_SafIpSliceGlbResIdx)/sizeof(g_SafIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i< glb_res_idx_size; i++) {
        ret = hal_clock_ip_init(g_saf_handle,g_SafIpSliceGlbResIdx[i],g_saf_ip_slice,glb_res_idx_size);
    }

    glb_res_idx_size = sizeof(g_SafBusSliceGlbResIdx)/sizeof(g_SafBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i< glb_res_idx_size; i++) {
        ret = hal_clock_bus_init(g_saf_handle,g_SafBusSliceGlbResIdx[i],g_saf_bus_slice,glb_res_idx_size);
    }
#endif
    /*set clock frequency*/
    clkgen_app_ip_cfg_t ip_cfg;
    ip_cfg.clk_src_select_num = 4;//pll5.root--796MHz
    ip_cfg.pre_div = 1;
    ip_cfg.post_div = 0;//sel_clk = (398*CLK_MHZ),
    ret = hal_clock_ipclk_set(g_saf_handle,RES_IP_SLICE_SAF_CE1,&ip_cfg);

    clkgen_app_bus_cfg_t bus_app_cfg;
    bus_app_cfg.clk_src_select_a_num = 5,//default--796MHz
    bus_app_cfg.clk_src_select_b_num = 5,//default--796MHz
    bus_app_cfg.clk_a_b_select = 0;
    bus_app_cfg.pre_div_a = 0;
    bus_app_cfg.pre_div_b = 0;
    bus_app_cfg.post_div = 0;
    bus_app_cfg.m_div = 0;
    bus_app_cfg.n_div = 0;
    bus_app_cfg.p_div = 0;
    bus_app_cfg.q_div = 0;
    ret = hal_clock_busclk_set(g_saf_handle,RES_BUS_SLICE_SAF_SAF_PLAT_CTL,bus_app_cfg);
    /*enable clock*/
    /*enable ce1 clock*/
    ret = hal_clock_enable(g_saf_handle,RES_GATING_EN_SAF_CE1);
    /*enable safety platform bus clock*/
    ret = hal_clock_enable(g_saf_handle,RES_GATING_EN_SAF_SAF_PLAT);

    hal_clock_release_handle(g_saf_handle);
    return 0;
}
#elif SEC_SYSTEM_CFG
static int cmd_clkgen(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint32_t get_clk = 0;
    uint16_t glb_res_idx_size = 0;
    static void *g_sec_handle;
    hal_sec_clock_set_default();
    hal_soc_clock_set_default();
    hal_disp_clock_set_default();
    ret = hal_clock_creat_handle(&g_sec_handle);
    if (!ret) {
        printf("cmd_clkgen creat handle failed\n");
        return -1;
    }

    /*init start*/
    glb_res_idx_size = sizeof(g_RefclkResGlbResIdx)/sizeof(g_RefclkResGlbResIdx[0]);
    for (uint8_t i = 0; i< glb_res_idx_size; i++) {
        ret = hal_clock_osc_init(g_sec_handle,g_RefclkResGlbResIdx[i],rc_24M,true);
    }
#if 0
    glb_res_idx_size = sizeof(g_SecIpSliceGlbResIdx)/sizeof(g_SecIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i< glb_res_idx_size; i++) {
        ret = hal_clock_ip_init(g_sec_handle,g_SecIpSliceGlbResIdx[i],g_sec_ip_slice,glb_res_idx_size);
    }

    glb_res_idx_size = sizeof(g_SecBusSliceGlbResIdx)/sizeof(g_SecBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i< glb_res_idx_size; i++) {
        ret = hal_clock_bus_init(g_sec_handle,g_SecBusSliceGlbResIdx[i],g_sec_bus_slice,glb_res_idx_size);
    }

    glb_res_idx_size = sizeof(g_SecCoreSliceGlbResIdx)/sizeof(g_SecCoreSliceGlbResIdx[0]);
    for (uint8_t i = 0; i< glb_res_idx_size; i++) {
        ret = hal_clock_core_init(g_sec_handle,g_SecCoreSliceGlbResIdx[i],g_sec_core_slice,glb_res_idx_size);
    }
#endif
    /*set clock frequency*/
    clkgen_app_ip_cfg_t ip_cfg;
    ip_cfg.clk_src_select_num = 4;//pll5.root--796MHz
    ip_cfg.pre_div = 1;
    ip_cfg.post_div = 0;//sel_clk = (398*CLK_MHZ),
    ret = hal_clock_ipclk_set(g_sec_handle,RES_IP_SLICE_SEC_CE2,&ip_cfg);

    clkgen_app_bus_cfg_t bus_app_cfg;
    bus_app_cfg.clk_src_select_a_num = 5,//default--796MHz
    bus_app_cfg.clk_src_select_b_num = 5,//default--796MHz
    bus_app_cfg.clk_a_b_select = 0;
    bus_app_cfg.pre_div_a = 0;
    bus_app_cfg.pre_div_b = 0;
    bus_app_cfg.post_div = 0;
    bus_app_cfg.m_div = 0;
    bus_app_cfg.n_div = 0;
    bus_app_cfg.p_div = 0;
    bus_app_cfg.q_div = 0;
    ret = hal_clock_busclk_set(g_sec_handle,RES_BUS_SLICE_SEC_SEC_PLAT_CTL,bus_app_cfg);

    clkgen_app_core_cfg_t core_app_cfg;
    core_app_cfg.clk_src_select_a_num = 5;//default--796MHz
    core_app_cfg.clk_src_select_b_num = 5;//default--796MHz
    core_app_cfg.clk_a_b_select = 0;
    core_app_cfg.post_div = 0;
    ret = hal_clock_core_init(g_sec_handle,RES_CORE_SLICE_SEC_MP_PLAT,&core_app_cfg);
    /*enable clock*/
    /*enable ce1 clock*/
    ret = hal_clock_enable(g_sec_handle,RES_GATING_EN_SEC_CE2);
    /*enable secure platform bus clock*/
    ret = hal_clock_enable(g_sec_handle,RES_GATING_EN_SEC_SEC_PLAT);
    /*enable secure platform core clock*/
    ret = hal_clock_enable(g_sec_handle,RES_GATING_EN_SEC_MP_PLAT);
    hal_clock_release_handle(g_sec_handle);
    return 0;
}
#else
static int cmd_clkgen(int argc, const cmd_args *argv)
{
    bool ret = true;
    uint32_t get_clk = 0;
    uint16_t glb_res_idx_size = 0;
    static void *g_soc_handle;
    //all the default clkgen initialization move to sec
    hal_sec_clock_set_default();
    hal_soc_clock_set_default();
    hal_disp_clock_set_default();
    ret = hal_clock_creat_handle(&g_soc_handle);
    if (!ret) {
        printf("cmd_clkgen creat handle failed\n");
        return -1;
    }

    /*init start*/
    glb_res_idx_size = sizeof(g_RefclkResGlbResIdx)/sizeof(g_RefclkResGlbResIdx[0]);
    for (uint8_t i = 0; i< glb_res_idx_size; i++) {
        ret = hal_clock_osc_init(g_soc_handle,g_RefclkResGlbResIdx[i],rc_24M,true);
    }
#if 0
    glb_res_idx_size = sizeof(g_SocIpSliceGlbResIdx)/sizeof(g_SocIpSliceGlbResIdx[0]);
    for (uint8_t i = 0; i< glb_res_idx_size; i++) {
        ret = hal_clock_ip_init(g_soc_handle,g_SocIpSliceGlbResIdx[i],g_soc_ip_slice, soc_ip_slice_max);
    }

    glb_res_idx_size = sizeof(g_SocBusSliceGlbResIdx)/sizeof(g_SocBusSliceGlbResIdx[0]);
    for (uint8_t i = 0; i< glb_res_idx_size; i++) {
        ret = hal_clock_bus_init(g_soc_handle,g_SocBusSliceGlbResIdx[i],g_soc_bus_slice,soc_bus_slice_max);
    }

    glb_res_idx_size = sizeof(g_SocCoreSliceGlbResIdx)/sizeof(g_SocCoreSliceGlbResIdx[0]);
    for (uint8_t i = 0; i< glb_res_idx_size; i++) {
        ret = hal_clock_core_init(g_soc_handle,g_SocCoreSliceGlbResIdx[i],g_soc_core_slice,soc_core_slice_max);
    }

    glb_res_idx_size = sizeof(g_UuuSliceGlbResIdx)/sizeof(g_UuuSliceGlbResIdx[0]);
    for (uint8_t i = 0; i< glb_res_idx_size; i++) {
        ret = hal_clock_uuu_init(g_soc_handle,g_UuuSliceGlbResIdx[i],g_uuu_wrapper,uuu_clock_wrapper_idx_max);
    }
#endif
    /*set clock frequency*/
    clkgen_app_ip_cfg_t ip_cfg;
    ip_cfg.clk_src_select_num = 4;//pll5.root--796MHz
    ip_cfg.pre_div = 1;
    ip_cfg.post_div = 0;//sel_clk = (398*CLK_MHZ),
    ret = hal_clock_ipclk_set(g_soc_handle,RES_IP_SLICE_SOC_VPU1,&ip_cfg);

    clkgen_app_bus_cfg_t bus_app_cfg;
    bus_app_cfg.clk_src_select_a_num = 5,//default--796MHz
    bus_app_cfg.clk_src_select_b_num = 5,//default--796MHz
    bus_app_cfg.clk_a_b_select = 0;
    bus_app_cfg.pre_div_a = 0;
    bus_app_cfg.pre_div_b = 0;
    bus_app_cfg.post_div = 0;
    bus_app_cfg.m_div = 0;
    bus_app_cfg.n_div = 0;
    bus_app_cfg.p_div = 0;
    bus_app_cfg.q_div = 0;
    ret = hal_clock_busclk_set(g_soc_handle,RES_BUS_SLICE_SOC_VPU_BUS_CTL,&bus_app_cfg);

    clkgen_app_core_cfg_t core_app_cfg;
    core_app_cfg.clk_src_select_a_num = 5;//default--796MHz
    core_app_cfg.clk_src_select_b_num = 5;//default--796MHz
    core_app_cfg.clk_a_b_select = 0;
    core_app_cfg.post_div = 0;
    ret = hal_clock_coreclk_set(g_soc_handle,RES_CORE_SLICE_SOC_CPU1A,&core_app_cfg);

    clkgen_app_uuu_cfg_t uuu_app_cfg;
    uuu_app_cfg.uuu_input_clk_sel = uuu_input_soc_clk;
    uuu_app_cfg.m_div =0;
    uuu_app_cfg.n_div = 0;
    uuu_app_cfg.p_div = 0;
    uuu_app_cfg.q_div = 0;
    uuu_app_cfg.low_power_mode_en = false;
    ret = hal_clock_uuuclk_set(g_soc_handle,RES_UUU_WRAP_SOC_CPU1A,&uuu_app_cfg);
    /*enable clock*/
    /*enable ce1 clock*/
    ret = hal_clock_enable(g_soc_handle,RES_GATING_EN_SOC_VPU1);
    /*enable secure platform bus clock*/
    ret = hal_clock_enable(g_soc_handle,RES_GATING_EN_SOC_VPU_BUS_0_VPU1_ACLK);
    ret = hal_clock_enable(g_soc_handle,RES_GATING_EN_SOC_VPU_BUS_1_VPU1_PCLK);
    ret = hal_clock_enable(g_soc_handle,RES_GATING_EN_SOC_VPU_BUS_0_VPU2_ACLK);
    ret = hal_clock_enable(g_soc_handle,RES_GATING_EN_SOC_VPU_BUS_1_VPU2_PCLK);
    /*enable secure platform core clock*/
    ret = hal_clock_enable(g_soc_handle,RES_GATING_EN_SOC_CPU1A_0_CORE_CLK0);
    ret = hal_clock_enable(g_soc_handle,RES_GATING_EN_SOC_CPU1A_0_CORE_CLK1);
    ret = hal_clock_enable(g_soc_handle,RES_GATING_EN_SOC_CPU1A_0_CORE_CLK2);
    ret = hal_clock_enable(g_soc_handle,RES_GATING_EN_SOC_CPU1A_0_CORE_CLK3);
    ret = hal_clock_enable(g_soc_handle,RES_GATING_EN_SOC_CPU1A_0_CORE_CLK4);
    ret = hal_clock_enable(g_soc_handle,RES_GATING_EN_SOC_CPU1A_0_CORE_CLK5);
    ret = hal_clock_enable(g_soc_handle,RES_GATING_EN_SOC_CPU1A_1);
    ret = hal_clock_enable(g_soc_handle,RES_GATING_EN_SOC_CPU1A_2_PCLK_ATCLK_GICCLK);
    hal_clock_release_handle(g_soc_handle);
    return 0;
}
#endif

static void clkgen_samplecode_entry(const struct app_descriptor *app, void *args)
{
    cmd_clkgen(0,0);
}

APP_START(clkgen_samplecode)
.flags = 0,
//.entry=clkgen_samplecode_entry,
APP_END
