
//*****************************************************************************
//
// csi_hal.c - Driver for the csi hal Module.
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
#include <kernel/mutex.h>
#include <stdlib.h>

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#include "csi_hal.h"
#include "res.h"
#include "chip_res.h"

#if EMULATION_PLATFORM_FPGA
#define FPGA_CSI0_INT (76)
#endif

#define LOG_LEVEL 4
#define CSI_RES_NUM 3


struct csi_table {
    uint32_t csi_idx;
    uint32_t res_glb_idx;
};

struct csi_table ct[5] = {
    {1, RES_MIPI_CSI_MIPI_CSI1},
    {2, RES_MIPI_CSI_MIPI_CSI2},
    {1, RES_CSI_CSI1},
    {2, RES_CSI_CSI2},
    {3, RES_CSI_CSI3}
};

//static domain_res_t g_csi_res_def = csi_res_def;
static mutex_t csi_mutex;

/* csi global instance */
static csi_instance_t g_csiInstance[CSI_RES_NUM] = {0};

#if !EMULATION_PLATFORM_FPGA
static void *mipi_csi_handle;
#endif

//*****************************************************************************
//
//! hal_csi_get_instance.
//!
//! \void.
//!
//! This function get csi instance hand.
//!
//! \return csi hanle
//
//*****************************************************************************
static csi_instance_t *hal_csi_get_instance(uint32_t csi_res_glb_idx)
{
    uint8_t i;
    struct csi_device *dev;
    int32_t cur_csi_res_index = 0;
    addr_t cur_csi_phy_addr = 0;
    uint32_t cur_csi_irq = 0;
    int32_t ret;

    ret = res_get_info_by_id(csi_res_glb_idx, &cur_csi_phy_addr,
                             &cur_csi_res_index);

    if (ret == -1) {
        dprintf(0, "%s: get resource 0x%x fail, ret=%d\n", __func__,
                csi_res_glb_idx, ret);
        return NULL;
    }

    dprintf(0, "%s(): find csi phy_addr=0x%lx, index=%d.\n", __func__,
            cur_csi_phy_addr, cur_csi_res_index);

#if EMULATION_PLATFORM_FPGA
    cur_csi_irq = FPGA_CSI0_INT;

    if (csi_res_glb_idx == RES_CSI_CSI1) {
        //cur_csi_phy_addr = FPGA_CSI0_BASE;
        //cur_csi_res_index = 1;
        cur_csi_irq = FPGA_CSI0_INT;
    }
    else if (csi_res_glb_idx == RES_CSI_CSI2) {
        //cur_csi_phy_addr = FPGA_CSI0_BASE+0x2000;
        //cur_csi_res_index = 2;
        cur_csi_irq = FPGA_CSI0_INT + 1;
    }

#else

    if (csi_res_glb_idx == RES_CSI_CSI1) {
        //cur_csi_res_index = 1;
        cur_csi_irq = CSI1_INTERRUPT_NUM;
    }
    else if (csi_res_glb_idx == RES_CSI_CSI2) {
        //cur_csi_res_index = 2;
        cur_csi_irq = CSI2_INTERRUPT_NUM;
    }
    else if (csi_res_glb_idx == RES_CSI_CSI3) {
        //cur_csi_res_index = 3;
        cur_csi_irq = CSI3_INTERRUPT_NUM;
    }

#endif

    dprintf(LOG_LEVEL, "irq=%d\n", cur_csi_irq);

    mutex_acquire(&csi_mutex);
    dprintf(LOG_LEVEL, "CSI_RES_NUM=%d\n", CSI_RES_NUM);

    i = cur_csi_res_index - 1;

    if (i < 0 || i > CSI_RES_NUM)
        dprintf(0, "err csi index: %d\n", i);

    dprintf(0, "g_csiInstance[%d]: initialized=%d, occupied=%d\n", i,
            g_csiInstance[i].initialized, g_csiInstance[i].occupied);

    if (!g_csiInstance[i].initialized) {
        uint8_t *buffer = (uint8_t *)&g_csiInstance[i];
        memset(buffer, 0, sizeof(csi_instance_t));

        /* get csi driver API table */
        //l_csi_get_controller_interface(&(g_csiInstance[i].controllerTable));
        dev = csi_host_init(cur_csi_res_index - 1, cur_csi_phy_addr, cur_csi_irq);

        g_csiInstance[i].occupied = 1;
        dev->priv = &g_csiInstance[i];
        g_csiInstance[i].initialized = true;
        g_csiInstance[i].host_id = cur_csi_res_index - 1;
        g_csiInstance[i].dev = dev;

#if !EMULATION_PLATFORM_FPGA

        if (g_csiInstance[i].host_id == 0) {
            hal_mipi_csi_creat_handle(&mipi_csi_handle, RES_MIPI_CSI_MIPI_CSI1);
        }
        else if (g_csiInstance[i].host_id == 1) {
            hal_mipi_csi_creat_handle(&mipi_csi_handle, RES_MIPI_CSI_MIPI_CSI2);
        }

        g_csiInstance[i].mipi_inst = mipi_csi_handle;
#endif

    } else {
        if(g_csiInstance[i].occupied){
            mutex_release(&csi_mutex);
            return NULL;
        }
    }

    mutex_release(&csi_mutex);
    return &g_csiInstance[i];
}

//*****************************************************************************
//
//! hal_csi_release_instance.
//!
//! \void.
//!
//! This function release csi instance hand.
//!
//! \return
//
//*****************************************************************************
static void hal_csi_release_instance(csi_instance_t *instance)
{
    mutex_acquire(&csi_mutex);
    instance->occupied = 0;
    mutex_release(&csi_mutex);
}

//*****************************************************************************
//
//! hal_csi_creat_handle.
//!
//! \handle csi handle for csi func.
//!
//! This function get hal handle.
//!
//! \return csi handle
//
//*****************************************************************************
bool hal_csi_creat_handle(void **handle, uint32_t csi_res_glb_idx)
{
    static int first=1;
    dprintf(LOG_LEVEL, "%s(): csi_res_glb_idx=0x%x\n", __func__,
            csi_res_glb_idx);

    csi_instance_t  *csiInstance = NULL;
    if(first) {
        mutex_init(&csi_mutex);
        first=0;
    }
    csiInstance = hal_csi_get_instance(csi_res_glb_idx);
    if (csiInstance == NULL) {
        printf("%s(): create fail\n", __func__);
        *handle = NULL;
        return false;
    }

    *handle = csiInstance;
    return true;
}

//*****************************************************************************
//
//! hal_csi_release_handle.
//!
//! \void.
//!
//! This function delete csi instance hand.
//!
//! \return
//
//*****************************************************************************
bool hal_csi_release_handle(void *handle)
{
    dprintf(LOG_LEVEL, "hal_csi_release_handle().\n");

    csi_instance_t *instance = NULL;
    CSI_HAL_ASSERT_PARAMETER(handle);

    mutex_acquire(&csi_mutex);
    instance = (csi_instance_t *)handle;
    instance->occupied = 0;
    mutex_release(&csi_mutex);
    return true;
}



//*****************************************************************************
//
//! hal_csi_add_bus.
//!
//! \handle csi handle for csi func.
//!
//! This function is for csi used csi_app_cfg parameter init.
//!
//! \return bool status
//
//*****************************************************************************

bool hal_csi_set_vdev(void *handle, struct v4l2_device *vdev)
{
    csi_instance_t *instance = NULL;
    dprintf(LOG_LEVEL, "hal_csi_set_vdev()\n");

    CSI_HAL_ASSERT_PARAMETER(handle);
    instance = (csi_instance_t *)handle;

    if (!instance->dev)
        return false;

    instance->dev->vdev = vdev;
    return true;
}

struct v4l2_device *hal_csi_get_vdev(void *handle)
{
    csi_instance_t *instance = NULL;
    dprintf(LOG_LEVEL, "hal_csi_get_vdev()\n");

    CSI_HAL_ASSERT_PARAMETER(handle);
    instance = (csi_instance_t *)handle;

    if (!instance->dev)
        return NULL;

    return instance->dev->vdev;
}

bool hal_csi_cfg_interface(void *handle,
                           struct v4l2_fwnode_endpoint endpoint)
{
    csi_instance_t *instance = NULL;
    dprintf(LOG_LEVEL, "hal_csi_cfg_interface(%d)\n", endpoint.bus_type);

    CSI_HAL_ASSERT_PARAMETER(handle);
    instance = (csi_instance_t *)handle;

    if (!instance->dev)
        return false;

    if (!instance->dev->ops.cfg_interface)
        return false;

    instance->dev->ops.cfg_interface(instance->dev, endpoint);
    printf("hal_csi_cfg_interface() end.\n");
    return true;
}

status_t hal_csi_init(void *handle)
{
    csi_instance_t *instance = NULL;
    int ret = 0;
    dprintf(LOG_LEVEL, "hal_csi_init()\n");

    CSI_HAL_ASSERT_PARAMETER(handle);
    instance = (csi_instance_t *)handle;

    if (!instance->dev)
        return -1;

    if (!instance->dev->ops.init)
        return -1;

    ret = instance->dev->ops.init(instance->dev);
    dprintf(LOG_LEVEL, "hal_csi_init() end.\n");

    if (ret < 0) return ret;


#if !EMULATION_PLATFORM_FPGA

    if (instance->host_id == 0 || instance->host_id == 1) {
        hal_mipi_csi_init(instance->mipi_inst);

        if (instance->dev->vdev)
            hal_mipi_csi_set_hline_time(instance->mipi_inst,
                                        instance->dev->vdev->ex_info.hsa, instance->dev->vdev->ex_info.hbp,
                                        instance->dev->vdev->ex_info.hsd);
    }

#endif

    return ret;
}

bool hal_csi_init_mem(void *handle)
{
    csi_instance_t *instance = NULL;
    dprintf(LOG_LEVEL, "hal_csi_init_mem()\n");

    CSI_HAL_ASSERT_PARAMETER(handle);
    instance = (csi_instance_t *)handle;

    int i, j;
    struct csi_image *img;
    addr_t baddr;
    static int mem_align = 0;
    size_t mem_size;

    mem_align &= 0x0F;

    for (i = 0; i < IMG_COUNT; i++) {
        img = instance->dev->ops.get_image(instance->dev, i);

        if (!img || !img->enable)
            continue;

        img->id = i;
        printf("i=%d, img->rgby_stride=%d, img->height=%d\n", i, img->rgby_stride,
               img->height);

        if (img->rgby_stride && img->height) {
            mem_size = img->rgby_stride * img->height + 0x10;

            for (j = 0; j < CAMERA_MAX_BUF; j++) {
                baddr = (addr_t)malloc(mem_size) + mem_align;
                memset((void *)baddr, 0xAA, mem_size);
                arch_clean_invalidate_cache_range(baddr, mem_size);
#if WITH_KERNEL_VM
                img->rgby_baddr[j] = vaddr_to_paddr((void *)baddr);
#else
                img->rgby_baddr[j] = baddr;
#endif
                dprintf(LOG_LEVEL, "CSI RGBY ADDR %d: 0x%lx,stride:%u\n",
                        j, img->rgby_baddr[j], img->rgby_stride);
            }
        }

        printf("img->u_stride=%d, img->v_stride=%d\n", img->u_stride,
               img->v_stride);

        if (img->u_stride && img->height) {
            mem_size = img->u_stride * img->height + 0x10;

            for (j = 0; j < CAMERA_MAX_BUF; j++) {
                baddr = (addr_t)malloc(mem_size) + mem_align;
                memset((void *)baddr, 0xBB, mem_size);
                arch_clean_invalidate_cache_range(baddr, mem_size);
#if WITH_KERNEL_VM
                img->u_baddr[j] = vaddr_to_paddr((void *)baddr);
#else
                img->u_baddr[j] = baddr;
#endif
                dprintf(LOG_LEVEL, "CSI U ADDR %d : 0x%lx, stride:%u\n",
                        j, img->u_baddr[j], img->u_stride);
            }
        }

        if (img->v_stride && img->height) {
            mem_size = img->v_stride * img->height + 0x10;

            for (j = 0; j < CAMERA_MAX_BUF; j++) {
                baddr = (addr_t)malloc(mem_size) + mem_align;
                memset((void *)baddr, 0xCC, mem_size);
                arch_clean_invalidate_cache_range(baddr, mem_size);
#if WITH_KERNEL_VM
                img->v_baddr[j] = vaddr_to_paddr((void *)baddr);
#else
                img->v_baddr[j] = baddr;
#endif
                dprintf(LOG_LEVEL, "CSI V ADDR %d: 0x%lx, stride:%u\n",
                        j, img->v_baddr[j], img->v_stride);
            }
        }

        instance->dev->ops.set_image(instance->dev, img);
        instance->dev->ops.cfg_mem(instance->dev, 1 << i);
    }

    // mem_align++;

    return true;
}


bool hal_csi_start(void *handle, uint32_t mask)
{
    csi_instance_t *instance = NULL;
    dprintf(LOG_LEVEL, "hal_csi_start()\n");

    CSI_HAL_ASSERT_PARAMETER(handle);
    instance = (csi_instance_t *)handle;

    if (!instance->dev)
        return false;

    if (!instance->dev->ops.start)
        return false;

    instance->dev->ops.start(instance->dev, mask);

#if !EMULATION_PLATFORM_FPGA

    if (instance->host_id == 0 || instance->host_id == 1) {
        if (instance->dev->vdev)
            hal_mipi_csi_set_phy_freq(instance->mipi_inst,
                                      instance->dev->vdev->ex_info.pclk, instance->dev->vdev->ex_info.lanes);

        hal_mipi_csi_start(instance->mipi_inst);
    }

#endif
    dprintf(LOG_LEVEL, "hal_csi_start() end.\n");
    return true;
}


static int csi_hal_fb_stream_thread(void *arg)
{
    struct csi_image *img = (struct csi_image *)arg;
    struct csi_device *dev = img->csi;
    int pos, n;
    csi_instance_t *instance = NULL;
    struct data_callback_info cbi;
    dprintf(LOG_LEVEL, "%s(): img->id=%d\n", __func__, img->id);

    pos = img->buf_pos;
    instance = (csi_instance_t *)dev->priv;

    while (dev->streaming) {
        //event_wait_timeout(&img->completion, 10000);
        event_wait(&img->completion);

        dprintf(LOG_LEVEL, "while: pos=%d, img->buf_pos=%d\n", pos, img->buf_pos);

        if (dev->using_queue) {
            n = CAMERA_BUF_POS(img->buf_pos);
            cbi.ip = dev->id;
            cbi.img_id = img->id;
            cbi.index = n;
            cbi.rgby_paddr = img->rgby_baddr[n];
            if(instance->cbs)
                instance->cbs(&cbi);
        }
        else {
            //printf("pos=%d, img->buf_pos=%d\n", pos, img->buf_pos);
            if (pos != img->buf_pos) {
                n = CAMERA_BUF_POS(img->buf_pos);
                if(instance->cb)
                    instance->cb(img->id, img->rgby_baddr[n]);
                pos = img->buf_pos;
            }
        }
    }

    printf("thread(): exit\n");
    return 0;
}

status_t hal_csi_set_display_info(void *handle, stream_func_t cb)
{
    csi_instance_t *instance = NULL;
    int i;
    struct csi_image *img;
    dprintf(LOG_LEVEL, "hal_csi_set_display_info()\n");

    CSI_HAL_ASSERT_PARAMETER(handle);
    instance = (csi_instance_t *)handle;

    if (!instance->dev)
        return 0;

    for (i = 0; i < IMG_COUNT; i++) {
        img = instance->dev->ops.get_image(instance->dev, i);

        if (!img || !img->enable)
            continue;

        instance->t[i] = thread_create("csitest stream thread",
                                       &csi_hal_fb_stream_thread, (void *)img, HIGH_PRIORITY,
                                       DEFAULT_STACK_SIZE);
        thread_resume(instance->t[i]);
    }

    instance->cb = cb;

    return 0;
}

status_t hal_csi_set_callback_sync(void *handle, uint32_t channel,
                                   cb_sync_func_t cb)
{
    csi_instance_t *instance = NULL;
    int i;
    struct csi_image *img;

    CSI_HAL_ASSERT_PARAMETER(handle);
    instance = (csi_instance_t *)handle;

    if (!instance->dev)
        return 0;

    i = channel;

    if (instance->t[i])
        return 0;

    img = instance->dev->ops.get_image(instance->dev, i);

    if (!img || !img->enable)
        return 0;

    instance->t[i] = thread_create("csitest stream thread",
                                   &csi_hal_fb_stream_thread, (void *)img, HIGH_PRIORITY,
                                   DEFAULT_STACK_SIZE);
    thread_resume(instance->t[i]);


    if (!instance->cbs)
        instance->cbs = cb;

    return 0;
}

bool hal_csi_stop(void *handle, uint32_t mask)
{
    csi_instance_t *instance = NULL;

    dprintf(LOG_LEVEL, "hal_csi_stop()\n");

    CSI_HAL_ASSERT_PARAMETER(handle);
    instance = (csi_instance_t *)handle;

    if (!instance->dev)
        return false;

    if (!instance->dev->ops.stop)
        return false;

    instance->dev->ops.stop(instance->dev, mask);
    printf("hal_csi_stop() end. 0x%lx\n", (addr_t)instance->display_fb);

#if !EMULATION_PLATFORM_FPGA

    if (instance->host_id == 0 || instance->host_id == 1) {
        if (instance->dev->mask_en == 0)
            hal_mipi_csi_stop(instance->mipi_inst);
    }

#endif

    return true;
}
