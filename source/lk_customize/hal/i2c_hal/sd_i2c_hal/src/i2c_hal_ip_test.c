//*****************************************************************************
//
// i2c_hal_ip_test.c - Driver for the i2c hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
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
#include "dw_i2c_test.h"
#include "i2c_hal_ip_test.h"
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include "lib/reg.h"

/*i2c global instance*/
static i2c_test_instance_t g_i2cTestInstance[DEFAULT_I2C_MAX_NUM] = {0};
static mutex_t i2c_mutex;

const i2c_test_glb_idx_to_num
g_i2c_test_glb_idx_to_num[DEFAULT_I2C_MAX_NUM]
= {
    {RES_I2C_I2C1, 1},
    {RES_I2C_I2C2, 2},
    {RES_I2C_I2C3, 3},
    {RES_I2C_I2C4, 4},
    {RES_I2C_I2C5, 5},
    {RES_I2C_I2C6, 6},
    {RES_I2C_I2C7, 7},
    {RES_I2C_I2C8, 8},
    {RES_I2C_I2C9, 9},
    {RES_I2C_I2C10, 10},
    {RES_I2C_I2C11, 11},
    {RES_I2C_I2C12, 12},
    {RES_I2C_I2C13, 13},
    {RES_I2C_I2C14, 14},
    {RES_I2C_I2C15, 15},
    {RES_I2C_I2C16, 16},
};

//static domain_res_t g_i2c_test_res_def = i2c_res_def;

/* i2c driver interface */
static const i2c_test_drv_controller_interface_t s_i2cTestDrvInterface = {
    dw_i2c_dump_all_reg_test,
    dw_i2c_read_only_reg_test,
    dw_i2c_rw_reg_test,
};

//*****************************************************************************
//
//! hal_i2c_get_controller_interface.
//!
//! \param controllerTable is i2c interface ptr
//!
//! This function get i2c driver interface.
//!
//! \return
//
//*****************************************************************************
static void hal_i2c_test_get_controller_interface(const
        i2c_test_drv_controller_interface_t **controllerTable)
{
    *controllerTable = &s_i2cTestDrvInterface;
}

//*****************************************************************************
//
//! hal_i2c_get_instance.
//!
//! \void.
//!
//! This function get i2c instance hand.
//!
//! \return i2c hanle
//
//*****************************************************************************
static i2c_test_instance_t *hal_i2c_test_get_instance(
    uint32_t i2c_res_glb_idx)
{
    uint8_t i = 0;
    int32_t cur_i2c_soc_busnum = 0;
    addr_t cur_i2c_phy_addr;

    if (res_get_info_by_id(i2c_res_glb_idx, &cur_i2c_phy_addr,
                           &cur_i2c_soc_busnum)) {
        LTRACEF("hal_i2c_test_creat_handle paramenter error i2c_res_glb_idx:%d\n",
                i2c_res_glb_idx);
        return false;
    }

    printf("hal_i2c_test_get_instance(): cur_i2c_soc_busnum=%d.\n",
           cur_i2c_soc_busnum);

    mutex_acquire(&i2c_mutex);

    for (i = 0; i < DEFAULT_I2C_MAX_NUM; i++) {
        if (g_i2cTestInstance[i].occupied != 1) {
            uint8_t *buffer = (uint8_t *)&g_i2cTestInstance[i];
            memset(buffer, 0, sizeof(i2c_test_instance_t));

            /* get i2c driver API table */
            hal_i2c_test_get_controller_interface(&
                                                  (g_i2cTestInstance[i].controllerTable));

            if (g_i2cTestInstance[i].controllerTable) {
                g_i2cTestInstance[i].cur_i2c_soc_busnum = cur_i2c_soc_busnum;
                g_i2cTestInstance[i].cur_i2c_phy_addr = cur_i2c_phy_addr;
                g_i2cTestInstance[i].occupied = 1;
                mutex_release(&i2c_mutex);
                return &g_i2cTestInstance[i];
            }
        }
    }

    mutex_release(&i2c_mutex);
    return NULL;
}

//*****************************************************************************
//
//! hal_i2c_release_instance.
//!
//! \void.
//!
//! This function release i2c instance hand.
//!
//! \return
//
//*****************************************************************************
static void hal_i2c_test_release_instance(i2c_test_instance_t *i2cInstance)
{
    mutex_acquire(&i2c_mutex);
    i2cInstance->occupied = 0;
    mutex_release(&i2c_mutex);
}


//*****************************************************************************
//
//! hal_i2c_creat_handle.
//!
//! \handle i2c handle for i2c func.
//!
//! This function get hal handle.
//!
//! \return i2c handle
//
//*****************************************************************************
bool hal_i2c_test_creat_handle(void **handle, uint32_t i2c_res_glb_idx)
{
    printf("hal_i2c_creat_handle(0x%x):+\n", i2c_res_glb_idx);
    i2c_test_instance_t  *instance = NULL;

    instance = hal_i2c_test_get_instance(i2c_res_glb_idx);

    if (instance == NULL) {
        return false;
    }

    printf("hal_i2c_creat_handle():.\n");
    *handle = instance;
    return true;
}

//*****************************************************************************
//
//! hal_i2c_release_handle.
//!
//! \void.
//!
//! This function delete i2c instance hand.
//!
//! \return
//
//*****************************************************************************
bool hal_i2c_test_release_handle(void *handle)
{
    i2c_test_instance_t *instance = NULL;

    HAL_ASSERT_PARAMETER(handle);

    instance = (i2c_test_instance_t *)handle;
    instance->occupied = 0;
    return true;
}

bool hal_i2c_dump_all_reg_test(void *handle)
{
    i2c_test_instance_t *instance = NULL;
    vaddr_t i2c_base_addr = 0;

    HAL_ASSERT_PARAMETER(handle);

    instance = (i2c_test_instance_t *)handle;
    i2c_base_addr = (vaddr_t)_ioaddr(
                        instance->cur_i2c_phy_addr);

    printf("hal_i2c_read_only_reg_test(): base=0x%lx.\n",
           instance->cur_i2c_phy_addr);

    if (instance->controllerTable->dump_all_reg_test)
        instance->controllerTable->dump_all_reg_test((
                    i2c_reg_type_t *)i2c_base_addr);

    return true;
}

bool hal_i2c_read_only_reg_test(void *handle)
{
    i2c_test_instance_t *instance = NULL;
    vaddr_t i2c_base_addr = 0;

    HAL_ASSERT_PARAMETER(handle);

    instance = (i2c_test_instance_t *)handle;
    i2c_base_addr = (vaddr_t)_ioaddr(
                        instance->cur_i2c_phy_addr);

    printf("hal_i2c_read_only_reg_test(): base=0x%lx.\n",
           instance->cur_i2c_phy_addr);

    if (instance->controllerTable->read_only_reg_test)
        instance->controllerTable->read_only_reg_test(i2c_base_addr);

    return true;
}

bool hal_i2c_rw_reg_test(void *handle)
{
    i2c_test_instance_t *instance = NULL;
    vaddr_t i2c_base_addr = 0;

    HAL_ASSERT_PARAMETER(handle);

    instance = (i2c_test_instance_t *)handle;
    i2c_base_addr = (vaddr_t)_ioaddr(
                        instance->cur_i2c_phy_addr);

    printf("hal_i2c_read_only_reg_test(): base=0x%lx.\n",
           instance->cur_i2c_phy_addr);

    if (instance->controllerTable->rw_reg_test)
        instance->controllerTable->rw_reg_test(i2c_base_addr);

    return true;
}
