/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <debug.h>
#include <string.h>
#include <app.h>
#include <lib/console.h>
#include <pwm_hal.h>
#include <hal_port.h>

#include "bl_rtos_conf.h"

int backlight_rtos_duty_set(int value)
{
    return 1;
}
