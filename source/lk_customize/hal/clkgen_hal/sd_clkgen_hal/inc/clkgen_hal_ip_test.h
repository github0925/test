//*****************************************************************************
//
// clkgen_hal_ip_test.h - Prototypes for the Watchdog ip test hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __CLKGEN_IP_TEST_H__
#define __CLKGEN_IP_TEST_H__
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
#include "clkgen_hal.h"

/*clkgen driver interface structure */
typedef struct _clkgen_test_drv_controller_interface {
    bool (*get_default_config)(clkgen_default_cfg_t *def_cfg);
    bool (*dump_reg_for_test)(vaddr_t base,
                              clkgen_debug_module_type module_type, uint32_t slice_idx,
                              uint16_t gating_idx);
    bool (*ipslice_readonlyreg_check_test)(vaddr_t base, uint32_t slice_idx);
    bool (*busslice_readonlyreg_check_test)(vaddr_t base, uint32_t slice_idx);
    bool (*coreslice_readonlyreg_check_test)(vaddr_t base, uint32_t slice_idx);
    bool (*other_readonlyreg_check_test)(vaddr_t base);
    bool (*ipslice_rw_reg_check_test)(vaddr_t base, uint32_t slice_idx);
    bool (*busslice_rw_reg_check_test)(vaddr_t base, uint32_t slice_idx);
    bool (*coreslice_rw_reg_check_test)(vaddr_t base, uint32_t slice_idx);
    bool (*lpgating_rw_reg_check_test)(vaddr_t base, uint32_t gating_idx);
    bool (*uuuslice_rw_reg_check_test)(vaddr_t base,
                                       uint16_t uuu_clock_wrapper_idx);
    bool (*other_rw_reg_check_test)(vaddr_t base);
} clkgen_test_drv_controller_interface_t;

/*clkgen instance */
typedef struct _clkgen_test_instance {
    mutex_t clkgenMutex;   /*!< clkgen layer mutex*/
    uint8_t occupied;   /*!< 0 - the instance is not occupied; 1 - the instance is occupied*/
#ifdef ENABLE_SD_CLKGEN
    clkgen_default_cfg_t def_cfg;
#endif
    bool clkgen_inited;
#ifdef ENABLE_SD_CLKGEN
    const clkgen_test_drv_controller_interface_t
    *controllerTable;  /*!< clkgen driver interface*/
#endif
} clkgen_test_instance_t;

bool hal_clkgen_test_creat_handle(void **handle);
bool hal_clkgen_test_release_handle(void *handle);
bool hal_clkgen_ipslice_readonlyreg_check_test(void *handle,
        uint32_t res_glb_idx);
bool hal_clkgen_busslice_readonlyreg_check_test(void *handle,
        uint32_t res_glb_idx);
bool hal_clkgen_coreslice_readonlyreg_check_test(void *handle,
        uint32_t res_glb_idx);
bool hal_clkgen_other_readonlyreg_check_test(void *handle,
        paddr_t phy_addr);
bool hal_clkgen_ipslice_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx);
bool hal_clkgen_busslice_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx);
bool hal_clkgen_coreslice_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx);
bool hal_clkgen_lpgating_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx);
bool hal_clkgen_uuuslice_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx);
bool hal_clkgen_other_rw_reg_check_test(void *handle, paddr_t phy_addr);
bool hal_clkgen_ip_clock_test(void *handle, uint32_t res_glb_idx,
                              const clkgen_ip_slice_t *ip_slice_default);
bool hal_clkgen_ip_clock_div_test(void *handle, uint32_t res_glb_idx,
                                  const clkgen_ip_slice_t *ip_slice_default);
bool hal_clkgen_bus_clock_test(void *handle, uint32_t res_glb_idx,
                               const clkgen_bus_slice_t *bus_slice_default);
bool hal_clkgen_bus_clock_div_test(void *handle, uint32_t res_glb_idx,
                                   const clkgen_bus_slice_t *bus_slice_default);
bool hal_clkgen_core_clock_test(void *handle, uint32_t res_glb_idx,
                                const clkgen_core_slice_t *core_slice_default);
bool hal_clkgen_core_clock_div_test(void *handle, uint32_t res_glb_idx,
                                    const clkgen_core_slice_t *core_slice_default);
//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif
#endif // __CLKGEN_IP_TEST_H__

