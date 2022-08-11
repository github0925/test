//*****************************************************************************
//
// wdg_drv_test.c - Driver for the Watchdog Timer Module.
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
#include <platform/interrupts.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <trace.h>
#include <reg.h>
#include <assert.h>
#include <platform.h>
#include "__regs_base.h"
#include "target_res.h"
#include "wdg_drv.h"
#include "wdg_drv_test.h"

static uint32_t g_watchdog_int_count = 0;
//*****************************************************************************
//
// for ip test code.
//! @}
//
//*****************************************************************************
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

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "reg:0x%x start:%d width:%d value:%d retrycount:%d\n",reg,start,width,value,retrycount);
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
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "reg_poll_value retrycount:%d \n",retrycount);
            return retrycount;
        }
        spin(1);
    }while (--retrycount);

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "reg_poll_value is error \n");
    return retrycount;
}

bool dump_all_reg_for_test(wdg_reg_type_t *base)
{
    uint32_t reg_value = 0;
    // Check the arguments.
    ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "#######################################\n");
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "dump_all_reg:\n");
    reg_value = base->wdg_ctrl;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_ctrl:0x%x\n",reg_value);
    reg_value = base->wdg_wtc;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_wtc:0x%x\n",reg_value);
    reg_value = base->wdg_wrc_ctl;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_wrc_ctl:0x%x\n",reg_value);
    reg_value = base->wdg_wrc_val;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_wrc_val:0x%x\n",reg_value);
    reg_value = base->wdg_wrc_seq;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_wrc_seq:0x%x\n",reg_value);
    reg_value = base->wdg_rst_ctl;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_rst_ctl:0x%x\n",reg_value);
    reg_value = base->wdg_ext_rst_ctl;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_ext_rst_ctl:0x%x\n",reg_value);
    reg_value = base->wdg_cnt;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_cnt:0x%x\n",reg_value);
    reg_value = base->wdg_tsw;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_tsw:0x%x\n",reg_value);
    reg_value = base->wdg_int;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_int:0x%x\n",reg_value);
    reg_value = base->wdg_lock;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_lock:0x%x\n",reg_value);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "#######################################\n");
    return true;
}

//*****************************************************************************
//
//! watchdog test interrupt handler for the wdg timer interrupt.
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
enum handler_return watchdog_irq_test_handle(void *arg)
{
	unsigned int instance = (uintptr_t)arg;
	LTRACEF("watchdog_irq_test_handle instance:%d\n",instance);
	//add reset or mendump func
	g_watchdog_int_count = 100;
	return INT_NO_RESCHEDULE;
}

//*****************************************************************************
//
//! Registers an interrupt handler for the wdg test timer interrupt.
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
bool wdg_test_Int_register(wdg_reg_type_t *base,int_handler call_func,bool overflow_int)
{
    uint32_t refresh_mechanism = 0;
    uint32_t wdg_int_num = 0;
    // Check the arguments.
    ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_test_Int_register:\n");
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
    register_int_handler(wdg_int_num,call_func,base);
    unmask_interrupt(wdg_int_num);
    return true;
}

//*****************************************************************************
//
//! Unregisters an interrupt handler for the wdg test timer interrupt.
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
bool wdg_test_int_unregister(wdg_reg_type_t *base,bool overflow_int)
{
    uint32_t refresh_mechanism = 0;
    uint32_t wdg_int_num = 0;
    // Check the arguments.
    ASSERT_PARAMETER(base);

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_test_int_unregister:\n");
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
//! wdg_test_get_default_config .
//! \config is Pointer to WDG config structure
//! This function initializes the WDG configure structure to default value.
//!
//! \return
//
//*****************************************************************************
void wdg_test_get_default_config(wdg_config_t *wdg_config)
{
    if(!wdg_config){
        LTRACEF("config paramenter error !!\n");
        return;
    }
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_test_get_default_config start................\n");
    // Watchdog global control register
    wdg_config->wdg_ctrl_config.clockSource = wdg_lp_clk;
    wdg_config->wdg_ctrl_config.enableAutostart = false;
    wdg_config->wdg_ctrl_config.enableDebugmode = false;
    wdg_config->wdg_ctrl_config.enableSelftest = false;
    wdg_config->wdg_ctrl_config.enableSoftRest = true;
    wdg_config->wdg_ctrl_config.enableSrcSelect = 0x1U;
    wdg_config->wdg_ctrl_config.enableWdg = false;
    wdg_config->wdg_ctrl_config.prescaler = 0x0U;
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
    wdg_config->wdg_reset_cfg.enableSysReset = true;
    wdg_config->wdg_reset_cfg.enableWdgResetEn = false;
    wdg_config->wdg_reset_cfg.plusRstWind = 0x0U;
    wdg_config->wdg_reset_cfg.SysRstMode = 0x0U;
    wdg_config->wdg_reset_cfg.wdgResetCnt = 0x0U;
    // Watchdog external reset control
    wdg_config->wdg_ext_reset_cfg.enableSysExtReset = false;
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
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_test_get_default_config end................\n");
    return;
}

//*****************************************************************************
//test1
//! WatchdogReadonlyRegCheckTest
//!
//! \param ulBase is the base address of the wdg timer module.
//!
//! This function is test watchdog read only register read it must match with expected value with the reset value
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool wdg_readonlyreg_check_test(wdg_reg_type_t* base)
{
    bool ret = true;
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_readonlyreg_check_test start................\n");
    ASSERT_PARAMETER(base);

    dump_all_reg_for_test(base);
    // check .wdg en sta reset is 0x0
    if((base->wdg_ctrl & WDG_CTRL_WDG_EN_STA_MASK) != 0x0){
        uint32_t temp_value = base->wdg_ctrl;
        LTRACEF("watchdog wdg en sta reset value is error:0x%x\n",temp_value);
        ret = false;
    }

    // check .wdg int sta reset is 0x0
    if(((base->wdg_int) & (WDG_INT_ILL_WIN_REFE_INT_STA_MASK|WDG_INT_ILL_SEQ_REFE_INT_STA_MASK|WDG_INT_OVERFLOW_INT_STA_MASK)) != 0x0){
        uint32_t temp_value = base->wdg_int;
        LTRACEF("watchdog wdg int sta reset value is error:0x%x\n",temp_value);
        ret = false;
    }

    /*#############################################*/
    //test wdg cnt
    if(((base->wdg_cnt) & 0xffffffff) != 0x00000000){
        uint32_t temp_value = base->wdg_cnt;
        LTRACEF("watchdog wdg cnt reset value is error:0x%x !=0x00000000 \n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog read only register check ok.........!!\n");
    dump_all_reg_for_test(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_readonlyreg_check_test end................\n");

    return ret;
}

//*****************************************************************************
//test2
//! wdg_rw_reg_check_test
//!
//! \param ulBase is the base address of the wdg timer module.
//!
//! This function is test watchdog rw register read it must read data  should be equal with the write data
//!	use 0xffffffff or 0x00000000 write and immediately read it should be equal with the write data
//!
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool wdg_rw_reg_check_test(wdg_reg_type_t* base)
{
    bool ret = true;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_rw_reg_check_test start................\n");
    // Check the arguments.
    ASSERT_PARAMETER(base);

    dump_all_reg_for_test(base);
    /*#############################################*/
    //test wdg ctrl
    if(((base->wdg_ctrl) & 0xffff03ff) != 0x003f00c2){
        uint32_t temp_value = base->wdg_ctrl;
        LTRACEF("watchdog wdg ctrl reset value is error:0x%x !=0x003f00c2\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_ctrl &= 0x00000000;
    base->wdg_ctrl |= 0x000003ff;
    if(((base->wdg_ctrl) & 0x000003ff) != 0x000003ff){
        uint32_t temp_value = base->wdg_ctrl;
        LTRACEF("watchdog wdg ctrl write test1 is error:0x%x\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_ctrl &= 0x00000000;
    if(((base->wdg_ctrl) & 0x000003ff) != 0x00000000){
        uint32_t temp_value = base->wdg_ctrl;
        LTRACEF("watchdog wdg ctrl write test2 is error:0x%x\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_ctrl &= 0x00000000;
    base->wdg_ctrl |= 0xffff0000;
    if(((base->wdg_ctrl) & 0xffff0000) != 0xffff0000){
        uint32_t temp_value = base->wdg_ctrl;
        LTRACEF("watchdog wdg ctrl write test3 is error:0x%x\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_ctrl &= 0x00000000;
    if(((base->wdg_ctrl) & 0xffff0000) != 0x00000000){
        uint32_t temp_value = base->wdg_ctrl;
        LTRACEF("watchdog wdg ctrl write test4 is error:0x%x\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }
    /*#############################################*/
    //test wdg wtc
    if((base->wdg_wtc) != 0xffffffff){
        uint32_t temp_value = base->wdg_wtc;
        LTRACEF("watchdog wdg wtc reset value is error:0x%x !=0xffffffff\n",temp_value);
        dump_all_reg_for_test(base);
        return false;
    }

    base->wdg_wtc &= 0x00000000;
    base->wdg_wtc |= 0xffffffff;
    if((base->wdg_wtc) != 0xffffffff){
        uint32_t temp_value = base->wdg_wtc;
        LTRACEF("watchdog wdg wtc write value is error:0x%x !=0xffffffff\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_wtc &= 0x00000000;
    if((base->wdg_wtc) != 0x00000000){
        uint32_t temp_value = base->wdg_wtc;
        LTRACEF("watchdog wdg wtc write value is error:0x%x !=0x00000000\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    /*#############################################*/
    //test wdg wrc ctl
    if(((base->wdg_wrc_ctl) & 0x0000000f) != 0x00000001){
        uint32_t temp_value = base->wdg_wrc_ctl;
        LTRACEF("watchdog wdg wrc ctl reset value is error:0x%x !=0x00000001\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_wrc_ctl &= 0x00000000;
    base->wdg_wrc_ctl |= 0x0000000f;
    if(((base->wdg_wrc_ctl) & 0x0000000f) != 0x00000007){
        uint32_t temp_value = base->wdg_wrc_ctl;
        LTRACEF("watchdog wdg wtc write value is error:0x%x !=0x00000007\n",temp_value);
        dump_all_reg_for_test(base);
        return false;
    }

    base->wdg_wrc_ctl &= 0x00000000;
    if(((base->wdg_wrc_ctl) & 0x0000000f) != 0x00000000){
        uint32_t temp_value = base->wdg_wrc_ctl;
        LTRACEF("watchdog wdg wtc write value is error:0x%x !=0x00000000\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    /*#############################################*/
    //test wdg wrc val
    if(((base->wdg_wrc_val) & 0xffffffff) != 0x00000000){
        uint32_t temp_value = base->wdg_wrc_val;
        LTRACEF("watchdog wdg wrc val reset value is error:0x%x !=0x00000000\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_wrc_val &= 0x00000000;
    base->wdg_wrc_val |= 0xffffffff;
    if((base->wdg_wrc_val) != 0xffffffff){
        uint32_t temp_value = base->wdg_wrc_val;
        LTRACEF("watchdog wdg wrc val write value is error:0x%x !=0xffffffff\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_wrc_val &= 0x00000000;
    if(((base->wdg_wrc_val) & 0xffffffff) != 0x00000000){
        uint32_t temp_value = base->wdg_wrc_val;
        LTRACEF("watchdog wdg wrc val write value is error:0x%x !=0x00000000\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    /*#############################################*/
    //test wdg wrc seq
    if(((base->wdg_wrc_seq) & 0xffffffff) != 0x00000000){
        uint32_t temp_value = base->wdg_wrc_seq;
        LTRACEF("watchdog wdg wrc seq reset value is error:0x%x !=0x00000000\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_wrc_seq &= 0x00000000;
    base->wdg_wrc_seq |= 0xffffffff;
    if((base->wdg_wrc_seq) != 0xffffffff){
        uint32_t temp_value = base->wdg_wrc_seq;
        LTRACEF("watchdog wdg wrc seq write value is error:0x%x !=0xffffffff\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_wrc_seq &= 0x00000000;
    if(((base->wdg_wrc_seq) & 0xffffffff) != 0x00000000){
        uint32_t temp_value = base->wdg_wrc_seq;
        LTRACEF("watchdog wdg wrc seq write value is error:0x%x !=0x00000000\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    /*#############################################*/
    //test wdg rst ctl
    if(((base->wdg_rst_ctl) & 0x07f7ffff) != 0x000300ff){
        uint32_t temp_value = base->wdg_rst_ctl;
        LTRACEF("watchdog wdg rst ctl reset value is error:0x%x !=0x000300ff \n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_rst_ctl &= 0x00000000;
    base->wdg_rst_ctl |= 0x07f7ffff;
    if(((base->wdg_rst_ctl) & 0x07f7ffff) != 0x07f7ffff){
        uint32_t temp_value = base->wdg_rst_ctl;
        LTRACEF("watchdog wdg rst ctl write value is error:0x%x !=0x07f7ffff\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_rst_ctl &= 0x00000000;
    if(((base->wdg_rst_ctl) & 0x07f7ffff) != 0x00000000){
        uint32_t temp_value = base->wdg_rst_ctl;
        LTRACEF("watchdog wdg rst ctl write value is error:0x%x !=0x00000000\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    /*#############################################*/
    //test wdg rst ctl

    if(((base->wdg_ext_rst_ctl) & 0x07f3ffff) != 0x000000ff){
        uint32_t temp_value = base->wdg_ext_rst_ctl;
        LTRACEF("watchdog wdg rst ctl reset value is error:0x%x !=0x000000ff \n",temp_value);
        dump_all_reg_for_test(base);
        return false;
    }

    base->wdg_ext_rst_ctl &= 0x00000000;
    base->wdg_ext_rst_ctl |= 0x07f3ffff;
    if(((base->wdg_ext_rst_ctl) & 0x07f3ffff) != 0x07f3ffff){
        uint32_t temp_value = base->wdg_ext_rst_ctl;
        LTRACEF("watchdog wdg rst ctl write value is error:0x%x !=0x07f3ffff\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_ext_rst_ctl &= 0x00000000;
    if(((base->wdg_ext_rst_ctl) & 0x07f3ffff) != 0x00000000){
        uint32_t temp_value = base->wdg_ext_rst_ctl;
        LTRACEF("watchdog wdg rst ctl write value is error:0x%x !=0x00000000\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    /*#############################################*/
    //test wdg tsw write only

    /*#############################################*/
    //test wdg int
    if(((base->wdg_int) & 0x0000001f) != 0x00000000){
        uint32_t temp_value = base->wdg_int;
        LTRACEF("watchdog wdg int reset value is error:0x%x !=0x00000000 \n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_int &= 0x00000000;
    base->wdg_int |= 0x000001c7;
    if(((base->wdg_int) & 0x000001c7) != 0x00000007){
        uint32_t temp_value = base->wdg_int;
        LTRACEF("watchdog wdg int write value is error:0x%x !=0x00000007\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    base->wdg_int &= 0x00000000;
    if(((base->wdg_int) & 0x0000001f) != 0x00000000){
        uint32_t temp_value = base->wdg_int;
        LTRACEF("watchdog wdg int write value is error:0x%x !=0x00000000\n",temp_value);
        dump_all_reg_for_test(base);
        ret = false;
    }

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_rw_reg_check_test end................ret:%d\n",ret);

    return ret;
}

//*****************************************************************************
//case1.2 1.3  test3
//! wdg_self_test
//!
//! \param ulBase is the base address of the wdg timer module.
//!
/*i.Enable the self test mode by a global selftest_en signal.32-bit wdg will be divided into 4*8 timer(WDG_S0--WDG_S3)
	each 8 bit timer has a terminal value(TV_S0--TV_S3)for WDG_S0-TV_S0 and so on.
	all the wdg timer values are AND and that should be used as a WDG overflow.timer restart happens only when WDG overflow
	1.programming the terminal value.
	2.WDG configure to genarate system reset request.
	3.check restart happens after terminal value.
*/
//!
//! \return true is check ok,else return false.
//need global reset enable for test
//*****************************************************************************
bool wdg_self_test(wdg_reg_type_t* base,const wdg_config_t *wdg_config)
{
    bool ret = true;
    uint32_t polling_count = 0;
    vaddr_t sem_base = (vaddr_t)SEM_BASE;
    uint32_t reg_read = 0;

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_self_test start....................\n");
    // Check the arguments.
    ASSERT_PARAMETER(base);
    if(wdg_config == NULL)
    {
        LTRACEF("base paramenter error base:\n");
        return false;
    }

    polling_count = wdg_get_ext_reset_cnt(base);
    if(polling_count == 0xff){
        wdg_clear_ext_reset_cnt(base);
        polling_count = wdg_get_ext_reset_cnt(base);
    }
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_self_test polling_count:%d\n",polling_count);
    if(polling_count == 0){
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_self_test1 polling_count:%d\n",polling_count);
        wdg_init(base,wdg_config);
        //wdg_enable_Interrupts(base);
        //case1.2
        //set wtc val
        base->wdg_wtc &= 0x00000000;
        base->wdg_wtc |= 0x0101010a;
        base->wdg_cnt &= 0x00000000;
        //set wtc self test
        base->wdg_ctrl |= WDG_CTRL_SELFTEST_TRIG_MASK;
        base->wdg_rst_ctl &= ~WDG_RST_CTRL_WDG_RST_EN_MASK;
        //enable sem global int
        reg_read = readl(sem_base);
        reg_read |=0x10000;
        writel(reg_read,sem_base);
        reg_read = readl(sem_base);
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "sem global:0x%x\n",reg_read);

        dump_all_reg_for_test(base);
        /*need sem globale int enable else failed*/
        wdg_enable(base);
        wdg_set_ext_reset_cnt(base,1);
        //polling_count = reg_poll_value(base,WDG_INT, 5, 1,1,10);
        wdg_delay_timeout(base,1);
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_self_test failed\n");
        wdg_disable(base);
        wdg_clear_ext_reset_cnt(base);
        ret = false;
    }else if(polling_count == 1){
        #if 0 //need enable rstgen core reset,dosn't enable gloable reset
        uint32_t reg_read =0;
        reg_read =wdg_get_reset_cnt(base);
        if(reg_read == 0xa){
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_self_test pass reg_read:%d\n",reg_read);
            ret = true;
        }else{
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_self_test failed reg_read:%d\n",reg_read);
            ret = false;
        }
        #endif
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_self_test pass reg_read:%d\n",reg_read);
        ret = true;
        wdg_clear_ext_reset_cnt(base);
        wdg_clear_reset_cnt(base);
    }else{
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_self_test failed\n");
        wdg_clear_ext_reset_cnt(base);
        wdg_clear_reset_cnt(base);
        ret = false;
    }
    dump_all_reg_for_test(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_self_test end....................\n");
    return ret;
}

//*****************************************************************************
//case1.4  test4
//! WatchdogTerminalTest
//!
//! \param ulBase is the base address of the wdg timer module.
//! \timeout watchdog timeout time *** ms.
//!
//! \this func is terminal from register
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool wdg_terminal_test(wdg_reg_type_t* base,const wdg_config_t *wdg_config,uint32_t timeout)
{
    bool ret = false;
    uint32_t polling_count = 0;
    // Check the arguments
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_terminal_test start.....timeout:%d\n",timeout);
    ASSERT_PARAMETER(base);
    if(wdg_config == NULL)
    {
        LTRACEF("base paramenter error base:\n");
        return false;
    }

    polling_count = wdg_get_ext_reset_cnt(base);
    if(polling_count == 0xff){
        wdg_clear_ext_reset_cnt(base);
        polling_count = wdg_get_ext_reset_cnt(base);
    }

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_terminal_test polling_count:%d\n",polling_count);
    if(polling_count == 0){
        //case1.4
        //g_watchdog_int_count = 0;
        wdg_init(base,wdg_config);
        //wdg_enable_Interrupts(base);
        wdg_set_timeout(base,timeout);
        //wdg_test_Int_register(base,&watchdog_irq_test_handle,true);
        dump_all_reg_for_test(base);
        wdg_enable(base);
        wdg_set_ext_reset_cnt(base,1);
        wdg_delay_timeout(base,timeout*2);
        wdg_disable(base);
        dump_all_reg_for_test(base);
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_terminal_test failed.\n");
        wdg_clear_ext_reset_cnt(base);
        ret = false;
        //polling_count = reg_poll_value(base,WDG_INT, 5, 1,1,200);
    }else if(polling_count ==1){
        ret = true;
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_terminal_test pass.\n");
    }else{
        /*test failed*/
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_terminal_test failed.\n");
        wdg_clear_ext_reset_cnt(base);
        ret = false;
    }
    dump_all_reg_for_test(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_terminal_test end....................\n");
    return ret;
}


//*****************************************************************************
//case1.5 test5 wait test.............
//! WatchdogTerminalFromFuseTest
//!
//! \param ulBase is the base address of the wdg timer module.
//! \timeout watchdog timeout time *** ms.
//!
//! \this func is terminal from fuse
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool wdg_terminal_from_fuse_test(wdg_reg_type_t* base,const wdg_config_t *wdg_config,uint32_t timeout)
{
    bool ret = false;
    uint32_t polling_count = 0;
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_terminal_from_fuse_test start.....timeout:%d\n",timeout);

    ASSERT_PARAMETER(base);
    if(wdg_config == NULL)
    {
        LTRACEF("base paramenter error base:\n");
        return false;
    }

    polling_count = wdg_get_ext_reset_cnt(base);
    if(polling_count == 0xff){
        wdg_clear_ext_reset_cnt(base);
        polling_count = wdg_get_ext_reset_cnt(base);
    }

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_terminal_from_fuse_test polling_count:%d\n",polling_count);
    if(polling_count == 0){
        //case1.5
        //g_watchdog_int_count = 0;
        wdg_init(base,wdg_config);
        //wdg_enable_Interrupts(base);
        //set terminal from fuse
        base->wdg_ctrl &= ~(WDG_CTRL_WDG_EN_SRC_MASK|WDG_CTRL_WTC_SRC_MASK);
        wdg_set_timeout(base,timeout);
        //wdg_test_Int_register(base,&watchdog_irq_test_handle,true);
        wdg_enable(base);
        wdg_set_ext_reset_cnt(base,1);
        wdg_delay_timeout(base,timeout*2);
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_terminal_from_fuse_test failed.\n");
        wdg_clear_ext_reset_cnt(base);
        wdg_disable(base);
        ret = false;
    }else if(polling_count ==1){
        ret = true;
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_terminal_from_fuse_test pass.\n");
    }else{
        /*test failed*/
        ret = false;
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_terminal_from_fuse_test failed.\n");
    }
    wdg_clear_ext_reset_cnt(base);
    dump_all_reg_for_test(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_terminal_from_fuse_test end....................\n");
    return ret;
}

//*****************************************************************************
//case1.6 test6
//! wdg_reset_control_restart_test
//!
//! \param ulBase is the base address of the wdg timer module.
//!
	/*1.programming the int_rst_en/ext_rst_en set to 1.
	2.programming the int_rst_mode/ext_rst_mode as a level or pulse mode.
	3.programming the pulse width.
	4.rst_cnt/ext_rst_cnt will be programmed for the time b/w wdg overflow and int_rst_req/ext_rst_req_b reset trigger.
	5.programming the wdg_rst_en is 1 in wdg rst ctl register then WDG module must be restarted when int_rst_req done.

	WDG module has to restart
	*/
//! \this func is terminal from fuse
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool wdg_reset_control_restart_test(wdg_reg_type_t* base,const wdg_config_t *wdg_config,uint32_t timeout)
{
    bool ret = true;
    //case1.6
    uint32_t polling_count = 0;
    uint32_t polling_count_reset_cnt = 0;
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_reset_control_restart_test start.....timeout:%d\n",timeout);
    ASSERT_PARAMETER(base);
    if(wdg_config == NULL)
    {
        LTRACEF("base paramenter error base:\n");
        return false;
    }

    polling_count = wdg_get_ext_reset_cnt(base);
    polling_count_reset_cnt = wdg_get_reset_cnt(base);
    if(polling_count == 0xff){
        wdg_clear_ext_reset_cnt(base);
        polling_count = wdg_get_ext_reset_cnt(base);
    }

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_reset_control_restart_test polling_count:%d\n",polling_count_reset_cnt);
    if(polling_count == 0){
        //g_watchdog_int_count = 0;
        // 1.programming the int_rst_en/ext_rst_en set to 1.
        wdg_init(base,wdg_config);
        //wdg_enable_Interrupts(base);
        // 2.programming the int_rst_mode/ext_rst_mode as a level or pulse mode. now set pulse
        // 3.programming the pulse width.
        base->wdg_rst_ctl |= (WDG_RST_CTRL_INT_RST_MODE_MASK | 0xff000000);
        base->wdg_ext_rst_ctl |= (WDG_EXT_RST_CTRL_INT_RST_MODE_MASK | 0xff000000);
        base->wdg_rst_ctl |= WDG_RST_CTRL_WDG_RST_EN_MASK;
        wdg_set_timeout(base,timeout);
        //wdg_test_Int_register(base,&watchdog_irq_test_handle,true);
        wdg_enable(base);
        wdg_set_ext_reset_cnt(base,0x8);
        wdg_set_reset_cnt(base,0x8);
        wdg_delay_timeout(base,timeout*2);
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_reset_control_restart_test failed.\n");
        wdg_clear_ext_reset_cnt(base);
        wdg_clear_reset_cnt(base);
        wdg_disable(base);
        ret = false;
    }else if(polling_count_reset_cnt == 0xff){
        /*test failed*/
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_reset_control_restart_test pass.\n");
        wdg_clear_ext_reset_cnt(base);
        wdg_clear_reset_cnt(base);
        ret = true;
    }else{
        /*test failed*/
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_reset_control_restart_test failed.\n");
        wdg_clear_ext_reset_cnt(base);
        wdg_clear_reset_cnt(base);
        ret = false;
    }
    dump_all_reg_for_test(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_reset_control_restart_test end....................\n");
    return ret;
}

//*****************************************************************************
//case1.7 test7
//! wdg_reset_control_donot_restart_test
//!
//! \param ulBase is the base address of the wdg timer module.
//! \param timeout watchdog timeout time
//!
	/*1.programming the int_rst_en/ext_rst_en set to 1.
	2.programming the int_rst_mode/ext_rst_mode as a level or pulse mode.
	3.programming the pulse width.
	4.rst_cnt/ext_rst_cnt will be programmed for the time b/w wdg overflow and int_rst_req/ext_rst_req_b reset trigger.
	5.programming the wdg_rst_en is 0 in wdg rst ctl register then WDG module doesn't restarted when int_rst_req done.

	WDG module doesn't restart
	*/
//! \this func is terminal from fuse
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool wdg_reset_control_donot_restart_test(wdg_reg_type_t* base,const wdg_config_t *wdg_config,uint32_t timeout)
{
    bool ret = true;
    //case1.6
    uint32_t polling_count = 0;
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_reset_control_restart_test start.....timeout:%d\n",timeout);
    ASSERT_PARAMETER(base);
    if(wdg_config == NULL)
    {
        LTRACEF("base paramenter error base:\n");
        return false;
    }

    polling_count = wdg_get_ext_reset_cnt(base);
    if(polling_count == 0xff){
        wdg_clear_ext_reset_cnt(base);
        polling_count = wdg_get_ext_reset_cnt(base);
    }

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_reset_control_donot_restart_test polling_count:%d\n",polling_count);
    if(polling_count == 0){
        //g_watchdog_int_count = 0;
        // 1.programming the int_rst_en/ext_rst_en set to 1.
        wdg_init(base,wdg_config);
        //wdg_enable_Interrupts(base);
        // 2.programming the int_rst_mode/ext_rst_mode as a level or pulse mode. now set pulse
        // 3.programming the pulse width.
        base->wdg_rst_ctl |= (WDG_RST_CTRL_INT_RST_MODE_MASK | 0xff000000);
        base->wdg_rst_ctl |= (WDG_RST_CTRL_INT_RST_MODE_MASK | 0xff000000);
        base->wdg_rst_ctl &= ~WDG_RST_CTRL_WDG_RST_EN_MASK;
        wdg_set_timeout(base,timeout);
        //wdg_test_Int_register(base,&watchdog_irq_test_handle,true);
        wdg_enable(base);
        wdg_set_reset_cnt(base,0x8);
        wdg_set_ext_reset_cnt(base,0x8);
        wdg_delay_timeout(base,timeout*2);
        wdg_disable(base);
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_reset_control_restart_test pass.\n");
        wdg_clear_ext_reset_cnt(base);
        wdg_clear_reset_cnt(base);
        ret = false;
    }else if(polling_count == 0x8){
        /*test failed*/
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_reset_control_restart_test success.\n");
        wdg_clear_ext_reset_cnt(base);
        wdg_clear_reset_cnt(base);
        ret = true;
    }else{
        /*test failed*/
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_reset_control_restart_test failed.\n");
        wdg_clear_ext_reset_cnt(base);
        wdg_clear_reset_cnt(base);
        ret = false;
    }
    dump_all_reg_for_test(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_reset_control_restart_test end....................\n");
    return ret;
}

//*****************************************************************************
//case1.8   test8
//! wdg_mode1_refresh_test
//!
//! \param ulBase is the base address of the wdg timer module.
//! \param timeout watchdog timeout time ms
//!
	/*1.programming the window_timeout.
	2.check whether the refresh event is released before or after the window timeout.

	check if the refresh occurs above the window timeout it has to genarate a IRQ or not
	*/
//! \this func is test mode1 refresh
//! \return true is test ok,else return false.
//
//*****************************************************************************
bool wdg_mode1_refresh_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout)
{
    bool ret = true;
    uint32_t polling_count = 0;
    uint32_t watchdog_refresh_count = 3;
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode1_refresh_test start.....timeout:%d\n",timeout);
    ASSERT_PARAMETER(base);
    if(wdg_config == NULL)
    {
        LTRACEF("base paramenter error base:\n");
        return false;
    }

    //case1.8
    //g_watchdog_int_count = 0;
    // 1.programming the int_rst_en/ext_rst_en set to 1.
    polling_count = wdg_get_ext_reset_cnt(base);
    if(polling_count == 0xff){
        wdg_clear_ext_reset_cnt(base);
        polling_count = wdg_get_ext_reset_cnt(base);
    }
    timeout = 100;
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode1_refresh_test polling_count:%d\n",polling_count);
    if(polling_count == 0){
        wdg_init(base,wdg_config);
        //wdg_enable_Interrupts(base);
        wdg_config->wdg_refresh_config.wdgModeSelect = wdg_mechanism_mode1;
        wdg_refesh_mechanism_select(base,wdg_config);
        wdg_set_timeout(base,timeout);
        //wdg_test_Int_register(base,&watchdog_irq_test_handle,true);
        dump_all_reg_for_test(base);
        wdg_enable(base);
        wdg_refresh(base);
        wdg_set_ext_reset_cnt(base,1);
        while(watchdog_refresh_count>0){
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode1_refresh_test watchdog_refresh_count:%d\n",watchdog_refresh_count);
            //refresh watchdog & /*watchdog timeout status is fasle*
            wdg_refresh(base);
            wdg_delay_timeout(base,timeout/2);
            watchdog_refresh_count--;
        }
        wdg_refresh(base);
        wdg_disable(base);
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog WatchdogMode1RefreshTest success!!\n");
        wdg_clear_ext_reset_cnt(base);
        ret = true;
    }else{
        /*test failed*/
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode1_refresh_test failed.\n");
        wdg_clear_ext_reset_cnt(base);
        ret = false;
    }
    dump_all_reg_for_test(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode1_refresh_test end....................\n");
    return ret;
}

//*****************************************************************************
//case1.9   test9
//! wdg_mode2_refresh_test
//!
//! \param ulBase is the base address of the wdg timer module.
//! \param timeout watchdog timeout time ms
//!
	/*1.programming the window_low_limit.
	2.check whether the refresh event is released before or after the window low limit.

	check
	1.if the refresh event occurs before the refresh window.
	2.it has to genarate an IRQ.
	3.after the preconfigured delay system reset will be triggerd or not.
	*/
//! \this func is test mode2 refresh if refresh time > window limit refresh success,else refresh failed
//! \return true is test ok,else return false.
//
//*****************************************************************************
bool wdg_mode2_refresh_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout)
{
    bool ret = true;
    uint32_t polling_count = 0;
    uint32_t watchdog_refresh_count = 2;
    uint32_t l_wtc_val = 0;
    uint32_t int_flag = 0;
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode2_refresh_test start.....timeout:%d\n",timeout);
    ASSERT_PARAMETER(base);
    if(wdg_config == NULL)
    {
        LTRACEF("base paramenter error base:\n");
        return false;
    }

    polling_count = wdg_get_ext_reset_cnt(base);
    if(polling_count == 0xff){
        wdg_clear_ext_reset_cnt(base);
        polling_count = wdg_get_ext_reset_cnt(base);
    }

    timeout = 100;

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode2_refresh_test polling_count:%d\n",polling_count);
    if(polling_count == 0){
        //case1.9
        //g_watchdog_int_count = 0;
        // Watchdog interrupt
        wdg_config->wdg_int_cfg.ill_win_refr_int_clr = false;
        wdg_config->wdg_int_cfg.ill_win_refr_int_en = true;
        wdg_config->wdg_int_cfg.overflow_int_clr = true;
        wdg_config->wdg_int_cfg.overflow_int_en = true;
        wdg_init(base,wdg_config);
        //wdg_enable_Interrupts(base);
        wdg_config->wdg_refresh_config.wdgModeSelect = wdg_mechanism_mode2;
        wdg_refesh_mechanism_select(base,wdg_config);
        wdg_set_timeout(base,timeout);
        //get wtc val
        l_wtc_val = (base->wdg_wtc);
        // need config limit window
        //set windows low limit
        base->wdg_wrc_val &= 0x0;
        base->wdg_wrc_val |= l_wtc_val/4;
        //wdg_test_Int_register(base,&watchdog_irq_test_handle,true);
        dump_all_reg_for_test(base);
        wdg_enable(base);
        while((watchdog_refresh_count>0) && ((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == 0)){
            //refresh watchdog & /*watchdog timeout status is fasle*
            wdg_delay_timeout(base,(timeout/2));
            wdg_refresh(base);
            int_flag = wdg_get_status_flag(base);
            dump_all_reg_for_test(base);
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_refresh_test int_flag:0x%x,watchdog_refresh_count:%d\n",int_flag,watchdog_refresh_count);
            watchdog_refresh_count--;
        }

        if((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == WDG_INT_ILL_WIN_REFE_INT_STA_MASK){
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_refresh_test failed!!\n");
            ret = false;
        }

        if(ret){
            watchdog_refresh_count = 5;
            while((watchdog_refresh_count>0) && ((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == 0)){
                //refresh watchdog & /*watchdog timeout status is fasle*
                //wdg_delay_timeout(base,(timeout/50));
                wdg_refresh(base);
                int_flag = wdg_get_status_flag(base);
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_refresh_test2 int_flag:0x%x,watchdog_refresh_count:%d\n",int_flag,watchdog_refresh_count);
                watchdog_refresh_count--;
            }

            if((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == WDG_INT_ILL_WIN_REFE_INT_STA_MASK){
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_refresh_test success!!\n");
                ret = true;
            }else{
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_refresh_test failed!!\n");
                ret = false;
            }
            wdg_clear_ext_reset_cnt(base);
        }
    }else{
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_refresh_test failed!!\n");
        wdg_clear_ext_reset_cnt(base);
        ret = false;
    }
    wdg_disable(base);
    dump_all_reg_for_test(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode2_refresh_test end....................\n");
    return ret;
}

//*****************************************************************************
//case2.0   test10
//! wdg_mode3_refresh_test
//!
//! \param ulBase is the base address of the wdg timer module.
//! \param timeout watchdog timeout time ms
//!
	/*1.CPU reads the wdg counter value and record that one as a timestamp as TS_r.
	2.cpu programme that one into the TS_w register.
	3.need to configure the delta value.
	4.if i.TS_r and TS_w are equal.
	ii.checking the difference between wdg_conter value and TS_w if the wdg_counter value-TS_w is less then the preconfigured value delta value both conditions are true means it is validate,otherwise it has to genarate an interrupt.

	check
	case1:if both the conditions are valid means it does not generate an interrupt.
	case2:if atleast one is not validate means it has too genarate an interrupt.

	//1.read wdg_cnt and record timestamp write to TSR(hw record last read  wdg_cnt)
	//2.do other process ........
	//3.wirte TSR to wdg_tsw
	//if wdg_count - wdg_tsw < delt refresh success else refresh failed
	*/
//! \this func is test mode3 refresh.
//! \return true is test ok,else return false.
//
//*****************************************************************************
bool wdg_mode3_refresh_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout)
{
    bool ret = true;
    uint32_t polling_count = 0;
    uint32_t watchdog_refresh_count = 5;
    uint32_t l_wtc_val = 0;
    uint32_t int_flag = 0;
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode3_refresh_test start.....timeout:%d\n",timeout);
    ASSERT_PARAMETER(base);
    if(wdg_config == NULL)
    {
        LTRACEF("base paramenter error base:\n");
        return false;
    }

    //case2.0
    //g_watchdog_int_count = 0;
    polling_count = wdg_get_ext_reset_cnt(base);
    if(polling_count == 0xff){
        wdg_clear_ext_reset_cnt(base);
        polling_count = wdg_get_ext_reset_cnt(base);
    }

    timeout = 400;

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode3_refresh_test polling_count:%d\n",polling_count);
    if(polling_count == 0){
        wdg_config->wdg_int_cfg.ill_seq_refr_int_clr = false;
        wdg_config->wdg_int_cfg.ill_seq_refr_int_en = true;
        wdg_config->wdg_int_cfg.ill_win_refr_int_clr = false;
        wdg_config->wdg_int_cfg.ill_win_refr_int_en = true;
        wdg_config->wdg_int_cfg.overflow_int_clr = true;
        wdg_config->wdg_int_cfg.overflow_int_en = true;
        wdg_init(base,wdg_config);
        //wdg_enable_Interrupts(base);
        wdg_set_timeout(base,timeout);

        //get wtc val
        l_wtc_val = (base->wdg_wtc);
        //need config delt time
        base->wdg_wrc_seq &= 0x0;
        base->wdg_wrc_seq |= l_wtc_val/2;
        wdg_config->wdg_refresh_config.wdgModeSelect = wdg_mechanism_mode3;
        wdg_refesh_mechanism_select(base,wdg_config);
        //wdg_test_Int_register(base,&watchdog_irq_test_handle,true);
        dump_all_reg_for_test(base);
        wdg_enable(base);
        while((watchdog_refresh_count>0) && ((int_flag & WDG_INT_ILL_SEQ_REFE_INT_STA_MASK) == 0)){
            //refresh watchdog & /*watchdog timeout status is fasle*
            //wdg_delay_timeout(base,1);
            wdg_record_wdgcnt(base);
            //wdg_delay_timeout(base,timeout/10);
            wdg_refresh(base);
            int_flag = wdg_get_status_flag(base);
            dump_all_reg_for_test(base);
            watchdog_refresh_count--;
        }

        if((int_flag & WDG_INT_ILL_SEQ_REFE_INT_STA_MASK) == WDG_INT_ILL_SEQ_REFE_INT_STA_MASK)
        {
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode3_refresh_test failed!!\n");
            ret = false;
        }else{
            watchdog_refresh_count = 2;
            while((watchdog_refresh_count>0) && ((int_flag & WDG_INT_ILL_SEQ_REFE_INT_STA_MASK) == 0)){
                //refresh watchdog & /*watchdog timeout status is fasle*
                wdg_record_wdgcnt(base);
                //wdg_delay_timeout(base,timeout*3/4);
                wdg_delay_timeout(base,1);
                wdg_refresh(base);
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode3_refresh_test delay\n");
                int_flag = wdg_get_status_flag(base);
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode3_refresh_test int_flag:0x%x\n",int_flag);
                dump_all_reg_for_test(base);
                watchdog_refresh_count--;
            }

            if((int_flag & WDG_INT_ILL_SEQ_REFE_INT_STA_MASK) == WDG_INT_ILL_SEQ_REFE_INT_STA_MASK)
            {
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode3_refresh_test success!!\n");
                ret = true;
            }else{
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode3_refresh_test failed!!\n");
                ret = false;
            }
        }
    }else{
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode3_refresh_test failed!!\n");
        wdg_clear_ext_reset_cnt(base);
        ret = false;
    }
    wdg_disable(base);
    wdg_clear_ext_reset_cnt(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode3_refresh_test end....................\n");
    return ret;
}

//*****************************************************************************
//case2.1 need sem int globale enable   test11
//! wdg_mode2_window_reset_test
//!
//! \param ulBase is the base address of the wdg timer module.
//! \timeout watchdog timeout time *** ms.
	/*1.programming the window_reset trigger.
	2.check whether the refresh event is release before or after the window reset trigger.

	check
	1.if the refresh occurs near to the window reset trigger.
	2.it has to genarate a fail safe IRQ.
	3.after the prommaged delay system reset trigger.
	*/
//!
//! \this func is terminal from register
//! \return true is check ok,else return false.
//
//*****************************************************************************
bool wdg_mode2_window_reset_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout)
{
    bool ret = true;
    uint32_t polling_count = 0;
    uint32_t watchdog_refresh_count = 3;
    uint32_t l_wtc_val = 0;
    uint32_t int_flag = 0;
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode2_window_reset_test start.....timeout:%d\n",timeout);
    ASSERT_PARAMETER(base);
    if(wdg_config == NULL)
    {
        LTRACEF("base paramenter error base:\n");
        return false;
    }

    polling_count = wdg_get_ext_reset_cnt(base);
    if(polling_count == 0xff){
        wdg_clear_ext_reset_cnt(base);
        polling_count = wdg_get_ext_reset_cnt(base);
    }

    timeout = 100;

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode2_window_reset_test polling_count:%d\n",polling_count);
    if(polling_count == 0){
        //case1.9
        //g_watchdog_int_count = 0;
        // Watchdog interrupt
        wdg_config->wdg_int_cfg.ill_win_refr_int_clr = false;
        wdg_config->wdg_int_cfg.ill_win_refr_int_en = true;
        wdg_config->wdg_int_cfg.overflow_int_clr = true;
        wdg_config->wdg_int_cfg.overflow_int_en = true;
        wdg_init(base,wdg_config);
        //wdg_enable_Interrupts(base);
        wdg_config->wdg_refresh_config.wdgModeSelect = wdg_mechanism_mode2;
        wdg_refesh_mechanism_select(base,wdg_config);
        wdg_set_timeout(base,timeout);
        //get wtc val
        l_wtc_val = (base->wdg_wtc);
        // need config limit window
        //set windows low limit
        base->wdg_wrc_val &= 0x0;
        base->wdg_wrc_val |= l_wtc_val/4;
        //wdg_test_Int_register(base,&watchdog_irq_test_handle,true);
        dump_all_reg_for_test(base);
        wdg_enable(base);
        while((watchdog_refresh_count>0) && ((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == 0)){
            //refresh watchdog & /*watchdog timeout status is fasle*
            wdg_delay_timeout(base,(timeout/2));
            wdg_refresh(base);
            int_flag = wdg_get_status_flag(base);
            dump_all_reg_for_test(base);
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_window_reset_test int_flag:0x%x,watchdog_refresh_count:%d\n",int_flag,watchdog_refresh_count);
            watchdog_refresh_count--;
        }

        if((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == WDG_INT_ILL_WIN_REFE_INT_STA_MASK){
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_window_reset_test failed!!\n");
            ret = false;
        }

        if(ret){
            watchdog_refresh_count = 5;
            while((watchdog_refresh_count>0) && ((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == 0)){
                //refresh watchdog & /*watchdog timeout status is fasle*
                //wdg_delay_timeout(base,(timeout/5));
                wdg_refresh(base);
                int_flag = wdg_get_status_flag(base);
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_window_reset_test int_flag:0x%x,watchdog_refresh_count:%d\n",int_flag,watchdog_refresh_count);
                watchdog_refresh_count--;
            }

            if((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == WDG_INT_ILL_WIN_REFE_INT_STA_MASK){
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_window_reset_test success!!\n");
                ret = true;
            }else{
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_window_reset_test failed!!\n");
                ret = false;
            }

            if(ret){
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_window_reset_test ok!!\n");
                wdg_set_ext_reset_cnt(base,1);
                wdg_delay_timeout(base,(timeout*2));
            }

            ret = false;
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_window_reset_test failed!!\n");
            wdg_clear_ext_reset_cnt(base);
        }
    }else if(polling_count == 1){
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_window_reset_test success\n");
        wdg_clear_ext_reset_cnt(base);
        ret = true;
    }else{
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_window_reset_test failed!!\n");
        wdg_clear_ext_reset_cnt(base);
        ret = false;
    }
    wdg_disable(base);
    dump_all_reg_for_test(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode2_window_reset_test end....................\n");
    return ret;
}
//*****************************************************************************
//case2.3   need sem int globale enable test12
//! wdg_mode2_1_refresh_test
//!
//! \param ulBase is the base address of the wdg timer module.
//! \param timeout watchdog timeout time ms
//!
	/*1.programming the window_low_limit.
	2.check whether the refresh event is released before or after the window low limit. or
	3.sequence based refresh will be combined with direct refresh.

	check
	1.if the refresh event occurs before the refresh window.
	2.it has to genarate an IRQ.
	3.check whether the two refresh occurs or not.
	*/
//! \this func is test mode2 refresh if refresh time > window limit refresh success,else refresh failed
//! \return true is test ok,else return false.
//
//*****************************************************************************
bool wdg_mode2_1_refresh_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout)
{
    bool ret = true;
    uint32_t polling_count = 0;
    uint32_t watchdog_refresh_count = 3;
    uint32_t l_wtc_val = 0;
    uint32_t int_flag = 0;
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode2_1_refresh_test start.....timeout:%d\n",timeout);
    ASSERT_PARAMETER(base);
    if(wdg_config == NULL)
    {
        LTRACEF("base paramenter error base:\n");
        return false;
    }

    polling_count = wdg_get_ext_reset_cnt(base);
    if(polling_count == 0xff){
        wdg_clear_ext_reset_cnt(base);
        polling_count = wdg_get_ext_reset_cnt(base);
    }

    timeout = 100;

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode2_1_refresh_test polling_count:%d\n",polling_count);
    if(polling_count == 0){
        //case1.9
        //g_watchdog_int_count = 0;
        // Watchdog interrupt
        wdg_config->wdg_int_cfg.ill_win_refr_int_clr = false;
        wdg_config->wdg_int_cfg.ill_win_refr_int_en = true;
        wdg_config->wdg_int_cfg.overflow_int_clr = true;
        wdg_config->wdg_int_cfg.overflow_int_en = true;
        wdg_init(base,wdg_config);
        //wdg_enable_Interrupts(base);
        wdg_config->wdg_refresh_config.wdgModeSelect = wdg_mechanism_mode2;
        wdg_refesh_mechanism_select(base,wdg_config);
        wdg_set_timeout(base,timeout);
        //get wtc val
        l_wtc_val = (base->wdg_wtc);
        // need config limit window
        //set windows low limit
        base->wdg_wrc_val &= 0x0;
        base->wdg_wrc_val |= l_wtc_val/4;
        //wdg_test_Int_register(base,&watchdog_irq_test_handle,true);
        dump_all_reg_for_test(base);
        wdg_enable(base);
        while((watchdog_refresh_count>0) && ((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == 0)){
            //refresh watchdog & /*watchdog timeout status is fasle*
            wdg_delay_timeout(base,(timeout/2));
            wdg_refresh(base);
            int_flag = wdg_get_status_flag(base);
            dump_all_reg_for_test(base);
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_1_refresh_test int_flag:0x%x,watchdog_refresh_count:%d\n",int_flag,watchdog_refresh_count);
            watchdog_refresh_count--;
        }

        if((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == WDG_INT_ILL_WIN_REFE_INT_STA_MASK){
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_1_refresh_test failed!!\n");
            ret = false;
        }

        if(ret){
            watchdog_refresh_count = 5;
            while((watchdog_refresh_count>0) && ((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == 0)){
                //refresh watchdog & /*watchdog timeout status is fasle*
                //wdg_delay_timeout(base,(timeout/5));
                wdg_refresh(base);
                int_flag = wdg_get_status_flag(base);
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_1_refresh_test int_flag:0x%x,watchdog_refresh_count:%d\n",int_flag,watchdog_refresh_count);
                watchdog_refresh_count--;
            }

            if((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == WDG_INT_ILL_WIN_REFE_INT_STA_MASK){
                watchdog_refresh_count = 5;
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_1_refresh_test ok!!\n");
                wdg_config->wdg_refresh_config.wdgModeSelect = wdg_mechanism_mode1;
                wdg_refesh_mechanism_select(base,wdg_config);
                wdg_refresh(base);
                while(watchdog_refresh_count>0){
                    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode2_1_refresh_test watchdog_refresh_count:%d\n",watchdog_refresh_count);
                    wdg_delay_timeout(base,timeout/2);
                    //refresh watchdog & /*watchdog timeout status is fasle*
                    wdg_refresh(base);
                    watchdog_refresh_count--;
                }
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_1_refresh_test success!!\n");
                ret = true;
            }else{
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_1_refresh_test failed!!\n");
                ret = false;
            }
            wdg_clear_ext_reset_cnt(base);
        }
    }else{
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_1_refresh_test failed!!\n");
        wdg_clear_ext_reset_cnt(base);
        ret = false;
    }
    wdg_disable(base);
    dump_all_reg_for_test(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode2_1_refresh_test end....................\n");
    return ret;
}
//*****************************************************************************
//case2.4   need sem int globale enable test13
//! wdg_mode3_2_1_refresh_test
//!
//! \param ulBase is the base address of the wdg timer module.
//! \param timeout watchdog timeout time ms
//!
	/*1.CPU reads the wdg counter value and record that one as a timestamp as TS_r.
	2.cpu programme that one into the TS_w register.
	3.need to configure the delta value.
	4.if i.TS_r and TS_w are equal.
	ii.checking the difference between wdg_conter value and TS_w if the wdg_counter value-TS_w is less then the preconfigured value delta value both conditions are true means it is validate,otherwise it has to genarate an interrupt.

	check
	case1:if both the conditions are valid means it does not generate an interrupt.
	case2:if atleast one is not validate means it has too genarate an interrupt.

	//1.read wdg_cnt and record timestamp write to TSR
	//2.do other process ........
	//3.wirte TSR to wdg_tsw
	//if wdg_count - wdg_tsw < delt refresh success else refresh failed
	*/
//! \this func is test mode3 refresh.
//! \return true is test ok,else return false.
//
//*****************************************************************************
bool wdg_mode3_2_1_refresh_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout)
{
    bool ret = true;
    uint32_t polling_count = 0;
    uint32_t watchdog_refresh_count = 3;
    uint32_t l_wtc_val = 0;
    uint32_t int_flag = 0;
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode3_refresh_test start.....timeout:%d\n",timeout);
    ASSERT_PARAMETER(base);
    if(wdg_config == NULL)
    {
        LTRACEF("base paramenter error base:\n");
        return false;
    }

    //case2.0
    //g_watchdog_int_count = 0;
    polling_count = wdg_get_ext_reset_cnt(base);
    if(polling_count == 0xff){
        wdg_clear_ext_reset_cnt(base);
        polling_count = wdg_get_ext_reset_cnt(base);
    }

    timeout = 400;

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode3_refresh_test polling_count:%d\n",polling_count);
    if(polling_count == 0){
        wdg_config->wdg_int_cfg.ill_seq_refr_int_clr = false;
        wdg_config->wdg_int_cfg.ill_seq_refr_int_en = true;
        wdg_config->wdg_int_cfg.ill_win_refr_int_clr = false;
        wdg_config->wdg_int_cfg.ill_win_refr_int_en = true;
        wdg_config->wdg_int_cfg.overflow_int_clr = false;
        wdg_config->wdg_int_cfg.overflow_int_en = true;
        wdg_init(base,wdg_config);
        //wdg_enable_Interrupts(base);
        wdg_set_timeout(base,timeout);

        //get wtc val
        l_wtc_val = (base->wdg_wtc);
        //need config delt time
        base->wdg_wrc_seq &= 0x0;
        base->wdg_wrc_seq |= l_wtc_val/2;
        base->wdg_wrc_val &= 0x0;
        base->wdg_wrc_val |= (l_wtc_val)/4;
        wdg_config->wdg_refresh_config.wdgModeSelect = wdg_mechanism_mode3;
        wdg_refesh_mechanism_select(base,wdg_config);
        //wdg_test_Int_register(base,&watchdog_irq_test_handle,true);
        dump_all_reg_for_test(base);
        wdg_enable(base);
        while((watchdog_refresh_count>0) && ((int_flag & WDG_INT_ILL_SEQ_REFE_INT_STA_MASK) == 0)){
            //refresh watchdog & /*watchdog timeout status is fasle*
            wdg_record_wdgcnt(base);
            //LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode3_refresh_test delay\n");
            wdg_refresh(base);
            int_flag = wdg_get_status_flag(base);
            watchdog_refresh_count--;
        }

        if((int_flag & WDG_INT_ILL_SEQ_REFE_INT_STA_MASK) == WDG_INT_ILL_SEQ_REFE_INT_STA_MASK)
        {
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode3_refresh_test failed!!\n");
            ret = false;
        }else{
            watchdog_refresh_count = 2;
            while((watchdog_refresh_count>0) && ((int_flag & WDG_INT_ILL_SEQ_REFE_INT_STA_MASK) == 0)){
                //refresh watchdog & /*watchdog timeout status is fasle*
                wdg_record_wdgcnt(base);
                wdg_delay_timeout(base,1);
                //for(uint32_t i = 0;i<=80;i++){
                //    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode3_refresh_test delay\n");
                //}
                wdg_refresh(base);
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode3_refresh_test delay\n");
                int_flag = wdg_get_status_flag(base);
                watchdog_refresh_count--;
            }

            if((int_flag & WDG_INT_ILL_SEQ_REFE_INT_STA_MASK) == WDG_INT_ILL_SEQ_REFE_INT_STA_MASK)
            {
                watchdog_refresh_count = 5;
                wdg_config->wdg_refresh_config.wdgModeSelect = wdg_mechanism_mode2;
                wdg_set_timeout(base,100);
                wdg_refesh_mechanism_select(base,wdg_config);
                while((watchdog_refresh_count>0) && ((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == 0)){
                    //refresh watchdog & /*watchdog timeout status is fasle*
                    wdg_delay_timeout(base,(timeout/2));
                    wdg_refresh(base);
                    int_flag = wdg_get_status_flag(base);
                    dump_all_reg_for_test(base);
                    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_refresh_test int_flag:0x%x,watchdog_refresh_count:%d\n",int_flag,watchdog_refresh_count);
                    watchdog_refresh_count--;
                }

                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode3_refresh_test ok!!\n");
                ret = true;
            }else{
                LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode3_refresh_test failed!!\n");
                ret = false;
            }

            if(ret){
                watchdog_refresh_count = 5;
                while((watchdog_refresh_count>0) && ((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == 0)){
                    //refresh watchdog & /*watchdog timeout status is fasle*
                    //wdg_delay_timeout(base,(timeout/5));
                    wdg_refresh(base);
                    int_flag = wdg_get_status_flag(base);
                    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_1_refresh_test int_flag:0x%x,watchdog_refresh_count:%d\n",int_flag,watchdog_refresh_count);
                    watchdog_refresh_count--;
                }

                if((int_flag & WDG_INT_ILL_WIN_REFE_INT_STA_MASK) == WDG_INT_ILL_WIN_REFE_INT_STA_MASK){
                    watchdog_refresh_count = 5;
                    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_1_refresh_test ok!!\n");
                    wdg_config->wdg_refresh_config.wdgModeSelect = wdg_mechanism_mode1;
                    wdg_refesh_mechanism_select(base,wdg_config);
                    wdg_set_timeout(base,100);
                    wdg_set_ext_reset_cnt(base,1);
                    wdg_refresh(base);
                    while(watchdog_refresh_count>0){
                        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode2_1_refresh_test watchdog_refresh_count:%d\n",watchdog_refresh_count);
                        wdg_delay_timeout(base,timeout/4);
                        //refresh watchdog & /*watchdog timeout status is fasle*
                        wdg_refresh(base);
                        watchdog_refresh_count--;
                    }
                    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_1_refresh_test success!!\n");
                    ret = true;
                }else{
                    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode2_1_refresh_test failed!!\n");
                    ret = false;
                }
            }
        }
    }else{
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode3_refresh_test failed!!\n");
        wdg_clear_ext_reset_cnt(base);
        ret = false;
    }
    wdg_disable(base);
    wdg_clear_ext_reset_cnt(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode3_refresh_test end....................\n");
    return ret;
}
//*****************************************************************************
//case2.5   test14
//! WatchdogMode1OverflowIntCheckTest
//!
//! \param ulBase is the base address of the wdg timer module.
//! \param timeout watchdog timeout time ms
//!
	/*need to genarate an interrupt and check the status register.
	in status register interrupt must be high
	*/
//! \this func is test mode1 refresh
//! \return true is test ok,else return false.
//
//*****************************************************************************
bool wdg_mode1_overflow_int_check_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout)
{
    bool ret = true;
    uint32_t polling_count = 0;
    uint32_t watchdog_refresh_count = 10;
    uint32_t int_flag = 0;
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode1_refresh_test start.....timeout:%d\n",timeout);
    ASSERT_PARAMETER(base);
    if(wdg_config == NULL)
    {
        LTRACEF("base paramenter error base:\n");
        return false;
    }

    //case1.8
    //g_watchdog_int_count = 0;
    // 1.programming the int_rst_en/ext_rst_en set to 1.
    polling_count = wdg_get_ext_reset_cnt(base);
    if(polling_count == 0xff){
        wdg_clear_ext_reset_cnt(base);
        polling_count = wdg_get_ext_reset_cnt(base);
    }

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode1_overflow_int_check_test polling_count:%d\n",polling_count);
    if(polling_count == 0){
        // Watchdog reset control
        wdg_config->wdg_reset_cfg.enableSysReset = false;
        wdg_config->wdg_reset_cfg.enableWdgResetEn = false;
        // Watchdog external reset control
        wdg_config->wdg_ext_reset_cfg.enableSysExtReset = false;
        wdg_config->wdg_int_cfg.overflow_int_clr = false;
        wdg_config->wdg_int_cfg.overflow_int_en = true;

        wdg_init(base,wdg_config);
        //wdg_enable_Interrupts(base);
        wdg_config->wdg_refresh_config.wdgModeSelect = wdg_mechanism_mode1;
        wdg_refesh_mechanism_select(base,wdg_config);
        wdg_set_timeout(base,timeout);
        //wdg_test_Int_register(base,&watchdog_irq_test_handle,true);
        dump_all_reg_for_test(base);
        wdg_enable(base);
        wdg_refresh(base);
        while((watchdog_refresh_count>0) && ((int_flag & WDG_INT_OVERFLOW_INT_STA_MASK) == 0)){
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode1_overflow_int_check_test watchdog_refresh_count:%d\n",watchdog_refresh_count);
            //refresh watchdog & /*watchdog timeout status is fasle*
            wdg_delay_timeout(base,timeout+2);
            wdg_refresh(base);
            int_flag = wdg_get_status_flag(base);
            watchdog_refresh_count--;
        }

        if(int_flag & WDG_INT_OVERFLOW_INT_STA_MASK){
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_mode1_overflow_int_check_test success!!\n");
            ret = true;
        }else{
            LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode1_overflow_int_check_test failed.\n");
            ret = false;
        }
    }else{
        /*test failed*/
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode1_overflow_int_check_test failed.\n");
        ret = false;
    }
    wdg_disable(base);
    wdg_clear_ext_reset_cnt(base);
    dump_all_reg_for_test(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_mode1_overflow_int_check_test end....................\n");
    return ret;
}
//*****************************************************************************
//case2.6   test15
//! WatchdogDebugModeTest
//!
//! \param ulBase is the base address of the wdg timer module.
//! \param timeout watchdog timeout time ms
//!
	/*need to genarate an interrupt and check the status register.
	in status register interrupt must be high
	*/
//! \this func is test mode1 refresh
//! \return true is test ok,else return false.
//
//*****************************************************************************
bool wdg_debug_mode_test(wdg_reg_type_t* base,wdg_config_t *wdg_config,uint32_t timeout)
{
    bool ret = true;
    uint32_t polling_count = 0;
    // Check the arguments.
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_debug_mode_test start.....timeout:%d\n",timeout);
    ASSERT_PARAMETER(base);
    if(wdg_config == NULL)
    {
        LTRACEF("base paramenter error base:\n");
        return false;
    }

    //case2.6
    polling_count = wdg_get_ext_reset_cnt(base);
    if(polling_count == 0xff){
        wdg_clear_ext_reset_cnt(base);
        polling_count = wdg_get_ext_reset_cnt(base);
    }

    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_debug_mode_test polling_count:%d\n",polling_count);
    if(polling_count == 0){
        wdg_init(base,wdg_config);
        //wdg_enable_Interrupts(base);
        wdg_config->wdg_refresh_config.wdgModeSelect = wdg_mechanism_mode1;
        wdg_refesh_mechanism_select(base,wdg_config);
        // 1.programming the int_rst_en/ext_rst_en set to 1.
        wdg_set_timeout(base,20000);
        //wdg_test_Int_register(base,&watchdog_irq_test_handle,true);
        /*#############################*/
        //need set scr debug mode else can not enable wdt debug halt
        {
            vaddr_t scr_sec_cpu1_dbgen_addr;
            vaddr_t scr_sec_cpu2_dbgen_addr;
            vaddr_t scr_saf_dbgen_addr;
            uint32_t reg_read = 0;
            scr_sec_cpu1_dbgen_addr =SCR_SEC_BASE+(0x428<<10);
            reg_read = readl(scr_sec_cpu1_dbgen_addr);
            reg_read |=0x10000;//bit16 enable cpu1 debug mode
            writel(reg_read,scr_sec_cpu1_dbgen_addr);
            scr_sec_cpu2_dbgen_addr =SCR_SEC_BASE+(0x408<<10);
            reg_read = readl(scr_sec_cpu2_dbgen_addr);
            reg_read |=0x10000;//bit16 enable cpu1 debug mode
            writel(reg_read,scr_sec_cpu2_dbgen_addr);
            scr_saf_dbgen_addr =SCR_SAF_BASE+(0x408<<10);
            reg_read = readl(scr_saf_dbgen_addr);
            reg_read |=0x10000;//bit16 enable safety debug mode
            writel(reg_read,scr_saf_dbgen_addr);
        }
        /*#############################*/
        wdg_enable(base);
        wdg_refresh(base);
        wdg_set_ext_reset_cnt(base,1);
        //set debug mode halt enable
        base->wdg_ctrl |= WDG_CTRL_DBG_HALT_EN_MASK;
        dump_all_reg_for_test(base);
        wdg_delay_timeout(base,timeout*2);

        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_debug_mode_test success!!\n");
        wdg_clear_ext_reset_cnt(base);
        ret = true;
    }else if(polling_count == 1){
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_debug_mode_test failed!!\n");
        wdg_clear_ext_reset_cnt(base);
        ret = false;
    }else{
        LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "watchdog wdg_debug_mode_test failed!!\n");
        wdg_clear_ext_reset_cnt(base);
        ret = false;
    }
    dump_all_reg_for_test(base);
    LTRACEF_LEVEL(DEFAULT_WATCHOUT_LOG_LEVEL, "wdg_debug_mode_test end....................\n");
    return ret;
}

//*****************************************************************************
//case2.7
//! WatchdogExtRstTest
//!
//! \param ulBase is the base address of the wdg timer module.
//! \param timeout watchdog timeout time ms
//!
	/*need to genarate an interrupt and check the status register.
	in status register interrupt must be high
	*/
//! \this func is test mode1 refresh
//! \return true is test ok,else return false.
//
//*****************************************************************************
//bool WatchdogExtRstTest(wdg_reg_type_t* base,uint32_t timeout)
//{
//	;
//}
