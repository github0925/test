/*
 * pmic_pf8200_dev.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pmic_pf8200_dev.c
 *
 * Revision History:
 * -----------------
 */
#include <dw_adc.h>
#include "board_start.h"
#include "board_init.h"
#include "i2c_hal.h"
#include "func_i2c.h"
#include "board_cfg.h"
#include "func_can.h"

#define ADC_REF_VOLTAGE     (1.8)
#define ADC_RESOLUTION      (12)

/* PF8200 I2C slave address. */
#define PF8200_ADDR         (0x8)

/* PF8200 registers. */
#define DEVICE_ID            0x0
#define REV_ID               0x1
#define EMREV                0x2
#define AMUX                 0x4A

#define AMUX_EN_SHIFT        5
#define AMUX_SEL_SHIFT       0

/**
 * @brief Voltage channel.
 *
 * AP domain powers sourced from separate DCDC/LDO units are sampled
 * by unique ADC channels. Safety domain powers are provided by PF8200
 * PMIC. These powers share one ADC channel, and we use PF8200 AMUX
 * to select the power to sample.
 */
struct voltage {
    const uint8_t num;

    const char  *name;

    /* SOC ADC channel. */
    int         adc_chnl;

    /* PMIC AMUX_SEL to select output voltage. See datasheet
     * table 74. Set as 0 for non PMIC voltages.
     */
    uint8_t     amux_sel;

    /* PMIC internal divide ratio, required if amux_sel is not 0. See
     * datasheet table 74.
     */
    float       internal_ratio;

    /* Voltage divide ratio, determined by on-board divider resistors. */
    float       ratio;
};

extern bool adc_single_polling_eoc(addr_t base, uint32_t ms_timeout);
extern void adc_reset_ctrl(addr_t base_addr);
extern void adc_analog_setup(addr_t base_addr);

static volatile bool g_enable_vol_monitor;

static struct voltage board_diag_voltages[] = {
    {
        .num = 0,
        .name = "+1V0_SW1",
        .adc_chnl = 6,
        .amux_sel = 0x4,
        .internal_ratio = 1,
        .ratio = 0.5,
    },
    {
        .num = 1,
        .name = "+0V9_SW2",
        .adc_chnl = 6,
        .amux_sel = 0x5,
        .internal_ratio = 1,
        .ratio = 0.5,
    },
    {
        .num = 2,
        .name = "+1V05_SW3",
        .adc_chnl = 6,
        .amux_sel = 0x6,
        .internal_ratio = 1,
        .ratio = 0.5,
    },
    {
        .num = 3,
        .name = "+1V8_SW4",
        .adc_chnl = 6,
        .amux_sel = 0x7,
        .internal_ratio = 1.25,
        .ratio = 0.5,
    },
    {
        .num = 4,
        .name = "+1V1_SW5",
        .adc_chnl = 6,
        .amux_sel = 0x8,
        .internal_ratio = 1,
        .ratio = 0.5,
    },
    {
        .num = 5,
        .name = "+0V8_SW6",
        .adc_chnl = 6,
        .amux_sel = 0x9,
        .internal_ratio = 1,
        .ratio = 0.5,
    },
    {
        .num = 6,
        .name = "+3V3_SW7",
        .adc_chnl = 6,
        .amux_sel = 0xa,
        .internal_ratio = 2.86,
        .ratio = 0.5,
    },
    {
        .num = 7,
        .name = "+1V8_LDO1",
        .adc_chnl = 6,
        .amux_sel = 0xb,
        .internal_ratio = 3.33,
        .ratio = 0.5,
    },
    {
        .num = 8,
        .name = "+1V5_LDO2",
        .adc_chnl = 6,
        .amux_sel = 0xc,
        .internal_ratio = 3.33,
        .ratio = 0.5,
    },
    {
        .num = 9,
        .name = "+1V8_LDO3",
        .adc_chnl = 6,
        .amux_sel = 0xd,
        .internal_ratio = 3.33,
        .ratio = 0.5,
    },
    {
        .num = 10,
        .name = "+1V8_LDO4",
        .adc_chnl = 6,
        .amux_sel = 0xe,
        .internal_ratio = 3.33,
        .ratio = 0.5,
    },
};

static uint32_t adc_sample(addr_t base, uint8_t channel)
{
    uint32_t result;

    adc_set_convert_mode(base, ADC_SINGLE_CH_SINGLE_E0);
    adc_select_single_ch(base, channel);
    adc_start_convert(base);

    if (!adc_single_polling_eoc(base, 10)) {
        printf("adc_single_polling_eoc timeout\n");
        return 0;
    }

    result = get_convert_result_reg(base);
    adc_clear_all_int_flag(base);

    return result;
}

static uint8_t pf8200_read_reg(void *handle, uint8_t reg)
{
    uint8_t val;

    if (!hal_i2c_read_reg_data(handle, PF8200_ADDR, &reg, 1, &val, 1)) {
        return 0;
    }

    return val;
}

static void pf8200_write_reg(void *handle, uint8_t reg, uint8_t val)
{
    if (!hal_i2c_write_reg_data(handle, PF8200_ADDR, &reg, 1, &val, 1)) {
        return;
    }
}

static uint16_t read_voltage(struct voltage *vol)
{
    adc_channel_init();

    return (uint16_t)adc_sample(APB_ADC_BASE, vol->adc_chnl);
}

static uint16_t read_pmic_voltage(void *handle, struct voltage *vol)
{
    /* Setup PMIC AMUX */
    pf8200_write_reg(handle, AMUX,
                     (1 << AMUX_EN_SHIFT) | (vol->amux_sel << AMUX_SEL_SHIFT));
    return read_voltage(vol);
}

static bool monitor_voltage(board_test_exec_t *exec, void *handle)
{
    bool ret = false;
    can_cmd_t *pf8200_cmd = (can_cmd_t *)exec->cmd;

    for (uint8_t num = 0; num < ARRAY_SIZE(board_diag_voltages); num++) {

        if (pf8200_cmd->recv_data != board_diag_voltages[num].num) {
            continue;
        }
        else {
            struct voltage *vol = &board_diag_voltages[num];

            uint32_t value = read_pmic_voltage(handle,
                                               vol) * vol->internal_ratio / vol->ratio;

            float _voltage = value * ADC_REF_VOLTAGE / 4095;
            set_para_value(exec->resp[2], (((uint16)(_voltage * 100)) >> 8) & 0xff);
            set_para_value(exec->resp[3], ((uint16)(_voltage * 100)) & 0xff);
            set_para_value(ret, true);
        }
    }

    return ret;
}

static bool pmic_pf8200_dev_opt(board_test_exec_t *exec, i2cx_dev_t i2cx_dev)
{
    bool ret = false;
    uint8_t dev_id;
    void *handle = i2c_chn_table.i2c_dev_desc[i2cx_dev.num]->handle;

    dev_id = pf8200_read_reg(handle, DEVICE_ID) & 0x0f;

    if (dev_id != PF8200_ADDR)
        return ret;

    adc_channel_init();

    ret = monitor_voltage(exec, handle);

    return ret;
}

bool match_dev_type_pf8200(board_test_exec_t *exec, i2cx_dev_t i2cx_dev)
{
    bool ret = false;

    ret = pmic_pf8200_dev_opt(exec, i2cx_dev);

    return ret;
}

uint16_t adc_result_form_pmic_dev(board_test_exec_t *exec)
{
    i2cx_dev_t i2cx_dev;

    set_para_value(i2cx_dev.mode, READ);
    set_para_value(i2cx_dev.num, 0x1);

    match_dev_type_pf8200(exec, i2cx_dev);

    return ((exec->resp[2] << 8) | exec->resp[3]);
}
