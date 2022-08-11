//*****************************************************************************
//
// mipi_csi_hal.h - Prototypes for the mipi csi hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __MIPI_CSI_HAL_H__
#define __MIPI_CSI_HAL_H__
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
#include "chip_res.h"
#include "__regs_base.h"


#include "sd_csi.h"
#include "sd_mipi_csi2.h"

#define LOCAL_TRACE 1


// Check the arguments.
#define HAL_MIPICSI_ASSERT_PARAMETER(handle)  \
if(handle == NULL){ \
    LTRACEF("paramenter error handle:%p\n",handle);    \
}   \


/*csi instance */
typedef struct _mipi_csi_instance {
    uint8_t occupied;   /* 0 - the instance is not occupied; 1 - the instance is occupied */
    mipi_csi_device *dev;
} mipi_csi_instance_t;



/**
  * create mipi csi bus handle
  * mipi_csi_res_glb_idx: global resource id
*/
bool hal_mipi_csi_creat_handle(void **handle,
                               uint32_t mipi_csi_res_glb_idx);

/**
  * release mipi csi bus handle
*/
bool hal_mipi_csi_release_handle(void *handle);


/**
  * mipi csi bus init
*/
bool hal_mipi_csi_init(void *handle);


/**
  * mipi csi bus set horizontal hsa, hbp, hsd time
*/
bool hal_mipi_csi_set_hline_time(void *handle, uint32_t hsa, uint32_t hbp,
                                 uint32_t hsd);

/**
  * mipi csi bus set phy freqrange
*/
bool hal_mipi_csi_set_phy_freq(void *handle, uint32_t freq,
                               uint32_t lanes);

/**
  * mipi csi host and phy enable
*/
bool hal_mipi_csi_start(void *handle);

/**
  * mipi csi host and phy disable
*/
bool hal_mipi_csi_stop(void *handle);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif

