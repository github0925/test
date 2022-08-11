//*****************************************************************************
//
// csi_hal.h - Prototypes for the csi hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __CSI_HAL_H__
#define __CSI_HAL_H__
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

#include <kernel/mutex.h>
#include <platform/debug.h>
#include <platform/interrupts.h>

#include "res.h"
#include "chip_res.h"
#include "__regs_base.h"
#include "sd_csi.h"
#include "irq.h"

#if !EMULATION_PLATFORM_FPGA
#include "mipi_csi_hal.h"
#endif


#define LOCAL_TRACE 1



#define SDV_CSI_DRIVER_VERSION (MAKE_VERSION(1, 0, 0)) /*!< Version 1.0.0 */
#define MAKE_VERSION(major, minor, bugfix) (((major) << 16) | ((minor) << 8) | (bugfix))


// Check the arguments.
#define CSI_HAL_ASSERT_PARAMETER(handle)  \
if(handle == NULL){ \
    LTRACEF("paramenter error handle:%p\n", handle);    \
}   \

typedef void (*stream_func_t)(uint32_t img_id, addr_t rgby_paddr);
//typedef void (*cb_sync_func_t)(uint32_t img_id, uint32_t index, addr_t rgby_paddr);
struct data_callback_info {
    uint32_t ip;
    uint32_t img_id;
    uint32_t index;
    addr_t rgby_paddr;
};
typedef void (*cb_sync_func_t)(struct data_callback_info *cbi);


/*csi instance */
typedef struct _csi_instance {
    uint8_t occupied;   /* 0 - the instance is not occupied; 1 - the instance is occupied */
    struct csi_device *dev;
    int host_id;
    bool initialized;
    uint8_t *display_fb;
    stream_func_t cb;
    cb_sync_func_t cbs;
    thread_t *t[IMG_COUNT];

#if !EMULATION_PLATFORM_FPGA
    void *mipi_inst;
#endif
} csi_instance_t;


/**
  * create csi bus handle
  * csi_res_glb_idx: global resource id
*/
bool hal_csi_creat_handle(void **handle, uint32_t csi_res_glb_idx);

/**
  * release csi bus handle
*/
bool hal_csi_release_handle(void *handle);


/**
  * bind csi handle with v4l2_device
*/
bool hal_csi_set_vdev(void *handle, struct v4l2_device *vdev);

/**
  * get v4l2_device from csi bus handle
*/
struct v4l2_device *hal_csi_get_vdev(void *handle);

/**
  * config csi bus interface
*/
bool hal_csi_cfg_interface(void *handle,
                           struct v4l2_fwnode_endpoint endpoint);


/**
  * csi bus init
*/
status_t hal_csi_init(void *handle);

/**
  * init csi memory
*/
bool hal_csi_init_mem(void *handle);

/**
  * csi bus start
*/
bool hal_csi_start(void *handle, uint32_t mask);

/**
  * csi bus stop
*/
bool hal_csi_stop(void *handle, uint32_t mask);

/**
  * set app display information
*/
status_t hal_csi_set_display_info(void *handle, stream_func_t cb);
status_t hal_csi_set_callback_sync(void *handle, uint32_t channel, cb_sync_func_t cb);



//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif

