//*****************************************************************************
//
// clkgen_hal.c - Driver for the clkgen hal Module.
//
// Copyright (c) 2019 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <platform/interrupts.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include "clkgen_hal.h"

//*****************************************************************************
//
//! hal_clock_creat_handle.
//!
//! \handle clkgen handle for clkgen func.
//!
//! This function creat clkgen handle.
//!
//! \return clkgen handle
//
//*****************************************************************************
bool hal_clock_creat_handle(void **handle)
{
    return true;
}

//*****************************************************************************
//
//! hal_clock_release_handle.
//!
//! \handle clkgen handle for clkgen func.
//!
//! This function release clkgen instance handle.
//!
//! \return
//
//*****************************************************************************
bool hal_clock_release_handle(void *handle)
{
    return true;
}

bool hal_saf_clock_set_default(void) {return true;}
bool hal_sec_clock_set_default(void) {return true;}
bool hal_soc_clock_set_default(void) {return true;}
bool hal_disp_clock_set_default(void) {return true;}

//*****************************************************************************
//
//! hal_clock_osc_init.
//!
//! \g_handle clock handle.
//!
//! This function is for system doamin fsrefclk init. application use all fsrefclk global index init
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_osc_init(void *g_handle, uint32_t res_glb_idx,
                        clkgen_app_fsrefclk_sel_type src_sel_mask, bool en_safety_mode)
{
    return true;
}

//*****************************************************************************
//
//! hal_clock_ip_init.
//!
//! \g_handle clock handle.
//!
//! This function is for application use all ip default value init
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_ip_init(void *g_handle, paddr_t phy_addr,
                       const clkgen_ip_slice_t *ip_slice_default)

{
    return true;
}

//*****************************************************************************
//
//! hal_clock_bus_init.
//!
//! \g_handle clock handle.
//!
//! This function is for application use all ip default value init
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_bus_init(void *g_handle, paddr_t phy_addr,
                        const clkgen_bus_slice_t *bus_slice_default)
{
    return true;
}

//*****************************************************************************
//
//! hal_clock_core_init.
//!
//! \g_handle clock handle.
//!
//! This function is for application use all ip default value init
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_core_init(void *g_handle, paddr_t phy_addr,
                         const clkgen_core_slice_t *core_slice_default)
{
    return true;
}

//*****************************************************************************
//
//! hal_clock_uuu_init.
//!
//! \g_handle clock handle.
//!
//! This function is for application use all ip default value init
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_uuu_init(void *g_handle, paddr_t phy_addr,
                        const clkgen_uuu_cfg_t *uuu_deafult)
{
    return true;
}

//*****************************************************************************
//
//! hal_clock_enable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \slice_idx_name module name please reference clkgen_hal slice idx name
//! \module_type is ip/bus/core module
//! This function is for set clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_enable(void *g_handle, uint32_t res_glb_idx)
{
    return true;
}

//*****************************************************************************
//
//! hal_clock_disable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \slice_idx_name module name please reference clkgen_hal slice idx name
//! \module_type is ip/bus/core module
//! This function is for set clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_disable(void *g_handle, uint32_t res_glb_idx)
{
    return true;
}
//*****************************************************************************
//
//! hal_clock_ipclk_set.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \ip_slice_idx_name module name please reference clkgen_hal slice idx name
//! \ip_cfg
//! \clk_src_sel please reference clkgen_hal_cfg.h
//! \pre_div clock pre division
//! \post_div  clock post division
//! This function is for set clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_ipclk_set(void *g_handle, uint32_t res_glb_idx,
                         clkgen_app_ip_cfg_t *ip_cfg)
{
    return true;
}
bool hal_clock_ipctl_get(void *g_handle, uint32_t res_glb_idx,
                         clkgen_ip_ctl *ctl) {return true;}
bool hal_clock_ipctl_set(void *g_handle, uint32_t res_glb_idx,
                         const clkgen_ip_ctl *ctl) {return true;}

//*****************************************************************************
//
//! hal_clock_ipclk_get.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \ip_slice_idx_name module name please reference clkgen_hal slice idx name
//! This function is for get module clock.
//!ref_clk_type:reference clock is 24M or 32k
//!ref_clk_div:reference clock division
//!
//! \return clock value = (reg_value*ref_clk_type)/ref_clk_div
//
//*****************************************************************************
uint32_t hal_clock_ipclk_get(void *g_handle, uint32_t res_glb_idx,
                             clkgen_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div)
{
    return true;
}
//*****************************************************************************
//
//! hal_clock_busclk_set.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \bus_slice_idx_name module name please reference clkgen_hal slice idx name
//! \bus_cfg
//! \clk_src_select_a_num;//0:a,1:b
//! \clk_src_select_b_num;//clk select num 0~SLICE_CLK_MAX_NUM
//! \clk_a_b_select;//0:a,1:b
//! \pre_div_a;
//! \pre_div_b;
//! \post_div;
//! \m_div;
//! \n_div;
//! \p_div;
//! \q_div;
//! This function is for set clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_busclk_set(void *g_handle, uint32_t res_glb_idx,
                          clkgen_app_bus_cfg_t *bus_app_cfg)
{
    return true;
}
bool hal_clock_busctl_get(void *g_handle, uint32_t res_glb_idx,
                          clkgen_bus_ctl *ctl, clkgen_bus_gasket *gasket) {return true;}
bool hal_clock_busctl_set(void *g_handle, uint32_t res_glb_idx,
                          const clkgen_bus_ctl *ctl, const clkgen_bus_gasket *gasket) {return true;}

//*****************************************************************************
//
//! hal_clock_ipclk_get.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \bus_slice_idx_name bus name please reference clkgen_hal bus slice idx name
//! This function is for get module clock.
//!ref_clk_type:reference clock is 24M or 32k
//!ref_clk_div:reference clock division
//!
//! \return clock value = (reg_value*ref_clk_type)/ref_clk_div
//
//*****************************************************************************
uint32_t hal_clock_busclk_get(void *g_handle, uint32_t res_glb_idx,
                              clkgen_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div)
{
    return true;
}

//*****************************************************************************
//
//! hal_clock_busclk_set.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \core_slice_idx_name core name please reference clkgen_hal slice idx name
//! \core_app_cfg
//! \clk_src_select_a_num;//0:a,1:b
//! \clk_src_select_b_num;//clk select num 0~SLICE_CLK_MAX_NUM
//! \clk_a_b_select;//0:a,1:b
//! \post_div;

//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_coreclk_set(void *g_handle, uint32_t res_glb_idx,
                           clkgen_app_core_cfg_t *core_app_cfg)
{
    return true;
}
bool hal_clock_corectl_get(void *g_handle, uint32_t res_glb_idx,
                           clkgen_core_ctl *ctl) {return true;}
bool hal_clock_corectl_set(void *g_handle, uint32_t res_glb_idx,
                           const clkgen_core_ctl *ctl) {return true;}

//*****************************************************************************
//
//! hal_clock_coreclk_get.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \core_slice_idx_name core name please reference clkgen_hal bus slice idx name
//! This function is for get core clock.
//!ref_clk_type:reference clock is 24M or 32k
//!ref_clk_div:reference clock division
//!
//! \return clock value = (reg_value*ref_clk_type)/ref_clk_div
//
//*****************************************************************************
uint32_t hal_clock_coreclk_get(void *g_handle, uint32_t res_glb_idx,
                               clkgen_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div)
{
    return true;
}

//*****************************************************************************
//
//! hal_clock_uuuclk_set.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \uuu_wrapper_idx_name uuu name please reference clkgen_hal slice idx name
//! \clkgen_app_uuu_cfg_t
//! \clkgen_uuu_input_type uuu_input_clk_sel;
//! \low_power_mode_en;
//! \m_div;
//! \n_div;
//! \p_div;
//! \q_div;
//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_uuuclk_set(void *g_handle, uint32_t res_glb_idx,
                          clkgen_app_uuu_cfg_t *uuu_app_cfg)
{
    return true;
}
bool hal_clock_uuuctl_get(void *g_handle, uint32_t res_glb_idx,
                          clkgen_uuu_ctl *ctl) {return true;}
bool hal_clock_uuuctl_set(void *g_handle, uint32_t res_glb_idx,
                          const clkgen_uuu_ctl *ctl) {return true;}

//*****************************************************************************
//
//! hal_clock_ipslice_debug_enable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! \dbg_div debug division
//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_ipslice_debug_enable(void *g_handle, uint32_t res_glb_idx,
                                    uint8_t dbg_div)
{
    return true;
}

//*****************************************************************************
//
//! hal_clock_ipslice_debug_disable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_ipslice_debug_disable(void *g_handle, uint32_t res_glb_idx)
{
    return true;
}

//*****************************************************************************
//
//! hal_clock_busslice_debug_enable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! \dbg_div debug division
//! This function is for set bus clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_busslice_debug_enable(void *g_handle, uint32_t res_glb_idx,
                                     uint8_t dbg_div)
{
    return true;
}

//*****************************************************************************
//
//! hal_clock_busslice_debug_disable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! This function is for set bus clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_busslice_debug_disable(void *g_handle, uint32_t res_glb_idx)
{
    return true;
}


//*****************************************************************************
//
//! hal_clock_coreslice_debug_enable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! \dbg_div debug division
//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_coreslice_debug_enable(void *g_handle, uint32_t res_glb_idx,
                                      uint8_t dbg_div)
{
    return true;
}

//*****************************************************************************
//
//! hal_clock_coreslice_debug_disable.
//!
//! \clock_ctrl please reference clock_control_type_idx struct,safety system is clkgen_saf_system and the same to other.
//! \idx_name module name please reference clkgen_hal slice idx name
//! This function is for set core clock.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clock_coreslice_debug_disable(void *g_handle,
                                       uint32_t res_glb_idx)
{
    return true;
}

