/*
 * tca9539.c
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

#include "TCA9539.h"

/*********** i2c wrapper foward declaration ***********/
bool TCA9539_i2c_init(uint32_t i2c_res_id, uint8_t address);
bool TCA9539_i2c_write_reg(uint32_t i2c_res_id, uint8_t reg, uint8_t data);
bool TCA9539_i2c_read_reg(uint32_t i2c_res_id, uint8_t reg, uint8_t *data);
bool TCA9539_i2c_deinit(uint32_t i2c_res_id);

void TCA9539_parse_cfg(const TCA9539_cfg_t *cfg, TCA9539_reg_val_t *val);
/*********** TCA9539 drvier implementation ***********/

/* use shadow register to avoid flip-flop control issue.*/
struct TCA9539_instance_t {
    uint32_t i2c_res_id;
    bool used;
    TCA9539_reg_val_t val;
} TCA9539_shadow[TCA9539_MAX_SLAVES] = {
    {0, false,}, {0, false,}, {0, false,}, {0, false,},
};

static inline void TCA9539_clr_shadow_register_value(TCA9539_reg_val_t *val)
{
    val->direction[0] = 0;
    val->direction[1] = 0;
    val->invertion[0] = 0;
    val->invertion[1] = 0;
    val->output[0] = 0;
    val->output[1] = 0;
    val->input[0] = 0;
    val->input[1] = 0;
}

static inline struct TCA9539_instance_t *TCA9539_install(uint32_t i2c_res_id)
{
    for (uint32_t i = 0; i < TCA9539_MAX_SLAVES; i++) {
        if (TCA9539_shadow[i].i2c_res_id == i2c_res_id) {
            return NULL;
        } else {
            if (!TCA9539_shadow[i].i2c_res_id) {
                TCA9539_shadow[i].i2c_res_id = i2c_res_id;

                if (!TCA9539_shadow[i].used) {
                    TCA9539_clr_shadow_register_value(&TCA9539_shadow[i].val);
                }

                return &TCA9539_shadow[i];
            }
        }
    }

    return NULL;
}

void *TCA9539_init(uint32_t i2c_res_id, uint8_t address, const TCA9539_cfg_t *cfg)
{
    bool ret = true;
    struct TCA9539_instance_t *shadow = TCA9539_install(i2c_res_id);

    if (!cfg) {
        TCA_TRACE("tca9539 invalid cfg para\n");
        return NULL;
    }

    if (!shadow) {
        TCA_TRACE("tca9539 invalid i2c res para.\n");
        return NULL;
    }

    if (!TCA9539_i2c_init(i2c_res_id, address)) {
        TCA_TRACE("tca9539 i2c init fail\n");
        return NULL;
    } else {
        if (!shadow->used) {
            TCA9539_parse_cfg(cfg, &shadow->val);

            ret &= TCA9539_i2c_write_reg(i2c_res_id, TCA9539_OUTPUT_REG0, shadow->val.output[0]);
            ret &= TCA9539_i2c_write_reg(i2c_res_id, TCA9539_OUTPUT_REG1, shadow->val.output[1]);

            ret &= TCA9539_i2c_write_reg(i2c_res_id, TCA9539_CONFIG_REG0, shadow->val.direction[0]);
            ret &= TCA9539_i2c_write_reg(i2c_res_id, TCA9539_CONFIG_REG1, shadow->val.direction[1]);

            ret &= TCA9539_i2c_write_reg(i2c_res_id, TCA9539_POLARITY_REG0, shadow->val.invertion[0]);
            ret &= TCA9539_i2c_write_reg(i2c_res_id, TCA9539_POLARITY_REG1, shadow->val.invertion[1]);

            shadow->used = true;
        }
    }

    if (true != ret) {
        TCA9539_deinit(shadow);
        return NULL;
    } else {
        return shadow;
    }
}

bool TCA9539_set_pin_dir(void *TCA9539, uint8_t group, uint8_t pin, uint8_t dir)
{
    uint8_t reg_addr = 0;
    uint8_t config;
    bool ret = true;
    struct TCA9539_instance_t *shadow = TCA9539;

    if (TCA9539_GROUP_0 == group) {
        reg_addr = TCA9539_CONFIG_REG0;
    } else if (TCA9539_GROUP_1 == group) {
        reg_addr = TCA9539_CONFIG_REG1;
    } else {
        TCA_TRACE("tca9539 invalid group para.\n");
        return false;
    }

    if (dir != TCA9539_CONFIG_OUTPUT && dir != TCA9539_CONFIG_INPUT) {
        TCA_TRACE("tca9539 invalid dir para.\n");
        return false;
    }

    if (!shadow) {
        TCA_TRACE("tca9539 invalid handle para.\n");
        return false;
    }

    config = shadow->val.direction[group];

    if (dir == TCA9539_CONFIG_OUTPUT) {
        config &= ~pin;
    } else {
        config |= pin;
    }

    ret &= TCA9539_i2c_write_reg(shadow->i2c_res_id, reg_addr, config);

    if (true != ret) {
        TCA_TRACE("tca9539 write config fail.\n");
        return false;
    }

    shadow->val.direction[group] = config;

    return ret;
}

bool TCA9539_write_pin_logic(void *TCA9539, uint8_t group, uint8_t pin, uint8_t logic)
{
    uint8_t reg_addr = 0;
    uint8_t output_reg = 0;
    bool ret = true;

    struct TCA9539_instance_t *shadow = TCA9539;

    if (TCA9539_GROUP_0 == group) {
        reg_addr = TCA9539_OUTPUT_REG0;
    } else if (TCA9539_GROUP_1 == group) {
        reg_addr = TCA9539_OUTPUT_REG1;
    } else {
        TCA_TRACE("tca9539 invalid group para.\n");
        return false;
    }

    if (logic != TCA9539_LOGIC_HIGH && logic != TCA9539_LOGIC_LOW) {
        TCA_TRACE("tca9539 invalid logic para.\n");
        return false;
    }

    if (!shadow) {
        TCA_TRACE("tca9539 invalid handle para.\n");
        return false;
    }

    output_reg = shadow->val.output[group];

    if (TCA9539_LOGIC_HIGH == logic) {
        output_reg |= pin;
    } else {
        output_reg &= ~pin;
    }

    ret &= TCA9539_i2c_write_reg(shadow->i2c_res_id, reg_addr, output_reg);

    if (true != ret) {
        TCA_TRACE("tca9539 write output fail.\n");
        return false;
    }

    shadow->val.output[group] = output_reg;

    return ret;
}

bool TCA9539_read_pin_logic(void *TCA9539, uint8_t group, uint8_t pin, uint8_t *state)
{
    uint8_t reg_addr = 0;
    uint8_t pin_logic = 0;
    bool ret = true;

    struct TCA9539_instance_t *shadow = TCA9539;

    if (TCA9539_GROUP_0 == group) {
        reg_addr = TCA9539_INPUT_REG0;
    } else if (TCA9539_GROUP_1 == group) {
        reg_addr = TCA9539_INPUT_REG1;
    } else {
        TCA_TRACE("tca9539 invalid group para.\n");
        return false;
    }

    if (!shadow) {
        TCA_TRACE("tca9539 invalid handle para.\n");
        return false;
    }

    ret &= TCA9539_i2c_read_reg(shadow->i2c_res_id, reg_addr, &pin_logic);

    if (true != ret) {
        TCA_TRACE("tca9539 read pin logic fail.\n");
        return false;
    }

    shadow->val.input[group] = pin_logic;

    *state = pin_logic & pin;

    return ret;
}

bool TCA9539_set_pin_input_invert(void *TCA9539, uint8_t group, uint8_t pin, uint8_t invert)
{
    uint8_t reg_addr = 0;
    uint8_t invert_reg = 0;
    bool ret = true;

    struct TCA9539_instance_t *shadow = TCA9539;

    if (TCA9539_GROUP_0 == group) {
        reg_addr = TCA9539_POLARITY_REG0;
    } else if (TCA9539_GROUP_1 == group) {
        reg_addr = TCA9539_POLARITY_REG1;
    } else {
        TCA_TRACE("tca9539 invalid group para.\n");
        return false;
    }

    if (invert != TCA9539_INPUT_INVERTION_ENABLE && invert != TCA9539_INPUT_INVERTION_DISABLE) {
        TCA_TRACE("tca9539 invalid invert para.\n");
        return false;
    }

    if (!shadow) {
        TCA_TRACE("tca9539 invalid handle para.\n");
        return false;
    }

    invert_reg = shadow->val.invertion[group];

    if (TCA9539_INPUT_INVERTION_ENABLE == invert) {
        invert_reg |= pin;
    } else {
        invert_reg &= ~pin;
    }

    ret &= TCA9539_i2c_write_reg(shadow->i2c_res_id, reg_addr, invert_reg);

    if (true != ret) {
        TCA_TRACE("tca9539 write invertion fail.\n");
        return false;
    }

    shadow->val.invertion[group] = invert_reg;

    return ret;
}

void TCA9539_deinit(void *TCA9539)
{
    struct TCA9539_instance_t *shadow = TCA9539;

    if (!shadow) {
        TCA_TRACE("tca9539 invalid handle para.\n");
        return;
    }

    TCA9539_i2c_deinit(shadow->i2c_res_id);
    shadow->i2c_res_id = 0;
}
