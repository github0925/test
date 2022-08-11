//*****************************************************************************
//
// spdif_hal_weak.c
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#include <sys/types.h>
#include <spdif_hal.h>

bool hal_spdif_create_handle(void **handle, u32 res_id)
{
    return true;
}

bool hal_spdif_release_handle(void *handle)
{
    return true;
}

bool hal_spdif_init(void *handle)
{
    return true;
}

bool hal_spdif_config(void *handle, spdif_cfg_info_t *spdif_config)
{
    return true;
}

bool hal_spdif_start(void *handle)
{
    return true;
}

bool hal_spdif_stop(void *handle)
{
    return true;
}

bool hal_spdif_int_transmit(void *handle)
{
    return true;
}

u32 hal_spdif_wait_tx_comp_intmode(void *handle, int timeout)
{
    return 0;
}

u32 hal_spdif_wait_rx_comp_intmode(void *handle, int timeout)
{
    return 0;
}

bool hal_spdif_sleep(void *handle)
{
    return true;
}

addr_t hal_spdif_get_fifo_addr(void *handle)
{
    return 0;
}

u32 hal_spdif_get_fifo_depth(void *handle)
{
    return 0;
}

void hal_spdif_show_reg_config(void *handle)
{

}

