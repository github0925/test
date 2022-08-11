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
#include "uart_hal.h"


bool hal_uart_creat_handle(void **handle, uint32_t res_glb_idx)
{
    return true;
}

bool hal_uart_release_handle(void *handle)
{
    return true;
}

void hal_uart_init(void *handle, hal_uart_cfg_t *cfg)
{
    return;
}

void hal_uart_putc(void *handle, char data)
{
    return;
}

void hal_uart_transmit(void *handle, char *buf, size_t size, bool async)
{
    return;
}

void hal_uart_receive(void *handle, char *buf, size_t size)
{
    return;
}

void hal_uart_9bits_putc(void *handle, char data, bool addr)
{
    return;
}

void hal_uart_9bits_transmit(void *handle, char addr, char *buf,
                             size_t size)
{
    return;
}

bool hal_uart_getc(void *handle, char *data)
{
    return true;
}

void hal_uart_int_cbk_register(void *handle, hal_uart_int_src_t int_src,
                            hal_uart_int_callback cbk)
{
    return;
}

void hal_uart_int_src_enable(void *handle, hal_uart_int_src_t int_src)
{
    return;
}

void hal_uart_int_src_disable(void *handle, hal_uart_int_src_t int_src)
{
    return;
}


