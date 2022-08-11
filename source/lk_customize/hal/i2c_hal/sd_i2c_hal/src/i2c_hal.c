//*****************************************************************************
//
// i2c_hal.c - Driver for the i2c hal Module.
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
#include "dw_i2c.h"
#include "i2c_hal.h"
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include "target_res.h"
#include "domain_res_cnt.h"

#define I2C_LOG INFO

#define LOCAL_TRACE 0

//#define I2C_RES_NUM 16
#ifndef I2C_RES_NUM
#define I2C_RES_NUM 0
#endif

#define MAX_I2C_DEVICE_NUM 16*2
#define MAX_I2C_RETRY 3
/* i2c global instance */
static i2c_instance_t g_i2cInstance[MAX_I2C_DEVICE_NUM] = {0};
//static mutex_t i2c_mutex;
spin_lock_t i2c_spin_lock = SPIN_LOCK_INITIAL_VALUE;

static dw_i2c_context g_i2c[I2C_RES_NUM];
static bool g_i2c_init = false;

const i2c_glb_idx_to_num g_i2c_glb_idx_to_num[DEFAULT_I2C_MAX_NUM] = {
    {RES_I2C_I2C1, 1, I2C1_IC_INTR_NUM},
    {RES_I2C_I2C2, 2, I2C2_IC_INTR_NUM},
    {RES_I2C_I2C3, 3, I2C3_IC_INTR_NUM},
    {RES_I2C_I2C4, 4, I2C4_IC_INTR_NUM},
    {RES_I2C_I2C5, 5, I2C5_IC_INTR_NUM},
    {RES_I2C_I2C6, 6, I2C6_IC_INTR_NUM},
    {RES_I2C_I2C7, 7, I2C7_IC_INTR_NUM},
    {RES_I2C_I2C8, 8, I2C8_IC_INTR_NUM},
    {RES_I2C_I2C9, 9, I2C9_IC_INTR_NUM},
    {RES_I2C_I2C10, 10, I2C10_IC_INTR_NUM},
    {RES_I2C_I2C11, 11, I2C11_IC_INTR_NUM},
    {RES_I2C_I2C12, 12, I2C12_IC_INTR_NUM},
    {RES_I2C_I2C13, 13, I2C13_IC_INTR_NUM},
    {RES_I2C_I2C14, 14, I2C14_IC_INTR_NUM},
    {RES_I2C_I2C15, 15, I2C15_IC_INTR_NUM},
    {RES_I2C_I2C16, 16, I2C16_IC_INTR_NUM},
};


/* i2c driver interface */
static const i2c_drv_controller_interface_t s_i2cDrvInterface = {
    dw_i2c_set_busconfig,
    dw_i2c_init_after,
    NULL, //dw_i2c_transmit,
    NULL, //dw_i2c_receive,
    dw_i2c_slave_receive,
    NULL, //dw_i2c_write_reg_bytes,
    NULL, //dw_i2c_read_reg_bytes,
    dw_i2c_scan,
    NULL, //dw_i2c_write_reg,
    dw_i2c_write_reg_data,
    dw_i2c_read_reg_data,
    dw_i2c_common_xfer,
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
static void hal_i2c_get_controller_interface(const
        i2c_drv_controller_interface_t **controllerTable)
{
    *controllerTable = &s_i2cDrvInterface;
}

//*****************************************************************************
//
//! hal_i2c_get_irq.
//!
//! \void.
//!
//! This function get i2c instance hand.
//!
//! \return i2c irq number
//
//*****************************************************************************
static uint32_t hal_i2c_get_irq(uint32_t i2c_res_glb_idx)
{
    int i;

    for (i = 0; i < DEFAULT_I2C_MAX_NUM; i++) {
        if (g_i2c_glb_idx_to_num[i].res_glb_idx == i2c_res_glb_idx)
            return g_i2c_glb_idx_to_num[i].irq;
    }

    return 0;
}

static void g_i2c_initial(void)
{
    int i;

    if (g_i2c_init)
        return ;

    for (i = 0; i < I2C_RES_NUM; i++) {
        g_i2c[i].bus = 0xff;
        g_i2c[i].bus_lock = SPIN_LOCK_INITIAL_VALUE;
        mutex_init(&g_i2c[i].bus_mutex);
    }

    g_i2c_init = true;
}
static int g_i2c_get_index(int32_t i2cno)
{
    int i;

    for (i = 0; i < I2C_RES_NUM; i++) {
        if (g_i2c[i].bus == (uint32_t)i2cno)
            return i;

        if (g_i2c[i].bus == 0xff)
            return i;
    }

    dprintf(0, "%s: index %d not found!\n", __func__, i2cno);
    return -1;
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
static i2c_instance_t *hal_i2c_get_instance(uint32_t i2c_res_glb_idx)
{
    uint8_t i = 0, j;
    dw_i2c_config_t l_config;
    uint8_t cur_i2c_res_index = 0;
    int32_t cur_i2c_soc_bus_index = 0;
    addr_t cur_i2c_phy_addr;
    uint32_t irq;
    spin_lock_saved_state_t states;

    if (res_get_info_by_id(i2c_res_glb_idx, &cur_i2c_phy_addr,
                           &cur_i2c_soc_bus_index)) {
        dprintf(0, "%s: error i2c_res_glb_idx:0x%x\n", __func__, i2c_res_glb_idx);
        goto err0;
    }

    dprintf(I2C_LOG, "%s(): cur_i2c_res_index=%d, cur_i2c_soc_bus_index=%d.\n",
            __func__, cur_i2c_res_index, cur_i2c_soc_bus_index);

    if (cur_i2c_soc_bus_index < 1) {
        dprintf(0, "%s: wrong bus index\n", __func__);
        goto err0;
    }

    cur_i2c_soc_bus_index = cur_i2c_soc_bus_index - 1;

    irq = hal_i2c_get_irq(i2c_res_glb_idx);
    if (irq == 0) {
        dprintf(0, "%s: wrong bus irq\n", __func__);
        goto err0;
    }

    //mutex_acquire(&i2c_mutex);
    spin_lock_irqsave(&i2c_spin_lock, states);
    g_i2c_initial();

    for (i = 0; i < MAX_I2C_DEVICE_NUM; i++) {
        if (g_i2cInstance[i].occupied != 1) {
            uint8_t *buffer = (uint8_t *)&g_i2cInstance[i];
            memset(buffer, 0, sizeof(i2c_instance_t));
            g_i2cInstance[i].occupied = 1;
            j = g_i2c_get_index(cur_i2c_soc_bus_index);

            if (j < 0)
                goto err;

            spin_unlock_irqrestore(&i2c_spin_lock, states);
            /* get i2c driver API table */
            hal_i2c_get_controller_interface(&(g_i2cInstance[i].controllerTable));

            if (g_i2cInstance[i].controllerTable) {
                g_i2cInstance[i].cur_i2c_soc_busnum = cur_i2c_soc_bus_index;
                dprintf(I2C_LOG, "%s(): i=%d, g_i2c[%d].is_added=%d.\n",
                        __func__, i, j, g_i2c[j].is_added);

                spin_lock_irqsave(&g_i2c[j].bus_lock, states);
                if (!g_i2c[j].is_added) {
#if WITH_KERNEL_VM
                    l_config.io_base = (vaddr_t)paddr_to_kvaddr(cur_i2c_phy_addr);
#else
                    l_config.io_base = cur_i2c_phy_addr;
#endif
                    l_config.speed = I2C_SPEED_FAST;
                    l_config.mode = MASTER_MODE;
                    l_config.addr_bits = ADDR_7BITS;
                    l_config.restart = true;
                    l_config.irq = irq;
                    l_config.poll = 0;
                    g_i2c[j].bus = cur_i2c_soc_bus_index;
                    g_i2cInstance[i].i2c_con = &g_i2c[j];

                    if (g_i2cInstance[i].controllerTable->set_busconfig) {
                        g_i2cInstance[i].controllerTable->set_busconfig(
                            g_i2cInstance[i].i2c_con, &l_config);
                    }
                }
                else {
                    g_i2cInstance[i].i2c_con = &g_i2c[j];
                }
                spin_unlock_irqrestore(&g_i2c[j].bus_lock, states);

                g_i2cInstance[i].i2c_cfg.speed = g_i2cInstance[i].i2c_con->info.speed;
                g_i2cInstance[i].i2c_cfg.addr_bits = g_i2cInstance[i].i2c_con->info.addr_bits;
                g_i2cInstance[i].i2c_cfg.mode = g_i2cInstance[i].i2c_con->info.mode;
                g_i2cInstance[i].i2c_cfg.slave_addr = g_i2cInstance[i].i2c_con->info.slave_addr;
                g_i2cInstance[i].i2c_cfg.poll = g_i2cInstance[i].i2c_con->info.poll;

                /*g_i2cInstance[i].occupied = 1;
                //mutex_release(&i2c_mutex);
                spin_unlock_irqrestore(&i2c_spin_lock, states);*/
                if (g_i2cInstance[i].controllerTable->init_after)
                    g_i2cInstance[i].controllerTable->init_after(g_i2cInstance[i].i2c_con);

                return &g_i2cInstance[i];
            }
            else {
                goto err0;
            }
        }
    }

err:
    //mutex_release(&i2c_mutex);
    spin_unlock_irqrestore(&i2c_spin_lock, states);
err0:
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
static void hal_i2c_release_instance(i2c_instance_t *i2cInstance)
{
    //mutex_acquire(&i2c_mutex);
    if (i2cInstance == NULL)
        return ;

    spin_lock_saved_state_t states;
    spin_lock_irqsave(&i2c_spin_lock, states);
    i2cInstance->occupied = 0;
    //mutex_release(&i2c_mutex);
    spin_unlock_irqrestore(&i2c_spin_lock, states);
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
bool hal_i2c_creat_handle(void **handle, uint32_t i2c_res_glb_idx)
{
    i2c_instance_t  *instance = NULL;
    HAL_ASSERT_PARAMETER(handle);
    instance = hal_i2c_get_instance(i2c_res_glb_idx);

    if (instance == NULL) {
        //mutex_destroy(&i2c_mutex);
        return false;
    }

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
bool hal_i2c_release_handle(void *handle)
{
    i2c_instance_t *instance = NULL;
    HAL_ASSERT_PARAMETER(handle);
    instance = (i2c_instance_t *)handle;
    instance->occupied = 0;
    //mutex_destroy(&i2c_mutex);
    return true;
}

i2c_app_config_t hal_i2c_get_busconfig(void *handle)
{
    i2c_app_config_t i2c_app_cfg;
    i2c_instance_t *instance = NULL;
    memset(&i2c_app_cfg, 0, sizeof(i2c_app_cfg));

    if (handle == NULL)
        return i2c_app_cfg;

    instance = (i2c_instance_t *)handle;
    i2c_app_cfg = instance->i2c_cfg;
    return i2c_app_cfg;
}

//*****************************************************************************
//
//! hal_i2c_set_busconfig.
//!
//! \handle i2c handle for i2c func.
//!
//! This function is for i2c used i2c_app_cfg parameter init.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_i2c_set_busconfig(void *handle, i2c_app_config_t *i2c_app_cfg)
{
    bool ret = false;
    i2c_instance_t *instance = NULL;
    HAL_ASSERT_PARAMETER(handle);
    dw_i2c_config_t l_config;
    instance = (i2c_instance_t *)handle;

    if (i2c_app_cfg->addr_bits == HAL_I2C_ADDR_10BITS) {
        dprintf(0, "%s(): 10bit address mode not implement!\n", __func__);
        return false;
    }

    if (i2c_app_cfg->mode == HAL_I2C_SLAVE_MODE) {
        dprintf(0, "%s(): slave mode not implement!\n", __func__);
        return false;
    }

    instance->i2c_cfg = *i2c_app_cfg;
    l_config.io_base = instance->i2c_con->info.io_base;
    l_config.irq = instance->i2c_con->info.irq;
    l_config.restart = true;
    l_config.mode = i2c_app_cfg->mode;
    l_config.speed = i2c_app_cfg->speed;
    l_config.addr_bits = i2c_app_cfg->addr_bits;
    l_config.slave_addr = i2c_app_cfg->slave_addr;
    l_config.poll = i2c_app_cfg->poll;

    if (instance->controllerTable->set_busconfig) {
        ret = instance->controllerTable->set_busconfig(
                  instance->i2c_con, &l_config);
    }

    if (!ret) {
        LTRACEF("hal_i2c_set_busconfig false\n");
        return false;
    }

    return true;
}

bool hal_i2c_transmit(void *handle, uint8_t address, const void *buf,
                      size_t cnt, bool start, bool stop)
{
    i2c_instance_t *instance = NULL;
    HAL_ASSERT_PARAMETER(handle);
    instance = (i2c_instance_t *)handle;

    if (instance->controllerTable->transmit)
        instance->controllerTable->transmit(instance->i2c_con,
                                            address, buf, cnt, start, stop);
    else
        dprintf(0, "%s(): not implement!\n", __func__);

    return true;
}

bool hal_i2c_receive(void *handle, uint8_t address, void *buf, size_t cnt,
                     bool start, bool stop)
{
    i2c_instance_t *instance = NULL;
    HAL_ASSERT_PARAMETER(handle);
    instance = (i2c_instance_t *)handle;

    if (instance->controllerTable->receive)
        instance->controllerTable->receive(instance->i2c_con,
                                           address, buf, cnt, start, stop);
    else
        dprintf(0, "%s(): not implement!\n", __func__);

    return true;
}

bool hal_i2c_write_reg_bytes(void *handle, uint8_t address, uint8_t reg,
                             void *buf, size_t cnt)
{
    i2c_instance_t *instance = NULL;
    HAL_ASSERT_PARAMETER(handle);
    instance = (i2c_instance_t *)handle;

    if (instance->controllerTable->write_reg_bytes)
        instance->controllerTable->write_reg_bytes(
            instance->i2c_con, address, reg, buf, cnt);
    else
        dprintf(0, "%s(): not implement!\n", __func__);

    return true;
}

bool hal_i2c_read_reg_bytes(void *handle, uint8_t address, uint8_t reg,
                            void *buf, size_t cnt)
{
    i2c_instance_t *instance = NULL;
    HAL_ASSERT_PARAMETER(handle);
    instance = (i2c_instance_t *)handle;

    if (instance->controllerTable->read_reg_bytes)
        instance->controllerTable->read_reg_bytes(
            instance->i2c_con, address, reg, buf, cnt);
    else
        dprintf(0, "%s(): not implement!\n", __func__);

    return true;
}

bool hal_i2c_slave_transmit(void *handle, void *buf, size_t cnt)
{
    dprintf(0, "%s(): not implement!\n", __func__);
    return false;
}

bool hal_i2c_slave_receive(void *handle, void *buf, size_t cnt)
{
    i2c_instance_t *instance = NULL;
    HAL_ASSERT_PARAMETER(handle);
    instance = (i2c_instance_t *)handle;

    if (instance->controllerTable->slave_receive)
        instance->controllerTable->slave_receive(instance->i2c_con,
                buf, cnt, true, true);
    else
        dprintf(0, "%s(): not implement!\n", __func__);

    return true;
}


bool hal_i2c_scan(void *handle, uint8_t address)
{
    i2c_instance_t *instance = NULL;
    status_t ret = 0;
    HAL_ASSERT_PARAMETER(handle);
    instance = (i2c_instance_t *)handle;

    if (instance->controllerTable->scan)
        ret = instance->controllerTable->scan(
                  instance->i2c_con, address);

    dprintf(0, "%s(): ret = %d,  not implement!\n", __func__, ret);
    return false;
}

bool hal_i2c_write_reg(void *handle, uint8_t address, void *reg,
                       size_t cnt)
{
    status_t ret = 0;
    i2c_instance_t *instance = NULL;
    int retry = 0;
    HAL_ASSERT_PARAMETER(handle);
    instance = (i2c_instance_t *)handle;

    if (instance->controllerTable->write_reg) {
        while (retry++ < MAX_I2C_RETRY) {
            ret = instance->controllerTable->write_reg(
                      instance->i2c_con, address, reg, cnt);

            if (!ret)
                break;
        }
    }
    else
        dprintf(0, "%s(): not implement!\n", __func__);

    dprintf(I2C_LOG, "%s(0x%x): ret=%d\n", __func__, address, ret);
    return ret;
}

status_t hal_i2c_write_reg_data(void *handle, uint8_t address, void *reg,
                                size_t reg_cnt, void *data, size_t cnt)
{
    i2c_instance_t *instance = NULL;
    status_t ret = -1;
    int retry = 0;
    HAL_ASSERT_PARAMETER(handle);
    instance = (i2c_instance_t *)handle;

    if (instance->controllerTable->write_reg_data) {
        while (retry++ < MAX_I2C_RETRY) {
            ret = instance->controllerTable->write_reg_data(
                      instance->i2c_con, address, reg, reg_cnt, data, cnt);

            if (!ret)
                break;
        }
    }

    dprintf(I2C_LOG, "%s: ret=%d\n", __func__, ret);
    return ret;
}

status_t hal_i2c_read_reg_data(void *handle, uint8_t address, void *reg,
                               size_t reg_cnt, void *data, size_t cnt)
{
    i2c_instance_t *instance = NULL;
    status_t ret = -1;
    int retry = 0;
    HAL_ASSERT_PARAMETER(handle);
    instance = (i2c_instance_t *)handle;

    if (instance->controllerTable->read_reg_data) {
        while (retry++ <  MAX_I2C_RETRY) {
            ret = instance->controllerTable->read_reg_data(
                      instance->i2c_con, address, reg, reg_cnt, data, cnt);

            if (!ret)
                break;
        }
    }

    dprintf(I2C_LOG, "%s: ret=%d\n", __func__, ret);
    return ret;
}

status_t hal_i2c_common_xfer(void *handle, struct i2c_msg *msgs, int num)
{
    i2c_instance_t *instance = NULL;
    status_t ret = -1;
    int retry = 0;
    HAL_ASSERT_PARAMETER(handle);
    instance = (i2c_instance_t *)handle;

    if (instance->controllerTable->common_xfer) {
        while (retry++ <  MAX_I2C_RETRY) {
            ret = instance->controllerTable->common_xfer(
                      instance->i2c_con, msgs, num);

            if (!ret)
                break;
        }
    }

    dprintf(I2C_LOG, "%s: ret=%d\n", __func__, ret);
    return ret;
}

