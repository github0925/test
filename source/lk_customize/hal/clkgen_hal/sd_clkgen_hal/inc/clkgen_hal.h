//*****************************************************************************
//
// clkgen_hal.h - Prototypes for the clkgen hal
//
// Copyright (c) 2019 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __CLKGEN_HAL_H__
#define __CLKGEN_HAL_H__
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
/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "__regs_base.h"
#include <kernel/mutex.h>
#ifdef ENABLE_SD_CLKGEN
#include "clkgen_drv.h"
#else
typedef struct _clkgen_default_cfg {
    uint8_t src_sel_mask;
    bool safety_mode;
} clkgen_default_cfg_t;
typedef enum _clkgen_debug_module {
    debug_module_ip = 0U,
    debug_module_bus = 1U,
    debug_module_core = 2U,
    debug_module_uuu = 3U,
    debug_module_max,
} clkgen_debug_module_type;
typedef union {
    struct  {
        u32 cg_en_a: 1;
        u32 src_sel_a: 3;
        u32 reserved1: 5;
        u32 a_b_sel: 1;
        u32 post_div_num: 6;
        u32 cg_en_b: 1;
        u32 src_sel_b: 3;
        u32 reserved2: 7;
        u32 cg_en_b_status: 1;
        u32 cg_en_a_status: 1;
        u32 reserved3: 2;
        u32 post_busy: 1;
    };

    u32 val;
} clkgen_core_ctl;

typedef union {

    struct   {
        u32 cg_en_a: 1;
        u32 src_sel_a: 3;
        u32 pre_div_num_a: 3;
        u32 reserved1: 2;
        u32 a_b_sel: 1;
        u32 post_div_num: 6;
        u32 cg_en_b: 1;
        u32 src_sel_b: 3;
        u32 pre_div_num_b: 3;
        u32 reserved2: 4;
        u32 cg_en_b_status: 1;
        u32 cg_en_a_status: 1;
        u32 pre_busy_b: 1;
        u32 pre_busy_a: 1;
        u32 post_busy: 1;
    };
    u32 val;
} clkgen_bus_ctl;

typedef union {

    struct   {
        u32 q_div_num: 3;
        u32 p_div_num: 3;
        u32 n_div_num: 3;
        u32 m_div_num: 3;
        u32 reserved: 16;
        u32 div_q_busy: 1;
        u32 div_p_busy: 1;
        u32 div_n_busy: 1;
        u32 div_m_busy: 1;
    };
    u32 val;
} clkgen_bus_gasket;


typedef union {

    struct  {
        u32 cg_en: 1;
        u32 src_sel: 3;
        u32 pre_div_num: 3;
        u32 reserved1: 3;
        u32 post_div_num: 6;
        u32 reserved2: 12;
        u32 cg_en_status: 1;
        u32 reserved3: 1;
        u32 pre_busy: 1;
        u32 post_busy: 1;
    };
    u32 val;
} clkgen_ip_ctl;

typedef union {
    struct  {
        u32 q_div: 4;
        u32 p_div: 4;
        u32 n_div: 4;
        u32 m_div: 4;
        u32 uuu_sel0: 1;
        u32 uuu_sel1: 1;
        u32 reserved1: 2;
        u32 uuu_gating_ack: 1;
        u32 reserved2: 6;
        u32 dbg_div: 4;
        u32 dbg_gating_en: 1;
    };

    u32 val;
} clkgen_uuu_ctl;

#endif


/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
#define MAKE_VERSION(major, minor, bugfix) (((major) << 16) | ((minor) << 8) | (bugfix))
#define SDV_CLKGEN_DRIVER_VERSION (MAKE_VERSION(1, 0, 0)) /*!< Version 1.0.0 */
#define CLK_MHZ (1000 * 1000)
#define CLK_KHZ 1000

#define DEFAULT_FSREFCLK_NUM    12
#define SLICE_CLK_MAX_NUM 8

#define DEFAULT_FSREFCLK_IDX_START (0x200/4)
#define DEFAULT_IPSLICE_IDX_START (0/4)
#define DEFAULT_LPGATING_IDX_START (0X400/4)
#define DEFAULT_BUSSLICE_IDX_START (0X200/4)
#define DEFAULT_CORESLICE_IDX_START (0X300/4)
#define DEFAULT_UUUSLICE_IDX_START (0X600/4)
/******************************************************************************/
/*--------------------------------Enumerations--------------------------------*/
/******************************************************************************/
/****************clkgen uuu input clk type****************/
/*clkgen uuu input clk type*/
typedef enum _clkgen_uuu_input_sel {
    uuu_input_soc_clk = 0U,
    uuu_input_pll_clk = 1U,
    uuu_input_soc_pll_clk = 2U,
    uuu_input_pll_pll_clk = 3U,
    uuu_input_max_clk,
} clkgen_uuu_input_type;

/****************clkgen slice type idx****************/
/*clkgen monitor reference clock*/
typedef enum _clkgen_mon_ref_clk {
    mon_ref_clk_24M = 0U,
    mon_ref_clk_32K = 1U,
    mon_ref_clk_max,
} clkgen_mon_ref_clk_type;

/*clkgen select reference clock*/
typedef enum _clkgen_app_fsrefclk_sel {
    xtal_saf_24M = 0U,
    xtal_ap_24M = 1U,
    rc_24M = 2U,
} clkgen_app_fsrefclk_sel_type;

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/

/*clkgen app ip slice struct */
typedef struct _clkgen_app_ip_cfg {
    uint8_t clk_src_select_num;//clk select num 0~SLICE_CLK_MAX_NUM
    uint32_t pre_div;
    uint32_t post_div;
} clkgen_app_ip_cfg_t;

/*clkgen app bus slice struct */
typedef struct _clkgen_app_bus_cfg {
    uint8_t clk_src_select_a_num;//0:a,1:b
    uint8_t clk_src_select_b_num;//clk select num 0~SLICE_CLK_MAX_NUM
    uint8_t clk_a_b_select;//0:a,1:b
    uint8_t pre_div_a;
    uint8_t pre_div_b;
    uint8_t post_div;
    uint8_t m_div;
    uint8_t n_div;
    uint8_t p_div;
    uint8_t q_div;
} clkgen_app_bus_cfg_t;

/*clkgen app core slice struct */
typedef struct _clkgen_app_core_cfg {
    uint8_t clk_src_select_a_num;//0:a,1:b
    uint8_t clk_src_select_b_num;//clk select num 0~SLICE_CLK_MAX_NUM
    uint8_t clk_a_b_select;//0:a,1:b
    uint32_t post_div;
} clkgen_app_core_cfg_t;

/*clkgen app uuu wrapper struct */
typedef struct _clkgen_app_uuu_cfg {
    clkgen_uuu_input_type uuu_input_clk_sel;
    bool low_power_mode_en;
    uint8_t m_div;
    uint8_t n_div;
    uint8_t p_div;
    uint8_t q_div;
} clkgen_app_uuu_cfg_t;

/****************clkgen app aprameter config end****************/

/*clkgen ip slice struct */
typedef struct _clkgen_ip_slice {
    int slice_index;
    const uint32_t clk_src_select[SLICE_CLK_MAX_NUM];
    uint8_t clk_src_select_num;//clk select num 0~SLICE_CLK_MAX_NUM
    uint32_t pre_div;
    uint32_t post_div;
    uint32_t sel_clk;
} clkgen_ip_slice_t;

/*clkgen bus slice struct */
typedef struct _clkgen_bus_slice {
    int slice_index;
    const uint32_t clk_src_select_a[SLICE_CLK_MAX_NUM];
    uint8_t clk_src_select_a_num;//0:a,1:b
    const uint32_t clk_src_select_b[SLICE_CLK_MAX_NUM];
    uint32_t clk_src_select_b_num;//clk select num 0~SLICE_CLK_MAX_NUM
    uint8_t clk_a_b_select;//0:a,1:b
    uint8_t pre_div_a;
    uint8_t pre_div_b;
    uint8_t post_div;
    uint8_t m_div;
    uint8_t n_div;
    uint8_t p_div;
    uint8_t q_div;
    uint32_t a_sel_clk;
    uint32_t b_sel_clk;
} clkgen_bus_slice_t;

/*clkgen core slice struct */
typedef struct _clkgen_core_slice {
    int slice_index;
    const uint32_t clk_src_select_a[SLICE_CLK_MAX_NUM];
    uint8_t clk_src_select_a_num;//0:a,1:b
    const uint32_t clk_src_select_b[SLICE_CLK_MAX_NUM];
    uint8_t clk_src_select_b_num;//clk select num 0~SLICE_CLK_MAX_NUM
    uint8_t clk_a_b_select;//0:a,1:b
    uint32_t post_div;
    uint32_t a_sel_clk;
    uint32_t b_sel_clk;
} clkgen_core_slice_t;

/*clkgen uuu config struct */
typedef struct _clkgen_uuu_cfg {
    int slice_index;
    uint8_t uuu_input_clk_sel;
    uint8_t m_div;
    uint8_t n_div;
    uint8_t p_div;
    uint8_t q_div;
    uint32_t sel_clk;
} clkgen_uuu_cfg_t;

/*clkgen uuu monitor struct */
typedef struct _clkgen_uuu_monitor {
    uint32_t res_id;
    uint32_t ip_idx;
} clkgen_uuu_monitor_t;

#ifdef ENABLE_SD_CLKGEN
/*clkgen driver interface structure */
typedef struct _clkgen_drv_controller_interface {
    bool (*get_default_config)(clkgen_default_cfg_t *def_cfg);
    bool (*fsrefclk_sel)(vaddr_t base, uint32_t scr_idx, uint32_t src_sel_mask,
                         bool safety_mode);
    bool (*gating_enable)(vaddr_t base, uint16_t gating_idx, bool enable);
    bool (*ip_slice_set)(vaddr_t base, uint8_t ip_slice_idx,
                         uint8_t clk_src_sel, uint16_t pre_div, uint16_t post_div);
    bool (*ip_ctl_get)(vaddr_t base, uint32_t slice_idx, clkgen_ip_ctl *ctl);
    bool (*ip_ctl_set)(vaddr_t base, uint32_t slice_idx,
                       const clkgen_ip_ctl *ctl);
    bool (*bus_slice_switch)(vaddr_t base,
                             clkgen_bus_slice_drv_t *bus_clk_cfg);
    bool (*bus_ctl_get)(vaddr_t base, uint32_t slice_idx, clkgen_bus_ctl *ctl,
                        clkgen_bus_gasket *gasket);
    bool (*bus_ctl_set)(vaddr_t base, uint32_t slice_idx,
                        const clkgen_bus_ctl *ctl,
                        const clkgen_bus_gasket *gasket);
    bool (*core_slice_switch)(vaddr_t base,
                              clkgen_core_slice_drv_t *core_clk_cfg);
    bool (*core_ctl_get)(vaddr_t base, uint32_t slice_idx,
                         clkgen_core_ctl *ctl);
    bool (*core_ctl_set)(vaddr_t base, uint32_t slice_idx,
                         const clkgen_core_ctl *ctl);
    uint32_t (*mon_ip_slice)(vaddr_t base, uint16_t ip_slice_idx,
                             clkgen_slice_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div,
                             clkgen_slice_mon_ret_type ret_type);
    uint32_t (*mon_bus_slice)(vaddr_t base, uint16_t bus_slice_idx,
                              clkgen_slice_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div,
                              clkgen_slice_mon_ret_type ret_type);
    uint32_t (*mon_core_slice)(vaddr_t base, uint16_t core_slice_idx,
                               clkgen_slice_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div,
                               clkgen_slice_mon_ret_type ret_type);
    bool (*uuu_clock_wrapper)(vaddr_t base, uint16_t uuu_clock_wrapper_idx,
                              clkgen_gasket_type_t *gasket_div, bool low_power_mode,
                              uint8_t input_clk_type);
    bool (*uuu_ctl_get)(vaddr_t base, uint32_t slice_idx, clkgen_uuu_ctl *ctl);
    bool (*uuu_ctl_set)(vaddr_t base, uint32_t slice_idx,
                        const clkgen_uuu_ctl *ctl);
    bool (*ipslice_debug_enable)(vaddr_t base, uint16_t slice_idx,
                                 uint8_t dbg_div);
    bool (*ipslice_debug_disable)(vaddr_t base);
    bool (*busslice_debug_enable)(vaddr_t base, uint16_t slice_idx,
                                  uint8_t dbg_div);
    bool (*busslice_debug_disable)(vaddr_t base);
    bool (*coreslice_debug_enable)(vaddr_t base, uint16_t slice_idx,
                                   uint8_t dbg_div);
    bool (*coreslice_debug_disable)(vaddr_t base);
    bool (*uuuslice_debug_enable)(vaddr_t base, uint16_t slice_idx,
                                  uint8_t dbg_div);
    bool (*uuuslice_debug_disable)(vaddr_t base, uint16_t slice_idx);
} clkgen_drv_controller_interface_t;
#endif
/*clkgen instance */
typedef struct _clkgen_instance {
    mutex_t clkgenMutex;   /*!< clkgen layer mutex*/
    uint8_t occupied;   /*!< 0 - the instance is not occupied; 1 - the instance is occupied*/
#ifdef ENABLE_SD_CLKGEN
    clkgen_default_cfg_t def_cfg;
#endif
    bool clkgen_inited;
#ifdef ENABLE_SD_CLKGEN
    const clkgen_drv_controller_interface_t
    *controllerTable;  /*!< clkgen driver interface*/
#endif
} clkgen_instance_t;
/******************************************************************************/
/*-------------------------Function Prototypes-------------------------*/
/******************************************************************************/
bool hal_clock_creat_handle(void **handle);
bool hal_clock_release_handle(void *handle);
bool hal_saf_clock_set_default(void);
bool hal_sec_clock_set_default(void);
bool hal_soc_clock_set_default(void);
bool hal_disp_clock_set_default(void);
bool hal_clock_osc_init(void *g_handle, uint32_t res_glb_idx,
                        clkgen_app_fsrefclk_sel_type src_sel_mask, bool en_safety_mode);
bool hal_clock_ip_init(void *g_handle, paddr_t phy_addr,
                       const clkgen_ip_slice_t *ip_slice_default);
bool hal_clock_bus_init(void *g_handle, paddr_t phy_addr,
                        const clkgen_bus_slice_t *bus_slice_default);
bool hal_clock_core_init(void *g_handle, paddr_t phy_addr,
                         const clkgen_core_slice_t *core_slice_default);
bool hal_clock_uuu_init(void *g_handle, paddr_t phy_addr,
                        const clkgen_uuu_cfg_t *uuu_default);
bool hal_clock_enable(void *g_handle, uint32_t res_glb_idx);
bool hal_clock_disable(void *g_handle, uint32_t res_glb_idx);
bool hal_clock_uuuclk_set(void *g_handle, uint32_t res_glb_idx,
                          clkgen_app_uuu_cfg_t *uuu_app_cfg);
bool hal_clock_ipclk_set(void *g_handle, uint32_t res_glb_idx,
                         clkgen_app_ip_cfg_t *ip_cfg);
bool hal_clock_ipctl_get(void *g_handle, uint32_t res_glb_idx,
                         clkgen_ip_ctl *ctl);
bool hal_clock_ipctl_set(void *g_handle, uint32_t res_glb_idx,
                         const clkgen_ip_ctl *ctl);
bool hal_clock_busclk_set(void *g_handle, uint32_t res_glb_idx,
                          clkgen_app_bus_cfg_t *bus_app_cfg);
bool hal_clock_busctl_get(void *g_handle, uint32_t res_glb_idx,
                          clkgen_bus_ctl *ctl, clkgen_bus_gasket *gasket);
bool hal_clock_busctl_set(void *g_handle, uint32_t res_glb_idx,
                          const clkgen_bus_ctl *ctl, const clkgen_bus_gasket *gasket);
bool hal_clock_corectl_get(void *g_handle, uint32_t res_glb_idx,
                           clkgen_core_ctl *ctl);
bool hal_clock_corectl_set(void *g_handle, uint32_t res_glb_idx,
                           const clkgen_core_ctl *ctl);
bool hal_clock_coreclk_set(void *g_handle, uint32_t res_glb_idx,
                           clkgen_app_core_cfg_t *core_app_cfg);
uint32_t hal_clock_ipclk_get(void *g_handle, uint32_t res_glb_idx,
                             clkgen_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div);
uint32_t hal_clock_busclk_get(void *g_handle, uint32_t res_glb_idx,
                              clkgen_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div);
uint32_t hal_clock_coreclk_get(void *g_handle, uint32_t res_glb_idx,
                               clkgen_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div);
uint32_t hal_clock_uuuclk_get(void *g_handle, uint32_t res_glb_idx,
                              clkgen_mon_ref_clk_type ref_clk_type, uint8_t ref_clk_div);
bool hal_clock_uuuctl_get(void *g_handle, uint32_t res_glb_idx,
                          clkgen_uuu_ctl *ctl);
bool hal_clock_uuuctl_set(void *g_handle, uint32_t res_glb_idx,
                          const clkgen_uuu_ctl *ctl);
bool hal_clock_ipslice_debug_enable(void *g_handle, uint32_t res_glb_idx,
                                    uint8_t dbg_div);
bool hal_clock_ipslice_debug_disable(void *g_handle, uint32_t res_glb_idx);
bool hal_clock_busslice_debug_enable(void *g_handle, uint32_t res_glb_idx,
                                     uint8_t dbg_div);
bool hal_clock_busslice_debug_disable(void *g_handle,
                                      uint32_t res_glb_idx);
bool hal_clock_coreslice_debug_enable(void *g_handle, uint32_t res_glb_idx,
                                      uint8_t dbg_div);
bool hal_clock_coreslice_debug_disable(void *g_handle,
                                       uint32_t res_glb_idx);
bool hal_clock_uuuslice_debug_enable(void *g_handle, uint32_t res_glb_idx,
                                     uint8_t dbg_div);
bool hal_clock_uuuslice_debug_disable(void *g_handle,
                                      uint32_t res_glb_idx);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif //

