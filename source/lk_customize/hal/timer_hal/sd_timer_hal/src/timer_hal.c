/*
* hal_timer.c
*
* Copyright (c) 2018 Semidrive Semiconductor.
* All rights reserved.
*
* Description: implement semidrive timer HAL
*
* Revision History:
* -----------------
* 011, 11/23/2019 wang yongjun implement this
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

#include <assert.h>
#include <err.h>
#include <kernel/event.h>
#include <kernel/vm.h>
#include <platform.h>
#include <platform/debug.h>
#include <platform/interrupts.h>
#include <reg.h>
#include <stdlib.h>
#include <debug.h>
#include "__regs_base.h"
#include "irq.h"

#include "timer_hal.h"

/*timer global instance*/
timer_instance_t g_timer_instance[DEFAULT_TIMER_MAX_NUM] = {0};
spin_lock_t timer_spin_lock = SPIN_LOCK_INITIAL_VALUE;

static const hal_timer_addr_to_irq_t
timer_addr2irq_table[DEFAULT_TIMER_MAX_NUM] = {
    { APB_TIMER1_BASE, TIMER1_CNT_OVF_IRQ_NUM, TIMER1_CHN_A_IRQ_NUM },
#ifndef RES_EPU
    { APB_TIMER2_BASE, TIMER2_CNT_OVF_IRQ_NUM, TIMER2_CHN_A_IRQ_NUM },
    { APB_TIMER3_BASE, TIMER3_CNT_OVF_IRQ_NUM, TIMER3_CHN_A_IRQ_NUM },
    { APB_TIMER4_BASE, TIMER4_CNT_OVF_IRQ_NUM, TIMER4_CHN_A_IRQ_NUM },
    { APB_TIMER5_BASE, TIMER5_CNT_OVF_IRQ_NUM, TIMER5_CHN_A_IRQ_NUM },
    { APB_TIMER6_BASE, TIMER6_CNT_OVF_IRQ_NUM, TIMER6_CHN_A_IRQ_NUM },
    { APB_TIMER7_BASE, TIMER7_CNT_OVF_IRQ_NUM, TIMER7_CHN_A_IRQ_NUM },
    { APB_TIMER8_BASE, TIMER8_CNT_OVF_IRQ_NUM, TIMER8_CHN_A_IRQ_NUM },
#endif
};

static bool hal_timer_addr_to_irq(uint32_t addr, uint32_t *ovf_irq_num,
                                  uint32_t *fun_irq_num)
{
    uint32_t i;

    for (i = 0; i < DEFAULT_TIMER_MAX_NUM; i++) {
        if (timer_addr2irq_table[i].addr == addr) {
            *ovf_irq_num = timer_addr2irq_table[i].ovf_irq_num;
            *fun_irq_num = timer_addr2irq_table[i].fun_irq_num;
            return true;
        }
    }

    return false;
}

static hal_timer_err_t hal_timer_get_instance(uint32_t res_glb_idx,
        timer_instance_t **instance)
{
    uint32_t index = 0;
    addr_t phy_addr;
    int32_t phy_num;
    uint32_t ovf_irq_num = HAL_TIMER_INVALID_IRQ_NUM;
    uint32_t fun_irq_num = HAL_TIMER_INVALID_IRQ_NUM;
    hal_timer_err_t ret_value = HAL_TIMER_RES_ERR_NOT_FIND;
    spin_lock_saved_state_t states;

    spin_lock_irqsave(&timer_spin_lock, states);

    *instance = NULL;

    if (! (res_get_info_by_id(res_glb_idx, &phy_addr, &phy_num) < 0) ) {
        for (index = 0; index < DEFAULT_TIMER_MAX_NUM; index++) {
            if (!g_timer_instance[index].occupied
                    && (hal_timer_addr_to_irq(phy_addr, &ovf_irq_num, &fun_irq_num))) {
                g_timer_instance[index].occupied = true;
#if WITH_KERNEL_VM
                g_timer_instance[index].timer = (sdrv_timer_t *)((
                                                    uint64_t)paddr_to_kvaddr((addr_t)phy_addr));
#else
                g_timer_instance[index].timer = (sdrv_timer_t *)(phy_addr);
#endif

                g_timer_instance[index].ovf_irq_num = ovf_irq_num;
                g_timer_instance[index].fun_irq_num = fun_irq_num;
                ret_value = HAL_TIMER_RES_OK;
                *instance = &g_timer_instance[index];
                spin_unlock_irqrestore(&timer_spin_lock, states);
                return ret_value;

            }
            else {
                continue;
            }

        }

        ret_value = HAL_TIMER_RES_ERR_OCCUPIED;
    }
    else {
        ret_value = HAL_TIMER_RES_ERR_NOT_FIND;
    }

    spin_unlock_irqrestore(&timer_spin_lock, states);
    return ret_value;
}

static void hal_timer_release_instance(timer_instance_t *instance)
{
    instance->occupied = false;
}

bool hal_timer_creat_handle(void **handle, uint32_t res_glb_idx)
{
    timer_instance_t *instance = NULL;

    if (timer_spin_lock != SPIN_LOCK_INITIAL_VALUE) {
        spin_lock_init(&timer_spin_lock);
    }

    if (hal_timer_get_instance(res_glb_idx, &instance) != HAL_TIMER_RES_OK) {
        *handle = NULL;
        return false;
    }

    *handle = instance;
    return true;
}

bool hal_timer_release_handle(void *handle)
{
    timer_instance_t *instance = (timer_instance_t *)handle;
    spin_lock_saved_state_t states;

    spin_lock_irqsave(&timer_spin_lock, states);
    instance->occupied = false;
    spin_unlock_irqrestore(&timer_spin_lock, states);

    return true;
}

/******************************************************************************
 ** \brief Timer HAL global init.
 **
 *****************************************************************************/
void hal_timer_global_init(void *handle, hal_timer_glb_cfg_t *cfg)
{
    timer_instance_t *instance = (timer_instance_t *)handle;
    sdrv_timer_t *timer = instance->timer;

    if (handle == NULL)
        return;

    timer_drv_clk_init(timer, cfg->clk_sel, cfg->clk_div);
    timer_drv_cascade_set(timer, cfg->cascade);

    instance->cnt_per_ms = (cfg->clk_frq / (cfg->clk_div + 1)) / 1000;
    instance->cnt_per_us = instance->cnt_per_ms / 1000;
}

/******************************************************************************
 ** \brief Timer HAL sub overflow channel overflow init.
 **
 *****************************************************************************/
void hal_timer_ovf_init(void *handle, hal_timer_sub_t sub_cntr,
                        hal_timer_ovf_cfg_t *cfg)
{
    timer_instance_t *instance = (timer_instance_t *)handle;
    sdrv_timer_t *timer = instance->timer;

    if (handle == NULL)
        return;

    timer_drv_ovf_set(timer, (timer_drv_sub_t)sub_cntr, cfg->ovf_val);
    timer_drv_cntr_set(timer, (timer_drv_sub_t)sub_cntr, cfg->cnt_val);

    instance->drv_context.periodic[sub_cntr] = cfg->periodic;
}

/******************************************************************************
 ** \brief Timer HAL sub function channel function init.
 **
 *****************************************************************************/
void hal_timer_func_init(void *handle, hal_timer_func_ch_t sub_cntr,
                         hal_timer_fun_cfg_t *cfg)
{
    timer_instance_t *instance = (timer_instance_t *)handle;
    sdrv_timer_t *timer = instance->timer;
    timer_drv_func_t drv_func_cfg;

    if (handle == NULL)
        return;

    drv_func_cfg.func_type = cfg->func.func_type;

    if (cfg->func.func_type == HAL_TIMER_FUNC_TYPE_CPT) {
        drv_func_cfg.sub_func.cpt_cfg.cpt_cnt_sel =
            cfg->func.sub_func.cpt_cfg.cpt_cnt_sel;
        drv_func_cfg.sub_func.cpt_cfg.trig_mode =
            cfg->func.sub_func.cpt_cfg.trig_mode;
        drv_func_cfg.sub_func.cpt_cfg.dual_cpt_mode =
            cfg->func.sub_func.cpt_cfg.dual_cpt_mode;
        drv_func_cfg.sub_func.cpt_cfg.single_mode =
            cfg->func.sub_func.cpt_cfg.single_mode;
        drv_func_cfg.sub_func.cpt_cfg.filter_dis =
            cfg->func.sub_func.cpt_cfg.filter_dis;
        drv_func_cfg.sub_func.cpt_cfg.filter_width =
            cfg->func.sub_func.cpt_cfg.filter_width;
        drv_func_cfg.sub_func.cpt_cfg.first_cpt_rst_en =
            cfg->func.sub_func.cpt_cfg.first_cpt_rst_en;
    }
    else if (cfg->func.func_type == HAL_TIMER_FUNC_TYPE_CMP) {
        drv_func_cfg.sub_func.cmp_cfg.cmp_cnt_sel =
            cfg->func.sub_func.cmp_cfg.cmp_cnt_sel;
        drv_func_cfg.sub_func.cmp_cfg.cmp0_out_mode =
            cfg->func.sub_func.cmp_cfg.cmp0_out_mode;
        drv_func_cfg.sub_func.cmp_cfg.cmp1_out_mode =
            cfg->func.sub_func.cmp_cfg.cmp1_out_mode;
        drv_func_cfg.sub_func.cmp_cfg.cmp0_pulse_width =
            cfg->func.sub_func.cmp_cfg.cmp0_pulse_width;
        drv_func_cfg.sub_func.cmp_cfg.cmp1_pulse_width =
            cfg->func.sub_func.cmp_cfg.cmp1_pulse_width;
        drv_func_cfg.sub_func.cmp_cfg.dual_cmp_mode =
            cfg->func.sub_func.cmp_cfg.dual_cmp_mode;
        drv_func_cfg.sub_func.cmp_cfg.single_mode =
            cfg->func.sub_func.cmp_cfg.single_mode;
        drv_func_cfg.sub_func.cmp_cfg.cmp_rst_en =
            cfg->func.sub_func.cmp_cfg.cmp_rst_en;
        drv_func_cfg.sub_func.cmp_cfg.cmp0_val =
            cfg->func.sub_func.cmp_cfg.cmp0_val;
        drv_func_cfg.sub_func.cmp_cfg.cmp1_val =
            cfg->func.sub_func.cmp_cfg.cmp1_val;
    }

    drv_func_cfg.dma_ctrl.dma_enable = cfg->func.dma_ctrl.dma_enable;
    drv_func_cfg.dma_ctrl.dma_sel = cfg->func.dma_ctrl.dma_sel;
    drv_func_cfg.dma_ctrl.fifo_wml = cfg->func.dma_ctrl.fifo_wml;

    timer_drv_func_init(timer, sub_cntr, &drv_func_cfg);
}

/******************************************************************************
 ** \brief Timer HAL init.
 **
 *****************************************************************************/
void hal_timer_init(void *handle, hal_timer_cfg_t *cfg)
{
    timer_instance_t *instance = (timer_instance_t *)handle;
    sdrv_timer_t *timer = instance->timer;
    timer_drv_func_t drv_func_cfg;

    if (handle == NULL)
        return;

    //Set the clock
    timer_drv_clk_init(timer, cfg->glb_cfg.clk_sel, cfg->glb_cfg.clk_div);

    //Set the G0&G1 cascade or not
    timer_drv_cascade_set(timer, cfg->glb_cfg.cascade);

    instance->cnt_per_ms = (cfg->glb_cfg.clk_frq / (cfg->glb_cfg.clk_div + 1)) /
                           1000;
    instance->cnt_per_us = instance->cnt_per_ms / 1000;

    //Set the overflow value, counter value, interrupt enable, interrupt callback
    for (hal_timer_sub_t i = 0; i < HAL_TIMER_CONTER_TOTAL; i++) {
        timer_drv_ovf_set(timer, i, cfg->ovf_cfg[i].ovf_val);
        timer_drv_cntr_set(timer, i, cfg->ovf_cfg[i].cnt_val);

        instance->drv_context.periodic[i] = cfg->ovf_cfg[i].periodic;
    }

    //Set function, interrupt enable, interrupt callback
    for (hal_timer_func_ch_t j = 0; j < HAL_TIMER_FUNC_CH_TOTAL; j++) {
        drv_func_cfg.func_type = cfg->func_cfg[j].func.func_type;

        if (cfg->func_cfg[j].func.func_type == HAL_TIMER_FUNC_TYPE_CPT) {
            drv_func_cfg.sub_func.cpt_cfg.cpt_cnt_sel =
                cfg->func_cfg[j].func.sub_func.cpt_cfg.cpt_cnt_sel;
            drv_func_cfg.sub_func.cpt_cfg.trig_mode =
                cfg->func_cfg[j].func.sub_func.cpt_cfg.trig_mode;
            drv_func_cfg.sub_func.cpt_cfg.dual_cpt_mode =
                cfg->func_cfg[j].func.sub_func.cpt_cfg.dual_cpt_mode;
            drv_func_cfg.sub_func.cpt_cfg.single_mode =
                cfg->func_cfg[j].func.sub_func.cpt_cfg.single_mode;
            drv_func_cfg.sub_func.cpt_cfg.filter_dis =
                cfg->func_cfg[j].func.sub_func.cpt_cfg.filter_dis;
            drv_func_cfg.sub_func.cpt_cfg.filter_width =
                cfg->func_cfg[j].func.sub_func.cpt_cfg.filter_width;
            drv_func_cfg.sub_func.cpt_cfg.first_cpt_rst_en =
                cfg->func_cfg[j].func.sub_func.cpt_cfg.first_cpt_rst_en;
        }
        else if (cfg->func_cfg[j].func.func_type == HAL_TIMER_FUNC_TYPE_CMP) {
            drv_func_cfg.sub_func.cmp_cfg.cmp_cnt_sel =
                cfg->func_cfg[j].func.sub_func.cmp_cfg.cmp_cnt_sel;
            drv_func_cfg.sub_func.cmp_cfg.cmp0_out_mode =
                cfg->func_cfg[j].func.sub_func.cmp_cfg.cmp0_out_mode;
            drv_func_cfg.sub_func.cmp_cfg.cmp1_out_mode =
                cfg->func_cfg[j].func.sub_func.cmp_cfg.cmp1_out_mode;
            drv_func_cfg.sub_func.cmp_cfg.cmp0_pulse_width =
                cfg->func_cfg[j].func.sub_func.cmp_cfg.cmp0_pulse_width;
            drv_func_cfg.sub_func.cmp_cfg.cmp1_pulse_width =
                cfg->func_cfg[j].func.sub_func.cmp_cfg.cmp1_pulse_width;
            drv_func_cfg.sub_func.cmp_cfg.dual_cmp_mode =
                cfg->func_cfg[j].func.sub_func.cmp_cfg.dual_cmp_mode;
            drv_func_cfg.sub_func.cmp_cfg.single_mode =
                cfg->func_cfg[j].func.sub_func.cmp_cfg.single_mode;
            drv_func_cfg.sub_func.cmp_cfg.cmp_rst_en =
                cfg->func_cfg[j].func.sub_func.cmp_cfg.cmp_rst_en;
            drv_func_cfg.sub_func.cmp_cfg.cmp0_val =
                cfg->func_cfg[j].func.sub_func.cmp_cfg.cmp0_val;
            drv_func_cfg.sub_func.cmp_cfg.cmp1_val =
                cfg->func_cfg[j].func.sub_func.cmp_cfg.cmp1_val;
        }

        drv_func_cfg.dma_ctrl.dma_enable =
            cfg->func_cfg[j].func.dma_ctrl.dma_enable;
        drv_func_cfg.dma_ctrl.dma_sel = cfg->func_cfg[j].func.dma_ctrl.dma_sel;
        drv_func_cfg.dma_ctrl.fifo_wml = cfg->func_cfg[j].func.dma_ctrl.fifo_wml;

        timer_drv_func_init(timer, j, &drv_func_cfg);
    }
}

/******************************************************************************
 ** \brief Timer HAL sse capture init.
 **
 *****************************************************************************/
void hal_timer_sse_cpt_init(      void *handle, hal_timer_func_ch_t cpt_ch,
                                  hal_timer_func_ch_t cmp_ch)
{
    if (handle == NULL)
        return;

    timer_instance_t *instance = (timer_instance_t *)handle;
    hal_timer_sse_cpt_init(instance->timer, cpt_ch, cmp_ch);
}

/******************************************************************************
 ** \brief Timer HAL sse compare init.
 **
 *****************************************************************************/
void hal_timer_sse_cmp_init(      void *handle, hal_timer_func_ch_t cmp_ch)
{
    if (handle == NULL)
        return;

    timer_instance_t *instance = (timer_instance_t *)handle;
    hal_timer_sse_cmp_init(instance->timer, cmp_ch);
}

/******************************************************************************
 ** \brief Timer HAL force compare output.
 **
 *****************************************************************************/
void hal_timer_cmp_force_out(void *handle, hal_timer_func_ch_t func_ch,
                             bool enable, bool level)
{
    if (handle == NULL)
        return;

    timer_instance_t *instance = (timer_instance_t *)handle;
    timer_drv_cmp_force_out(instance->timer, func_ch, enable, level);
}

/******************************************************************************
 ** \brief Timer HAL capture value get.
 **
 *****************************************************************************/
uint32_t hal_timer_cpt_value_get(void *handle, hal_timer_func_ch_t func_ch)
{
    if (handle == NULL)
        return 0;

    timer_instance_t *instance = (timer_instance_t *)handle;
    return timer_drv_cpt_value_get(instance->timer, func_ch);
}

/******************************************************************************
 ** \brief Timer HAL capture value get.
 **
 *****************************************************************************/
void hal_timer_cmp_value_set(void *handle, hal_timer_func_ch_t func_ch,
                             uint32_t value0, uint32_t value1)
{
    if (handle == NULL)
        return;

    timer_instance_t *instance = (timer_instance_t *)handle;
    timer_drv_cmp_value_set(instance->timer, func_ch, value0, value1);
}

/******************************************************************************
 ** \brief Enable compare function of the channel.
 **
 ** \param [in]    handle    Pointer to the handle create
 ** \param [in]    func_ch   Sub function channel
 *****************************************************************************/
void hal_timer_func_cmp_enable(void *handle, hal_timer_func_ch_t func_ch)
{
    if (handle == NULL)
        return;

    timer_instance_t *instance = (timer_instance_t *)handle;
    timer_drv_func_cmp_enable(instance->timer, func_ch);
}

/******************************************************************************
 ** \brief Disable compare function of the channel.
 **
 ** \param [in]    handle    Pointer to the handle create
 ** \param [in]    func_ch   Sub function channel
 *****************************************************************************/
void hal_timer_func_cmp_disable(void *handle, hal_timer_func_ch_t func_ch)
{
    if (handle == NULL)
        return;

    timer_instance_t *instance = (timer_instance_t *)handle;
    timer_drv_func_cmp_disable(instance->timer, func_ch);
}

/******************************************************************************
 ** \brief Enable capture function of the channel.
 **
 ** \param [in]    handle    Pointer to the handle create
 ** \param [in]    func_ch   Sub function channel
 *****************************************************************************/
void hal_timer_func_cpt_enable(void *handle, hal_timer_func_ch_t func_ch)
{
    if (handle == NULL)
        return;

    timer_instance_t *instance = (timer_instance_t *)handle;
    timer_drv_func_cpt_enable(instance->timer, func_ch);
}

/******************************************************************************
 ** \brief Disable capture function of the channel.
 **
 ** \param [in]    handle    Pointer to the handle create
 ** \param [in]    func_ch   Sub function channel
 *****************************************************************************/
void hal_timer_func_cpt_disable(void *handle, hal_timer_func_ch_t func_ch)
{
    if (handle == NULL)
        return;

    timer_instance_t *instance = (timer_instance_t *)handle;
    timer_drv_func_cpt_enable(instance->timer, func_ch);
}

/******************************************************************************
 ** \brief Enable one of the interrupt bits.
 **
 ** \param [in]    arg    Pointer to the handle create
 *****************************************************************************/
void hal_timer_int_src_enable(void *handle, hal_timer_int_src_t int_src)
{
    if (handle == NULL)
        return;

    timer_instance_t *instance = (timer_instance_t *)handle;
    timer_drv_int_sta_enable(instance->timer, int_src);
    timer_drv_int_sig_enable(instance->timer, int_src);
}

/******************************************************************************
 ** \brief Disable one of the interrupt bits.
 **
 ** \param [in]    arg    Pointer to the handle create
 *****************************************************************************/
void hal_timer_int_src_disable(void *handle, hal_timer_int_src_t int_src)
{
    if (handle == NULL)
        return;

    timer_instance_t *instance = (timer_instance_t *)handle;
    timer_drv_int_sta_disable(instance->timer, int_src);
    timer_drv_int_sig_disable(instance->timer, int_src);
}

/******************************************************************************
 ** \brief Overflow IRQ handle.
 **
 ** \param [in]    arg    Pointer to the handle create
 *****************************************************************************/
static enum handler_return hal_timer_ovf_irq_handle(void *arg)
{
    timer_instance_t *instance = (timer_instance_t *)arg;

    if (instance == NULL)
        return INT_NO_RESCHEDULE;

    return timer_drv_ovf_irq_handle(instance->timer, &(instance->drv_context));
}

/******************************************************************************
 ** \brief Function IRQ handle.
 **
 ** \param [in]    arg    Pointer to the handle create
 *****************************************************************************/
static enum handler_return hal_timer_func_irq_handle(void *arg)
{
    timer_instance_t *instance = (timer_instance_t *)arg;

    if (instance == NULL)
        return INT_NO_RESCHEDULE;

    return timer_drv_func_irq_handle(instance->timer,
                                     &(instance->drv_context));
}

void hal_timer_int_cbk_register(void *handle, hal_timer_int_src_t int_src,
                                hal_timer_int_cbk cbk)
{
    if (handle == NULL)
        return;

    timer_instance_t *instance = (timer_instance_t *)handle;

    if ((int_src >= HAL_TIMER_CNT_G0_OVF_INT_SRC)
            && (int_src <= HAL_TIMER_CNT_G1_OVF_INT_SRC)) {
        instance->drv_context.global_ovf_cbk[int_src -
                                                     HAL_TIMER_CNT_G0_OVF_INT_SRC] = cbk;

        if (instance->ovf_irq_num != HAL_TIMER_INVALID_IRQ_NUM) {
            register_int_handler(instance->ovf_irq_num, hal_timer_ovf_irq_handle,
                                 handle);
            unmask_interrupt(instance->ovf_irq_num);
        }
    }
    else if ((int_src >= HAL_TIMER_CNT_LA_OVF_INT_SRC)
             && (int_src <= HAL_TIMER_CNT_LD_OVF_INT_SRC)) {
        instance->drv_context.local_ovf_cbk[int_src - HAL_TIMER_CNT_LA_OVF_INT_SRC]
            = cbk;

        if (instance->fun_irq_num != HAL_TIMER_INVALID_IRQ_NUM) {
            register_int_handler(instance->fun_irq_num, hal_timer_func_irq_handle,
                                 handle);
            unmask_interrupt(instance->fun_irq_num);
        }
    }
    else if ((int_src >= HAL_TIMER_CPT_A_INT_SRC)
             && (int_src <= HAL_TIMER_CPT_D_INT_SRC)) {
        instance->drv_context.local_cpt_cbk[int_src - HAL_TIMER_CPT_A_INT_SRC] =
            cbk;

        if (instance->fun_irq_num != HAL_TIMER_INVALID_IRQ_NUM) {
            register_int_handler(instance->fun_irq_num, hal_timer_func_irq_handle,
                                 handle);
            unmask_interrupt(instance->fun_irq_num);
        }
    }
    else if ((int_src >= HAL_TIMER_CMP_A_INT_SRC)
             && (int_src <= HAL_TIMER_CMP_D_INT_SRC)) {
        instance->drv_context.local_cmp_cbk[int_src - HAL_TIMER_CMP_A_INT_SRC] =
            cbk;

        if (instance->fun_irq_num != HAL_TIMER_INVALID_IRQ_NUM) {
            register_int_handler(instance->fun_irq_num, hal_timer_func_irq_handle,
                                 handle);
            unmask_interrupt(instance->fun_irq_num);
        }
    }
    else if ((int_src >= HAL_TIMER_FIFO_A_UNDERRUN_INT_SRC)
             && (int_src <= HAL_TIMER_FIFO_D_UNDERRUN_INT_SRC)) {
        instance->drv_context.local_underrun_cbk[int_src -
                        HAL_TIMER_FIFO_A_UNDERRUN_INT_SRC] = cbk;

        if (instance->fun_irq_num != HAL_TIMER_INVALID_IRQ_NUM) {
            register_int_handler(instance->fun_irq_num, hal_timer_func_irq_handle,
                                 handle);
            unmask_interrupt(instance->fun_irq_num);
        }
    }
    else if ((int_src >= HAL_TIMER_FIFO_A_OVERRUN_INT_SRC)
             && (int_src <= HAL_TIMER_FIFO_D_OVERRUN_INT_SRC)) {
        instance->drv_context.local_overrun_cbk[int_src -
                                                        HAL_TIMER_FIFO_A_OVERRUN_INT_SRC] = cbk;

        if (instance->fun_irq_num != HAL_TIMER_INVALID_IRQ_NUM) {
            register_int_handler(instance->fun_irq_num, hal_timer_func_irq_handle,
                                 handle);
            unmask_interrupt(instance->fun_irq_num);
        }
    }
}

/******************************************************************************
 ** \brief Clear the interrupt status flag
 **
 ** \param [in]    handle    Pointer to the hanld create
 ** \param [in]    offset    Offset of the interrupt status flag
 *****************************************************************************/
void hal_timer_int_sta_clear(void *handle, hal_timer_int_src_t int_src)
{
    if (handle == NULL)
        return;

    timer_instance_t *instance = (timer_instance_t *)handle;
    timer_drv_int_sta_clear(instance->timer, int_src);
}

/******************************************************************************
 ** \brief Get the sun counter value
 **
 ** \param [in]    handle       Pointer to the hanld create
 ** \param [in]    sub_timer    Sub counter of the timer
 *****************************************************************************/
uint64_t hal_timer_glb_cntr_get(void *handle)
{
    uint32_t g0_val, g1_val, g0_new;
    spin_lock_saved_state_t state;
    timer_instance_t *instance = (timer_instance_t *)handle;

    if (handle == NULL)
        return 0;

    arch_interrupt_save(&state, SPIN_LOCK_FLAG_INTERRUPTS);

    g0_new = timer_drv_cntr_get(instance->timer, HAL_TIMER_G0);
    do {
        g0_val = g0_new;
        g1_val = timer_drv_cntr_get(instance->timer, HAL_TIMER_G1);
        g0_new = timer_drv_cntr_get(instance->timer, HAL_TIMER_G0);
    } while (g0_val > g0_new);

    arch_interrupt_restore(state, SPIN_LOCK_FLAG_INTERRUPTS);

    return ((uint64_t)g0_val | (((uint64_t)g1_val) << 32U));
}

/******************************************************************************
 ** \brief Timer HAL global init.
 **
 *****************************************************************************/
uint64_t hal_timer_ms_to_cntr(void *handle, uint32_t ms)
{
    timer_instance_t *instance = (timer_instance_t *)handle;

    if (handle == NULL)
        return 0xFFFFFFFFFFFFFFFF;

    return (ms * instance->cnt_per_ms);
}

uint32_t hal_timer_cntr_to_us(void* handle, uint32_t cntr)
{
    timer_instance_t *instance = (timer_instance_t *)handle;

    if (handle == NULL)
        return 0;

    return (cntr / instance->cnt_per_us);
}

uint32_t hal_timer_cntr_to_ms(void* handle, uint32_t cntr)
{
    timer_instance_t *instance = (timer_instance_t *)handle;

    if (handle == NULL)
        return 0;

    return (cntr / instance->cnt_per_ms);
}

uint32_t hal_timer_timer_cpt_value_get(void *handle, hal_timer_func_ch_t func_ch)
{
    timer_instance_t *instance = (timer_instance_t *)handle;
    uint32_t ret = 0;
    switch(func_ch)
    {
    case HAL_TIMER_FUNC_CH_A:
        ret = instance->timer->fifo_a;
    break;
    case HAL_TIMER_FUNC_CH_B:
        ret = instance->timer->fifo_b;
    break;
    case HAL_TIMER_FUNC_CH_C:
        ret = instance->timer->fifo_c;
    break;
    case HAL_TIMER_FUNC_CH_D:
        ret = instance->timer->fifo_d;
    break;

    default:
        ret = 0;
    break;

    }

    return ret;
}

uint32_t hal_timer_cpt_get_fifo_items_num(void *handle, hal_timer_func_ch_t func_ch)
{
    timer_instance_t *instance = (timer_instance_t *)handle;

    return (uint8_t)(((instance->timer->fifo_sta >> (func_ch * 8)) & 0x7C) >> 2);
}


//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

