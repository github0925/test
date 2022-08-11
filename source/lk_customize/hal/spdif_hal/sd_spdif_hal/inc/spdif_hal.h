//*****************************************************************************
//
// spdif_hal.h
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#ifndef __SPDIF_HAL_H__
#define __SPDIF_HAL_H__

#include <sys/types.h>
#include <stdbool.h>

#define SPDIF_FIFO_DEPTH 64
#define SPDIF_FIFO_THRESHOLD_DEFAULT 0xffff

typedef struct _spdif_res {
    u32 res_id;
    u32 ctrl_id;
    u32 interrupt_num;
} spdif_res_t;

typedef struct _spdif_res_info {
    u32 res_id;
    u32 ctrl_id;
    u32 interrupt_num;
    u32 phy_addr;
} spdif_res_info_t;

typedef enum {
    SPDIF_TR_MOD_NO_INIT = 0,
    SPDIF_RECEIVER,
    SPDIF_TRANSMITTER,
} spdif_tr_mode_t;

typedef enum {
    SPDIF_CH_MOD_NO_INIT = 0,
    SPDIF_STEREO,
    SPDIF_MONO,
} spdif_ch_mode_t;

typedef enum {
    SPDIF_TRANSFER_MOD_NO_INIT = 0,
    SPDIF_TR_WITH_INT,
    SPDIF_TR_WITH_DMA,
} spdif_tranfer_mode_t;

typedef enum {
    SPDIF_DISABLE,
    SPDIF_ENABLE,
} spdif_func_enable_t;

typedef enum {
    SPDIF_FORMAT_NO_INIT = 0,
    SPDIF_FORMAT_8BITS,
    SPDIF_FORMAT_16BITS,
    SPDIF_FORMAT_20BITS,
    SPDIF_FORMAT_24BITS,
} spdif_format_t;

typedef struct {
    enum handler_return (* parity_int_handle)(void *cfg);
    enum handler_return (* ovrerr_int_handle)(void *cfg);
    enum handler_return (* underr_int_handle)(void *cfg);
    enum handler_return (* empty_int_handle)(void *cfg);
    enum handler_return (* aempty_int_handle)(void *cfg);
    enum handler_return (* full_int_handle)(void *cfg);
    enum handler_return (* afull_int_handle)(void *cfg);
    enum handler_return (* syncerr_int_handle)(void *cfg);
    enum handler_return (* lock_int_handle)(void *cfg);
    enum handler_return (* begin_int_handle)(void *cfg);
} spdif_int_handle_t;

typedef struct {
    spdif_tr_mode_t tr_mode;
    u32 tsample_rate;
    spdif_tranfer_mode_t transfer_mode;
    spdif_format_t resolution;
    spdif_ch_mode_t ch_mode;
    spdif_func_enable_t duplicate;
    spdif_func_enable_t preamble_delay;
    spdif_func_enable_t parity;
    spdif_func_enable_t validity;
    spdif_func_enable_t use_fifo_if;
    u32 aempty_threshold;
    u32 afull_threshold;
    u8 *ptx_buffer;
    u32 tx_count;
    u8 *prx_buffer;
    u32 rx_count;
    spdif_int_handle_t int_handle;
} spdif_cfg_info_t;

//*****************************************************************************
//
//! hal_spdif_create_handle.
//!
//! \param handle spdif_handle pointer to pointer.
//!
//! \param res_id resource id.
//!
//! This function get hal handle by resource id.
//!
//! \return true if create handle successfully or false.
//
//*****************************************************************************
bool hal_spdif_create_handle(void **handle, u32 res_id);

//*****************************************************************************
//
//! hal_spdif_release_handle.
//!
//! \param handle spdif_handle pointer.
//!
//! This function release spdif hal handle.
//!
//! \return true if release handle successfully.
//
//*****************************************************************************
bool hal_spdif_release_handle(void *handle);

//*****************************************************************************
//
//! hal_spdif_init.
//!
//! \param handle spdif_handle pointer.
//!
//! This function init spdif: register int, get res info.
//!
//! \return true if init successfully or false.
//
//*****************************************************************************
bool hal_spdif_init(void *handle);

//*****************************************************************************
//
//! hal_spdif_config.
//!
//! \param handle spdif_handle pointer.
//!
//! \param spdif_config spdif config pointer.
//!
//! This function sets spdif config.
//!
//! \return true if config setup successfully or false.
//
//*****************************************************************************
bool hal_spdif_config(void *handle, spdif_cfg_info_t *spdif_config);

//*****************************************************************************
//
//! hal_spdif_start.
//!
//! \param handle spdif_handle pointer.
//!
//! This function starts the spdif.
//!
//! \return true if start spdif successfully.
//
//*****************************************************************************
bool hal_spdif_start(void *handle);

//*****************************************************************************
//
//! hal_spdif_stop.
//!
//! \param handle spdif_handle pointer.
//!
//! This function stops the spdif.
//!
//! \return true if stop spdif successfully.
//
//*****************************************************************************
bool hal_spdif_stop(void *handle);

//*****************************************************************************
//
//! hal_spdif_int_transmit.
//!
//! \param handle spdif_handle pointer.
//!
//! This function start the spdif transmitting by using cpu.
//!
//! \return true if spdif transmit successfully.
//
//*****************************************************************************
bool hal_spdif_int_transmit(void *handle);

//*****************************************************************************
//
//! hal_spdif_wait_tx_comp_intmode.
//!
//! \param handle spdif_handle pointer.
//!
//! \param timeout max wait time.
//!
//! This function waits till tx cpu transfer complete.
//!
//! \return tx count left.
//
//****************************************************************************
u32 hal_spdif_wait_tx_comp_intmode(void *handle, int timeout);

//*****************************************************************************
//
//! hal_spdif_wait_rx_comp_intmode.
//!
//! \param handle spdif_handle pointer.
//!
//! \param timeout max wait time.
//!
//! This function waits till rx cpu transfer complete.
//!
//! \return rx count left.
//
//****************************************************************************
u32 hal_spdif_wait_rx_comp_intmode(void *handle, int timeout);

//*****************************************************************************
//
//! hal_spdif_sleep.
//!
//! \param handle spdif_handle pointer.
//!
//! This function gets spdif sleep.
//!
//! \return true if sleep successfully or false.
//
//*****************************************************************************
bool hal_spdif_sleep(void *handle);

//*****************************************************************************
//
//! hal_spdif_get_fifo_addr.
//!
//! \param handle spdif_handle pointer.
//!
//! This function gets spdif fifo address, mainly for dma use.
//!
//! \return spdif fifo physical address .
//
//*****************************************************************************
addr_t hal_spdif_get_fifo_addr(void *handle);

//*****************************************************************************
//
//! hal_spdif_get_fifo_depth.
//!
//! \param handle spdif_handle pointer.
//!
//! This function gets spdif fifo depth, for fifo threshold settings.
//!
//! \return spdif fifo depth.
//
//*****************************************************************************
u32 hal_spdif_get_fifo_depth(void *handle);

//*****************************************************************************
//
//! hal_spdif_show_reg_config.
//!
//! \param handle spdif_handle pointer.
//!
//! This function prints spdif controller's register value.
//
//*****************************************************************************
void hal_spdif_show_reg_config(void *handle);

#endif