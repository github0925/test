//*****************************************************************************
//
// spdif_hal.c
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <kernel/mutex.h>
#include <sys/types.h>
#include <lk/init.h>
#include "irq.h"
#include <chip_res.h>
#include <spdif_hal.h>
#include <spdif.h>
#include <string.h>
#include <chip_res.h>
#include <lib/reg.h>
#include <res.h>
#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#define DEFAULT_SPDIF_MAX_NUM 4

typedef struct _spdif_drv {
    bool (*init)(spdif_top_cfg_t *);
    bool (*config)(spdif_top_cfg_t *);
    bool (*start)(spdif_top_cfg_t *);
    bool (*stop)(spdif_top_cfg_t *);
    bool (*int_transmit)(spdif_top_cfg_t *);
    bool (*sleep)(spdif_top_cfg_t *);
    void (*reg_cur_setting)(spdif_top_cfg_t *);
} spdif_drv_t;

typedef struct _spdif_instance {
    const spdif_drv_t *drv;
    spdif_top_cfg_t cfg;
    spdif_res_info_t res;
    uint8_t occupied;
    bool inited;
    bool enabled;
} spdif_instance_t;

static spdif_instance_t g_spdif_instance[DEFAULT_SPDIF_MAX_NUM];
static mutex_t spdif_mutex;

static const spdif_res_t g_spdif_irq_table[] = {
    {RES_SPDIF_SPDIF1, 1, SPDIF1_INTREQ_NUM},
    {RES_SPDIF_SPDIF2, 2, SPDIF2_INTREQ_NUM},
    {RES_SPDIF_SPDIF3, 3, SPDIF3_INTREQ_NUM},
    {RES_SPDIF_SPDIF4, 4, SPDIF4_INTREQ_NUM},
};

static const spdif_drv_t g_spdif_drv = {
    spdif_init,
    spdif_config,
    spdif_start,
    spdif_stop,
    spdif_int_transmit,
    spdif_sleep,
    spdif_show_current_cfg,
};

static void hal_spdif_get_controller_interface(const spdif_drv_t **drv)
{
    *drv = &g_spdif_drv;
}

static spdif_instance_t *hal_spdif_get_instance(u32 res_id)
{
    u32 ret = 0;
    int32_t spdif_ctrl_id = 0, id = 0;
    paddr_t phy_addr = 0;

    mutex_acquire(&spdif_mutex);

    // get res info.
    ret = res_get_info_by_id(res_id, &phy_addr, &spdif_ctrl_id);
    id = spdif_ctrl_id - 1;

    if ((ret == 0) && (g_spdif_instance[id].occupied != 1)) {
        memset(&g_spdif_instance[id], 0, sizeof(spdif_instance_t));

        hal_spdif_get_controller_interface(&g_spdif_instance[id].drv);

        if (g_spdif_instance[id].drv) {
            g_spdif_instance[id].occupied = 1;
            g_spdif_instance[id].res.res_id = res_id;
            g_spdif_instance[id].res.ctrl_id = spdif_ctrl_id;
            g_spdif_instance[id].res.phy_addr = phy_addr;
            g_spdif_instance[id].res.interrupt_num =
                g_spdif_irq_table[id].interrupt_num;
            mutex_release(&spdif_mutex);
            return &g_spdif_instance[id];
        }
    }

    mutex_release(&spdif_mutex);
    return NULL;
}

static void hal_spdif_release_instance(spdif_instance_t *spdif_instance)
{
    mutex_acquire(&spdif_mutex);
    spdif_instance->occupied = 0;
    mutex_release(&spdif_mutex);
}

bool hal_spdif_create_handle(void **handle, u32 res_id)
{
    spdif_instance_t *instance = hal_spdif_get_instance(res_id);

    if (instance == NULL) {
        return false;
    }

    *handle = instance;
    return true;
}

bool hal_spdif_release_handle(void *handle)
{
    ASSERT(handle);
    hal_spdif_release_instance(handle);

    return true;
}

bool hal_spdif_init(void *handle)
{
    ASSERT(handle);
    spdif_instance_t *instance = handle;

    if (instance->inited) {
        dprintf(INFO, "hal_spdif_init has inited.\n");
        return false;
    }

    instance->cfg.bus = instance->res.ctrl_id - 1;
    instance->cfg.interrupt_num = instance->res.interrupt_num;
    instance->cfg.base_addr = (addr_t)_ioaddr(instance->res.phy_addr);
    instance->inited = true;

    return instance->drv->init(&instance->cfg);
}

bool hal_spdif_deinit(void *handle)
{
    spdif_instance_t *instance = handle;

    ASSERT(handle);

    if (instance->inited == false) {
        dprintf(INFO, "hal_spdif_init has not inited.\n");
        return false;
    }

    instance->inited = false;

    return true;
}

bool hal_spdif_config(void *handle, spdif_cfg_info_t *spdif_config)
{
    spdif_instance_t *instance = handle;
    ASSERT(instance && instance->drv->config);
    memcpy(&instance->cfg.cfg_info, spdif_config, sizeof(spdif_cfg_info_t));
    return instance->drv->config(&instance->cfg);
}

bool hal_spdif_start(void *handle)
{
    spdif_instance_t *instance = handle;
    ASSERT(instance && instance->drv->start);

    return instance->drv->start(&instance->cfg);
}

bool hal_spdif_stop(void *handle)
{
    spdif_instance_t *instance = handle;
    ASSERT(instance && instance->drv->stop);

    return instance->drv->stop(&instance->cfg);
}

bool hal_spdif_int_transmit(void *handle)
{
    spdif_instance_t *instance = handle;
    ASSERT(instance && instance->drv->int_transmit);

    return instance->drv->int_transmit(&instance->cfg);
}

u32 hal_spdif_wait_tx_comp_intmode(void *handle, int timeout)
{
    ASSERT(handle);
    spdif_instance_t *instance = handle;
    event_wait_timeout(&(instance->cfg.tx_comp), timeout);

    return instance->cfg.cfg_info.tx_count;
}

u32 hal_spdif_wait_rx_comp_intmode(void *handle, int timeout)
{
    ASSERT(handle);
    spdif_instance_t *instance = handle;
    event_wait_timeout(&(instance->cfg.rx_comp), timeout);

    return instance->cfg.cfg_info.rx_count;
}

bool hal_spdif_sleep(void *handle)
{
    ASSERT(handle);
    spdif_instance_t *instance = handle;

    return instance->drv->sleep(&instance->cfg);
}

addr_t hal_spdif_get_fifo_addr(void *handle)
{
    ASSERT(handle);
    spdif_instance_t *instance = handle;

    return instance->res.phy_addr + SPDIF_FIFO_ADDR_OFFSET;
}

u32 hal_spdif_get_fifo_depth(void *handle)
{
    ASSERT(handle);

    return SPDIF_FIFO_DEPTH;
}

void hal_spdif_show_reg_config(void *handle)
{
    ASSERT(handle);
    spdif_instance_t *instance = handle;

    return instance->drv->reg_cur_setting(&instance->cfg);
}

void hal_spdif_mutex_init(uint level)
{
    mutex_init(&spdif_mutex);
    mutex_init(&spdif_mutex);
}

LK_INIT_HOOK(spdif__mutex, hal_spdif_mutex_init, LK_INIT_LEVEL_APPS);
