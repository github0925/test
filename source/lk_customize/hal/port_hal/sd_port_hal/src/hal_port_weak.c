/*
 * hal_port_weak.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: port/iomux driver.
 *
 * Revision History:
 * -----------------
 * 0.1, 9/18/2019 init version
 */

#include <assert.h>
#include <debug.h>
#include <platform.h>
#include <stdint.h>
#include <stdlib.h>
#include <trace.h>

#include "hal_port.h"

bool hal_port_creat_handle(void **handle, uint32_t port_res_glb_idx)
{
    return true;
}

bool hal_port_release_handle(void **handle)
{
    return true;
}

int hal_port_init(void *handle)
{
    return 0;
}

int hal_port_set_pin_direction(void *handle, const Port_PinType pin,
                               const Port_PinDirectionType direction)
{
    return 0;
}

int hal_port_refresh_port_direction(void *handle)
{
    return 0;
}

int hal_port_set_pin_mode(void *handle, const Port_PinType pin,
                          const Port_PinModeType mode)
{
    return 0;
}

int hal_port_set_to_gpioctrl(void *handle, const gpio_ctrl_t gpio_ctrl,
                          const Port_PinType pin)
{
    return 0;
}

int hal_port_get_pin_info(void *handle, uint32_t pin_num,
    Port_PinModeType *pin_mode, uint32_t * input_select,
    uint32_t * gpio_ctrl, int32_t * gpio_config)
{
    return 0;
}

