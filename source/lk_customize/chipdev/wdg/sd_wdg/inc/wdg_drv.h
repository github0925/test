//*****************************************************************************
//
// wdg_drv.h - Prototypes for the Watchdog Timer API
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __DW_WDG_H__
#define __DW_WDG_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif
#include <platform/interrupts.h>
#include <lib/reg.h>

#define LOCAL_TRACE 0
//*****************************************************************************
//
// This section defines the log level and other define
// WATCHDOG component
//
//*****************************************************************************
#define DEFAULT_WATCHOUT_LOG_LEVEL  1
#define DEFAULT_WDG_MAGIC  'wdog'

//*****************************************************************************
//
// This section defines io
// WDG component
//
//*****************************************************************************
#ifdef __cplusplus
  #define   __I     volatile    /*!< Defines 'read only' permissions                 */
#else
  #define   __I     volatile const   /*!< Defines 'read only' permissions                 */
#endif
#define __O         volatile             /*!< Defines 'write only' permissions                */
#define __IO        volatile             /*!< Defines 'read / write' permissions              */

//*****************************************************************************
//
// This section defines the watchdog number
// WDG component
//
//*****************************************************************************

/*watchdog really number.*/
typedef enum _wdg_really_num
{
    wdg_really_num1 = 0x1U,
    wdg_really_num2 = 0x2U,
    wdg_really_num3 = 0x3U,
    wdg_really_num4 = 0x4U,
    wdg_really_num5 = 0x5U,
    wdg_really_num6 = 0x6U,
    wdg_really_num7 = 0x7U,
    wdg_really_num8 = 0x8U,
    wdg_really_num_max = 0x8U,
} wdg_really_num_t;
//*****************************************************************************
//
// This section defines the fwatchdog int
// WDG component
// reference irq_v_0_5_3.h
//*****************************************************************************
#define WDT_IRQ_NUM_BASE            32
#define wdt_real_irq_num(n)         ((n) + WDT_IRQ_NUM_BASE)

#define WDG1_ILL_WIN_REFR_INT_NUM   wdt_real_irq_num(187)//IRQ_GIC1_WDG1_ILL_WIN_REFR_INT_NUM
#define WDG1_ILL_SEQ_REFR_INT_NUM   wdt_real_irq_num(187)//IRQ_GIC4_WDG1_ILL_SEQ_REFR_INT_NUM
#define WDG1_OVFLOW_INT_NUM         wdt_real_irq_num(188)//IRQ_GIC4_WDG1_OVFLOW_INT_NUM

#define WDG2_ILL_WIN_REFR_INT_NUM   wdt_real_irq_num(23)//IRQ_VDSP_WDG2_ILL_WIN_REFR_INT_NUM
#define WDG2_ILL_SEQ_REFR_INT_NUM   wdt_real_irq_num(23)//IRQ_VDSP_WDG2_ILL_SEQ_REFR_INT_NUM
#define WDG2_OVFLOW_INT_NUM         wdt_real_irq_num(23)//IRQ_VDSP_WDG2_OVFLOW_INT_NUM

#define WDG3_ILL_WIN_REFR_INT_NUM   wdt_real_irq_num(189)//IRQ_GIC2_WDG3_ILL_WIN_REFR_INT_NUM
#define WDG3_ILL_SEQ_REFR_INT_NUM   wdt_real_irq_num(189)//IRQ_GIC2_WDG3ILL_SEQ_REFR_INT_NUM
#define WDG3_OVFLOW_INT_NUM         wdt_real_irq_num(190)//IRQ_GIC2_WDG3_OVFLOW_INT_NUM

#define WDG4_ILL_WIN_REFR_INT_NUM   wdt_real_irq_num(189)//Q_GIC3_WDG4_ILL_WIN_REFR_INT_NUM
#define WDG4_ILL_SEQ_REFR_INT_NUM   wdt_real_irq_num(189)//IRQ_GIC3_WDG4_ILL_SEQ_REFR_INT_NUM
#define WDG4_OVFLOW_INT_NUM         wdt_real_irq_num(190)//IRQ_GIC3_WDG4_OVFLOW_INT_NUM 222

#define WDG5_ILL_WIN_REFR_INT_NUM   wdt_real_irq_num(189)//Q_GIC4_WDG5_ILL_WIN_REFR_INT_NUM
#define WDG5_ILL_SEQ_REFR_INT_NUM   wdt_real_irq_num(189)//IRQ_GIC4_WDG5_ILL_SEQ_REFR_INT_NUM
#define WDG5_OVFLOW_INT_NUM         wdt_real_irq_num(190)//IRQ_GIC4_WDG5_OVFLOW_INT_NUM 222

#define WDG6_ILL_WIN_REFR_INT_NUM   wdt_real_irq_num(189)//Q_GIC5_WDG6_ILL_WIN_REFR_INT_NUM
#define WDG6_ILL_SEQ_REFR_INT_NUM   wdt_real_irq_num(189)//IRQ_GIC5_WDG6_ILL_SEQ_REFR_INT_NUM
#define WDG6_OVFLOW_INT_NUM         wdt_real_irq_num(190)//IRQ_GIC5_WDG6_OVFLOW_INT_NUM 222

#define WDG7_ILL_WIN_REFR_INT_NUM   wdt_real_irq_num(27)//IRQ_ADSP_WDG7_ILL_WIN_REFR_INT_NUM
#define WDG7_ILL_SEQ_REFR_INT_NUM   wdt_real_irq_num(27)//IRQ_ADSP_WDG7_ILL_SEQ_REFR_INT_NUM
#define WDG7_OVFLOW_INT_NUM         wdt_real_irq_num(27)//IRQ_ADSP_WDG7_OVFLOW_INT_NUM

#define WDG8_ILL_WIN_REFR_INT_NUM   wdt_real_irq_num(191)//IRQ_GIC4_WDG8_ILL_WIN_REFR_INT_NUM
#define WDG8_ILL_SEQ_REFR_INT_NUM   wdt_real_irq_num(191)//IRQ_GIC4_WDG8_ILL_SEQ_REFR_INT_NUM
#define WDG8_OVFLOW_INT_NUM         wdt_real_irq_num(192)//IRQ_GIC4_WDG8_OVFLOW_INT_NUM
//*****************************************************************************
//
// This section defines the register offsets of
// WDG component
//
//*****************************************************************************
// Watchdog global control register
#define WDG_CTRL    0x00000000
// Watchdog terminal count value
#define WDG_WTC 0x00000004
// Watchdog refresh control
#define WDG_WRC_CTL 0x00000008
// Watchdog Refresh window limit
#define WDG_WRC_VAL 0x0000000C
// Watchdog refresh sequence delta
#define WDG_WRC_SEQ 0x00000010
// Watchdog reset control
#define WDG_RST_CTL 0x00000014
// Watchdog external reset control
#define WDG_EXT_RST_CTL 0x00000018
// Watchdog counter
#define WDG_CNT     0x0000001C
// Watchdog timestamp
#define WDG_TWS 0x00000020
// Watchdog interrupt
#define WDG_INT     0x00000024
// Watchdog lock
#define WDG_LOCK    0x00000040

//*****************************************************************************
//
// This section defines all mask
//
//*****************************************************************************
//wdg ctrl
#define WDG_CTRL_SOFT_RST_MASK      ((uint32_t) (1 << 0))
#define WDG_CTRL_SOFT_RST_SHIFT                (0U)
#define WDG_CTRL_SOFT_RST(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_CTRL_SOFT_RST_SHIFT)) & WDG_CTRL_SOFT_RST_MASK)

#define WDG_CTRL_WDG_EN_MASK        ((uint32_t) (1 << 1))
#define WDG_CTRL_WDG_EN_SHIFT                (1U)
#define WDG_CTRL_WDG_EN(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_CTRL_WDG_EN_SHIFT)) & WDG_CTRL_WDG_EN_MASK)

#define WDG_CTRL_CLK_SRC_MASK       (((uint32_t) (1 << 2)) | ((uint32_t) (1 << 3)) |((uint32_t) (1 << 4)))
#define WDG_CTRL_CLK_SRC_SHIFT                (2U)
#define WDG_CTRL_CLK_SRC(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_CTRL_CLK_SRC_SHIFT)) & WDG_CTRL_CLK_SRC_MASK)

#define WDG_CTRL_WTC_SRC_MASK       ((uint32_t) (1 << 5))
#define WDG_CTRL_WTC_SRC_SHIFT                (5U)
#define WDG_CTRL_WTC_SRC(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_CTRL_WTC_SRC_SHIFT)) & WDG_CTRL_WTC_SRC_MASK)

#define WDG_CTRL_AUTO_RESTART_MASK      ((uint32_t) (1 << 6))
#define WDG_CTRL_AUTO_RESTART_SHIFT                (6U)
#define WDG_CTRL_AUTO_RESTART(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_CTRL_AUTO_RESTART_SHIFT)) & WDG_CTRL_AUTO_RESTART_MASK)

#define WDG_CTRL_DBG_HALT_EN_MASK       ((uint32_t) (1 << 7))
#define WDG_CTRL_DBG_HALT_EN_SHIFT              (7U)
#define WDG_CTRL_DBG_HALT_EN(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_CTRL_DBG_HALT_EN_SHIFT)) & WDG_CTRL_DBG_HALT_EN_MASK)

#define WDG_CTRL_WDG_EN_SRC_MASK        ((uint32_t) (1 << 8))
#define WDG_CTRL_WDG_EN_SRC_SHIFT                (8U)
#define WDG_CTRL_WDG_EN_SRC(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_CTRL_WDG_EN_SRC_SHIFT)) & WDG_CTRL_WDG_EN_SRC_MASK)

#define WDG_CTRL_SELFTEST_TRIG_MASK     ((uint32_t) (1 << 9))
#define WDG_CTRL_SELFTEST_TRIG_SHIFT                (9U)
#define WDG_CTRL_SELFTEST_TRIG(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_CTRL_SELFTEST_TRIG_SHIFT)) & WDG_CTRL_SELFTEST_TRIG_MASK)

#define WDG_CTRL_WDG_EN_STA_MASK        ((uint32_t) (1 << 10))
#define WDG_CTRL_WDG_EN_STA_SHIFT                (10U)
#define WDG_CTRL_WDG_EN_STA(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_CTRL_WDG_EN_STA_SHIFT)) & WDG_CTRL_WDG_EN_STA_MASK)

#define WDG_CTRL_PRE_DIV_NUM_MASK       0xFFFF0000
#define WDG_CTRL_PRE_DIV_NUM_SHIFT                (16U)
#define WDG_CTRL_PRE_DIV_NUM(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_CTRL_PRE_DIV_NUM_SHIFT)) & WDG_CTRL_PRE_DIV_NUM_MASK)
//wdg wtc
#define WDG_WTC_MASK        0xffffffff
//wdg wrc ctl
#define WDG_WRC_CTRL_MODEM0_MASK        ((uint32_t) (1 << 0))
#define WDG_WRC_CTRL_MODEM0_SHIFT                (0U)
#define WDG_WRC_CTRL_MODEM0(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_WRC_CTRL_MODEM0_SHIFT)) & WDG_WRC_CTRL_MODEM0_MASK)

#define WDG_WRC_CTRL_MODEM1_MASK        ((uint32_t) (1 << 1))
#define WDG_WRC_CTRL_MODEM1_SHIFT                (1U)
#define WDG_WRC_CTRL_MODEM1(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_WRC_CTRL_MODEM1_SHIFT)) & WDG_WRC_CTRL_MODEM1_MASK)

#define WDG_WRC_CTRL_SEQ_REFR_MASK      ((uint32_t) (1 << 2))
#define WDG_WRC_CTRL_SEQ_REFR_SHIFT                (2U)
#define WDG_WRC_CTRL_SEQ_REFR(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_WRC_CTRL_SEQ_REFR_SHIFT)) & WDG_WRC_CTRL_SEQ_REFR_MASK)

#define WDG_WRC_CTRL_REFR_TRIG_MASK     ((uint32_t) (1 << 3))
#define WDG_WRC_CTRL_REFR_TRIG_SHIFT                (3U)
#define WDG_WRC_CTRL_REFR_TRIG(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_WRC_CTRL_REFR_TRIG_SHIFT)) & WDG_WRC_CTRL_REFR_TRIG_MASK)
//wdg wrc value
#define WDG_WRC_MASK        0xffffffff
//wdg wrc seq value
#define WDG_WRC_SEQ_MASK        0xffffffff
//wdg rst ctl
#define WDG_RST_CTRL_RST_CNT_MASK       0x0000ffff
#define WDG_RST_CTRL_RST_CNT_SHIFT                (0U)
#define WDG_RST_CTRL_RST_CNT(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_RST_CTRL_RST_CNT_SHIFT)) & WDG_RST_CTRL_RST_CNT_MASK)
#define WDG_RST_CTRL_INT_RST_EN_MASK        ((uint32_t) (1 << 16))
#define WDG_RST_CTRL_INT_RST_EN_SHIFT                (16U)
#define WDG_RST_CTRL_INT_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_RST_CTRL_INT_RST_EN_SHIFT)) & WDG_RST_CTRL_INT_RST_EN_MASK)

#define WDG_RST_CTRL_INT_RST_MODE_MASK      ((uint32_t) (1 << 17))
#define WDG_RST_CTRL_INT_RST_MODE_SHIFT                (17U)
#define WDG_RST_CTRL_INT_RST_MODE(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_RST_CTRL_INT_RST_MODE_SHIFT)) & WDG_RST_CTRL_INT_RST_MODE_MASK)

#define WDG_RST_CTRL_WDG_RST_EN_MASK        ((uint32_t) (1 << 18))
#define WDG_RST_CTRL_WDG_RST_EN_SHIFT                (18U)
#define WDG_RST_CTRL_WDG_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_RST_CTRL_WDG_RST_EN_SHIFT)) & WDG_RST_CTRL_WDG_RST_EN_MASK)

#define WDG_RST_CTRL_RST_WIN_MASK       0x0ff00000
#define WDG_RST_CTRL_RST_WIN_SHIFT                (20U)
#define WDG_RST_CTRL_RST_WIN(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_RST_CTRL_RST_WIN_SHIFT)) & WDG_RST_CTRL_RST_WIN_MASK)

//wdg ext rst ctl
#define WDG_EXT_RST_CTRL_RST_CNT_MASK       0x0000ffff
#define WDG_EXT_RST_CTRL_RST_CNT_SHIFT                (0U)
#define WDG_EXT_RST_CTRL_RST_CNT(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_EXT_RST_CTRL_RST_CNT_SHIFT)) & WDG_EXT_RST_CTRL_RST_CNT_MASK)

#define WDG_EXT_RST_CTRL_INT_RST_EN_MASK        ((uint32_t) (1 << 16))
#define WDG_EXT_RST_CTRL_INT_RST_EN_SHIFT                (16U)
#define WDG_EXT_RST_CTRL_INT_RST_EN(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_EXT_RST_CTRL_INT_RST_EN_SHIFT)) & WDG_EXT_RST_CTRL_INT_RST_EN_MASK)

#define WDG_EXT_RST_CTRL_INT_RST_MODE_MASK      ((uint32_t) (1 << 17))
#define WDG_EXT_RST_CTRL_INT_RST_MODE_SHIFT                (17U)
#define WDG_EXT_RST_CTRL_INT_RST_MODE(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_EXT_RST_CTRL_INT_RST_MODE_SHIFT)) & WDG_EXT_RST_CTRL_INT_RST_MODE_MASK)

#define WDG_EXT_RST_CTRL_RST_WIN_MASK       0x0ff00000
#define WDG_EXT_RST_CTRL_RST_WIN_SHIFT                (20U)
#define WDG_EXT_RST_CTRL_RST_WIN(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_EXT_RST_CTRL_RST_WIN_SHIFT)) & WDG_EXT_RST_CTRL_RST_WIN_MASK)

//wdg int
#define WDG_INT_ILL_WIN_REFE_INT_EN_MASK        ((uint32_t) (1 << 0))
#define WDG_INT_ILL_WIN_REFE_INT_EN_SHIFT                (0U)
#define WDG_INT_ILL_WIN_REFE_INT_EN(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_INT_ILL_WIN_REFE_INT_EN_SHIFT)) & WDG_INT_ILL_WIN_REFE_INT_EN_MASK)

#define WDG_INT_ILL_SEQ_REFE_INT_EN_MASK        ((uint32_t) (1 << 1))
#define WDG_INT_ILL_SEQ_REFE_INT_EN_SHIFT                (1U)
#define WDG_INT_ILL_SEQ_REFE_INT_EN(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_INT_ILL_SEQ_REFE_INT_EN_SHIFT)) & WDG_INT_ILL_SEQ_REFE_INT_EN_MASK)

#define WDG_INT_OVERFLOW_INT_EN_MASK        ((uint32_t) (1 << 2))
#define WDG_INT_OVERFLOW_INT_EN_SHIFT                (2U)
#define WDG_INT_OVERFLOW_INT_EN(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_INT_OVERFLOW_INT_EN_SHIFT)) & WDG_INT_OVERFLOW_INT_EN_MASK)

#define WDG_INT_ILL_WIN_REFE_INT_STA_MASK       ((uint32_t) (1 << 3))
#define WDG_INT_ILL_WIN_REFE_INT_STA_SHIFT                (3U)
#define WDG_INT_ILL_WIN_REFE_INT_STA(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_INT_ILL_WIN_REFE_INT_STA_SHIFT)) & WDG_INT_ILL_WIN_REFE_INT_STA_MASK)

#define WDG_INT_ILL_SEQ_REFE_INT_STA_MASK       ((uint32_t) (1 << 4))
#define WDG_INT_ILL_SEQ_REFE_INT_STA_SHIFT                (4U)
#define WDG_INT_ILL_SEQ_REFE_INT_STA(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_INT_ILL_SEQ_REFE_INT_STA_SHIFT)) & WDG_INT_ILL_SEQ_REFE_INT_STA_MASK)

#define WDG_INT_OVERFLOW_INT_STA_MASK       ((uint32_t) (1 << 5))
#define WDG_INT_OVERFLOW_INT_STA_SHIFT                (5U)
#define WDG_INT_OVERFLOW_INT_STA(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_INT_OVERFLOW_INT_STA_SHIFT)) & WDG_INT_OVERFLOW_INT_STA_MASK)

#define WDG_INT_ILL_WIN_REFE_INT_CLR_MASK       ((uint32_t) (1 << 6))
#define WDG_INT_ILL_WIN_REFE_INT_CLR_SHIFT                (6U)
#define WDG_INT_ILL_WIN_REFE_INT_CLR(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_INT_ILL_WIN_REFE_INT_CLR_SHIFT)) & WDG_INT_ILL_WIN_REFE_INT_CLR_MASK)

#define WDG_INT_ILL_SEQ_REFE_INT_CLR_MASK       ((uint32_t) (1 << 7))
#define WDG_INT_ILL_SEQ_REFE_INT_CLR_SHIFT                (7U)
#define WDG_INT_ILL_SEQ_REFE_INT_CLR(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_INT_ILL_SEQ_REFE_INT_CLR_SHIFT)) & WDG_INT_ILL_SEQ_REFE_INT_CLR_MASK)

#define WDG_INT_OVERFLOW_INT_CLR_MASK       ((uint32_t) (1 << 8))
#define WDG_INT_OVERFLOW_INT_CLR_SHIFT                (8U)
#define WDG_INT_OVERFLOW_INT_CLR(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_INT_OVERFLOW_INT_CLR_SHIFT)) & WDG_INT_OVERFLOW_INT_CLR_MASK)

//wdg lock
#define WDG_LOCK_CTL_LOCK_MASK      ((uint32_t) (1 << 0))
#define WDG_LOCK_CTL_LOCK_SHIFT                (0U)
#define WDG_LOCK_CTL_LOCK(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_LOCK_CTL_LOCK_SHIFT)) & WDG_LOCK_CTL_LOCK_MASK)

#define WDG_LOCK_WTC_LOCK_MASK      ((uint32_t) (1 << 1))
#define WDG_LOCK_WTC_LOCK_SHIFT                (1U)
#define WDG_LOCK_WTC_LOCK(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_LOCK_WTC_LOCK_SHIFT)) & WDG_LOCK_WTC_LOCK_MASK)

#define WDG_LOCK_WRC_LOCK_MASK      ((uint32_t) (1 << 2))
#define WDG_LOCK_WRC_LOCK_SHIFT                (2U)
#define WDG_LOCK_WRC_LOCK(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_LOCK_WRC_LOCK_SHIFT)) & WDG_LOCK_WRC_LOCK_MASK)

#define WDG_LOCK_RST_LOCK_MASK      ((uint32_t) (1 << 3))
#define WDG_LOCK_RST_LOCK_SHIFT                (3U)
#define WDG_LOCK_RST_LOCK(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_LOCK_RST_LOCK_SHIFT)) & WDG_LOCK_RST_LOCK_MASK)

#define WDG_LOCK_EXT_RST_LOCK_MASK      ((uint32_t) (1 << 4))
#define WDG_LOCK_EXT_RST_LOCK_SHIFT                (4U)
#define WDG_LOCK_EXT_RST_LOCK(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_LOCK_EXT_RST_LOCK_SHIFT)) & WDG_LOCK_EXT_RST_LOCK_MASK)

#define WDG_LOCK_INT_LOCK_MASK      ((uint32_t) (1 << 5))
#define WDG_LOCK_INT_LOCK_SHIFT                (5U)
#define WDG_LOCK_INT_LOCK(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_LOCK_INT_LOCK_SHIFT)) & WDG_LOCK_INT_LOCK_MASK)

#define WDG_LOCK_CLK_SRC_LOCK_MASK      ((uint32_t) (1 << 6))
#define WDG_LOCK_CLK_SRC_LOCK_SHIFT                (6U)
#define WDG_LOCK_CLK_SRC_LOCK(x)                   (((uint32_t)(((uint32_t)(x)) << WDG_LOCK_CLK_SRC_LOCK_SHIFT)) & WDG_LOCK_CLK_SRC_LOCK_MASK)


//*****************************************************************************
//
// This section defines the default
// WDG component
//
//*****************************************************************************
#define DEFAULT_WATCHOUT_TIMEOUT_TIME   1000  // ms
#define DEFAULT_WATCHOUT_WTC_MAX    0xffffffff  // wdg wtc max count and you can modify it for need bit0~bit31
#define DEFAULT_WATCHOUT_DIV_MAX    0x0000ffff  // wdg wtc max count and you can modify it for need bit16~bit31
#define DEFAULT_WATCHOUT_WIN_LOW_LIMIT  0xffffffff  // wdg win low limit bit0~bit31s
#define DEFAULT_WATCHOUT_SEQ_DELTA  0xffffffff  // wdg win low limit bit0~bit31s
#define DEFAULT_WATCHOUT_TIMESTAMP  0xffffffff  // wdg win low limit bit0~bit31s

/*watchdog clock source. */
#define MHZ (1000 * 1000)
#define KHZ 1000
// main clk
#define WDG_MAIN_CLK        24*MHZ
// bus clk
#define WDG_BUS_CLK 250*MHZ
// ext clk
#define WDG_EXT_CLK 26*MHZ
// tie off
#define WDG_TIE_OFF 0
// lp clk 32khz xtal
#define WDG_LP_CLK  32*KHZ

#define WDG1_BASE    ((wdg_reg_type_t *)_ioaddr(APB_WDT1_BASE))
#define WDG2_BASE    ((wdg_reg_type_t *)_ioaddr(APB_WDT2_BASE))
#define WDG3_BASE    ((wdg_reg_type_t *)_ioaddr(APB_WDT3_BASE))
#define WDG4_BASE    ((wdg_reg_type_t *)_ioaddr(APB_WDT4_BASE))
#define WDG5_BASE    ((wdg_reg_type_t *)_ioaddr(APB_WDT5_BASE))
#define WDG6_BASE    ((wdg_reg_type_t *)_ioaddr(APB_WDT6_BASE))
#define WDG7_BASE    ((wdg_reg_type_t *)_ioaddr(APB_WDT7_BASE))
#define WDG8_BASE    ((wdg_reg_type_t *)_ioaddr(APB_WDT8_BASE))

 /* This structure contains the wdg status flags for use in the wdg functions.*/
/*wdg enable status is bit10*/
typedef enum _wdg_status_flags
{
    wdg_enabled_status_flag = WDG_CTRL_WDG_EN_STA_MASK, /*!< Running flag, set when WDG is enabled*/
    wdg_ill_win_refr_int_status_flag = WDG_INT_ILL_WIN_REFE_INT_STA_MASK, /*!< Interrupt flag, set when an exception occurs*/
    wdg_ill_seq_refr_int_status_flag = WDG_INT_ILL_SEQ_REFE_INT_STA_MASK,
    wdg_overflow_int_status_flag = WDG_INT_OVERFLOW_INT_STA_MASK,
}wdg_status_flags;

/*This section defines the fresh mode select*/
typedef enum _wdg_mechanism_mode
{
    wdg_mechanism_mode1 = 0x1U, /*!< normal refresh mode */
    wdg_mechanism_mode2 = 0x2U, /*!< window limit refresh mode */
    wdg_mechanism_mode3 = 0x3U, /*!< seq delta refresh mode */
    wdg_mechanism_mode_max = 0x3U, /*!< seq delta refresh mode */
} wdg_mechanism_mode;

#if 0
/*watchdog clock prescaler. bit16~bit31*/
typedef enum _wdg_clock_prescaler
{
    wdg_clock_prescaler_div1 = 0x0U, /*!< div by 1 */
    wdg_clock_prescaler_div2 = 0x1U, /*!< div by 2 */
    wdg_clock_prescaler_div3 = 0x2U, /*!< div by 3 */
    wdg_clock_prescaler_div4 = 0x3U, /*!< div by 4 */
    wdg_clock_prescaler_div5 = 0x4U, /*!< div by 5 */
    wdg_clock_prescaler_div6 = 0x5U, /*!< div by 6 */
    wdg_clock_prescaler_div7 = 0x6U, /*!< div by 7 */
    wdg_clock_prescaler_div8 = 0x7U, /*!< div by 8 */
} wdg_clock_prescaler;
#endif

typedef enum _wdg_clock_source
{
    wdg_main_clk = 0x0U,  // main clk
    wdg_bus_clk = 0x1U, // bus clk
    wdg_ext_clk = 0x2U, // bus clk
    wdg_tie_off = 0x3U,// tie off
    wdg_lp_clk = 0x4U,// lp clk 32khz xtal
    wdg_max_clk_num = 0x5U,// lp clk 32khz xtal
} wdg_clock_source;

typedef struct _wdg_reg_type
{
  __IO uint32_t wdg_ctrl;//Watchdog global control register 0x00000000
  __IO uint32_t wdg_wtc;// Watchdog terminal count value    0x00000004
  __IO uint32_t wdg_wrc_ctl;// Watchdog refresh control 0x00000008
  __IO uint32_t wdg_wrc_val;// Watchdog Refresh window limit    0x0000000C
  __IO uint32_t wdg_wrc_seq;// Watchdog refresh sequence delta  0x00000010
  __IO uint32_t wdg_rst_ctl;// Watchdog reset control   0x00000014
  __IO uint32_t wdg_ext_rst_ctl;// Watchdog external reset control 0x00000018
  __IO uint32_t wdg_cnt;// Watchdog counter 0x0000001C
  __IO uint32_t wdg_tsw;// Watchdog timestamp 0x00000020
  __IO uint32_t wdg_int;// Watchdog interrupt 0x00000024
  __IO uint32_t nulldata[7];// NULL
  __IO uint32_t wdg_lock;//// Watchdog lock 0x00000040
} wdg_reg_type_t;

/*! watchdog configuration structure. */
typedef struct _wdg_ctrl_cfg
{
    bool enableSoftRest;    /*watchdog softreset,can auto clear bit0*/
    bool enableWdg;     /*enables or disables watchdog bit1*/
    wdg_clock_source clockSource;  /*clock source select bit2~bit4*/
    uint8_t terminalCountSrc;   /*watchdog terminal count value source*/
                                            /*1:from register wdg WTC;0:from fuse ro soc integration bit5*/
    bool enableAutostart;   /*watchdog overflow auto restart enable bit6*/
    bool enableDebugmode;   /*watchdog halt enable in debug mode bit7*/
    uint8_t enableSrcSelect; /*watchdog module enable source select 1:from register wdg enable:0:from fuse bit8*/
    bool enableSelftest;    /*watchdog selftest mode enable bit9*/
    uint16_t prescaler; /*clock prescaler value bit16~bit31*/
} wdg_ctrl_cfg_t;

/*! watchdog WRC CTRL for refresh control register. */
typedef struct _wdg_refresh_cfg
{
    uint8_t wdgModeSelect;/*refresh mode select bit0~bit1*/
    bool enableSeqRefresh;/*sequence based WDG refresh enable bit2*/
    bool enableRefreshTrig;/*refresh trigger,suto clear*/
} wdg_refresh_cfg_t;

/*! watchdog for reset control register. */
typedef struct _wdg_reset_cfg
{
    uint16_t wdgResetCnt;/*watchdog reset count bit0~bit15*/
    bool enableSysReset;/*internal system reset enable bit16*/
    uint8_t SysRstMode;/*internal system reset request mode bit17 1:pluse mode;0:level mode*/
    bool enableWdgResetEn;/*internal system reset restart wdg enable bit18*/
    uint8_t plusRstWind;/*plus mode reset windows bit20~bit27*/
} wdg_reset_cfg_t;

/*! watchdog for ext reset control register. */
typedef struct _wdg_ext_reset_cfg
{
    uint16_t wdgResetCnt;/*watchdog reset count bit0~bit15*/
    bool enableSysExtReset;/*internal system reset enable bit16*/
    uint8_t SysExtRstMode;/*internal system reset request mode bit17 1:pluse mode;0:level mode*/
    uint8_t plusRstWind;/*plus mode reset windows bit20~bit27*/
} wdg_ext_reset_cfg_t;

/*! watchdog for int control register. */
typedef struct _wdg_int_cfg
{
    bool    ill_win_refr_int_en;
    bool    ill_seq_refr_int_en;
    bool    overflow_int_en;
    bool    ill_win_refr_int_clr;
    bool    ill_seq_refr_int_clr;
    bool    overflow_int_clr;
} wdg_int_cfg_t;

/*! watchdog for lock control register. */
typedef struct _wdg_lock_cfg
{
    bool    ctl_lock;   /*lock for wtc control register*/
    bool    wtc_lock;   /*lock for wtc register*/
    bool    wrc_lock;   /*lock for refresh register*/
    bool    rst_lock;   /*lock for reset register*/
    bool    ext_rst_lock;   /*lock for ext reset register*/
    bool    int_lock;   /*lock for wtc interrupt register*/
    bool    clk_src_lock;   /*lock for clk_src in WDG_CTRL  register*/
} wdg_lock_cfg_t;

/*! watchdog configuration structure. */
typedef struct _wdg_config
{
    wdg_ctrl_cfg_t  wdg_ctrl_config;
    uint32_t    wdg_timeout; /*watchdog WTC*/
    wdg_refresh_cfg_t   wdg_refresh_config;
    uint32_t    refresh_wind_limit; /*watchdog refresh windows limit*/
    uint32_t    refresh_seq_delta; /*watchdog refresh sequence delta*/
    wdg_reset_cfg_t wdg_reset_cfg;
    wdg_ext_reset_cfg_t wdg_ext_reset_cfg;
    uint32_t    wdg_tsw;
    wdg_int_cfg_t wdg_int_cfg;
    wdg_lock_cfg_t wdg_lock_cfg;
} wdg_config_t;

// Check the arguments.
#define ASSERT_PARAMETER(base)  \
if((base != WDG1_BASE)                  \
    && (base != WDG2_BASE)              \
    && (base != WDG3_BASE)              \
    && (base != WDG4_BASE)              \
    && (base != WDG5_BASE)              \
    && (base != WDG6_BASE)              \
    && (base != WDG7_BASE)              \
    && (base != WDG8_BASE))             \
{                                                       \
    LTRACEF("base paramenter error \n");    \
    return false;                                       \
}                                                       \
//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
void wdg_get_default_config(wdg_config_t *wdg_config);
bool wdg_set_timeout(wdg_reg_type_t *base,uint32_t timeout_ms);
bool wdg_set_window_limit(wdg_reg_type_t *base,uint32_t window_timeout_ms);
bool wdg_set_seq_delta(wdg_reg_type_t *base,uint32_t seq_delta_timeout_ms);
bool wdg_refesh_mechanism_select(wdg_reg_type_t *base,const wdg_config_t *wdg_config);
uint32_t wdg_get_refesh_mechanism(wdg_reg_type_t *base);
bool wdg_init(wdg_reg_type_t *base,const wdg_config_t *wdg_config);
bool wdg_deInit(wdg_reg_type_t *base);
bool wdg_set_testmode_config(wdg_reg_type_t *base, const wdg_config_t *wdg_config);
bool wdg_enable(wdg_reg_type_t *base);
bool wdg_disable(wdg_reg_type_t *base);
bool wdg_enable_Interrupts(wdg_reg_type_t *base);
bool wdg_disable_Interrupts(wdg_reg_type_t *base);
uint32_t wdg_get_status_flag(wdg_reg_type_t *base);
bool wdg_clear_status_flag(wdg_reg_type_t *base,uint32_t mask);
bool wdg_refresh(wdg_reg_type_t *base);
uint32_t wdg_get_reset_cnt(wdg_reg_type_t *base);
bool wdg_set_ext_reset_cnt(wdg_reg_type_t *base,uint32_t rst_delay);
uint32_t wdg_get_ext_reset_cnt(wdg_reg_type_t *base);
bool wdg_clear_reset_cnt(wdg_reg_type_t *base);
bool wdg_set_reset_cnt(wdg_reg_type_t *base,uint32_t rst_delay);
bool wdg_clear_ext_reset_cnt(wdg_reg_type_t *base);
bool wdg_Int_register(void *handle,wdg_reg_type_t *base,int_handler call_func,bool overflow_int);
bool wdg_int_unregister(wdg_reg_type_t *base,bool overflow_int);
bool wdg_int_clear(wdg_reg_type_t *base);
bool wdg_halt_enable(wdg_reg_type_t *base);
bool wdg_halt_disable(wdg_reg_type_t *base);
bool wdg_record_wdgcnt(wdg_reg_type_t *base);
bool wdg_delay_timeout(wdg_reg_type_t *base,uint32_t timeout_ms);
uint32_t wdg_get_cnt(wdg_reg_type_t *base);
bool wdg_set_reset(wdg_reg_type_t *base,const wdg_config_t* wdg_config);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __DW_WDG_H__
