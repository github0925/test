//*****************************************************************************
//
// i2c_hal.h - Prototypes for the i2c hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __I2C_HAL_H__
#define __I2C_HAL_H__
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
#include "dw_i2c.h"

#include <kernel/mutex.h>
#include <platform/debug.h>
#include <platform/interrupts.h>
#include "chip_res.h"
#include "res.h"

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
typedef struct _i2c_glb_idx_to_num {
    uint32_t res_glb_idx;
    uint32_t i2c_num;
    uint32_t irq;
} i2c_glb_idx_to_num;


#if ENABLE_SD_I2C
/* i2c driver interface structure */
typedef struct _i2c_drv_controller_interface {
    bool (*set_busconfig)(dw_i2c_context *p_i2c_con,
                          const dw_i2c_config_t *i2c_config);
    void (*init_after)(dw_i2c_context *p_i2c_con);
    status_t (*transmit)(dw_i2c_context *p_i2c_con, uint8_t address,
                         const void *buf, size_t cnt, bool start, bool stop);
    status_t (*receive)(dw_i2c_context *p_i2c_con, uint8_t address,
                        void *buf, size_t cnt, bool start, bool stop);
    status_t (*slave_receive)(dw_i2c_context *p_i2c_con, void *buf,
                              size_t cnt, bool start, bool stop);
    status_t (*write_reg_bytes)(dw_i2c_context *p_i2c_con, uint8_t address,
                                uint8_t reg, void *buf, size_t cnt);
    status_t (*read_reg_bytes)(dw_i2c_context *p_i2c_con, uint8_t address,
                               uint8_t reg, void *buf, size_t cnt);
    status_t (*scan)(dw_i2c_context *p_i2c_con, uint8_t address);
    status_t (*write_reg)(dw_i2c_context *p_i2c_con, uint8_t address,
                          void *reg, size_t cnt);
    status_t (*write_reg_data)(dw_i2c_context *p_i2c_con, uint8_t address,
                               void *reg, size_t cnt_reg, void *data, size_t cnt);
    status_t (*read_reg_data)(dw_i2c_context *p_i2c_con, uint8_t address,
                              void *reg, size_t cnt_reg, void *data, size_t cnt);
    status_t (*common_xfer)(dw_i2c_context *p_i2c_con, struct i2c_msg *msgs, int num);
} i2c_drv_controller_interface_t;
#endif

/* i2c config param for app. */
typedef enum {
    HAL_I2C_SPEED_STANDARD = 1,
    HAL_I2C_SPEED_FAST,
    HAL_I2C_SPEED_HIGH,
} hal_i2c_speed_e;

typedef enum {
    HAL_I2C_ADDR_7BITS,
    HAL_I2C_ADDR_10BITS
} hal_i2c_addr_bits_e;

typedef enum {
    HAL_I2C_SLAVE_MODE,
    HAL_I2C_MASTER_MODE,
} hal_i2c_mode_e;

/* Describes i2c configuration structure for app. */
typedef struct _i2c_app_config {
    hal_i2c_speed_e speed;
    hal_i2c_addr_bits_e addr_bits;
    hal_i2c_mode_e  mode;
    uint32_t slave_addr;
    uint32_t poll;
} i2c_app_config_t;


/* i2c instance */
typedef struct _i2c_instance {
#if ENABLE_SD_I2C
    dw_i2c_context *i2c_con;
    const i2c_drv_controller_interface_t
    *controllerTable;  /* i2c driver interface */
#endif
    uint8_t occupied;   /* 0 - the instance is not occupied; 1 - the instance is occupied */
    i2c_app_config_t i2c_cfg;
    uint8_t cur_i2c_res_index;
    uint8_t cur_i2c_soc_busnum;
} i2c_instance_t;


/**
  * create i2c bus handle
  * i2c_res_glb_idx: global resource id
*/
bool hal_i2c_creat_handle(void **handle, uint32_t i2c_res_glb_idx);

/**
  * release i2c bus handle
*/
bool hal_i2c_release_handle(void *handle);

/**
  * get i2c bus current config
*/
i2c_app_config_t hal_i2c_get_busconfig(void *handle);

/**
  * reset i2c bus config
*/
bool hal_i2c_set_busconfig(void *handle, i2c_app_config_t *cfg);

/**
  * i2c write to slave
  * address: slave device id
  * buf: write data or register value
  * start,stop: send flag bit
*/
bool hal_i2c_transmit(void *handle, uint8_t address, const void *buf,
                      size_t cnt, bool start, bool stop);
/**
  * i2c read from slave
  * address: slave device id
  * buf: read data
  * start,stop: send flag bit
*/
bool hal_i2c_receive(void *handle, uint8_t address, void *buf, size_t cnt,
                     bool start, bool stop);


bool hal_i2c_slave_transmit(void *handle, void *buf, size_t cnt);

bool hal_i2c_slave_receive(void *handle, void *buf, size_t cnt);

/**
  * i2c write to slave
  * address: slave device id
  * reg: slave register address
  * buf: write data
*/
bool hal_i2c_write_reg_bytes(void *handle, uint8_t address, uint8_t reg,
                             void *buf, size_t cnt);
/**
  * i2c read from slave
  * address: slave device id
  * reg: slave register address
  * buf: read data
*/
bool hal_i2c_read_reg_bytes(void *handle, uint8_t address, uint8_t reg,
                            void *buf, size_t cnt);

bool hal_i2c_scan(void *handle, uint8_t address);
bool hal_i2c_write_reg(void *handle, uint8_t address, void *reg,
                       size_t cnt);
status_t hal_i2c_write_reg_data(void *handle, uint8_t address, void *reg,
                                size_t reg_cnt, void *data, size_t cnt);
status_t hal_i2c_read_reg_data(void *handle, uint8_t address, void *reg,
                               size_t reg_cnt, void *data, size_t cnt);
status_t hal_i2c_common_xfer(void *handle, struct i2c_msg *msgs, int num);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif // __I2C_HAL_H__

