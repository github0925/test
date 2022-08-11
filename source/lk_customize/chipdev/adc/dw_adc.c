/*
* dw_adc.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement semidrive lk fuse drv
*
* Revision History:
* -----------------
* 001, 08/07/2019 arrayhu implement this
*/
#include <trace.h>
#include <err.h>
#include <reg.h>
#include <assert.h>
#include <platform/debug.h>
//#include <__regs_int.h>
//#include <platform/interrupts.h>

#include "dw_adc.h"
#include "__regs_ap_adc.h"

#ifndef ADC_TIMERTAMP_OFF
#define ADC_TIMERTAMP_OFF 		(0x0c<<0)
#endif

#ifndef ADC_TIMERTAMP_BITS_OFFSET
#define ADC_TIMERTAMP_BITS_OFFSET	0x0
#endif
lk_time_t current_time(void);
/*
* analog IP SW reset bit set to 1 or 0
* function:ctrl rst analog ip
*/
void adc_set_dwc_rst_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_CTRL);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_CTRL_DWC_SW_RST) : (write_v &= ~BIT_AP_APB_ADC_ADC_CTRL_DWC_SW_RST);
	writel(write_v, base + REG_AP_APB_ADC_ADC_CTRL);
}

/*
* adc ctrl reset bit set to 1 or 0
* function:ctrl reset
*/
void adc_set_ctrl_rst_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_CTRL);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_CTRL_SW_RST) : (write_v &= ~BIT_AP_APB_ADC_ADC_CTRL_SW_RST);
	writel(write_v, base + REG_AP_APB_ADC_ADC_CTRL);
}

/*
* adc power down mode bit set to 1 or 0
*/
void adc_set_power_down_mode_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_CTRL);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_CTRL_POWERDOWN) : (write_v &= ~BIT_AP_APB_ADC_ADC_CTRL_POWERDOWN);
	writel(write_v, base + REG_AP_APB_ADC_ADC_CTRL);
}

/*
* adc standby mode bit set to 1 or 0
*/
void adc_set_standby_mode_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_CTRL);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_CTRL_STANDBY) : (write_v &= ~BIT_AP_APB_ADC_ADC_CTRL_STANDBY);
	writel(write_v, base + REG_AP_APB_ADC_ADC_CTRL);
}

/*
* adc differential mode bit set to 1 or 0
*/
void adc_set_delta_mode_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_CTRL);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_CTRL_DIFF_EN) : (write_v &= ~BIT_AP_APB_ADC_ADC_CTRL_DIFF_EN);
	writel(write_v, base + REG_AP_APB_ADC_ADC_CTRL);
}

/*
* adc SELRES bits set
* ADC resolution set
*/
void adc_set_resolution_bits(addr_t base, u32 bits_value)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_CTRL);
	write_v &= ~(u32)(0x3 << ADC_CTRL_SELRES_FIELD_OFFSET);
	write_v |= (u32)(bits_value << ADC_CTRL_SELRES_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_CTRL);
}

/*
* adc SELBG bit set to 1 or 0
*/
void adc_set_SELBG_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_CTRL);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_CTRL_SELBG) : (write_v &= ~BIT_AP_APB_ADC_ADC_CTRL_SELBG);
	writel(write_v, base + REG_AP_APB_ADC_ADC_CTRL);
}

/*
* adc SELREF bit set to 1 or 0
*/
void adc_set_SELREF_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_CTRL);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_CTRL_SELREF) : (write_v &= ~BIT_AP_APB_ADC_ADC_CTRL_SELREF);
	writel(write_v, base + REG_AP_APB_ADC_ADC_CTRL);
}

/*
* adc enable bit set to 1 or 0
* should be set to 1 allround the adc job period
*/
void adc_set_ADC_en_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_CTRL);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_CTRL_ENADC) : (write_v &= ~BIT_AP_APB_ADC_ADC_CTRL_ENADC);
	writel(write_v, base + REG_AP_APB_ADC_ADC_CTRL);
}

/*
* adc analog IP clk en bit set to 1 or 0
*/
void adc_set_DWC_CLK_en_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_CLK_CFG);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_CLK_CFG_DWC_CLK_EN) : (write_v &= ~BIT_AP_APB_ADC_ADC_CLK_CFG_DWC_CLK_EN);
	writel(write_v, base + REG_AP_APB_ADC_ADC_CLK_CFG);
}

/*
* adc clk divider value bits set
*/
void adc_set_clk_divider_bits(addr_t base, u32 bits_value)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_CLK_CFG);
	write_v &= ~(u32)(0xff << ADC_CLK_CFG_DIV_NUM_FIELD_OFFSET);
	write_v |= (u32)(bits_value << ADC_CLK_CFG_DIV_NUM_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_CLK_CFG);
}

/*
* adc clk src select bits set
*/
static void adc_set_clk_src_bits(addr_t base, u32 bits_value)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_CLK_CFG);
	write_v &= ~(u32)(0x3 << ADC_CLK_CFG_SRC_SEL_FIELD_OFFSET);
	write_v |= (u32)(bits_value << ADC_CLK_CFG_SRC_SEL_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_CLK_CFG);
}

/*
* adc timer count bits set
*/
void adc_set_timer_count_bits(addr_t base, u32 bits_value)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_TIMER);
	write_v &= ~(u32)(0xffff << ADC_TIMER_TERMINAL_VAL_FIELD_OFFSET);
	write_v |= (u32)(bits_value << ADC_TIMER_TERMINAL_VAL_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_TIMER);
}

/*
* adc timertamp en bit set to 1 or 0
* start timer
*/
void adc_set_timertamp_en_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_TIMER);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_TIMER_TS_EN) : (write_v &= ~BIT_AP_APB_ADC_ADC_TIMER_TS_EN);
	writel(write_v, base + REG_AP_APB_ADC_ADC_TIMER);
}

/*
* adc auto reload timer en bit set to 1 or 0
* start a sequence converting reset timer autoly
*/
void adc_set_auto_en_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_TIMER);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_TIMER_AUTO_EN) : (write_v &= ~BIT_AP_APB_ADC_ADC_TIMER_AUTO_EN);
	writel(write_v, base + REG_AP_APB_ADC_ADC_TIMER);
}

/*
* adc force reset timer en bit set to 1 or 0
*/
void adc_set_force_reload_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_TIMER);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_TIMER_RELOAD) : (write_v &= ~BIT_AP_APB_ADC_ADC_TIMER_RELOAD);
	writel(write_v, base + REG_AP_APB_ADC_ADC_TIMER);
}

/*
* adc timertamp bits read
*/
u32 adc_get_timertamp_bits(addr_t base)
{
	return (readl(base + ADC_TIMERTAMP_OFF) & 0xffff);
}

/*
* adc test mode select bits set
*/
void adc_set_test_mode_sel_bits(addr_t base, u32 bits_value)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_TEST);
	write_v &= ~(u32)(0x3 << ADC_TEST_SEL_FIELD_OFFSET);
	write_v |= (u32)(bits_value << ADC_TEST_SEL_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_TEST);
}

/*
* adc test control bits set
*/
void adc_set_test_control_bits(addr_t base, u32 bits_value)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_TEST);
	write_v &= ~(u32)(0x7 << ADC_TEST_ENCTR_FIELD_OFFSET);
	write_v |= (u32)(bits_value << ADC_TEST_ENCTR_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_TEST);
}

/*
* adc test mode en bit set to 1 or 0
*/
void adc_set_test_mode_en_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_TEST);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_TEST_TEST_MODE) : (write_v &= ~BIT_AP_APB_ADC_ADC_TEST_TEST_MODE);
	writel(write_v, base + REG_AP_APB_ADC_ADC_TEST);
}

#ifndef BIT_AP_APB_ADC_ADC_IMASK_TMR_OVER
#define BIT_AP_APB_ADC_ADC_IMASK_TMR_OVER (BIT_(4))
#endif
#ifndef BIT_AP_APB_ADC_ADC_IMASK_EOL
#define BIT_AP_APB_ADC_ADC_IMASK_TMR_EOL (BIT_(3))
#endif
#ifndef BIT_AP_APB_ADC_ADC_IMASK_CTC
#define BIT_AP_APB_ADC_ADC_IMASK_CTC (BIT_(2))
#endif

/*
* adc timer over int mask bit set to 1 or 0
*/
void adc_set_tmr_int_mask_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_IMASK);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_IMASK_TMR_OVER) : (write_v &= ~BIT_AP_APB_ADC_ADC_IMASK_TMR_OVER);
	writel(write_v, base + REG_AP_APB_ADC_ADC_IMASK);
}

/*
* adc end of loop int mask bit set to 1 or 0
*/
void adc_set_EOL_int_mask_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_IMASK);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_IMASK_EOL) : (write_v &= ~BIT_AP_APB_ADC_ADC_IMASK_EOL);
	writel(write_v, base + REG_AP_APB_ADC_ADC_IMASK);
}

/*
* adc CTC int mask bit set to 1 or 0
*/
void adc_set_CTC_int_mask_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_IMASK);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_IMASK_CTC) : (write_v &= ~BIT_AP_APB_ADC_ADC_IMASK_CTC);
	writel(write_v, base + REG_AP_APB_ADC_ADC_IMASK);
}
/*
* adc fifo int mask bit set to 1 or 0
*/
void adc_set_fifo_int_mask_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_IMASK);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_IMASK_WML) : (write_v &= ~BIT_AP_APB_ADC_ADC_IMASK_WML);
	writel(write_v, base + REG_AP_APB_ADC_ADC_IMASK);
}

/*
* adc conversion end int mask bit set to 1 or 0
*/
void adc_set_conv_end_int_mask_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_IMASK);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_IMASK_EOC) : (write_v &= ~BIT_AP_APB_ADC_ADC_IMASK_EOC);
	writel(write_v, base + REG_AP_APB_ADC_ADC_IMASK);
}

/*
* adc clear int bits
* int_bits should be adc_int_flag_e_t(a) | adc_int_flag_e_t(b)...
*/
static void adc_clear_int_bits(addr_t base, u32 int_bits)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_IFLAG);
	write_v |= int_bits;
	writel(write_v, base + REG_AP_APB_ADC_ADC_IFLAG);
}
/*
* retrun the int flags reg value
*/
u32 get_int_flags_reg_val(addr_t base)
{
	return (readl(base + REG_AP_APB_ADC_ADC_IFLAG) & 0x3f);
}

/*
* adc fifo dma request threshold bits set
*/
void adc_set_fifo_threshold_dma(addr_t base, u32 threshold)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_DMA_CFG);
	write_v &= ~(u32)(0xff << ADC_DMA_CFG_FIFO_TRIG_VAL_FIELD_OFFSET);
	write_v |= (u32)((threshold & 0xff) << ADC_DMA_CFG_FIFO_TRIG_VAL_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_DMA_CFG);
}

/*
* adc dma sw ack clear
*/
void adc_clear_dma_ack_bit(addr_t base)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_DMA_CFG);
	write_v &= ~BIT_AP_APB_ADC_ADC_DMA_CFG_SW_ACK;
	writel(write_v, base + REG_AP_APB_ADC_ADC_DMA_CFG);
}

/*
* check if dma sw ack
*/
bool if_dma_sw_ack(addr_t base)
{
	return ((readl(base + REG_AP_APB_ADC_ADC_DMA_CFG) & BIT_AP_APB_ADC_ADC_DMA_CFG_SW_ACK) ? true : false);
}

/*
* fixme:adc dma single en set to 1 or 0
*/
void adc_set_dma_single_en_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_DMA_CFG);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_DMA_CFG_SINGLE_EN) : (write_v &= ~BIT_AP_APB_ADC_ADC_DMA_CFG_SINGLE_EN);
	writel(write_v, base + REG_AP_APB_ADC_ADC_DMA_CFG);
}

/*
* fixme:adc dma en set to 1 or 0
*/
void adc_set_dma_en_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_DMA_CFG);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_DMA_CFG_DMA_EN) : (write_v &= ~BIT_AP_APB_ADC_ADC_DMA_CFG_DMA_EN);
	writel(write_v, base + REG_AP_APB_ADC_ADC_DMA_CFG);
}

/*
* adc interval bits set
*/
int adc_set_interval_bits(addr_t base, u32 interval)
{
	if (interval > 0xff)
	{
		return ERR_GENERIC;
	}
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_SSC);
	write_v &= ~(u32)(0xff << ADC_SSC_INTERVAL_FIELD_OFFSET);
	write_v |= (u32)(interval << ADC_SSC_INTERVAL_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_SSC);
	if (write_v != readl(base + REG_AP_APB_ADC_ADC_SSC))
	{
		printf("write interval fail\n");
		return ERR_GENERIC;
	}
	return NO_ERROR;
}

/*
* adc start converting
*/
void adc_start_convert(addr_t base)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_SSC);
	write_v |= BIT_AP_APB_ADC_ADC_SSC_CONV_START;
	writel(write_v, base + REG_AP_APB_ADC_ADC_SSC);
}

/*
* adc stop continuous converting
*/
void adc_stop_convert(addr_t base)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_SSC);
	write_v |= BIT_AP_APB_ADC_ADC_SSC_CONV_STOP;
	writel(write_v, base + REG_AP_APB_ADC_ADC_SSC);
}

/*
* adc init conversion
* use this init before really convert to make sure the first convert is right
*/
void adc_init_convert(addr_t base)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_SSC);
	write_v |= BIT_AP_APB_ADC_ADC_SSC_CONV_INIT;
	writel(write_v, base + REG_AP_APB_ADC_ADC_SSC);
}

/*
* adc converting mode set
* convert_mode is an enum element
*/
void adc_set_convert_mode(addr_t base, adc_convert_mode_t convert_mode)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_SSC);
	write_v &= ~(u32)(0xff << ADC_SSC_CONV_MODE_FIELD_OFFSET);
	write_v |= (u32)(convert_mode << ADC_SSC_CONV_MODE_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_SSC);
}

/*
* adc setup time set
* config_num is 0-3
*/
void adc_set_setup_time(addr_t base, u32 time_value, u32 config_num)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_SETUP);
	write_v &= ~(u32)(0xf << (config_num * 4));
	write_v |= (u32)((time_value & 0xf) << (config_num * 4));
	writel(write_v, base + REG_AP_APB_ADC_ADC_SETUP);
}

/*
* adc single ch conversion time count bits set
*/
void adc_set_CTC_bits(addr_t base, u32 bits_value)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_SCHC);
	write_v &= ~(u32)(0xffff << ADC_SCHC_CTC_FIELD_OFFSET);
	write_v |= (u32)((bits_value & 0xffff) << ADC_SCHC_CTC_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_SCHC);
}

/*
* adc single ch select bits set
*/
int adc_select_single_ch(addr_t base, u32 ch)
{
	if (ch > 0x7f)
	{
		printf("selected ch:%d is not surpported\n", ch);
		return ERR_GENERIC;
	}

	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_SCHC);
	write_v &= ~(u32)(0x7f << ADC_SCHC_CSEL_FIELD_OFFSET);
	write_v |= (u32)((ch & 0x7f) << ADC_SCHC_CSEL_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_SCHC);

	adc_set_repeat(base,!0);

	return NO_ERROR;
}

/*
* adc multiple ch repeat set
*/
void adc_set_repeat(addr_t base, u32 repeat)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_MCHC);
	write_v &= ~(u32)(0xf << ADC_MCHC_REPEAT_FIELD_OFFSET);
	write_v |= (u32)((repeat & 0xf) << ADC_MCHC_REPEAT_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_MCHC);
}

/*
* adc multiple entry loop set
*/
void adc_set_entry_loop(addr_t base, u32 loop)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_MCHC);
	write_v &= ~(u32)(0x3f << ADC_MCHC_LOOP_END_FIELD_OFFSET);
	write_v |= (u32)((loop & 0x3f) << ADC_MCHC_LOOP_END_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_MCHC);
}

/*
* adc fifo threshold for interrupt
*/
void adc_set_fifo_threashold_int(addr_t base, u32 threshold)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_FIFO);
	write_v &= ~(u32)(0xff << ADC_FIFO_THRE_FIELD_OFFSET);
	write_v |= (u32)((threshold & 0xff) << ADC_FIFO_THRE_FIELD_OFFSET);
	writel(write_v, base + REG_AP_APB_ADC_ADC_FIFO);
}

/*
* adc fifo en set to 1 or 0
*/
void adc_set_fifo_en_bit(addr_t base, bool is_to_1)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_FIFO);
	is_to_1 ? (write_v |= BIT_AP_APB_ADC_ADC_FIFO_ENABLE) : (write_v &= ~BIT_AP_APB_ADC_ADC_FIFO_ENABLE);
	writel(write_v, base + REG_AP_APB_ADC_ADC_FIFO);
}

/*
* adc flush fifo
*/
void adc_flush_fifo(addr_t base)
{
	u32 write_v = readl(base + REG_AP_APB_ADC_ADC_FIFO);
	write_v |= BIT_AP_APB_ADC_ADC_FIFO_FLUSH;
	writel(write_v, base + REG_AP_APB_ADC_ADC_FIFO);
}

/*
* adc idle status flag check
* 1=idle
*/
bool is_adc_idle(addr_t base)
{
	return ((readl(base + REG_AP_APB_ADC_ADC_FIFO) & BIT_AP_APB_ADC_ADC_FIFO_IDLE) ? true : false);
}

/*
* fixme:adc fifo full status flag check
* ? rst 0 ,0 = idle? 1=idle?
*/
bool is_adc_fifo_full(addr_t base)
{
	return ((readl(base + REG_AP_APB_ADC_ADC_FIFO) & BIT_AP_APB_ADC_ADC_FIFO_FULL) ? true : false); //1=full
}

/*
* fixme:adc fifo empty status flag check
* ? rst 0 ,0 = idle? 1=idle? why use two bits
*/
bool is_adc_fifo_empty(addr_t base)
{
	return ((readl(base + REG_AP_APB_ADC_ADC_FIFO) & BIT_AP_APB_ADC_ADC_FIFO_EMPTY) ? true : false); //1=empty
}

/*
* adc get fifo WML
* WML is the num of data avalible
*/
u32 adc_get_fifo_WML(addr_t base)
{
	return (readl(base + REG_AP_APB_ADC_ADC_FIFO) & 0x7f);
}
#ifndef ADC_RSLT_OFF
#define ADC_RSLT_OFF 		(ADC_APB_AB0_BASE_ADDR + (0x100<<0))
#endif
/*
* for get convert result without fifo
*/
u32 get_convert_result_reg(addr_t base)
{
	u32 ret = readl(base + ADC_RSLT_OFF);
	u32 dummy = readl(base + ADC_RSLT_OFF);
	(void)dummy;
	return ret;
}

/*
* adc fifo reg value read
* awalys read reg 0x40,also 0x44 ...
* hardware will give the fifo value and display the valid data num
*/
u32 get_fifo_reg_value(addr_t base)
{
	return (readl(base + ADC_RSLT_OFF));
}

/*
* adc entrys set
* should init a entry cfg struct array as the input
* reg addr from REG_AP_APB_ADC_ADC_ENTRY ,4 bytes step,the max entries num is 64
*/
int adc_cfg_enties(addr_t base, adc_entry_cfg_t *entry_cfg_array)
{
	u32 i = 0;

	u32 count = sizeof(entry_cfg_array) / sizeof(adc_entry_cfg_t);
	u32 entries_num = ((count <= ADC_MAX_ENTRIES_NUM) ? count : ADC_MAX_ENTRIES_NUM);

	for (i = 0; i < entries_num; i++)
	{
		writel(entry_cfg_array[i].v_entry, base + REG_AP_APB_ADC_ADC_ENTRY + (i * 4));
	}
	return i;//entries number
}

/*
* adc entry set
* set an entry cfg to entry given index
*/
int adc_cfg_an_entry(addr_t base, adc_entry_cfg_t entry_cfg, int index)
{
	if (index > ADC_MAX_ENTRIES_NUM)
	{
		return ERR_GENERIC;
	}

	writel(entry_cfg.v_entry, base + REG_AP_APB_ADC_ADC_ENTRY + (index * 4));
	return NO_ERROR;
}

/*
* adc monitor set
* should init a monitor cfg struct array as the input
* reg addr from REG_AP_APB_ADC_ADC_MON_CSR,REG_AP_APB_ADC_ADC_MON_THRE,8 bytes step, max monitor num is 8
*/
int adc_cfg_monitors(addr_t base, adc_monitor_cfg_t *monitor_cfg_array)
{
	u32 i = 0;

	u32 count = sizeof(monitor_cfg_array) / sizeof(adc_monitor_cfg_t);
	u32 monitors_num = ((count <= ADC_MAX_ENTRIES_NUM) ? count : ADC_MAX_MONITORS_NUM);

	for (i = 0; i < monitors_num; i++)
	{
		writel(monitor_cfg_array[i].v_mon_csr, base + REG_AP_APB_ADC_ADC_MON_CSR + (i * 8));
		writel(monitor_cfg_array[i].v_mon_threshold, base + REG_AP_APB_ADC_ADC_MON_THRE + (i * 8));
	}
	return i;//monitors number
}

/*
* setup analog module
*/
void adc_analog_setup(addr_t base_addr)
{
	adc_set_SELBG_bit(base_addr, false);
	adc_set_SELREF_bit(base_addr, false);
	adc_set_ADC_en_bit(base_addr, true);
	adc_set_dwc_rst_bit(base_addr, true);
	spin(200);
	adc_set_dwc_rst_bit(base_addr, false);

	adc_init_convert(base_addr);
}

/*
* reset adc ctrl
*/
void adc_reset_ctrl(addr_t base_addr)
{
	adc_set_ctrl_rst_bit(base_addr, true);
	adc_set_ctrl_rst_bit(base_addr, false);
}

/*
* reset crtl and setup analog ip
*/
void adc_init(addr_t base_addr)
{
	adc_reset_ctrl(base_addr);
	adc_analog_setup(base_addr);
}

void adc_dinit(addr_t base_addr)
{
	adc_set_dwc_rst_bit(base_addr, true);
	adc_set_ctrl_rst_bit(base_addr, true);
	adc_set_ADC_en_bit(base_addr, false);
}

void adc_set_clk_src(addr_t base_addr, adc_clk_source_e_t src_type)
{
	adc_set_clk_src_bits(base_addr, src_type);
}

void adc_set_resolution(addr_t base_addr, adc_resolution_e_t adc_resolution)
{
	adc_set_resolution_bits(base_addr, adc_resolution);
}

void adc_disable_all_int(addr_t base_addr)
{

	u32 write_v = readl(base_addr + REG_AP_APB_ADC_ADC_IMASK);
	write_v &= ~(ADC_EOC_INT_FLAG_E1 | \
	             ADC_WML_INT_FLAG_E2 | \
	             ADC_CTC_INT_FLAG_E4 | \
	             ADC_EOL_INT_FLAG_E8 | \
	             ADC_OVF_INT_FLAG_E16);
	writel(write_v, base_addr + REG_AP_APB_ADC_ADC_IMASK);
}

/*
* Read more then 4 bytes data with single bus access to avoid adc fifo overwrite
* read_num:total words(4bytes) to read
*/
int adc_read_fifo_burst4(addr_t base_addr, u32 *buffer, u32 read_num)
{
	u32 i;
	if (!buffer)
	{
		printf("error:input ptr is null\n");
		return ERR_GENERIC;
	}

	if (read_num <= ADC_FIFO_MAX_DEPTH)
	{
		while (adc_get_fifo_WML(base_addr) >= read_num);
		for (i = 0; i < read_num; i++)
		{
			*(buffer++) = get_convert_result_reg(base_addr);
		}
		return NO_ERROR;
	}
	else
	{
		// addr_t fifo_reg_addr = base_addr + 0x40;
		for (i = 0; i < read_num / 4; i++)
		{
			buffer += (i * 4);
			while (adc_get_fifo_WML(base_addr) < 4);
			// __asm__ __volatile__(
			//     "ldr r0,r1,[%0]\n"
			//     "str r0,r1,[%1]\n"
			//     :
			//     :"r"(fifo_reg_addr), "r"(buffer)
			//     :"memory", "r0", "r1"
			// );
		}

		u32 tail_num = read_num - (i * 4);
		while (adc_get_fifo_WML(base_addr) < tail_num);

		for (u32 j = 0; j < tail_num ; j++)
		{
			*(buffer + j) = get_convert_result_reg(base_addr);
		}
		return NO_ERROR;
	}
	return ERR_GENERIC;
}
/*
* adc_single_convert()
* adc single channel single convert without using dma read
* Disable all the interrupt,disable timestamp,disable dma request
* restul_data:write convert result to this ptr
*/
int adc_single_convert(addr_t base_addr, u32 ch, void *result_data)
{
	if (!result_data)
	{
		printf("error:input ptr is null\n");
		return ERR_GENERIC;
	}

	adc_set_interval_bits(base_addr, 0x20);
	//single ch single conv
	adc_set_convert_mode(base_addr, ADC_SINGLE_CH_SINGLE_E0); // ADC_SINGLE_CH_SINGLE_E0 ADC_SINGLE_CH_BACK2BACK_E1
	//select ch
	if (ERR_GENERIC == adc_select_single_ch(base_addr, ch))
	{
		return ERR_GENERIC;
	}
	//fifo en
	adc_set_fifo_en_bit(base_addr, true);
	//if en interrupt,the fifo maybe read by intrrupt handler
	adc_disable_all_int(base_addr);
	//timertamp disable
	adc_set_timertamp_en_bit(base_addr, false);
	//disable dma
	adc_set_dma_en_bit(base_addr, false);
	adc_set_dma_single_en_bit(base_addr, false);
	//interval 32
	adc_set_interval_bits(base_addr, 0x20);
	//fulsh fifo
	adc_flush_fifo(base_addr);
	//start convert
	adc_start_convert(base_addr);

	//fifo is not empty,single convert end
	while (!adc_get_fifo_WML(base_addr));
	u32 read_data = get_convert_result_reg(base_addr);
	if ((((read_data >> 16) & 0x7f) != ch) || (read_data & BIT_AP_APB_ADC_ADC_RSLT_TS_IND))
	{
		printf("error:convert result ch error or timestamp\n");
		return ERR_GENERIC;
	}
	*(u32 *)result_data = read_data & 0xffff;

	return NO_ERROR;
}

/*
* adc_single_convert_dma()
* adc single channel single convert without using dma read
* Disable all the interrupt,disable timestamp,enable dma request
* no result data return,dma read the result data
*/
int adc_single_convert_dma(addr_t base_addr, u32 ch)
{
	//single ch single conv
	adc_set_convert_mode(base_addr, ADC_SINGLE_CH_SINGLE_E0);
	//select ch
	if (ERR_GENERIC == adc_select_single_ch(base_addr, ch))
	{
		return ERR_GENERIC;
	}
	//fifo en
	adc_set_fifo_en_bit(base_addr, true);
	//if en EOC interrupt,the fifo maybe read by intrrupt handler
	adc_disable_all_int(base_addr);
	//timertamp disable
	adc_set_timertamp_en_bit(base_addr, false);
	//enable dma
	adc_set_dma_en_bit(base_addr, true);
	adc_set_dma_single_en_bit(base_addr, true);
	//set fifo WML threshold for dma request
	adc_set_fifo_threshold_dma(base_addr, 1);
	//fulsh fifo
	adc_flush_fifo(base_addr);
	//start convert
	adc_start_convert(base_addr);

	return NO_ERROR;
}
/*
* fix interval=n
*/
int adc_fix_interval(addr_t base_addr, u32 ch, u32 n)
{
	if (ERR_GENERIC == adc_select_single_ch(base_addr, ch))
	{
		return ERR_GENERIC;
	}

	if (ERR_GENERIC == adc_set_interval_bits(base_addr, n))
	{
		return ERR_GENERIC;
	}

	adc_set_convert_mode(base_addr, ADC_SINGLE_CH_INTERVAL_E2);//single ch fix interval

	adc_disable_all_int(base_addr);
	adc_set_fifo_en_bit(base_addr, true);
	adc_set_timertamp_en_bit(base_addr, false);
	adc_set_dma_en_bit(base_addr, false);
	adc_set_dma_single_en_bit(base_addr, false);

	adc_flush_fifo(base_addr);
	adc_start_convert(base_addr);

	return NO_ERROR;
}

int adc_fix_interval_dma(addr_t base_addr, u32 ch, u32 n, u32 fifo_request_thold)
{
	if (ERR_GENERIC == adc_select_single_ch(base_addr, ch))
	{
		return ERR_GENERIC;
	}

	if (ERR_GENERIC == adc_set_interval_bits(base_addr, n))
	{
		return ERR_GENERIC;
	}

	adc_set_convert_mode(base_addr, ADC_SINGLE_CH_INTERVAL_E2);//single ch fix interval

	adc_disable_all_int(base_addr);
	adc_set_fifo_en_bit(base_addr, true);
	adc_set_timertamp_en_bit(base_addr, false);
	adc_set_dma_en_bit(base_addr, true);
	adc_set_dma_single_en_bit(base_addr, true);
	adc_set_fifo_threshold_dma(base_addr, fifo_request_thold);

	adc_flush_fifo(base_addr);
	adc_start_convert(base_addr);

	return NO_ERROR;
}

/*
* just set continuous mode and start converting,no result read in this function
* Dma using,interrupt using ,fifo using, timertamp using need call some api
* before this function call
*/
int adc_start_single_ch_continuous(addr_t base_addr, u32 ch)
{
	if (ERR_GENERIC == adc_select_single_ch(base_addr, ch))
	{
		return ERR_GENERIC;
	}

	adc_set_convert_mode(base_addr, ADC_SINGLE_CH_BACK2BACK_E1);

	adc_start_convert(base_addr);

	return NO_ERROR;
}

/*
* average value of the convert results.average_num=n will average n+1 results.
*/
int adc_single_ch_average_n(addr_t base_addr, u32 ch, u32 *result_data, unsigned char average_num)
{
	u32 read_data = 0, sum_data = 0;

	if ((!result_data))
	{
		printf("inuput ptr is null\n");
		return ERR_GENERIC;
	}

	if (ERR_GENERIC == adc_start_single_ch_continuous(base_addr, ch))
	{
		return ERR_GENERIC;
	}

	for (unsigned char i = 0; i < average_num + 1; i++)
	{
		//fifo is not empty,single convert end
		while (!adc_get_fifo_WML(base_addr));
		read_data = get_convert_result_reg(base_addr);
		if ((((read_data >> 16) & 0x7f) != ch) || (read_data & BIT_AP_APB_ADC_ADC_RSLT_TS_IND))
		{
			printf("error:convert result ch error or timestamp\n");
			return ERR_GENERIC;
		}
		sum_data += (read_data & 0xffff);
	}

	* result_data = sum_data / (average_num + 1);
	return NO_ERROR;
}

/*
* just set multiple ch mode and start converting,no result read in this function
* Dma using,interrupt using ,fifo using, timertamp using need call some api
* before this function call
*/
int adc_set_multiple_ch(addr_t base_addr, adc_loop_repeat_cfg_t mchc_v, adc_entry_cfg_t *entry_cfg, adc_monitor_cfg_t *mon_cfg)
{
	if (!entry_cfg || !mon_cfg)
	{
		printf("error:input ptr is null\n");
		return ERR_GENERIC;
	}

	adc_set_convert_mode(base_addr, ADC_MULTIPLE_CH_E3);

	u32 count = (mchc_v.loop_end >= ADC_MAX_ENTRIES_NUM) ? ADC_MAX_ENTRIES_NUM : mchc_v.loop_end;
	mchc_v.loop_end = count;
	//set common repeat and loop_end
	writel(mchc_v.v_mchc, base_addr + REG_AP_APB_ADC_ADC_MCHC);
	//set entries
	//u32 count = (mchc_v.loop_end >= ADC_MAX_ENTRIES_NUM ) ? ADC_MAX_ENTRIES_NUM : mchc_v.loop_end;
	for (u32 i = 0; i < count; i++)
	{
		writel(entry_cfg[i].v_entry, base_addr + REG_AP_APB_ADC_ADC_ENTRY + (i * 4));
	}
	//set monitors
	adc_cfg_monitors(base_addr, mon_cfg);

	adc_start_convert(base_addr);

	return NO_ERROR;
}

int adc_clear_all_int_flag(addr_t base_addr)
{
	adc_clear_int_bits(base_addr, ADC_EOC_INT_FLAG_E1 | ADC_WML_INT_FLAG_E2 | \
	                   ADC_CTC_INT_FLAG_E4 | ADC_EOL_INT_FLAG_E8 | \
	                   ADC_OVF_INT_FLAG_E16 | ADC_DUMMY_INT_FLAG_E32);
	return NO_ERROR;
}

int adc_clear_int_flag(addr_t base_addr, adc_int_flag_e_t flag_type)
{
	adc_clear_int_bits(base_addr, flag_type);
	return NO_ERROR;
}

bool adc_single_polling_eoc(addr_t base,uint32_t ms_timeout)
{
	uint32_t now = current_time();
	addr_t reg = base + REG_AP_APB_ADC_ADC_IFLAG;
	uint32_t val = 0;
	while(current_time() < (now + ms_timeout*1000))
	{
		val = readl(reg);
		if(val & (1 << ADC_IFLAG_EOC_FIELD_OFFSET)) return true;
	}

	return false;
}

