//*****************************************************************************
//
// pwm_hal.h - Prototypes for the pwm hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __PWM_HAL_H__
#define __PWM_HAL_H__
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

#include <__regs_base.h>
#include <kernel/mutex.h>
#include <platform/interrupts.h>
#include <res.h>
#include <chip_res.h>
#if ENABLE_SD_PWM
#include "pwm_drv.h"
#endif

#define SDV_PWM_DRIVER_VERSION (MAKE_VERSION(1, 0, 0)) /*!< Version 1.0.0 */
#define MAKE_VERSION(major, minor, bugfix) (((major) << 16) | ((minor) << 8) | (bugfix))
#define DEFAULT_PWM_MAX_NUM  8

// Check the arguments.
#define HAL_PWM_ASSERT_PARAMETER(handle)  \
if(handle == NULL){ \
    LTRACEF("parameter error handle:%p\n", handle);    \
    return false;   \
}   \

/**
 *****************************************************************************
 ** \brief macro define
 *****************************************************************************/
#define HAL_PWM_IRQ_NUM_INVALID 0xFFFFFFFF
#define HAL_PWM_DEBUG_LEVEL ALWAYS

/**
 *****************************************************************************
 ** \brief enum define
 *****************************************************************************/
typedef enum {
    HAL_PWM_CHN_A = 0,
    HAL_PWM_CHN_B,
    HAL_PWM_CHN_C,
    HAL_PWM_CHN_D,
    HAL_PWM_CHN_TOTAL
} hal_pwm_chn_t;

typedef enum {
    HAL_PWM_SRC_CLK_HF = 0,
    HAL_PWM_SRC_CLK_AHF,
    HAL_PWM_SRC_CLK_RESERVE,
    HAL_PWM_SRC_CLK_EXT,
} hal_pwm_src_clk_t;

typedef enum {
    HAL_PWM_CONTINUE_CMP = 0,
    HAL_PWM_ONE_SHOT_CMP,
} hal_pwm_single_mode_t;

typedef enum {
    HAL_PWM_EDGE_ALIGN_MODE = 0,
    HAL_PWM_CENTER_ALIGN_MODE,
} hal_pwm_align_mode_t;

typedef enum {
    HAL_PWM_PHASE_POLARITY_POS = 0,
    HAL_PWM_PHASE_POLARITY_NEG,
} hal_pwm_phase_polarity_t;

typedef enum {
    HAL_PWM_PCM_DRIVE_MONO_CHANNEL = 0,
    HAL_PWM_PCM_DRIVE_DUAL_CHANNEL,
    HAL_PWM_PCM_DRIVE_DUAL_H_BRIDGE,
    HAL_PWM_PCM_DRIVE_FOUR_H_BRIDGE,
} hal_pwm_pcm_drive_mode_t;

typedef enum {
    HAL_PWM_CHN_A_WORK = 0,
    HAL_PWM_CHN_A_B_WORK,
    HAL_PWM_CHN_A_B_C_D_WORK,
} hal_pwm_group_num_t;

typedef enum {
    HAL_PWM_FORCE_OUT_DISABLE = 0,
    HAL_PWM_FORCE_OUT_HIGH,
    HAL_PWM_FORCE_OUT_LOW,
} hal_pwm_force_out_t;

typedef enum {
    HAL_PWM_INT_SRC_CMP_EVENT = 0,
    HAL_PWM_INT_SRC_CNT_G0_OVF,
    HAL_PWM_INT_SRC_FIFO_UNDERRUN,
} hal_pwm_int_src_t;

/**
 *****************************************************************************
 ** \brief data type define
 *****************************************************************************/
typedef void (*hal_pwm_pcm_play_complete_cbk)(void *arg);

/* audio pcm pwm configure structure */
typedef struct {
    uint32_t sample_freq;   //pcm data sample frequency
    uint32_t data_bits;     //pcm data format
    hal_pwm_pcm_drive_mode_t drive_mode;
    hal_pwm_pcm_play_complete_cbk play_complete_cbk;
} hal_pwm_pcm_cfg_t;

/* simple pwm configure structure */
typedef struct {
    hal_pwm_src_clk_t clk_src;  //pwm source clock select
    uint16_t clk_div;   //pwm source clock div number
} hal_pwm_clk_cfg_t;

typedef struct {
    uint8_t duty;   //simple pwm duty, unit is %
    hal_pwm_phase_polarity_t phase;  //simple pwm phase polarity
} hal_pwm_simple_cmp_cfg_t;

typedef struct {
    uint32_t freq;  //simple pwm frequency
    hal_pwm_group_num_t grp_num;    //group number
    hal_pwm_single_mode_t single_mode; //single one shot mode
    hal_pwm_align_mode_t align_mode;   //edge align or center align mode
    hal_pwm_simple_cmp_cfg_t cmp_cfg[HAL_PWM_CHN_TOTAL];    //sub-channel compare configure
} hal_pwm_simple_cfg_t;

typedef void (*hal_pwm_int_func_cbk)(void *arg);

typedef struct {
    bool occupied;
    uint32_t irq_num;   //pwm irq number
    uint32_t phy_num;
    uint32_t dma_chan_type;
#if ENABLE_SD_PWM
    sdrv_pwm_t *pwmc;   //pwm peripheral base address
    drv_pwm_simple_context_t pwm_simple_ctx;  //simple pwm context
    drv_pwm_pcm_context_t pwm_pcm_ctx;  //audio pcm pwm context
    drv_pwm_int_cbk_t pwm_int_cbk;  //pwm interrupt callback function
#endif
} pwm_instance_t;

/**
 *****************************************************************************
 ** \brief uart function interface descriptor.
 *****************************************************************************/

/******************************************************************************
 ** \brief create the handle of pwm
 **
 ** \param [in]   res_glb_idx    global resource index
 ** \param [out]  handle         pointer of the handle created
 *****************************************************************************/
bool hal_pwm_creat_handle(void **handle, uint32_t res_glb_idx);

/******************************************************************************
 ** \brief release the handle of pwm
 **
 ** \param [in]  handle         pointer of the handle created
 *****************************************************************************/
bool hal_pwm_release_handle(void *handle);

/******************************************************************************
 ** \brief simple pwm init
 **
 ** \param [in]   handle         pointer of the handle created
 ** \param [in]   pwm_cfg        simple pwm config parameter
 *****************************************************************************/
void hal_pwm_simple_init(void *handle, hal_pwm_simple_cfg_t* pwm_cfg);

/******************************************************************************
 ** \brief simple pwm start
 **
 ** \param [in]   handle         pointer of the handle created
 *****************************************************************************/
void hal_pwm_simple_start(void *handle);

/******************************************************************************
 ** \brief simple pwm stop compare
 **
 ** \param [in]   handle         pointer of the handle created
 *****************************************************************************/
void hal_pwm_simple_stop(void *handle);

/******************************************************************************
 ** \brief simple pwm duty set
 **
 ** \param [in]   handle         pointer of the handle created
 ** \param [in]   chn            the channel of pwm
 ** \param [in]   duty           pwm duty set, unit is %
 *****************************************************************************/
void hal_pwm_simple_duty_set(void *handle, hal_pwm_chn_t chn, uint8_t duty);

/******************************************************************************
 ** \brief audio pcm pwm init
 **
 ** \param [in]   handle         pointer of the handle created
 ** \param [in]   pcm_cfg        pointer of pcm pwm config parameter
 *****************************************************************************/
void hal_pwm_pcm_init(void *handle, hal_pwm_pcm_cfg_t* pcm_cfg);

/******************************************************************************
 ** \brief audio pcm pwm play start
 **
 ** \param [in]   handle         pointer of the handle created
 ** \param [in]   data_addr      pointer of pcm data
 ** \param [in]   data_size      the size of pcm data
 *****************************************************************************/
void hal_pwm_pcm_play_start(void *handle, uint8_t* data_addr, uint32_t data_size);

/******************************************************************************
 ** \brief pause the current pcm play
 **
 ** \param [in]   handle         pointer of the handle created
 *****************************************************************************/
void hal_pwm_pcm_play_pause(void *handle);

/******************************************************************************
 ** \brief resume the pcm paused before
 **
 ** \param [in]   handle         pointer of the handle created
 *****************************************************************************/
void hal_pwm_pcm_play_resume(void *handle);

/******************************************************************************
 ** \brief update the pcm pwm config
 **
 ** \param [in]   handle         pointer of the handle created
 ** \param [in]   pcm_cfg        pointer of pcm pwm config parameter
 *****************************************************************************/
void hal_pwm_pcm_cfg_update(void *handle, hal_pwm_pcm_cfg_t* pcm_cfg);

/******************************************************************************
 ** \brief force pwm output
 **
 ** \param [in]   handle         pointer of the handle created
 ** \param [in]   chn            the channel of pwm
 ** \param [in]   force_out      0: disable force output,
 **                              1: force output high
 **                              2: force output low
 *****************************************************************************/
void hal_pwm_force_output(void *handle, hal_pwm_chn_t chn, hal_pwm_force_out_t force_out);

/******************************************************************************
 ** \brief enable the interrupt source
 **
 ** \param [in]   handle         pointer of the handle created
 ** \param [in]   int_src        interrupt source
 *****************************************************************************/
void hal_pwm_int_enable(void *handle, hal_pwm_int_src_t int_src);

/******************************************************************************
 ** \brief disable the interrupt source
 **
 ** \param [in]   handle         pointer of the handle created
 ** \param [in]   int_src        interrupt source
 *****************************************************************************/
void hal_pwm_int_disable(void *handle, hal_pwm_int_src_t int_src);

/******************************************************************************
 ** \brief pwm interrupt handle function
 **
 ** \param [in]   handle         pointer of the handle created
 *****************************************************************************/
enum handler_return hal_pwm_irq_handle(void *handle);

/******************************************************************************
 ** \brief register the interrupt callback function
 **
 ** \param [in]   handle         pointer of the handle created
 ** \param [in]   int_src        interrupt source
 ** \param [in]   cbk            pointer of interrupt callback function
 *****************************************************************************/
void hal_pwm_int_cbk_register(void *handle, hal_pwm_int_src_t int_src, hal_pwm_int_func_cbk cbk);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif

