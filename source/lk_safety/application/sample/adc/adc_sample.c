/*
 * adc_sample.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ADC sample application.
 *
 * Revision History:
 * -----------------
 */
#include <lib/console.h>
#include "dw_adc.h"
#include "__regs_base.h"

#define ADC_REF_VOLTAGE     (1.8)
#define ADC_RESOLUTION      (12)

void adc_reset_ctrl(addr_t base_addr);
void adc_set_resolution(addr_t base_addr, adc_resolution_e_t adc_resolution);
void adc_set_SELREF_bit(addr_t base, bool is_to_1);
void adc_set_clk_src(addr_t base_addr, adc_clk_source_e_t src_type);
void adc_set_DWC_CLK_en_bit(addr_t base, bool is_to_1);
void adc_set_ADC_en_bit(addr_t base, bool is_to_1);
void adc_analog_setup(addr_t base_addr);
void adc_init_convert(addr_t base);
void adc_set_clk_divider_bits(addr_t base, u32 bits_value);
void adc_set_delta_mode_bit(addr_t base, bool is_to_1);
void adc_set_convert_mode(addr_t base, adc_convert_mode_t convert_mode);
int adc_select_single_ch(addr_t base, u32 ch);
void adc_start_convert(addr_t base);
bool adc_single_polling_eoc(addr_t base, uint32_t ms_timeout);
u32 get_convert_result_reg(addr_t base);
int adc_clear_all_int_flag(addr_t base_addr);
void adc_set_SELBG_bit(addr_t base, bool is_to_1);

void adc_initialize(addr_t base, bool extref, bool differential,
                    adc_resolution_e_t resolution, adc_clk_source_e_t clksrc, uint8_t clkdiv)
{
    adc_reset_ctrl(base);   //rst adcc
    adc_set_SELREF_bit(base, !extref); //use external refh(VDDA1.8)

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
    adc_init_convert(base);//dummy conversion
}

uint32_t adc_single_channel_sample_once(addr_t base, uint8_t channel)
{
    uint32_t result = 0;
    adc_set_convert_mode(base, ADC_SINGLE_CH_SINGLE_E0);
    result = adc_select_single_ch(base, channel);

    if (result != 0)
        return 0;

    adc_start_convert(base);

    if (adc_single_polling_eoc(base, 1) == false) {
        printf("polling eoc @1ms timeout.\n");
        return 0;
    }

    result  = get_convert_result_reg(base);
    adc_clear_all_int_flag(base);

    return result;
}

int adc_sample_once(int argc, const cmd_args *argv)
{
    uint32_t channel = 0;
    uint32_t value;
    float voltage;

    adc_initialize(APB_ADC_BASE, true, false, ADC_12_BITS_E3,
                   ADC_SRC_CLK_ALT_E0, 8);

    if (argc == 2) {
        channel = argv[1].u;
    }

    /* Vsample = Rsample * Vref / [(2^resolution)-1] */
    value = (uint16_t)adc_single_channel_sample_once(APB_ADC_BASE, channel);
    voltage = value * ADC_REF_VOLTAGE / ((1 << ADC_RESOLUTION) - 1);
    printf("sample %d: %f\n", channel, voltage);

    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("ads", "adc sample once", (console_cmd)&adc_sample_once)
STATIC_COMMAND_END(ads);
#endif
