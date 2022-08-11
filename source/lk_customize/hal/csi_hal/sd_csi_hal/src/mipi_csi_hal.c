//*****************************************************************************
//
// mipi_csi_hal.c - Driver for the mipi csi hal Module.
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

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#include "mipi_csi_hal.h"
#include "sd_mipi_csi2.h"
#include "res.h"
#include "chip_res.h"


#define MIPICSI_LOG 0

#if 0
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
#endif

//static domain_res_t g_mipi_csi_res_def = mipi_csi_res_def;
static mutex_t mipicsi_mutex;


#define MIPI_CSI_RES_NUM 2
/* csi global instance */
static mipi_csi_instance_t g_mipicsiInstance[MIPI_CSI_RES_NUM] = {0};


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
static mipi_csi_instance_t *hal_mipi_csi_get_instance(
    uint32_t mipi_csi_res_glb_idx)
{
    uint8_t i;
    mipi_csi_device *dev;
    int32_t cur_mipi_csi_res_index;
    addr_t cur_mipi_csi_phy_addr;

#if 0

    for (i = 0; i < g_mipi_csi_res_def.res_num; i++) {
        if (g_mipi_csi_res_def.res_info[i].res_glb_idx == mipi_csi_res_glb_idx) {
            cur_mipi_csi_res_index = i;
        }
    }

#else
    res_get_info_by_id(mipi_csi_res_glb_idx, &cur_mipi_csi_phy_addr,
                       &cur_mipi_csi_res_index);
#endif
    dprintf(MIPICSI_LOG, "%s(): find mipi_csi index: %d - phy_addr=0x%lx.\n",
            __func__, cur_mipi_csi_res_index, cur_mipi_csi_phy_addr);

    mutex_acquire(&mipicsi_mutex);
    i = cur_mipi_csi_res_index - 1;

    if (g_mipicsiInstance[i].occupied != 1) {
        uint8_t *buffer = (uint8_t *)&g_mipicsiInstance[i];
        memset(buffer, 0, sizeof(mipi_csi_instance_t));

        dev = sd_mipi_csi2_host_init(cur_mipi_csi_res_index - 1,
                                     cur_mipi_csi_phy_addr);

        if (dev == NULL) {
            printf("get mipi csi %d error\n", (cur_mipi_csi_res_index - 1));
            return NULL;
        }

        g_mipicsiInstance[i].occupied = 1;
        g_mipicsiInstance[i].dev = dev;
    }

    mutex_release(&mipicsi_mutex);
    return &g_mipicsiInstance[i];
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
static void hal_mipi_csi_release_instance(mipi_csi_instance_t *instance)
{
    mutex_acquire(&mipicsi_mutex);
    instance->occupied = 0;
    mutex_release(&mipicsi_mutex);
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
bool hal_mipi_csi_creat_handle(void **handle,
                               uint32_t mipi_csi_res_glb_idx)
{
    dprintf(MIPICSI_LOG, "%s(): res_id=0x%x\n", __func__,
            mipi_csi_res_glb_idx);

    mipi_csi_instance_t  *instance = NULL;
    mutex_init(&mipicsi_mutex);

    instance = hal_mipi_csi_get_instance(mipi_csi_res_glb_idx);

    if (instance == NULL) {
        return false;
    }

    *handle = instance;

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
bool hal_mipi_csi_release_handle(void *handle)
{
    dprintf(MIPICSI_LOG, "%s().\n", __func__);

    mipi_csi_instance_t *instance = NULL;
    HAL_MIPICSI_ASSERT_PARAMETER(handle);

    mutex_acquire(&mipicsi_mutex);
    instance = (mipi_csi_instance_t *)handle;
    instance->occupied = 0;
    mutex_release(&mipicsi_mutex);
    return true;
}



//*****************************************************************************
//
//! hal_mipi_csi_init.
//!
//! \handle mipi csi handle for mipi csi func.
//!
//! This function is for mipi csi bus init.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_mipi_csi_init(void *handle)
{
    dprintf(MIPICSI_LOG, "%s()\n", __func__);
    mipi_csi_instance_t *instance = NULL;
    HAL_MIPICSI_ASSERT_PARAMETER(handle);
    instance = (mipi_csi_instance_t *)handle;

    //stance->dev->ops.start(instance->dev, start);

    sd_mipi_csi2_init_cfg(instance->dev);
    dprintf(MIPICSI_LOG, "%s() end\n", __func__);

    return true;
}


//*****************************************************************************
//
//! hal_mipi_csi_set_hline_time.
//!
//! \handle mipi csi handle for mipi csi func.
//!
//! This function is for setting horizental parameters: hsa, hbp, hsd.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_mipi_csi_set_hline_time(void *handle, uint32_t hsa, uint32_t hbp,
                                 uint32_t hsd)
{
    dprintf(MIPICSI_LOG, "%s()\n", __func__);
    mipi_csi_instance_t *instance = NULL;
    HAL_MIPICSI_ASSERT_PARAMETER(handle);
    instance = (mipi_csi_instance_t *)handle;

    sd_mipi_csi2_set_hline_time(instance->dev, hsa, hbp, hsd);
    return true;
}


//*****************************************************************************
//
//! hal_mipi_csi_set_phy_freq.
//!
//! \handle mipi csi handle for mipi csi func.
//!
//! This function is for setting mipi csi phy freq range.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_mipi_csi_set_phy_freq(void *handle, uint32_t freq, uint32_t lanes)
{
    dprintf(MIPICSI_LOG, "%s()\n", __func__);
    mipi_csi_instance_t *instance = NULL;
    HAL_MIPICSI_ASSERT_PARAMETER(handle);
    instance = (mipi_csi_instance_t *)handle;

    sd_mipi_csi2_set_phy_freq(instance->dev, freq, lanes);
    return true;
}


//*****************************************************************************
//
//! hal_mipi_csi_start.
//!
//! \handle mipi csi handle for mipi csi func.
//!
//! This function is for starting mipi host and phy.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_mipi_csi_start(void *handle)
{
    dprintf(MIPICSI_LOG, "%s()\n", __func__);
    mipi_csi_instance_t *instance = NULL;
    HAL_MIPICSI_ASSERT_PARAMETER(handle);
    instance = (mipi_csi_instance_t *)handle;
    dprintf(MIPICSI_LOG, "instance->occupied=%d\n", instance->occupied);
    sd_mipi_csi2_start(instance->dev);
    return true;
}

//*****************************************************************************
//
//! hal_mipi_csi_stop.
//!
//! \handle mipi csi handle for mipi csi func.
//!
//! This function is for stopping mipi host and phy.
//!
//! \return bool status
//
//*****************************************************************************
bool hal_mipi_csi_stop(void *handle)
{
    dprintf(MIPICSI_LOG, "%s()\n", __func__);
    mipi_csi_instance_t *instance = NULL;
    HAL_MIPICSI_ASSERT_PARAMETER(handle);
    instance = (mipi_csi_instance_t *)handle;
    dprintf(MIPICSI_LOG, "instance->occupied=%d\n", instance->occupied);
    sd_mipi_csi2_stop(instance->dev);
    return true;
}
