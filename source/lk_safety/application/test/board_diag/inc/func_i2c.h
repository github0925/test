/*
 * func_ii4
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#ifndef __FUN_DIO_H_
#define __FUN_DIO_H_
#include "board_start.h"
#include "board_cfg.h"

#define I2C_CHANNEL_NUM 4

typedef enum {
    READ  = 0,
    WRITE = 1
}TCA9539_MODE;

typedef struct {
    TCA9539_MODE mode;
    uint8_t num;
}i2cx_dev_t;

typedef bool(*tca9539_opt)(board_test_exec_t *exec, i2cx_dev_t i2cx_dev);
typedef bool(*i2c_dev_type)(board_test_exec_t *exec, i2cx_dev_t i2cx_dev);

typedef struct{
    #define taca9535_pin_max 18
    uint8_t order;
    uint8_t pin_num;
    uint8_t channel;
    void *handle;
    i2c_dev_type dev_type;
}i2c_dev_desc_t;

typedef struct{  
    i2c_dev_desc_t *i2c_dev_desc[I2C_CHANNEL_NUM];
}i2c_chn_table_t;

extern i2c_chn_table_t i2c_chn_table;

extern bool i2c_channel_to_write(uint8_t chn, uint8_t pin, uint8_t val);
extern uint8_t i2c_channel_to_read(uint8_t chn, uint8_t pin);
extern void i2cx_gpio_set_system_power_mgr(uint8_t state);

extern uint16_t adc_result_form_pmic_dev(board_test_exec_t *exec);
extern bool match_dev_type_tca9539(board_test_exec_t *exec, i2cx_dev_t i2cx_dev);
extern bool match_dev_type_5949(board_test_exec_t *exec, i2cx_dev_t i2cx_dev);
extern bool match_dev_type_pf8200(board_test_exec_t *exec, i2cx_dev_t i2cx_dev);
#endif