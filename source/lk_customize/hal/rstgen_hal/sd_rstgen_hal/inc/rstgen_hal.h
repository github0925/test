//*****************************************************************************
//
// rstgen_hal.h - Prototypes for the rstgen hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __RSTGEN_HAL_H__
#define __RSTGEN_HAL_H__
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
#include "__regs_base.h"
#if ENABLE_SD_RSTGEN
#include "rstgen_drv.h"
#endif
#include <kernel/mutex.h>

#define MAKE_VERSION(major, minor, bugfix) (((major) << 16) | ((minor) << 8) | (bugfix))
#define SDV_RSTGEN_DRIVER_VERSION (MAKE_VERSION(1, 0, 0)) /*!< Version 1.0.0 */
#define DEFAULT_RSTGEN_MAX_NUM 2

/*rstgen global reset enable mask  saf_ss:bit3 for wdt1 sec_ss:bit3~bit8 = wdt2~wdt7*/
typedef enum _rstgen_glb_rst_en
{
    rstgen_glb_rst_self_rst_en = ((uint32_t) (1 << 0)),
    rstgen_glb_rst_sem_rst_en = ((uint32_t) (1 << 1)),
    rstgen_glb_rst_dbg_rst_en = ((uint32_t) (1 << 2)),
    rstgen_glb_rst_saf_wdg1_rst_en = ((uint32_t) (1 << 3)),
    rstgen_glb_rst_sec_wdg2_rst_en = ((uint32_t) (1 << 3)),
    rstgen_glb_rst_sec_wdg3_rst_en = ((uint32_t) (1 << 4)),
    rstgen_glb_rst_sec_wdg4_rst_en = ((uint32_t) (1 << 5)),
    rstgen_glb_rst_sec_wdg5_rst_en = ((uint32_t) (1 << 6)),
    rstgen_glb_rst_sec_wdg6_rst_en = ((uint32_t) (1 << 7)),
    rstgen_glb_rst_sec_wdg7_rst_en = ((uint32_t) (1 << 8)),
} rstgen_glb_rst_en;

typedef enum _rstgen_core_rst_en
{
    rstgen_core_sw_rst_en = ((uint32_t) (1 << 0)),
    rstgen_core_dbg_rst_en = ((uint32_t) (1 << 1)),
    rstgen_core_wdg_rst_en = ((uint32_t) (1 << 2)),
} rstgen_core_rst_en;

/*rstgen global reset status mask  saf_ss:bit3 for wdt1 sec_ss:bit3~bit8 = wdt2~wdt7*/
typedef enum _rstgen_glb_rst_sta
{
    rstgen_glb_rst_pre_sw_sta = ((uint32_t) (1 << 0)),
    rstgen_glb_rst_self_sw_sta = ((uint32_t) (1 << 1)),
    rstgen_glb_rst_sem_sw_sta = ((uint32_t) (1 << 2)),
    rstgen_glb_rst_dbg_sw_sta = ((uint32_t) (1 << 3)),
    rstgen_glb_rst_saf_wdg1_sta = ((uint32_t) (1 << 4)),
    rstgen_glb_rst_sec_wdg2_sta = ((uint32_t) (1 << 4)),
    rstgen_glb_rst_sec_wdg3_sta = ((uint32_t) (1 << 5)),
    rstgen_glb_rst_sec_wdg4_sta = ((uint32_t) (1 << 6)),
    rstgen_glb_rst_sec_wdg5_sta = ((uint32_t) (1 << 7)),
    rstgen_glb_rst_sec_wdg6_sta = ((uint32_t) (1 << 8)),
    rstgen_glb_rst_sec_wdg7_sta = ((uint32_t) (1 << 9)),

    rstgen_glb_rst_sinc_pre_sw_sta = ((uint32_t) (1 << 16)),
    rstgen_glb_rst_sinc_self_sw_sta = ((uint32_t) (1 << 17)),
    rstgen_glb_rst_sinc_sem_sw_sta = ((uint32_t) (1 << 18)),
    rstgen_glb_rst_sinc_dbg_sw_sta = ((uint32_t) (1 << 19)),
    rstgen_glb_rst_sinc_saf_wdg1_sta = ((uint32_t) (1 << 20)),
    rstgen_glb_rst_sinc_sec_wdg2_sta = ((uint32_t) (1 << 20)),
    rstgen_glb_rst_sinc_sec_wdg3_sta = ((uint32_t) (1 << 21)),
    rstgen_glb_rst_sinc_sec_wdg4_sta = ((uint32_t) (1 << 22)),
    rstgen_glb_rst_sinc_sec_wdg5_sta = ((uint32_t) (1 << 23)),
    rstgen_glb_rst_sinc_sec_wdg6_sta = ((uint32_t) (1 << 24)),
    rstgen_glb_rst_sinc_sec_wdg7_sta = ((uint32_t) (1 << 25)),
} rstgen_glb_rst_sta;

/*rstgen isolation index,is only control by sec system*/
typedef enum _rstgen_iso_idx
{
    pcie_iso_b = 0U,
    usb_iso_b = 1U,
    cpu1_iso_b = 2U,
    gpu1_iso_b = 3U,
    ddr_iso_b = 4U,
    iso_idx_max,
} rstgen_iso_idx;

/*rstgen safety core index*/
typedef enum _rstgen_saf_core_idx
{
    saf_core_cr5_saf = 0x0U,
    saf_core_max
} rstgen_saf_core_idx;

/*rstgen ap core index*/
typedef enum _rstgen_ap_core_idx
{
    ap_core_vdsp = 0U,
    ap_core_cr5_sec = 1U,
    ap_core_cr5_mp = 2U,
    ap_core_cpu1_core_all = 3U,
    ap_core_cpu2_core = 4U,
    ap_core_adsp = 5U,
    ap_core_max,
} rstgen_ap_core_idx;

/*rstgen safety module index*/
typedef enum _rstgen_saf_module_idx
{
    saf_module_gic1 = 0U,
    saf_module_ospi1 = 1U,
    saf_module_enet1 = 2U,
    saf_module_i2s_sc1 = 3U,
    saf_module_i2s_sc2 = 4U,
    saf_module_canfd1 = 5U,
    saf_module_canfd2 = 6U,
    saf_module_canfd3 = 7U,
    saf_module_canfd4 = 8U,
    saf_module_sem1 = 9U,
    saf_module_sem2 = 10U,
    saf_module_max,
} rstgen_saf_module_idx;

/*rstgen ap modue index*/
typedef enum _rstgen_ap_module_idx
{
    ap_module_ospi2 = 0U,
    ap_module_i2s_sc3 = 1U,
    ap_module_i2s_sc4 = 2U,
    ap_module_i2s_sc5 = 3U,
    ap_module_i2s_sc6 = 4U,
    ap_module_i2s_sc7 = 5U,
    ap_module_i2s_sc8 = 6U,
    ap_module_i2s_mc1 = 7U,
    ap_module_i2s_mc2 = 8U,
    ap_module_canfd5 = 9U,
    ap_module_canfd6 = 10U,
    ap_module_canfd7 = 11U,
    ap_module_canfd8 = 12U,
    ap_module_enet2 = 13U,
    ap_module_mshc1 = 14U,
    ap_module_mshc2 = 15U,
    ap_module_mshc3 = 16U,
    ap_module_mshc4 = 17U,
    ap_module_adsp = 18U,
    ap_module_gic2 = 19U,
    ap_module_gic3 = 20U,
    ap_module_cpu1_core0_warm = 21U,
    ap_module_cpu1_core1_warm = 22U,
    ap_module_cpu1_core2_warm = 23U,
    ap_module_cpu1_core3_warm = 24U,
    ap_module_cpu1_core4_warm = 25U,
    ap_module_cpu1_core5_warm = 26U,
    ap_module_cpu1_scu_warm = 27U,
    ap_module_ddr_ss = 29U,
    ap_module_ddr_sw0 = 30U,
    ap_module_ddr_sw1 = 31U,
    ap_module_ddr_sw2 = 32U,
    ap_module_gic4 = 33U,
    ap_module_gic5 = 34U,
    ap_module_cssys_treset_n = 36U,
    ap_module_nna = 37U,
    ap_module_vdsp_DReset = 38U,
    ap_module_vpu1 = 39U,
    ap_module_vpu2 = 40U,
    ap_module_mjpeg = 41U,
    ap_module_cpu1_ss = 45U,
    ap_module_cpu2_ss = 46U,
    ap_module_mipi_csi1 = 56U,
    ap_module_mipi_csi2 = 57U,
    ap_module_mipi_csi3 = 58U,
    ap_module_mipi_dsi1 = 59U,
    ap_module_mipi_dsi2 = 60U,
    ap_module_dc1 = 61U,
    ap_module_dc2 = 62U,
    ap_module_dc3 = 63U,
    ap_module_dc4 = 64U,
    ap_module_dc5 = 65U,
    ap_module_dp1 = 66U,
    ap_module_dp2 = 67U,
    ap_module_dp3 = 68U,
    ap_module_lvds_ss = 69U,
    ap_module_csi1 = 70U,
    ap_module_csi2 = 71U,
    ap_module_csi3 = 72U,
    ap_module_disp_mux = 73U,
    ap_module_g2d1 = 74U,
    ap_module_g2d2 = 75U,
    ap_module_gpu1_core = 80U,
    ap_module_gpu1_ss = 81U,
    ap_module_gpu2_core = 82U,
    ap_module_gpu2_ss = 83U,
    ap_module_dbg_req = 89U,
    ap_module_canfd9 = 90U,
    ap_module_canfd10 = 91U,
    ap_module_canfd11 = 92U,
    ap_module_canfd12 = 93U,
    ap_module_canfd13 = 94U,
    ap_module_canfd14 = 95U,
    ap_module_canfd15 = 96U,
    ap_module_canfd16 = 97U,
    ap_module_canfd17 = 98U,
    ap_module_canfd18 = 99U,
    ap_module_canfd19 = 100U,
    ap_module_canfd20 = 101U,
    ap_module_max,
} rstgen_ap_module_idx;

/*rstgen driver interface structure */
typedef struct _RstgenResManageIdToIdx
{
    uint32_t res_glb_idx;
    uint32_t res_idx;
} RstgenResManageIdToIdx;

/*rstgen driver interface structure */
typedef struct _rstgen_drv_controller_interface
{
    void (*get_default_config)(uint32_t *global_rst_maks);
    bool (*init)(vaddr_t base,const uint32_t global_rst_maks);
    bool (*global_rst_enable)(vaddr_t base,uint32_t mask);
    bool (*global_rst_disable)(vaddr_t base,uint32_t mask);
    bool (*sw_self_rst)(vaddr_t base,bool rst_release);
    bool (*sw_oth_rst)(vaddr_t base,bool rst_release);
    uint32_t (*get_rst_sta)(vaddr_t base);
    bool (*iso_enable)(vaddr_t base,uint32_t iso_idx);
    bool (*iso_disable)(vaddr_t base,uint32_t iso_idx);
    uint32_t (*iso_status)(vaddr_t base,uint32_t iso_idx);
    bool (*core_reset)(vaddr_t base,uint32_t core_idx);
    bool (*core_ctl)(vaddr_t base,uint32_t core_idx,bool release);
    bool (*module_ctl)(vaddr_t base,uint32_t module_idx,bool rst_release);
    uint32_t (*module_reset_status)(vaddr_t base,uint32_t module_idx);
    uint32_t (*core_reset_status)(vaddr_t base,uint32_t core_idx);
} rstgen_drv_controller_interface_t;

typedef struct
{
    uint32_t res_glb_idx;
    bool glb_self_rst_en;
    bool glb_sem_rst_en;
    bool glb_dbg_rst_en;
    bool glb_wdg1_rst_en;
    bool glb_wdg2_rst_en;
    bool glb_wdg3_rst_en;
    bool glb_wdg4_rst_en;
    bool glb_wdg5_rst_en;
    bool glb_wdg6_rst_en;
    bool glb_wdg7_rst_en;
    bool glb_wdg8_rst_en;
} rstgen_capability_cfg_t;

typedef struct
{
    uint32_t version;
    char res_category[20];
    uint8_t res_max_num;
    rstgen_capability_cfg_t res_cap[DEFAULT_RSTGEN_MAX_NUM];
} rstgen_capability_t;

typedef struct
{
    bool glb_self_rst_en;
    bool glb_sem_rst_en;
    bool glb_dbg_rst_en;
    bool glb_wdg1_rst_en;
    bool glb_wdg2_rst_en;
    bool glb_wdg3_rst_en;
    bool glb_wdg4_rst_en;
    bool glb_wdg5_rst_en;
    bool glb_wdg6_rst_en;
    bool glb_wdg7_rst_en;
    bool glb_wdg8_rst_en;
} rstgen_res_config_t;

/*rstgen instance */
typedef struct _rstgen_instance
{
    uint32_t global_rst_maks;   /*!< rstgen global reset mask*/
    mutex_t rstgenMutex;   /*!< rstgen layer mutex*/
    uint8_t occupied;   /*!< 0 - the instance is not occupied; 1 - the instance is occupied*/
    bool rstgen_inited;
    paddr_t phy_addr;
    rstgen_res_config_t rstgen_res[2];
    const rstgen_drv_controller_interface_t *controllerTable;  /*!< rstgen driver interface*/
} rstgen_instance_t;

extern const RstgenResManageIdToIdx g_IsoResIdToIsoRstgenIdx[iso_idx_max];
extern const RstgenResManageIdToIdx g_CoreResIdToCoreRstgenIdx[saf_core_max+ap_core_max];
extern const RstgenResManageIdToIdx g_ModuleResIdToModuleRstgenIdx[saf_module_max+ap_module_max];

bool hal_rstgen_creat_handle(void **handle,uint32_t global_rst_res_idx);
bool hal_rstgen_release_handle(void *handle);
bool hal_rstgen_init(void *handle);
bool hal_rstgen_sw_self_rst(void *handle);
bool hal_rstgen_sw_oth_rst(void *handle);
uint32_t hal_rstgen_get_rst_sta(void *handle);
bool hal_rstgen_iso_enable(void *handle,uint32_t res_glb_idx);
bool hal_rstgen_iso_disable(void *handle,uint32_t res_glb_idx);
uint32_t hal_rstgen_iso_status(void *handle,uint32_t res_glb_idx);
bool hal_rstgen_core_reset(void *handle,uint32_t res_glb_idx);
bool hal_rstgen_core_ctl(void *handle,uint32_t res_glb_idx, bool release);
bool hal_rstgen_module_reset(void *handle,uint32_t res_glb_idx);
bool hal_rstgen_module_ctl(void *handle,uint32_t res_glb_idx, bool release);
uint32_t hal_rstgen_module_status(void *handle, uint32_t res_glb_idx);
uint32_t hal_rstgen_core_status(void *handle, uint32_t res_glb_idx);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __WDG_IP_TEST_H__

