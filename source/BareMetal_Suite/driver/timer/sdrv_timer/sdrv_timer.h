/********************************************************
 *      Copyright(c) 2018   Semidrive  Semiconductor    *
 *      All rights reserved.                            *
 ********************************************************/

#ifndef __SDRV_TIMER_H__
#define __SDRV_TIMER_H__

#include <common_hdr.h>
#include <soc.h>

U32 sdrv_tmr_init(module_e id);
U32 sdrv_tmr_get_freq(module_e id);
U64 sdrv_tmr_tick(module_e id);
BOOL sdrv_tmr_is_enabled(module_e id);

#endif
