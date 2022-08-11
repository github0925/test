/*
* ds90ub9xx.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 05/21/2020 BI create this file
*/
#ifndef __DS90UB9XX_H__
#define __DS90UB9XX_H__

#include <disp_common.h>
enum ds90ub9xx_gpio {
    GPIO0 = 0,
    GPIO1,
    GPIO2,
    GPIO3,
    D_GPIO0 = 4,
    D_GPIO1,
    D_GPIO2,
    D_GPIO3,
    GPIO5_REG,
    GPIO6_REG,
    GPIO7_REG,
    GPIO8_REG,
    GPIO9_REG,
    DS_GPIO_NUM,
};

#define UB948_GPIO5_REG_SHIFT 0
#define UB948_GPIO6_REG_SHIFT 4
#define UB948_GPIO7_REG_SHIFT 0
#define UB948_GPIO8_REG_SHIFT 4
#define UB948_GPIO9_REG_SHIFT 0

#define UB948_GPIO5_REG_MASK 0x0f
#define UB948_GPIO6_REG_MASK 0xf0
#define UB948_GPIO7_REG_MASK 0x0f
#define UB948_GPIO8_REG_MASK 0xf0
#define UB948_GPIO9_REG_MASK 0x0f

#define UB948_GPIO5_STATUS_SHIFT 5
#define UB948_GPIO6_STATUS_SHIFT 6
#define UB948_GPIO7_STATUS_SHIFT 7
#define UB948_GPIO8_STATUS_SHIFT 0
#define UB948_GPIO9_STATUS_SHIFT 1

#define UB948_GPIO5_STATUS_MASK (1 << UB948_GPIO5_STATUS_SHIFT)
#define UB948_GPIO6_STATUS_MASK (1 << UB948_GPIO6_STATUS_SHIFT)
#define UB948_GPIO7_STATUS_MASK (1 << UB948_GPIO7_STATUS_SHIFT)
#define UB948_GPIO8_STATUS_MASK (1 << UB948_GPIO8_STATUS_SHIFT)
#define UB948_GPIO9_STATUS_MASK (1 << UB948_GPIO9_STATUS_SHIFT)

enum ds_gpio_direct {
    INPUT,
    OUTPUT,
};

enum ds_gpio_level {
    HIGHT,
    LOW,
};

enum des_port_slect {
    DES_PORT0,
    DES_PORT1,
};

enum ser_port_slect {
    SER_PORT0,
    SER_PORT1,
};

enum ser_type_slect {
    SER_941,
    SER_947,
};

struct ds_gpio_config {
    enum ds90ub9xx_gpio gpio;
    uint8_t ser_reg;
    uint8_t des_reg;
    uint8_t value_mask;
    uint8_t ser_val;
    uint8_t des_val;
};

/* 941 reg config */
#define UB941_TX_STS_SHIFT      6
#define UB941_TX_STS_MASK       (1 << UB941_TX_STS_SHIFT)
#define UB941_FPD3_PORT_SHIFT   4
#define UB941_FPD3_PORT_MASK    (3 << UB941_FPD3_PORT_SHIFT)

enum port_link_status {
    DUAL_LINK = 0,
    SINGLE_LINK_PORT0,
    SINGLE_LINK_PORT1,
    BOTH_PORTS,
};
/*****************/

void ds90ub941as_init(void* i2c_handle, uint32_t slave_addr);
void ds90ub948_init(void* i2c_handle, uint32_t slave_addr);
void ds90ub941as_Enable_Dualdsi_DualPort(void* i2c_handle, uint32_t slave_addr);
bool ub941_get_serdes_link(void *i2c_handle, uint32_t ub941_addr);

void ds90ub947_init(void* i2c_handle, uint32_t slave_addr);
void ds90ub948_match_947_init(void* i2c_handle, uint32_t slave_addr);
void ds90ub947_bl_init(void);

void ds90ub927_init(void* i2c_handle, uint32_t slave_addr);
void ds90ub928_match_927_init(void* i2c_handle, uint32_t slave_addr);

int ds90ub9xx_read_reg(void* i2c_handle, uint32_t slave_addr, uint8_t reg, uint8_t *val);
int ds90ub9xx_write_reg(void* i2c_handle, uint32_t slave_addr, uint8_t reg, uint8_t val);
int ds90ub9xx_write_reg_witch_mask(void* i2c_handle, uint32_t slave_addr, uint8_t reg,
    uint8_t val, uint8_t mask);
int ds90ub94x_948_gpio_config(void* i2c_handle, uint32_t ser_addr, uint32_t des_addr,
    enum ds90ub9xx_gpio gpio, enum ds_gpio_direct direct, enum des_port_slect des_port);
int ds90ub94x_948_slaveI2CID_config(void* i2c_handle, uint32_t ser_addr, uint8_t slave_ID,
    enum ser_type_slect ser_type, enum ser_port_slect ser_port, uint8_t index);
int ds90ub948_gpio_output(void* i2c_handle, uint32_t des_addr, enum ds90ub9xx_gpio gpio,
    enum ds_gpio_level level);
int ds90ub94x_read_gpio_reg(void* i2c_handle, uint32_t ser_addr, enum ds90ub9xx_gpio gpio);
#endif //__DS90UB9XX_H__
