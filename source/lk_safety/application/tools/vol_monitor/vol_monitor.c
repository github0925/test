/*
 * vol_monitor.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 voltage monitor.
 *
 * Revision History:
 * -----------------
 */
#include <app.h>
#include <kernel/event.h>
#include <lib/console.h>

#include "dw_adc.h"
#include "i2c_hal.h"

#define ADC_REF_VOLTAGE     (1.8)
#define ADC_RESOLUTION      (12)

/* PF8200 I2C slave address. */
#define PF8200_ADDR         (0x8)

/* PF8200 registers. */
#define DEVICE_ID   0x0
#define REV_ID      0x1
#define EMREV       0x2
#define AMUX        0x4A

#define AMUX_EN_SHIFT   5
#define AMUX_SEL_SHIFT  0

/**
 * @brief Voltage channel.
 *
 * AP domain powers sourced from separate DCDC/LDO units are sampled
 * by unique ADC channels. Safety domain powers are provided by PF8200
 * PMIC. These powers share one ADC channel, and we use PF8200 AMUX
 * to select the power to sample.
 */
struct voltage {
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

static event_t g_vol_monitor_evt;
static volatile bool g_enable_vol_monitor;

static struct voltage g_ii4_voltages[] = {
    {
        .name = "+2V5_ETH",
        .adc_chnl = 1,
        .ratio = 0.5,
    },
    {
        .name = "+3V3_DCDC",
        .adc_chnl = 2,
        .ratio = 0.3115,
    },
    {
        .name = "+0V8_A_CORE",
        .adc_chnl = 4,
        .ratio = 1,
    },
    {
        .name = "+5V",
        .adc_chnl = 5,
        .ratio = 0.3115,
    },
    {
        .name = "+1V0_SW1",
        .adc_chnl = 6,
        .amux_sel = 0x4,
        .internal_ratio = 1,
        .ratio = 0.5,
    },
    {
        .name = "+0V9_SW2",
        .adc_chnl = 6,
        .amux_sel = 0x5,
        .internal_ratio = 1,
        .ratio = 0.5,
    },
    {
        .name = "+1V05_SW3",
        .adc_chnl = 6,
        .amux_sel = 0x6,
        .internal_ratio = 1,
        .ratio = 0.5,
    },
    {
        .name = "+1V8_SW4",
        .adc_chnl = 6,
        .amux_sel = 0x7,
        .internal_ratio = 1.25,
        .ratio = 0.5,
    },
    {
        .name = "+1V1_SW5",
        .adc_chnl = 6,
        .amux_sel = 0x8,
        .internal_ratio = 1,
        .ratio = 0.5,
    },
    {
        .name = "+0V8_SW6",
        .adc_chnl = 6,
        .amux_sel = 0x9,
        .internal_ratio = 1,
        .ratio = 0.5,
    },
    {
        .name = "+3V3_SW7",
        .adc_chnl = 6,
        .amux_sel = 0xa,
        .internal_ratio = 2.86,
        .ratio = 0.5,
    },
    {
        .name = "+1V8_LDO1",
        .adc_chnl = 6,
        .amux_sel = 0xb,
        .internal_ratio = 3.33,
        .ratio = 0.5,
    },
    {
        .name = "+1V5_LDO2",
        .adc_chnl = 6,
        .amux_sel = 0xc,
        .internal_ratio = 3.33,
        .ratio = 0.5,
    },
    {
        .name = "+1V8_LDO3",
        .adc_chnl = 6,
        .amux_sel = 0xd,
        .internal_ratio = 3.33,
        .ratio = 0.5,
    },
    {
        .name = "+1V8_LDO4",
        .adc_chnl = 6,
        .amux_sel = 0xe,
        .internal_ratio = 3.33,
        .ratio = 0.5,
    },
};

static int vol_monitor_on(int argc, const cmd_args *argv)
{
    if (argc != 2) {
        dprintf(CRITICAL, "input argv error\n");
        return -1;
    }

    g_enable_vol_monitor = !!argv[1].u;
    event_signal(&g_vol_monitor_evt, false);

    return 0;
}

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

static void adc_initialize(addr_t base, bool extref, bool differential,
                           adc_resolution_e_t resolution, adc_clk_source_e_t clksrc,
                           uint8_t clkdiv)
{
    adc_reset_ctrl(base);
    adc_set_SELREF_bit(base, !extref);

    if (!extref) {
        adc_set_SELBG_bit(base, true);
    }

    adc_set_delta_mode_bit(base, differential);
    adc_set_resolution(base, resolution);
    adc_set_clk_src(base, clksrc);
    adc_set_clk_divider_bits(base, clkdiv - 1);
    adc_set_DWC_CLK_en_bit(base, true);
    adc_set_ADC_en_bit(base, true);
    adc_analog_setup(base);
    adc_init_convert(base);
}

static uint8_t pf8200_read_reg(void *i2c_handle, uint8_t reg)
{
    uint8_t val;

    if (hal_i2c_read_reg_data(i2c_handle, PF8200_ADDR, &reg, 1, &val, 1)) {
        printf("%s error\n", __func__);
        return 0;
    }

    return val;
}

static void pf8200_write_reg(void *i2c_handle, uint8_t reg, uint8_t val)
{
    if (hal_i2c_write_reg_data(i2c_handle, PF8200_ADDR, &reg, 1, &val, 1)) {
        printf("%s error\n", __func__);
    }
}

static uint16_t read_voltage(struct voltage *vol)
{
    adc_initialize(APB_ADC_BASE, true, false,
                   ADC_12_BITS_E3, ADC_SRC_CLK_ALT_E0, 8);
    return (uint16_t)adc_sample(APB_ADC_BASE, vol->adc_chnl);
}

static uint16_t read_pmic_voltage(void *i2c_handle, struct voltage *vol)
{
    /* Setup PMIC AMUX */
    pf8200_write_reg(i2c_handle, AMUX,
                     (1 << AMUX_EN_SHIFT) | (vol->amux_sel << AMUX_SEL_SHIFT));
    return read_voltage(vol);
}

static void monitor_voltage(void *i2c_handle)
{
    for (size_t i = 0; i < sizeof(g_ii4_voltages) / sizeof(g_ii4_voltages[0]);
            i++) {
        struct voltage *vol = &g_ii4_voltages[i];
        uint32_t value;

        if (vol->amux_sel) {
            value = read_pmic_voltage(i2c_handle, vol) *
                    vol->internal_ratio / vol->ratio;
        }
        else {
            value = read_voltage(vol) / vol->ratio;
        }

        /* Vsample = Rsample * Vref / [(2^resolution)-1] */
        float _voltage = value * ADC_REF_VOLTAGE / ((1 << ADC_RESOLUTION) - 1);
        printf("[%s]: %f\n", vol->name, _voltage);
    }
}

static void vol_monitor_main(const struct app_descriptor *app, void *args)
{
    void *i2c_handle;
    uint8_t device_id;
    uint8_t emrev;

    printf("voltage monitor start...\n");

    if (!hal_i2c_creat_handle(&i2c_handle, RES_I2C_I2C4)) {
        printf("Can not create I2C handle\n");
        return;
    }

    ASSERT(i2c_handle != NULL);

    device_id = pf8200_read_reg(i2c_handle, DEVICE_ID);
    emrev = pf8200_read_reg(i2c_handle, EMREV);

    printf("PF8200 device_id 0x%x, device_fam 0x%x\n",
           device_id & 0xf, device_id >> 4);
    printf("PF8200 emrev 0x%x, prog_id 0x%x\n",
           emrev & 0xf, emrev >> 4);

    adc_initialize(APB_ADC_BASE, true, false,
                   ADC_12_BITS_E3, ADC_SRC_CLK_ALT_E0, 8);
    event_init(&g_vol_monitor_evt, false, EVENT_FLAG_AUTOUNSIGNAL);

    while (true) {
        if (!g_enable_vol_monitor)
            event_wait(&g_vol_monitor_evt);

        if (g_enable_vol_monitor) {
            printf("\nSampling voltages...\n");
            monitor_voltage(i2c_handle);
            thread_sleep(5000);
        }
    }

    hal_i2c_release_handle(i2c_handle);
}

// LK console cmd
#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
STATIC_COMMAND_START
STATIC_COMMAND("vol_mon", "vol_mon 1 or vol_mon 0",
               (console_cmd)&vol_monitor_on)
STATIC_COMMAND_END(sem_monitor_test);
#endif

APP_START(sem_monitor_init)
.flags = 0,
.entry = vol_monitor_main,
APP_END
