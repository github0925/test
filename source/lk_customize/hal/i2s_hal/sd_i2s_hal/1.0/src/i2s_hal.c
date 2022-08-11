//*****************************************************************************
//
// i2s_hal.c
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include <platform/interrupts.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <lk/init.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include <kernel/thread.h>
#include <kernel/spinlock.h>
#include "i2s_hal.h"
#include "irq.h"
#include <sys/types.h>
#include <kernel/event.h>
#include <kernel/mutex.h>
#include "chip_res.h"
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif
#include "scr_hal.h"
#if ENABLE_SD_I2S
#include "i2s_cadence_sc.h"
#include "i2s_cadence_mc.h"
#endif

#define DEFAULT_I2S_MC_MAX_NUM 2
#define DEFAULT_I2S_SC_MAX_NUM 8

typedef struct _i2s_mc_drv {
    bool (*reg)(i2s_mc_config_info *cfg);
    bool (*config)(i2s_mc_init_t *cfg_info, void *handle);
    bool (*start)(void *handle);
    bool (*stop)(void *handle);
    enum handler_return (*transmit)(void *dev);
    void (*reg_cur_setting)(i2s_mc_config_info *dev);
} i2s_mc_drv_t;

typedef struct _i2s_sc_drv {
    bool (*reg)(i2s_sc_config_info *cfg);
    bool (*config)(i2s_sc_config_info *dev, i2s_sc_init_t *cfg);
    bool (*start)(void *handle);
    bool (*stop)(void *handle);
    enum handler_return (*transmit)(void *dev);
    void (*reg_cur_setting)(i2s_sc_config_info *dev);
} i2s_sc_drv_t;

typedef struct _i2s_mc_instance {
    const i2s_mc_drv_t *drv;
    i2s_mc_config_info cfg;
    uint8_t occupied;   /* !< 0 - the instance is not occupied; 1 - the instance is occupied */
    i2s_res_config_t res;
    bool inited;
    bool enabled;
} i2s_mc_instance_t;

/* i2s_sc instance  */
typedef struct _i2s_sc_instance {
    const i2s_sc_drv_t *drv;    /* !< i2s_sc driver interface */
    i2s_sc_config_info cfg;    /* !< i2s sc config */
    uint8_t occupied;    /* !< 0 - the instance is not occupied; 1 - the instance is occupied */
    i2s_res_config_t res;
    bool inited;
    bool enabled;
} i2s_sc_instance_t;    // using typedef def could skip a "struct" typing.

static i2s_mc_instance_t g_i2s_mc_instance[DEFAULT_I2S_MC_MAX_NUM];
static i2s_sc_instance_t g_i2s_sc_instance[DEFAULT_I2S_SC_MAX_NUM];
static mutex_t g_i2s_mc_mutex;
static mutex_t g_i2s_sc_mutex;

/* get irq number */
static const i2s_res_t g_i2s_irq_table[] = {
    { RES_I2S_SC_I2S_SC1, 1, I2S_SC1_INTERRUPT_NUM, 400000000 },
    { RES_I2S_SC_I2S_SC2, 2, I2S_SC2_INTERRUPT_NUM, 400000000 },
    { RES_I2S_SC_I2S_SC3, 3, I2S_SC3_INTERRUPT_NUM, 398000000 },
    { RES_I2S_SC_I2S_SC4, 4, I2S_SC4_INTERRUPT_NUM, 398000000 },
    { RES_I2S_SC_I2S_SC5, 5, I2S_SC5_INTERRUPT_NUM, 398000000 },
    { RES_I2S_SC_I2S_SC6, 6, I2S_SC6_INTERRUPT_NUM, 398000000 },
    { RES_I2S_SC_I2S_SC7, 7, I2S_SC7_INTERRUPT_NUM, 398000000 },
    { RES_I2S_SC_I2S_SC8, 8, I2S_SC8_INTERRUPT_NUM, 398000000 },
    { RES_I2S_MC_I2S_MC1, 9, I2S_MC1_INTREQ_NUM, 398000000 },
    { RES_I2S_MC_I2S_MC2, 10, I2S_MC2_INTREQ_NUM, 398000000 },
};

/* i2s mc driver interface */
static const i2s_mc_drv_t g_i2s_mc_drv = {
    i2s_mc_register,
    i2s_mc_config,
    i2s_mc_start,
    i2s_mc_stop,
    i2s_mc_transmit_intmode,
    i2s_mc_reg_cur_setting,    //for debug
};

/* i2s sc driver interface */
static const i2s_sc_drv_t g_i2s_sc_drv = {
    i2s_sc_register,
    i2s_sc_config,
    i2s_sc_start,
    i2s_sc_stop,
    i2s_sc_transmit_intmode,
    i2s_sc_reg_cur_setting,
};

static void hal_i2s_sc_get_controller_interface(const i2s_sc_drv_t **drv)
{
    *drv = &g_i2s_sc_drv;
}

static i2s_sc_instance_t *hal_i2s_sc_get_instance(u_int32_t res_id)
{
    uint32_t ret = 0;
    int32_t i2s_ctrl_id = 0, id = 0;
    paddr_t phy_addr = 0;

    mutex_acquire(&g_i2s_sc_mutex);
    // get paddr and control id.
    ret = res_get_info_by_id(res_id, &phy_addr, &i2s_ctrl_id);

    id = i2s_ctrl_id - 1;

    if ((ret == 0) && (g_i2s_sc_instance[id].occupied != 1)) {
        memset(&g_i2s_sc_instance[id], 0, sizeof(i2s_sc_instance_t));

        /* get i2s sc driver API table */
        hal_i2s_sc_get_controller_interface(&(g_i2s_sc_instance[id].drv));

        if (g_i2s_sc_instance[id].drv) {
            g_i2s_sc_instance[id].occupied = 1;
            g_i2s_sc_instance[id].res.res_id = res_id;
            g_i2s_sc_instance[id].res.ctrl_id = i2s_ctrl_id;
            g_i2s_sc_instance[id].res.phy_addr = phy_addr;
            g_i2s_sc_instance[id].res.irq_num = g_i2s_irq_table[id].irq_num;
            g_i2s_sc_instance[id].res.clock = g_i2s_irq_table[id].clock;
            mutex_release(&g_i2s_sc_mutex);
            return &g_i2s_sc_instance[id];
        }
    }

    mutex_release(&g_i2s_sc_mutex);
    return NULL;
}

static void hal_i2s_sc_release_instance(i2s_sc_instance_t *i2s_sc_Instance)
{
    mutex_acquire(&g_i2s_sc_mutex);
    i2s_sc_Instance->occupied = 0;

    event_destroy(&(i2s_sc_Instance->cfg.tx_comp));
    event_destroy(&(i2s_sc_Instance->cfg.rx_comp));

    mutex_release(&g_i2s_sc_mutex);
}

bool hal_i2s_sc_create_handle(void **handle, u_int32_t res_id)
{
    i2s_sc_instance_t *i2s_sc_Instance = hal_i2s_sc_get_instance(res_id);

    if (i2s_sc_Instance == NULL) {
        return false;
    }

    *handle = i2s_sc_Instance;
    return true;
}

bool hal_i2s_sc_release_handle(void *handle)
{
    ASSERT(handle);
    hal_i2s_sc_release_instance(handle);

    return true;
}

bool hal_i2s_sc_init(void *handle)
{
    ASSERT(handle);
    i2s_sc_instance_t *instance = handle;

    if (instance->inited) {
        dprintf(INFO, "hal_i2s_sc_init has inited.\n");
        return false;
    }

    instance->cfg.bus = instance->res.ctrl_id - 1;
    instance->cfg.clock = instance->res.clock;
    instance->cfg.interrupt_num = instance->res.irq_num;
    instance->cfg.base_addr = (paddr_t)_ioaddr(instance->res.phy_addr);
    event_init(&(instance->cfg.tx_comp), false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&(instance->cfg.rx_comp), false, EVENT_FLAG_AUTOUNSIGNAL);
    instance->inited = true;

    return instance->drv->reg(&instance->cfg);
}

bool hal_i2s_sc_deinit(void *handle)
{
    i2s_sc_instance_t *instance = handle;

    ASSERT(instance);

    if (instance->inited == false) {
        dprintf(INFO, "hal_i2s_sc_init has not inited\n");
        return false;
    }

    instance->inited = false;

    return true;
}

bool hal_i2s_sc_config(void *handle, i2s_sc_init_t *i2s_config)
{

    i2s_sc_instance_t *instance = handle;
    ASSERT(instance && instance->drv->config);
    int id = instance->cfg.bus;

    /* set scr  */
    if ((i2s_config->mode == I2S_SC_MOD_MASTER_FULL_DUPLEX) ||
            (i2s_config->mode == I2S_SC_MOD_SLAVE_FULL_DUPLEX)) {
        dprintf(INFO, "config sc full duplex\n");

        scr_signal_t signal;

        if (id < 2) {
            signal = SCR_SAFETY__RW__pio_i2s_sc12_oectrl_1_0;
        }
        else {
            id -= 2;
            signal = SCR_SEC__RW__pio_i2s_sc_sdo_sdi_ctrl_5_0;
        }

        scr_handle_t scr_handle = hal_scr_create_handle(signal);
        uint32_t data = hal_scr_get(scr_handle);
        dprintf(INFO, "get scr reg value: 0x%x\n", data);
        data |= 1 << (id);
        hal_scr_set(scr_handle, data);
        dprintf(INFO, "after set scr reg value: 0x%x\n", hal_scr_get(scr_handle));
        hal_scr_delete_handle(scr_handle);
    }

    return instance->drv->config(&instance->cfg, i2s_config);
}

bool hal_i2s_sc_start(void *handle)
{
    i2s_sc_instance_t *instance = handle;
    ASSERT(instance && instance->drv->start);

    return instance->drv->start(&instance->cfg);
}

bool hal_i2s_sc_stop(void *handle)
{
    i2s_sc_instance_t *instance = handle;
    ASSERT(instance && instance->drv->start);

    return instance->drv->stop(&instance->cfg);
}

bool hal_i2s_sc_transmit(void *handle)
{
    i2s_sc_instance_t *instance = handle;
    ASSERT(instance && instance->drv->transmit);

    return instance->drv->transmit(&instance->cfg);
}

paddr_t hal_i2s_sc_get_fifo_addr(void *handle)
{
    ASSERT(handle);
    i2s_sc_instance_t *instance = handle;

    return instance->res.phy_addr + I2S_SC_FIFO_OFFSET;
}

int hal_i2s_sc_wait_tx_comp_intmode(void *handle, int timeout)
{
    ASSERT(handle);
    i2s_sc_instance_t *instance = handle;
    event_wait_timeout(&(instance->cfg.tx_comp), timeout);

    return instance->cfg.cfg_info.tx_count;
}

int hal_i2s_sc_wait_rx_comp_intmode(void *handle, int timeout)
{
    ASSERT(handle);
    i2s_sc_instance_t *instance = handle;
    event_wait_timeout(&(instance->cfg.rx_comp), timeout);

    return instance->cfg.cfg_info.rx_count;
}

void hal_i2s_sc_show_config(void *handle)
{
    ASSERT(handle);
    i2s_sc_instance_t *instance = handle;

    instance->drv->reg_cur_setting(&instance->cfg);
    return;
}

//*****************************************************************************
//
//! hal_i2s_mc_get_controller_interface.
//!
//! \param drv is i2s mc interface ptr
//!
//! This function get i2s mc driver interface.
//!
//
//*****************************************************************************
static void hal_i2s_mc_get_controller_interface(const i2s_mc_drv_t **drv)
{
    *drv = &g_i2s_mc_drv;
}

//*****************************************************************************
//
//! hal_i2s_mc_get_instance.
//!
//! This function get i2s mc instance .
//!
//! \return i2s mc handle
//
//*****************************************************************************
static i2s_mc_instance_t *hal_i2s_mc_get_instance(u_int32_t res_id)
{
    uint32_t ret = 0;
    int32_t i2s_ctrl_id = 0, id = 0;
    paddr_t phy_addr = 0;

    mutex_acquire(&g_i2s_mc_mutex);
    // get paddr and control id.
    ret = res_get_info_by_id(res_id, &phy_addr, &i2s_ctrl_id);

    id = i2s_ctrl_id - 1;

    if ((ret == 0) && (g_i2s_mc_instance[id].occupied != 1)) {

        memset(&g_i2s_mc_instance[id], 0, sizeof(i2s_mc_instance_t));

        /* get i2s sc driver API table */
        hal_i2s_mc_get_controller_interface(&(g_i2s_mc_instance[id].drv));

        if (g_i2s_mc_instance[id].drv) {
            g_i2s_mc_instance[id].occupied = 1;
            g_i2s_mc_instance[id].res.res_id = res_id;
            g_i2s_mc_instance[id].res.ctrl_id = i2s_ctrl_id;
            g_i2s_mc_instance[id].res.phy_addr = phy_addr;
            g_i2s_mc_instance[id].res.irq_num = g_i2s_irq_table[id + 8].irq_num;
            g_i2s_mc_instance[id].res.clock = g_i2s_irq_table[id + 8].clock;
            mutex_release(&g_i2s_mc_mutex);
            return &g_i2s_mc_instance[id];
        }
    }

    mutex_release(&g_i2s_mc_mutex);
    return NULL;
}

//*****************************************************************************
//
//! hal_i2s mc_release_instance.
//!
//! \void.
//!
//! This function release i2s mc instance handle.
//!
//
//*****************************************************************************
static void hal_i2s_mc_release_instance(i2s_mc_instance_t *i2s_mc_instance)
{
    mutex_acquire(&g_i2s_mc_mutex);
    i2s_mc_instance->occupied = 0;
    mutex_release(&g_i2s_mc_mutex);
}

//*****************************************************************************
//
//! hal_i2s_mc_create_handle.
//!
//! \handle i2s mc handle for i2s mc func.
//!
//! This function get hal handle.
//!
//! \return i2s mc handle
//
//*****************************************************************************
bool hal_i2s_mc_create_handle(void **handle, u_int32_t res_id)
{
    i2s_mc_instance_t *i2s_mc_instance = hal_i2s_mc_get_instance(res_id);

    if (i2s_mc_instance == NULL) {
        return false;
    }

    *handle = i2s_mc_instance;

    return true;
}

//*****************************************************************************
//
//! hal_i2s_mc_release_handle.
//!
//! \handle i2s mc handle to release.
//!
//! This function delete i2s mc instance handle.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_i2s_mc_release_handle(void *handle)
{
    ASSERT(handle);
    hal_i2s_mc_release_instance(handle);

    return true;
}

//*****************************************************************************
//
//! hal_i2s_mc_init.
//!
//! \handle i2s mc handle for i2s mc func.
//!
//! This function is for i2s mc  used i2s mc resource parameters and platform init.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_i2s_mc_init(void *handle)
{
    ASSERT(handle);
    i2s_mc_instance_t *instance = handle;

    if (instance->inited) {
        dprintf(INFO, "hal_i2s_mc_init has inited\n");
        return false;
    }

    instance->cfg.bus = instance->res.ctrl_id - 1;
    instance->cfg.clock = instance->res.clock;
    instance->cfg.interrupt_num = instance->res.irq_num;
    instance->cfg.base_addr = (paddr_t)_ioaddr(instance->res.phy_addr);
    instance->inited = true;

    return instance->drv->reg(&instance->cfg);
}

//*****************************************************************************
//
//! hal_i2s_mc_deinit.
//!
//! \handle i2s mc handle for i2s func.
//!
//! This function is for i2s mc deinit.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_i2s_mc_deinit(void *handle)
{
    ASSERT(handle);
    i2s_mc_instance_t *instance = handle;

    if (instance->inited == false) {
        dprintf(INFO, "hal_i2s_mc_init has not inited\n");
        return false;
    }

    hal_i2s_mc_release_instance(instance);
    instance->inited = false;

    return true;
}

//*****************************************************************************
//
//! hal_i2s_mc_config.
//!
//! \handle i2s mc handle for i2s mc func.
//!
//! This function is for i2s mc parameters setting.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_i2s_mc_config(void *handle, i2s_mc_init_t *i2s_config)
{
    i2s_mc_instance_t *instance = handle;
    ASSERT(instance && instance->drv->config);

    return instance->drv->config(i2s_config, &instance->cfg);
}

//*****************************************************************************
//
//! hal_i2s_mc_start
//!
//! \handle i2s mc handle for i2s mc func.
//!
//! This function is for i2s mc  enbale.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_i2s_mc_start(void *handle)
{
    i2s_mc_instance_t *instance = handle;
    ASSERT(instance && instance->drv->start);

    return instance->drv->start(&instance->cfg);
}

//*****************************************************************************
//
//! hal_i2s_mc_stop.
//!
//! \handle i2s mc handle for i2s mc func.
//!
//! This function is for i2s mc disabale.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_i2s_mc_stop(void *handle)
{
    i2s_mc_instance_t *instance = handle;
    ASSERT(instance && instance->drv->stop);

    return instance->drv->stop(&instance->cfg);
}

//*****************************************************************************
//
//! hal_i2s_mc_transmit.
//!
//! \handle i2s mc handle for i2s mc  func.
//!=
//! This function is for i2s mc transmitting data using interrupt mode.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_i2s_mc_transmit(void *handle)
{
    i2s_mc_instance_t *instance = handle;
    ASSERT(instance && instance->drv->transmit);

    return instance->drv->transmit(&instance->cfg);
}

//*****************************************************************************
//
//! hal_i2s_mc_get_fifo_addr_4dma
//!
//! \param handle is a i2s mc handle
//!
//! This function get i2s mc handle fifo address, made it recognizable to dma.
//! \return fifo address.
//
//*****************************************************************************
paddr_t hal_i2s_mc_get_fifo_addr(void *handle)
{
    ASSERT(handle);
    i2s_mc_instance_t *instance = handle;

    return instance->res.phy_addr + I2S_MC_FIFO_OFFSET;
}

void hal_i2s_mc_show_config(void *handle)
{
    ASSERT(handle);
    i2s_mc_instance_t *instance = handle;

    instance->drv->reg_cur_setting(&instance->cfg);
    return;
}

void hal_i2s_hook_init(uint level)
{
    mutex_init(&g_i2s_mc_mutex);
    mutex_init(&g_i2s_sc_mutex);
}

LK_INIT_HOOK(i2s_mutex, hal_i2s_hook_init, LK_INIT_LEVEL_APPS);
