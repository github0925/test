//*****************************************************************************
//
// i2c_hal_ip_test.h - Prototypes for the i2c hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __I2C_HAL_IP_TEST_H__
#define __I2C_HAL_IP_TEST_H__
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
#include "__regs_base.h"
#if ENABLE_SD_I2C
#include "dw_i2c.h"
#endif

#include <kernel/mutex.h>
#include <platform/debug.h>
#include <platform/interrupts.h>
#include "chip_res.h"
#include "i2c_hal.h"

#define LOCAL_TRACE 1

#define SDV_I2C_DRIVER_VERSION (MAKE_VERSION(1, 0, 0)) /*!< Version 1.0.0 */
#define MAKE_VERSION(major, minor, bugfix) (((major) << 16) | ((minor) << 8) | (bugfix))

#define DEFAULT_I2C_MAX_NUM  16

/* Check the arguments. */
#define HAL_ASSERT_PARAMETER(handle)  \
if(handle == NULL){ \
    LTRACEF("paramenter error handle:%p\n",handle);    \
    return false;   \
}   \

/* i2c global index to i2c bus number in whole soc */
typedef struct _i2c_test_glb_idx_to_num {
    uint32_t res_glb_idx;
    uint32_t i2c_num;
} i2c_test_glb_idx_to_num;

#if ENABLE_SD_I2C
/* i2c driver interface structure */
typedef struct _i2c_test_drv_controller_interface {
    bool (*dump_all_reg_test)(i2c_reg_type_t *type);
    bool (*read_only_reg_test)(vaddr_t base);
    bool (*rw_reg_test)(vaddr_t base);
} i2c_test_drv_controller_interface_t;
#endif


/* i2c instance */
typedef struct _i2c_test_instance {
#if ENABLE_SD_I2C
    dw_i2c_config_t i2c_cfg;   /* i2c test config */
    const i2c_test_drv_controller_interface_t
    *controllerTable;  /* i2c driver interface */
#endif
    uint8_t occupied;   /* 0 - the instance is not occupied; 1 - the instance is occupied */

    addr_t cur_i2c_phy_addr;
    uint8_t cur_i2c_soc_busnum;
} i2c_test_instance_t;


bool hal_i2c_test_creat_handle(void **handle, uint32_t i2c_res_glb_idx);
bool hal_i2c_test_release_handle(void *handle);
bool hal_i2c_dump_all_reg_test(void *handle);
bool hal_i2c_read_only_reg_test(void *handle);
bool hal_i2c_rw_reg_test(void *handle);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __I2C_HAL_IP_TEST_H__

