//*****************************************************************************
//
// uart_hal_weak.c - Driver for the uart hal Module.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <platform/interrupts.h>
#include <sys/types.h>
#include <platform/debug.h>
#include <trace.h>
#include <string.h>
#include <assert.h>
#include <platform.h>
#include "pwm_hal.h"


bool hal_pwm_creat_handle(void **handle, uint32_t res_glb_idx)
{
    return true;
}

bool hal_pwm_release_handle(void *handle)
{
    return true;
}

void hal_pwm_simple_init(void *handle, hal_pwm_simple_cfg_t* pwm_cfg)
{
    return;
}

void hal_pwm_simple_start(void *handle)
{
    return;
}

void hal_pwm_simple_stop(void *handle)
{
    return;
}

void hal_pwm_simple_duty_set(void *handle, hal_pwm_chn_t chn, uint8_t duty)
{
    return;
}

void hal_pwm_pcm_init(void *handle, hal_pwm_pcm_cfg_t* pcm_cfg)
{
    return;
}

void hal_pwm_pcm_play_start(void *handle, uint8_t* data_addr, uint32_t data_size)
{
    return;
}

void hal_pwm_pcm_play_stop(void *handle)
{
    return;
}

void hal_pwm_force_output(void *handle, hal_pwm_chn_t chn, hal_pwm_force_out_t force_out)
{
    return;
}

void hal_pwm_int_enable(void *handle, hal_pwm_int_src_t int_src)
{
    return;
}

void hal_pwm_int_disable(void *handle, hal_pwm_int_src_t int_src)
{
    return;
}

enum handler_return hal_pwm_irq_handle(void *handle)
{
    return INT_NO_RESCHEDULE;
}

void hal_pwm_int_cbk_register(void *handle, hal_pwm_int_src_t int_src, hal_pwm_int_func_cbk cbk)
{
    return;
}

void hal_pwm_pcm_cfg_update(void *handle, hal_pwm_pcm_cfg_t* pcm_cfg)
{
	return;
}


