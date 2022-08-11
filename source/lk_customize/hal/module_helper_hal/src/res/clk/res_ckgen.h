#include <clkgen_hal.h>

#if MODULE_HELPER_CKGEN_SEC
/*CKGEN SEC*/
static int SEC_MP_SPARE0_1PLAT_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL4_ROOT, CLK_ID_PLL5_ROOT, CLK_ID_PLL3_ROOT, CLK_ID_PLL3_DIVA
};
static int CLK_ID_CE2_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL5_ROOT, CLK_ID_PLL4_ROOT, CLK_ID_PLL3_ROOT, CLK_ID_PLL3_DIVA
};
static int SEC_I2C_SPI_UART_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL3_DIVC, CLK_ID_PLL3_DIVD, CLK_ID_PLL5_DIVD, CLK_ID_PLL3_DIVB
};
static int SEC_EMMC1234_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL5_DIVA, CLK_ID_PLL4_DIVB, CLK_ID_PLL5_DIVC, CLK_ID_PLL3_DIVA
};
static int SEC_ENET2_TX_RMII_PHY_REF_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL4_DIVC
};
static int CLK_ID_ENET2_TIMER_SEC_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL4_DIVA, CLK_ID_PLL5_ROOT, CLK_ID_PLL5_DIVA, CLK_ID_PLL3_DIVA
};

static int SEC_SPDIF1234_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL6_ROOT, CLK_ID_PLL7_ROOT, CLK_ID_PLL5_DIVB, CLK_ID_PLL3_DIVD
};
static int CLK_ID_OSPI2_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC, CLK_ID_PLL4_DIVB,
    CLK_ID_PLL5_DIVB, CLK_ID_PLL5_DIVC, CLK_ID_PLL5_DIVA
};
static int SEC_TIMER3TO8_PWM3TO8_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC, CLK_ID_PLL5_DIVA,
    CLK_ID_PLL7_DIVC, CLK_ID_PLL3_DIVA, CLK_ID_PLL6_DIVD
};
static int SEC_I2S_MCLK2_MC1_SC3_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_EXT_AUD1, CLK_ID_PLL6_ROOT,
    CLK_ID_PLL7_ROOT, CLK_ID_PLL5_DIVA, CLK_ID_PLL3_DIVA
};
static int SEC_I2S_MCLK3_MC2_SC5_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_EXT_AUD3, CLK_ID_PLL6_ROOT,
    CLK_ID_PLL7_ROOT, CLK_ID_PLL5_DIVA, CLK_ID_PLL3_DIVA
};
static int SEC_I2S_SC4_SC7_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_EXT_AUD2, CLK_ID_PLL6_ROOT,
    CLK_ID_PLL7_ROOT, CLK_ID_PLL5_DIVA, CLK_ID_PLL3_DIVA
};
static int SEC_I2S_SC6_SC8_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_EXT_AUD4, CLK_ID_PLL6_ROOT,
    CLK_ID_PLL7_ROOT, CLK_ID_PLL5_DIVA, CLK_ID_PLL3_DIVA
};
static int SEC_CSI_MCLK12_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC, CLK_ID_PLL3_DIVB,
    CLK_ID_PLL6_DIVB, CLK_ID_PLL5_ROOT, CLK_ID_PLL4_DIVA
};
static int CLK_ID_GIC4_GIC5_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC, CLK_ID_PLL4_DIVA,
    CLK_ID_PLL5_DIVA
};
static int CLK_ID_CAN5_TO_20_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC, CLK_ID_PLL3_DIVC,
    CLK_ID_PLL5_DIVA, CLK_ID_PLL3_DIVA, CLK_ID_PLL5_DIVD
};
static int CLK_ID_TRACE_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC, CLK_ID_PLL4_DIVB,
    CLK_ID_PLL5_DIVA, CLK_ID_PLL3_DIVA, CLK_ID_PLL4_ROOT
};
static int CLK_ID_SYS_CNT_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC, CLK_ID_PLL4_ROOT,
    CLK_ID_PLL5_ROOT, CLK_ID_PLL3_ROOT, CLK_ID_PLL6_ROOT
};
static int CLK_ID_MSHC_TIMER_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC
};
static int CLK_ID_HPI_CLK600_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC, CLK_ID_PLL3_DIVA,
    CLK_ID_PLL3_ROOT, CLK_ID_PLL4_ROOT, CLK_ID_PLL5_ROOT
};
static int CLK_ID_HPI_CLK800_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC, CLK_ID_PLL5_ROOT,
    CLK_ID_PLL3_ROOT, CLK_ID_PLL4_ROOT, CLK_ID_PLL3_DIVA
};

#if 0 //no resid
enum SEC_CLKIN SEC_CLK_table_SPARE0[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC, CLK_ID_PLL4_ROOT,
    CLK_ID_PLL5_ROOT, CLK_ID_PLL3_ROOT, CLK_ID_PLL3_DIVA
};
enum SEC_CLKIN SEC_CLK_table_SPARE1[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC, CLK_ID_PLL4_ROOT,
    CLK_ID_PLL5_ROOT, CLK_ID_PLL3_ROOT, CLK_ID_PLL3_DIVA
};

enum SEC_CLKIN SEC_CLK_table_ADC_SPARE2[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC, CLK_ID_PLL5_ROOT,
    CLK_ID_PLL4_ROOT, CLK_ID_PLL3_DIVA, CLK_ID_PLL6
};
#endif
#endif
#define CKGEN_SEC_ITEMS \
    CLK_ITEM_WITH_TABLE(CLK_ID_SEC_PLAT, RES_BUS_SLICE_SEC_SEC_PLAT_CTL,"SEC_PLAT", SEC_MP_SPARE0_1PLAT_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_MP_PLAT, RES_CORE_SLICE_SEC_MP_PLAT,"MP_PLAT", SEC_MP_SPARE0_1PLAT_tab), \
    CLK_ITEM_WITH_PARENT(CLK_ID_CE2, RES_IP_SLICE_SEC_CE2, "CE2"),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_I2C_SEC0, RES_IP_SLICE_SEC_I2C_SEC0, "I2C_SEC0", SEC_I2C_SPI_UART_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_I2C_SEC1, RES_IP_SLICE_SEC_I2C_SEC1, "I2C_SEC1", SEC_I2C_SPI_UART_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_SPI_SEC0, RES_IP_SLICE_SEC_SPI_SEC0, "SPI_SEC0", SEC_I2C_SPI_UART_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_SPI_SEC1, RES_IP_SLICE_SEC_SPI_SEC1, "SPI_SEC1", SEC_I2C_SPI_UART_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_UART_SEC0, RES_IP_SLICE_SEC_UART_SEC0, "UART_SEC0", SEC_I2C_SPI_UART_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_UART_SEC1, RES_IP_SLICE_SEC_UART_SEC1, "UART_SEC1", SEC_I2C_SPI_UART_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_EMMC1, RES_IP_SLICE_SEC_EMMC1, "EMMC1", SEC_EMMC1234_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_EMMC2, RES_IP_SLICE_SEC_EMMC2, "EMMC2", SEC_EMMC1234_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_EMMC3, RES_IP_SLICE_SEC_EMMC3, "EMMC3", SEC_EMMC1234_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_EMMC4, RES_IP_SLICE_SEC_EMMC4, "EMMC4", SEC_EMMC1234_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_ENET2_TX, RES_IP_SLICE_SEC_ENET2_TX, "ENET2_TX", SEC_ENET2_TX_RMII_PHY_REF_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_ENET2_RMII, RES_IP_SLICE_SEC_ENET2_RMII, "ENET2_RMII", SEC_ENET2_TX_RMII_PHY_REF_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_ENET2_PHY_REF, RES_IP_SLICE_SEC_ENET2_PHY_REF, "ENET2_PHY_REF", SEC_ENET2_TX_RMII_PHY_REF_tab),  \
    CLK_ITEM_WITH_PARENT(CLK_ID_ENET2_TIMER_SEC, RES_IP_SLICE_SEC_ENET2_TIMER_SEC, "ENET2_TIMER"),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_SPDIF1, RES_IP_SLICE_SEC_SPDIF1, "SPDIF1", SEC_SPDIF1234_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_SPDIF2, RES_IP_SLICE_SEC_SPDIF2, "SPDIF2", SEC_SPDIF1234_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_SPDIF3, RES_IP_SLICE_SEC_SPDIF3, "SPDIF3", SEC_SPDIF1234_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_SPDIF4, RES_IP_SLICE_SEC_SPDIF4, "SPDIF4", SEC_SPDIF1234_tab),   \
    CLK_ITEM_WITH_PARENT(CLK_ID_OSPI2, RES_IP_SLICE_SEC_OSPI2, "OSPI2"),    \
    CLK_ITEM_WITH_TABLE(CLK_ID_TIMER3, RES_IP_SLICE_SEC_TIMER3, "TIMER3", SEC_TIMER3TO8_PWM3TO8_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_TIMER4, RES_IP_SLICE_SEC_TIMER4, "TIMER4", SEC_TIMER3TO8_PWM3TO8_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_TIMER5, RES_IP_SLICE_SEC_TIMER5, "TIMER5", SEC_TIMER3TO8_PWM3TO8_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_TIMER6, RES_IP_SLICE_SEC_TIMER6, "TIMER6", SEC_TIMER3TO8_PWM3TO8_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_TIMER7, RES_IP_SLICE_SEC_TIMER7, "TIMER7", SEC_TIMER3TO8_PWM3TO8_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_TIMER8, RES_IP_SLICE_SEC_TIMER8, "TIMER8", SEC_TIMER3TO8_PWM3TO8_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_PWM3, RES_IP_SLICE_SEC_PWM3, "PWM3", SEC_TIMER3TO8_PWM3TO8_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_PWM4, RES_IP_SLICE_SEC_PWM4, "PWM4", SEC_TIMER3TO8_PWM3TO8_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_PWM5, RES_IP_SLICE_SEC_PWM5, "PWM5", SEC_TIMER3TO8_PWM3TO8_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_PWM6, RES_IP_SLICE_SEC_PWM6, "PWM6", SEC_TIMER3TO8_PWM3TO8_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_PWM7, RES_IP_SLICE_SEC_PWM7, "PWM7", SEC_TIMER3TO8_PWM3TO8_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_PWM8, RES_IP_SLICE_SEC_PWM8, "PWM8", SEC_TIMER3TO8_PWM3TO8_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_I2S_MCLK2, RES_IP_SLICE_SEC_I2S_MCLK2, "I2S_MCLK2", SEC_I2S_MCLK2_MC1_SC3_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_I2S_MC1, RES_IP_SLICE_SEC_I2S_MC1, "I2S_MC1", SEC_I2S_MCLK2_MC1_SC3_tab),    \
    CLK_ITEM_WITH_TABLE(CLK_ID_I2S_SC3, RES_IP_SLICE_SEC_I2S_SC3, "I2S_SC3", SEC_I2S_MCLK2_MC1_SC3_tab),    \
    CLK_ITEM_WITH_TABLE(CLK_ID_I2S_MCLK3, RES_IP_SLICE_SEC_I2S_MCLK3, "I2S_MCLK3", SEC_I2S_MCLK3_MC2_SC5_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_I2S_MC2, RES_IP_SLICE_SEC_I2S_MC2, "I2S_MC2", SEC_I2S_MCLK3_MC2_SC5_tab),    \
    CLK_ITEM_WITH_TABLE(CLK_ID_I2S_SC5, RES_IP_SLICE_SEC_I2S_SC5, "I2S_SC5", SEC_I2S_MCLK3_MC2_SC5_tab),    \
    CLK_ITEM_WITH_TABLE(CLK_ID_I2S_SC4, RES_IP_SLICE_SEC_I2S_SC4, "I2S_SC4", SEC_I2S_SC4_SC7_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_I2S_SC7, RES_IP_SLICE_SEC_I2S_SC7, "I2S_SC7", SEC_I2S_SC4_SC7_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_I2S_SC6, RES_IP_SLICE_SEC_I2S_SC6, "I2S_SC6", SEC_I2S_SC6_SC8_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_I2S_SC8, RES_IP_SLICE_SEC_I2S_SC8, "I2S_SC8", SEC_I2S_SC6_SC8_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_CSI_MCLK1, RES_IP_SLICE_SEC_CSI_MCLK1, "CSI_MCLK1", SEC_CSI_MCLK12_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_CSI_MCLK2, RES_IP_SLICE_SEC_CSI_MCLK2, "CSI_MCLK2", SEC_CSI_MCLK12_tab), \
    CLK_ITEM_WITH_PARENT(CLK_ID_GIC4_GIC5, RES_IP_SLICE_SEC_GIC4_GIC5, "GIC4_GIC5"),    \
    CLK_ITEM_WITH_PARENT(CLK_ID_CAN5_TO_20, RES_IP_SLICE_SEC_CAN5_CAN20, "CAN5_TO_CAN20"),  \
    CLK_ITEM_WITH_PARENT(CLK_ID_TRACE, RES_IP_SLICE_SEC_TRACE, "TRACE"),    \
    CLK_ITEM_WITH_PARENT(CLK_ID_SYS_CNT, RES_IP_SLICE_SEC_SYS_CNT, "SYS_CNT"),  \
    CLK_ITEM_WITH_PARENT(CLK_ID_MSHC_TIMER, RES_IP_SLICE_SEC_MSHC_TIMER, "MSHC_TIMER"), \
    CLK_ITEM_WITH_PARENT(CLK_ID_HPI_CLK600, RES_IP_SLICE_SEC_HPI_CLK600, "HPI_CLK600"), \
    CLK_ITEM_WITH_PARENT(CLK_ID_HPI_CLK800, RES_IP_SLICE_SEC_HPI_CLK800, "HPI_CLK800"), \

#if MODULE_HELPER_CKGEN_DISP
/*CKGEN DISP*/
/*
no resid
dc5_alt_dsp_clk

*/
static int DISP_MIPI_CSI123_PIX_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL_DISP_DIVB, CLK_ID_PLL_DISP_ROOT
};

static int DISP_DP123_DC12345_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL_DISP_DIVA, CLK_ID_PLL_DISP_ROOT
};

static int DISP_EXT_AUD1234_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL_LVDS1_ROOT, CLK_ID_PLL_LVDS2_ROOT, CLK_ID_PLL_LVDS3_ROOT, CLK_ID_PLL_LVDS4_ROOT
};
#endif

#define CKGEN_DISP_ITEMS \
    CLK_ITEM_WITH_TABLE(CLK_ID_MIPI_CSI1_PIX, RES_IP_SLICE_DISP_MIPI_CSI1_PIX, "MIPI_CSI1_PIX", DISP_MIPI_CSI123_PIX_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_MIPI_CSI2_PIX, RES_IP_SLICE_DISP_MIPI_CSI2_PIX, "MIPI_CSI2_PIX", DISP_MIPI_CSI123_PIX_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_MIPI_CSI3_PIX, RES_IP_SLICE_DISP_MIPI_CSI3_PIX, "MIPI_CSI3_PIX", DISP_MIPI_CSI123_PIX_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_DISP_BUS, RES_BUS_SLICE_DISP_DISP_BUS_CTL, "DISP_BUS", DISP_MIPI_CSI123_PIX_tab),    \
    CLK_ITEM_WITH_TABLE(CLK_ID_DP1, RES_IP_SLICE_DISP_DP1, "DP1", DISP_DP123_DC12345_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_DP2, RES_IP_SLICE_DISP_DP2, "DP2", DISP_DP123_DC12345_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_DP3, RES_IP_SLICE_DISP_DP3, "DP3", DISP_DP123_DC12345_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_DC1, RES_IP_SLICE_DISP_DC1, "DC1", DISP_DP123_DC12345_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_DC2, RES_IP_SLICE_DISP_DC2, "DC2", DISP_DP123_DC12345_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_DC3, RES_IP_SLICE_DISP_DC3, "DC3", DISP_DP123_DC12345_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_DC4, RES_IP_SLICE_DISP_DC4, "DC4", DISP_DP123_DC12345_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_DC5, RES_IP_SLICE_DISP_DC5, "DC5", DISP_DP123_DC12345_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_EXT_AUD1, RES_IP_SLICE_DISP_EXT_AUD1, "EXT_AUD1", DISP_EXT_AUD1234_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_EXT_AUD2, RES_IP_SLICE_DISP_EXT_AUD2, "EXT_AUD2", DISP_EXT_AUD1234_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_EXT_AUD3, RES_IP_SLICE_DISP_EXT_AUD3, "EXT_AUD3", DISP_EXT_AUD1234_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_EXT_AUD4, RES_IP_SLICE_DISP_EXT_AUD4, "EXT_AUD4", DISP_EXT_AUD1234_tab), \


#if MODULE_HELPER_CKGEN_SAF
/*CKGEN SAF*/
static int CLK_ID_SAF_PLAT_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL1_ROOT, CLK_ID_PLL2_ROOT, CLK_ID_PLL1_DIVA, CLK_ID_PLL2_DIVA
};
static int CLK_ID_CE1_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL2_ROOT, CLK_ID_PLL1_ROOT
};
static int SAF_I2C_UART_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL2_DIVD, CLK_ID_PLL2_DIVB, CLK_ID_PLL1_DIVB, CLK_ID_PLL1_DIVD
};

static int CLK_ID_SPI_SAF_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL1_DIVB, CLK_ID_PLL2_DIVB, CLK_ID_PLL2_DIVD, CLK_ID_PLL1_DIVD
};
static int CLK_ID_I2S_MCLK1_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL1_DIVA, CLK_ID_PLL2_DIVA, CLK_ID_PLL1_DIVB, CLK_ID_PLL2_DIVB
};
static int CLK_ID_I2S_SC1_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_EXT_AUD3,
    CLK_ID_PLL1_DIVA, CLK_ID_PLL2_DIVA, CLK_ID_PLL1_DIVB, CLK_ID_PLL2_DIVB
};
static int CLK_ID_I2S_SC2_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_EXT_AUD4,
    CLK_ID_PLL1_DIVA, CLK_ID_PLL2_DIVA, CLK_ID_PLL1_DIVB, CLK_ID_PLL2_DIVB
};
static int SAF_ENET1_TX_RMII_PHY_REF_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL1_DIVC
};
static int CLK_ID_ENET1_TIMER_SEC_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL1_DIVA, CLK_ID_PLL2_ROOT, CLK_ID_PLL2_DIVA, CLK_ID_PLL2_DIVC
};
static int CLK_ID_OSPI1_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL1_DIVB, CLK_ID_PLL2_DIVB, CLK_ID_PLL2_DIVC, CLK_ID_PLL2_DIVA
};
static int SAF_TIMER12_PWM12_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL2_DIVA, CLK_ID_PLL1_DIVD, CLK_ID_PLL1_DIVA, CLK_ID_PLL2_DIVC
};
static int CLK_ID_CAN1_TO_4_tab[] = {
    CLK_ID_RC_24M, CLK_ID_RC_RTC, CLK_ID_XTAL_24M, CLK_ID_XTAL_RTC,
    CLK_ID_PLL2_DIVD, CLK_ID_PLL1_DIVD, CLK_ID_PLL1_DIVA
};
#endif

#define CKGEN_SAF_ITEMS \
    CLK_ITEM_WITH_PARENT(CLK_ID_SAF_PLAT, RES_BUS_SLICE_SAF_SAF_PLAT_CTL, "SAF_PLAT"),  \
    CLK_ITEM_WITH_PARENT(CLK_ID_CE1, RES_IP_SLICE_SAF_CE1, "CE1"),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_I2C_SAF, RES_IP_SLICE_SAF_I2C_SAF, "I2C_SAF", SAF_I2C_UART_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_UART_SAF, RES_IP_SLICE_SAF_UART_SAF, "UART_SAF", SAF_I2C_UART_tab),  \
    CLK_ITEM_WITH_PARENT(CLK_ID_SPI_SAF, RES_IP_SLICE_SAF_SPI_SAF, "SPI_SAF"),  \
    CLK_ITEM_WITH_PARENT(CLK_ID_I2S_MCLK1, RES_IP_SLICE_SAF_I2S_MCLK1, "I2S_MCLK1"),    \
    CLK_ITEM_WITH_PARENT(CLK_ID_I2S_SC1, RES_IP_SLICE_SAF_I2S_SC1, "I2S_SC1"),  \
    CLK_ITEM_WITH_PARENT(CLK_ID_I2S_SC2, RES_IP_SLICE_SAF_I2S_SC2, "I2S_SC2"),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_ENET1_TX, RES_IP_SLICE_SAF_ENET1_TX, "ENET1_TX", SAF_ENET1_TX_RMII_PHY_REF_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_ENET1_RMII, RES_IP_SLICE_SAF_ENET1_RMII, "ENET1_RMII", SAF_ENET1_TX_RMII_PHY_REF_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_ENET1_PHY_REF, RES_IP_SLICE_SAF_ENET1_PHY_REF, "ENET1_PHY_REF", SAF_ENET1_TX_RMII_PHY_REF_tab),  \
    CLK_ITEM_WITH_PARENT(CLK_ID_ENET1_TIMER_SEC, RES_IP_SLICE_SAF_ENET1_TIMER_SEC, "ENET1_TIMER_SEC"),  \
    CLK_ITEM_WITH_PARENT(CLK_ID_OSPI1, RES_IP_SLICE_SAF_OSPI1, "OSPI1"),    \
    CLK_ITEM_WITH_TABLE(CLK_ID_TIMER1, RES_IP_SLICE_SAF_TIMER1, "TIMER1", SAF_TIMER12_PWM12_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_TIMER2, RES_IP_SLICE_SAF_TIMER2, "TIMER2", SAF_TIMER12_PWM12_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_PWM1, RES_IP_SLICE_SAF_PWM1, "PWM1", SAF_TIMER12_PWM12_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_PWM2, RES_IP_SLICE_SAF_PWM2, "PWM2", SAF_TIMER12_PWM12_tab), \
    CLK_ITEM_WITH_PARENT(CLK_ID_CAN1_TO_4, RES_IP_SLICE_SAF_CAN_1_TO_4, "CAN1_TO_4"),   \


#if MODULE_HELPER_CKGEN_SOC
/*CKGEN SOC*/
static int SOC_UUU_CKGEN_tab[] = {
    CLK_ID_RC_24M, CLK_ID_XTAL_24M, CLK_ID_PLL1_DIVA, CLK_ID_PLL2_DIVA,
};
static int CLK_ID_NOC_BUS_tab[] = {
    CLK_ID_RC_24M, CLK_ID_XTAL_24M, CLK_ID_PLL1_DIVA, CLK_ID_PLL2_DIVA,
    CLK_ID_PLL_HPI_ROOT
};
#endif

#define CKGEN_SOC_ITEMS \
    CLK_ITEM_WITH_TABLE(CLK_ID_CPU1A, RES_CORE_SLICE_SOC_CPU1A, "CPU1A", SOC_UUU_CKGEN_tab),    \
    CLK_ITEM_WITH_TABLE(CLK_ID_CPU1B, RES_CORE_SLICE_SOC_CPU1B, "CPU1B", SOC_UUU_CKGEN_tab),    \
    CLK_ITEM_WITH_TABLE(CLK_ID_CPU2, RES_CORE_SLICE_SOC_CPU2, "CPU2", SOC_UUU_CKGEN_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_GPU1, RES_CORE_SLICE_SOC_GPU1, "GPU1", SOC_UUU_CKGEN_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_GPU2, RES_CORE_SLICE_SOC_GPU2, "GPU2", SOC_UUU_CKGEN_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_VPU1, RES_IP_SLICE_SOC_VPU1, "VPU1", SOC_UUU_CKGEN_tab), \
    CLK_ITEM_WITH_TABLE(CLK_ID_MJPEG, RES_IP_SLICE_SOC_MJPEG, "MJPEG", SOC_UUU_CKGEN_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_VPU_BUS, RES_BUS_SLICE_SOC_VPU_BUS_CTL, "VPU_BUS", SOC_UUU_CKGEN_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_VSN_BUS, RES_BUS_SLICE_SOC_VSN_BUS_CTL, "VSN_BUS", SOC_UUU_CKGEN_tab),   \
    CLK_ITEM_WITH_TABLE(CLK_ID_DDR, RES_CORE_SLICE_SOC_DDR, "DDR", SOC_UUU_CKGEN_tab),  \
    CLK_ITEM_WITH_TABLE(CLK_ID_HIS_BUS, RES_BUS_SLICE_SOC_HIS_BUS_CTL, "HIS_BUS", SOC_UUU_CKGEN_tab),   \
    CLK_ITEM_WITH_PARENT(CLK_ID_NOC_BUS, RES_BUS_SLICE_SOC_NOC_BUS_CLOCK_CTL, "NOC_BUS"),   \

/*CKGEN UUU*/
/*
sample
static int CLK_ID_CPU1A_uuu_sel0_tab[] = {
    CLK_ID_CPU1A, CLK_ID_PLL_CPU1A_ROOT
};
static int CLK_ID_CPU1A_uuu_mnpq_tab[] = {
    CLK_ID_CPU1A_SEL0
};
static int CLK_ID_CPU1A_0_tab[] = {//sel1
    CLK_ID_CPU1A_M, CLK_ID_PLL_CPU1A_ROOT
};
*/
#define UUU_ITEM_TABLE(_id, _pll_id)    \
    static int _id ##_uuu_sel0_tab[] = {    \
        _id, _pll_id    \
    };  \
    static int _id ##_uuu_mnpq_tab[] = {    \
        _id ##_SEL0 \
    };  \
    static int _id ##_0_tab[] = {/*sel1*/   \
        _id ##_M, _pll_id   \
    };

#define CKGEN_UUU_COMPOSITE_ITEM(_id, _resid, _name)    \
    CLK_UUU_ITEM_WITH_TABLE(_id ##_SEL0, _resid, UUU_SEL0, _name"SEL0", _id ##_uuu_sel0_tab),   \
    CLK_UUU_ITEM_WITH_TABLE(_id ##_M, _resid, UUU_M, _name"M", _id ##_uuu_mnpq_tab),    \
    CLK_UUU_ITEM_WITH_TABLE(_id ##_0, _resid, UUU_SEL1, _name"0", _id ##_0_tab),    \
    CLK_UUU_ITEM_WITH_TABLE(_id ##_1, _resid, UUU_N, _name"1", _id ##_uuu_mnpq_tab),    \
    CLK_UUU_ITEM_WITH_TABLE(_id ##_2, _resid, UUU_P, _name"2", _id ##_uuu_mnpq_tab),    \
    CLK_UUU_ITEM_WITH_TABLE(_id ##_3, _resid, UUU_Q, _name"3", _id ##_uuu_mnpq_tab) \


#if MODULE_HELPER_CKGEN_UUU
UUU_ITEM_TABLE(CLK_ID_CPU1A, CLK_ID_PLL_CPU1A_ROOT)
UUU_ITEM_TABLE(CLK_ID_CPU1B, CLK_ID_PLL_CPU1B_ROOT)
UUU_ITEM_TABLE(CLK_ID_CPU2, CLK_ID_PLL_CPU2_ROOT)
UUU_ITEM_TABLE(CLK_ID_GPU1, CLK_ID_PLL_GPU1_ROOT)
UUU_ITEM_TABLE(CLK_ID_GPU2, CLK_ID_PLL_GPU2_ROOT)
UUU_ITEM_TABLE(CLK_ID_VPU1, CLK_ID_PLL_VPU_DIVB)
UUU_ITEM_TABLE(CLK_ID_MJPEG, CLK_ID_PLL_VPU_DIVB)
UUU_ITEM_TABLE(CLK_ID_VPU_BUS, CLK_ID_PLL_VPU_DIVA)
UUU_ITEM_TABLE(CLK_ID_VSN_BUS, CLK_ID_PLL_VSN_ROOT)
UUU_ITEM_TABLE(CLK_ID_DDR, CLK_ID_PLL_DDR_ROOT)
UUU_ITEM_TABLE(CLK_ID_HIS_BUS, CLK_ID_PLL_HIS_ROOT)
#endif

#define CKGEN_UUU_ITEMS \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_CPU1A, RES_UUU_WRAP_SOC_CPU1A, "CPU1A_"),   \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_CPU1B, RES_UUU_WRAP_SOC_CPU1B, "CPU1B_"),   \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_CPU2, RES_UUU_WRAP_SOC_CPU2, "CPU2_"),  \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_GPU1, RES_UUU_WRAP_SOC_GPU1, "GPU1_"),  \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_GPU2, RES_UUU_WRAP_SOC_GPU2, "GPU2_"),  \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_VPU1, RES_UUU_WRAP_SOC_VPU1, "VPU1_"),  \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_MJPEG, RES_UUU_WRAP_SOC_MJPEG, "MJPEG_"),   \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_VPU_BUS, RES_UUU_WRAP_SOC_VPU_BUS, "VPU_BUS_"), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_VSN_BUS, RES_UUU_WRAP_SOC_VSN, "VSN_BUS_"), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_DDR, RES_UUU_WRAP_SOC_DDR, "DDR_"), \
    CKGEN_UUU_COMPOSITE_ITEM(CLK_ID_HIS_BUS, RES_UUU_WRAP_SOC_HIS_BUS, "HIS_BUS_"), \

/* gating*/
static int set_ckgen_gate_endis(unsigned long resid, bool enable)
{
    int ret = 0;
    bool result;
    void *g_handle;
    ret = hal_clock_creat_handle(&g_handle);

    if (!ret) {
        printf("do set_ckgen_gate_endis creat handle failed\n");
        return -1;
    }

    result = enable ? hal_clock_enable(g_handle,
                                       resid) : hal_clock_disable(g_handle, resid);

    if (!result) {
        ret = -1;
    }

    hal_clock_release_handle(g_handle);
    return ret;
}

/*BUS*/
static int get_ckgen_bus_ctl(unsigned long resid, clkgen_bus_ctl *ctl,
                             clkgen_bus_gasket *gasket)
{
    bool ret = false;
    void *g_handle;
    ret = hal_clock_creat_handle(&g_handle);

    if (!ret) {
        printf("do %s creat handle failed\n", __func__);
        return -1;
    }

    ret = hal_clock_busctl_get(g_handle, resid, ctl, gasket);
    hal_clock_release_handle(g_handle);
    return ret ? 0 : -1;
}

static int set_ckgen_bus_ctl(unsigned long resid,
                             const clkgen_bus_ctl *ctl,
                             const clkgen_bus_gasket *gasket)
{
    bool ret = false;
    void *g_handle;
    ret = hal_clock_creat_handle(&g_handle);

    if (!ret) {
        printf("do %s creat handle failed\n", __func__);
        return -1;
    }

    ret = hal_clock_busctl_set(g_handle, resid, ctl, gasket);
    hal_clock_release_handle(g_handle);
    return ret ? 0 : -1;
}

static int set_ckgen_bus_endis(unsigned long resid, bool enable)
{
    int ret = 0;
    clkgen_bus_ctl ctl;

    if (get_ckgen_bus_ctl(resid, &ctl, NULL) < 0) {
        return -1;
    }

    if (!enable) {
        ctl.cg_en_a = ctl.cg_en_b = 0;
    }
    else {
        ctl.cg_en_a = ctl.cg_en_b = 1;
    }

    ret |= set_ckgen_bus_ctl(resid, &ctl, NULL);
    return ret;
}
static bool get_ckgen_bus_endis(unsigned long resid)
{
    clkgen_bus_ctl ctl;

    if (get_ckgen_bus_ctl(resid, &ctl, NULL) < 0) {
        return false;
    }

    if (ctl.a_b_sel == 0) {
        return ctl.cg_en_a == 1;
    }
    else {
        return ctl.cg_en_b == 1;
    }
}

static unsigned long get_ckgen_bus_rate(unsigned long resid,
                                        unsigned long prate, bool bymonitor)
{
    unsigned long rate;
    int ret;

    if (bymonitor) {
        void *g_handle;
        ret = hal_clock_creat_handle(&g_handle);

        if (!ret) {
            printf("do get_ckgen_bus_rate creat handle failed\n");
            return 0;
        }

        rate = hal_clock_busclk_get(g_handle, resid,
                                    mon_ref_clk_24M, 0);

        if (rate == 0) //maybe it's too slow, need monitor with 32k
            rate = hal_clock_busclk_get(g_handle, resid,
                                        mon_ref_clk_32K, 0);

        hal_clock_release_handle(g_handle);
    }
    else {
        clkgen_bus_ctl ctl;
        int div;

        if (get_ckgen_bus_ctl(resid, &ctl, NULL) < 0) {
            return -1;
        }

        if (ctl.a_b_sel == 0) { //A PATH
            div = (ctl.pre_div_num_a + 1) * (ctl.post_div_num + 1);
        }
        else { //B PATH
            div = (ctl.pre_div_num_b + 1) * (ctl.post_div_num + 1);
        }

        rate = prate / div;
    }

    return rate;
}
static int set_ckgen_bus_rate(unsigned long resid, unsigned long prate,
                              unsigned long freq)
{
    int ret = 0;
    clkgen_bus_ctl ctl;

    if (get_ckgen_bus_ctl(resid, &ctl, NULL) < 0) {
        return -1;
    }

    int i, j;
    int prediv_max = 1 << 3;
    int postdiv_max = 1 << 6;
    unsigned int div = 0;
    div = prate / freq;

    for (i = 1; i <= prediv_max; i++) {
        for (j = 1; j <= postdiv_max; j++) {
            if ((i * j) == (int)div) {
                goto found;
            }
        }

        //dprintf(CRITICAL, "div %d i %d, j %d\n",div, i,j);
    }

found:
    ASSERT(i <= prediv_max && j <= postdiv_max);
    //dprintf(CRITICAL, "div %d found i %d, j %d\n",div, i,j);
    ctl.pre_div_num_a = ctl.pre_div_num_b = i - 1;
    ctl.post_div_num = j - 1;
    ret |= set_ckgen_bus_ctl(resid, &ctl, NULL);
    return ret;
}

static int get_ckgen_bus_parent(unsigned long resid)
{
    clkgen_bus_ctl ctl;

    if (get_ckgen_bus_ctl(resid, &ctl, NULL) < 0) {
        return -1;
    }

    return (ctl.a_b_sel == 0) ? ctl.src_sel_a : ctl.src_sel_b;
}
static int set_ckgen_bus_parent(unsigned long resid, int p_index)
{
    int ret = 0;
    clkgen_bus_ctl ctl;

    if (get_ckgen_bus_ctl(resid, &ctl, NULL) < 0) {
        return -1;
    }

    ctl.src_sel_a = ctl.src_sel_b = p_index;
    ret |= set_ckgen_bus_ctl(resid, &ctl, NULL);
    return ret;
}
static unsigned long get_ckgen_bus_ip_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int i, j;
    int prediv_max = 1 << 3;
    int postdiv_max = 1 << 6;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, bestprate = *prate,
                  prate_cur = *prate, rate;
    unsigned long diff;
    unsigned int maxdiv = prediv_max * postdiv_max, mindiv, bestdiv;
    struct clk *p = clk->parents[pindex];
    maxdiv = MIN(clk->maxdiv, prediv_max * postdiv_max);
    maxdiv = MIN(UINT32_MAX / freq, maxdiv);
    mindiv = MAX(clk->mindiv, 1);
    maxdiv = MAX(maxdiv, mindiv);
    //initial value of bestdiv is the fix div or current div.
    bestdiv = mindiv;

    for (i = 1; i <= prediv_max; i++) {
        for (j = 1; j <= postdiv_max; j++) {
            if (((i * j) < (int)mindiv) || ((i * j) > (int)maxdiv)) {
                break;
            }

            prate_cur = *prate;
            rate = res_clk_div_round_rate(clk, p, i * j, &prate_cur, freq);

            if (!res_clk_is_valid_round_rate(clk, pindex, rate, prate_cur)) { continue; }

            diff = abs_clk(freq, prate_cur / (i * j));

            if (diff == 0) {
                *prate = prate_cur;
                return freq;
            }

            if (diff < bestratediff) {
                bestratediff = diff;
                bestprate = prate_cur;
                bestdiv = (i * j);
            }
        }
    }

    *prate = bestprate;
    bestrate = (bestprate) / (bestdiv);
    return bestrate;
}

/*CORE */
static int get_ckgen_core_ctl(unsigned long resid, clkgen_core_ctl *ctl)
{
    bool ret = false;
    void *g_handle;
    ret = hal_clock_creat_handle(&g_handle);

    if (!ret) {
        printf("do %s creat handle failed\n", __func__);
        return -1;
    }

    ret = hal_clock_corectl_get(g_handle, resid, ctl);
    hal_clock_release_handle(g_handle);
    return ret ? 0 : -1;
}

static int set_ckgen_core_ctl(unsigned long resid, clkgen_core_ctl *ctl)
{
    bool ret = false;
    void *g_handle;
    ret = hal_clock_creat_handle(&g_handle);

    if (!ret) {
        printf("do %s creat handle failed\n", __func__);
        return -1;
    }

    ret = hal_clock_corectl_set(g_handle, resid, ctl);
    hal_clock_release_handle(g_handle);
    return ret ? 0 : -1;
}

static int set_ckgen_core_endis(unsigned long resid, bool enable)
{
    int ret = 0;
    clkgen_core_ctl ctl;

    if (get_ckgen_core_ctl(resid, &ctl) < 0) {
        return -1;
    }

    if (!enable) {
        ctl.cg_en_a = 0;
        ctl.cg_en_b = 0;
    }
    else {
        if (ctl.a_b_sel == 0) {
            ctl.cg_en_a = 1;
        }
        else {
            ctl.cg_en_b = 1;
        }
    }

    ret |= set_ckgen_core_ctl(resid, &ctl);
    return ret;
}
static bool get_ckgen_core_endis(unsigned long resid)
{
    clkgen_core_ctl ctl;

    if (get_ckgen_core_ctl(resid, &ctl) < 0) {
        return false;
    }

    if (ctl.a_b_sel == 0) {
        return ctl.cg_en_a == 1;
    }
    else {
        return ctl.cg_en_b == 1;
    }
}

static unsigned long get_ckgen_core_rate(unsigned long resid,
        unsigned long prate, bool bymonitor)
{
    unsigned long rate;
    int ret;

    if (bymonitor) {
        void *g_handle;
        ret = hal_clock_creat_handle(&g_handle);

        if (!ret) {
            printf("do get_ckgen_bus_rate creat handle failed\n");
            return 0;
        }

        rate = hal_clock_coreclk_get(g_handle, resid,
                                     mon_ref_clk_24M, 0);

        if (rate == 0) //maybe it's too slow, need monitor with 32k
            rate = hal_clock_coreclk_get(g_handle, resid,
                                         mon_ref_clk_32K, 0);

        hal_clock_release_handle(g_handle);
    }
    else {
        clkgen_core_ctl ctl;
        int div;

        if (get_ckgen_core_ctl(resid, &ctl) < 0) {
            return -1;
        }

        div = ctl.post_div_num + 1;
        rate = prate / div;
    }

    return rate;
}

static int set_ckgen_core_rate(unsigned long resid, unsigned long prate,
                               unsigned long freq)
{
    int ret = 0;
    clkgen_core_ctl ctl;
    unsigned int postdiv_max = 1 << 6;
    unsigned int div = 0;

    if (get_ckgen_core_ctl(resid, &ctl) < 0) {
        return -1;
    }

    div = prate / freq;
    ASSERT(div <= postdiv_max);
    ctl.post_div_num = div - 1;
    ret |= set_ckgen_core_ctl(resid, &ctl);
    return ret;
}

static int get_ckgen_core_parent(unsigned long resid)
{
    clkgen_core_ctl ctl;

    if (get_ckgen_core_ctl(resid, &ctl) < 0) {
        return -1;
    }

    return (ctl.a_b_sel == 0) ? ctl.src_sel_a : ctl.src_sel_b;
}
static int set_ckgen_core_parent(unsigned long resid, int p_index)
{
    int ret = 0;
    clkgen_core_ctl ctl;

    if (get_ckgen_core_ctl(resid, &ctl) < 0) {
        return -1;
    }

    ctl.src_sel_a = ctl.src_sel_b = p_index;
    ret |= set_ckgen_core_ctl(resid, &ctl);
    return ret;
}

static unsigned long get_ckgen_core_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int i;
    int postdiv_max = 1 << 6;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, bestprate = *prate,
                  prate_cur = *prate, rate;
    unsigned int maxdiv, mindiv, bestdiv;
    unsigned long diff;
    struct clk *p = clk->parents[pindex];
    maxdiv = MIN(clk->maxdiv, postdiv_max);
    maxdiv = MIN(UINT32_MAX / freq, maxdiv);
    mindiv = MAX(clk->mindiv, 1);
    maxdiv = MAX(maxdiv, mindiv);
    //initial value of bestdiv is the fix div or current div.
    bestdiv = mindiv;

    for (i = (int)mindiv; i <= (int)maxdiv; i++) {
        prate_cur = *prate;
        rate = res_clk_div_round_rate(clk, p, i, &prate_cur, freq);

        if (!res_clk_is_valid_round_rate(clk, pindex, rate, prate_cur)) { continue; }

        diff = abs_clk(freq, prate_cur / (i));

        if (diff == 0) {
            *prate = prate_cur;
            return freq;
        }

        if (diff < bestratediff) {
            bestratediff = diff;
            bestprate = prate_cur;
            bestdiv = i;
        }
    }

    *prate = bestprate;
    bestrate = (bestprate) / (bestdiv);
    return bestrate;
}


/*IP*/
static int get_ckgen_ip_ctl(unsigned long resid, clkgen_ip_ctl *ctl)
{
    bool ret = false;
    void *g_handle;
    ret = hal_clock_creat_handle(&g_handle);

    if (!ret) {
        printf("do %s creat handle failed\n", __func__);
        return -1;
    }

    ret = hal_clock_ipctl_get(g_handle, resid, ctl);
    hal_clock_release_handle(g_handle);
    return ret ? 0 : -1;
}

static int set_ckgen_ip_ctl(unsigned long resid, clkgen_ip_ctl *ctl)
{
    bool ret = false;
    void *g_handle;
    ret = hal_clock_creat_handle(&g_handle);

    if (!ret) {
        printf("do %s creat handle failed\n", __func__);
        return -1;
    }

    ret = hal_clock_ipctl_set(g_handle, resid, ctl);
    hal_clock_release_handle(g_handle);
    return ret ? 0 : -1;
}


static int set_ckgen_ip_endis(unsigned long resid, bool enable)
{
    int ret = 0;
    clkgen_ip_ctl ctl;

    if (get_ckgen_ip_ctl(resid, &ctl) < 0) {
        return -1;
    }

    if (!enable) {
        ctl.cg_en = 0;
    }
    else {
        ctl.cg_en = 1;
    }

    ret |= set_ckgen_ip_ctl(resid, &ctl);
    return ret;
}
static bool get_ckgen_ip_endis(unsigned long resid)
{
    clkgen_ip_ctl ctl;

    if (get_ckgen_ip_ctl(resid, &ctl) < 0) {
        return false;
    }

    return ctl.cg_en == 1;
}

static unsigned long get_ckgen_ip_rate(unsigned long resid,
                                       unsigned long prate, bool bymonitor)
{
    unsigned long rate;
    int ret;

    if (bymonitor) {
        void *g_handle;
        ret = hal_clock_creat_handle(&g_handle);

        if (!ret) {
            printf("do get_ckgen_bus_rate creat handle failed\n");
            return 0;
        }

        rate = hal_clock_ipclk_get(g_handle, resid,
                                   mon_ref_clk_24M, 0);

        if (rate == 0) //maybe it's too slow, try monitor with 32k
            rate = hal_clock_ipclk_get(g_handle, resid,
                                       mon_ref_clk_32K, 0);

        hal_clock_release_handle(g_handle);
    }
    else {
        clkgen_ip_ctl ctl;
        int div;

        if (get_ckgen_ip_ctl(resid, &ctl) < 0) {
            return -1;
        }

        div = (ctl.pre_div_num + 1) * (ctl.post_div_num + 1);
        rate = prate / div;
    }

    return rate;
}

static int set_ckgen_ip_rate(unsigned long resid, unsigned long prate,
                             unsigned long freq)
{
    int ret = 0;
    clkgen_ip_ctl ctl;

    if (get_ckgen_ip_ctl(resid, &ctl) < 0) {
        return -1;
    }

    int i, j;
    int prediv_max = 1 << 3;
    int postdiv_max = 1 << 6;
    unsigned int div = 0;
    div = prate / freq;

    for (i = 1; i <= prediv_max; i++) {
        for (j = 1; j <= postdiv_max; j++) {
            if ((i * j) == (int)div) {
                goto found;
            }
        }

        //dprintf(CRITICAL, "div %d i %d, j %d\n",div, i,j);
    }

    ASSERT(i <= prediv_max && j <= postdiv_max);
found:
    //dprintf(CRITICAL, "div %d found i %d, j %d\n",div, i,j);
    ctl.pre_div_num = i - 1;
    ctl.post_div_num = j - 1;
    ret |= set_ckgen_ip_ctl(resid, &ctl);
    return ret;
}

static int get_ckgen_ip_parent(unsigned long resid)
{
    clkgen_ip_ctl ctl;

    if (get_ckgen_ip_ctl(resid, &ctl) < 0) {
        return -1;
    }

    return ctl.src_sel;
}
static int set_ckgen_ip_parent(unsigned long resid, int p_index)
{
    int ret = 0;
    clkgen_ip_ctl ctl;

    if (get_ckgen_ip_ctl(resid, &ctl) < 0) {
        return -1;
    }

    ctl.src_sel = p_index;
    ret |= set_ckgen_ip_ctl(resid, &ctl);
    return ret;
}

/*UUU*/
static unsigned long get_ckgen_uuu_rate(unsigned long resid, int uuu_type,
                                        unsigned long prate, bool bymonitor);

static int get_ckgen_uuu_ctl(unsigned long resid, clkgen_uuu_ctl *ctl)
{
    bool ret = false;
    void *g_handle;
    ret = hal_clock_creat_handle(&g_handle);

    if (!ret) {
        printf("do get_ckgen_ip_ctl creat handle failed\n");
        return -1;
    }

    ret = hal_clock_uuuctl_get(g_handle, resid, ctl);
    hal_clock_release_handle(g_handle);
    return ret ? 0 : -1;
}

static int set_ckgen_uuu_ctl(unsigned long resid, clkgen_uuu_ctl *ctl)
{
    bool ret = false;
    void *g_handle;
    ret = hal_clock_creat_handle(&g_handle);

    if (!ret) {
        printf("do get_ckgen_ip_ctl creat handle failed\n");
        return -1;
    }

    ret = hal_clock_uuuctl_set(g_handle, resid, ctl);
    hal_clock_release_handle(g_handle);
    return ret ? 0 : -1;
}


static int set_ckgen_uuu_endis(unsigned long resid, bool enable)
{
    //int ret;
    //uuu always enabled
    return 0;
}

static bool get_ckgen_uuu_endis(unsigned long resid, int uuu_type)
{
    //int ret;
    if (uuu_type == UUU_SEL0
            || uuu_type == UUU_SEL1) { //dummy node, return true.
        return true;
    }

    unsigned long freq = get_ckgen_uuu_rate(resid, uuu_type, 0, true);

    if (freq) {
        return true;
    }
    else {
        return false;
    }
}


static unsigned long get_ckgen_uuu_rate(unsigned long resid, int uuu_type,
                                        unsigned long prate, bool bymonitor)
{
    int ret;
    clkgen_uuu_ctl ctl;

    if (get_ckgen_uuu_ctl(resid, &ctl) < 0) {
        return -1;
    }

    if (uuu_type == UUU_SEL0
            || uuu_type == UUU_SEL1) { //dummy node, return parent rate.
        return prate;
    }

    if (bymonitor) {
        unsigned long rate;
        void *g_handle;
        ret = hal_clock_creat_handle(&g_handle);

        if (!ret) {
            printf("do get_ckgen_uuu_rate creat handle failed\n");
            return 0;
        }

        rate = hal_clock_uuuclk_get(g_handle, resid,
                                    mon_ref_clk_24M, 0);

        if (rate == 0) //maybe it's too slow, try monitor with 32k
            rate = hal_clock_uuuclk_get(g_handle, resid,
                                        mon_ref_clk_32K, 0);

        hal_clock_release_handle(g_handle);
        //the monitor point is after the M, so...
        prate = rate * (ctl.m_div + 1);
    }

    if (uuu_type == UUU_M) {
        return prate / (ctl.m_div + 1);
    }
    else if (uuu_type == UUU_N) {
        return prate / (ctl.n_div + 1);
    }
    else if (uuu_type == UUU_P) {
        return prate / (ctl.p_div + 1);
    }
    else if (uuu_type == UUU_Q) {
        return prate / (ctl.q_div + 1);
    }

    return 0;
}

static int set_ckgen_uuu_rate(unsigned long resid, int uuu_type,
                              unsigned long prate, unsigned long freq)
{
    int ret = 0;
    clkgen_uuu_ctl ctl;
    unsigned int maxdiv = 0;

    if (get_ckgen_uuu_ctl(resid, &ctl) < 0) {
        return -1;
    }

    if (uuu_type == UUU_SEL0 || uuu_type == UUU_SEL1) { //dummy node.
        return 0;
    }
    else if (uuu_type >= UUU_M && uuu_type <= UUU_Q) {
        maxdiv = 1 << 4;
    }
    else {
        //could not be here
        ASSERT(0);
    }

    unsigned int div = 0;
    div = prate / freq;
    ASSERT(div <= maxdiv);

    //dprintf(CRITICAL, "uuu div %d found.\n",div);
    if (uuu_type == UUU_M) {
        ctl.m_div = div - 1;
    }
    else if (uuu_type == UUU_N) {
        ctl.n_div = div - 1;
    }
    else if (uuu_type == UUU_P) {
        ctl.p_div = div - 1;
    }
    else if (uuu_type == UUU_Q) {
        ctl.q_div = div - 1;
    }

    ret |= set_ckgen_uuu_ctl(resid, &ctl);
    return ret;
}

static int get_ckgen_uuu_parent(unsigned long resid, int uuu_type)
{
    clkgen_uuu_ctl ctl;

    if (get_ckgen_uuu_ctl(resid, &ctl) < 0) {
        return -1;
    }

    if (uuu_type == UUU_SEL0) {
        return ctl.uuu_sel0;
    }
    else if (uuu_type >= UUU_M
             && uuu_type <= UUU_Q) {//mnpq only have one parent.
        return 0;
    }
    else {  // sel1
        return ctl.uuu_sel1;
    }
}

static int set_ckgen_uuu_parent(unsigned long resid, int uuu_type,
                                int p_index)
{
    int ret = 0;
    clkgen_uuu_ctl ctl;

    if (get_ckgen_uuu_ctl(resid, &ctl) < 0) {
        return -1;
    }

    if (uuu_type == UUU_SEL0) {
        ctl.uuu_sel0 = p_index;
    }
    else if (uuu_type >= UUU_M
             && uuu_type <= UUU_Q) {//mnpq only have one parent.
        return 0;
    }
    else {  // sel1
        ctl.uuu_sel1 = p_index;
    }

    ret |= set_ckgen_uuu_ctl(resid, &ctl);
    return ret;
}

static unsigned long get_ckgen_uuu_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int i;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, bestprate = *prate,
                  prate_cur = *prate, rate;
    unsigned int mindiv = 1, maxdiv = 0, bestdiv = 0;
    unsigned long diff;
    int uuu_type = clk->uuu.uuu_type;
    struct clk *p = clk->parents[pindex];

    if (uuu_type == UUU_SEL0 || uuu_type == UUU_SEL1) {
        struct clk_request req;
        req.request = freq;
        bestrate = res_clk_round_rate(clk->parents[pindex], &req);

        if (!res_clk_is_valid_round_rate(clk, pindex, bestrate, bestrate)) {
            //dprintf(CRITICAL,"uuusel%d %s p %d bestrate %lu refcnt %d not valid\n", uuu_type, clk->name, pindex, bestrate, clk_get_refcount(clk));
            bestrate = *prate;
        }
        else {
            //dprintf(CRITICAL,"uuusel%d %s p %d bestrate %lu refcnt %d\n", uuu_type, clk->name, pindex, bestrate, clk_get_refcount(clk, true));
        }

        *prate = bestrate;
        return bestrate;
    }
    else if (uuu_type >= UUU_M && uuu_type <= UUU_Q) {
        maxdiv = MIN(clk->maxdiv, 1 << 4);
        maxdiv = MIN(UINT32_MAX / freq, maxdiv);
        mindiv = MAX(clk->mindiv, 1);
        maxdiv = MAX(maxdiv, mindiv);
        //initial value of bestdiv is the fix div or current div.
        bestdiv = mindiv;
    }
    else {
        //could not be here
        ASSERT(0);
    }

    for (i = (int)mindiv; i <= (int)maxdiv; i++) {
        prate_cur = *prate;
        rate = res_clk_div_round_rate(clk, p, i, &prate_cur, freq);

        //dprintf(CRITICAL, "ckgen uuu %s old p %lu req %lu round get p %lu, rate %lu div %d\n", clk->name, *prate, freq, prate_cur, rate, i);
        if (!res_clk_is_valid_round_rate(clk, pindex, rate, prate_cur)) { continue; }

        diff = abs_clk(freq, prate_cur / (i));

        //dprintf(CRITICAL, "ckgen uuu %s old p %lu req %lu round get p %lu ,diff %lu div %d\n", clk->name, *prate, freq, prate_cur, diff, i);
        if (diff == 0) {
            *prate = prate_cur;
            return freq;
        }

        if (diff < bestratediff) {
            bestratediff = diff;
            bestprate = prate_cur;
            bestdiv = i;
        }
    }

    *prate = bestprate;
    bestrate = bestprate / bestdiv;
    //dprintf(CRITICAL, "ckgen uuu %s round get best %lu prate %lu\n", clk->name, bestrate, bestprate);
    return bestrate;
}


