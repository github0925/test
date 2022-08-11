/*
* pwm_drv.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement semidrive pwm driver
*
* Revision History:
* -----------------
* 010, 11/26/2019 wangyongjun implement this
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

#include "pwm_drv.h"


/******************************************************************************
 ** clock           sample_freq          clock/sample_freq
 ** 400,000,000        8000                50000(0xC350)
 **                   11025                36281(0x8DB9)
 **                   16000                25000(0x61A8)
 **                   22050                18140(0x46DC)
 **                   32000                12500(0x30D4)
 **                   44100                 9070(0x236E)
 **                   48000                 8333(0x208D)
 **                   96000                 4166(0x1046)
*****************************************************************************/
#if (DRV_PWM_PCM_TUNE_GET_METHOD == DRV_PWM_PCM_TUNE_TABLE_METHOD)
static const drv_pwm_pcm_tune_table_t drv_pwm_pcm_tune_table[DRV_PWM_PCM_TABLE_DATA_FORMAT_NUM] =
{
    {   /*data bits*/
        8,
        {
            /*sample freq*/ /*clk div*/ /*ovf val*/ /*clip en*/ /*clip bits*/ /*drop bits*/
            {  8000,        194,         0xFF,          0,          0,       0 }, //(400,000,000)/(11+1)/(255) = 7843
            { 16000,        97,          0xFF,          0,          0,       0 }, //(400,000,000)/(97+1)/(255+1) = 16006
            { 32000,        48,          0xFF,          0,          0,       0 }, //(400,000,000)/(48+1)/(255+1) = 32012
            { 48000,        32,          0xFF,          0,          0,       0 }, //(400,000,000)/(32+1)/(255+1) = 47534
            { 96000,        15,          0xFF,          0,          0,       0 }, //(400,000,000)/(15+1)/(255+1) = 98039
            { 11025,        141,         0xFF,          0,          0,       0 }, //(400,000,000)/(141+1)/(255+1) = 11046
            { 22050,        70,          0xFF,          0,          0,       0 }, //(400,000,000)/(70+1)/(255+1) = 22093
            { 44100,        34,          0xFF,          0,          0,       0 }, //(400,000,000)/(34+1)/(255+1) = 44817
        }
    },

    {   /*data bits*/
        16,
        {
            /*sample freq*/ /*clk div*/ /*ovf val*/ /*clip en*/ /*clip bits*/ /*drop bits*/
            {  8000,        2,          0x3FFE,          1,          1,       0 }, //(400,000,000)/(2+1)/(16382+1) = 8138
            { 16000,        2,          0x1FFE,          1,          2,       0 }, //(400,000,000)/(2+1)/(8190+1) = 16278
            { 32000,        2,          0x0FFE,          1,          3,       0 }, //(400,000,000)/(2+1)/(4094+1) = 32560
            { 48000,        0,          0x1FFE,          1,          2,       0 }, //(400,000,000)/(0+1)/(8190+1) = 48834
            { 96000,        0,          0x0FFE,          1,          3,       0 }, //(400,000,000)/(0+1)/(4094+1) = 97680
            { 11025,        0,          0x8DB8,          1,          0,       0 }, //(400,000,000)/(0+1)/(36280+1) = 11025
            { 22050,        0,          0x46DB,          1,          1,       0 }, //(400,000,000)/(0+1)/(18139+1) = 22050
            { 44100,        0,          0x236D,          1,          2,       0 }, //(400,000,000)/(0+1)/(9069+1) = 44100
        }
    },
};
#endif

/******************************************************************************
 ** \brief Return the none zero bits index.
 **
 ** \param [in]
 *****************************************************************************/
static uint32_t drv_pwm_none_zero_bits_index(uint32_t num)
{
    uint32_t index = 0;

    while(num) {
        index++;
        num >>= 1;
    }
    if(index >= 1) {
        index--;
    }

    return index;
}

/******************************************************************************
 ** \brief Reset the pwm ip module.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 *****************************************************************************/
static void drv_pwm_reset(sdrv_pwm_t* pwm_dev)
{
    sdrv_pwm_cmp_ctrl_t cmp_ctrl;

    cmp_ctrl.val = pwm_dev->cmp_ctrl.val;
    cmp_ctrl.sw_rst = 1;
    pwm_dev->cmp_ctrl.val = cmp_ctrl.val;
}

/******************************************************************************
 ** \brief Clock configure.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 *****************************************************************************/
static void drv_pwm_clk_init(sdrv_pwm_t* pwm_dev, drv_pwm_src_clk_t clk_src, uint16_t clk_div)
{
    sdrv_pwm_cnt_g0_config_t cnt_g0_config;

    cnt_g0_config.val = pwm_dev->cnt_g0_config.val;
    cnt_g0_config.src_clk_sel = clk_src;
    cnt_g0_config.div_num = clk_div;
    pwm_dev->cnt_g0_config.val = cnt_g0_config.val;
}

/******************************************************************************
 ** \brief Clock configure.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 *****************************************************************************/
static void drv_pwm_single_mode_set(sdrv_pwm_t* pwm_dev, drv_pwm_single_mode_t mode)
{
    sdrv_pwm_cmp_ctrl_t cmp_ctrl;

    cmp_ctrl.val = pwm_dev->cmp_ctrl.val;
    cmp_ctrl.sw_rst = 0;
    cmp_ctrl.single_mode = mode;
	pwm_dev->cmp_ctrl.val = cmp_ctrl.val;
}

/******************************************************************************
 ** \brief Clock configure.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 *****************************************************************************/
static void drv_pwm_group_num_set(sdrv_pwm_t* pwm_dev, drv_pwm_group_num_t grp_num)
{
    sdrv_pwm_cmp_config_t cmp_config;

    cmp_config.val = pwm_dev->cmp_config.val;
    cmp_config.grp_num = grp_num;
	pwm_dev->cmp_config.val = cmp_config.val;
}

/******************************************************************************
 ** \brief Clock configure.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 *****************************************************************************/
static void drv_pwm_dual_cmp_mode_set(sdrv_pwm_t* pwm_dev, bool enable)
{
    sdrv_pwm_cmp_config_t cmp_config;

    cmp_config.val = pwm_dev->cmp_config.val;
    cmp_config.dual_cmp_mode = enable;
    pwm_dev->cmp_config.val = cmp_config.val;
}

/******************************************************************************
 ** \brief Clock configure.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 *****************************************************************************/
static void drv_pwm_edge_align_chn_set(sdrv_pwm_t* pwm_dev, drv_pwm_chn_t chn, drv_pwm_phase_polarity_t phase)
{
    sdrv_pwm_cmp_ch_config0_t cmp_cfg0_temp;
    sdrv_pwm_cmp_ch_config1_t cmp_cfg1_temp;
    uint32_t* cmp_cfg0_ptr;
    uint32_t* cmp_cfg1_ptr;

    if (chn == DRV_PWM_CHN_A) {
        cmp_cfg0_ptr = (uint32_t*)&(pwm_dev->cmp_a_config0.val);
        cmp_cfg1_ptr = (uint32_t*)&(pwm_dev->cmp_a_config1.val);
    }
    else if (chn == DRV_PWM_CHN_B) {
        cmp_cfg0_ptr = (uint32_t*)&(pwm_dev->cmp_b_config0.val);
        cmp_cfg1_ptr = (uint32_t*)&(pwm_dev->cmp_b_config1.val);
    }
    else if (chn == DRV_PWM_CHN_C) {
        cmp_cfg0_ptr = (uint32_t*)&(pwm_dev->cmp_c_config0.val);
        cmp_cfg1_ptr = (uint32_t*)&(pwm_dev->cmp_c_config1.val);
    }
    else if (chn == DRV_PWM_CHN_D) {
        cmp_cfg0_ptr = (uint32_t*)&(pwm_dev->cmp_d_config0.val);
        cmp_cfg1_ptr = (uint32_t*)&(pwm_dev->cmp_d_config1.val);
    }
    else {
        return;
    }

    cmp_cfg0_temp.val = *cmp_cfg0_ptr;
    cmp_cfg1_temp.val = *cmp_cfg1_ptr;

    if(phase == DRV_PWM_PHASE_POLARITY_POS) {
        cmp_cfg0_temp.cmp0_out_mode = DRV_CMP_OUT_LEVEL_LOW;
        cmp_cfg1_temp.ovf_out_mode = DRV_CMP_OUT_LEVEL_HIGH;
    }
    else {
        cmp_cfg0_temp.cmp0_out_mode = DRV_CMP_OUT_LEVEL_HIGH;
        cmp_cfg1_temp.ovf_out_mode = DRV_CMP_OUT_LEVEL_LOW;
    }

    *cmp_cfg0_ptr = cmp_cfg0_temp.val;
    *cmp_cfg1_ptr = cmp_cfg1_temp.val;
}

/******************************************************************************
 ** \brief Clock configure.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 *****************************************************************************/
static void drv_pwm_center_align_chn_set(sdrv_pwm_t* pwm_dev, drv_pwm_chn_t chn, drv_pwm_phase_polarity_t phase)
{
    sdrv_pwm_cmp_ch_config0_t cmp_cfg0_temp;
    sdrv_pwm_cmp_ch_config1_t cmp_cfg1_temp;
    uint32_t* cmp_cfg0_ptr;
    uint32_t* cmp_cfg1_ptr;

    if (chn == DRV_PWM_CHN_A) {
        cmp_cfg0_ptr = (uint32_t*)&(pwm_dev->cmp_a_config0.val);
        cmp_cfg1_ptr = (uint32_t*)&(pwm_dev->cmp_a_config1.val);
    }
    else if (chn == DRV_PWM_CHN_B) {
        cmp_cfg0_ptr = (uint32_t*)&(pwm_dev->cmp_b_config0.val);
        cmp_cfg1_ptr = (uint32_t*)&(pwm_dev->cmp_b_config1.val);
    }
    else if (chn == DRV_PWM_CHN_C) {
        cmp_cfg0_ptr = (uint32_t*)&(pwm_dev->cmp_c_config0.val);
        cmp_cfg1_ptr = (uint32_t*)&(pwm_dev->cmp_c_config1.val);
    }
    else if (chn == DRV_PWM_CHN_D) {
        cmp_cfg0_ptr = (uint32_t*)&(pwm_dev->cmp_d_config0.val);
        cmp_cfg1_ptr = (uint32_t*)&(pwm_dev->cmp_d_config1.val);
    }
    else {
        return;
    }

    cmp_cfg0_temp.val = *cmp_cfg0_ptr;
    cmp_cfg1_temp.val = *cmp_cfg1_ptr;

    if(phase == DRV_PWM_PHASE_POLARITY_POS) {
        cmp_cfg0_temp.cmp0_out_mode = DRV_CMP_OUT_LEVEL_HIGH;
        cmp_cfg0_temp.cmp1_out_mode = DRV_CMP_OUT_LEVEL_LOW;
        cmp_cfg1_temp.ovf_out_mode = DRV_CMP_OUT_KEEP;
    }
    else {
        cmp_cfg0_temp.cmp0_out_mode = DRV_CMP_OUT_LEVEL_LOW;
        cmp_cfg0_temp.cmp1_out_mode = DRV_CMP_OUT_LEVEL_HIGH;
        cmp_cfg1_temp.ovf_out_mode = DRV_CMP_OUT_KEEP;
    }

    *cmp_cfg0_ptr = cmp_cfg0_temp.val;
    *cmp_cfg1_ptr = cmp_cfg1_temp.val;
}

/******************************************************************************
 ** \brief Clock configure.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 *****************************************************************************/
static void drv_pwm_edge_align_duty_set( sdrv_pwm_t* pwm_dev,
                                         drv_pwm_chn_t chn,
                                         uint32_t ovf_val,
                                         drv_pwm_phase_polarity_t phase,
                                         uint8_t duty )
{
    uint32_t cmp_val;

    cmp_val = duty * ((uint64_t)ovf_val + 1) / 100;

    if (chn == DRV_PWM_CHN_A) {
        pwm_dev->cmp0_a_val = cmp_val;
        //dprintf(DRV_PWM_DEBUG_LEVEL, "Channel A, duty:%d, ovf_val:%d, cmp_val:%d\n", duty, ovf_val, cmp_val);
    }
    else if (chn == DRV_PWM_CHN_B) {
        pwm_dev->cmp0_b_val = cmp_val;
        //dprintf(DRV_PWM_DEBUG_LEVEL, "Channel B, duty:%d, ovf_val:%d, cmp_val:%d\n", duty, ovf_val, cmp_val);
    }
    else if (chn == DRV_PWM_CHN_C) {
        pwm_dev->cmp0_c_val = cmp_val;
        //dprintf(DRV_PWM_DEBUG_LEVEL, "Channel C, duty:%d, ovf_val:%d, cmp_val:%d\n", duty, ovf_val, cmp_val);
    }
    else if (chn == DRV_PWM_CHN_D) {
        pwm_dev->cmp0_d_val = cmp_val;
        //dprintf(DRV_PWM_DEBUG_LEVEL, "Channel D, duty:%d, ovf_val:%d, cmp_val:%d\n", duty, ovf_val, cmp_val);
    }
    else {
        return;
    }

    /* updater cmp register */
    pwm_dev->cmp_val_upt = 1;
    //while(pwm_dev->cmp_val_upt & 0x00000001);
}

/******************************************************************************
 ** \brief Clock configure.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 *****************************************************************************/
static void drv_pwm_center_align_duty_set( sdrv_pwm_t* pwm_dev,
                                           drv_pwm_chn_t chn,
                                           uint32_t ovf_val,
                                           drv_pwm_phase_polarity_t phase,
                                           uint8_t duty )
{
    uint32_t cmp0_val;
    uint32_t cmp1_val;

    cmp0_val = (100 - duty) * ((uint64_t)ovf_val + 1) / (2 * 100);
    cmp1_val = (ovf_val + 1) - cmp0_val;

    if (chn == DRV_PWM_CHN_A) {
        pwm_dev->cmp0_a_val = cmp0_val;
        pwm_dev->cmp1_a_val = cmp1_val;
    }
    else if (chn == DRV_PWM_CHN_B) {
        pwm_dev->cmp0_b_val = cmp0_val;
        pwm_dev->cmp1_b_val = cmp1_val;
    }
    else if (chn == DRV_PWM_CHN_C) {
        pwm_dev->cmp0_c_val = cmp0_val;
        pwm_dev->cmp1_c_val = cmp1_val;
    }
    else if (chn == DRV_PWM_CHN_D) {
        pwm_dev->cmp0_d_val = cmp0_val;
        pwm_dev->cmp1_d_val = cmp1_val;
    }
    else {
        return;
    }

    /* updater cmp register */
    pwm_dev->cmp_val_upt = 1;
    //while(pwm_dev->cmp_val_upt & 0x00000001);
}

static void drv_pwm_count_reset(sdrv_pwm_t* pwm_dev)
{
    sdrv_pwm_cnt_g0_config_t cnt_g0_config;

    cnt_g0_config.val = pwm_dev->cnt_g0_config.val;
    cnt_g0_config.frc_rld = 1;
    cnt_g0_config.int_clr = 1;
    pwm_dev->cnt_g0_config.val = cnt_g0_config.val;

    //while(pwm_dev->cnt_g0_config.frc_rld);
}

/******************************************************************************
 ** \brief Simple pwm init.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 ** \param [in] pwm_cfg  Pointer to the pwm configure parameter.
 *****************************************************************************/
void drv_pwm_simple_init( sdrv_pwm_t* pwm_dev,
                          drv_pwm_simple_cfg_t* pwm_cfg,
                          drv_pwm_simple_context_t* pwm_simple_ctx )
{
    uint32_t loop;
    uint32_t ovf_val;
    uint32_t ovf_bits;
    drv_pwm_clk_cfg_t clk_cfg;
    sdrv_pwm_cmp_config_t cmp_config;

    /* Software reset whole IP */
    drv_pwm_reset(pwm_dev);

    /* Select soucre clock and clock div */
    clk_cfg.clk_src = DRV_PWM_SRC_CLK_HF;
    clk_cfg.clk_div = DRV_PWM_SIMPLE_CLOCK_DIV_NUM;
    drv_pwm_clk_init(pwm_dev, clk_cfg.clk_src, clk_cfg.clk_div);

    /* Single mode/Consecutive mode */
    drv_pwm_single_mode_set(pwm_dev, pwm_cfg->single_mode);

	/* config group number */
    drv_pwm_group_num_set(pwm_dev, pwm_cfg->grp_num);

    /* Frequency: overflow value */
    ovf_val = ( (DRV_PWM_HF_CLOCK_FREQ) / (clk_cfg.clk_div + 1) ) / ( pwm_cfg->freq );
    if(ovf_val > 0) {
        ovf_val -= 1;
    }
    pwm_dev->cnt_g0_ovf = ovf_val;

    /* reload cnount g0 */
    drv_pwm_count_reset(pwm_dev);

    ovf_bits = drv_pwm_none_zero_bits_index(ovf_val) + 1;
    cmp_config.val = pwm_dev->cmp_config.val;
    if(ovf_bits <= 8) {
        cmp_config.data_format = DRV_PWM_CMP_DATA_8BITS;
    }
    else if(ovf_bits <= 16) {
        cmp_config.data_format = DRV_PWM_CMP_DATA_16BITS;
    }
    else {
        cmp_config.data_format = DRV_PWM_CMP_DATA_32BITS;
    }
    pwm_dev->cmp_config.val = cmp_config.val;

    /* check the duty range */
    for(loop = 0; loop < DRV_PWM_CHN_TOTAL; loop++) {
        if(pwm_cfg->cmp_cfg[loop].duty > 100) {
            pwm_cfg->cmp_cfg[loop].duty = 100;
        }
    }

    /* save to context */
    pwm_simple_ctx->ovf_val = ovf_val;
    pwm_simple_ctx->align_mode = pwm_cfg->align_mode;

    /* config align mode */
    if(pwm_cfg->align_mode == DRV_PWM_EDGE_ALIGN_MODE) {

        /* single compare register */
        drv_pwm_dual_cmp_mode_set(pwm_dev, false);

        /* compare mode config */
        drv_pwm_edge_align_chn_set(pwm_dev, DRV_PWM_CHN_A, pwm_cfg->cmp_cfg[DRV_PWM_CHN_A].phase);
        /* save to context */
        pwm_simple_ctx->phase[DRV_PWM_CHN_A] = pwm_cfg->cmp_cfg[DRV_PWM_CHN_A].phase;
        /* duty: compare value */
        drv_pwm_edge_align_duty_set( pwm_dev, DRV_PWM_CHN_A, ovf_val,
                                     pwm_cfg->cmp_cfg[DRV_PWM_CHN_A].phase,
                                     pwm_cfg->cmp_cfg[DRV_PWM_CHN_A].duty );

        if((pwm_cfg->grp_num == DRV_PWM_CHN_A_B_WORK) || (pwm_cfg->grp_num == DRV_PWM_CHN_A_B_C_D_WORK)) {
            /* compare mode config */
            drv_pwm_edge_align_chn_set(pwm_dev, DRV_PWM_CHN_B, pwm_cfg->cmp_cfg[DRV_PWM_CHN_B].phase);
            /* save to context */
            pwm_simple_ctx->phase[DRV_PWM_CHN_B] = pwm_cfg->cmp_cfg[DRV_PWM_CHN_B].phase;
            /* duty: compare value */
            drv_pwm_edge_align_duty_set( pwm_dev, DRV_PWM_CHN_B, ovf_val,
                                         pwm_cfg->cmp_cfg[DRV_PWM_CHN_B].phase,
                                         pwm_cfg->cmp_cfg[DRV_PWM_CHN_B].duty );

        }

        if(pwm_cfg->grp_num == DRV_PWM_CHN_A_B_C_D_WORK) {
            /* compare mode config */
            drv_pwm_edge_align_chn_set(pwm_dev, DRV_PWM_CHN_C, pwm_cfg->cmp_cfg[DRV_PWM_CHN_C].phase);
            drv_pwm_edge_align_chn_set(pwm_dev, DRV_PWM_CHN_D, pwm_cfg->cmp_cfg[DRV_PWM_CHN_D].phase);
            /* save to context */
            pwm_simple_ctx->phase[DRV_PWM_CHN_C] = pwm_cfg->cmp_cfg[DRV_PWM_CHN_C].phase;
            pwm_simple_ctx->phase[DRV_PWM_CHN_D] = pwm_cfg->cmp_cfg[DRV_PWM_CHN_D].phase;
            /* duty: compare value */
            drv_pwm_edge_align_duty_set( pwm_dev, DRV_PWM_CHN_C, ovf_val,
                                         pwm_cfg->cmp_cfg[DRV_PWM_CHN_C].phase,
                                         pwm_cfg->cmp_cfg[DRV_PWM_CHN_C].duty );
            drv_pwm_edge_align_duty_set( pwm_dev, DRV_PWM_CHN_D, ovf_val,
                                         pwm_cfg->cmp_cfg[DRV_PWM_CHN_D].phase,
                                         pwm_cfg->cmp_cfg[DRV_PWM_CHN_D].duty );
        }
    }
    else {

        /* dual compare register */
        drv_pwm_dual_cmp_mode_set(pwm_dev, true);

        /* compare mode config */
        drv_pwm_center_align_chn_set(pwm_dev, DRV_PWM_CHN_A, pwm_cfg->cmp_cfg[DRV_PWM_CHN_A].phase);
        /* save to context */
        pwm_simple_ctx->phase[DRV_PWM_CHN_A] = pwm_cfg->cmp_cfg[DRV_PWM_CHN_A].phase;
        /* duty: compare value */
        drv_pwm_center_align_duty_set( pwm_dev, DRV_PWM_CHN_A, ovf_val,
                                       pwm_cfg->cmp_cfg[DRV_PWM_CHN_A].phase,
                                       pwm_cfg->cmp_cfg[DRV_PWM_CHN_A].duty );

        if((pwm_cfg->grp_num == DRV_PWM_CHN_A_B_WORK) || (pwm_cfg->grp_num == DRV_PWM_CHN_A_B_C_D_WORK)) {
            /* compare mode config */
            drv_pwm_center_align_chn_set(pwm_dev, DRV_PWM_CHN_B, pwm_cfg->cmp_cfg[DRV_PWM_CHN_B].phase);
            /* save to context */
            pwm_simple_ctx->phase[DRV_PWM_CHN_B] = pwm_cfg->cmp_cfg[DRV_PWM_CHN_B].phase;
            /* duty: compare value */
            drv_pwm_center_align_duty_set( pwm_dev, DRV_PWM_CHN_B, ovf_val,
                                           pwm_cfg->cmp_cfg[DRV_PWM_CHN_B].phase,
                                           pwm_cfg->cmp_cfg[DRV_PWM_CHN_B].duty );

        }

        if(pwm_cfg->grp_num == DRV_PWM_CHN_A_B_C_D_WORK) {
            /* compare mode config */
            drv_pwm_center_align_chn_set(pwm_dev, DRV_PWM_CHN_C, pwm_cfg->cmp_cfg[DRV_PWM_CHN_C].phase);
            drv_pwm_center_align_chn_set(pwm_dev, DRV_PWM_CHN_D, pwm_cfg->cmp_cfg[DRV_PWM_CHN_D].phase);
            /* save to context */
            pwm_simple_ctx->phase[DRV_PWM_CHN_C] = pwm_cfg->cmp_cfg[DRV_PWM_CHN_C].phase;
            pwm_simple_ctx->phase[DRV_PWM_CHN_D] = pwm_cfg->cmp_cfg[DRV_PWM_CHN_D].phase;
            /* duty: compare value */
            drv_pwm_center_align_duty_set( pwm_dev, DRV_PWM_CHN_C, ovf_val,
                                           pwm_cfg->cmp_cfg[DRV_PWM_CHN_C].phase,
                                           pwm_cfg->cmp_cfg[DRV_PWM_CHN_C].duty );
            drv_pwm_center_align_duty_set( pwm_dev, DRV_PWM_CHN_D, ovf_val,
                                           pwm_cfg->cmp_cfg[DRV_PWM_CHN_D].phase,
                                           pwm_cfg->cmp_cfg[DRV_PWM_CHN_D].duty );
        }
    }

    dprintf(DRV_PWM_DEBUG_LEVEL, "Simple PWM register init:\n");
    dprintf(DRV_PWM_DEBUG_LEVEL, "cnt_g0_config:0x%x, ", pwm_dev->cnt_g0_config.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cnt_g0_ovf:0x%x, ", pwm_dev->cnt_g0_ovf);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_config:0x%x, ", pwm_dev->cmp_config.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_ctrl:0x%x, ", pwm_dev->cmp_ctrl.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "dither_ctrl:0x%x\n", pwm_dev->dither_ctrl.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_a_cfg0:0x%x, cmp_a_cfg1:0x%x, ", pwm_dev->cmp_a_config0.val, pwm_dev->cmp_a_config1.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_b_cfg0:0x%x, cmp_b_cfg1:0x%x, ", pwm_dev->cmp_b_config0.val, pwm_dev->cmp_b_config1.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_c_cfg0:0x%x, cmp_c_cfg1:0x%x, ", pwm_dev->cmp_c_config0.val, pwm_dev->cmp_c_config1.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_d_cfg0:0x%x, cmp_d_cfg1:0x%x\n", pwm_dev->cmp_d_config0.val, pwm_dev->cmp_d_config1.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_a_val0:0x%x, cmp_a_val1:0x%x, ", pwm_dev->cmp0_a_val, pwm_dev->cmp1_a_val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_b_val0:0x%x, cmp_b_val1:0x%x, ", pwm_dev->cmp0_b_val, pwm_dev->cmp1_b_val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_c_val0:0x%x, cmp_c_val1:0x%x, ", pwm_dev->cmp0_c_val, pwm_dev->cmp1_c_val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_d_val0:0x%x, cmp_d_val1:0x%x\n", pwm_dev->cmp0_d_val, pwm_dev->cmp1_d_val);
}

/******************************************************************************
 ** \brief pwm open.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 *****************************************************************************/
void drv_pwm_cmp_dma_enable(sdrv_pwm_t* pwm_dev)
{
    sdrv_pwm_cmp_config_t cmp_config;

    cmp_config.val = pwm_dev->cmp_config.val;
    cmp_config.dma_en = 1;
    pwm_dev->cmp_config.val = cmp_config.val;
}

void drv_pwm_cmp_out_start(sdrv_pwm_t* pwm_dev)
{
    sdrv_pwm_cmp_ctrl_t cmp_ctrl;

    drv_pwm_force_output(pwm_dev, DRV_PWM_CHN_A, DRV_PWM_FORCE_OUT_DISABLE);
    drv_pwm_force_output(pwm_dev, DRV_PWM_CHN_B, DRV_PWM_FORCE_OUT_DISABLE);
    drv_pwm_force_output(pwm_dev, DRV_PWM_CHN_C, DRV_PWM_FORCE_OUT_DISABLE);
    drv_pwm_force_output(pwm_dev, DRV_PWM_CHN_D, DRV_PWM_FORCE_OUT_DISABLE);

    cmp_ctrl.val = pwm_dev->cmp_ctrl.val;
    cmp_ctrl.cmp_en = 1;
    pwm_dev->cmp_ctrl.val = cmp_ctrl.val;
}

/******************************************************************************
 ** \brief pwm close.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 *****************************************************************************/
void drv_pwm_cmp_dma_disable(sdrv_pwm_t* pwm_dev)
{
    sdrv_pwm_cmp_config_t cmp_config;

    cmp_config.val = pwm_dev->cmp_config.val;
    cmp_config.dma_en = 0;
    pwm_dev->cmp_config.val = cmp_config.val;
}

void drv_pwm_cmp_out_stop(sdrv_pwm_t* pwm_dev)
{
    sdrv_pwm_cmp_ctrl_t cmp_ctrl;

    cmp_ctrl.val = pwm_dev->cmp_ctrl.val;
    cmp_ctrl.cmp_en = 0;
    pwm_dev->cmp_ctrl.val = cmp_ctrl.val;

    drv_pwm_force_output(pwm_dev, DRV_PWM_CHN_A, DRV_PWM_FORCE_OUT_LOW);
    drv_pwm_force_output(pwm_dev, DRV_PWM_CHN_B, DRV_PWM_FORCE_OUT_LOW);
    drv_pwm_force_output(pwm_dev, DRV_PWM_CHN_C, DRV_PWM_FORCE_OUT_LOW);
    drv_pwm_force_output(pwm_dev, DRV_PWM_CHN_D, DRV_PWM_FORCE_OUT_LOW);
}

/******************************************************************************
 ** \brief Set the pwm duty, the unit is percent.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 ** \param [in] duty     pwm duty, unit percent.
 *****************************************************************************/
void drv_pwm_simple_duty_set( sdrv_pwm_t* pwm_dev,
                              drv_pwm_chn_t chn,
                              drv_pwm_simple_context_t* ctx,
                              uint8_t duty )
{
    if(ctx->align_mode == DRV_PWM_EDGE_ALIGN_MODE) {
        drv_pwm_edge_align_duty_set(pwm_dev, chn, ctx->ovf_val, ctx->phase[chn], duty);
    }
    else if(ctx->align_mode == DRV_PWM_CENTER_ALIGN_MODE) {
        drv_pwm_center_align_duty_set(pwm_dev, chn, ctx->ovf_val, ctx->phase[chn], duty);
    }
}

#if (DRV_PWM_PCM_TUNE_GET_METHOD == DRV_PWM_PCM_TUNE_TABLE_METHOD)
static bool drv_pwm_pcm_tune_table_get(uint32_t data_bits, uint32_t sample_freq, drv_pwm_pcm_tune_config_t* tune_cfg)
{
    uint32_t i, j;
    bool ret_val = false;

    for(i = 0; i < DRV_PWM_PCM_TABLE_DATA_FORMAT_NUM; i++) {
        if(drv_pwm_pcm_tune_table[i].data_bits == data_bits) {
            for(j = 0; j < DRV_PWM_PCM_TABLE_SAMPLE_FREQ_NUM; j++) {
                if(drv_pwm_pcm_tune_table[i].tune_para[j].sample_freq == sample_freq) {
                    tune_cfg->clk_div = drv_pwm_pcm_tune_table[i].tune_para[j].clk_div;
                    tune_cfg->ovf_val = drv_pwm_pcm_tune_table[i].tune_para[j].ovf_val;
                    tune_cfg->dither_en = drv_pwm_pcm_tune_table[i].tune_para[j].dither_en;
                    tune_cfg->dither_clip_rslt = drv_pwm_pcm_tune_table[i].tune_para[j].dither_clip_rslt;
                    tune_cfg->dither_drop = drv_pwm_pcm_tune_table[i].tune_para[j].dither_drop;
                    ret_val = true;
                    break;
                }
            }
        }
    }

    if(ret_val == false) {
        tune_cfg->clk_div = drv_pwm_pcm_tune_table[0].tune_para[0].clk_div;
        tune_cfg->ovf_val = drv_pwm_pcm_tune_table[0].tune_para[0].ovf_val;
        tune_cfg->dither_en = drv_pwm_pcm_tune_table[0].tune_para[0].dither_en;
        tune_cfg->dither_clip_rslt = drv_pwm_pcm_tune_table[0].tune_para[0].dither_clip_rslt;
        tune_cfg->dither_drop = drv_pwm_pcm_tune_table[0].tune_para[0].dither_drop;
        dprintf(ALWAYS, "PCM PWM not find the configure!\n");
    }

    dprintf(DRV_PWM_DEBUG_LEVEL, "PCM PWM tune table para get:\n");
    dprintf(DRV_PWM_DEBUG_LEVEL, "clk_div:%d, ", tune_cfg->clk_div);
    dprintf(DRV_PWM_DEBUG_LEVEL, "ovf_val:%d, ", tune_cfg->ovf_val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "dither_en:%d, ", tune_cfg->dither_en);
    dprintf(DRV_PWM_DEBUG_LEVEL, "dither_clip_rlt:%d!\n", tune_cfg->dither_clip_rslt);

    return ret_val;
}
#else
static void drv_pwm_pcm_tune_calc_get( uint32_t data_format,
                                       uint32_t sample_freq,
                                       drv_pwm_pcm_tune_config_t* tune_cfg )
{
    uint16_t clk_div = 0;
    uint32_t ovf_val = 0;
    uint32_t ovf_bits = 0;
    uint32_t div_bits = 0;
    uint32_t dither_bits = 0;

    ovf_val = ( DRV_PWM_HF_CLOCK_FREQ / (clk_div + 1) / sample_freq );
    ovf_bits = drv_pwm_none_zero_bits_index(ovf_val);
    if(ovf_bits < data_format) {   // dither input
        dither_bits = data_format - ovf_bits;
    }
    else {  // clock div
        div_bits = ovf_bits - data_format;
        clk_div = (clk_div + 1) * (1 << div_bits) - 1;
        ovf_val = ( DRV_PWM_HF_CLOCK_FREQ / (clk_div + 1) / sample_freq );
    }

    tune_cfg->clk_div = clk_div;
    tune_cfg->ovf_val = ovf_val;

    if(dither_bits > 0) {
        tune_cfg->dither_en = 1;
        tune_cfg->dither_clip_rslt = dither_bits - 1;
        tune_cfg->dither_drop = 0;
    }
    else {
        tune_cfg->dither_en = 0;
        tune_cfg->dither_clip_rslt = 0;
        tune_cfg->dither_drop = 0;
    }

    dprintf(DRV_PWM_DEBUG_LEVEL, "PCM PWM tune calc para get:\n");
    dprintf(DRV_PWM_DEBUG_LEVEL, "clk_div:%d, ", tune_cfg->clk_div);
    dprintf(DRV_PWM_DEBUG_LEVEL, "ovf_val:%d, ", tune_cfg->ovf_val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "dither_en:%d, ", tune_cfg->dither_en);
    dprintf(DRV_PWM_DEBUG_LEVEL, "dither_clip_rlt:%d!\n", tune_cfg->dither_clip_rslt);
}
#endif

/******************************************************************************
 ** \brief Audio pcm pwm init.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 ** \param [in] pwm_cfg  Pointer to the pwm configure parameter.
 *****************************************************************************/
void drv_pwm_pcm_init(sdrv_pwm_t* pwm_dev, drv_pwm_pcm_cfg_t* pcm_cfg)
{
    uint32_t ovf_val;
    drv_pwm_clk_cfg_t clk_cfg;
    sdrv_pwm_cmp_config_t cmp_cfg;
    sdrv_pwm_dither_ctrl_t dither_ctrl;
    uint32_t mfc_up;
    sdrv_pwm_sse_ctrl_t sse_ctrl;
    drv_pwm_pcm_tune_config_t tune_para;

    // Get the clk_div, ovf_val, dither parameter
#if (DRV_PWM_PCM_TUNE_GET_METHOD == DRV_PWM_PCM_TUNE_TABLE_METHOD)
    drv_pwm_pcm_tune_table_get(pcm_cfg->data_bits, pcm_cfg->sample_freq, &tune_para);
#else
    drv_pwm_pcm_tune_calc_get(pcm_cfg->data_bits, pcm_cfg->sample_freq, &tune_para);
#endif

    // Set the clock parameter
    clk_cfg.clk_src = DRV_PWM_SRC_CLK_HF;
    clk_cfg.clk_div = tune_para.clk_div;

    // Set the overflow parameter
    ovf_val = tune_para.ovf_val;

    // Set the cmp_config parameter
    cmp_cfg.dma_en = 0;
    cmp_cfg.fifo_wml = 0;
    cmp_cfg.rpt_num = 0;
    cmp_cfg.dual_cmp_mode = 0;
    if (pcm_cfg->drive_mode == DRV_PWM_PCM_DRIVE_MONO_CHANNEL) {
        cmp_cfg.grp_num = DRV_PWM_CHN_A_WORK;
    }
    else if (pcm_cfg->drive_mode == DRV_PWM_PCM_DRIVE_DUAL_CHANNEL) {
        cmp_cfg.grp_num = DRV_PWM_CHN_A_B_WORK;
    }
    else if (pcm_cfg->drive_mode == DRV_PWM_PCM_DRIVE_DUAL_H_BRIDGE) {
        cmp_cfg.grp_num = DRV_PWM_CHN_A_B_WORK;
    }
    else if (pcm_cfg->drive_mode == DRV_PWM_PCM_DRIVE_FOUR_H_BRIDGE) {
        cmp_cfg.grp_num = DRV_PWM_CHN_A_B_C_D_WORK;
    }
    else {
        cmp_cfg.grp_num = DRV_PWM_CHN_A_WORK;
    }
    if(pcm_cfg->data_bits == 8) {
        cmp_cfg.data_format = DRV_PWM_CMP_DATA_8BITS;
    }
    else if(pcm_cfg->data_bits == 16) {
        cmp_cfg.data_format = DRV_PWM_CMP_DATA_16BITS;
    }
    else if(pcm_cfg->data_bits == 32) {
        cmp_cfg.data_format = DRV_PWM_CMP_DATA_32BITS;
    }
    else {
        cmp_cfg.data_format = DRV_PWM_CMP_DATA_8BITS;
    }

    // Set the dither parameter
    dither_ctrl.dither_en = tune_para.dither_en;
    dither_ctrl.init_offset_en = 0;
    dither_ctrl.in_rslt = (pcm_cfg->data_bits / 8) - 1;
    dither_ctrl.drop = tune_para.dither_drop;
    dither_ctrl.clip_rslt = tune_para.dither_clip_rslt;
    dither_ctrl.init_offset = 0;

    // Set the dither parameter
    mfc_up = 0;

    // Set the dither parameter
    sse_ctrl.sse_en_a = 0;
    sse_ctrl.sse_en_b = 0;
    sse_ctrl.sse_en_c = 0;
    sse_ctrl.sse_en_d = 0;

    /* Step 1: software reset whole IP */
    drv_pwm_reset(pwm_dev);

    /* Step 2: select soucre clock and clock div */
    drv_pwm_clk_init(pwm_dev, clk_cfg.clk_src, clk_cfg.clk_div);

    /* Step 3.1: set dither config */
    pwm_dev->dither_ctrl.val = dither_ctrl.val;

    /* Step 3.2: set mfc up config */
    pwm_dev->mfc_ctrl = mfc_up;

    /* Step 3.3: set sse configure */
    pwm_dev->sse_ctrl.val = sse_ctrl.val;

    /* Step 4.1: set overflow value */
    pwm_dev->cnt_g0_ovf = ovf_val;

    /* Step 4.2: reload cnount g0 and update overflow value */
    drv_pwm_count_reset(pwm_dev);

    /* Step 5: set compare config */
    pwm_dev->cmp_config.val = cmp_cfg.val;

    /* Step 6: set compare out mode config */
    if (pcm_cfg->drive_mode == DRV_PWM_PCM_DRIVE_MONO_CHANNEL) {
        drv_pwm_edge_align_chn_set(pwm_dev, DRV_PWM_CHN_A, DRV_PWM_PHASE_POLARITY_POS);
    }
    else if (pcm_cfg->drive_mode == DRV_PWM_PCM_DRIVE_DUAL_CHANNEL) {
        drv_pwm_edge_align_chn_set(pwm_dev, DRV_PWM_CHN_A, DRV_PWM_PHASE_POLARITY_POS);
        drv_pwm_edge_align_chn_set(pwm_dev, DRV_PWM_CHN_B, DRV_PWM_PHASE_POLARITY_POS);
    }
    else if (pcm_cfg->drive_mode == DRV_PWM_PCM_DRIVE_DUAL_H_BRIDGE) {
        drv_pwm_edge_align_chn_set(pwm_dev, DRV_PWM_CHN_A, DRV_PWM_PHASE_POLARITY_POS);
        drv_pwm_edge_align_chn_set(pwm_dev, DRV_PWM_CHN_B, DRV_PWM_PHASE_POLARITY_NEG);
    }
    else if (pcm_cfg->drive_mode == DRV_PWM_PCM_DRIVE_FOUR_H_BRIDGE) {
        drv_pwm_edge_align_chn_set(pwm_dev, DRV_PWM_CHN_A, DRV_PWM_PHASE_POLARITY_POS);
        drv_pwm_edge_align_chn_set(pwm_dev, DRV_PWM_CHN_B, DRV_PWM_PHASE_POLARITY_NEG);
        drv_pwm_edge_align_chn_set(pwm_dev, DRV_PWM_CHN_C, DRV_PWM_PHASE_POLARITY_POS);
        drv_pwm_edge_align_chn_set(pwm_dev, DRV_PWM_CHN_D, DRV_PWM_PHASE_POLARITY_NEG);
    }

    /* Step 7: At least write one compare value for FIFO_ENTRY for DMA mode */
    pwm_dev->fifo_entry = 0;

    /* Step 8: If DMA mode use, configure FIFO_WML and set DMA_EN */
    cmp_cfg.val = pwm_dev->cmp_config.val;
    cmp_cfg.dma_en = 1;
    cmp_cfg.fifo_wml = DRV_PWM_FIFO_WML;
    pwm_dev->cmp_config.val = cmp_cfg.val;

    /* Step 9: set single mode/consecutive mode */
    drv_pwm_single_mode_set(pwm_dev, DRV_PWM_CONTINUE_CMP);

    dprintf(DRV_PWM_DEBUG_LEVEL, "PCM PWM register init:\n");
    dprintf(DRV_PWM_DEBUG_LEVEL, "cnt_g0_config:0x%x, ", pwm_dev->cnt_g0_config.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cnt_g0_ovf:0x%x, ", pwm_dev->cnt_g0_ovf);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_config:0x%x, ", pwm_dev->cmp_config.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_ctrl:0x%x, ", pwm_dev->cmp_ctrl.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "dither_ctrl:0x%x\n", pwm_dev->dither_ctrl.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_a_cfg0:0x%x, cmp_a_cfg1:0x%x, ", pwm_dev->cmp_a_config0.val, pwm_dev->cmp_a_config1.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_b_cfg0:0x%x, cmp_b_cfg1:0x%x, ", pwm_dev->cmp_b_config0.val, pwm_dev->cmp_b_config1.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_c_cfg0:0x%x, cmp_c_cfg1:0x%x, ", pwm_dev->cmp_c_config0.val, pwm_dev->cmp_c_config1.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_d_cfg0:0x%x, cmp_d_cfg1:0x%x\n", pwm_dev->cmp_d_config0.val, pwm_dev->cmp_d_config1.val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_a_val0:0x%x, cmp_a_val1:0x%x, ", pwm_dev->cmp0_a_val, pwm_dev->cmp1_a_val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_b_val0:0x%x, cmp_b_val1:0x%x, ", pwm_dev->cmp0_b_val, pwm_dev->cmp1_b_val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_c_val0:0x%x, cmp_c_val1:0x%x, ", pwm_dev->cmp0_c_val, pwm_dev->cmp1_c_val);
    dprintf(DRV_PWM_DEBUG_LEVEL, "cmp_d_val0:0x%x, cmp_d_val1:0x%x\n", pwm_dev->cmp0_d_val, pwm_dev->cmp1_d_val);
}

/******************************************************************************
 ** \brief pwm close.
 **
 ** \param [in] pwm_dev  Pointer to the pwm controller base address.
 *****************************************************************************/
void drv_pwm_force_output(sdrv_pwm_t* pwm_dev, drv_pwm_chn_t chn, drv_pwm_force_out_t force_out)
{
    sdrv_pwm_cmp_ch_config1_t cmp_cfg1_temp;
    uint32_t* cmp_cfg1_ptr;

    if (chn == DRV_PWM_CHN_A) {
        cmp_cfg1_ptr = (uint32_t*)&(pwm_dev->cmp_a_config1.val);
    }
    else if (chn == DRV_PWM_CHN_B) {
        cmp_cfg1_ptr = (uint32_t*)&(pwm_dev->cmp_b_config1.val);
    }
    else if (chn == DRV_PWM_CHN_C) {
        cmp_cfg1_ptr = (uint32_t*)&(pwm_dev->cmp_c_config1.val);
    }
    else if (chn == DRV_PWM_CHN_D) {
        cmp_cfg1_ptr = (uint32_t*)&(pwm_dev->cmp_d_config1.val);
    }
    else {
        return;
    }

    cmp_cfg1_temp.val = *cmp_cfg1_ptr;
    if(force_out == DRV_PWM_FORCE_OUT_DISABLE) {
        cmp_cfg1_temp.frc_high = 0;
        cmp_cfg1_temp.frc_low = 0;
    }
    else if(force_out == DRV_PWM_FORCE_OUT_HIGH) {
        cmp_cfg1_temp.frc_high = 1;
        cmp_cfg1_temp.frc_low = 0;
    }
    else if(force_out == DRV_PWM_FORCE_OUT_LOW) {
        cmp_cfg1_temp.frc_high = 0;
        cmp_cfg1_temp.frc_low = 1;
    }

    *cmp_cfg1_ptr = cmp_cfg1_temp.val;
}

void drv_pwm_int_enable(sdrv_pwm_t* pwm_dev, drv_pwm_int_src_t int_src)
{
    uint32_t int_sta_en;
    uint32_t int_sig_en;

    int_sta_en = pwm_dev->int_sta_en;
    int_sig_en = pwm_dev->int_sig_en;

    int_sta_en |= (1 << int_src);
    int_sig_en |= (1 << int_src);

    pwm_dev->int_sta_en = int_sta_en;
    pwm_dev->int_sig_en = int_sig_en;
}

void drv_pwm_int_disable(sdrv_pwm_t* pwm_dev, drv_pwm_int_src_t int_src)
{
    uint32_t int_sta_en;
    uint32_t int_sig_en;

    int_sta_en = pwm_dev->int_sta_en;
    int_sig_en = pwm_dev->int_sig_en;

    int_sta_en &= ~(1 << int_src);
    int_sig_en &= ~(1 << int_src);

    pwm_dev->int_sta_en = int_sta_en;
    pwm_dev->int_sig_en = int_sig_en;
}

enum handler_return drv_pwm_irq_handle( sdrv_pwm_t* pwm_dev,
                                                drv_pwm_int_cbk_t *int_cbk,
                                                void *handle )
{
    uint32_t int_sta = pwm_dev->int_sta;

    pwm_dev->int_sta = int_sta;

    if(int_sta & (1 << INT_STA_CMP_EVENT_FIELD_OFFSET)) {
        if(int_cbk->cmp_event_cbk != NULL) {
            int_cbk->cmp_event_cbk((void *)handle);
        }
    }
    if(int_sta & (1 << INT_STA_CNT_G0_OVF_FIELD_OFFSET)) {
        if(int_cbk->cnt_g0_ovf_cbk != NULL) {
            int_cbk->cnt_g0_ovf_cbk((void *)handle);
        }
    }
    if(int_sta & (1 << INT_STA_FIFO_UNDERRUN_FIELD_OFFSET)) {
        if(int_cbk->fifo_underrun_cbk != NULL) {
            int_cbk->fifo_underrun_cbk((void *)handle);
        }
    }

    return INT_NO_RESCHEDULE;
}


//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

