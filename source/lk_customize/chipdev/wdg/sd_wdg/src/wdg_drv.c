//*****************************************************************************
//
// wdg_drv.c - Driver for the Watchdog Timer Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup wdg_api
//! @{
//
//*****************************************************************************
#include <sys/types.h>
#include <platform/debug.h>
#include <trace.h>
#include <reg.h>
#include <assert.h>
#include <platform.h>
#include "__regs_base.h"
#include "target_res.h"
#include "wdg_drv.h"

#define WDG_SD_IP_TEST	1
static uint32_t g_tsr[wdg_really_num_max] = {0};
static uint32_t g_clk_select[wdg_max_clk_num] = {WDG_MAIN_CLK,WDG_BUS_CLK,WDG_EXT_CLK,WDG_TIE_OFF,WDG_LP_CLK};

//*****************************************************************************
//
//! reg_poll_value .
//! \base is watchdog baseaddress
//! \reg is the reg address of the rstgen module.
//!	\start is register start bit
//!	\width is register start bit to end bit number
//!	\value is write to register value
//!	\retrycount is polling count
//! This function is polling reg status .
//!
//! \return true when polling status equal value else return false
//
//*****************************************************************************
static inline uint32_t reg_poll_value(wdg_reg_type_t* base,uint32_t reg,int start, int width,uint32_t value,uint32_t retrycount)
{
    uint32_t v = 0;

    //LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "reg:0x%x start:%d width:%d value:%d retrycount:%d\n",reg,start,width,value,retrycount);
    do {
        switch (reg) {
            case WDG_CTRL:
                {
                    v= base->wdg_ctrl;
                    break;
                }
            case WDG_WTC:
                {
                    v= base->wdg_wtc;
                    break;
                }
            case WDG_WRC_CTL:
                {
                    v= base->wdg_wrc_ctl;
                    break;
                }
            case WDG_WRC_VAL:
                {
                    v= base->wdg_wrc_val;
                    break;
                }
            case WDG_WRC_SEQ:
                {
                    v= base->wdg_wrc_seq;
                    break;
                }
            case WDG_RST_CTL:
                {
                    v= base->wdg_rst_ctl;
                    break;
                }
            case WDG_EXT_RST_CTL:
                {
                    v= base->wdg_ext_rst_ctl;
                    break;
                }
            case WDG_CNT:
                {
                    v= base->wdg_cnt;
                    break;
                }
            case WDG_TWS:
                {
                    v= base->wdg_tsw;
                    break;
                }

            case WDG_INT:
                {
                    v= base->wdg_int;
                    break;
                }
            case WDG_LOCK:
                {
                    v= base->wdg_lock;
                    break;
                }
            default:
                {
                    v=0;
                    break;
                }
        }

        if (((v>>start) & ((1<<width)-1)) == value){
            return retrycount;
        }
    }while (--retrycount);

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "is error retrycount:%d \n",retrycount);

    return retrycount;
}

//*****************************************************************************
//
//! wdg_unlock.
//!
//! \param base is the base address of the wdg timer module baseaddress.
//! \param unlock_mask is need lock mask.
//!
//! This function is unlock watchdog reg lock
//! registers.
//!
//! \return None.
//
//*****************************************************************************
static bool wdg_unlock(wdg_reg_type_t *base,uint32_t unlock_mask)
{
    // Check the arguments.
    ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_unlock:lock_mask:%d\n",unlock_mask);

    base->wdg_lock &=~unlock_mask;

    return true;
}

//*****************************************************************************
//
//! wdg_clk_select .
//! \base is watchdog base address
//!	\config is watchdog config information
//! This function is select watchdog clk.
//!
//! \return true if the wdg clock is selected else return false
//
//*****************************************************************************
static bool wdg_clk_select(wdg_reg_type_t *base,const wdg_config_t *wdg_config)
{
    // Check the arguments.
    ASSERT_PARAMETER(base);

    if((wdg_config->wdg_ctrl_config.clockSource !=wdg_main_clk)
        && (wdg_config->wdg_ctrl_config.clockSource !=wdg_bus_clk)
        && (wdg_config->wdg_ctrl_config.clockSource !=wdg_ext_clk)
        && (wdg_config->wdg_ctrl_config.clockSource !=wdg_tie_off)
        && (wdg_config->wdg_ctrl_config.clockSource !=wdg_lp_clk))
    {
        LTRACEF("clk_select_mask paramenter error clk_select_mask:%d\n",wdg_config->wdg_ctrl_config.clockSource);
        return false;
    }

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "clock clk_select:%d\n",wdg_config->wdg_ctrl_config.clockSource);
    wdg_unlock(base,WDG_LOCK_CTL_LOCK_MASK);
    //clear clk select bit
    base->wdg_ctrl &=~WDG_CTRL_CLK_SRC_MASK;
    //set clk select
    base->wdg_ctrl |=WDG_CTRL_CLK_SRC(wdg_config->wdg_ctrl_config.clockSource);

    base->wdg_ctrl &= ~WDG_CTRL_PRE_DIV_NUM_MASK;
    base->wdg_ctrl |= WDG_CTRL_PRE_DIV_NUM(wdg_config->wdg_ctrl_config.prescaler);

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "WatchdogClkSelect success clockSource:%d\n",wdg_config->wdg_ctrl_config.clockSource);
    return true;
}

//*****************************************************************************
//
//! wdg_get_default_config .
//! \config is Pointer to WDG config structure
//! This function initializes the WDG configure structure to default value.
//!
//! \return
//
//*****************************************************************************
void wdg_get_default_config(wdg_config_t *wdg_config)
{
    if(!wdg_config){
        LTRACEF("config paramenter error !!\n");
        return;
    }
    // Watchdog global control register
    wdg_config->wdg_ctrl_config.clockSource = wdg_main_clk;
    wdg_config->wdg_ctrl_config.enableAutostart = false;
    wdg_config->wdg_ctrl_config.enableDebugmode = false;
    wdg_config->wdg_ctrl_config.enableSelftest = false;
    wdg_config->wdg_ctrl_config.enableSoftRest = true;
    wdg_config->wdg_ctrl_config.enableSrcSelect = 0x1U;
    wdg_config->wdg_ctrl_config.enableWdg = false;
    wdg_config->wdg_ctrl_config.prescaler = 11999U;
    wdg_config->wdg_ctrl_config.terminalCountSrc = 0x1U;
    // Watchdog terminal count value
    wdg_config->wdg_timeout = DEFAULT_WATCHOUT_TIMEOUT_TIME;
    // Watchdog refresh control
    wdg_config->wdg_refresh_config.enableRefreshTrig = false;
    wdg_config->wdg_refresh_config.enableSeqRefresh = false;
    wdg_config->wdg_refresh_config.wdgModeSelect = wdg_mechanism_mode1;
    // Watchdog Refresh window limit
    wdg_config->refresh_wind_limit = DEFAULT_WATCHOUT_WIN_LOW_LIMIT;
    // Watchdog refresh sequence delta
    wdg_config->refresh_seq_delta = DEFAULT_WATCHOUT_SEQ_DELTA;
    // Watchdog reset control
    wdg_config->wdg_reset_cfg.enableSysReset = false;
    wdg_config->wdg_reset_cfg.enableWdgResetEn = false;
    wdg_config->wdg_reset_cfg.plusRstWind = 0x0U;
    wdg_config->wdg_reset_cfg.SysRstMode = 0x0U;
    wdg_config->wdg_reset_cfg.wdgResetCnt = 0x0U;
    // Watchdog external reset control
    wdg_config->wdg_ext_reset_cfg.enableSysExtReset = true;
    wdg_config->wdg_ext_reset_cfg.plusRstWind = 0x0U;
    wdg_config->wdg_ext_reset_cfg.SysExtRstMode = 0x0U;
    wdg_config->wdg_ext_reset_cfg.wdgResetCnt = 0x0U;
    // Watchdog timestamp
    wdg_config->wdg_tsw = DEFAULT_WATCHOUT_TIMESTAMP;
    // Watchdog interrupt
    wdg_config->wdg_int_cfg.ill_seq_refr_int_clr = false;
    wdg_config->wdg_int_cfg.ill_seq_refr_int_en = false;
    wdg_config->wdg_int_cfg.ill_win_refr_int_clr = false;
    wdg_config->wdg_int_cfg.ill_win_refr_int_en = false;
    wdg_config->wdg_int_cfg.overflow_int_clr = true;
    wdg_config->wdg_int_cfg.overflow_int_en = true;
    // Watchdog lock
    wdg_config->wdg_lock_cfg.clk_src_lock = false;
    wdg_config->wdg_lock_cfg.ctl_lock = false;
    wdg_config->wdg_lock_cfg.ext_rst_lock = false;
    wdg_config->wdg_lock_cfg.int_lock = false;
    wdg_config->wdg_lock_cfg.rst_lock = false;
    wdg_config->wdg_lock_cfg.wrc_lock = false;
    wdg_config->wdg_lock_cfg.wtc_lock = false;

    return;
}

//*****************************************************************************
//
//! wdg_set_timeout .
//!
//! \param base WDG peripheral base address.
//!	\timeoutCount WDG timeout value, ms
//! This function is set watchdog timeout.
//!
//! \return Returns \b true if the wdg timer is set ok and \b false
//! if it is not.
//
//*****************************************************************************
bool wdg_set_timeout(wdg_reg_type_t *base,uint32_t timeout_ms)
{
    uint32_t clk_mask = wdg_main_clk;
    uint32_t freq = 0;
    uint32_t divisor = 1;
    uint32_t wtcon = 0;

    ASSERT_PARAMETER(base);

    //get clk frequence
    //read clk source
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_set_timeout:%d\n",timeout_ms);
    clk_mask = ((base->wdg_ctrl & WDG_CTRL_CLK_SRC_MASK) >> WDG_CTRL_CLK_SRC_SHIFT);
    divisor = ((base->wdg_ctrl & WDG_CTRL_PRE_DIV_NUM_MASK) >> WDG_CTRL_PRE_DIV_NUM_SHIFT) + 1;
    freq = g_clk_select[clk_mask];
    wtcon = timeout_ms*((freq / divisor)/1000);//timeout is ms and change to s

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "clk_mask:%d,freq:%d,divisor:0x%x wtcon:0x%x\n",clk_mask,freq,divisor,wtcon);

    wdg_unlock(base,WDG_LOCK_WTC_LOCK_MASK);
    //clear wtc val
    base->wdg_wtc &= ~WDG_WTC_MASK;
    //set wtc val
    base->wdg_wtc |= wtcon;
    return true;
}

//*****************************************************************************
//
//! wdg_delay_timeout .
//!
//! \param base WDG peripheral base address.
//!	\timeout_ms WDG timeout value, ms
//! This function is for watchdog test delay.
//!
//! \return Returns \b true if the wdg timer is set ok and \b false
//! if it is not.
//
//*****************************************************************************
bool wdg_delay_timeout(wdg_reg_type_t *base,uint32_t timeout_ms)
{
    uint32_t clk_mask = wdg_main_clk;
    uint32_t freq = 0;
    uint32_t divisor = 1;
    uint32_t wtcon = 0;
    uint32_t start_wdg_cnt = 0;
    uint32_t current_wdg_cnt = 0;
    uint32_t wdg_wtc_value = 0;
    uint32_t count =200;

    // Check the arguments.
    ASSERT_PARAMETER(base);

    //get clk frequence
    //read clk source
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_delay_timeout:%d\n",timeout_ms);
    clk_mask = ((base->wdg_ctrl & WDG_CTRL_CLK_SRC_MASK) >> WDG_CTRL_CLK_SRC_SHIFT);
    divisor = ((base->wdg_ctrl & WDG_CTRL_PRE_DIV_NUM_MASK) >> WDG_CTRL_PRE_DIV_NUM_SHIFT) + 1;
    freq = g_clk_select[clk_mask];
    wtcon = timeout_ms*((freq / divisor)/1000);//timeout is ms and change to s

    wdg_wtc_value = base->wdg_wtc;
    start_wdg_cnt =base->wdg_cnt;
    current_wdg_cnt = base->wdg_cnt;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wtcon:%d,wdg_wtc_value:%d;start_wdg_cnt:%d;current_wdg_cnt:%d\n",wtcon,wdg_wtc_value,start_wdg_cnt,current_wdg_cnt);
    if(wtcon > wdg_wtc_value){
        while((current_wdg_cnt < wdg_wtc_value) && (start_wdg_cnt <= current_wdg_cnt)){
            current_wdg_cnt = base->wdg_cnt;
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "current_wdg_cnt1:%d\n",current_wdg_cnt);
        }
        while(count--){
            ;
        }
    }else{
        while((current_wdg_cnt < wtcon) && (start_wdg_cnt <= current_wdg_cnt)){
            current_wdg_cnt = base->wdg_cnt;
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "current_wdg_cnt2:%d\n",current_wdg_cnt);
        }
    }
    return true;
}

//*****************************************************************************
//
//! wdg_set_window_limit .
//!
//! \param base WDG peripheral base address.
//!	\window_timeout_ms WDG window timeout value(ms).
//! This function sets the wdg window value..
//!
//! \return Returns \b true if the wdg timer is set ok and \b false
//! if it is not.
//
//*****************************************************************************
bool wdg_set_window_limit(wdg_reg_type_t *base,uint32_t window_timeout_ms)
{
    uint32_t clk_mask = wdg_main_clk;
    uint32_t freq = 0;
    uint32_t divisor = 1;
    uint32_t wincon = 0;

    // Check the arguments.
    ASSERT_PARAMETER(base);

    //get clk frequence
    //read clk source
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_set_window_limit:%d\n",window_timeout_ms);
    clk_mask = ((base->wdg_ctrl & WDG_CTRL_CLK_SRC_MASK) >> WDG_CTRL_CLK_SRC_SHIFT);
    divisor = ((base->wdg_ctrl & WDG_CTRL_PRE_DIV_NUM_MASK) >> WDG_CTRL_PRE_DIV_NUM_SHIFT) + 1;
    freq = g_clk_select[clk_mask];
    wincon = window_timeout_ms*((freq / divisor)/1000);//timeout is ms and change to s

    //clear wrc val
    base->wdg_wrc_val &= 0x00000000U;
    //set wrc val
    base->wdg_wrc_val |= wincon;

    return true;
}

//*****************************************************************************
//
//! wdg_set_seq_delta .
//!
//! \param base WDG peripheral base address.
//!	\seq_delta_timeout_ms WDG seq delta timeout value(ms).
//! This function sets the wdg seq delta value..
//!
//! \return Returns \b true if the wdg timer is set ok and \b false
//! if it is not.
//
//*****************************************************************************
bool wdg_set_seq_delta(wdg_reg_type_t *base,uint32_t seq_delta_timeout_ms)
{
    uint32_t clk_mask = wdg_main_clk;
    uint32_t freq = 0;
    uint32_t divisor = 1;
    uint32_t seqcon = 0;

    // Check the arguments.
    ASSERT_PARAMETER(base);
    //get clk frequence
    //read clk source
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_set_seq_delta:%d\n",seq_delta_timeout_ms);
    clk_mask = ((base->wdg_ctrl & WDG_CTRL_CLK_SRC_MASK) >> WDG_CTRL_CLK_SRC_SHIFT);
    divisor = ((base->wdg_ctrl & WDG_CTRL_PRE_DIV_NUM_MASK) >> WDG_CTRL_PRE_DIV_NUM_SHIFT) + 1;
    freq = g_clk_select[clk_mask];
    seqcon = seq_delta_timeout_ms*((freq / divisor)/1000);//timeout is ms and change to s

    //clear wrc seq val
    base->wdg_wrc_seq &= 0x00000000U;
    //set wrc seq val
    base->wdg_wrc_seq |= seqcon;

    return true;
}

//*****************************************************************************
//
//! wdg_refesh_mechanism_select.
//!
//! \param base WDG peripheral base address
//! \param config The functional test configuration of WDG
//!
//! This function set watchdog refresh mechanism.
//!
//! \select success return true else false
//
//*****************************************************************************
bool wdg_refesh_mechanism_select(wdg_reg_type_t *base,const wdg_config_t *wdg_config)
{
    // Check the arguments.
    ASSERT_PARAMETER(base);

    if(wdg_config->wdg_refresh_config.wdgModeSelect > wdg_mechanism_mode_max){
        LTRACEF("refresh_mechanism paramenter error refresh_mechanism:%d\n",wdg_config->wdg_refresh_config.wdgModeSelect);
        return false;
    }

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "refresh_mechanism after:%d\n",wdg_config->wdg_refresh_config.wdgModeSelect);

    switch (wdg_config->wdg_refresh_config.wdgModeSelect) {
        case wdg_mechanism_mode1:
        {
            // 1.set mode0_en in wdg_wrc_ctrl to 0x01,modem1_en in wdg_wrc_ctrl to 0x0
            base->wdg_wrc_ctl &= ~(WDG_WRC_CTRL_MODEM0_MASK | WDG_WRC_CTRL_MODEM1_MASK);
            base->wdg_wrc_ctl |= WDG_WRC_CTRL_MODEM0_MASK;
            break;
        }
        case wdg_mechanism_mode2:
        {
            // 1.set proper wtc value
            // 2.set win_low_limit in wdg_wrc_val to proper value
            // 3.set both mode0_en and mode1_en in wdg_wrc_ctrl to 0x01

            base->wdg_wrc_ctl &= ~(WDG_WRC_CTRL_MODEM0_MASK | WDG_WRC_CTRL_MODEM1_MASK);
            base->wdg_wrc_ctl |= WDG_WRC_CTRL_MODEM1_MASK;
            break;
        }
        case wdg_mechanism_mode3:
        {
            // 1.set seq_delta in wdg_wrc_seq to proper value.
            // 2.set seq_refr_en in wdg_wrc_ctrl to 0x01
            base->wdg_wrc_ctl &= ~(WDG_WRC_CTRL_MODEM0_MASK | WDG_WRC_CTRL_MODEM1_MASK);
            base->wdg_wrc_ctl |= (WDG_WRC_CTRL_SEQ_REFR_MASK);
            break;
        }
        default:
        {
            LTRACEF("watchdog mode set err refresh_mode:%d\n",wdg_config->wdg_refresh_config.wdgModeSelect);
            break;
        }
    }
    return true;
}

//*****************************************************************************
//
//! Refesh mechanism get.
//!
//! \param base WDG peripheral base address
//! This function is get watchdog mechanism mode(mode1 mode2 mode3).
//!
//! \return mechanism mode value.
//
//*****************************************************************************
uint32_t wdg_get_refesh_mechanism(wdg_reg_type_t *base)
{
    uint32_t refresh_mechanism = 0;
    // Check the arguments.
    ASSERT_PARAMETER(base);

    refresh_mechanism = base->wdg_wrc_ctl & (WDG_WRC_CTRL_MODEM0_MASK | WDG_WRC_CTRL_MODEM1_MASK);

    //LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "refresh_mechanism get:%d\n",refresh_mechanism);

    if((WDG_WRC_CTRL_MODEM1_MASK & refresh_mechanism) == WDG_WRC_CTRL_MODEM1_MASK){
        //LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "refresh_mechanism moded2" );
        return wdg_mechanism_mode2;
    }else if(WDG_WRC_CTRL_MODEM0_MASK & refresh_mechanism){
        //LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "refresh_mechanism moded1" );
        return wdg_mechanism_mode1;
    }else{
        //LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "refresh_mechanism moded3" );
        return wdg_mechanism_mode3;
    }
    return wdg_mechanism_mode1;
}

//*****************************************************************************
//
//! Initializes the WDG .
//!
//! \param base   WDG peripheral base address
//! \config The configuration of WDG
//!
//!This function initializes the WDG. When called, the WDG runs according to the configuration.
//!If user wants to reconfigure WDG without forcing a reset first, enableUpdate must be set to true
//!in configuration.
//!
//! \return Returns \b true if the wdg timer is enabled and \b false
//! if it is not.
//
//*****************************************************************************
bool wdg_init(wdg_reg_type_t *base,const wdg_config_t *wdg_config)
{
    uint32_t unlock_mask = WDG_LOCK_CTL_LOCK_MASK
                                        |WDG_LOCK_WTC_LOCK_MASK
                                        |WDG_LOCK_WRC_LOCK_MASK
                                        |WDG_LOCK_RST_LOCK_MASK
                                        |WDG_LOCK_EXT_RST_LOCK_MASK
                                        |WDG_LOCK_INT_LOCK_MASK
                                        |WDG_LOCK_CLK_SRC_LOCK_MASK;
    // Check the arguments.
    ASSERT_PARAMETER(base);

    /* Disable the global interrupts. Otherwise, an interrupt could effectively invalidate the unlock sequence
     * and the WCT may expire. After the configuration finishes, re-enable the global interrupts. */
    //__disable_irq();
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog init start \n");
    wdg_unlock(base,unlock_mask);
    // 1.set wdg_en_src in wdg_ctrl to 0x0 and clear wdg en src to 0x0
    base->wdg_ctrl = 0x0U;
    // 2.set wdg_en_src in wdg_ctrl to 0x1,choose wdg module enable from register and set wdg_src in wdg_ctrl to 0x1
    base->wdg_ctrl |= WDG_CTRL_WDG_EN_SRC(wdg_config->wdg_ctrl_config.enableSrcSelect)|WDG_CTRL_WTC_SRC(wdg_config->wdg_ctrl_config.terminalCountSrc);
    // 3.wait until wdg_en_sta to 0x0 //does not need wait after test add code
    //polling wdg_en_sta to 0x0
    if(reg_poll_value(base,WDG_CTRL,10,1,0,100) == 0){
        LTRACEF("wait wdg_en_sta is timeout\n");
        return false;
    }

    //clk select this select main clk
    //.wdg overflow interrupt time is calculated by:clock period*wdg termination value*(pre_div_num+1)
    //.wdg internal reset request time is calculated by: wdg overflow interrupt time + clock period*rst_cnt value(in wdg_rst_ctl)
    //.wdg external reset request time is calculated by: wdg overflow interrupt time+clock period*rst_cnt value(in wdg_ext_rst_ctl)
    wdg_clk_select(base,wdg_config);
    // 4.set pre_div_num/wtc_src/auto_restart in wdg_ctrl to proper value  //default set kick peroid 200ms,timeout value 1000ms
    //set wdg_wtc to proper value.(only active when wct_src set to 0x1,otherwise it is 0x753)
    //set wdg_wrc_ctrl wdg_wrc_val wdg_wrc_seq wdg_rst_ctrl wdg_ext_rst_ctrl wdg_int to proper value
    //set clk_src in wdg_crtl to choose proper clock source
    wdg_set_timeout(base,wdg_config->wdg_timeout);
    //wdg_en_src in wdg_ctrl to 0x01
    //wait until wdg_en_sta to 0x01
    //set soft_rst in wdg_ctrl to 0x01
    //11.wait until soft_rst in wdg_ctrl to 0x0

    //set refresh mode1
    wdg_refesh_mechanism_select(base,wdg_config);
    //set windows low limit
    base->wdg_wrc_val &= ~WDG_WRC_MASK;
    base->wdg_wrc_val |= wdg_config->refresh_wind_limit;

    //set sequence delta
    base->wdg_wrc_seq &= ~WDG_WRC_SEQ_MASK;
    base->wdg_wrc_seq |= wdg_config->refresh_seq_delta;

    //set wdg tsw
    base->wdg_tsw &= 0x00000000;
    base->wdg_tsw |= wdg_config->wdg_tsw;

    //set wdg int enable overflow int
    base->wdg_int &= 0x00000000;
    base->wdg_int |= WDG_INT_ILL_WIN_REFE_INT_EN(wdg_config->wdg_int_cfg.ill_win_refr_int_en)
                                |WDG_INT_ILL_SEQ_REFE_INT_EN(wdg_config->wdg_int_cfg.ill_seq_refr_int_en)
                                |WDG_INT_OVERFLOW_INT_EN(wdg_config->wdg_int_cfg.overflow_int_en)
                                |WDG_INT_ILL_WIN_REFE_INT_CLR(wdg_config->wdg_int_cfg.ill_win_refr_int_clr)
                                |WDG_INT_ILL_SEQ_REFE_INT_CLR(wdg_config->wdg_int_cfg.ill_seq_refr_int_clr)
                                |WDG_INT_OVERFLOW_INT_CLR(wdg_config->wdg_int_cfg.overflow_int_clr);
    //disable auto_restart set 0
    base->wdg_ctrl |= WDG_CTRL_AUTO_RESTART(wdg_config->wdg_ctrl_config.enableAutostart);
    //__enable_irq();
    return true;
}

//*****************************************************************************
//
//! Shuts down the WDG.
//!
//! \param base   WDG peripheral base address
//!
//! \This function shuts down the WDG
//!
//! \return Returns \b true if the wdg timer is enabled and \b false
//! if it is not.
//
//*****************************************************************************
bool wdg_deInit(wdg_reg_type_t *base)
{
    uint32_t unlock_mask = WDG_LOCK_CTL_LOCK_MASK
                                        |WDG_LOCK_WTC_LOCK_MASK
                                        |WDG_LOCK_WRC_LOCK_MASK
                                        |WDG_LOCK_RST_LOCK_MASK
                                        |WDG_LOCK_EXT_RST_LOCK_MASK
                                        |WDG_LOCK_INT_LOCK_MASK
                                        |WDG_LOCK_CLK_SRC_LOCK_MASK;

    // Check the arguments.
    ASSERT_PARAMETER(base);

    //__disable_irq();
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog deinit start \n");
    wdg_unlock(base,unlock_mask);
    base->wdg_rst_ctl &= 0xffff0000;
    base->wdg_ext_rst_ctl &= 0xffff0000;
    //__enable_irq();
    return true;
}

//*****************************************************************************
//
//! Shuts down the WDG.
//!
//! \param base   WDG peripheral base address
//!
//! \This function shuts down the WDG
//!
//! \return Returns \b true if the wdg timer is enabled and \b false
//! if it is not.
//
//*****************************************************************************
bool wdg_set_testmode_config(wdg_reg_type_t *base, const wdg_config_t *wdg_config)
{
    uint32_t unlock_mask = WDG_LOCK_CTL_LOCK_MASK
                                        |WDG_LOCK_WTC_LOCK_MASK
                                        |WDG_LOCK_WRC_LOCK_MASK
                                        |WDG_LOCK_RST_LOCK_MASK
                                        |WDG_LOCK_EXT_RST_LOCK_MASK
                                        |WDG_LOCK_INT_LOCK_MASK
                                        |WDG_LOCK_CLK_SRC_LOCK_MASK;

    // Check the arguments.
    ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog set test mode start \n");
    wdg_unlock(base,unlock_mask);
    base->wdg_rst_ctl &= 0xffff0000;
    base->wdg_ext_rst_ctl &= 0xffff0000;
    //clear wtc val
    base->wdg_wtc &= ~WDG_WTC_MASK;
    //set wtc val 0xffffffff
    base->wdg_wtc |= 0xffffffff;
    base->wdg_ctrl |= WDG_CTRL_SELFTEST_TRIG(wdg_config->wdg_ctrl_config.enableSelftest);

    return true;
}

//*****************************************************************************
//
//! wdg_enable.
//!
//! \param base WDG peripheral base address
//!
//! This function enables the wdg timer counter and interrupt.
//!
//!
//! \enable success return true else retuen false
//
//*****************************************************************************
bool wdg_enable(wdg_reg_type_t *base)
{
    // Check the arguments.
    ASSERT_PARAMETER(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog enable \n");
    wdg_unlock(base,WDG_LOCK_CTL_LOCK_MASK);

    base->wdg_ctrl |= WDG_CTRL_WDG_EN_MASK;
    if(!reg_poll_value(base,WDG_CTRL,10,1,1,1000)){
        LTRACEF("wait en_sta timeout\n");
        return false;
    }

    //enable soft_reset set 1 for reset os or func enable wdg
    base->wdg_ctrl |= WDG_CTRL_SOFT_RST(1);
    if(!reg_poll_value(base,WDG_CTRL,0,1,0,1000)){
        LTRACEF("wait soft_rst is timeout\n");
        return false;
    }

    return true;
}
//*****************************************************************************
//
//! wdg_disable.
//!
//! \param base WDG peripheral base address
//!
//! This function disables the wdg timer counter and interrupt.
//!
//! \enable success return true else retuen false
//
//*****************************************************************************
bool wdg_disable(wdg_reg_type_t *base)
{
    // Check the arguments.
    ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog disable \n");
    wdg_unlock(base,WDG_LOCK_CTL_LOCK_MASK);
    base->wdg_ctrl |= WDG_CTRL_SOFT_RST(1);
    if(reg_poll_value(base,WDG_CTRL,0,1,0,100) == 0){
        LTRACEF("wait soft_rst is timeout\n");
        return false;
    }
    base->wdg_ctrl &= ~WDG_CTRL_WDG_EN_MASK;
    return true;
}
//*****************************************************************************
//
//! Enables the wdg timer interrupt.
//!
//! \param base WDG peripheral base address
//!
//! This function enables the wdg timer interrupt.
//!
//! \note This function has no effect if the wdg timer has been locked.
//!
//! \sa WatchdogLock(), WatchdogUnlock(), WatchdogEnable()
//!
//! \return None.
//
//*****************************************************************************
bool wdg_enable_Interrupts(wdg_reg_type_t *base)
{
    uint32_t refresh_mechanism = 0;
    // Check the arguments.
    ASSERT_PARAMETER(base);

    refresh_mechanism = wdg_get_refesh_mechanism(base);
    wdg_unlock(base,WDG_LOCK_INT_LOCK_MASK);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_enable_Interrupts start refresh_mechanism:%d\n",refresh_mechanism);
    switch (refresh_mechanism) {
        case wdg_mechanism_mode1:
        {
            base->wdg_int |= WDG_INT_OVERFLOW_INT_EN_MASK;
            break;
        }
        case wdg_mechanism_mode2:
        {
            base->wdg_int |= WDG_INT_ILL_WIN_REFE_INT_EN_MASK;
            break;
        }
        case wdg_mechanism_mode3:
        {
            base->wdg_int |= WDG_INT_ILL_SEQ_REFE_INT_EN_MASK;
            break;
        }
        default:
        {
            LTRACEF("watchdog int enable err,refresh_mechanism:%d\n",refresh_mechanism);
            break;
        }
    }

    return true;
}

//*****************************************************************************
//
//! disables the wdg timer interrupt.
//!
//! \param base WDG peripheral base address
//!
//! This function enables the wdg timer interrupt.
//!
//! \note This function has no effect if the wdg timer has been locked.
//!
//! \sa WatchdogLock(), WatchdogUnlock(), WatchdogEnable()
//!
//! \return None.
//
//*****************************************************************************
bool wdg_disable_Interrupts(wdg_reg_type_t *base)
{
    uint32_t refresh_mechanism = 0;
    // Check the arguments.
    ASSERT_PARAMETER(base);

    refresh_mechanism = wdg_get_refesh_mechanism(base);
    wdg_unlock(base,WDG_LOCK_INT_LOCK_MASK);
    switch (refresh_mechanism) {
        case wdg_mechanism_mode1:
        {
            base->wdg_int &= ~WDG_INT_OVERFLOW_INT_EN_MASK;
            break;
        }
        case wdg_mechanism_mode2:
        {
            base->wdg_int &= ~WDG_INT_ILL_WIN_REFE_INT_EN_MASK;
            break;
        }
        case wdg_mechanism_mode3:
        {
            base->wdg_int &= ~WDG_INT_ILL_SEQ_REFE_INT_EN_MASK;
            break;
        }
        default:
        {
            LTRACEF("watchdog int disable err refresh_mechanism:%d\n",refresh_mechanism);
            break;
        }
    }

    return true;
}

//*****************************************************************************
//
//! Gets wdg all status flags
//!
//! \param base wdg peripheral base address
//!
//! This function gets all status flags.
//!
//! \return status flag please see wdg_status_flags
//
//*****************************************************************************
uint32_t wdg_get_status_flag(wdg_reg_type_t *base)
{
    uint32_t status_flag = 0U;
    uint32_t temp_test = 0;

    // Check the arguments.
    ASSERT_PARAMETER(base);

    status_flag |= (base->wdg_ctrl & WDG_CTRL_WDG_EN_STA_MASK);
    status_flag |= (base->wdg_int & (WDG_INT_ILL_WIN_REFE_INT_STA_MASK | WDG_INT_ILL_SEQ_REFE_INT_STA_MASK | WDG_INT_OVERFLOW_INT_STA_MASK));
    temp_test =base->wdg_int;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_get_status_flag status_flag:0x%x,temp_test:0x%x\n",status_flag,temp_test);

    return status_flag;
}

//*****************************************************************************
//
//! clear wdg status flags
//!
//! \param base wdg peripheral base address
//!
//! This function clear status flags.
//!
//! \return true or false
//
//*****************************************************************************
bool wdg_clear_status_flag(wdg_reg_type_t *base,uint32_t mask)
{
    // Check the arguments.
    ASSERT_PARAMETER(base);

    if(mask & WDG_CTRL_WDG_EN_STA_MASK){
        base->wdg_ctrl &= ~WDG_CTRL_WDG_EN_STA_MASK;
    }

    base->wdg_int &= ~(mask & (WDG_INT_ILL_WIN_REFE_INT_STA_MASK | WDG_INT_ILL_SEQ_REFE_INT_STA_MASK | WDG_INT_OVERFLOW_INT_STA_MASK));

    return true;
}

//*****************************************************************************
//
//! wdg_addr_to_number.
//! \param base wdg peripheral base address
//!
//! This function is trans watchdog addr to number 0~7.
//!
//! \return None.
//
//*****************************************************************************
static uint32_t wdg_addr_to_number(wdg_reg_type_t *base)
{
    uint32_t wdg_number = wdg_really_num1;

    // Check the arguments.
    ASSERT_PARAMETER(base);

    if(WDG1_BASE == base){
        wdg_number = wdg_really_num1;
    }else if(WDG2_BASE == base){
        wdg_number = wdg_really_num2;
    }else if(WDG3_BASE == base){
        wdg_number = wdg_really_num3;
    }else if(WDG4_BASE == base){
        wdg_number = wdg_really_num4;
    }else if(WDG5_BASE == base){
        wdg_number = wdg_really_num5;
    }else if(WDG6_BASE == base){
        wdg_number = wdg_really_num6;
    }else if(WDG7_BASE == base){
        wdg_number = wdg_really_num7;
    }else if(WDG8_BASE == base){
        wdg_number = wdg_really_num8;
    }else{
        wdg_number = wdg_really_num1;
    }

    return wdg_number;
}

//*****************************************************************************
//
//! wdg_record_wdgcnt.
//! \param base wdg peripheral base address
//!
//! This function is update wdg cnt value to global g_tsr.
//!
//! \return None.
//
//*****************************************************************************
bool wdg_record_wdgcnt(wdg_reg_type_t *base)
{
    uint32_t wdg_number = 0;
    // Check the arguments.
    ASSERT_PARAMETER(base);

    wdg_number = wdg_addr_to_number(base);
    // 3.read wdg_cnt and record it as tsr
    g_tsr[wdg_number] = base->wdg_cnt;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_record_wdgcnt wdg_number:%d,g_tsr[%d]:%d\n",wdg_number,wdg_number,g_tsr[wdg_number]);
    return true;
}

//*****************************************************************************
//
//! wdg_refresh .
//!
//! \param base wdg peripheral base address
//!
//! This function feeds the WDG.
//!
//! \if refresh success return true else return false
//
//*****************************************************************************
bool wdg_refresh(wdg_reg_type_t *base)
{
    uint32_t refresh_mechanism = wdg_mechanism_mode1;
    // Check the arguments.
    ASSERT_PARAMETER(base);

    refresh_mechanism = wdg_get_refesh_mechanism(base);

    if(refresh_mechanism > wdg_mechanism_mode_max){
        LTRACEF("refresh_mechanism paramenter error refresh_mechanism:%d\n",refresh_mechanism);
        return false;
    }

    //LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "refresh_mechanism after:%d\n",refresh_mechanism);

    switch (refresh_mechanism) {
        case wdg_mechanism_mode1:
        case wdg_mechanism_mode2:
        {
            // 2.set refr_trig in wdg_wrc_ctrl to 0x01
            // 4.set refr_trig in wdg_wrc_ctrl to 0x01
            base->wdg_wrc_ctl |=WDG_WRC_CTRL_REFR_TRIG_MASK;
            break;
        }
        case wdg_mechanism_mode3:
        {
            // 3.read wdg_cnt and record it as tsr ref WatchdogMechanism3UpdataWdtCnt func
            // 4.write tsr to wdg_tsw. refesh will happen if the condition was right.
            uint32_t wdg_number = wdg_addr_to_number(base);
            //LTRACEF("watchdog mode set err wdg_number:%d,g_tsr[%d]:0x%x\n",wdg_number,wdg_number,g_tsr[wdg_number]);
            base->wdg_tsw = g_tsr[wdg_number];
            break;
        }
        default:
        {
            LTRACEF("watchdog mode set err refresh_mode:%d\n",refresh_mechanism);
            break;
        }
    }

    return true;
}
//*****************************************************************************
//
//! wdg_set_reset_cnt
//!
//! \param base wdg peripheral base address
//!
//! This function is set reset count value.
//!
//! \return true or false
//
//*****************************************************************************
bool wdg_set_reset_cnt(wdg_reg_type_t *base,uint32_t rst_delay)
{
    // Check the arguments.
    ASSERT_PARAMETER(base);

    //base->wdg_rst_ctl &= ~WDG_RST_CTRL_RST_CNT_MASK;
    base->wdg_rst_ctl |=WDG_RST_CTRL_RST_CNT(rst_delay);

    return true;
}

//*****************************************************************************
//
//! wdg_get_reset_cnt
//!
//! \param base wdg peripheral base address
//!
//! This function gets reset count.
//!
//! \return reset count
//
//*****************************************************************************
uint32_t wdg_get_reset_cnt(wdg_reg_type_t *base)
{
    uint32_t reset_cnt = 0U;

    // Check the arguments.
    ASSERT_PARAMETER(base);

    reset_cnt |= (base->wdg_rst_ctl & WDG_RST_CTRL_RST_CNT_MASK);

    return reset_cnt;
}
//*****************************************************************************
//
//! wdg_clear_reset_cnt
//!
//! \param base wdg peripheral base address
//!
//! This function clear reset count.
//!
//! \return true or false
//
//*****************************************************************************
bool wdg_clear_reset_cnt(wdg_reg_type_t *base)
{
    // Check the arguments.
    ASSERT_PARAMETER(base);

    base->wdg_rst_ctl &= ~WDG_RST_CTRL_RST_CNT_MASK;

    return true;
}
//*****************************************************************************
//
//! wdg_set_ext_reset_cnt
//!
//! \param base wdg peripheral base address
//!
//! This function set external reset count.
//!
//! \return external reset count
//
//*****************************************************************************
bool wdg_set_ext_reset_cnt(wdg_reg_type_t *base,uint32_t rst_delay)
{
    // Check the arguments.
    ASSERT_PARAMETER(base);

    base->wdg_ext_rst_ctl &= ~WDG_EXT_RST_CTRL_RST_CNT_MASK;
    base->wdg_ext_rst_ctl |=WDG_EXT_RST_CTRL_RST_CNT(rst_delay);

    return true;
}

//*****************************************************************************
//
//! wdg_get_ext_reset_cnt
//!
//! \param base wdg peripheral base address
//!
//! This function gets external reset count.
//!
//! \return external reset count
//
//*****************************************************************************
uint32_t wdg_get_ext_reset_cnt(wdg_reg_type_t *base)
{
    uint32_t ext_reset_cnt = 0U;

    // Check the arguments.
    ASSERT_PARAMETER(base);

    ext_reset_cnt |= (base->wdg_ext_rst_ctl & WDG_EXT_RST_CTRL_RST_CNT_MASK);

    return ext_reset_cnt;
}

//*****************************************************************************
//
//! wdg_clear_ext_reset_cnt
//!
//! \param base wdg peripheral base address
//!
//! This function clear external reset count.
//!
//! \return true or false
//
//*****************************************************************************
bool wdg_clear_ext_reset_cnt(wdg_reg_type_t *base)
{
    // Check the arguments.
    ASSERT_PARAMETER(base);

    base->wdg_ext_rst_ctl &= ~WDG_EXT_RST_CTRL_RST_CNT_MASK;

    return true;
}
#if 0
//*****************************************************************************
//
//! watchdog interrupt handler for the wdg timer interrupt.
//!
//! wdg timer interrupt occurs.
//!
//! This function does the actual registering of the interrupt handler.  This
//! function also enables the global interrupt in the interrupt controller; the
//! wdg timer interrupt must be enabled via WatchdogEnable().  It is the
//! interrupt handler's responsibility to clear the interrupt source via
//! WatchdogIntClear().
//! handlers.
//!
//! \note For parts with a wdg timer module that has the ability to
//! generate an NMI instead of a standard interrupt, this function registers
//! the standard wdg interrupt handler.  To register the NMI wdg
//! handler, use IntRegister() to register the handler for the
//! \b FAULT_NMI interrupt.
//!
//! \return None.
//
//*****************************************************************************
enum handler_return wdg_irq_handle(void *arg)
{
    unsigned int instance = (uintptr_t)arg;
    LTRACEF("watchdog_irq_handle instance:%d\n",instance);
    //add reset or mendump func
    platform_halt(HALT_ACTION_REBOOT, HALT_REASON_HW_WDG);
    /* We should never get here; watchdog handlers should always be fatal. */
    DEBUG_ASSERT(false);
    return INT_NO_RESCHEDULE;
}
#endif
//*****************************************************************************
//
//! Registers an interrupt handler for the wdg timer interrupt.
//!
//! \param base wdg peripheral base address
//! \param pfnHandler is a pointer to the function to be called when the
//! \overflow_int is register over flow int
//! wdg timer interrupt occurs.
//!
//! This function does the actual registering of the interrupt handler.  This
//! function also enables the global interrupt in the interrupt controller; the
//! wdg timer interrupt must be enabled via WatchdogEnable().  It is the
//! interrupt handler's responsibility to clear the interrupt source via
//! WatchdogIntClear().
//!
//! \sa IntRegister() for important information about registering interrupt
//! handlers.
//!
//! \note For parts with a wdg timer module that has the ability to
//! generate an NMI instead of a standard interrupt, this function registers
//! the standard wdg interrupt handler.  To register the NMI wdg
//! handler, use IntRegister() to register the handler for the
//! \b FAULT_NMI interrupt.
//!
//! \return true or false.
//
//*****************************************************************************
bool wdg_Int_register(void *handle,wdg_reg_type_t *base,int_handler call_func,bool overflow_int)
{
    uint32_t refresh_mechanism = 0;
    uint32_t wdg_int_num = 0;
    // Check the arguments.
    ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_Int_register:\n");
    refresh_mechanism = wdg_get_refesh_mechanism(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog  refresh_mechanism:%d\n",refresh_mechanism);
    if(WDG1_BASE == base){
        if(overflow_int){
            wdg_int_num = WDG1_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG1_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode3 & refresh_mechanism){
            wdg_int_num = WDG1_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG1_OVFLOW_INT_NUM;
        }
    }else if(WDG2_BASE == base){
        if(overflow_int){
            wdg_int_num = WDG2_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG2_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode3 & refresh_mechanism){
            wdg_int_num = WDG2_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG2_OVFLOW_INT_NUM;
        }
    }else if(WDG3_BASE == base){
        if(overflow_int){
            wdg_int_num = WDG3_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG3_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode3 & refresh_mechanism){
            wdg_int_num = WDG3_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG3_OVFLOW_INT_NUM;
        }
    }else if(WDG4_BASE == base){
        if(overflow_int){
            wdg_int_num = WDG4_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG4_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode3 & refresh_mechanism){
            wdg_int_num = WDG4_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG4_OVFLOW_INT_NUM;
        }
    }else if(WDG5_BASE == base){
        if(overflow_int){
            wdg_int_num = WDG5_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG5_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode3 & refresh_mechanism){
            wdg_int_num = WDG5_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG5_OVFLOW_INT_NUM;
        }
    }else if(WDG6_BASE == base){
        if(overflow_int){
            wdg_int_num = WDG6_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG6_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode3 & refresh_mechanism){
            wdg_int_num = WDG6_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG6_OVFLOW_INT_NUM;
        }
    }else if(WDG7_BASE == base){
        if(overflow_int){
            wdg_int_num = WDG7_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG7_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode3 & refresh_mechanism){
            wdg_int_num = WDG7_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG7_OVFLOW_INT_NUM;
        }
    }else if(WDG8_BASE == base){
        if(overflow_int){
            wdg_int_num = WDG8_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG8_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode3 & refresh_mechanism){
            wdg_int_num = WDG8_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG8_OVFLOW_INT_NUM;
        }
    }else{
        if(overflow_int){
            wdg_int_num = WDG1_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG1_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode3 & refresh_mechanism){
            wdg_int_num = WDG1_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG1_OVFLOW_INT_NUM;
        }
    }

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_int_num:%d\n",wdg_int_num);
    register_int_handler(wdg_int_num,call_func,handle);
    unmask_interrupt(wdg_int_num);
    return true;
}

//*****************************************************************************
//
//! Unregisters an interrupt handler for the wdg timer interrupt.
//!
//! \param base wdg peripheral base address
//!
//! This function does the actual unregistering of the interrupt handler.  This
//! function clears the handler to be called when a wdg timer interrupt
//! occurs.  This function also masks off the interrupt in the interrupt
//! controller so that the interrupt handler no longer is called.
//!
//! \sa IntRegister() for important information about registering interrupt
//! handlers.
//!
//! \note For parts with a wdg timer module that has the ability to
//! generate an NMI instead of a standard interrupt, this function unregisters
//! the standard wdg interrupt handler.  To unregister the NMI wdg
//! handler, use IntUnregister() to unregister the handler for the
//! \b FAULT_NMI interrupt.
//!
//! \return true or false.
//
//*****************************************************************************
bool wdg_int_unregister(wdg_reg_type_t *base,bool overflow_int)
{
    uint32_t refresh_mechanism = 0;
    uint32_t wdg_int_num = 0;
    // Check the arguments.
    ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_int_unregister:\n");
    refresh_mechanism = wdg_get_refesh_mechanism(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog refresh_mechanism:%d\n",refresh_mechanism);

    if(WDG1_BASE ==base){
        if(overflow_int){
            wdg_int_num = WDG1_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG1_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG1_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG1_OVFLOW_INT_NUM;
        }
    }else if(WDG2_BASE ==base){
        if(overflow_int){
            wdg_int_num = WDG2_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG2_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG2_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG2_OVFLOW_INT_NUM;
        }
    }else if(WDG3_BASE ==base){
        if(overflow_int){
            wdg_int_num = WDG3_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG3_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG3_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG3_OVFLOW_INT_NUM;
        }
    }else if(WDG4_BASE ==base){
        if(overflow_int){
            wdg_int_num = WDG4_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG4_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG4_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG4_OVFLOW_INT_NUM;
        }
    }else if(WDG5_BASE ==base){
        if(overflow_int){
            wdg_int_num = WDG5_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG5_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG5_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG5_OVFLOW_INT_NUM;
        }
    }else if(WDG6_BASE ==base){
        if(overflow_int){
            wdg_int_num = WDG6_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG6_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG6_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG6_OVFLOW_INT_NUM;
        }
    }else if(WDG7_BASE ==base){
        if(overflow_int){
            wdg_int_num = WDG7_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG7_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG7_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG7_OVFLOW_INT_NUM;
        }
    }else if(WDG8_BASE ==base){
        if(overflow_int){
            wdg_int_num = WDG8_OVFLOW_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG8_ILL_WIN_REFR_INT_NUM;
        }else if(wdg_mechanism_mode2 & refresh_mechanism){
            wdg_int_num = WDG8_ILL_SEQ_REFR_INT_NUM;
        }else{
            wdg_int_num = WDG8_OVFLOW_INT_NUM;
        }
    }

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_int_num:%d\n",wdg_int_num);
    mask_interrupt(wdg_int_num);
    return true;
}

//*****************************************************************************
//
//! Clears the wdg timer interrupt.
//!
//! \param base wdg peripheral base address
//!
//! The wdg timer interrupt source is cleared, so that it no longer
//! asserts.
//!
//! \note Because there is a write buffer in the Cortex-M processor, it may
//! take several clock cycles before the interrupt source is actually cleared.
//! Therefore, it is recommended that the interrupt source be cleared early in
//! the interrupt handler (as opposed to the very last action) to avoid
//! returning from the interrupt handler before the interrupt source is
//! actually cleared.  Failure to do so may result in the interrupt handler
//! being immediately reentered (because the interrupt controller still sees
//! the interrupt source asserted).
//!
//! \return true or false.
//
//*****************************************************************************
bool wdg_int_clear(wdg_reg_type_t *base)
{
    uint32_t refresh_mechanism = 0;
    // Check the arguments.
    ASSERT_PARAMETER(base);

    refresh_mechanism = wdg_get_refesh_mechanism(base);

    switch (refresh_mechanism) {
        case wdg_mechanism_mode1:
        {
            base->wdg_int |= WDG_INT_OVERFLOW_INT_CLR_MASK;
            break;
        }
        case wdg_mechanism_mode2:
        {
            base->wdg_int |= WDG_INT_ILL_WIN_REFE_INT_CLR_MASK;
            break;
        }
        case wdg_mechanism_mode3:
        {
            base->wdg_int |= WDG_INT_ILL_SEQ_REFE_INT_CLR_MASK;
            break;
        }
        default:
        {
            LTRACEF("watchdog int enable err refresh_mechanism:%d\n",refresh_mechanism);
            return false;
        }
    }
    return true;
}

//*****************************************************************************
//
//! Enables halt of the wdg timer during debug events.
//!
//! \param base wdg peripheral base address
//!
//! This function allows the wdg timer to stop counting when the processor
//! is stopped by the debugger.  By doing so, the wdg is prevented from
//! expiring (typically almost immediately from a human time perspective) and
//! resetting the system (if reset is enabled).  The wdg instead expires
//! after the appropriate number of processor cycles have been executed while
//! debugging (or at the appropriate time after the processor has been
//! restarted).
//!
//! \return true or false.
//
//*****************************************************************************
bool wdg_halt_enable(wdg_reg_type_t *base)
{
    // Check the arguments.
    ASSERT_PARAMETER(base);
    //
    // Enable timer stalling.
    //
    base->wdg_ctrl |= WDG_CTRL_DBG_HALT_EN_MASK;

    return true;
}

//*****************************************************************************
//
//! Disables halt of the wdg timer during debug events.
//!
//! \param base wdg peripheral base address
//!
//! This function disables the debug mode stall of the wdg timer.  By
//! doing so, the wdg timer continues to count regardless of the processor
//! debug state.
//!
//! \return None.
//
//*****************************************************************************
bool wdg_halt_disable(wdg_reg_type_t *base)
{
    // Check the arguments.
    ASSERT_PARAMETER(base);
    //
    // Disable timer stalling.
    //
    base->wdg_ctrl &= ~(WDG_CTRL_DBG_HALT_EN_MASK);

    return true;
}

uint32_t wdg_get_cnt(wdg_reg_type_t *base)
{
    ASSERT_PARAMETER(base);
    return base->wdg_cnt;
}


bool wdg_set_reset(wdg_reg_type_t *base,const wdg_config_t* wdg_config)
{
    uint32_t unlock_mask = WDG_LOCK_RST_LOCK_MASK
                            |WDG_LOCK_EXT_RST_LOCK_MASK;

    ASSERT_PARAMETER(base);
    wdg_unlock(base,unlock_mask);

    wdg_set_reset_cnt(base,0xf);
    base->wdg_rst_ctl |= WDG_RST_CTRL_RST_CNT(wdg_config->wdg_reset_cfg.wdgResetCnt)
                                        |WDG_RST_CTRL_INT_RST_EN(wdg_config->wdg_reset_cfg.enableSysReset)
                                        |WDG_RST_CTRL_INT_RST_MODE(wdg_config->wdg_reset_cfg.SysRstMode)
                                        |WDG_RST_CTRL_WDG_RST_EN(wdg_config->wdg_reset_cfg.enableWdgResetEn)
                                        |WDG_RST_CTRL_RST_WIN(wdg_config->wdg_reset_cfg.plusRstWind);

    //set ext reset ctrl clear reset mode,not need ext reset mode,if need reset pmic so need config
    base->wdg_ext_rst_ctl &= 0x00000000;
    base->wdg_ext_rst_ctl |= WDG_EXT_RST_CTRL_RST_CNT(wdg_config->wdg_ext_reset_cfg.wdgResetCnt)
                                        |WDG_EXT_RST_CTRL_INT_RST_EN(wdg_config->wdg_ext_reset_cfg.enableSysExtReset)
                                        |WDG_EXT_RST_CTRL_INT_RST_MODE(wdg_config->wdg_ext_reset_cfg.SysExtRstMode)
                                        |WDG_EXT_RST_CTRL_RST_WIN(wdg_config->wdg_ext_reset_cfg.plusRstWind);

    return true;
}
