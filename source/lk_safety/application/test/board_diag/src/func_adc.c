/*
 * func_adc.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include <dw_adc.h>
#include <__regs_base.h>
#include "board_start.h"
#include "board_init.h"
#include "board_cfg.h"
#include "func_can.h"
#include "func_i2c.h"

#define ADC_REFERENCE_VOLTAGE 180
#define ADC_ACCURACY 4095
#define INPUT_V_BAT  120
#define get_read_timers_value(x, y) (x <= y)? (x++):(x=1)
#define is_get_channel_legal(x, y) (((x*READ_ADC_CHN_MAX+y)<ADC_CHN_TOTAL) ? true:false)

const static com_chn_table_t adc_chn_table[] = {
    {0x00, ADC_CHN_0},
    {0x01, ADC_CHN_1},
    {0x02, ADC_CHN_2},
    {0x03, ADC_CHN_3},
    {0x04, ADC_CHN_4},
    {0x05, ADC_CHN_5},
    {0x06, ADC_PMIC_CHN},
    {0x07, ADC_CHN_7},
};

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
int  adc_select_single_ch(addr_t base, u32 ch);
void adc_start_convert(addr_t base);
bool adc_single_polling_eoc(addr_t base, uint32_t ms_timeout);
u32  get_convert_result_reg(addr_t base);
int  adc_clear_all_int_flag(addr_t base_addr);
void adc_set_SELBG_bit(addr_t base, bool is_to_1);

static void adc_initialize(addr_t base, bool extref, bool differential,
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

static uint16_t adc_single_channel_sample_once(addr_t base, uint8_t channel)
{
    uint16_t result = 0;
    adc_set_convert_mode(base, ADC_SINGLE_CH_SINGLE_E0);
    result = adc_select_single_ch(base, channel);

    if (result != 0) return 0;

    adc_start_convert(base);

    if (adc_single_polling_eoc(base, 1) == false) {

        return 0;
    }

    result  = (uint16_t)get_convert_result_reg(base);

    adc_clear_all_int_flag(base);

    return result;
}

void adc_channel_init(void)
{
    adc_initialize(APB_ADC_BASE, true, false, ADC_12_BITS_E3, ADC_SRC_CLK_ALT_E0,
                   8);
}

static uint16_t adc_channel_read(uint8_t channel)
{
    uint16_t result = 0;

    result = (uint16_t)(adc_single_channel_sample_once(APB_ADC_BASE,
                        channel));

    return result;
}

static uint16_t adc_result_conver_to_actual_value(uint16_t res)
{
    return (uint16_t)(((float)res / ADC_ACCURACY) * ADC_REFERENCE_VOLTAGE);
}
/*adc read value single*/
static bool _adc_single_read(board_test_exec_t *exec)
{
    bool ret = false;
    uint16_t result;
    uint8_t cmdStatus = CMD_PARA_ERR;
    can_cmd_t *adc_cmd = (can_cmd_t *)exec->cmd;

    if (adc_cmd->dev_id == g_step_case_table[ADC_SERIAL_ID].cmd_id) {

        for (uint8_t num = 0; num < ARRAY_SIZE(adc_chn_table); num++) {

            if (adc_cmd->route_channel_id != adc_chn_table[num].pin_num) {
                continue;
            }
            else {
                if (adc_cmd->route_channel_id == ADC_PMIC_CHN) {
                    result = adc_result_form_pmic_dev(exec);//adc value from pmic_pf8200 devices
                }
                else {
                    result = adc_result_conver_to_actual_value(
                                 adc_channel_read( //adc value from chip adc channel
                                     adc_cmd->route_channel_id));
                }

                set_para_value(cmdStatus, NORMAL_DEAL);
                set_para_value(ret, true);
                dprintf(debug_show_null, "adc channel %d to read\n", adc_cmd->route_channel_id);
                dprintf(debug_show_null, "adc result is %x\n", result);
                break;
            }
        }
    }
    else {
        set_resp_err_state(exec->resp[0], cmdStatus);
        return ret;
    }

    if (ret != true) {
        set_resp_err_state(exec->resp[0], cmdStatus);
        return ret;
    }

    set_para_value(exec->resp[0], cmdStatus);
    set_para_value(exec->resp[1], adc_cmd->route_channel_id);
    set_para_value(exec->resp[2], (uint8_t)((result >> 8) & 0xff));
    set_para_value(exec->resp[3], (uint8_t)(result & 0xff));

    return ret;
}
/*adc read value periodic*/
static bool _adc_period_read(board_test_exec_t *exec)
{
    bool ret = false;
    canx_opt_t canx_opt;
    uint8_t adc_chn;
    uint8_t read_timers = 0;
    uint8_t cmdStatus = NORMAL_DEAL;
    uint16_t result[READ_ADC_CHN_MAX];
    uint32_t respCanID = PERIODIC_RESP_ADC;

    uint8_t adc_loop_cycle = ADC_CHN_TOTAL / READ_ADC_CHN_MAX;

    ADC_CHN_TOTAL % READ_ADC_CHN_MAX ? adc_loop_cycle++ :
    (adc_loop_cycle = adc_loop_cycle);

    set_para_value(canx_opt.can_send, (can_send_t *)canx_opt.pay_load);

    for (uint8_t num = 0; num < adc_loop_cycle; num++) {

        for (uint8_t cn = 0; cn < READ_ADC_CHN_MAX; cn++) {

            adc_chn = (read_timers * READ_ADC_CHN_MAX + cn);

            if (adc_chn == ADC_PMIC_CHN) {
                result[cn] = adc_result_form_pmic_dev(exec);//adc value from pmic_pf8200 devices
            }
            else {
                result[cn] = adc_result_conver_to_actual_value(adc_channel_read(
                                 adc_chn));//adc value from chip adc channel
            }
        }

        get_read_timers_value(read_timers, adc_loop_cycle);
        set_para_value(exec->resp[0], cmdStatus);
        set_para_value(exec->resp[1], read_timers);
        set_para_value(exec->resp[2], (uint8_t)((result[0] >> 8) & 0xff));
        set_para_value(exec->resp[3], (uint8_t)(result[0] & 0xff));
        set_para_value(exec->resp[4], (uint8_t)((result[1] >> 8) & 0xff));
        set_para_value(exec->resp[5], (uint8_t)(result[1] & 0xff));
        set_para_value(exec->resp[6], (uint8_t)((result[2] >> 8) & 0xff));
        set_para_value(exec->resp[7], (uint8_t)(result[2] & 0xff));

        can_common_response(exec, respCanID);
    }

    return ret;
}
/*adc function process*/
bool board_adc_reply_deal(board_test_exec_t *exec, board_test_state_e state)
{
    bool ret = false;

    if (state == STATE_SINGLE) {
        ret = _adc_single_read(exec);
        set_para_value(exec->board_response, can_common_response); //set callback
    }
    else if (state == STATE_PERIODIC) {
        ret = _adc_period_read(exec);
        set_para_value(exec->board_response, NULL);
    }

    return ret;
}
