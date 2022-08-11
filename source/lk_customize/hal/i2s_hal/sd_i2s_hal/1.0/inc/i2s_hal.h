//*****************************************************************************
//
// i2s_sc_hal.h
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//
//*****************************************************************************

#ifndef __I2S_HAL_H__
#define __I2S_HAL_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include <kernel/mutex.h>
#include "__regs_base.h"
#include "res.h"
#include "chip_res.h"
#include "system_cfg.h"

#define MAKE_VERSION(major, minor, bugfix) (((major) << 16) | ((minor) << 8) | (bugfix))
#define I2S_FUNC_WITH_DMA 0
#define I2S_FUNC_WITH_INT 1
#define I2S_SAMPLE_8_BIT 7
#define I2S_SAMPLE_16_BIT 15
#define I2S_SAMPLE_24_BIT 23
#define I2S_SAMPLE_32_BIT 31

typedef enum {
    I2S_STD_NO_INIT = 0,
    I2S_STD_PHILLIPS,
    I2S_STD_LEFT_JUSTIFIED,
    I2S_STD_RIGHT_JUSTIFIED,
} i2s_standard;

typedef struct _i2s_res {
    uint32_t res_id;
    uint32_t ctrl_id;
    uint32_t irq_num;
    uint32_t clock;
} i2s_res_t;

typedef struct {
    uint32_t ctrl_id;
    uint32_t res_id;
    uint32_t irq_num;
    paddr_t phy_addr;
    uint32_t clock;
} i2s_res_config_t;

typedef enum {
    I2S_SC_MOD_NO_INIT = 0,
    I2S_SC_MOD_MASTER_TX,
    I2S_SC_MOD_MASTER_RX,
    I2S_SC_MOD_SLAVE_TX,
    I2S_SC_MOD_SLAVE_RX,
    I2S_SC_MOD_MASTER_FULL_DUPLEX,
    I2S_SC_MOD_SLAVE_FULL_DUPLEX
} i2s_sc_mode;

typedef enum {
    I2S_SC_CHN_NO_INIT = 0,
    I2S_SC_CHN_MONO,
    I2S_SC_CHN_STEREO,
    I2S_SC_CHN_TDM
} i2s_sc_chnmode;

typedef enum {
    I2S_MC_MOD_NO_INIT = 0,
    I2S_MC_MOD_MASTER,
    I2S_MC_MOD_SLAVE,
} i2s_mc_mode;

/* prototype for sc i2s */
typedef struct {
    uint32_t mode;
    uint8_t chn_mode;
    uint8_t standard;
    uint32_t audio_freq;          /* Master:Unit:Hz,Slave:No need */
    uint8_t chn_width;            /* total number of data bit,0:8sck,1:12sck,...7:32sck */
    uint8_t tx_sample_resolution; /* transmit valid number of data bit,0:1bit,...31:32bit */
    uint8_t rx_sample_resolution; /* receive valid number of data bit,0:1bit,...31:32bit */
    uint8_t ws_mode;              /* TDM mode:WS signal change occurs after given number of channels */
    uint16_t tdm_tx_chn_en;       /* TDM mode bit map:bit0->chn0...bit15->chn15,0:disable,1:enable */
    uint16_t tdm_rx_chn_en;       /* TDM mode bit map:bit0->chn0...bit15->chn15,0:disable,1:enable */
    uint8_t func_mode;            /* 0:mode functions with dma ,1:mode functions with interrput */
    uint8_t *ptx_buffer;
    uint32_t tx_count;
    uint8_t *prx_buffer;
    uint32_t rx_count;

    enum handler_return (*exception_int_handle)(void *dev, uint8_t err);
    enum handler_return (*fifo_empty_int_handle)(void *dev);
    enum handler_return (*fifo_aempty_int_handle)(void *dev);
    enum handler_return (*fifo_full_int_handle)(void *dev);
    enum handler_return (*fifo_afull_int_handle)(void *dev);
    enum handler_return (*rfifo_empty_int_handle)(void *dev);
    enum handler_return (*rfifo_aempty_int_handle)(void *dev);
    enum handler_return (*rfifo_full_int_handle)(void *dev);
    enum handler_return (*rfifo_afull_int_handle)(void *dev);
} i2s_sc_init_t;

/* prototype for mc i2s.  */
typedef struct {
    uint8_t tx_mode;
    uint8_t tx_standard;
    uint32_t tx_audio_freq;
    uint8_t tx_sample_resolution;
    uint8_t rx_standard;
    uint8_t rx_mode;
    uint32_t rx_audio_freq;
    uint8_t rx_sample_resolution;
    uint8_t chn_enable; /* bit map:bit0->chn0...bit7->chn7,0:disable,1:enable */
    uint8_t chn_int_en;
    uint8_t chn_direction; /* bit map:bit0->chn0...bit7->chn7,0:receiver,1:transmitter */
    uint8_t func_mode;     /* 0:mode functions with dma ,1:mode functions with interrput */
    uint8_t *ptx_buffer;
    uint32_t tx_count;
    uint8_t *prx_buffer;
    uint32_t rx_count;
    uint8_t loop_back_test_en;

    enum handler_return (*underrun_int_handle)(void *dev, uint8_t err_mask);
    enum handler_return (*overrun_int_handle)(void *dev, uint8_t err_mask);
    enum handler_return (*tfifo_empty_int_handle)(void *dev);
    enum handler_return (*tfifo_aempty_int_handle)(void *dev);
    enum handler_return (*tfifo_full_int_handle)(void *dev);
    enum handler_return (*tfifo_afull_int_handle)(void *dev);
    enum handler_return (*rfifo_empty_int_handle)(void *dev);
    enum handler_return (*rfifo_aempty_int_handle)(void *dev);
    enum handler_return (*rfifo_full_int_handle)(void *dev);
    enum handler_return (*rfifo_afull_int_handle)(void *dev);
} i2s_mc_init_t;

/**
* @deprecated
* @brief create i2s sc handle.
*
* get i2s sc dev handle by resource id
*
* @param handle i2s sc device handle
* @param i2s_sc_res_glb_idx i2s sc res global index
*
* @return \b true if get handle successfully or \b false
*
*/
bool hal_i2s_sc_create_handle(void **handle, uint32_t i2s_sc_res_glb_idx);

/**
* @deprecated
* @brief release i2s sc handle.
*
* release i2s sc dev handle
*
* @param handle i2s sc device handle
*
* @return \b true
*
*/
bool hal_i2s_sc_release_handle(void *handle);

/**
* @deprecated
* @brief init i2s sc dev.
*
* init dev info and event, set init bit true,
* register and unmask interrupt
*
* @param handle i2s sc device handle
*
* @return \b true
*
*/
bool hal_i2s_sc_init(void *handle);

/**
* @deprecated
* @brief deinit i2s sc dev.
*
* set init bit false
*
* @param handle i2s sc device handle
*
* @return \b true
*
*/
bool hal_i2s_sc_deinit(void *handle);

/**
* @deprecated
* @brief config i2s sc dev.
*
* config i2s sc dev by i2s_sc_init_t type config
*
* @param handle i2s sc dev handle
* @param i2s_config i2s config info
*
* @return \b true if success or \b false
*
*/
bool hal_i2s_sc_config(void *handle, i2s_sc_init_t *i2s_config);

/**
* @deprecated
* @brief start i2s sc dev data transfer.
*
* @param handle i2s sc dev handle
*
* @return \b true if succeed or \b false
*/
bool hal_i2s_sc_start(void *handle);

/**
* @deprecated
* @brief stop i2s sc dev data transfer.
*
* @param handle i2s sc dev handle
*
* @return \b true if succeed or \b false
*/
bool hal_i2s_sc_stop(void *handle);

/**
* @deprecated
* @brief start i2s sc dev data transfer by cpu.
*
* @param handle i2s sc dev handle
*
* @return \b true if succeed or \b false
*/
bool hal_i2s_sc_transmit(void *handle);

/**
* @deprecated
* @brief get dev fifo address.
*
* @param handle i2s sc dev handle
*
* @return i2s sc dev fifo physical address
*/
paddr_t hal_i2s_sc_get_fifo_addr(void *handle);

/**
* @deprecated
* @brief wait tx transfer event complete by cpu.
*
* @param handle i2s sc dev handle
* @param timeout wait time count
*
* @return 0 if transfer completed, otherwise return untransfered number
*/
int hal_i2s_sc_wait_tx_comp_intmode(void *handle, int timeout);

/**
* @deprecated
* @brief wait rx transfer event complete by cpu.
*
* @param handle i2s sc dev handle
* @param timeout wait time count
*
* @return 0 if transfer completed, otherwise return untransfered number
*/
int hal_i2s_sc_wait_rx_comp_intmode(void *handle, int timeout);

//*****************************************************************************
//
//! function : hal_i2s_mc_create_handle
//!
//! arg      : i2s sc device handle
//!
//! return   : i2s sc devide fifo address
//!
//
//*****************************************************************************
bool hal_i2s_mc_create_handle(void **handle, uint32_t i2s_mc_res_glb_idx);

//*****************************************************************************
//
//! function : hal_i2s_mc_release_handle
//!
//! arg      : i2s mc device handle, i2s mc res global index
//!
//! return   : true if success, false if fail
//!
//
//*****************************************************************************
bool hal_i2s_mc_release_handle(void *handle);

//*****************************************************************************
//
//! function : hal_i2s_mc_init
//!
//! arg      : i2s mc device handle
//!
//! return   : true if success, false if fail
//!
//
//*****************************************************************************
bool hal_i2s_mc_init(void *handle);

//*****************************************************************************
//
//! function : hal_i2s_mc_deinit
//!
//! arg      : i2s mc device handle
//!
//! return   : true if success, false if fail
//!
//
//*****************************************************************************
bool hal_i2s_mc_deinit(void *handle);

//*****************************************************************************
//
//! function : hal_i2s_mc_config
//!
//! arg      : i2s mc device handle
//!
//! return   : true if success, false if fail
//!
//
//*****************************************************************************
bool hal_i2s_mc_config(void *handle, i2s_mc_init_t *i2s_config);

//*****************************************************************************
//
//! function : hal_i2s_mc_start
//!
//! arg      : i2s mc device handle, i2s mc config struct ptr
//!
//! return   : true if success, false if fail
//!
//
//*****************************************************************************
bool hal_i2s_mc_start(void *handle);

//*****************************************************************************
//
//! function : hal_i2s_mc_stop
//!
//! arg      : i2s mc device handle
//!
//! return   : true if success, false if fail
//!
//
//*****************************************************************************
bool hal_i2s_mc_stop(void *handle);

//*****************************************************************************
//
//! function : hal_i2s_mc_transmit
//!
//! arg      : i2s mc device handle
//!
//! return   : true if success, false if fail
//!
//
//*****************************************************************************
bool hal_i2s_mc_transmit(void *handle);

//*****************************************************************************
//
//! function : hal_i2s_mc_get_fifo_addr
//!
//! arg      : i2s mc device handle
//!
//! return   : true if success, false if fail
//!
//
//*****************************************************************************
paddr_t hal_i2s_mc_get_fifo_addr(void *handle);

//*****************************************************************************
//
//! function : hal_i2s_sc_show_config
//!
//! arg      : i2s mc device handle
//!
//! return   : true if success, false if fail
//!
//
//*****************************************************************************
void hal_i2s_sc_show_config(void *handle);

//*****************************************************************************
//
//! function : hal_i2s_mc_show_config
//!
//! arg      : i2s mc device handle
//!
//! return   : true if success, false if fail
//!
//
//*****************************************************************************
void hal_i2s_mc_show_config(void *handle);

#ifdef __cplusplus
}
#endif

#endif
