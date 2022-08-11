//*****************************************************************************
//
// clkgen_hal_ip_test.c - Driver for the clkgen hal Module.
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
#include <platform.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>
#include "clkgen_drv_test.h"
#include "clkgen_hal_ip_test.h"
#include "clkgen_hal.h"
#include "res.h"

#define LOCAL_TRACE 0
/*clkgen global instance*/
static clkgen_test_instance_t g_ClkgenTestInstance = {
    .occupied = 0,
};

spin_lock_t clkgen_test_spin_lock = SPIN_LOCK_INITIAL_VALUE;

/*clkgen driver interface*/
static const clkgen_test_drv_controller_interface_t
s_ClkgenTestDrvInterface = {
    clkgen_get_default_config,
    clkgen_dump_reg_for_test,
    clkgen_ipslice_readonlyreg_check_test,
    clkgen_busslice_readonlyreg_check_test,
    clkgen_coreslice_readonlyreg_check_test,
    clkgen_other_readonlyreg_check_test,
    clkgen_ipslice_rw_reg_check_test,
    clkgen_busslice_rw_reg_check_test,
    clkgen_coreslice_rw_reg_check_test,
    clkgen_lpgating_rw_reg_check_test,
    clkgen_uuuslice_rw_reg_check_test,
    clkgen_other_rw_reg_check_test,
};
//*****************************************************************************
//
//! hal_clkgen_test_get_controller_interface.
//!
//! \param controllerTable is clkgen interface ptr
//!
//! This function get clkgen driver interface.
//!
//! \return
//
//*****************************************************************************
static void hal_clkgen_test_get_controller_interface(
    const clkgen_test_drv_controller_interface_t **controllerTable)
{
    *controllerTable = &s_ClkgenTestDrvInterface;
}
//*****************************************************************************
//
//! hal_clkgen_test_get_instance.
//!
//! \void.
//!
//! This function get clkgen instance hand.
//!
//! \return clkgen hanle
//
//*****************************************************************************
static clkgen_test_instance_t *hal_clkgen_test_get_instance(void)
{
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&clkgen_test_spin_lock, states);

    if (g_ClkgenTestInstance.occupied != 1) {
        memset(&g_ClkgenTestInstance, 0, sizeof(clkgen_test_instance_t));
        /* get clkgen driver API table */
        hal_clkgen_test_get_controller_interface(&
                (g_ClkgenTestInstance.controllerTable));

        if (g_ClkgenTestInstance.controllerTable) {
            g_ClkgenTestInstance.occupied = 1;
            //sec clkgen
            g_ClkgenTestInstance.controllerTable->get_default_config(&
                    (g_ClkgenTestInstance.def_cfg));
        }

        spin_unlock_irqrestore(&clkgen_test_spin_lock, states);
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                      "hal_clkgen_test_get_instance is ok \n");
        return &g_ClkgenTestInstance;
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clkgen_test_get_instance is failed \n");
    spin_unlock_irqrestore(&clkgen_test_spin_lock, states);
    return NULL;
}
//*****************************************************************************
//
//! hal_clkgen_test_release_instance.
//!
//! \void.
//!
//! This function release clkgen instance hand.
//!
//! \return
//
//*****************************************************************************
static void hal_clkgen_test_release_instance(clkgen_test_instance_t
        *clkgenInstance)
{
    spin_lock_saved_state_t states;
    spin_lock_irqsave(&clkgen_test_spin_lock, states);
    clkgenInstance->occupied = 0;
    spin_unlock_irqrestore(&clkgen_test_spin_lock, states);
}
//*****************************************************************************
//
//! hal_clkgen_test_creat_handle.
//!
//! \handle clkgen handle for clkgen func.
//!
//! This function get hal handle.
//!
//! \return clkgen handle
//
//*****************************************************************************
bool hal_clkgen_test_creat_handle(void **handle)
{
    clkgen_test_instance_t  *clkgenInstance = NULL;

    if (handle == NULL) {
        LTRACEF("hal_clkgen_test_creat_handle paramenter error handle:%p\n",
                handle);
        return false;
    }

    if (clkgen_test_spin_lock != SPIN_LOCK_INITIAL_VALUE) {
        spin_lock_init(&clkgen_test_spin_lock);
    }

    clkgenInstance = hal_clkgen_test_get_instance();

    if (clkgenInstance == NULL) {
        return false;
    }

    mutex_init(&clkgenInstance->clkgenMutex);

    *handle = clkgenInstance;
    return true;
}

//*****************************************************************************
//
//! hal_clkgen_test_release_handle.
//!
//! \handle clkgen handle for clkgen func.
//!
//! This function delete clkgen instance handle.
//!
//! \return
//
//*****************************************************************************
bool hal_clkgen_test_release_handle(void *handle)
{
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    if (handle == NULL) {
        LTRACEF("hal_clkgen_test_release_handle paramenter error handle:%p\n",
                handle);
        return false;
    }

    l_clkgenInstance = (clkgen_test_instance_t *)handle;
    l_clkgenInstance->occupied = 0;
    mutex_destroy(&l_clkgenInstance->clkgenMutex);

    return true;
}
//*****************************************************************************
//case1.1 test1
//! hal_clkgen_ipslice_readonlyreg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx ip global resoure index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_ipslice_readonlyreg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_test_instance_t *l_clkgenInstance = NULL;
    int32_t ip_slice_idx = -1;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &ip_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_ipslice_readonlyreg_check_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((ip_slice_idx != -1) && (ip_slice_idx >= DEFAULT_IPSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->ipslice_readonlyreg_check_test) {
        ip_slice_idx -= DEFAULT_IPSLICE_IDX_START;

        if (l_clkgenInstance->controllerTable->ipslice_readonlyreg_check_test(
                    clkgen_base_addr, ip_slice_idx)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clkgen_ipslice_readonlyreg_check_test success res_glb_idx:0x%x,ip_slice_idx:%d\n",
                          res_glb_idx, ip_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clkgen_ipslice_readonlyreg_check_test failed res_glb_idx:0x%x,ip_slice_idx:%d\n",
            res_glb_idx, ip_slice_idx);
    return false;
}

//*****************************************************************************
//case1.1   test2
//! hal_clkgen_busslice_readonlyreg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx ip global resource index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_busslice_readonlyreg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    int32_t bus_slice_idx = -1;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &bus_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_busslice_readonlyreg_check_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((bus_slice_idx != -1) && (bus_slice_idx >= DEFAULT_BUSSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->busslice_readonlyreg_check_test) {
        bus_slice_idx -= DEFAULT_BUSSLICE_IDX_START;

        if (l_clkgenInstance->controllerTable->busslice_readonlyreg_check_test(
                    clkgen_base_addr, bus_slice_idx)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clkgen_busslice_readonlyreg_check_test success res_glb_idx:0x%x,ip_slice_idx:%d\n",
                          res_glb_idx, bus_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clkgen_busslice_readonlyreg_check_test failed res_glb_idx:0x%x,ip_slice_idx:%d\n",
            res_glb_idx, bus_slice_idx);
    return false;
}

//*****************************************************************************
//case1.1 test3
//! hal_clkgen_coreslice_readonlyreg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx core slice global resource index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_coreslice_readonlyreg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    int32_t core_slice_idx = -1;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &core_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_coreslice_readonlyreg_check_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((core_slice_idx != -1)
            && (core_slice_idx >= DEFAULT_CORESLICE_IDX_START)
            && l_clkgenInstance->controllerTable->coreslice_readonlyreg_check_test) {
        core_slice_idx -= DEFAULT_CORESLICE_IDX_START;

        if (l_clkgenInstance->controllerTable->coreslice_readonlyreg_check_test(
                    clkgen_base_addr, core_slice_idx)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clkgen_coreslice_readonlyreg_check_test success res_glb_idx:0x%x,ip_slice_idx:%d\n",
                          res_glb_idx, core_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clkgen_coreslice_readonlyreg_check_test failed res_glb_idx:0x%x,ip_slice_idx:%d\n",
            res_glb_idx, core_slice_idx);
    return false;
}

//*****************************************************************************
//case1.1.test4
//! hal_clkgen_other_readonlyreg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \phy_addr saf sec soc display phy address
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_other_readonlyreg_check_test(void *handle,
        paddr_t phy_addr)
{
    bool ret = false;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;
    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clkgen_other_readonlyreg_check_test start \n");

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if (l_clkgenInstance->controllerTable->other_readonlyreg_check_test) {
        ret = l_clkgenInstance->controllerTable->other_readonlyreg_check_test(
                  clkgen_base_addr);
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clkgen_other_readonlyreg_check_test end\n");
    return ret;
}

//*****************************************************************************
//case1.2 test4
//! hal_clkgen_ipslice_rw_reg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx ip slice global resource index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_ipslice_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_test_instance_t *l_clkgenInstance = NULL;
    int32_t ip_slice_idx = -1;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &ip_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_ipslice_rw_reg_check_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((ip_slice_idx != -1) && (ip_slice_idx >= DEFAULT_IPSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->ipslice_rw_reg_check_test) {
        ip_slice_idx -= DEFAULT_IPSLICE_IDX_START;

        if (l_clkgenInstance->controllerTable->ipslice_rw_reg_check_test(
                    clkgen_base_addr, ip_slice_idx)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clkgen_ipslice_rw_reg_check_test success res_glb_idx:0x%x,ip_slice_idx:%d\n",
                          res_glb_idx, ip_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clkgen_ipslice_rw_reg_check_test failed res_glb_idx:0x%x,ip_slice_idx:%d\n",
            res_glb_idx, ip_slice_idx);
    return false;
}

//*****************************************************************************
//case1.2   test5
//! hal_clkgen_busslice_rw_reg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx bus slice global resource index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_busslice_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    int32_t bus_slice_idx = -1;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &bus_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_busslice_rw_reg_check_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((bus_slice_idx != -1) && (bus_slice_idx >= DEFAULT_BUSSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->busslice_rw_reg_check_test) {
        bus_slice_idx -= DEFAULT_BUSSLICE_IDX_START;
        bus_slice_idx /= 2;

        if (l_clkgenInstance->controllerTable->busslice_rw_reg_check_test(
                    clkgen_base_addr, bus_slice_idx)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clkgen_busslice_rw_reg_check_test success res_glb_idx:0x%x, slice_idx:%d\n",
                          res_glb_idx, bus_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clkgen_busslice_rw_reg_check_test failed res_glb_idx:0x%x, slice_idx:%d\n",
            res_glb_idx, bus_slice_idx);
    return false;
}


//*****************************************************************************
//case1.2 test6
//! hal_clkgen_coreslice_rw_reg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx ip slice start index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_coreslice_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    int32_t core_slice_idx = -1;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &core_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_coreslice_rw_reg_check_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    //coreslice id
    if ((core_slice_idx != -1)
            && (core_slice_idx >= DEFAULT_CORESLICE_IDX_START
                && core_slice_idx < DEFAULT_UUUSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->coreslice_rw_reg_check_test) {
        core_slice_idx -= DEFAULT_CORESLICE_IDX_START;

        if (l_clkgenInstance->controllerTable->coreslice_rw_reg_check_test(
                    clkgen_base_addr, core_slice_idx)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clkgen_coreslice_rw_reg_check_test success res_glb_idx:0x%x, slice_idx:%d\n",
                          res_glb_idx, core_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clkgen_coreslice_rw_reg_check_test failed res_glb_idx:0x%x, slice_idx:%d\n",
            res_glb_idx, core_slice_idx);
    return false;
}

//*****************************************************************************
//case1.2 test6
//! hal_clkgen_lpgating_rw_reg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx gating global resource index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_lpgating_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    int ret = -1;
    int32_t gating_idx = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &gating_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_lpgating_rw_reg_check_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    //enable clk gating
    if ((gating_idx >= DEFAULT_LPGATING_IDX_START) && (gating_idx != -1)
            && l_clkgenInstance->controllerTable->lpgating_rw_reg_check_test) {
        gating_idx -= DEFAULT_LPGATING_IDX_START;

        if (l_clkgenInstance->controllerTable->lpgating_rw_reg_check_test(
                    clkgen_base_addr, gating_idx)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clkgen_lpgating_rw_reg_check_test res_glb_idx:0x%x success gating_idx:%d\n",
                          res_glb_idx, gating_idx);
            return true;
        }
    }

    LTRACEF("hal_clkgen_lpgating_rw_reg_check_test res_glb_idx:0x%x enable failed gating_idx:%d\n",
            res_glb_idx, gating_idx);
    return false;
}

//*****************************************************************************
//case1.2 test7
//! hal_clkgen_uuuslice_rw_reg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx ip slice start index
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_uuuslice_rw_reg_check_test(void *handle,
        uint32_t res_glb_idx)
{
    int ret = -1;
    paddr_t phy_addr;
    vaddr_t clkgen_base_addr = 0x0;
    int32_t uuu_slice_idx = -1;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &uuu_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_uuuslice_rw_reg_check_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if ((uuu_slice_idx != -1) && (uuu_slice_idx >= DEFAULT_UUUSLICE_IDX_START)
            && l_clkgenInstance->controllerTable->uuuslice_rw_reg_check_test) {
        //set clk src and div
        uuu_slice_idx -= DEFAULT_UUUSLICE_IDX_START;

        if (l_clkgenInstance->controllerTable->uuuslice_rw_reg_check_test(
                    clkgen_base_addr, uuu_slice_idx)) {
            LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                          "hal_clkgen_uuuslice_rw_reg_check_test success res_glb_idx:0x%x  slice_idx:%d\n",
                          res_glb_idx, uuu_slice_idx);
            return true;
        }
    }

    LTRACEF("hal_clkgen_uuuslice_rw_reg_check_test not find res_glb_idx:0x%x  slice_idx:%d\n",
            res_glb_idx, uuu_slice_idx);
    return false;
}

//*****************************************************************************
//case1.2 test7
//! hal_clkgen_other_rw_reg_check_test.
//!
//! \handle clkgen handle for clkgen func.
//! \phy_addr saf sec soc display phy address
//!
//! This function is for clkgen readonly register test.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_other_rw_reg_check_test(void *handle, paddr_t phy_addr)
{
    bool ret = false;
    vaddr_t clkgen_base_addr = 0x0;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;
    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clkgen_other_rw_reg_check_test start \n");

    clkgen_base_addr = (vaddr_t)_ioaddr(phy_addr);

    if (l_clkgenInstance->controllerTable->other_rw_reg_check_test) {
        ret = l_clkgenInstance->controllerTable->other_rw_reg_check_test(
                  clkgen_base_addr);
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clkgen_other_rw_reg_check_test end\n");
    return ret;
}

//*****************************************************************************
//case1.3 test8
//! hal_clkgen_ip_clock_test.
//!
//! \handle clkgen handle for clkgen func.
//! \res_glb_idx ip slice global resource index
//! \ip_slice_default ip slice default value
//!
//! This function is for clkgen ip clock test .
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_ip_clock_test(void *handle, uint32_t res_glb_idx,
                              const clkgen_ip_slice_t *ip_slice_default)
{
    int ret = -1;
    uint32_t get_clk = 0;
    int32_t ip_slice_idx = -1;
    paddr_t phy_addr;
    clkgen_app_ip_cfg_t ip_cfg;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &ip_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_ip_clock_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    ip_slice_idx -= DEFAULT_IPSLICE_IDX_START;
    ip_cfg.pre_div = ip_slice_default[ip_slice_idx].pre_div;
    ip_cfg.post_div = ip_slice_default[ip_slice_idx].post_div;
    ip_cfg.clk_src_select_num =
        ip_slice_default[ip_slice_idx].clk_src_select_num;

    if (!hal_clock_ipclk_set(l_clkgenInstance, res_glb_idx, &ip_cfg)) {
        LTRACEF("hal_clkgen_ip_clock_test res_glb_idx:0x%x test failed\n",
                res_glb_idx);
        return false;
    }

    spin(100);

    if (ip_slice_default[ip_slice_idx].sel_clk > 2 * 24 * CLK_MHZ ) {
        get_clk = hal_clock_ipclk_get(l_clkgenInstance, res_glb_idx,
                                      mon_ref_clk_24M, 0);
    }
    else {
        get_clk = hal_clock_ipclk_get(l_clkgenInstance, res_glb_idx,
                                      mon_ref_clk_32K, 0);
    }

    if ((get_clk <= (ip_slice_default[ip_slice_idx].sel_clk * 95 / 100))
            || (get_clk >= (ip_slice_default[ip_slice_idx].sel_clk * 105 / 100))) {
        LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                      "hal_clkgen_ip_clock_test get_clk:%d ; sel_clk:%d\n", get_clk,
                      ip_slice_default[ip_slice_idx].sel_clk);
        return false;
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clkgen_ip_clock_test success get_clk:%d ; sel_clk:%d\n", get_clk,
                  ip_slice_default[ip_slice_idx].sel_clk);
    return true;
}

//*****************************************************************************
//case1.4 test9
//! hal_clkgen_ip_clock_div_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx ip slice start index
//! \ip_slice_default ip slice default value
//!
//! This function is for clkgen ip clock div test .
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_ip_clock_div_test(void *handle, uint32_t res_glb_idx,
                                  const clkgen_ip_slice_t *ip_slice_default)
{
    int ret = -1;
    uint32_t get_clk = 0;
    int32_t ip_slice_idx = -1;
    paddr_t phy_addr;
    clkgen_app_ip_cfg_t ip_cfg;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &ip_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_ip_clock_div_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    ip_slice_idx -= DEFAULT_IPSLICE_IDX_START;
    ip_cfg.pre_div = 1;
    ip_cfg.post_div = 1;
    ip_cfg.clk_src_select_num =
        ip_slice_default[ip_slice_idx].clk_src_select_num;

    if (!hal_clock_ipclk_set(l_clkgenInstance, res_glb_idx, &ip_cfg)) {
        LTRACEF("hal_clkgen_ip_clock_div_test res_glb_idx:0x%x failed ip_slice_idx:%d \n",
                res_glb_idx, ip_slice_idx);
        return false;
    }

    spin(100);

    if (ip_slice_default[ip_slice_idx].sel_clk > 2 * 24 * CLK_MHZ ) {
        get_clk = hal_clock_ipclk_get(l_clkgenInstance, res_glb_idx,
                                      mon_ref_clk_24M, 0);
    }
    else {
        get_clk = hal_clock_ipclk_get(l_clkgenInstance, res_glb_idx,
                                      mon_ref_clk_32K, 0);
    }

    if ((get_clk <= ((ip_slice_default[ip_slice_idx].sel_clk /
                      (ip_cfg.pre_div + 1) / (ip_cfg.post_div + 1)) * (95 / 100)))
            || (get_clk >= ((ip_slice_default[ip_slice_idx].sel_clk /
                             (ip_cfg.pre_div + 1) / (ip_cfg.post_div + 1)) * (105 / 100)))) {
        LTRACEF("hal_clkgen_ip_clock_div_test failed get_clk:%d ; sel_clk:%d\n",
                get_clk, ip_slice_default[ip_slice_idx].sel_clk);
        return false;
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clkgen_ip_clock_div_test success get_clk:%d ; sel_clk:%d\n", get_clk,
                  ip_slice_default[ip_slice_idx].sel_clk);
    return true;
}
//*****************************************************************************
//case1.5 test10
//! hal_clkgen_bus_clock_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx bus slice start index
//! \bus_slice_default bus slice default frequence
//!
//! This function is for clkgen bus clock test .
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_bus_clock_test(void *handle, uint32_t res_glb_idx,
                               const clkgen_bus_slice_t *bus_slice_default)
{
    int ret = -1;
    paddr_t phy_addr;
    int32_t bus_slice_idx = -1;
    uint32_t get_clk = 0;
    clkgen_app_bus_cfg_t bus_app_cfg;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &bus_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_bus_clock_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    bus_slice_idx -= DEFAULT_BUSSLICE_IDX_START;
    bus_slice_idx /= 2;
    bus_app_cfg.clk_src_select_a_num =
        bus_slice_default[bus_slice_idx].clk_src_select_a_num;//default--796MHz
    bus_app_cfg.clk_src_select_b_num =
        bus_slice_default[bus_slice_idx].clk_src_select_b_num,//default--796MHz
        bus_app_cfg.clk_a_b_select =
            bus_slice_default[bus_slice_idx].clk_a_b_select;
    bus_app_cfg.pre_div_a = bus_slice_default[bus_slice_idx].pre_div_a;
    bus_app_cfg.pre_div_b = bus_slice_default[bus_slice_idx].pre_div_b;
    bus_app_cfg.post_div = bus_slice_default[bus_slice_idx].post_div;
    bus_app_cfg.m_div = bus_slice_default[bus_slice_idx].m_div;
    bus_app_cfg.n_div = bus_slice_default[bus_slice_idx].n_div;
    bus_app_cfg.p_div = bus_slice_default[bus_slice_idx].p_div;
    bus_app_cfg.q_div = bus_slice_default[bus_slice_idx].q_div;

    if (!hal_clock_busclk_set(l_clkgenInstance, res_glb_idx, &bus_app_cfg)) {
        LTRACEF("hal_clkgen_bus_clock_test res_glb_idx:0x%x failed\n",
                res_glb_idx);
        return false;
    }

    spin(100);

    if (bus_slice_default[bus_slice_idx].a_sel_clk > 2 * 24 * CLK_MHZ ) {
        get_clk = hal_clock_busclk_get(l_clkgenInstance, res_glb_idx,
                                       mon_ref_clk_24M, 0);
    }
    else {
        get_clk = hal_clock_busclk_get(l_clkgenInstance, res_glb_idx,
                                       mon_ref_clk_32K, 0);
    }

    if ((get_clk <= (bus_slice_default[bus_slice_idx].a_sel_clk * 95 / 100))
            || (get_clk >= (bus_slice_default[bus_slice_idx].a_sel_clk * 105 / 100))) {
        LTRACEF("hal_clkgen_bus_clock_test failed get_clk:%d ; sel_clk:%d\n",
                get_clk, bus_slice_default[bus_slice_idx].a_sel_clk);
        return false;
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clkgen_bus_clock_test success get_clk:%d ; sel_clk:%d\n", get_clk,
                  bus_slice_default[bus_slice_idx].a_sel_clk);
    return true;
}

//*****************************************************************************
//case1.6 test11
//! hal_clkgen_bus_clock_div_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx bus slice start index
//! \bus_slice_default bus slice default frequence
//!
//! This function is for clkgen bus clock div test .
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_bus_clock_div_test(void *handle, uint32_t res_glb_idx,
                                   const clkgen_bus_slice_t *bus_slice_default)
{
    int ret = -1;
    paddr_t phy_addr;
    int32_t bus_slice_idx = -1;
    uint32_t get_clk = 0;
    clkgen_app_bus_cfg_t bus_app_cfg;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &bus_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_bus_clock_div_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    bus_slice_idx -= DEFAULT_BUSSLICE_IDX_START;
    bus_slice_idx /= 2;
    bus_app_cfg.clk_src_select_a_num =
        bus_slice_default[bus_slice_idx].clk_src_select_a_num;//default--796MHz
    bus_app_cfg.clk_src_select_b_num =
        bus_slice_default[bus_slice_idx].clk_src_select_b_num,//default--796MHz
        bus_app_cfg.clk_a_b_select =
            bus_slice_default[bus_slice_idx].clk_a_b_select;
    bus_app_cfg.pre_div_a = 1;
    bus_app_cfg.pre_div_b = 1;
    bus_app_cfg.post_div = 1;
    bus_app_cfg.m_div = bus_slice_default[bus_slice_idx].m_div;
    bus_app_cfg.n_div = bus_slice_default[bus_slice_idx].n_div;
    bus_app_cfg.p_div = bus_slice_default[bus_slice_idx].p_div;
    bus_app_cfg.q_div = bus_slice_default[bus_slice_idx].q_div;

    if (!hal_clock_busclk_set(l_clkgenInstance, res_glb_idx, &bus_app_cfg)) {
        LTRACEF("hal_clkgen_bus_clock_div_test res_glb_idx:0x%x failed\n",
                res_glb_idx);
        return false;
    }

    spin(100);

    if (bus_slice_default[bus_slice_idx].a_sel_clk > 2 * 24 * CLK_MHZ ) {
        get_clk = hal_clock_busclk_get(l_clkgenInstance, res_glb_idx,
                                       mon_ref_clk_24M, 0);
    }
    else {
        get_clk = hal_clock_busclk_get(l_clkgenInstance, res_glb_idx,
                                       mon_ref_clk_32K, 0);
    }

    if ((get_clk <= ((bus_slice_default[bus_slice_idx].a_sel_clk /
                      (bus_app_cfg.pre_div_a + 1) / (bus_app_cfg.post_div + 1)) * (95 / 100)))
            || (get_clk >= ((bus_slice_default[bus_slice_idx].a_sel_clk /
                             (bus_app_cfg.pre_div_a + 1) / (bus_app_cfg.post_div + 1)) *
                            (105 / 100)))) {
        LTRACEF("hal_clkgen_bus_clock_div_test failed get_clk:%d ; sel_clk:%d\n",
                get_clk, bus_slice_default[bus_slice_idx].a_sel_clk);
        return false;
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clkgen_bus_clock_div_test success get_clk:%d ; sel_clk:%d\n", get_clk,
                  bus_slice_default[bus_slice_idx].a_sel_clk);
    return true;
}

//*****************************************************************************
//case1.7 test12
//! hal_clkgen_core_clock_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx core slice start index
//! \core_slice_default core slice default frequence
//!
//! This function is for clkgen bus clock test .
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_core_clock_test(void *handle, uint32_t res_glb_idx,
                                const clkgen_core_slice_t *core_slice_default)
{
    int ret = -1;
    paddr_t phy_addr;
    int32_t core_slice_idx = -1;
    uint32_t get_clk = 0;
    clkgen_app_core_cfg_t core_app_cfg;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &core_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_core_clock_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    core_slice_idx -= DEFAULT_CORESLICE_IDX_START;
    core_app_cfg.clk_src_select_a_num =
        core_slice_default[core_slice_idx].clk_src_select_a_num;
    core_app_cfg.clk_src_select_b_num =
        core_slice_default[core_slice_idx].clk_src_select_b_num;
    core_app_cfg.clk_a_b_select =
        core_slice_default[core_slice_idx].clk_a_b_select;
    core_app_cfg.post_div = core_slice_default[core_slice_idx].post_div;

    if (!hal_clock_coreclk_set(l_clkgenInstance, res_glb_idx, &core_app_cfg)) {
        LTRACEF("hal_clkgen_core_clock_test res_glb_idx:0x%x failed\n",
                res_glb_idx);
        return false;
    }

    spin(100);

    if (core_slice_default[core_slice_idx].a_sel_clk > 2 * 24 * CLK_MHZ ) {
        get_clk = hal_clock_coreclk_get(l_clkgenInstance, res_glb_idx,
                                        mon_ref_clk_24M, 0);
    }
    else {
        get_clk = hal_clock_coreclk_get(l_clkgenInstance, res_glb_idx,
                                        mon_ref_clk_32K, 0);
    }

    if ((get_clk <= (core_slice_default[core_slice_idx].a_sel_clk * 95 / 100))
            || (get_clk >= (core_slice_default[core_slice_idx].a_sel_clk * 105 /
                            100))) {
        LTRACEF("hal_clkgen_core_clock_test failed get_clk:%d ; sel_clk:%d\n",
                get_clk, core_slice_default[core_slice_idx].a_sel_clk);
        return false;
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clkgen_bus_clock_test success get_clk:%d ; sel_clk:%d\n", get_clk,
                  core_slice_default[core_slice_idx].a_sel_clk);
    return true;
}
//*****************************************************************************
//case1.8 test13
//! hal_clkgen_core_clock_div_test.
//!
//! \handle clkgen handle for clkgen func.
//! \start_idx core slice start index
//!
//! This function is for clkgen core clock div test .
//!
//! \return bool status
//
//*****************************************************************************
bool hal_clkgen_core_clock_div_test(void *handle, uint32_t res_glb_idx,
                                    const clkgen_core_slice_t *core_slice_default)
{
    int ret = -1;
    paddr_t phy_addr;
    int32_t core_slice_idx = -1;
    uint32_t get_clk = 0;
    clkgen_app_core_cfg_t core_app_cfg;
    clkgen_test_instance_t *l_clkgenInstance = NULL;

    ASSERT((handle != NULL));

    l_clkgenInstance = (clkgen_test_instance_t *)handle;

    ret = res_get_info_by_id(res_glb_idx, &phy_addr, &core_slice_idx);

    if (ret == -1) {
        LTRACEF("hal_clkgen_core_clock_div_test res_glb_idx:0x%x is not find\n",
                res_glb_idx);
        return true;
    }

    core_slice_idx -= DEFAULT_CORESLICE_IDX_START;
    core_app_cfg.clk_src_select_a_num =
        core_slice_default[core_slice_idx].clk_src_select_a_num;
    core_app_cfg.clk_src_select_b_num =
        core_slice_default[core_slice_idx].clk_src_select_b_num;
    core_app_cfg.clk_a_b_select =
        core_slice_default[core_slice_idx].clk_a_b_select;
    core_app_cfg.post_div = 1;

    if (!hal_clock_coreclk_set(l_clkgenInstance, res_glb_idx, &core_app_cfg)) {
        LTRACEF("hal_clkgen_core_clock_div_test res_glb_idx:0x%x failed\n",
                res_glb_idx);
        return false;
    }

    spin(100);

    if (core_slice_default[core_slice_idx].a_sel_clk > 2 * 24 * CLK_MHZ ) {
        get_clk = hal_clock_coreclk_get(l_clkgenInstance, res_glb_idx,
                                        mon_ref_clk_24M, 0);
    }
    else {
        get_clk = hal_clock_coreclk_get(l_clkgenInstance, res_glb_idx,
                                        mon_ref_clk_32K, 0);
    }

    if ((get_clk <= ((core_slice_default[core_slice_idx].a_sel_clk /
                      (core_app_cfg.post_div + 1)) * (95 / 100)))
            || (get_clk >= ((core_slice_default[core_slice_idx].a_sel_clk /
                             (core_app_cfg.post_div + 1)) * (105 / 100)))) {
        LTRACEF("hal_clkgen_core_clock_div_test failed get_clk:%d ; sel_clk:%d\n",
                get_clk, core_slice_default[core_slice_idx].a_sel_clk);
        return false;
    }

    LTRACEF_LEVEL(DEFAULT_CLKGEN_LOG_LEVEL,
                  "hal_clkgen_core_clock_div_test success get_clk:%d ; sel_clk:%d\n",
                  get_clk, core_slice_default[core_slice_idx].a_sel_clk);
    return true;
}
