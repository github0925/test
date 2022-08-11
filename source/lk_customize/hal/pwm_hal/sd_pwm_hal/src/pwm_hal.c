/*
* pwm_hal.c
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

#include <platform/interrupts.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include <kernel/thread.h>
#include <kernel/vm.h>
#include <kernel/spinlock.h>
#include <irq.h>
#include "pwm_drv.h"
#include "pwm_hal.h"
#if ENABLE_SD_DMA
#include "dma_hal.h"
#endif


typedef struct {
    uint32_t addr;
    uint32_t irq_num;
    uint32_t dma_chan_type;
} hal_pwm_addr_to_irq_t;

typedef enum {
    HAL_PWM_PCM_PLAY_IDLE_STATUS = 0,
    HAL_PWM_PCM_PLAY_PROC_STATUS,
    HAL_PWM_PCM_PLAY_TERMINATE_STATUS,
    HAL_PWM_PCM_PLAY_STOP_STATUS,
} hal_pwm_pcm_play_status_t;

/*pwm global instance*/
pwm_instance_t g_pwm_instance[DEFAULT_PWM_MAX_NUM] = { 0 };
spin_lock_t pwm_spin_lock = SPIN_LOCK_INITIAL_VALUE;

#if ENABLE_SD_DMA
static const hal_pwm_addr_to_irq_t pwm_addr2irq_table[DEFAULT_PWM_MAX_NUM] = {
    { APB_PWM1_BASE, PWM1_PWM_IRQ_NUM, DMA_PERI_PWM1 },
    { APB_PWM2_BASE, PWM2_PWM_IRQ_NUM, DMA_PERI_PWM2 },
    { APB_PWM3_BASE, PWM3_PWM_IRQ_NUM, DMA_PERI_PWM3 },
    { APB_PWM4_BASE, PWM4_PWM_IRQ_NUM, DMA_PERI_PWM4 },
    { APB_PWM5_BASE, PWM5_PWM_IRQ_NUM, DMA_PERI_PWM5 },
    { APB_PWM6_BASE, PWM6_PWM_IRQ_NUM, DMA_PERI_PWM6 },
    { APB_PWM7_BASE, PWM7_PWM_IRQ_NUM, DMA_PERI_PWM7 },
    { APB_PWM8_BASE, PWM8_PWM_IRQ_NUM, DMA_PERI_PWM8 },
};
#else
static const hal_pwm_addr_to_irq_t pwm_addr2irq_table[DEFAULT_PWM_MAX_NUM] = {
    { APB_PWM1_BASE, PWM1_PWM_IRQ_NUM, 0 },
    { APB_PWM2_BASE, PWM2_PWM_IRQ_NUM, 0 },
    { APB_PWM3_BASE, PWM3_PWM_IRQ_NUM, 0 },
    { APB_PWM4_BASE, PWM4_PWM_IRQ_NUM, 0 },
    { APB_PWM5_BASE, PWM5_PWM_IRQ_NUM, 0 },
    { APB_PWM6_BASE, PWM6_PWM_IRQ_NUM, 0 },
    { APB_PWM7_BASE, PWM7_PWM_IRQ_NUM, 0 },
    { APB_PWM8_BASE, PWM8_PWM_IRQ_NUM, 0 },
};
#endif
/******************************************************************************
 ** \brief Translate the pwm controller base address to irq number
 **
 ** \param [in]
 *****************************************************************************/
static bool hal_pwm_addr_to_irq(uint32_t addr, uint32_t *irq_num, uint32_t *dma_ch_type)
{
    uint32_t i;

    for (i = 0; i < DEFAULT_PWM_MAX_NUM; i++) {
        if (pwm_addr2irq_table[i].addr == addr) {
            *irq_num = pwm_addr2irq_table[i].irq_num;
            *dma_ch_type = pwm_addr2irq_table[i].dma_chan_type;
            return true;
        }
    }

    return false;
}

/******************************************************************************
 ** \brief Get the instance of pwm
 **
 ** \param [in]   res_glb_idx      global resource index
 ** \param [out]  instance         pointer of the instance get
 ** \return       pwm_hal_err_t   result of get instance
 *****************************************************************************/
static pwm_instance_t* hal_pwm_get_instance(uint32_t res_glb_idx)
{
    uint32_t index = 0;
    addr_t phy_addr;
    int32_t phy_num;
    int32_t res_rst;
    uint32_t irq_num = HAL_PWM_IRQ_NUM_INVALID;
    uint32_t dma_ch_type;
    spin_lock_saved_state_t states;

    spin_lock_irqsave(&pwm_spin_lock, states);

    res_rst = res_get_info_by_id(res_glb_idx, &phy_addr, &phy_num);
    if(res_rst < 0) {
        return NULL;
    }

    for (index = 0; index < DEFAULT_PWM_MAX_NUM; index++) {
        if ((!g_pwm_instance[index].occupied) && (hal_pwm_addr_to_irq(phy_addr, &irq_num, &dma_ch_type))) {
            g_pwm_instance[index].occupied = true;
#if WITH_KERNEL_VM
            g_pwm_instance[index].pwmc = (sdrv_pwm_t *)((uint64_t)paddr_to_kvaddr((addr_t)phy_addr));
#else
            g_pwm_instance[index].pwmc = (sdrv_pwm_t *)(phy_addr);
#endif
            g_pwm_instance[index].irq_num = irq_num;
            g_pwm_instance[index].phy_num = phy_num;
            g_pwm_instance[index].dma_chan_type = dma_ch_type;
            spin_unlock_irqrestore(&pwm_spin_lock, states);
            return &g_pwm_instance[index];
        }
    }

    spin_unlock_irqrestore(&pwm_spin_lock, states);
    return NULL;
}

/******************************************************************************
 ** \brief Release the instance of pwm
 **
 ** \param [out]  instance         pointer of the instance get
 *****************************************************************************/
static void hal_pwm_release_instance(pwm_instance_t *instance)
{
    instance->occupied = false;
}

/******************************************************************************
 ** \brief Create the handle of pwm
 **
 ** \param [in]   res_glb_idx      global resource index
 ** \param [out]  handle           pointer of the handle create
 ** \return       bool             result of get instance
 *****************************************************************************/
bool hal_pwm_creat_handle(void **handle, uint32_t res_glb_idx)
{
    pwm_instance_t *pwm_inst = NULL;

    if (pwm_spin_lock != SPIN_LOCK_INITIAL_VALUE) {
        spin_lock_init(&pwm_spin_lock);
    }

    pwm_inst = hal_pwm_get_instance(res_glb_idx);

    if(pwm_inst == NULL) {
        *handle = NULL;
        return false;
    }

    *handle = pwm_inst;
    return true;
}

/******************************************************************************
 ** \brief Release the handle of pwm
 **
 ** \param [out]  handle           pointer of the handle create
 ** \return       bool             result of get instance
 *****************************************************************************/
bool hal_pwm_release_handle(void *handle)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;
    spin_lock_saved_state_t states;

    if(handle == NULL) {
        return false;
    }

    spin_lock_irqsave(&pwm_spin_lock, states);
    pwm_inst->occupied = false;
    spin_unlock_irqrestore(&pwm_spin_lock, states);

    return true;
}

void hal_pwm_simple_init(void *handle, hal_pwm_simple_cfg_t* pwm_cfg)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;
    drv_pwm_simple_cfg_t drv_cfg;
    uint32_t index;

    if(handle == NULL) {
        return;
    }

    //simple pwm frequency
    drv_cfg.freq = pwm_cfg->freq;
    //group number
    drv_cfg.grp_num = pwm_cfg->grp_num;
    //single one shot mode
    drv_cfg.single_mode = pwm_cfg->single_mode;
    //pwm wave edge align or center align mode
    drv_cfg.align_mode = pwm_cfg->align_mode;
    //sub-channel compare configure
    for(index = 0; index < HAL_PWM_CHN_TOTAL; index++) {
        drv_cfg.cmp_cfg[index].duty = pwm_cfg->cmp_cfg[index].duty;
        drv_cfg.cmp_cfg[index].phase = pwm_cfg->cmp_cfg[index].phase;
    }

    drv_pwm_simple_init(pwm_inst->pwmc, &drv_cfg, &(pwm_inst->pwm_simple_ctx));
}

void hal_pwm_simple_start(void *handle)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;

    if(handle == NULL) {
        return;
    }

    drv_pwm_cmp_out_start(pwm_inst->pwmc);
}

void hal_pwm_simple_stop(void *handle)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;

    if(handle == NULL) {
        return;
    }

    drv_pwm_cmp_out_stop(pwm_inst->pwmc);
}

#if ENABLE_SD_DMA
static void hal_pwm_pcm_dma_cfg(void *handle)
{
    struct dma_dev_cfg dma_cfg;
    struct dma_chan *tx_chan;

    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;
    if(handle == NULL) {
        return;
    }

    dma_cfg.direction = DMA_MEM2DEV;
    dma_cfg.src_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    dma_cfg.dst_addr_width = DMA_DEV_BUSWIDTH_4_BYTES;
    dma_cfg.src_maxburst = DMA_BURST_TR_4ITEMS;
    dma_cfg.dst_maxburst = DMA_BURST_TR_4ITEMS;

    dma_cfg.dst_addr = (addr_t)(&(pwm_inst->pwmc->fifo_entry));

    tx_chan = hal_dma_chan_req(pwm_inst->dma_chan_type);

    if (tx_chan) {
        hal_dma_dev_config(tx_chan, &dma_cfg);
        pwm_inst->pwm_pcm_ctx.tx_chan = tx_chan;
        dprintf(HAL_PWM_DEBUG_LEVEL, "PCM PWM DMA cfg success!\n");
    }
    else {
        pwm_inst->pwm_pcm_ctx.tx_chan = NULL;
        dprintf(HAL_PWM_DEBUG_LEVEL, "PCM PWM DMA cfg fail!\n");
    }
}
#endif

static void hal_pwm_pcm_fifo_underrun_cbk(void *arg)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)arg;

    pwm_inst->pwmc->fifo_entry = 0;
    //drv_pwm_cmp_out_stop(pwm_inst->pwmc);
    drv_pwm_int_disable(pwm_inst->pwmc, HAL_PWM_INT_SRC_FIFO_UNDERRUN);
    dprintf(HAL_PWM_DEBUG_LEVEL, "PWM%d fifo underrun!\n", pwm_inst->phy_num);
}

void hal_pwm_pcm_init(void *handle, hal_pwm_pcm_cfg_t* pcm_cfg)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;
    drv_pwm_pcm_cfg_t drv_cfg;

    if(handle == NULL) {
        return;
    }

    pwm_inst->pwm_pcm_ctx.data_size = 0;
    pwm_inst->pwm_pcm_ctx.data_size_shadow = 0;
    pwm_inst->pwm_pcm_ctx.tx_chan = NULL;
    pwm_inst->pwm_pcm_ctx.tx_desc = NULL;
    pwm_inst->pwm_pcm_ctx.play_status = DRV_PWM_PCM_PLAY_IDLE_STATUS;
    pwm_inst->pwm_pcm_ctx.play_complete_cbk = pcm_cfg->play_complete_cbk;
    pwm_inst->pwm_pcm_ctx.data_bits = pcm_cfg->data_bits;

    //pcm data sample frequency
    drv_cfg.sample_freq = pcm_cfg->sample_freq;
    //pcm data format
    drv_cfg.data_bits = pcm_cfg->data_bits;
    //pcm output driver mode
    drv_cfg.drive_mode = pcm_cfg->drive_mode;
    //play complete callback
    drv_cfg.play_complete_cbk = pcm_cfg->play_complete_cbk;

    drv_pwm_pcm_init(pwm_inst->pwmc, &drv_cfg);
    drv_pwm_cmp_dma_disable(pwm_inst->pwmc);
    drv_pwm_cmp_out_stop(pwm_inst->pwmc);
#if ENABLE_SD_DMA
    hal_pwm_pcm_dma_cfg(handle);
#endif
    hal_pwm_int_cbk_register(handle, HAL_PWM_INT_SRC_FIFO_UNDERRUN, hal_pwm_pcm_fifo_underrun_cbk);
    drv_pwm_int_disable(pwm_inst->pwmc, HAL_PWM_INT_SRC_FIFO_UNDERRUN);
    register_int_handler(pwm_inst->irq_num, hal_pwm_irq_handle, handle);
    unmask_interrupt(pwm_inst->irq_num);
}

void hal_pwm_pcm_cfg_update(void *handle, hal_pwm_pcm_cfg_t* pcm_cfg)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;
    drv_pwm_pcm_cfg_t drv_cfg;

    //pcm data sample frequency
    drv_cfg.sample_freq = pcm_cfg->sample_freq;
    //pcm data format
    drv_cfg.data_bits = pcm_cfg->data_bits;
    //pcm output driver mode
    drv_cfg.drive_mode = pcm_cfg->drive_mode;

    drv_pwm_pcm_init(pwm_inst->pwmc, &drv_cfg);
}

#if ENABLE_SD_DMA
static void pcm_pwm_dma_tx_irq_evt_handle(enum dma_status status, uint32_t err, void *ctx)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)ctx;

    if(status == DMA_COMP) {
        dprintf(HAL_PWM_DEBUG_LEVEL, "PCM PWM DMA transfer completed!\n");
        pwm_inst->pwm_pcm_ctx.play_status = DRV_PWM_PCM_PLAY_IDLE_STATUS;
        pwm_inst->pwm_pcm_ctx.data_size_shadow = 0;
        drv_pwm_int_enable(pwm_inst->pwmc, HAL_PWM_INT_SRC_FIFO_UNDERRUN);
        if(pwm_inst->pwm_pcm_ctx.play_complete_cbk) {
            pwm_inst->pwm_pcm_ctx.play_complete_cbk(pwm_inst);
        }
    }
    else {
        dprintf(HAL_PWM_DEBUG_LEVEL, "PCM PWM DMA transfer err, status:%d!\n", status);
        pwm_inst->pwm_pcm_ctx.play_status = DRV_PWM_PCM_PLAY_IDLE_STATUS;
        dprintf(HAL_PWM_DEBUG_LEVEL, "PCM PWM DMA re-transfer!\n");
        hal_pwm_pcm_play_start(pwm_inst, pwm_inst->pwm_pcm_ctx.data_addr, pwm_inst->pwm_pcm_ctx.data_size);
    }
}
#endif

void hal_pwm_pcm_play_start(void *handle, uint8_t* data_addr, uint32_t data_size)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;

    if(handle == NULL) {
        dprintf(HAL_PWM_DEBUG_LEVEL, "PCM PWM handle is NULL!\n");
        return;
    }

    if(data_size == 0) {
        dprintf(HAL_PWM_DEBUG_LEVEL, "PCM PWM play data size is 0!\n");
        return;
    }

    /* save to pwm pcm context */
    pwm_inst->pwm_pcm_ctx.data_addr = data_addr;
    pwm_inst->pwm_pcm_ctx.data_size = data_size;
    pwm_inst->pwm_pcm_ctx.data_addr_shadow = data_addr;
    pwm_inst->pwm_pcm_ctx.data_size_shadow = data_size;

    drv_pwm_int_disable(pwm_inst->pwmc, HAL_PWM_INT_SRC_FIFO_UNDERRUN);

#if ENABLE_SD_DMA
    if ((pwm_inst->pwm_pcm_ctx.play_status != DRV_PWM_PCM_PLAY_IDLE_STATUS) &&
       (pwm_inst->pwm_pcm_ctx.play_status != DRV_PWM_PCM_PLAY_TERMINATE_STATUS)) {
        dprintf(HAL_PWM_DEBUG_LEVEL, "PCM PWM DMA terminate!\n");
        pwm_inst->pwm_pcm_ctx.play_status = DRV_PWM_PCM_PLAY_TERMINATE_STATUS;
        hal_dma_terminate(pwm_inst->pwm_pcm_ctx.tx_desc);
    }

    if (pwm_inst->pwm_pcm_ctx.tx_desc) {
        dprintf(HAL_PWM_DEBUG_LEVEL, "PCM PWM DMA free desc!\n");
        hal_dma_free_desc(pwm_inst->pwm_pcm_ctx.tx_desc);
        hal_pwm_pcm_dma_cfg(handle);
    }

    dprintf(HAL_PWM_DEBUG_LEVEL, "PCM PWM DMA prepare device!\n");
    pwm_inst->pwm_pcm_ctx.tx_desc = hal_prep_dma_dev(pwm_inst->pwm_pcm_ctx.tx_chan,
                                                     (void*)data_addr,
                                                     data_size,
                                                     DMA_INTERRUPT);

    // setup callback function. and need set DMA_INTERRUPT flag.
    pwm_inst->pwm_pcm_ctx.tx_desc->dmac_irq_evt_handle = pcm_pwm_dma_tx_irq_evt_handle;
    pwm_inst->pwm_pcm_ctx.tx_desc->context = (void *)handle;
    pwm_inst->pwm_pcm_ctx.play_status = DRV_PWM_PCM_PLAY_PROC_STATUS;
    arch_clean_cache_range((addr_t)data_addr, data_size);
    dprintf(HAL_PWM_DEBUG_LEVEL, "PCM PWM DMA submit!\n");
    hal_dma_submit(pwm_inst->pwm_pcm_ctx.tx_desc);
#endif

    /* enable compare */
    drv_pwm_cmp_dma_enable(pwm_inst->pwmc);
    drv_pwm_cmp_out_start(pwm_inst->pwmc);
}

void hal_pwm_pcm_play_pause(void *handle)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;

    if(handle == NULL) {
        return;
    }

    if(pwm_inst->pwm_pcm_ctx.play_status != DRV_PWM_PCM_PLAY_PROC_STATUS) {
        return;
    }

    /* disable compare and force output low level */
    drv_pwm_cmp_dma_disable(pwm_inst->pwmc);
    drv_pwm_cmp_out_stop(pwm_inst->pwmc);

    /* update the play status */
    pwm_inst->pwm_pcm_ctx.play_status = DRV_PWM_PCM_PLAY_STOP_STATUS;

    dprintf(HAL_PWM_DEBUG_LEVEL, "PCM PWM stop!\n");
}

void hal_pwm_pcm_play_resume(void *handle)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;

    if(handle == NULL) {
        return;
    }

    if(pwm_inst->pwm_pcm_ctx.play_status != DRV_PWM_PCM_PLAY_STOP_STATUS) {
        return;
    }

    /* update the play status */
    pwm_inst->pwm_pcm_ctx.play_status = DRV_PWM_PCM_PLAY_PROC_STATUS;

    /* enable compare */
    drv_pwm_cmp_dma_enable(pwm_inst->pwmc);
    drv_pwm_cmp_out_start(pwm_inst->pwmc);

    dprintf(HAL_PWM_DEBUG_LEVEL, "PCM PWM resume!\n");
}

void hal_pwm_force_output(void *handle, hal_pwm_chn_t chn, hal_pwm_force_out_t force_out)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;

    if(handle == NULL) {
        return;
    }

    drv_pwm_force_output(pwm_inst->pwmc, chn, force_out);
}

void hal_pwm_simple_duty_set(void *handle, hal_pwm_chn_t chn, uint8_t duty)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;

    if(handle == NULL) {
        return;
    }

    drv_pwm_simple_duty_set(pwm_inst->pwmc, chn, &(pwm_inst->pwm_simple_ctx), duty);
}

void hal_pwm_int_enable(void *handle, hal_pwm_int_src_t int_src)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;

    if(handle == NULL) {
        return;
    }

    drv_pwm_int_enable(pwm_inst->pwmc, int_src);
}

void hal_pwm_int_disable(void *handle, hal_pwm_int_src_t int_src)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;

    if(handle == NULL) {
        return;
    }

    drv_pwm_int_disable(pwm_inst->pwmc, int_src);
}

void hal_pwm_int_cbk_register(void *handle, hal_pwm_int_src_t int_src, hal_pwm_int_func_cbk cbk)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;

    if(handle == NULL) {
        return;
    }

    if(int_src == HAL_PWM_INT_SRC_CMP_EVENT) {
        pwm_inst->pwm_int_cbk.cmp_event_cbk = cbk;
    }
    else if(int_src == HAL_PWM_INT_SRC_CNT_G0_OVF) {
        pwm_inst->pwm_int_cbk.cnt_g0_ovf_cbk = cbk;
    }
    else if(int_src == HAL_PWM_INT_SRC_FIFO_UNDERRUN) {
        pwm_inst->pwm_int_cbk.fifo_underrun_cbk = cbk;
    }
}

enum handler_return hal_pwm_irq_handle(void *handle)
{
    pwm_instance_t *pwm_inst = (pwm_instance_t *)handle;

    if (pwm_inst == NULL)
        return INT_NO_RESCHEDULE;

    return drv_pwm_irq_handle(pwm_inst->pwmc, &(pwm_inst->pwm_int_cbk), handle);
}


#ifdef __cplusplus
}
#endif

