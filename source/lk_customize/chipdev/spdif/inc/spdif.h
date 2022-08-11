//*****************************************************************************
//
// spdif.h
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef __SPDIF_H__
#define __SPDIF_H__

#include <sys/types.h>
#include <spdif_hal.h>
#include <kernel/event.h>

#define SPDIF_FIFO_ADDR_OFFSET 0x0010U

typedef struct {
    u32 bus;
    u32 is_added;
    u32 interrupt_num;
    addr_t base_addr;
    spdif_cfg_info_t cfg_info;
    event_t tx_comp;
    event_t rx_comp;
} spdif_top_cfg_t;

bool spdif_init(spdif_top_cfg_t *cfg);
bool spdif_config(spdif_top_cfg_t *cfg);
bool spdif_start(spdif_top_cfg_t *cfg);
bool spdif_stop(spdif_top_cfg_t *cfg);
void spdif_show_current_cfg(spdif_top_cfg_t
                            *cfg);//print controller cfg and cfg info
bool spdif_sleep(spdif_top_cfg_t *cfg);
bool spdif_int_transmit(spdif_top_cfg_t *cfg);

#endif
