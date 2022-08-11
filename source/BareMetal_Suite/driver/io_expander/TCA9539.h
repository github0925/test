/*
 * tca9539.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: TI tca9539-q1 i2c io expander driver.
 *
 * Revision History:
 * -----------------
 * 0.1, 2/9/2020 init version
 */
#ifndef __TCA9539_H
#define __TCA9539_H

#include <stdint.h>
#include <stdbool.h>
#include "types_def.h"

#if TCA_TRACE_ENABLE
#include <stdio.h>
#define TCA_TRACE(...) printf(__VA_ARGS__)
#else
#define TCA_TRACE(...) do{}while(0)
#endif

#define TCA9539_MAX_SLAVES     4

#define TCA9539_INPUT_REG0      ((uint8_t)0x00)     // Input status register 0
#define TCA9539_INPUT_REG1      ((uint8_t)0x01)     // Input status register 0

#define TCA9539_OUTPUT_REG0     ((uint8_t)0x02)     // Output register to change state of output BIT set to 1, output set HIGH
#define TCA9539_OUTPUT_REG1    ((uint8_t)0x03)      // Output register to change state of output BIT set to 1, output set HIGH

#define TCA9539_POLARITY_REG0   ((uint8_t)0x04)     // Polarity inversion register. BIT '1' inverts input polarity of register 0x00
#define TCA9539_POLARITY_REG1   ((uint8_t)0x05)     // Polarity inversion register. BIT '1' inverts input polarity of register 0x00

#define TCA9539_CONFIG_REG0     ((uint8_t)0x06)     // Configuration register. BIT = '1' sets port to input BIT = '0' sets port to output
#define TCA9539_CONFIG_REG1     ((uint8_t)0x07)     // Configuration register. BIT = '1' sets port to input BIT = '0' sets port to output

#define TCA9539_LOGIC_HIGH             (1)
#define TCA9539_LOGIC_LOW              (0)

#define TCA9539_CONFIG_OUTPUT           (0)
#define TCA9539_CONFIG_INPUT            (1)

#define TCA9539_INPUT_INVERTION_ENABLE  (1)
#define TCA9539_INPUT_INVERTION_DISABLE (0)

#define TCA9539_GROUP_0                 (0)
#define TCA9539_GROUP_1                 (1)

#define TCA9539_PIN(x)                  (((uint8_t)1) << (x))
#define TCA9539_PIN_ALL                 ((uint8_t)0xFF)

typedef struct TCA9539_pin_cfg_t {
    uint8_t dir;
    uint8_t invert;
    uint8_t output_logic;
} TCA9539_group_cfg_t[8];

typedef TCA9539_group_cfg_t TCA9539_cfg_t[2];

typedef volatile struct TCA9539_reg_val_t {
    uint8_t output[2];
    uint8_t input[2];
    uint8_t direction[2];
    uint8_t invertion[2];
} TCA9539_reg_val_t;

/*********** TCA9539 drvier declaration ***********/

extern const TCA9539_cfg_t TCA9539_DEFAULT_CFG_TABLE;

/* Initialize TCA9539 io expander.
 *
 * i2c_res_id: specify the connected i2c res id.
 *
 * address: TCA9539 slave address
 *
 * cfg: static configuration on initial. Choose the above
 * symbol to use or modify.
 *
 * CFG will only be used on power state. if init again after deinit,
 * the return handle may maintain the origin state in last inited state.
 *
 * ret: TCA9539 handle.
 */
void *TCA9539_init(uint32_t i2c_res_id, uint8_t address, const TCA9539_cfg_t *cfg);

/* write pin logic on TCA9539 io expander.
 *
 * group: should be macro of TCA9539_GROUP_0 or TCA9539_GROUP_1
 *
 * pin: use TCA9539_PIN(x) macro. x should be from 0 ~ 7. Or
 * TCA9539_PIN_ALL could be used for all pin. pins could be use OR to
 * gather. e.g. TCA9539_PIN(1) | TCA9539_PIN(2)
 *
 * logic: should be TCA9539_LOGIC_HIGH or TCA9539_LOGIC_LOW
 *
 * ret: true as manipulation succ, or false means failed.
 */
bool TCA9539_write_pin_logic(void *TCA9539, uint8_t group, uint8_t pin, uint8_t logic);

/* read pin logic on TCA9539 io expander.
 *
 * group: should be macro of TCA9539_GROUP_0 or TCA9539_GROUP_1
 *
 * pin: use TCA9539_PIN(x) macro. x should be from 0 ~ 7. Or
 * TCA9539_PIN_ALL could be used for all pin. pins could be use OR to
 * gather. e.g. TCA9539_PIN(1) | TCA9539_PIN(2)
 *
 * state: pin state of specified pin per bit.
 *
 * ret: true as manipulation succ, or false means failed.
 */
bool TCA9539_read_pin_logic(void *TCA9539, uint8_t group, uint8_t pin, uint8_t *state);

/* set pin direction on TCA9539 io expander.
 *
 * group: should be macro of TCA9539_GROUP_0 or TCA9539_GROUP_1
 *
 * pin: use TCA9539_PIN(x) macro. x should be from 0 ~ 7. Or
 * TCA9539_PIN_ALL could be used for all pin. pins could be use OR to
 * gather. e.g. TCA9539_PIN(1) | TCA9539_PIN(2)
 *
 * dir: should be TCA9539_CONFIG_OUTPUT or TCA9539_CONFIG_INPUT
 *
 * ret: true as manipulation succ, or false means failed.
 */
bool TCA9539_set_pin_dir(void *TCA9539, uint8_t group, uint8_t pin, uint8_t dir);

/* set pin invertion on TCA9539 io expander. This was only available
 * while the corresponding pin was set to input mode. The input logic
 * on pin will be inverted while in expression.
 *
 * group: should be macro of TCA9539_GROUP_0 or TCA9539_GROUP_1
 *
 * pin: use TCA9539_PIN(x) macro. x should be from 0 ~ 7. Or
 * TCA9539_PIN_ALL could be used for all pin. pins could be use `|` to
 * gather. e.g. TCA9539_PIN(1) | TCA9539_PIN(2)
 *
 * invert: should be TCA9539_INPUT_INVERTION_ENABLE or
 * TCA9539_INPUT_INVERTION_DISABLE.
 *
 * ret: true as manipulation succ, or false means failed.
 */
bool TCA9539_set_pin_input_invert(void *TCA9539, uint8_t group, uint8_t pin, uint8_t invert);

/* TCA9539_deinit
 *
 * handle: TCA9539 handle to be deinit.
 *
 * Pin state will not change and will be inherited once
 * init again. The POR will clear the inherited pin state.
 *
 * ret: true as manipulation succ, or false means failed.
 */
void TCA9539_deinit(void *TCA9539);

#endif
