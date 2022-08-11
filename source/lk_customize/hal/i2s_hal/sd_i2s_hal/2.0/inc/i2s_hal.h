//*****************************************************************************
//
// i2s_hal.h
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************
#ifndef __I2S_HAL_H__
#define __I2S_HAL_H__

#include <kernel/mutex.h>
#include "__regs_base.h"
#include <sys/types.h>
#include "res.h"
#include "chip_res.h"
#include "system_cfg.h"
#include <sd_audio.h>
#include <dma_hal.h>

typedef void (*dmac_irq_evt_handle)(enum dma_status status, u32 param,
        void *context);
/**
* @brief create i2s sc handle.
*
* get i2s sc dev handle by resource id
*
* @param handle i2s sc device handle
* @param i2s_sc_res_glb_idx i2s sc res global index
*
* @return \b true if succeed or \b false
*
*/
bool hal_sd_i2s_sc_create_handle(void **handle, u_int32_t res_id);

/**
* @brief release i2s sc handle.
*
* release i2s sc dev handle
*
* @param handle i2s sc device handle
*
* @return \b true
*
*/
bool hal_sd_i2s_sc_release_handle(void *handle);

/**
* @brief i2s sc dev start up.
*
* @param handle i2s sc dev handle
* @param pcm_info pcm info
*
* @return \b true if success or \b false
*
*/
bool hal_sd_i2s_sc_start_up(void *handle, pcm_params_t pcm_info);

/**
* @brief set i2s interface format.
*
* @param handle i2s sc device handle
*
* @return \b true
*
*/
bool hal_sd_i2s_sc_set_format(void *handle);

/**
* @brief set some hardware params.
*
* @param handle i2s sc device handle
*
* @return \b true
*
*/
bool hal_sd_i2s_sc_set_hw_params(void *handle);

/**
* @brief trigger event by cmd.
*
* trigger playback/capture start/stop
*
* @param handle i2s sc device handle
*
* @return \b true
*
*/
bool hal_sd_i2s_sc_trigger(void *handle, int cmd);

/**
* @brief stop i2s sc dev.
*
* disable i2s sc dev
*
* release i2s sc dev handle
*
* @param handle i2s sc device handle
*
* @return \b true
*
*/
bool hal_sd_i2s_sc_shutdown(void *handle);

/**
* @brief get dev fifo address.
*
* @param handle i2s sc dev handle
*
* @return i2s sc dev fifo physical address
*/
paddr_t hal_sd_i2s_sc_get_fifo_addr(void *handle);

/**
* @brief show dev register value.
*
* @param handle i2s sc device handle
*
*/
void hal_sd_i2s_sc_show_config(void *handle);

/**
* @brief wait tx transfer event complete by cpu.
*
* @param handle i2s sc dev handle
* @param timeout wait time count
*
* @return 0 if transfer completed, otherwise return untransfered number
*/
int hal_sd_i2s_sc_wait_tx_comp_intmode(void *handle, int timeout);

/**
* @brief wait rx transfer event complete by cpu.
*
* @param handle i2s sc dev handle
* @param timeout wait time count
*
* @return 0 if transfer completed, otherwise return untransfered number
*/
int hal_sd_i2s_sc_wait_rx_comp_intmode(void *handle, int timeout);

/**
 * @brief trigger i2s dma playback start
 *
 * set necessary dma cfg and start both dma and i2s.
 *
 * @param i2s_handle i2s handle
 * @param dma_cap dma cap
 * @param obj_addr addr that dma transfer to
 * @param len buffer length
 * @param dma_irq_handle actions when interruput comes
 * @param cmd playback start or capture start
 *
 * @return dma_desc
*/
struct dma_desc *hal_i2s_trigger_dma_tr_start(void *i2s_handle,
        uint32_t dma_cap, void *obj_addr, uint32_t len,
        dmac_irq_evt_handle dma_irq_handle, uint32_t cmd);

#endif