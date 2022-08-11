/*
* dw_adc.h
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement semidrive fuse drv head file
*
* Revision History:
* -----------------
* 001, 08/13/2019 arrayhu implement this
*/
#ifndef __DW_ADC_H__
#define __DW_ADC_H__

#include <assert.h>
#include <sys/types.h>

#include "__regs_ap_adc.h"
#define ADC_MAX_ENTRIES_NUM		64
#define ADC_MAX_MONITORS_NUM		8
#define ADC_FIFO_MAX_DEPTH		128
#define ADC_CONVERT_RET_TIME 1
typedef enum
{
	ADC_EOC_INT_FLAG_E1 = 0x01,
	ADC_WML_INT_FLAG_E2 = 0x02,
	ADC_CTC_INT_FLAG_E4 = 0x04,
	ADC_EOL_INT_FLAG_E8 = 0x08,
	ADC_OVF_INT_FLAG_E16 = 0x10,
	ADC_DUMMY_INT_FLAG_E32 = 0x20,
} adc_int_flag_e_t;

typedef enum
{
	ADC_SINGLE_CH_SINGLE_E0 = 0x00,
	ADC_SINGLE_CH_BACK2BACK_E1,
	ADC_SINGLE_CH_INTERVAL_E2,
	ADC_MULTIPLE_CH_E3,
} adc_convert_mode_t;

typedef enum
{
	ADC_SRC_CLK_ALT_E0 = 0x00,
	ADC_SRC_CLK_PCLK_E1 = 0x01,
	ADC_SRC_CLK_EXT_E2 = 0x02,
} adc_clk_source_e_t;

typedef enum
{
	ADC_6_BITS_E0 = 0x00,
	ADC_8_BITS_E1 = 0x01,
	ADC_10_BITS_E2 = 0x02,
	ADC_12_BITS_E3 = 0x03,
} adc_resolution_e_t;

typedef union
{
	struct
	{
		volatile u16 result : 16;
		volatile u8 ch : 7;
		volatile u32 rsvd_23_30 : 8;
		volatile u8 is_timetap : 1;
	};
	volatile u32 v_result_reg;
} adc_result_reg_t;

typedef union
{
	struct
	{
		volatile u8 loop_end : 6;
		volatile u8 rsvd_6_7 : 2;
		volatile u8 repeat : 4;
		volatile u32 rsvd_12_31 : 20;
	};
	volatile u32 v_mchc;
} adc_loop_repeat_cfg_t;

typedef	union
{
	struct
	{
		volatile u8 rct_v : 8;
		volatile u8 ch : 7;
		volatile u8 rsvd_15_15 : 1;
		volatile u8 setup_time_sel : 2;
		volatile u8 rsvd_18_20 : 3;
		volatile u8 ignore_result_en : 1;
		volatile u8 skip_en : 1;
		volatile u8 entry_en : 1;
		volatile u8 sample_cycle : 8;
	};
	volatile u32 v_entry;
} adc_entry_cfg_t;

typedef struct
{
	union
	{
		struct
		{
			volatile u8 monitor_en : 1;
			volatile u8 monitor_ch : 7;
			volatile u8 low_flag : 1;
			volatile u8 high_flag : 1;
			volatile u8 in_range_flag : 1;
			volatile u8 out_of_range_flag : 1;
			volatile u8 discard_v_result : 1;
			volatile u8 rsvd_13_15 : 3;
			volatile u8 v_detected : 1;
			volatile u16 rsvd_17_31 : 15;
		};
		volatile u32 v_mon_csr;
	};
	union
	{
		struct
		{
			volatile u16 v_low_threshold : 12;
			volatile u8 rsvd_12_15 : 4;
			volatile u16 v_high_threshold : 12;
			volatile u8 rsvd_28_31 : 4;
		};
		volatile u32 v_mon_threshold;
	};

} adc_monitor_cfg_t;

void adc_set_power_down_mode_bit(addr_t base, bool is_to_1);
void adc_set_standby_mode_bit(addr_t base, bool is_to_1);
void adc_set_delta_mode_bit(addr_t base, bool is_to_1);
//to set analog internel clock src
void adc_set_SELBG_bit(addr_t base, bool is_to_1);
void adc_set_SELREF_bit(addr_t base, bool is_to_1);
void adc_set_ADC_en_bit(addr_t base, bool is_to_1);
void adc_set_DWC_CLK_en_bit(addr_t base, bool is_to_1);
void adc_set_clk_divider_bits(addr_t base, u32 bits_value);
void adc_set_timer_count_bits(addr_t base, u32 bits_value);
u32 adc_get_timertamp_bits(addr_t base);
void adc_set_test_mode_sel_bits(addr_t base, u32 bits_value);
void adc_set_test_control_bits(addr_t base, u32 bits_value);
void adc_set_test_mode_en_bit(addr_t base, bool is_to_1);
void adc_set_tmr_int_mask_bit(addr_t base, bool is_to_1);
void adc_set_EOL_int_mask_bit(addr_t base, bool is_to_1);
void adc_set_CTC_int_mask_bit(addr_t base, bool is_to_1);
void adc_set_fifo_int_mask_bit(addr_t base, bool is_to_1);
void adc_set_conv_end_int_mask_bit(addr_t base, bool is_to_1);
u32 get_int_flags_reg_val(addr_t base);
void adc_set_fifo_threshold_dma(addr_t base, u32 threshold);
void adc_clear_dma_ack_bit(addr_t base);
bool if_dma_sw_ack(addr_t base);
void adc_set_dma_single_en_bit(addr_t base, bool is_to_1);
void adc_set_dma_en_bit(addr_t base, bool is_to_1);
void adc_set_timertamp_en_bit(addr_t base, bool is_to_1);
int adc_set_interval_bits(addr_t base, u32 interval);
void adc_start_convert(addr_t base);
void adc_stop_convert(addr_t base);
void adc_init_convert(addr_t base);
void adc_set_convert_mode(addr_t base, adc_convert_mode_t convert_mode);
void adc_set_setup_time(addr_t base, u32 time_value, u32 config_num);
void adc_set_CTC_bits(addr_t base, u32 bits_value);
int adc_select_single_ch(addr_t base, u32 ch);
void adc_set_repeat(addr_t base, u32 repeat);
void adc_set_entry_loop(addr_t base, u32 loop);
int adc_cfg_an_entry(addr_t base, adc_entry_cfg_t entry_cfg, int index);
void adc_set_fifo_threashold_int(addr_t base, u32 threshold);
void adc_set_fifo_en_bit(addr_t base, bool is_to_1);
void adc_flush_fifo(addr_t base);
bool is_adc_idle(addr_t base);
bool is_adc_fifo_full(addr_t base);
bool is_adc_fifo_empty(addr_t base);
u32 adc_get_fifo_WML(addr_t base);
u32 get_convert_result_reg(addr_t base);
u32 get_fifo_reg_value(addr_t base);
int adc_cfg_enties(addr_t base, adc_entry_cfg_t *entry_cfg_array);
int adc_cfg_monitors(addr_t base, adc_monitor_cfg_t *monitor_cfg_array);
void adc_init(addr_t base_addr);
void adc_dinit(addr_t base_addr);
void adc_set_resolution(addr_t base_addr, adc_resolution_e_t adc_resolution);
int adc_read_fifo_burst4(addr_t base_addr, u32 *buffer, u32 read_num);
int adc_single_convert(addr_t base_addr, u32 ch, void *result_data);
int adc_single_convert_dma(addr_t base_addr, u32 ch);
int adc_fix_interval(addr_t base_addr, u32 ch, u32 n);
int adc_fix_interval_dma(addr_t base_addr, u32 ch, u32 n, u32 fifo_request_thold);
int adc_single_ch_average_n(addr_t base_addr, u32 ch, u32 *result_data, u8 average_num);
int adc_start_single_ch_continuous(addr_t base_addr, u32 ch);
int adc_set_multiple_ch(addr_t base_addr, adc_loop_repeat_cfg_t mchc_v, adc_entry_cfg_t *entry_cfg, adc_monitor_cfg_t *mon_cfg);
void adc_disable_all_int(addr_t base_addr);
int adc_clear_all_int_flag(addr_t base_addr);
int adc_clear_int_flag(addr_t base_addr, adc_int_flag_e_t flag_type);
//adc Analog module and ctrl reset
void adc_reset_ctrl(addr_t base_addr);

//set adc clk source
void adc_set_clk_src(addr_t base_addr, adc_clk_source_e_t src_type);

#endif

