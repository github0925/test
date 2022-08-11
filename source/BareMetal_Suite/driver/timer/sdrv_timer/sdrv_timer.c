/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <common_hdr.h>
#include <service.h>
#include <time.h>
#include <soc.h>
#include "timer_reg.h"

static U32 g0_tick_saved = 0, g1_tick_saved = 0;

U32 sdrv_tmr_init(module_e id)
{
    sdrv_timer_t *tmr = (sdrv_timer_t *)soc_get_module_base(id);

    g0_tick_saved = tmr->cnt_g0;
    g1_tick_saved = tmr->cnt_g1;

    soc_config_clk(id, FREQ_DEFAULT);

    /* xtal1 may not fully ready, thus we use safety.fsrefclk output ("LF" clock
     * of timer)  */
    /* the mux here is not glitch-less */
    U32 val = tmr->tim_clk_config;
    val &= ~(FM_TIM_CLK_CONFIG_SRC_CLK_SEL | FM_TIM_CLK_CONFIG_DIV_NUM);
    val |= (FV_TIM_CLK_CONFIG_SRC_CLK_SEL(SOC_TMR_CLK_SRC_SEL)
            | FV_TIM_CLK_CONFIG_DIV_NUM(SOC_TMR_CLK_DIV));
    tmr->tim_clk_config = val;

    tmr->cnt_g0_ovf = 0xFFFFFFFF;
    tmr->cnt_g1_ovf = 0xFFFFFFFF;

    tmr->cnt_config |= BM_CNT_CONFIG_CASCADE_MODE;
    tmr->cnt_config |= (BM_CNT_CONFIG_CNT_G1_RLD | BM_CNT_CONFIG_CNT_G0_RLD);

    while ((BM_CNT_CONFIG_CNT_G1_RLD
            | BM_CNT_CONFIG_CNT_G0_RLD) & tmr->cnt_config);

    return 0;
}

U32 sdrv_tmr_get_freq(module_e id)
{
    return 1000000;
}

U64 sdrv_tmr_tick(module_e id)
{
    sdrv_timer_t *tmr = (sdrv_timer_t *)soc_get_module_base(id);

    return ((U64)tmr->cnt_g0 | (((U64)tmr->cnt_g1) << 32U));
}

BOOL sdrv_tmr_is_enabled(module_e id)
{
    /* timer get running once clock been feeded */
    sdrv_timer_t *tmr = (sdrv_timer_t *)soc_get_module_base(id);

    return ((tmr->cnt_config & BM_CNT_CONFIG_CASCADE_MODE) ==
            BM_CNT_CONFIG_CASCADE_MODE);
}
