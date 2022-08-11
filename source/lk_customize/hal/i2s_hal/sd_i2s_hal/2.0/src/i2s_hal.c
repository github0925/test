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
#endif
#include "dma_hal.h"

#define DEFAULT_I2S_SC_MAX_NUM 8

typedef struct _i2s_sc_drv {
    bool (*start_up)(struct dev_controller_info dev, pcm_params_t pcm_info);
    bool (*set_format)(struct dev_controller_info dev, pcm_params_t pcm_info);
    bool (*set_hw_parameter)(struct dev_controller_info dev,
                             pcm_params_t pcm_info);
    bool (*trigger)(struct dev_controller_info dev, int cmd);
    bool (*shutdown)(struct dev_controller_info dev);
    void (*reg_print)(struct dev_controller_info *dev);
} i2s_sc_drv_t;

/* i2s_sc instance  */
typedef struct _i2s_sc_instance {
    const i2s_sc_drv_t *drv;    /* !< i2s_sc driver interface */
    pcm_params_t pcm;
    struct dev_controller_info dev;
    uint8_t occupied;    /* !< 0 - the instance is not occupied; 1 - the instance is occupied */
    bool inited;
    bool enabled;
} i2s_sc_instance_t;

static i2s_sc_instance_t g_i2s_sc_instance[DEFAULT_I2S_SC_MAX_NUM];
static mutex_t g_i2s_sc_mutex;

static const struct dev_controller_info g_i2s_irq_table[] = {
    { RES_I2S_SC_I2S_SC1, 1, I2S_SC1_INTERRUPT_NUM },
    { RES_I2S_SC_I2S_SC2, 2, I2S_SC2_INTERRUPT_NUM },
    { RES_I2S_SC_I2S_SC3, 3, I2S_SC3_INTERRUPT_NUM },
    { RES_I2S_SC_I2S_SC4, 4, I2S_SC4_INTERRUPT_NUM },
    { RES_I2S_SC_I2S_SC5, 5, I2S_SC5_INTERRUPT_NUM },
    { RES_I2S_SC_I2S_SC6, 6, I2S_SC6_INTERRUPT_NUM },
    { RES_I2S_SC_I2S_SC7, 7, I2S_SC7_INTERRUPT_NUM },
    { RES_I2S_SC_I2S_SC8, 8, I2S_SC8_INTERRUPT_NUM },
};

/* i2s sc driver interface */
static const i2s_sc_drv_t g_i2s_sc_drv = {
    sdrv_i2s_sc_startup,
    sdrv_i2s_sc_set_format,
    sdrv_i2s_sc_set_hw_parameters,
    sdrv_i2s_sc_trigger,
    sdrv_i2s_sc_shutdown,
    sdrv_i2s_sc_reg_cur_setting,
};

static int width_map_i2s2dma(int i2s_width)
{
    switch (i2s_width) {
        case SD_AUDIO_SAMPLE_WIDTH_8BITS:
            return DMA_DEV_BUSWIDTH_1_BYTE;

        case SD_AUDIO_SAMPLE_WIDTH_16BITS:
            return DMA_DEV_BUSWIDTH_2_BYTES;

        case SD_AUDIO_SAMPLE_WIDTH_24BITS:
            return DMA_DEV_BUSWIDTH_4_BYTES;

        case SD_AUDIO_SAMPLE_WIDTH_32BITS:
            return DMA_DEV_BUSWIDTH_4_BYTES;

        default:
            dprintf(INFO, "unknown i2s sample resolution\n");
            return 0;
    }
}

static void hal_i2s_sc_get_controller_interface(const i2s_sc_drv_t **drv)
{
    *drv = &g_i2s_sc_drv;
}

static i2s_sc_instance_t *hal_sd_i2s_sc_get_instance(u_int32_t res_id)
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
            g_i2s_sc_instance[id].dev.bus = id;
            g_i2s_sc_instance[id].dev.addr = (paddr_t)_ioaddr(phy_addr);
            g_i2s_sc_instance[id].dev.irq_num = g_i2s_irq_table[id].irq_num;
            mutex_release(&g_i2s_sc_mutex);
            return &g_i2s_sc_instance[id];
        }
    }

    mutex_release(&g_i2s_sc_mutex);
    return NULL;
}

bool hal_sd_i2s_sc_create_handle(void **handle, u_int32_t res_id)
{
    i2s_sc_instance_t *i2s_sc_Instance = hal_sd_i2s_sc_get_instance(res_id);

    if (i2s_sc_Instance == NULL) {
        return false;
    }

    *handle = i2s_sc_Instance;
    return true;
}

static void hal_sd_i2s_sc_release_instance(i2s_sc_instance_t
        *i2s_sc_Instance)
{
    mutex_acquire(&g_i2s_sc_mutex);
    i2s_sc_Instance->occupied = 0;

    event_destroy(&(i2s_sc_Instance->pcm.xctrl.tx_comp));
    event_destroy(&(i2s_sc_Instance->pcm.xctrl.rx_comp));

    mutex_release(&g_i2s_sc_mutex);
}

bool hal_sd_i2s_sc_release_handle(void *handle)
{
    ASSERT(handle);
    hal_sd_i2s_sc_release_instance(handle);

    return true;
}

bool hal_sd_i2s_sc_start_up(void *handle, pcm_params_t pcm_info)
{
    ASSERT(handle);
    i2s_sc_instance_t *instance = handle;
    instance->pcm = pcm_info;
    struct dev_controller_info dev = instance->dev;
    u32 dir_mode = pcm_info.mode & SD_AUDIO_DIR_MODE_ENABLE;
    int id = dev.bus;

    /* set scr  */
    if ((dir_mode == SD_AUDIO_DIR_MODE_FULL_DUPLEX)
            || (dir_mode == SD_AUDIO_DIR_MODE_MULTI_CHAN_FULL_DUPLEX)) {
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

    event_init(&(instance->pcm.xctrl.tx_comp), false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&(instance->pcm.xctrl.rx_comp), false, EVENT_FLAG_AUTOUNSIGNAL);

    return instance->drv->start_up(dev, pcm_info);
}

bool hal_sd_i2s_sc_set_format(void *handle)
{
    i2s_sc_instance_t *instance = handle;
    ASSERT(instance && instance->drv);
    pcm_params_t pcm_info = instance->pcm;
    struct dev_controller_info dev = instance->dev;

    return instance->drv->set_format(dev, pcm_info);
}

bool hal_sd_i2s_sc_set_hw_params(void *handle)
{
    i2s_sc_instance_t *instance = handle;
    ASSERT(instance && instance->drv);
    pcm_params_t pcm_info = instance->pcm;
    struct dev_controller_info dev = instance->dev;

    return instance->drv->set_hw_parameter(dev, pcm_info);
}

bool hal_sd_i2s_sc_trigger(void *handle, int cmd)
{
    i2s_sc_instance_t *instance = handle;
    ASSERT(instance && instance->drv);
    struct dev_controller_info dev = instance->dev;

    return instance->drv->trigger(dev, cmd);
}

bool hal_sd_i2s_sc_shutdown(void *handle)
{
    i2s_sc_instance_t *instance = handle;
    ASSERT(instance && instance->drv);
    struct dev_controller_info dev = instance->dev;

    instance->drv->trigger(dev, SD_AUDIO_PCM_TRIGGER_PLAYBACK_STOP);
    instance->drv->trigger(dev, SD_AUDIO_PCM_TRIGGER_CAPTURE_STOP);
    return instance->drv->shutdown(dev);
}

paddr_t hal_sd_i2s_sc_get_fifo_addr(void *handle)
{
    ASSERT(handle);
    i2s_sc_instance_t *instance = handle;

    return _paddr((void *)(instance->dev.addr + I2S_SC_FIFO_OFFSET));
}

void hal_sd_i2s_sc_show_config(void *handle)
{
    i2s_sc_instance_t *instance = handle;
    ASSERT(handle && instance->drv && instance->drv->reg_print);

    instance->drv->reg_print(&instance->dev);
    return;
}

int hal_sd_i2s_sc_wait_tx_comp_intmode(void *handle, int timeout)
{
    ASSERT(handle);
    i2s_sc_instance_t *instance = handle;
    event_wait_timeout(&(instance->pcm.xctrl.tx_comp), timeout);

    return 0;
}

int hal_sd_i2s_sc_wait_rx_comp_intmode(void *handle, int timeout)
{
    ASSERT(handle);
    i2s_sc_instance_t *instance = handle;
    event_wait_timeout(&(instance->pcm.xctrl.rx_comp), timeout);

    return 0;
}

struct dma_desc *hal_i2s_trigger_dma_tr_start(void *i2s_handle,
        uint32_t dma_cap, void *obj_addr, uint32_t len,
        dmac_irq_evt_handle dma_irq_handle, uint32_t cmd)
{
    ASSERT(i2s_handle);
    i2s_sc_instance_t *instance = i2s_handle;
    pcm_params_t pcm = instance->pcm;
    struct dma_dev_cfg dma_cfg;
    struct dma_desc *dma_desc;
    if(len == 0)
        return NULL;
    dma_cfg.direction =
            (cmd == SD_AUDIO_PCM_TRIGGER_PLAYBACK_START)? DMA_MEM2DEV : DMA_DEV2MEM;
    dma_cfg.src_addr = hal_sd_i2s_sc_get_fifo_addr(i2s_handle);
    dma_cfg.dst_addr = hal_sd_i2s_sc_get_fifo_addr(i2s_handle);
    dma_cfg.src_maxburst = DMA_BURST_TR_32ITEMS;
    dma_cfg.dst_maxburst = DMA_BURST_TR_32ITEMS;
    dma_cfg.src_addr_width = width_map_i2s2dma(pcm.resolution);
    dma_cfg.dst_addr_width = width_map_i2s2dma(pcm.resolution);
    struct dma_chan *chan = hal_dma_chan_req(dma_cap);
    hal_dma_dev_config(chan, &dma_cfg);

    if (cmd == SD_AUDIO_PCM_TRIGGER_PLAYBACK_START)
        dma_desc = hal_prep_dma_dev(chan, (void *)obj_addr, len, DMA_INTERRUPT);
    else if (cmd == SD_AUDIO_PCM_TRIGGER_CAPTURE_START)
        dma_desc = hal_prep_dma_cyclic(chan, (void *)obj_addr, len,
                                       len / 2,
                                       DMA_INTERRUPT);
    else
        return NULL;

    dma_desc->dmac_irq_evt_handle = dma_irq_handle;
    dma_desc->context = (void *)0xa5a5;

    hal_sd_i2s_sc_trigger(i2s_handle, cmd);
    hal_dma_submit(dma_desc);
    return dma_desc;
}

void hal_i2s_hook_init(uint level)
{
    mutex_init(&g_i2s_sc_mutex);
}

LK_INIT_HOOK(i2s_sc_mutex, hal_i2s_hook_init, LK_INIT_LEVEL_APPS);
