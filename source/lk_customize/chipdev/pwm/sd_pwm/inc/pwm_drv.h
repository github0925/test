/*
* pwm_drv.h
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: semidrive pwm headfile
*
* Revision History:
* -----------------
* 010, 11/26/2019 wang yongjun implement this
*/

#ifndef __SD_PWM_DRV_H__
#define __SD_PWM_DRV_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include <__regs_apb_pwm.h>

/*! ****************************************************************************
* .Define[global]
********************************************************************************/
#define DRV_PWM_HF_CLOCK_FREQ (398000000ul)

#define DRV_PWM_SIMPLE_CLOCK_DIV_NUM  1

#define DRV_PWM_PCM_TUNE_TABLE_METHOD 0
#define DRV_PWM_PCM_TUNE_CALCU_METHOD 1
#define DRV_PWM_PCM_TUNE_GET_METHOD   DRV_PWM_PCM_TUNE_TABLE_METHOD

#define DRV_PWM_FIFO_WML   32
#define DRV_PWM_FIFO_DEPTH 64

#define DRV_PWM_DEBUG_LEVEL ALWAYS

/*! ****************************************************************************
* .Enum[global]
********************************************************************************/
typedef enum {
    DRV_PWM_CHN_A = 0,
    DRV_PWM_CHN_B,
    DRV_PWM_CHN_C,
    DRV_PWM_CHN_D,
    DRV_PWM_CHN_TOTAL
} drv_pwm_chn_t;

typedef enum {
    DRV_PWM_SRC_CLK_HF = 0,
    DRV_PWM_SRC_CLK_AHF,
    DRV_PWM_SRC_CLK_RESERVE,
    DRV_PWM_SRC_CLK_EXT,
} drv_pwm_src_clk_t;

typedef enum {
    DRV_PWM_CONTINUE_CMP = 0,
    DRV_PWM_ONE_SHOT_CMP,
} drv_pwm_single_mode_t;

typedef enum {
	DRV_CMP_OUT_POSITIVE_PULSE = 0,
	DRV_CMP_OUT_NEGATIVE_PULSE,
	DRV_CMP_OUT_SIGNAL_TOGGLE,
	DRV_CMP_OUT_LEVEL_HIGH,
	DRV_CMP_OUT_LEVEL_LOW,
	DRV_CMP_OUT_KEEP,
} drv_pwm_cmp_out_mode_t;

typedef enum {
    DRV_PWM_EDGE_ALIGN_MODE = 0,
    DRV_PWM_CENTER_ALIGN_MODE,
} drv_pwm_align_mode_t;

typedef enum {
    DRV_PWM_PHASE_POLARITY_POS = 0,
    DRV_PWM_PHASE_POLARITY_NEG,
} drv_pwm_phase_polarity_t;

typedef enum {
	DRV_PWM_PCM_DRIVE_MONO_CHANNEL = 0,
	DRV_PWM_PCM_DRIVE_DUAL_CHANNEL,
	DRV_PWM_PCM_DRIVE_DUAL_H_BRIDGE,
	DRV_PWM_PCM_DRIVE_FOUR_H_BRIDGE,
} drv_pwm_pcm_drive_mode_t;

typedef enum {
    DRV_PWM_CHN_A_WORK = 0,
    DRV_PWM_CHN_A_B_WORK,
    DRV_PWM_CHN_A_B_C_D_WORK,
} drv_pwm_group_num_t;

typedef enum {
    DRV_PWM_CMP_DATA_32BITS = 0,
    DRV_PWM_CMP_DATA_16BITS,
    DRV_PWM_CMP_DATA_8BITS,
} drv_pwm_data_format_t;

typedef enum {
    DRV_PWM_DITHER_IN_8BITS = 0,
    DRV_PWM_DITHER_IN_16BITS,
    DRV_PWM_DITHER_IN_24BITS,
    DRV_PWM_DITHER_IN_32BITS,
} drv_pwm_dither_in_width_t;

typedef enum {
    DRV_PWM_FORCE_OUT_DISABLE = 0,
    DRV_PWM_FORCE_OUT_HIGH,
    DRV_PWM_FORCE_OUT_LOW,
} drv_pwm_force_out_t;

typedef enum {
    DRV_PWM_INT_SRC_CMP_EVENT = 0,
    DRV_PWM_INT_SRC_CNT_G0_OVF,
    DRV_PWM_INT_SRC_FIFO_UNDERRUN,
} drv_pwm_int_src_t;

typedef enum {
    DRV_PWM_PCM_PLAY_IDLE_STATUS = 0,
    DRV_PWM_PCM_PLAY_PROC_STATUS,
    DRV_PWM_PCM_PLAY_TERMINATE_STATUS,
    DRV_PWM_PCM_PLAY_STOP_STATUS,
} drv_pwm_pcm_play_status_t;

/*! ****************************************************************************
* .Structure[global]
********************************************************************************/
typedef void (*drv_pwm_pcm_play_complete_cbk)(void *arg);

/* audio pcm pwm configure structure */
typedef struct {
    uint32_t sample_freq;   //pcm data sample frequency
    uint32_t data_bits;   //pcm data format
    drv_pwm_pcm_drive_mode_t drive_mode;
    drv_pwm_pcm_play_complete_cbk play_complete_cbk;
} drv_pwm_pcm_cfg_t;

/* simple pwm configure structure */
typedef struct {
    drv_pwm_src_clk_t clk_src;  //pwm source clock select
    uint16_t clk_div;   //pwm source clock div number
} drv_pwm_clk_cfg_t;

typedef struct {
    uint8_t duty;   //simple pwm duty, unit is %
    drv_pwm_phase_polarity_t phase;  //simple pwm phase polarity
} drv_pwm_simple_cmp_cfg_t;

typedef struct {
    uint32_t freq;  //simple pwm frequency
    drv_pwm_group_num_t grp_num;    //group number
    drv_pwm_single_mode_t single_mode; //single one shot mode
    drv_pwm_align_mode_t align_mode;   //pwm wave edge align or center align mode
    drv_pwm_simple_cmp_cfg_t cmp_cfg[DRV_PWM_CHN_TOTAL];    //sub-channel compare configure
} drv_pwm_simple_cfg_t;

/* simple pwm context */
typedef struct {
    uint32_t ovf_val;
    drv_pwm_align_mode_t align_mode;
    drv_pwm_phase_polarity_t phase[DRV_PWM_CHN_TOTAL];
} drv_pwm_simple_context_t;

typedef struct {
    struct dma_chan *tx_chan;
    struct dma_desc *tx_desc;
    uint8_t* data_addr;
    uint32_t data_size;
    uint8_t* data_addr_shadow;
    uint32_t data_size_shadow;
    drv_pwm_pcm_play_status_t play_status;
    drv_pwm_pcm_play_complete_cbk play_complete_cbk;
    uint32_t data_bits;
} drv_pwm_pcm_context_t;

typedef void (*drv_pwm_int_func_cbk)(void *arg);

typedef struct {
    drv_pwm_int_func_cbk cmp_event_cbk;
    drv_pwm_int_func_cbk cnt_g0_ovf_cbk;
    drv_pwm_int_func_cbk fifo_underrun_cbk;
} drv_pwm_int_cbk_t;

#define DRV_PWM_PCM_TABLE_DATA_FORMAT_NUM 2
#define DRV_PWM_PCM_TABLE_SAMPLE_FREQ_NUM 8

typedef struct {
    uint32_t sample_freq;
    uint16_t clk_div;
    uint32_t ovf_val;
    uint8_t  dither_en;
    uint8_t  dither_clip_rslt;
    uint8_t  dither_drop;
} drv_pwm_pcm_tune_para_t;

typedef struct {
    uint16_t clk_div;
    uint32_t ovf_val;
    uint8_t  dither_en;
    uint8_t  dither_clip_rslt;
    uint8_t  dither_drop;
} drv_pwm_pcm_tune_config_t;

typedef struct {
    uint32_t data_bits;
    drv_pwm_pcm_tune_para_t tune_para[DRV_PWM_PCM_TABLE_SAMPLE_FREQ_NUM];
} drv_pwm_pcm_tune_table_t;


/*! ****************************************************************************
* .Function[global]
********************************************************************************/
void drv_pwm_simple_init(sdrv_pwm_t* pwm_dev, drv_pwm_simple_cfg_t* pwm_cfg, drv_pwm_simple_context_t* pwm_simple_ctx);
void drv_pwm_simple_duty_set(sdrv_pwm_t* pwm_dev, drv_pwm_chn_t chn, drv_pwm_simple_context_t* ctx, uint8_t duty);
void drv_pwm_pcm_init(sdrv_pwm_t* pwm_dev, drv_pwm_pcm_cfg_t* pcm_cfg);
void drv_pwm_cmp_out_start(sdrv_pwm_t* pwm_dev);
void drv_pwm_cmp_out_stop(sdrv_pwm_t* pwm_dev);
void drv_pwm_cmp_dma_enable(sdrv_pwm_t* pwm_dev);
void drv_pwm_cmp_dma_disable(sdrv_pwm_t* pwm_dev);
void drv_pwm_force_output(sdrv_pwm_t* pwm_dev, drv_pwm_chn_t chn, drv_pwm_force_out_t force_out);
void drv_pwm_int_enable(sdrv_pwm_t* pwm_dev, drv_pwm_int_src_t int_src);
void drv_pwm_int_disable(sdrv_pwm_t* pwm_dev, drv_pwm_int_src_t int_src);
enum handler_return drv_pwm_irq_handle(sdrv_pwm_t* pwm_dev, drv_pwm_int_cbk_t *int_cbk, void *handle);

#ifdef __cplusplus
}
#endif

#endif


