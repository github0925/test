/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 *******************************************************/

/**
 * @file    host_tmr.h
 * @brief   header file of 'Host' timer
 */

#ifndef __HOST_TMR_H__
#define __HOST_TMR_H__

#include <common_hdr.h>
#include <service.h>

U32 host_tmr_init(module_e id);
U32 host_tmr_get_freq(module_e id);
U64 host_tmr_tick(module_e id);
BOOL host_tmr_is_enabled(module_e id);

#endif // __HOST_TMR_H__
