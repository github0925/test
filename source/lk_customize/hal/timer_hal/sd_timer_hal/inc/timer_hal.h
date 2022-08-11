/*
* timer_hal.h
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: semidrive timer HAL headfile
*
* Revision History:
* -----------------
* 011, 01/24/2019 wang yongjun implement this
*/

#ifndef __HAL_TIMER_H__
#define __HAL_TIMER_H__
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
#if ENABLE_SD_TIMER
#include "timer_drv.h"
#endif

/*! ****************************************************************************
* .DEFINE
*******************************************************************************/
#define SDV_TIMER_DRIVER_VERSION (MAKE_VERSION(1, 0, 0)) /*!< Version 1.0.0 */
#define MAKE_VERSION(major, minor, bugfix) (((major) << 16) | ((minor) << 8) | (bugfix))
#define DEFAULT_TIMER_MAX_NUM  8

// Check the arguments.
#define HAL_ASSERT_PARAMETER(handle)  \
if(handle == NULL){ \
    LTRACEF("paramenter error handle:%p\n", handle);    \
    return false;   \
}   \

#define HAL_TIMER_INVALID_IRQ_NUM 0xFFFFFFFF


typedef enum {
    HAL_TIMER_RES_ERR_OCCUPIED = 0,
    HAL_TIMER_RES_ERR_NOT_FIND,
    HAL_TIMER_RES_OK,
} hal_timer_err_t;

/*! ****************************************************************************
* .DATA_TYPES[global]
********************************************************************************/
typedef enum {
#if ENABLE_SD_TIMER
    HAL_TIMER_G0 = TIMER_DRV_G0,
    HAL_TIMER_G1 = TIMER_DRV_G1,
    HAL_TIMER_LOCAL_A = TIMER_DRV_LOCAL_A,
    HAL_TIMER_LOCAL_B = TIMER_DRV_LOCAL_B,
    HAL_TIMER_LOCAL_C = TIMER_DRV_LOCAL_C,
    HAL_TIMER_LOCAL_D = TIMER_DRV_LOCAL_D,
#else
    HAL_TIMER_G0 = 0,
    HAL_TIMER_G1,
    HAL_TIMER_LOCAL_A,
    HAL_TIMER_LOCAL_B,
    HAL_TIMER_LOCAL_C,
    HAL_TIMER_LOCAL_D,
#endif
    HAL_TIMER_CONTER_TOTAL
} hal_timer_sub_t;

typedef enum {
#if ENABLE_SD_TIMER
    HAL_TIMER_OVF_G0 = TIMER_DRV_OVF_G0,
    HAL_TIMER_OVF_G1 = TIMER_DRV_OVF_G1,
#else
    HAL_TIMER_OVF_G0 = 0,
    HAL_TIMER_OVF_G1,
#endif
    HAL_TIMER_OVF_CH_TOTAL
} hal_timer_ovf_ch_t;

typedef enum {
#if ENABLE_SD_TIMER
    HAL_TIMER_FUNC_CH_A = TIMER_DRV_FUNC_CH_A,
    HAL_TIMER_FUNC_CH_B = TIMER_DRV_FUNC_CH_B,
    HAL_TIMER_FUNC_CH_C = TIMER_DRV_FUNC_CH_C,
    HAL_TIMER_FUNC_CH_D = TIMER_DRV_FUNC_CH_D,
#else
    HAL_TIMER_FUNC_CH_A = 0,
    HAL_TIMER_FUNC_CH_B,
    HAL_TIMER_FUNC_CH_C,
    HAL_TIMER_FUNC_CH_D,
#endif
    HAL_TIMER_FUNC_CH_TOTAL
} hal_timer_func_ch_t;

typedef enum {
#if ENABLE_SD_TIMER
    HAL_TIMER_FUNC_TYPE_NONE = TIMER_DRV_FUNC_TYPE_NONE,
    HAL_TIMER_FUNC_TYPE_CPT = TIMER_DRV_FUNC_TYPE_CPT,
    HAL_TIMER_FUNC_TYPE_CMP = TIMER_DRV_FUNC_TYPE_CMP,
#else
    HAL_TIMER_FUNC_TYPE_NONE = 0,
    HAL_TIMER_FUNC_TYPE_CPT,
    HAL_TIMER_FUNC_TYPE_CMP,
#endif
    HAL_TIMER_FUNC_TYPE_TOTAL
} hal_timer_func_type_t;

typedef enum {
#if ENABLE_SD_TIMER
    HAL_TIMER_CMP_POS_PULSE = TIMER_DRV_CMP_POS_PULSE,
    HAL_TIMER_CMP_NEG_PULSE = TIMER_DRV_CMP_NEG_PULSE,
    HAL_TIMER_CMP_SIG_TOGGLE = TIMER_DRV_CMP_SIG_TOGGLE,
    HAL_TIMER_CMP_LEVEL_HIGH = TIMER_DRV_CMP_LEVEL_HIGH,
    HAL_TIMER_CMP_LEVEL_LOW = TIMER_DRV_CMP_LEVEL_LOW,
#else
    HAL_TIMER_CMP_POS_PULSE = 0,
    HAL_TIMER_CMP_NEG_PULSE,
    HAL_TIMER_CMP_SIG_TOGGLE,
    HAL_TIMER_CMP_LEVEL_HIGH,
    HAL_TIMER_CMP_LEVEL_LOW,
#endif
    HAL_TIMER_CMP_OUT_MODE_TOTAL
} hal_timer_cmp_out_mode_t;

typedef enum {
#if ENABLE_SD_TIMER
    HAL_TIMER_DMA_SEL_CMP = TIMER_DRV_DMA_SEL_CMP,
    HAL_TIMER_DMA_SEL_CPT = TIMER_DRV_DMA_SEL_CPT,
    HAL_TIMER_DMA_SEL_OVERFLOW = TIMER_DRV_DMA_SEL_OVERFLOW,
#else
    HAL_TIMER_DMA_SEL_CMP = 0,
    HAL_TIMER_DMA_SEL_CPT,
    HAL_TIMER_DMA_SEL_OVERFLOW,
#endif
} hal_timer_dma_select_t;

typedef enum {
#if ENABLE_SD_TIMER
    HAL_TIMER_CMP_CNT_G0 = TIMER_DRV_CMP_CNT_G0,
    HAL_TIMER_CMP_CNT_LOCAL = TIMER_DRV_CMP_CNT_LOCAL,
#else
    HAL_TIMER_CMP_CNT_G0 = 0,
    HAL_TIMER_CMP_CNT_LOCAL,
#endif
} hal_timer_cmp_cntr_t;

typedef enum {
#if ENABLE_SD_TIMER
    HAL_TIMER_CPT_CNT_G0 = TIMER_DRV_CPT_CNT_G0,
    HAL_TIMER_CPT_CNT_G0G1 = TIMER_DRV_CPT_CNT_G0G1,
    HAL_TIMER_CPT_CNT_LOCAL = TIMER_DRV_CPT_CNT_LOCAL,
#else
    HAL_TIMER_CPT_CNT_G0 = 0,
    HAL_TIMER_CPT_CNT_G0G1,
    HAL_TIMER_CPT_CNT_LOCAL,
#endif
} hal_timer_cpt_cntr_t;

typedef enum {
#if ENABLE_SD_TIMER
    HAL_TIMER_CPT_RISING_EDGE = TIMER_DRV_CPT_RISING_EDGE,
    HAL_TIMER_CPT_FALLING_EDGE = TIMER_DRV_CPT_FALLING_EDGE,
    HAL_TIMER_CPT_TOGGLE_EDGE = TIMER_DRV_CPT_TOGGLE_EDGE,
#else
    HAL_TIMER_CPT_RISING_EDGE = 0,
    HAL_TIMER_CPT_FALLING_EDGE,
    HAL_TIMER_CPT_TOGGLE_EDGE,
#endif
} hal_timer_cpt_edge_t;

typedef enum {
#if ENABLE_SD_TIMER
    HAL_TIMER_CPT_A_INT_SRC = TIMER_DRV_CPT_A_INT_SRC,
    HAL_TIMER_CPT_B_INT_SRC = TIMER_DRV_CPT_B_INT_SRC,
    HAL_TIMER_CPT_C_INT_SRC = TIMER_DRV_CPT_C_INT_SRC,
    HAL_TIMER_CPT_D_INT_SRC = TIMER_DRV_CPT_D_INT_SRC,
    HAL_TIMER_CMP_A_INT_SRC = TIMER_DRV_CMP_A_INT_SRC,
    HAL_TIMER_CMP_B_INT_SRC = TIMER_DRV_CMP_B_INT_SRC,
    HAL_TIMER_CMP_C_INT_SRC = TIMER_DRV_CMP_C_INT_SRC,
    HAL_TIMER_CMP_D_INT_SRC = TIMER_DRV_CMP_D_INT_SRC,
    HAL_TIMER_CNT_G0_OVF_INT_SRC = TIMER_DRV_CNT_G0_OVF_INT_SRC,
    HAL_TIMER_CNT_G1_OVF_INT_SRC = TIMER_DRV_CNT_G1_OVF_INT_SRC,
    HAL_TIMER_CNT_LA_OVF_INT_SRC = TIMER_DRV_CNT_LA_OVF_INT_SRC,
    HAL_TIMER_CNT_LB_OVF_INT_SRC = TIMER_DRV_CNT_LB_OVF_INT_SRC,
    HAL_TIMER_CNT_LC_OVF_INT_SRC = TIMER_DRV_CNT_LC_OVF_INT_SRC,
    HAL_TIMER_CNT_LD_OVF_INT_SRC = TIMER_DRV_CNT_LD_OVF_INT_SRC,
    HAL_TIMER_CNT_LA_UNF_INT_SRC = TIMER_DRV_CNT_LA_UNF_INT_SRC,
    HAL_TIMER_RESVD0,
    HAL_TIMER_FIFO_A_UNDERRUN_INT_SRC = TIMER_DRV_FIFO_A_UNDERRUN_INT_SRC,
    HAL_TIMER_FIFO_B_UNDERRUN_INT_SRC = TIMER_DRV_FIFO_B_UNDERRUN_INT_SRC,
    HAL_TIMER_FIFO_C_UNDERRUN_INT_SRC = TIMER_DRV_FIFO_C_UNDERRUN_INT_SRC,
    HAL_TIMER_FIFO_D_UNDERRUN_INT_SRC = TIMER_DRV_FIFO_D_UNDERRUN_INT_SRC,
    HAL_TIMER_FIFO_A_OVERRUN_INT_SRC = TIMER_DRV_FIFO_A_OVERRUN_INT_SRC,
    HAL_TIMER_FIFO_B_OVERRUN_INT_SRC = TIMER_DRV_FIFO_B_OVERRUN_INT_SRC,
    HAL_TIMER_FIFO_C_OVERRUN_INT_SRC = TIMER_DRV_FIFO_C_OVERRUN_INT_SRC,
    HAL_TIMER_FIFO_D_OVERRUN_INT_SRC = TIMER_DRV_FIFO_D_OVERRUN_INT_SRC,
#else
    HAL_TIMER_CPT_A_INT_SRC = 0,
    HAL_TIMER_CPT_B_INT_SRC,
    HAL_TIMER_CPT_C_INT_SRC,
    HAL_TIMER_CPT_D_INT_SRC,
    HAL_TIMER_CMP_A_INT_SRC,
    HAL_TIMER_CMP_B_INT_SRC,
    HAL_TIMER_CMP_C_INT_SRC,
    HAL_TIMER_CMP_D_INT_SRC,
    HAL_TIMER_CNT_G0_OVF_INT_SRC,
    HAL_TIMER_CNT_G1_OVF_INT_SRC,
    HAL_TIMER_CNT_LA_OVF_INT_SRC,
    HAL_TIMER_CNT_LB_OVF_INT_SRC,
    HAL_TIMER_CNT_LC_OVF_INT_SRC,
    HAL_TIMER_CNT_LD_OVF_INT_SRC,
    HAL_TIMER_CNT_LA_UNF_INT_SRC,
    HAL_TIMER_RESVD0,
    HAL_TIMER_FIFO_A_UNDERRUN_INT_SRCT,
    HAL_TIMER_FIFO_B_UNDERRUN_INT_SRC,
    HAL_TIMER_FIFO_C_UNDERRUN_INT_SRC,
    HAL_TIMER_FIFO_D_UNDERRUN_INT_SRC,
    HAL_TIMER_FIFO_A_OVERRUN_INT_SRC,
    HAL_TIMER_FIFO_B_OVERRUN_INT_SRCT,
    HAL_TIMER_FIFO_C_OVERRUN_INT_SRC,
    HAL_TIMER_FIFO_D_OVERRUN_INT_SRC,
#endif
} hal_timer_int_src_t;

typedef enum {
#if ENABLE_SD_TIMER
    HAL_TIMER_SEL_HF_CLK = TIMER_DRV_SEL_HF_CLK,
    HAL_TIMER_SEL_AHF_CLK = TIMER_DRV_SEL_AHF_CLK,
    HAL_TIMER_SEL_LF_CLK = TIMER_DRV_SEL_LF_CLK,
    HAL_TIMER_SEL_LP_CLK = TIMER_DRV_SEL_LP_CLK,
#else
    HAL_TIMER_SEL_HF_CLK = 0,
    HAL_TIMER_SEL_AHF_CLK,
    HAL_TIMER_SEL_LF_CLK,
    HAL_TIMER_SEL_LP_CLK,
#endif
} hal_timer_clk_sel_t;

typedef struct {
    uint8_t cpt_cnt_sel;
    uint8_t trig_mode;
    uint8_t dual_cpt_mode;
    uint8_t single_mode;
    uint8_t filter_dis;
    uint8_t filter_width;
    uint8_t rsvd0;
    uint8_t rsvd1;
    uint32_t first_cpt_rst_en;
    uint32_t rsvd2;
} hal_timer_func_cpt_t;

typedef struct {
    uint8_t cmp_cnt_sel;
    uint8_t cmp0_out_mode;
    uint8_t cmp1_out_mode;
    uint8_t cmp0_pulse_width;
    uint8_t cmp1_pulse_width;
    uint8_t dual_cmp_mode;
    uint8_t single_mode;
    uint8_t cmp_rst_en;
    uint32_t cmp0_val;
    uint32_t cmp1_val;
} hal_timer_func_cmp_t;

typedef union {
    hal_timer_func_cpt_t cpt_cfg;
    hal_timer_func_cmp_t cmp_cfg;
} hal_timer_sub_func_t;

typedef struct {
    uint8_t dma_enable;
    uint8_t dma_sel;
    uint8_t fifo_wml;
} hal_timer_dma_ctrl_t;

typedef struct {
    hal_timer_func_type_t func_type;
    hal_timer_sub_func_t sub_func;
    hal_timer_dma_ctrl_t dma_ctrl;
} hal_timer_func_t;

typedef enum handler_return (*hal_timer_int_cbk)(void);

typedef struct {
    hal_timer_int_cbk global_ovf_cbk[HAL_TIMER_OVF_CH_TOTAL];
    hal_timer_int_cbk local_ovf_cbk[HAL_TIMER_FUNC_CH_TOTAL];
    hal_timer_int_cbk local_cmp_cbk[HAL_TIMER_FUNC_CH_TOTAL];
    hal_timer_int_cbk local_cpt_cbk[HAL_TIMER_FUNC_CH_TOTAL];
    hal_timer_int_cbk local_overrun_cbk[HAL_TIMER_FUNC_CH_TOTAL];
    hal_timer_int_cbk local_underrun_cbk[HAL_TIMER_FUNC_CH_TOTAL];
} hal_timer_int_cbk_t;

typedef struct {
    uint32_t clk_sel;
    uint32_t clk_frq;
    uint32_t clk_div;
    bool cascade;
} hal_timer_glb_cfg_t;

typedef struct {
    bool periodic;
    uint32_t cnt_val;
    uint32_t ovf_val;
} hal_timer_ovf_cfg_t;

typedef struct {
    hal_timer_func_t func;
} hal_timer_fun_cfg_t;

typedef struct {
    hal_timer_glb_cfg_t glb_cfg;
    hal_timer_ovf_cfg_t ovf_cfg[HAL_TIMER_CONTER_TOTAL];
    hal_timer_fun_cfg_t func_cfg[HAL_TIMER_FUNC_CH_TOTAL];
} hal_timer_cfg_t;

/**
 *****************************************************************************
 ** \brief uart instance information descriptor.
 *****************************************************************************/
typedef struct {
    bool occupied;
    uint32_t ovf_irq_num;   //timer ovf irq number
    uint32_t fun_irq_num;   //timer function irq number
    uint32_t cnt_per_ms;
    uint32_t cnt_per_us;
#if ENABLE_SD_TIMER
    sdrv_timer_t *timer;    //uart peripheral base address
    timer_drv_context_t drv_context;     //driver context
#endif
} timer_instance_t;

typedef struct {
    uint32_t addr;
    uint32_t ovf_irq_num;
    uint32_t fun_irq_num;
} hal_timer_addr_to_irq_t;

/*! ****************************************************************************
* .FUNCTION_PROTOTYPES[global]
********************************************************************************/
bool hal_timer_creat_handle(void **handle, uint32_t res_glb_idx);
bool hal_timer_release_handle(void *handle);
void hal_timer_global_init(void *handle, hal_timer_glb_cfg_t *cfg);
void hal_timer_ovf_init(void *handle, hal_timer_sub_t sub_cntr,
                        hal_timer_ovf_cfg_t *cfg);
void hal_timer_func_init(void *handle, hal_timer_func_ch_t sub_cntr,
                         hal_timer_fun_cfg_t *cfg);
void hal_timer_init(void *handle, hal_timer_cfg_t *cfg);
void hal_timer_cmp_force_out(void *handle, hal_timer_func_ch_t func_ch,
                             bool enable, bool level);
uint32_t hal_timer_timer_cpt_value_get(void *handle,
                                       hal_timer_func_ch_t func_ch);
void hal_timer_cmp_value_set(void *handle, hal_timer_func_ch_t func_ch,
                             uint32_t value0, uint32_t value1);
void hal_timer_func_cmp_enable(void *handle, hal_timer_func_ch_t func_ch);
void hal_timer_func_cmp_disable(void *handle, hal_timer_func_ch_t func_ch);
void hal_timer_func_cpt_enable(void *handle, hal_timer_func_ch_t func_ch);
void hal_timer_func_cpt_disable(void *handle, hal_timer_func_ch_t func_ch);
void hal_timer_int_src_enable(void *handle, hal_timer_int_src_t int_src);
void hal_timer_int_src_disable(void *handle, hal_timer_int_src_t int_src);
void hal_timer_int_sta_clear(void *handle, hal_timer_int_src_t int_src);
void hal_timer_int_cbk_register(void *handle, hal_timer_int_src_t int_src,
                                hal_timer_int_cbk cbk);
uint64_t hal_timer_glb_cntr_get(void *handle);
uint64_t hal_timer_ms_to_cntr(void *handle, uint32_t ms);
uint32_t hal_timer_cpt_get_fifo_items_num(void *handle, hal_timer_func_ch_t func_ch);
uint32_t hal_timer_cntr_to_ms(void* handle, uint32_t cntr);
uint32_t hal_timer_cntr_to_us(void* handle, uint32_t cntr);
#ifdef __cplusplus
}
#endif
#endif
