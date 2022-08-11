//*****************************************************************************
//
// i2s_hal_weak.c
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include "i2s_hal.h"

bool hal_sd_i2s_sc_create_handle(void **handle, u_int32_t res_id)
{
    return true;
}

bool hal_sd_i2s_sc_release_handle(void *handle)
{
    return true;
}

bool hal_sd_i2s_sc_start_up(void *handle, pcm_params_t pcm_info)
{
    return true;
}

bool hal_sd_i2s_sc_set_format(void *handle)
{
    return true;
}

bool hal_sd_i2s_sc_set_hw_params(void *handle)
{
    return true;
}

bool hal_sd_i2s_sc_trigger(void *handle, int cmd)
{
    return true;
}

bool hal_sd_i2s_sc_shutdown(void *handle)
{
    return true;
}

paddr_t hal_sd_i2s_sc_get_fifo_addr(void *handle)
{
    return 0;
}

void hal_sd_i2s_sc_show_config(void *handle)
{

}

int hal_sd_i2s_sc_wait_tx_comp_intmode(void *handle, int timeout)
{
    return 0;
}

int hal_sd_i2s_sc_wait_rx_comp_intmode(void *handle, int timeout)
{
    return 0;
}

struct dma_desc *hal_i2s_trigger_dma_tr_start(void *i2s_handle,
        uint32_t dma_cap, void *obj_addr, uint32_t len,
        void *dma_irq_handle, uint32_t cmd)
{
    return NULL;
}