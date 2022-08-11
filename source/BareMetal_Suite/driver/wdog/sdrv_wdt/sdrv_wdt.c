/********************************************************
 *          Copyright(c) 2019   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <wdog/wdog.h>
#include <srv_timer/srv_timer.h>
#include "wdt_reg.h"
#include "soc.h"

BOOL sdrv_wdt_is_enabled(uintptr_t base)
{
    wdt_t *w = (wdt_t *) base;
    U32 v = w->wdt_ctrl;

    if (v & BM_WDT_CTRL_WDT_EN_SRC) {
        return ((w->wdt_ctrl & BM_WDT_CTRL_WDT_EN) != 0);
    } else {
        return soc_wdt_is_hw_en(base);
    }
}

void sdrv_wdt_cfg(uintptr_t base, U32 tmo_tick)
{
    wdt_t *w = (wdt_t *) base;

    w->wdt_ctrl |= BM_WDT_CTRL_WDT_EN_SRC;
    w->wdt_ctrl &= ~BM_WDT_CTRL_WDT_EN;

    while (w->wdt_ctrl & BM_WDT_CTRL_WDT_EN_STA);

    /* clk sourced from main clock (24MHz RC), div24, auto-restart,
     * wdt from register, enable from register WDT_EN */
    /* if wdt_en/wtc_src fuse blown, settings will be from fuse */
#define WDT_CTRL_CFG_VAL    \
            ( BM_WDT_CTRL_AUTO_RESTART  | \
              BM_WDT_CTRL_WTC_SRC |   \
              FV_WDT_CTRL_CLK_SRC(0) |  \
              FV_WDT_CTRL_PRE_DIV_NUM(23))

    w->wdt_ctrl = WDT_CTRL_CFG_VAL;

    if (0 == tmo_tick) tmo_tick = 1;

    w->wdt_wtc = tmo_tick;

    /* mode0 (refresh without any condition) */
    w->wdt_wrc_ctl = BM_WDT_WRC_CTL_MODE0_EN;

    /* pulse mode (expected by rstgen), internal system reset request enable */
    w->wdt_rst_ctl = BM_WDT_RST_CTL_INT_RST_MODE | BM_WDT_RST_CTL_INT_RST_EN
                     /* a non-zero num needed here otherwise int_rst
                      * assert once int_rst_en asserted*/
                     | FV_WDT_RST_CTL_RST_CNT(0xF);

    /* wdt_en to enable wdt (also gate on wdt_clk) */
    w->wdt_ctrl |= BM_WDT_CTRL_WDT_EN | BM_WDT_CTRL_WDT_EN_SRC;

    /* 3 clks needed until wdt_en settle down */
    while (!(w->wdt_ctrl & BM_WDT_CTRL_WDT_EN_STA));

    /* reset wdt clock domain so we always start at a known point. */
    /* assertion of soft reset will reset things in WDT clock domain ( the apb
     * part will not be reset, including wdt_en, the status bits will be reset
     * since these bits come from WDT clock domain.) */
    /* one wdt clock needed to finish the resetting */
    w->wdt_ctrl |= BM_WDT_CTRL_SOFT_RST;    /* a self-clear bit */

    /* 3 wdt clock to wait soft reset done
     * by default, wdt feed by 24MHz, thus 1 us is enough*/
    while (w->wdt_ctrl & BM_WDT_CTRL_SOFT_RST);
}

void sdrv_wdt_enable(uintptr_t base)
{
    wdt_t *w = (wdt_t *) base;

    w->wdt_ctrl |= BM_WDT_CTRL_WDT_EN | BM_WDT_CTRL_WDT_EN_SRC;

    while (!(w->wdt_ctrl & BM_WDT_CTRL_WDT_EN_STA));
}

void sdrv_wdt_disable(uintptr_t base)
{
    wdt_t *w = (wdt_t *) base;

    w->wdt_ctrl |= BM_WDT_CTRL_WDT_EN_SRC;
    w->wdt_ctrl &= ~BM_WDT_CTRL_WDT_EN;

    while (w->wdt_ctrl & BM_WDT_CTRL_WDT_EN_STA);
}

void sdrv_wdt_refresh(uintptr_t base)
{
    wdt_t *w = (wdt_t *) base;

    if (w->wdt_ctrl & BM_WDT_CTRL_WDT_EN) {
        if (w->wdt_wrc_ctl & BM_WDT_WRC_CTL_MODE0_EN) {
            w->wdt_wrc_ctl |= BM_WDT_WRC_CTL_REFR_TRIG;
        } else {
            DBG("%s: wdt driver so far only support mode0\n", __FUNCTION__);
        }
    } else {
        DBG("%s: opps, wdt(@0x%x) not enabled yet.\n", __FUNCTION__, (U32)base);
    }
}

void rf_sdrv_wdt_refresh_if_en(uintptr_t base) __RAM_FUNC__;
void rf_sdrv_wdt_refresh_if_en(uintptr_t base)
{
    wdt_t *w = (wdt_t *) base;

    if (w->wdt_ctrl & BM_WDT_CTRL_WDT_EN) {
        if (w->wdt_wrc_ctl & BM_WDT_WRC_CTL_MODE0_EN) {
            w->wdt_wrc_ctl |= BM_WDT_WRC_CTL_REFR_TRIG;
        }
    }
}

U32 sdrv_wdt_get_cnt(uintptr_t base)
{
    wdt_t *w = (wdt_t *) base;

    return w->wdt_cnt;
}

U32 sdrv_wdt_get_tmo_us(uintptr_t base)
{
    wdt_t *w = (wdt_t *) base;
    return w->wdt_wtc;
}

const hw_wdog_ops_t sdrv_wdt_ops = {
    sdrv_wdt_is_enabled,
    sdrv_wdt_cfg,
    sdrv_wdt_enable,
    sdrv_wdt_disable,
    sdrv_wdt_refresh,
    sdrv_wdt_get_cnt,
    sdrv_wdt_get_tmo_us,

    rf_sdrv_wdt_refresh_if_en,
};
