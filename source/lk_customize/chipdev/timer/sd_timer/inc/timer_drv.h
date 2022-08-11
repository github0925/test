/*
* timer_drv.h
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: semidrive timer headfile
*
* Revision History:
* -----------------
* 011, 10/10/2019 wang yongjun implement this
*/

#ifndef __TIMER_DRV_H__
#define __TIMER_DRV_H__

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

#include <sys/types.h>
#include <stdbool.h>
#include "timer_reg.h"

/*! ****************************************************************************
* .ENUM[global]
********************************************************************************/
typedef enum {
    TIMER_DRV_G0 = 0,
    TIMER_DRV_G1,
    TIMER_DRV_LOCAL_A,
    TIMER_DRV_LOCAL_B,
    TIMER_DRV_LOCAL_C,
    TIMER_DRV_LOCAL_D,

    TIMER_DRV_CONTER_TOTAL
} timer_drv_sub_t;

typedef enum {
    TIMER_DRV_OVF_G0 = 0,
    TIMER_DRV_OVF_G1,

    TIMER_DRV_OVF_CH_TOTAL
} timer_drv_ovf_ch_t;

typedef enum {
    TIMER_DRV_FUNC_CH_A = 0,
    TIMER_DRV_FUNC_CH_B,
    TIMER_DRV_FUNC_CH_C,
    TIMER_DRV_FUNC_CH_D,

    TIMER_DRV_FUNC_CH_TOTAL
} timer_drv_func_ch_t;

typedef enum {
    TIMER_DRV_FUNC_TYPE_NONE = 0,
    TIMER_DRV_FUNC_TYPE_CPT,
    TIMER_DRV_FUNC_TYPE_CMP,

    TIMER_DRV_FUNC_TYPE_TOTAL
} timer_drv_func_type_t;

typedef enum {
    TIMER_DRV_CMP_POS_PULSE = 0,
    TIMER_DRV_CMP_NEG_PULSE,
    TIMER_DRV_CMP_SIG_TOGGLE,
    TIMER_DRV_CMP_LEVEL_HIGH,
    TIMER_DRV_CMP_LEVEL_LOW,
} timer_drv_cmp_out_mode_t;

typedef enum {
    TIMER_DRV_DMA_SEL_CMP = 0,
    TIMER_DRV_DMA_SEL_CPT,
    TIMER_DRV_DMA_SEL_OVERFLOW,
} timer_drv_dma_select_t;

typedef enum {
    TIMER_DRV_CMP_CNT_G0 = 0,
    TIMER_DRV_CMP_CNT_LOCAL,
} timer_drv_cmp_cntr_t;

typedef enum {
    TIMER_DRV_CPT_CNT_G0 = 0,
    TIMER_DRV_CPT_CNT_G0G1,
    TIMER_DRV_CPT_CNT_LOCAL,
} timer_drv_cpt_cntr_t;

typedef enum {
    TIMER_DRV_CPT_RISING_EDGE = 0,
    TIMER_DRV_CPT_FALLING_EDGE,
    TIMER_DRV_CPT_TOGGLE_EDGE,
} timer_drv_cpt_edge_t;

typedef enum {
    TIMER_DRV_CPT_A_INT_SRC = 0,
    TIMER_DRV_CPT_B_INT_SRC,
    TIMER_DRV_CPT_C_INT_SRC,
    TIMER_DRV_CPT_D_INT_SRC,
    TIMER_DRV_CMP_A_INT_SRC,
    TIMER_DRV_CMP_B_INT_SRC,
    TIMER_DRV_CMP_C_INT_SRC,
    TIMER_DRV_CMP_D_INT_SRC,

    TIMER_DRV_CNT_G0_OVF_INT_SRC,
    TIMER_DRV_CNT_G1_OVF_INT_SRC,
    TIMER_DRV_CNT_LA_OVF_INT_SRC,
    TIMER_DRV_CNT_LB_OVF_INT_SRC,
    TIMER_DRV_CNT_LC_OVF_INT_SRC,
    TIMER_DRV_CNT_LD_OVF_INT_SRC,
    TIMER_DRV_CNT_LA_UNF_INT_SRC,
    TIMER_DRV_RESVD0,

    TIMER_DRV_FIFO_A_UNDERRUN_INT_SRC,
    TIMER_DRV_FIFO_B_UNDERRUN_INT_SRC,
    TIMER_DRV_FIFO_C_UNDERRUN_INT_SRC,
    TIMER_DRV_FIFO_D_UNDERRUN_INT_SRC,
    TIMER_DRV_FIFO_A_OVERRUN_INT_SRC,
    TIMER_DRV_FIFO_B_OVERRUN_INT_SRC,
    TIMER_DRV_FIFO_C_OVERRUN_INT_SRC,
    TIMER_DRV_FIFO_D_OVERRUN_INT_SRC,
} timer_drv_int_src_t;

typedef enum {
    TIMER_DRV_SEL_HF_CLK = 0,
    TIMER_DRV_SEL_AHF_CLK,
    TIMER_DRV_SEL_LF_CLK,
    TIMER_DRV_SEL_LP_CLK,
} timer_drv_clk_sel_t;

/*! ****************************************************************************
* .DATA_TYPES[global]
********************************************************************************/
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
} timer_drv_func_cpt_t;

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
} timer_drv_func_cmp_t;

typedef union {
    timer_drv_func_cpt_t cpt_cfg;
    timer_drv_func_cmp_t cmp_cfg;
} timer_drv_sub_func_t;

typedef struct {
    uint8_t dma_enable;
    uint8_t dma_sel;
    uint8_t fifo_wml;
} timer_drv_dma_ctrl_t;

typedef struct {
    timer_drv_func_type_t func_type;
    timer_drv_sub_func_t sub_func;
    timer_drv_dma_ctrl_t dma_ctrl;
} timer_drv_func_t;

typedef enum handler_return (*timer_drv_int_cbk)(void);

typedef struct {
    bool periodic[TIMER_DRV_CONTER_TOTAL];
    timer_drv_int_cbk global_ovf_cbk[TIMER_DRV_OVF_CH_TOTAL];
    timer_drv_int_cbk local_ovf_cbk[TIMER_DRV_FUNC_CH_TOTAL];
    timer_drv_int_cbk local_cmp_cbk[TIMER_DRV_FUNC_CH_TOTAL];
    timer_drv_int_cbk local_cpt_cbk[TIMER_DRV_FUNC_CH_TOTAL];
    timer_drv_int_cbk local_overrun_cbk[TIMER_DRV_FUNC_CH_TOTAL];
    timer_drv_int_cbk local_underrun_cbk[TIMER_DRV_FUNC_CH_TOTAL];
} timer_drv_context_t;

/*! ****************************************************************************
* .FUNCTION_PROTOTYPES[global]
********************************************************************************/
void timer_drv_cntr_reset(sdrv_timer_t *timer, timer_drv_sub_t sub_timer,
                          bool wait_rld);
void timer_drv_cntr_set(sdrv_timer_t *timer, timer_drv_sub_t sub_timer,
                        uint32_t val);
uint32_t timer_drv_cntr_get(sdrv_timer_t *timer,
                            timer_drv_sub_t sub_timer);
void timer_drv_ovf_set(sdrv_timer_t *timer, timer_drv_sub_t sub_timer,
                       uint32_t val);
void timer_drv_cascade_set(sdrv_timer_t *timer, bool cascade);
void timer_drv_clk_init(sdrv_timer_t *timer, uint32_t clk_sel,
                        uint32_t clk_div);
void timer_drv_fifo_reset(sdrv_timer_t *timer,
                          timer_drv_func_ch_t func_ch);
void timer_drv_func_cha_dma_init(sdrv_timer_t *timer, uint32_t dma_sel,
                                 uint32_t fifo_wml);
void timer_drv_func_chb_dma_init(sdrv_timer_t *timer, uint32_t dma_sel,
                                 uint32_t fifo_wml);
void timer_drv_func_chc_dma_init(sdrv_timer_t *timer, uint32_t dma_sel,
                                 uint32_t fifo_wml);
void timer_drv_func_chd_dma_init(sdrv_timer_t *timer, uint32_t dma_sel,
                                 uint32_t fifo_wml);
void timer_drv_func_cpt_enable(sdrv_timer_t *timer,
                               timer_drv_func_ch_t func_ch);
void timer_drv_func_cpt_disable(sdrv_timer_t *timer,
                                timer_drv_func_ch_t func_ch);
void timer_drv_func_cmp_enable(sdrv_timer_t *timer,
                               timer_drv_func_ch_t func_ch);
void timer_drv_func_cmp_disable(sdrv_timer_t *timer,
                                timer_drv_func_ch_t func_ch);
void timer_drv_func_init(sdrv_timer_t *timer, timer_drv_func_ch_t func_ch,
                         timer_drv_func_t *func_cfg);
void timer_drv_int_sta_enable(sdrv_timer_t *timer,
                              timer_drv_int_src_t offset);
void timer_drv_int_sta_disable(sdrv_timer_t *timer,
                               timer_drv_int_src_t offset);
void timer_drv_int_sig_enable(sdrv_timer_t *timer,
                              timer_drv_int_src_t offset);
void timer_drv_int_sig_disable(sdrv_timer_t *timer,
                               timer_drv_int_src_t offset);
void timer_drv_cnt_start(sdrv_timer_t *timer, timer_drv_sub_t sub_timer);
void timer_drv_cnt_stop(sdrv_timer_t *timer, timer_drv_sub_t sub_timer);
void timer_drv_cmp_force_out(sdrv_timer_t *timer,
                             timer_drv_func_ch_t func_ch, bool enable, bool level);
uint32_t timer_drv_cpt_value_get(sdrv_timer_t *timer,
                                 timer_drv_func_ch_t func_ch);
void timer_drv_cmp_value_set(sdrv_timer_t *timer,
                             timer_drv_func_ch_t func_ch, uint32_t value0, uint32_t value1);
void timer_drv_sse_cpt_init(sdrv_timer_t *timer,
                            timer_drv_func_ch_t cpt_ch, bool cpt_en, timer_drv_func_ch_t cmp_ch);
void timer_drv_sse_cmp_init(sdrv_timer_t *timer,
                            timer_drv_func_ch_t cmp_ch, bool cmp_en);
void timer_drv_int_sta_clear(sdrv_timer_t *timer,
                             timer_drv_int_src_t offset);
enum handler_return timer_drv_ovf_irq_handle(sdrv_timer_t *timer,
        timer_drv_context_t *drv_context);
enum handler_return timer_drv_func_irq_handle(sdrv_timer_t *timer,
        timer_drv_context_t *drv_context);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif
#endif
