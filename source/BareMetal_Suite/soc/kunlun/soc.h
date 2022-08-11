/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    soc.h
 * @brief   soc overall header file
 */

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include <soc_def.h>
#include <soc_hal.h>
#include "soc_share.h"
#include "iomux_ctrl_safety_reg.h"
#include "iomux_ctrl_ap_reg.h"
#include <srv_pin/srv_pin.h>
#include "rst_idx.h"
#include "ckgate.h"
#include "scr_bits.h"

#include "fusemap.h"

#if defined(TGT_sec)
#include "sec/soc_sec.h"
#include "sec/memory_map.h"
#elif defined(TGT_safe)
#include "safe/soc_safe.h"
#include "safe/memory_map.h"
#else
#include "ap/soc_ap.h"
#include "ap/memory_map.h"
#endif

#include "irq.h"

#define IOMUX_PAD_REG_BASE      0x000
#define IOMUX_MUX_REG_BASE      0x200
#define IOMUX_IN_SEL_REG_BASE   0x400

typedef enum {
    IOMUXC_SAFE = 1,
    IOMUXC_SEC  = 2,
    IOMUXC_RTC = 3,
} kunlun_iomuxc_e;

static inline
uintptr_t soc_get_pin_mux_reg_addr(kunlun_iomuxc_e ctrl_id, U32 reg_id)
{
    uintptr_t ctrl_base = (ctrl_id == IOMUXC_SAFE ? APB_IOMUXC_SAF_BASE :
                           ctrl_id == IOMUXC_SEC ? APB_IOMUXC_SEC_BASE :
                           ctrl_id == IOMUXC_RTC ? APB_IOMUXC_RTC_BASE : 0);

    uintptr_t offset = (ctrl_id == IOMUXC_RTC ? (IOMUX_MUX_REG_BASE + (reg_id) * 4) :
                        SOC_IOMUX_REG_MAP(IOMUX_MUX_REG_BASE + (reg_id) * 4));

    return (ctrl_base + offset);
}

static inline
uintptr_t soc_get_pin_pad_reg_addr(kunlun_iomuxc_e ctrl_id, U32 reg_id)
{
    uintptr_t ctrl_base = (ctrl_id == IOMUXC_SAFE ? APB_IOMUXC_SAF_BASE :
                           ctrl_id == IOMUXC_SEC ? APB_IOMUXC_SEC_BASE :
                           ctrl_id == IOMUXC_RTC ? APB_IOMUXC_RTC_BASE : 0);
    uintptr_t offset = (ctrl_id == IOMUXC_RTC ? (IOMUX_PAD_REG_BASE + (reg_id) * 4) :
                        SOC_IOMUX_REG_MAP(IOMUX_PAD_REG_BASE + (reg_id) * 4));

    return (ctrl_base + offset);
}

static inline
U32 soc_get_pin_in_sel_reg_addr(kunlun_iomuxc_e ctrl_id, U32 reg_id)
{
    uintptr_t ctrl_base = (ctrl_id == IOMUXC_SAFE ? APB_IOMUXC_SAF_BASE :
                           ctrl_id == IOMUXC_SEC ? APB_IOMUXC_SEC_BASE :
                           ctrl_id == IOMUXC_RTC ? APB_IOMUXC_RTC_BASE : 0);
    uintptr_t offset = (ctrl_id == IOMUXC_RTC ? (IOMUX_IN_SEL_REG_BASE + (reg_id) * 4) :
                        SOC_IOMUX_REG_MAP(IOMUX_IN_SEL_REG_BASE + (reg_id) * 4));

    return (ctrl_base + offset);
}

#define INPUT_SOURCE_SELECT_SEL_NONE_OFF    \
            (IOMUX_IN_SEL_REG_BASE + SEL_NONE * 4)
#define IO_PAD_CONFIG_PAD_NONE_OFF          \
            (IOMUX_PAD_REG_BASE + PAD_NONE * 4)
#define PIN_MUX_CONFIG_MUX_NONE_OFF         \
            (IOMUX_MUX_REG_BASE + MUX_NONE * 4)

#define PIN_DCLR(id, mux, pad, in_sel, mux_val, pad_setting, in_sel_val) \
    {PAD_REG_SEL((id), (PIN_MUX_CONFIG_##pad##_OFF - IOMUX_MUX_REG_BASE)/4, \
        (IO_PAD_CONFIG_##pad##_OFF - IOMUX_PAD_REG_BASE)/4, \
        (INPUT_SOURCE_SELECT_##in_sel##_OFF - IOMUX_IN_SEL_REG_BASE)/4),\
        mux_val, pad_setting, in_sel_val}

#define SOC_PLL_REFCLK_FREQ     (24*1000*1000)

#define SOC_TMR_CLK_SRC_SEL     2   /* 24MHz, safety.fsrefclk output */
#define SOC_TMR_CLK_DIV         23  /* div24 actually */

#define SOC_us_TO_TICK(us)      (us)
#define SOC_TICK_TO_us(tick)    (tick)

#define IRAM_CTRL_DATA_INIT_OFF   0x80
#define BM_DATA_INIT_INIT   (0x01U << 0U)
#define BM_DATA_INIT_LOCK   (0x01U << 31U)
#define IRAM_CTRL_MEM_ERR_INT_STA_EN_OFF    0x04
#define IRAM_CTRL_MEM_ERR_INT_SIG_EN_OFF    0x08

#include <iomux/iomux_def.h>

/* Pull-Up, default drive strength */
#define OSPI_PAD_SETTING    \
    (FV_IO_PAD_CONFIG_DS(1) | BM_IO_PAD_CONFIG_PS | BM_IO_PAD_CONFIG_PE)
/* Pull-down on DQS */
#define OSPI_DQS_PAD_SETTING    \
    (FV_IO_PAD_CONFIG_DS(1) | BM_IO_PAD_CONFIG_PE)
#define OSPI_DAT_PAD_SETTING  FV_IO_PAD_CONFIG_DS(1)

/* For mshc phy pins, 'PE' connected to PU and 'PS' to PD acutally. */
#define BM_MSHCPHY_PAD_CONFIG_PU    BM_IO_PAD_CONFIG_PE
#define BM_MSHCPHY_PAD_CONFIG_PD    BM_IO_PAD_CONFIG_PS
/* Pull-up, default drive strength */
#define SDMMC_DAT_PAD_SETTING  \
    (FV_IO_PAD_CONFIG_DS(1) | BM_MSHCPHY_PAD_CONFIG_PU)
/* Pull-down, default drive strength */
#define SDMMC_CLK_PAD_SETTING  \
    (FV_IO_PAD_CONFIG_DS(1) | BM_MSHCPHY_PAD_CONFIG_PD)

#define SAFE_DIV(v) (v)

#define BT_INFOR_ADDR   0x4CFF80    /* shall match link script */

enum {
    MB_CPU_CR5_SAF,
    MB_CPU_CR5_SEC,
    MB_CPU_CR5_MP,
    MB_CPU_AP1,
    MB_CPU_AP2,
};

void clean_cache_range(const void *start, U32 len);
void clean_invalidate_cache_range(const void *start, U32 len);
void invalidate_cache_range(const void *start, U32 len);

void soc_gpio_mux(module_e ctrl, U32 io);
bool soc_wdt_is_hw_en(U32 base);

#endif  // __PLATFORM_H__
