/*
* timer_drv.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement semidrive timer driver
*
* Revision History:
* -----------------
* 011, 10/10/2019 wangyongjun implement this
*/

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

#include <trace.h>
#include <err.h>
#include <assert.h>
#include <platform/debug.h>
#include <sys/types.h>
#include <platform/interrupts.h>

#include "timer_drv.h"


/******************************************************************************
 ** \brief Reset the sub timer counter value to 0.
 **
 ** \param [in] timer      Pointer to the timer controller base address.
 ** \param [in] sub_timer  Sub counter index.
 ** \param [in] wait_rld   Whether wait for sub counter reset to 0.
 *****************************************************************************/
void timer_drv_cntr_reset(sdrv_timer_t *timer, timer_drv_sub_t sub_timer,
                          bool wait_rld)
{
    timer->cnt_config |= (1 << sub_timer);

    if (wait_rld) {
        while (timer->cnt_config & (1 << sub_timer));
    }
}

/******************************************************************************
 ** \brief Set the sub timer counter value.
 **
 ** \param [in] timer      Pointer to the timer controller base address.
 ** \param [in] sub_timer  Sub counter index.
 ** \param [in] val        Set value for sub counter.
 *****************************************************************************/
void timer_drv_cntr_set(sdrv_timer_t *timer, timer_drv_sub_t sub_timer,
                        uint32_t val)
{
    if (val == 0) {
        timer_drv_cntr_reset(timer, sub_timer, true);
    }
    else {
        if (sub_timer == TIMER_DRV_G0) {
            timer->cnt_g0_init = val;
        }
        else if (sub_timer == TIMER_DRV_G1) {
            timer->cnt_g1_init = val;
        }
        else if (sub_timer == TIMER_DRV_LOCAL_A) {
            timer->cnt_local_a_init = val;
        }
        else if (sub_timer == TIMER_DRV_LOCAL_B) {
            timer->cnt_local_b_init = val;
        }
        else if (sub_timer == TIMER_DRV_LOCAL_C) {
            timer->cnt_local_c_init = val;
        }
        else if (sub_timer == TIMER_DRV_LOCAL_D) {
            timer->cnt_local_d_init = val;
        }
    }
}

/******************************************************************************
 ** \brief Get the sub timer counter value.
 **
 ** \param  [in] timer      Pointer to the timer controller base address.
 ** \param  [in] sub_timer  Sub counter index.
 ** \return [in] val        Value of sub counter.
 *****************************************************************************/
uint32_t timer_drv_cntr_get(sdrv_timer_t *timer, timer_drv_sub_t sub_timer)
{
    uint32_t val = 0;

    if (sub_timer == TIMER_DRV_G0) {
        val = timer->cnt_g0;
    }
    else if (sub_timer == TIMER_DRV_G1) {
        val = timer->cnt_g1;
    }
    else if (sub_timer == TIMER_DRV_LOCAL_A) {
        val = timer->cnt_local_a;
    }
    else if (sub_timer == TIMER_DRV_LOCAL_B) {
        val = timer->cnt_local_b;
    }
    else if (sub_timer == TIMER_DRV_LOCAL_C) {
        val = timer->cnt_local_c;
    }
    else if (sub_timer == TIMER_DRV_LOCAL_D) {
        val = timer->cnt_local_d;
    }

    return val;
}

/******************************************************************************
 ** \brief Set the sub timer over flow value.
 **
 ** \param [in] timer      Pointer to the timer register.
 ** \param [in] sub_timer  Sub counter index.
 ** \param [in] val        Set sub counter overflow value.
 *****************************************************************************/
void timer_drv_ovf_set(sdrv_timer_t *timer, timer_drv_sub_t sub_timer,
                       uint32_t val)
{
    if (sub_timer == TIMER_DRV_G0) {
        timer->cnt_g0_ovf = val;
    }
    else if (sub_timer == TIMER_DRV_G1) {
        timer->cnt_g1_ovf = val;
    }
    else if (sub_timer == TIMER_DRV_LOCAL_A) {
        timer->cnt_local_a_ovf = val;
    }
    else if (sub_timer == TIMER_DRV_LOCAL_B) {
        timer->cnt_local_b_ovf = val;
    }
    else if (sub_timer == TIMER_DRV_LOCAL_C) {
        timer->cnt_local_c_ovf = val;
    }
    else if (sub_timer == TIMER_DRV_LOCAL_D) {
        timer->cnt_local_d_ovf = val;
    }
}

/******************************************************************************
 ** \brief Set the sub timer G1 cascaded G0 or not.
 **
 ** \param [in] timer      Pointer to the timer register.
 ** \param [in] cascade    G1 cascaded G0 or not.
 *****************************************************************************/
void timer_drv_cascade_set(sdrv_timer_t *timer, bool cascade)
{
    if (cascade) {
        timer->cnt_config |= BM_CNT_CONFIG_CASCADE_MODE;
    }
    else {
        timer->cnt_config &= (~BM_CNT_CONFIG_CASCADE_MODE);
    }
}

/******************************************************************************
 ** \brief Set the timer clock source select and clock div.
 **
 ** \param [in] timer      Pointer to the timer register.
 ** \param [in] clk_cfg    Poniter to the clock configure.
 **                        src_clk_sel: 00 - High frequency clock, update to 400MHZ
 **                                     01 - Alternative high frequency clock, update to 400MHZ
 **                                     10 - Low frequency clock, typically from 24MHZ OSC
 **                                     11 - Low power clock, typically from low speed on chip RCOSC
 *****************************************************************************/
void timer_drv_clk_init(sdrv_timer_t *timer, uint32_t clk_sel,
                        uint32_t clk_div)
{
    u32 value = timer->tim_clk_config;

    value &= ~(FM_TIM_CLK_CONFIG_SRC_CLK_SEL | FM_TIM_CLK_CONFIG_DIV_NUM);
    value |= (FV_TIM_CLK_CONFIG_SRC_CLK_SEL(clk_sel) |
              FV_TIM_CLK_CONFIG_DIV_NUM(clk_div));

    timer->tim_clk_config = value;
}

/******************************************************************************
 ** \brief Reset the fifo of function chanel.
 **
 ** \param [in] timer      Pointer to the timer register.
 ** \param [in] func_ch    Function channel index.
 *****************************************************************************/
void timer_drv_fifo_reset(sdrv_timer_t          *timer,
                          timer_drv_func_ch_t func_ch)
{
    if (func_ch == TIMER_DRV_FUNC_CH_A) {
        timer->sw_rst |= BM_SW_RST_CHN_A_SOFT_RST;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_B) {
        timer->sw_rst |= BM_SW_RST_CHN_B_SOFT_RST;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_C) {
        timer->sw_rst |= BM_SW_RST_CHN_C_SOFT_RST;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_D) {
        timer->sw_rst |= BM_SW_RST_CHN_D_SOFT_RST;
    }
}

/******************************************************************************
 ** \brief Init the timer DMA control of function channel a.
 **
 ** \param [in] timer      Pointer to the timer register.
 ** \param [in] dma_ctrl   Poniter to the DMA control data.
 **                        fifo_entry: fifo entry address.
 **                        dma_sel: 1x - use for local timer over flow.
 **                                 01 - use for capture.
 **                                 00 - use for compare.
 **                        fifo_wml: water mark level of fifo.
 *****************************************************************************/
void timer_drv_func_cha_dma_init(sdrv_timer_t *timer, uint32_t dma_sel,
                                 uint32_t fifo_wml)
{
    u32 value = timer->dma_ctrl;

    value &= ~(FM_DMA_CTRL_CHN_A_WML | FM_DMA_CTRL_CHN_A_SEL |
               BM_DMA_CTRL_CHN_A_EN);
    value |= (FV_DMA_CTRL_CHN_A_WML(fifo_wml) | (FV_DMA_CTRL_CHN_A_SEL(
                  dma_sel)) | BM_DMA_CTRL_CHN_A_EN);

    timer->dma_ctrl = value;
}

/******************************************************************************
 ** \brief Init the timer DMA control of function channel b.
 **
 ** \param [in] timer      Pointer to the timer register.
 ** \param [in] dma_ctrl   Poniter to the DMA control data.
 **                        fifo_entry: fifo entry address.
 **                        dma_sel: 1x - use for local timer over flow.
 **                                 01 - use for capture.
 **                                 00 - use for compare.
 **                        fifo_wml: water mark level of fifo.
 *****************************************************************************/
void timer_drv_func_chb_dma_init(sdrv_timer_t *timer, uint32_t dma_sel,
                                 uint32_t fifo_wml)
{
    u32 value = timer->dma_ctrl;

    value &= ~(FM_DMA_CTRL_CHN_B_WML | FM_DMA_CTRL_CHN_B_SEL |
               BM_DMA_CTRL_CHN_B_EN);
    value |= (FV_DMA_CTRL_CHN_B_WML(fifo_wml) | (FV_DMA_CTRL_CHN_B_SEL(
                  dma_sel)) | BM_DMA_CTRL_CHN_B_EN);

    timer->dma_ctrl = value;
}

/******************************************************************************
 ** \brief Init the timer DMA control of function channel c.
 **
 ** \param [in] timer      Pointer to the timer register.
 ** \param [in] dma_ctrl   Poniter to the DMA control data.
 **                        fifo_entry: fifo entry address.
 **                        dma_sel: 1x - use for local timer over flow.
 **                                 01 - use for capture.
 **                                 00 - use for compare.
 **                        fifo_wml: water mark level of fifo.
 *****************************************************************************/
void timer_drv_func_chc_dma_init(sdrv_timer_t *timer, uint32_t dma_sel,
                                 uint32_t fifo_wml)
{
    u32 value = timer->dma_ctrl;

    value &= ~(FM_DMA_CTRL_CHN_C_WML | FM_DMA_CTRL_CHN_C_SEL |
               BM_DMA_CTRL_CHN_C_EN);
    value |= (FV_DMA_CTRL_CHN_C_WML(fifo_wml) | (FV_DMA_CTRL_CHN_C_SEL(
                  dma_sel)) | BM_DMA_CTRL_CHN_C_EN);

    timer->dma_ctrl = value;
}

/******************************************************************************
 ** \brief Init the timer DMA control of function channel d.
 **
 ** \param [in] timer      Pointer to the timer register.
 ** \param [in] dma_ctrl   Poniter to the DMA control data.
 **                        fifo_entry: fifo entry address.
 **                        dma_sel: 1x - use for local timer over flow.
 **                                 01 - use for capture.
 **                                 00 - use for compare.
 **                        fifo_wml: water mark level of fifo.
 *****************************************************************************/
void timer_drv_func_chd_dma_init(sdrv_timer_t *timer, uint32_t dma_sel,
                                 uint32_t fifo_wml)
{
    u32 value = timer->dma_ctrl;

    value &= ~(FM_DMA_CTRL_CHN_D_WML | FM_DMA_CTRL_CHN_D_SEL |
               BM_DMA_CTRL_CHN_D_EN);
    value |= (FV_DMA_CTRL_CHN_D_WML(fifo_wml) | (FV_DMA_CTRL_CHN_D_SEL(
                  dma_sel)) | BM_DMA_CTRL_CHN_D_EN);

    timer->dma_ctrl = value;
}

/******************************************************************************
 ** \brief Enable capture function of channel x.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] func_ch   Function channel index.
 *****************************************************************************/
void timer_drv_func_cpt_enable(sdrv_timer_t *timer,
                               timer_drv_func_ch_t func_ch)
{
    if (func_ch == TIMER_DRV_FUNC_CH_A) {
        timer->cpt_a_config |= BM_CPT_A_CONFIG_EN;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_B) {
        timer->cpt_b_config |= BM_CPT_B_CONFIG_EN;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_C) {
        timer->cpt_c_config |= BM_CPT_C_CONFIG_EN;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_D) {
        timer->cpt_d_config |= BM_CPT_D_CONFIG_EN;
    }
}

/******************************************************************************
 ** \brief Disable capture function of channel x.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] func_ch   Function channel index.
 *****************************************************************************/
void timer_drv_func_cpt_disable(sdrv_timer_t *timer,
                                timer_drv_func_ch_t func_ch)
{
    if (func_ch == TIMER_DRV_FUNC_CH_A) {
        timer->cpt_a_config &= (~BM_CPT_A_CONFIG_EN);
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_B) {
        timer->cpt_b_config &= (~BM_CPT_B_CONFIG_EN);
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_C) {
        timer->cpt_c_config &= (~BM_CPT_C_CONFIG_EN);
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_D) {
        timer->cpt_d_config &= (~BM_CPT_D_CONFIG_EN);
    }
}

/******************************************************************************
 ** \brief Enable compare function of channel x.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] func_ch   Function channel index.
 *****************************************************************************/
void timer_drv_func_cmp_enable(sdrv_timer_t *timer,
                               timer_drv_func_ch_t func_ch)
{
    if (func_ch == TIMER_DRV_FUNC_CH_A) {
        timer->cmp_a_config |= BM_CMP_A_CONFIG_EN;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_B) {
        timer->cmp_b_config |= BM_CMP_B_CONFIG_EN;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_C) {
        timer->cmp_c_config |= BM_CMP_C_CONFIG_EN;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_D) {
        timer->cmp_d_config |= BM_CMP_D_CONFIG_EN;
    }
}

/******************************************************************************
 ** \brief Disable compare function of channel x.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] func_ch   Function channel index.
 *****************************************************************************/
void timer_drv_func_cmp_disable(sdrv_timer_t *timer,
                                timer_drv_func_ch_t func_ch)
{
    if (func_ch == TIMER_DRV_FUNC_CH_A) {
        timer->cmp_a_config &= (~BM_CMP_A_CONFIG_EN);
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_B) {
        timer->cmp_b_config &= (~BM_CMP_B_CONFIG_EN);
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_C) {
        timer->cmp_c_config &= (~BM_CMP_C_CONFIG_EN);
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_D) {
        timer->cmp_d_config &= (~BM_CMP_D_CONFIG_EN);
    }
}

/******************************************************************************
 ** \brief Compare function configure of timer function channel a.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] cmp_cfg   Pointer to configure data.
 **                       cmp_cnt_sel: 0 - select G0 for compare
 **                                    1 - select local timer for compare
 **                       dual_cmp_mode: 1 - Generate the compare event for 2 consecutive compare
 **                       one_shot_mode: 1 - Only generate one time compare event.
 **                                      0 - Continously compare until SW clear EN bit.
 **                       cmp0_out_mode: 000 - Positive pulse
 **                                      001 - Negative pulse
 **                                      010 - Signal toggle
 **                                      011 - Level high
 **                                      010 - Level low
 **                                      other: Keep
 **                       cmp0_pulse_width: width when output positive or negative
 **                       cmp_rst_en: 1 - local counter a will be reset by compare event
 *****************************************************************************/
void timer_drv_func_cha_cmp_set(sdrv_timer_t *timer,
                                timer_drv_func_cmp_t *cmp_cfg)
{
    u32 value = timer->cmp_a_config;

    timer->cmp0_a_val = cmp_cfg->cmp0_val;
    timer->cmp1_a_val = cmp_cfg->cmp1_val;

    timer->cmp_a_val_upt = 1;

    value &= ~( BM_CMP_A_CONFIG_CNT_SEL | BM_CMP_A_CONFIG_DUAL_CMP_MODE |
                BM_CMP_A_CONFIG_SINGLE_MODE |
                FM_CMP_A_CONFIG_CMP0_OUT_MODE | FM_CMP_A_CONFIG_CMP0_PULSE_WID |
                FM_CMP_A_CONFIG_CMP1_OUT_MODE | FM_CMP_A_CONFIG_CMP1_PULSE_WID );

    value |= ( (cmp_cfg->cmp_cnt_sel << 3) | (cmp_cfg->dual_cmp_mode << 2) |
               (cmp_cfg->single_mode << 1) |
               FV_CMP_A_CONFIG_CMP0_OUT_MODE(cmp_cfg->cmp0_out_mode) |
               FV_CMP_A_CONFIG_CMP0_PULSE_WID(cmp_cfg->cmp0_pulse_width) |
               FV_CMP_A_CONFIG_CMP1_OUT_MODE(cmp_cfg->cmp1_out_mode) |
               FV_CMP_A_CONFIG_CMP1_PULSE_WID(cmp_cfg->cmp1_pulse_width) );

    timer->cmp_a_config = value;

    if (cmp_cfg->cmp_rst_en) {
        timer->cnt_config |= BM_CNT_CONFIG_LOCAL_A_RST_EN;
    }
    else {
        timer->cnt_config &= ~BM_CNT_CONFIG_LOCAL_A_RST_EN;
    }
}

/******************************************************************************
 ** \brief Compare function configure of timer function channel b.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] cmp_cfg   Pointer to configure data.
 *****************************************************************************/
void timer_drv_func_chb_cmp_set(sdrv_timer_t *timer,
                                timer_drv_func_cmp_t *cmp_cfg)
{
    uint32_t value = timer->cmp_b_config;

    timer->cmp0_b_val = cmp_cfg->cmp0_val;
    timer->cmp1_b_val = cmp_cfg->cmp1_val;

    timer->cmp_b_val_upt = 1;

    value &= ~(BM_CMP_B_CONFIG_CNT_SEL | BM_CMP_B_CONFIG_DUAL_CMP_MODE |
               BM_CMP_B_CONFIG_SINGLE_MODE |
               FM_CMP_B_CONFIG_CMP0_OUT_MODE | FM_CMP_B_CONFIG_CMP0_PULSE_WID |
               FM_CMP_B_CONFIG_CMP1_OUT_MODE | FM_CMP_B_CONFIG_CMP1_PULSE_WID );

    value |= ((cmp_cfg->cmp_cnt_sel << 3) | (cmp_cfg->dual_cmp_mode << 2) |
              (cmp_cfg->single_mode << 1) |
              FV_CMP_B_CONFIG_CMP0_OUT_MODE(cmp_cfg->cmp0_out_mode) |
              FV_CMP_B_CONFIG_CMP0_PULSE_WID(cmp_cfg->cmp0_pulse_width) |
              FV_CMP_B_CONFIG_CMP1_OUT_MODE(cmp_cfg->cmp1_out_mode) |
              FV_CMP_B_CONFIG_CMP1_PULSE_WID(cmp_cfg->cmp1_pulse_width) );

    timer->cmp_b_config = value;


    if (cmp_cfg->cmp_rst_en) {
        timer->cnt_config |= BM_CNT_CONFIG_LOCAL_B_RST_EN;
    }
    else {
        timer->cnt_config &= ~BM_CNT_CONFIG_LOCAL_B_RST_EN;
    }
}

/******************************************************************************
 ** \brief Compare function configure of timer function channel c.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] cmp_cfg   Pointer to configure data.
 *****************************************************************************/
void timer_drv_func_chc_cmp_set(sdrv_timer_t *timer,
                                timer_drv_func_cmp_t *cmp_cfg)
{
    uint32_t value = timer->cmp_c_config;

    timer->cmp0_c_val = cmp_cfg->cmp0_val;
    timer->cmp1_c_val = cmp_cfg->cmp1_val;

    timer->cmp_c_val_upt = 1;

    value &= ~(BM_CMP_C_CONFIG_CNT_SEL | BM_CMP_C_CONFIG_DUAL_CMP_MODE |
               BM_CMP_C_CONFIG_SINGLE_MODE |
               FM_CMP_C_CONFIG_CMP0_OUT_MODE | FM_CMP_C_CONFIG_CMP0_PULSE_WID |
               FM_CMP_C_CONFIG_CMP1_OUT_MODE | FM_CMP_C_CONFIG_CMP1_PULSE_WID );

    value |= ((cmp_cfg->cmp_cnt_sel << 3) | (cmp_cfg->dual_cmp_mode << 2) |
              (cmp_cfg->single_mode << 1) |
              FV_CMP_C_CONFIG_CMP0_OUT_MODE(cmp_cfg->cmp0_out_mode) |
              FV_CMP_C_CONFIG_CMP0_PULSE_WID(cmp_cfg->cmp0_pulse_width) |
              FV_CMP_C_CONFIG_CMP1_OUT_MODE(cmp_cfg->cmp1_out_mode) |
              FV_CMP_C_CONFIG_CMP1_PULSE_WID(cmp_cfg->cmp1_pulse_width) );

    timer->cmp_c_config = value;



    if (cmp_cfg->cmp_rst_en) {
        timer->cnt_config |= BM_CNT_CONFIG_LOCAL_C_RST_EN;
    }
    else {
        timer->cnt_config &= ~BM_CNT_CONFIG_LOCAL_C_RST_EN;
    }
}

/******************************************************************************
 ** \brief Compare function configure of timer function channel d.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] cmp_cfg   Pointer to configure data.
 *****************************************************************************/
void timer_drv_func_chd_cmp_set(sdrv_timer_t *timer,
                                timer_drv_func_cmp_t *cmp_cfg)
{
    uint32_t value = timer->cmp_d_config;

    timer->cmp0_d_val = cmp_cfg->cmp0_val;
    timer->cmp1_d_val = cmp_cfg->cmp1_val;

    timer->cmp_d_val_upt = 1;

    value &= ~( BM_CMP_D_CONFIG_CNT_SEL | BM_CMP_D_CONFIG_DUAL_CMP_MODE |
                BM_CMP_D_CONFIG_SINGLE_MODE |
                FM_CMP_D_CONFIG_CMP0_OUT_MODE | FM_CMP_D_CONFIG_CMP0_PULSE_WID |
                FM_CMP_D_CONFIG_CMP1_OUT_MODE | FM_CMP_D_CONFIG_CMP1_PULSE_WID );

    value |= ( (cmp_cfg->cmp_cnt_sel << 3) | (cmp_cfg->dual_cmp_mode << 2) |
               (cmp_cfg->single_mode << 1) |
               FV_CMP_D_CONFIG_CMP0_OUT_MODE(cmp_cfg->cmp0_out_mode) |
               FV_CMP_D_CONFIG_CMP0_PULSE_WID(cmp_cfg->cmp0_pulse_width) |
               FV_CMP_D_CONFIG_CMP1_OUT_MODE(cmp_cfg->cmp1_out_mode) |
               FV_CMP_D_CONFIG_CMP1_PULSE_WID(cmp_cfg->cmp1_pulse_width) );

    timer->cmp_d_config = value;



    if (cmp_cfg->cmp_rst_en) {
        timer->cnt_config |= BM_CNT_CONFIG_LOCAL_D_RST_EN;
    }
    else {
        timer->cnt_config &= ~BM_CNT_CONFIG_LOCAL_D_RST_EN;
    }
}

/******************************************************************************
 ** \brief Capture function configure of timer function channel a.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] cpt_cfg   Pointer to configure data.
 **                       cpt_cnt_sel: 00 - select G0 for compare
 **                                    01 - G0+G1 64 bit timer, do not use for DUAL_CPT_MODE
 **                                    10 - select local timer for compare
 **                       dual_cpt_mode: 1 - Generate the capture event for 2 consecutive compare
 **                       one_shot_mode: 1 - Only generate one time compare event.
 **                                      0 - Continously compare until SW clear EN bit.
 **                       input_mode: 000 - rising edge
 **                                   001 - falling edge
 **                                   010 - signal toggle
 **                       first_cpt_rst_en: 1 - local counter a will be reset by first capture event
 *****************************************************************************/
void timer_drv_func_cha_cpt_set(sdrv_timer_t *timer,
                                timer_drv_func_cpt_t *cpt_cfg)
{
    uint32_t value = timer->cpt_a_config;

    value &= ~( FM_CPT_A_CONFIG_CNT_SEL | BM_CPT_A_CONFIG_DUAL_CPT_MODE |
                BM_CPT_A_CONFIG_SINGLE_MODE | FM_CPT_A_CONFIG_CPT_TRIG_MODE );

    value |= ( (cpt_cfg->cpt_cnt_sel << 3) | (cpt_cfg->dual_cpt_mode << 2) |
               (cpt_cfg->single_mode << 1) |
               FV_CPT_A_CONFIG_CPT_TRIG_MODE(cpt_cfg->trig_mode) );

    timer->cpt_a_config = value;
}

/******************************************************************************
 ** \brief Capture function configure of timer function channel b.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] cpt_cfg   Pointer to configure data.
 *****************************************************************************/
void timer_drv_func_chb_cpt_set(sdrv_timer_t *timer,
                                timer_drv_func_cpt_t *cpt_cfg)
{
    uint32_t value = timer->cpt_b_config;

    value &= ~( FM_CPT_B_CONFIG_CNT_SEL | BM_CPT_B_CONFIG_DUAL_CPT_MODE |
                BM_CPT_B_CONFIG_SINGLE_MODE | FM_CPT_B_CONFIG_CPT_TRIG_MODE );

    value |= ( (cpt_cfg->cpt_cnt_sel << 3) | (cpt_cfg->dual_cpt_mode << 2) |
               (cpt_cfg->single_mode << 1) |
               FV_CPT_B_CONFIG_CPT_TRIG_MODE(cpt_cfg->trig_mode) );

    timer->cpt_b_config = value;
}

/******************************************************************************
 ** \brief Capture function configure of timer function channel c.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] cpt_cfg   Pointer to configure data.
 *****************************************************************************/
void timer_drv_func_chc_cpt_set(sdrv_timer_t *timer,
                                timer_drv_func_cpt_t *cpt_cfg)
{
    uint32_t value = timer->cpt_c_config;

    value &= ~( FM_CPT_C_CONFIG_CNT_SEL | BM_CPT_C_CONFIG_DUAL_CPT_MODE |
                BM_CPT_C_CONFIG_SINGLE_MODE | FM_CPT_C_CONFIG_CPT_TRIG_MODE );

    value |= ( (cpt_cfg->cpt_cnt_sel << 3) | (cpt_cfg->dual_cpt_mode << 2) |
               (cpt_cfg->single_mode << 1) |
               FV_CPT_C_CONFIG_CPT_TRIG_MODE(cpt_cfg->trig_mode) );

    timer->cpt_c_config = value;
}

/******************************************************************************
 ** \brief Capture function configure of timer function channel d.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] cpt_cfg   Pointer to configure data.
 *****************************************************************************/
void timer_drv_func_chd_cpt_set(sdrv_timer_t *timer,
                                timer_drv_func_cpt_t *cpt_cfg)
{
    uint32_t value = timer->cpt_d_config;

    value &= ~( FM_CPT_D_CONFIG_CNT_SEL | BM_CPT_D_CONFIG_DUAL_CPT_MODE |
                BM_CPT_D_CONFIG_SINGLE_MODE | FM_CPT_D_CONFIG_CPT_TRIG_MODE );

    value |= ( (cpt_cfg->cpt_cnt_sel << 3) | (cpt_cfg->dual_cpt_mode << 2) |
               (cpt_cfg->single_mode << 1) |
               FV_CPT_A_CONFIG_CPT_TRIG_MODE(cpt_cfg->trig_mode) );

    timer->cpt_d_config = value;
}

/******************************************************************************
 ** \brief Function configure.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] func_ch   Function channel index.
 ** \param [in] cpt_cfg   Pointer to configure data.
 *****************************************************************************/
void timer_drv_func_init(sdrv_timer_t *timer, timer_drv_func_ch_t func_ch,
                         timer_drv_func_t *func_cfg)
{
    if (func_cfg->dma_ctrl.dma_enable) {
        if (func_ch == TIMER_DRV_FUNC_CH_A) {
            timer_drv_func_cha_dma_init(timer, func_cfg->dma_ctrl.dma_sel,
                                        func_cfg->dma_ctrl.fifo_wml);
        }
        else if (func_ch == TIMER_DRV_FUNC_CH_B) {
            timer_drv_func_chb_dma_init(timer, func_cfg->dma_ctrl.dma_sel,
                                        func_cfg->dma_ctrl.fifo_wml);
        }
        else if (func_ch == TIMER_DRV_FUNC_CH_C) {
            timer_drv_func_chc_dma_init(timer, func_cfg->dma_ctrl.dma_sel,
                                        func_cfg->dma_ctrl.fifo_wml);
        }
        else if (func_ch == TIMER_DRV_FUNC_CH_D) {
            timer_drv_func_chd_dma_init(timer, func_cfg->dma_ctrl.dma_sel,
                                        func_cfg->dma_ctrl.fifo_wml);
        }
    }

    if (func_cfg->func_type == TIMER_DRV_FUNC_TYPE_NONE) {
        timer_drv_func_cpt_disable(timer, func_ch);
        timer_drv_func_cmp_disable(timer, func_ch);
    }
    else if (func_cfg->func_type == TIMER_DRV_FUNC_TYPE_CMP) {
        if (func_ch == TIMER_DRV_FUNC_CH_A) {
            timer_drv_func_cha_cmp_set(timer, &(func_cfg->sub_func.cmp_cfg));
        }
        else if (func_ch == TIMER_DRV_FUNC_CH_B) {
            timer_drv_func_chb_cmp_set(timer, &(func_cfg->sub_func.cmp_cfg));
        }
        else if (func_ch == TIMER_DRV_FUNC_CH_C) {
            timer_drv_func_chc_cmp_set(timer, &(func_cfg->sub_func.cmp_cfg));
        }
        else if (func_ch == TIMER_DRV_FUNC_CH_D) {
            timer_drv_func_chd_cmp_set(timer, &(func_cfg->sub_func.cmp_cfg));
        }

        timer_drv_func_cmp_enable(timer, func_ch);
        timer_drv_func_cpt_disable(timer, func_ch);
    }
    else if (func_cfg->func_type == TIMER_DRV_FUNC_TYPE_CPT) {
        if (func_ch == TIMER_DRV_FUNC_CH_A) {
            timer_drv_func_cha_cpt_set(timer, &(func_cfg->sub_func.cpt_cfg));
        }
        else if (func_ch == TIMER_DRV_FUNC_CH_B) {
            timer_drv_func_chb_cpt_set(timer, &(func_cfg->sub_func.cpt_cfg));
        }
        else if (func_ch == TIMER_DRV_FUNC_CH_C) {
            timer_drv_func_chc_cpt_set(timer, &(func_cfg->sub_func.cpt_cfg));
        }
        else if (func_ch == TIMER_DRV_FUNC_CH_D) {
            timer_drv_func_chd_cpt_set(timer, &(func_cfg->sub_func.cpt_cfg));
        }

        timer_drv_func_cpt_enable(timer, func_ch);
        timer_drv_func_cmp_disable(timer, func_ch);
    }
}

/******************************************************************************
 ** \brief Enable timer interrupt state.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] offset    intterrupt state bit offset.
 *****************************************************************************/
void timer_drv_int_sta_enable(sdrv_timer_t *timer,
                              timer_drv_int_src_t offset)
{
    timer->int_sta_en |= (1 << offset);
}

/******************************************************************************
 ** \brief Disable timer interrupt state.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] offset    intterrupt state bit offset.
 *****************************************************************************/
void timer_drv_int_sta_disable(sdrv_timer_t *timer,
                               timer_drv_int_src_t offset)
{
    timer->int_sta_en &= ~(1 << offset);
}

/******************************************************************************
 ** \brief Enable timer interrupt signal.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] offset    intterrupt state bit offset.
 *****************************************************************************/
void timer_drv_int_sig_enable(sdrv_timer_t *timer,
                              timer_drv_int_src_t offset)
{
    timer->int_sig_en |= (1 << offset);
}

/******************************************************************************
 ** \brief Disable timer interrupt signal.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] offset    intterrupt state bit offset.
 *****************************************************************************/
void timer_drv_int_sig_disable(sdrv_timer_t *timer,
                               timer_drv_int_src_t offset)
{
    timer->int_sig_en &= ~(1 << offset);
}

/******************************************************************************
 ** \brief Disable timer interrupt signal.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] func_ch   Function channel index.
 ** \param [in] enable    Force compare output or not.
 ** \param [in] level     Force output level.
 *****************************************************************************/
void timer_drv_cmp_force_out(sdrv_timer_t *timer,
                             timer_drv_func_ch_t func_ch, bool enable, bool level)
{
    if (enable) {
        if (level) {
            if (func_ch == TIMER_DRV_FUNC_CH_A) {
                timer->cmp_a_config |=  BM_CMP_A_CONFIG_FRC_HIGH;
                timer->cmp_a_config &= ~BM_CMP_A_CONFIG_FRC_LOW;
            }
            else if (func_ch == TIMER_DRV_FUNC_CH_B) {
                timer->cmp_b_config |=  BM_CMP_B_CONFIG_FRC_HIGH;
                timer->cmp_b_config &= ~BM_CMP_B_CONFIG_FRC_LOW;
            }
            else if (func_ch == TIMER_DRV_FUNC_CH_C) {
                timer->cmp_c_config |=  BM_CMP_C_CONFIG_FRC_HIGH;
                timer->cmp_c_config &= ~BM_CMP_C_CONFIG_FRC_LOW;
            }
            else if (func_ch == TIMER_DRV_FUNC_CH_D) {
                timer->cmp_d_config |=  BM_CMP_D_CONFIG_FRC_HIGH;
                timer->cmp_d_config &= ~BM_CMP_D_CONFIG_FRC_LOW;
            }
        }
        else {
            if (func_ch == TIMER_DRV_FUNC_CH_A) {
                timer->cmp_a_config &= ~BM_CMP_A_CONFIG_FRC_HIGH;
                timer->cmp_a_config |=  BM_CMP_A_CONFIG_FRC_LOW;
            }
            else if (func_ch == TIMER_DRV_FUNC_CH_B) {
                timer->cmp_b_config &= ~BM_CMP_B_CONFIG_FRC_HIGH;
                timer->cmp_b_config |=  BM_CMP_B_CONFIG_FRC_LOW;
            }
            else if (func_ch == TIMER_DRV_FUNC_CH_C) {
                timer->cmp_c_config &= ~BM_CMP_C_CONFIG_FRC_HIGH;
                timer->cmp_c_config |=  BM_CMP_C_CONFIG_FRC_LOW;
            }
            else if (func_ch == TIMER_DRV_FUNC_CH_D) {
                timer->cmp_d_config &= ~BM_CMP_D_CONFIG_FRC_HIGH;
                timer->cmp_d_config |=  BM_CMP_D_CONFIG_FRC_LOW;
            }
        }
    }
    else {
        if (func_ch == TIMER_DRV_FUNC_CH_A) {
            timer->cmp_a_config &= ~(BM_CMP_A_CONFIG_FRC_HIGH |
                                     BM_CMP_A_CONFIG_FRC_LOW);
        }
        else if (func_ch == TIMER_DRV_FUNC_CH_B) {
            timer->cmp_b_config &= ~(BM_CMP_B_CONFIG_FRC_HIGH |
                                     BM_CMP_B_CONFIG_FRC_LOW);
        }
        else if (func_ch == TIMER_DRV_FUNC_CH_C) {
            timer->cmp_c_config &= ~(BM_CMP_C_CONFIG_FRC_HIGH |
                                     BM_CMP_C_CONFIG_FRC_LOW);
        }
        else if (func_ch == TIMER_DRV_FUNC_CH_D) {
            timer->cmp_d_config &= ~(BM_CMP_D_CONFIG_FRC_HIGH |
                                     BM_CMP_D_CONFIG_FRC_LOW);
        }
    }
}

/******************************************************************************
 ** \brief Get the capture value.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] func_ch   Function channel index.
 *****************************************************************************/
uint32_t timer_drv_cpt_value_get(sdrv_timer_t *timer,
                                 timer_drv_func_ch_t func_ch)
{
    uint32_t value = 0;

    if (func_ch == TIMER_DRV_FUNC_CH_A) {
        value = timer->fifo_a;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_B) {
        value = timer->fifo_b;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_C) {
        value = timer->fifo_c;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_D) {
        value = timer->fifo_d;
    }

    return value;
}

/******************************************************************************
 ** \brief Set the compare value.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] func_ch   Function channel index.
 *****************************************************************************/
void timer_drv_cmp_value_set(sdrv_timer_t *timer,
                             timer_drv_func_ch_t func_ch, uint32_t value0, uint32_t value1)
{
    if (func_ch == TIMER_DRV_FUNC_CH_A) {
        timer->cmp0_a_val = value0;
        timer->cmp1_a_val = value1;
        timer->cmp_a_val_upt = 1;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_B) {
        timer->cmp0_b_val = value0;
        timer->cmp1_b_val = value1;
        timer->cmp_b_val_upt = 1;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_C) {
        timer->cmp0_c_val = value0;
        timer->cmp1_c_val = value1;
        timer->cmp_c_val_upt = 1;
    }
    else if (func_ch == TIMER_DRV_FUNC_CH_D) {
        timer->cmp0_d_val = value0;
        timer->cmp1_d_val = value1;
        timer->cmp_d_val_upt = 1;
    }
}

/******************************************************************************
 ** \brief Init the SSE capture function.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] cpt_ch    Synthesis 4x cpt inputs and one of compare signal into one signal for cpt_ch.
 *****************************************************************************/
void timer_drv_sse_cpt_init(sdrv_timer_t *timer,
                            timer_drv_func_ch_t cpt_ch, bool cpt_en, timer_drv_func_ch_t cmp_ch)
{
    if (cpt_ch == TIMER_DRV_FUNC_CH_A) {
        if (cpt_en) {
            timer->sse_ctrl &= ~FM_SSE_CTRL_CPT_CMP_SEL_A;
            timer->sse_ctrl |= (BM_SSE_CTRL_CPT_A_SYS_EN | FV_SSE_CTRL_CPT_CMP_SEL_A(
                                    cmp_ch));
        }
        else {
            timer->sse_ctrl &= ~BM_SSE_CTRL_CPT_A_SYS_EN;
        }
    }
    else if (cpt_ch == TIMER_DRV_FUNC_CH_B) {
        if (cpt_en) {
            timer->sse_ctrl &= ~FM_SSE_CTRL_CPT_CMP_SEL_B;
            timer->sse_ctrl |= (BM_SSE_CTRL_CPT_B_SYS_EN | FV_SSE_CTRL_CPT_CMP_SEL_B(
                                    cmp_ch));
        }
        else {
            timer->sse_ctrl &= ~BM_SSE_CTRL_CPT_B_SYS_EN;
        }
    }
    else if (cpt_ch == TIMER_DRV_FUNC_CH_C) {
        if (cpt_en) {
            timer->sse_ctrl &= ~FM_SSE_CTRL_CPT_CMP_SEL_C;
            timer->sse_ctrl |= (BM_SSE_CTRL_CPT_C_SYS_EN | FV_SSE_CTRL_CPT_CMP_SEL_C(
                                    cmp_ch));
        }
        else {
            timer->sse_ctrl &= ~BM_SSE_CTRL_CPT_C_SYS_EN;
        }
    }
    else if (cpt_ch == TIMER_DRV_FUNC_CH_D) {
        if (cpt_en) {
            timer->sse_ctrl &= ~FM_SSE_CTRL_CPT_CMP_SEL_D;
            timer->sse_ctrl |= (BM_SSE_CTRL_CPT_D_SYS_EN | FV_SSE_CTRL_CPT_CMP_SEL_D(
                                    cmp_ch));
        }
        else {
            timer->sse_ctrl &= ~BM_SSE_CTRL_CPT_D_SYS_EN;
        }
    }
}

/******************************************************************************
 ** \brief Init the SSE capture function.
 **
 ** \param [in] timer     Pointer to the timer register.
 ** \param [in] cmp_ch    Synthesis 4x cmp inputs and timer_clk signal into one signal for cmp_ch.
 *****************************************************************************/
void timer_drv_sse_cmp_init(sdrv_timer_t *timer,
                            timer_drv_func_ch_t cmp_ch, bool cmp_en)
{
    if (cmp_ch == TIMER_DRV_FUNC_CH_A) {
        if (cmp_en) {
            timer->sse_ctrl |= BM_SSE_CTRL_CMP_A_SYS_EN;
        }
        else {
            timer->sse_ctrl &= ~BM_SSE_CTRL_CMP_A_SYS_EN;
        }
    }
    else if (cmp_ch == TIMER_DRV_FUNC_CH_B) {
        if (cmp_en) {
            timer->sse_ctrl |= BM_SSE_CTRL_CMP_B_SYS_EN;
        }
        else {
            timer->sse_ctrl &= ~BM_SSE_CTRL_CMP_B_SYS_EN;
        }
    }
    else if (cmp_ch == TIMER_DRV_FUNC_CH_C) {
        if (cmp_en) {
            timer->sse_ctrl |= BM_SSE_CTRL_CMP_C_SYS_EN;
        }
        else {
            timer->sse_ctrl &= ~BM_SSE_CTRL_CMP_C_SYS_EN;
        }
    }
    else if (cmp_ch == TIMER_DRV_FUNC_CH_D) {
        if (cmp_en) {
            timer->sse_ctrl |= BM_SSE_CTRL_CMP_D_SYS_EN;
        }
        else {
            timer->sse_ctrl &= ~BM_SSE_CTRL_CMP_D_SYS_EN;
        }
    }
}

/******************************************************************************
 ** \brief Timer overflow IRQ handle.
 **
 ** \param [in] timer           Pointer to the timer controller base address.
 ** \param [in] drv_context
 *****************************************************************************/
enum handler_return timer_drv_ovf_irq_handle(sdrv_timer_t *timer,
        timer_drv_context_t *drv_context)
{
    if (timer->int_sta & BM_INT_STA_CNT_G0_OVF) {
        timer->int_sta |= BM_INT_STA_CNT_G0_OVF;

        if (!drv_context->periodic) {
            timer_drv_int_sta_disable(timer, TIMER_DRV_CNT_G0_OVF_INT_SRC);
            timer_drv_int_sig_disable(timer, TIMER_DRV_CNT_G0_OVF_INT_SRC);
        }

        if (drv_context->global_ovf_cbk[TIMER_DRV_OVF_G0] != NULL) {
            return drv_context->global_ovf_cbk[TIMER_DRV_OVF_G0]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_CNT_G1_OVF) {
        timer->int_sta |= BM_INT_STA_CNT_G1_OVF;

        if (!drv_context->periodic) {
            timer_drv_int_sta_disable(timer, TIMER_DRV_CNT_G1_OVF_INT_SRC);
            timer_drv_int_sig_disable(timer, TIMER_DRV_CNT_G1_OVF_INT_SRC);
        }

        if (drv_context->global_ovf_cbk[TIMER_DRV_OVF_G1] != NULL) {
            return drv_context->global_ovf_cbk[TIMER_DRV_OVF_G1]();
        }
    }

    return INT_RESCHEDULE;
}

/******************************************************************************
 ** \brief Timer function IRQ handle.
 **
 ** \param [in] timer           Pointer to the timer controller base address.
 ** \param [in] drv_context
 *****************************************************************************/
enum handler_return timer_drv_func_irq_handle(sdrv_timer_t *timer,
        timer_drv_context_t *drv_context)
{
    //Func A
    if (timer->int_sta & BM_INT_STA_CNT_LOCAL_A_OVF) {
        timer->int_sta |= BM_INT_STA_CNT_LOCAL_A_OVF;

        if (!drv_context->periodic) {
            timer_drv_int_sta_disable(timer, TIMER_DRV_CNT_LA_OVF_INT_SRC);
            timer_drv_int_sig_disable(timer, TIMER_DRV_CNT_LA_OVF_INT_SRC);
        }

        if (drv_context->local_ovf_cbk[TIMER_DRV_FUNC_CH_A] != NULL) {
            return drv_context->local_ovf_cbk[TIMER_DRV_FUNC_CH_A]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_CMP_A) {
        timer->int_sta |= BM_INT_STA_CMP_A;

        if (drv_context->local_cmp_cbk[TIMER_DRV_FUNC_CH_A] != NULL) {
            return drv_context->local_cmp_cbk[TIMER_DRV_FUNC_CH_A]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_CPT_A) {
        timer->int_sta |= BM_INT_STA_CPT_A;

        if (drv_context->local_cpt_cbk[TIMER_DRV_FUNC_CH_A] != NULL) {
            return drv_context->local_cpt_cbk[TIMER_DRV_FUNC_CH_A]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_FIFO_A_OVERRUN) {
        timer->int_sta |= BM_INT_STA_FIFO_A_OVERRUN;

        if (drv_context->local_overrun_cbk[TIMER_DRV_FUNC_CH_A] != NULL) {
            return drv_context->local_overrun_cbk[TIMER_DRV_FUNC_CH_A]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_FIFO_A_UNDERRUN) {
        timer->int_sta  |= BM_INT_STA_FIFO_A_UNDERRUN;

        if (drv_context->local_underrun_cbk[TIMER_DRV_FUNC_CH_A] != NULL) {
            return drv_context->local_underrun_cbk[TIMER_DRV_FUNC_CH_A]();
        }
    }

    //Func B
    else if (timer->int_sta & BM_INT_STA_CNT_LOCAL_B_OVF) {
        timer->int_sta |= BM_INT_STA_CNT_LOCAL_B_OVF;

        if (!drv_context->periodic) {
            timer_drv_int_sta_disable(timer, TIMER_DRV_CNT_LB_OVF_INT_SRC);
            timer_drv_int_sig_disable(timer, TIMER_DRV_CNT_LB_OVF_INT_SRC);
        }

        if (drv_context->local_ovf_cbk[TIMER_DRV_FUNC_CH_B] != NULL) {
            return drv_context->local_ovf_cbk[TIMER_DRV_FUNC_CH_B]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_CMP_B) {
        timer->int_sta |= BM_INT_STA_CMP_B;

        if (drv_context->local_cmp_cbk[TIMER_DRV_FUNC_CH_B] != NULL) {
            return drv_context->local_cmp_cbk[TIMER_DRV_FUNC_CH_B]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_CPT_B) {
        timer->int_sta |= BM_INT_STA_CPT_B;

        if (drv_context->local_cpt_cbk[TIMER_DRV_FUNC_CH_B] != NULL) {
            return drv_context->local_cpt_cbk[TIMER_DRV_FUNC_CH_B]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_FIFO_B_OVERRUN) {
        timer->int_sta |= BM_INT_STA_FIFO_B_OVERRUN;

        if (drv_context->local_overrun_cbk[TIMER_DRV_FUNC_CH_B] != NULL) {
            return drv_context->local_overrun_cbk[TIMER_DRV_FUNC_CH_B]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_FIFO_B_UNDERRUN) {
        timer->int_sta  |= BM_INT_STA_FIFO_B_UNDERRUN;

        if (drv_context->local_underrun_cbk[TIMER_DRV_FUNC_CH_B] != NULL) {
            return drv_context->local_underrun_cbk[TIMER_DRV_FUNC_CH_B]();
        }
    }

    //Func C
    else if (timer->int_sta & BM_INT_STA_CNT_LOCAL_C_OVF) {
        timer->int_sta |= BM_INT_STA_CNT_LOCAL_C_OVF;

        if (!drv_context->periodic) {
            timer_drv_int_sta_disable(timer, TIMER_DRV_CNT_LC_OVF_INT_SRC);
            timer_drv_int_sig_disable(timer, TIMER_DRV_CNT_LC_OVF_INT_SRC);
        }

        if (drv_context->local_ovf_cbk[TIMER_DRV_FUNC_CH_C] != NULL) {
            return drv_context->local_ovf_cbk[TIMER_DRV_FUNC_CH_C]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_CMP_C) {
        timer->int_sta |= BM_INT_STA_CMP_C;

        if (drv_context->local_cmp_cbk[TIMER_DRV_FUNC_CH_C] != NULL) {
            return drv_context->local_cmp_cbk[TIMER_DRV_FUNC_CH_C]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_CPT_C) {
        timer->int_sta |= BM_INT_STA_CPT_C;

        if (drv_context->local_cpt_cbk[TIMER_DRV_FUNC_CH_C] != NULL) {
            return drv_context->local_cpt_cbk[TIMER_DRV_FUNC_CH_C]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_FIFO_C_OVERRUN) {
        timer->int_sta |= BM_INT_STA_FIFO_C_OVERRUN;

        if (drv_context->local_overrun_cbk[TIMER_DRV_FUNC_CH_C] != NULL) {
            return drv_context->local_overrun_cbk[TIMER_DRV_FUNC_CH_C]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_FIFO_C_UNDERRUN) {
        timer->int_sta |= BM_INT_STA_FIFO_C_UNDERRUN;

        if (drv_context->local_underrun_cbk[TIMER_DRV_FUNC_CH_C] != NULL) {
            return drv_context->local_underrun_cbk[TIMER_DRV_FUNC_CH_C]();
        }
    }

    //Func D
    else if (timer->int_sta & BM_INT_STA_CNT_LOCAL_D_OVF) {
        timer->int_sta |= BM_INT_STA_CNT_LOCAL_D_OVF;

        if (!drv_context->periodic) {
            timer_drv_int_sta_disable(timer, TIMER_DRV_CNT_LD_OVF_INT_SRC);
            timer_drv_int_sig_disable(timer, TIMER_DRV_CNT_LD_OVF_INT_SRC);
        }

        if (drv_context->local_ovf_cbk[TIMER_DRV_FUNC_CH_D] != NULL) {
            return drv_context->local_ovf_cbk[TIMER_DRV_FUNC_CH_D]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_CMP_D) {
        timer->int_sta |= BM_INT_STA_CMP_D;

        if (drv_context->local_cmp_cbk[TIMER_DRV_FUNC_CH_D] != NULL) {
            return drv_context->local_cmp_cbk[TIMER_DRV_FUNC_CH_D]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_CPT_D) {
        timer->int_sta |= BM_INT_STA_CPT_D;

        if (drv_context->local_cpt_cbk[TIMER_DRV_FUNC_CH_D] != NULL) {
            return drv_context->local_cpt_cbk[TIMER_DRV_FUNC_CH_D]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_FIFO_D_OVERRUN) {
        timer->int_sta |= BM_INT_STA_FIFO_D_OVERRUN;

        if (drv_context->local_overrun_cbk[TIMER_DRV_FUNC_CH_D] != NULL) {
            return drv_context->local_overrun_cbk[TIMER_DRV_FUNC_CH_D]();
        }
    }
    else if (timer->int_sta & BM_INT_STA_FIFO_D_UNDERRUN) {
        timer->int_sta |= BM_INT_STA_FIFO_D_UNDERRUN;

        if (drv_context->local_underrun_cbk[TIMER_DRV_FUNC_CH_D] != NULL) {
            return drv_context->local_underrun_cbk[TIMER_DRV_FUNC_CH_D]();
        }
    }

    return INT_NO_RESCHEDULE;
}

/******************************************************************************
 ** \brief Clear the timer interrupt status flag.
 **
 ** \param [in] timer           Pointer to the timer controller base address.
 ** \param [in] drv_context
 *****************************************************************************/
void timer_drv_int_sta_clear(sdrv_timer_t *timer,
                             timer_drv_int_src_t offset)
{
    timer->int_sta |= (1 << offset);
}

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

