//*****************************************************************************
//
// i2s_hal_weak.c
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include "i2s_hal.h"

bool hal_i2s_sc_create_handle(void **handle, uint32_t i2s_sc_res_glb_idx)
{
    return true;
}

bool hal_i2s_sc_release_handle(void *handle)
{
    return true;
}

bool hal_i2s_sc_init(void *handle)
{
    return true;
}

bool hal_i2s_sc_deinit(void *handle)
{
    return true;
}

bool hal_i2s_sc_config(void *handle, i2s_sc_init_t *i2s_config)
{
    return true;
}

bool hal_i2s_sc_start(void *handle)
{
    return true;
}

bool hal_i2s_sc_stop(void *handle)
{
    return true;
}

bool hal_i2s_sc_transmit(void *handle)
{
    return true;
}

int hal_i2s_sc_wait_tx_comp_intmode(void *handle, int timeout)
{
    return 0;
}

int hal_i2s_sc_wait_rx_comp_intmode(void *handle, int timeout)
{
    return 0;
}

paddr_t hal_i2s_sc_get_fifo_addr(void *handle)
{
    return 0;
}

bool hal_i2s_mc_create_handle(void **handle, uint32_t i2s_mc_res_glb_idx)
{
    return true;
}

bool hal_i2s_mc_release_handle(void *handle)
{
    return true;
}

bool hal_i2s_mc_init(void *handle)
{
    return true;
}

bool hal_i2s_mc_deinit(void *handle)
{
    return true;
}

bool hal_i2s_mc_config(void *handle, i2s_mc_init_t *i2s_config)
{
    return true;
}

bool hal_i2s_mc_start(void *handle)
{
    return true;
}

bool hal_i2s_mc_stop(void *handle)
{
    return true;
}

bool hal_i2s_mc_transmit(void *handle)
{
    return true;
}

paddr_t hal_i2s_mc_get_fifo_addr(void *handle)
{
    return 0;
}
