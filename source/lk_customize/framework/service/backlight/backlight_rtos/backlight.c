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

static void *bl_pwm_handle;

static void backlight_rtos_init(void)
{
    hal_pwm_simple_cfg_t hal_cfg;

    hal_cfg.freq = BL_RTOS_PWM_FRQ;
    hal_cfg.grp_num = BL_RTOS_PWM_GRP;
    hal_cfg.single_mode = HAL_PWM_CONTINUE_CMP;
    hal_cfg.align_mode = HAL_PWM_EDGE_ALIGN_MODE;
    hal_cfg.cmp_cfg[BL_RTOS_PWM_CH].phase = HAL_PWM_PHASE_POLARITY_POS;
#if BL_RTOS_REVERSE_CONTROL
    hal_cfg.cmp_cfg[BL_RTOS_PWM_CH].duty = BL_RTOS_PWM_MAX_DUTY - BL_RTOS_PWM_DEFAULT_DUTY;
#else
    hal_cfg.cmp_cfg[BL_RTOS_PWM_CH].duty = BL_RTOS_PWM_DEFAULT_DUTY;
#endif
    hal_pwm_creat_handle(&bl_pwm_handle, BL_RTOS_PWM_RES);

    if(bl_pwm_handle != NULL) {
        hal_pwm_simple_init(bl_pwm_handle, &hal_cfg);
        hal_pwm_simple_start(bl_pwm_handle);
        dprintf(BL_RTOS_DEBUG_LEVEL, "backlight rtos pwm inited, frequency: %d, duty: percent %d!\n",
                                    hal_cfg.freq,
                                    BL_RTOS_REVERSE_CONTROL ? (BL_RTOS_PWM_MAX_DUTY - hal_cfg.cmp_cfg[BL_RTOS_PWM_CH].duty)
                                    : hal_cfg.cmp_cfg[BL_RTOS_PWM_CH].duty);
    }
}

int backlight_rtos_duty_set(int value)
{
    uint8_t duty;

    if(value > BL_RTOS_PWM_MAX_DUTY || value < 0)
        return 0;

#if BL_RTOS_REVERSE_CONTROL
    duty = BL_RTOS_PWM_MAX_DUTY - value;
#else
    duty = value;
#endif
    hal_pwm_simple_duty_set(bl_pwm_handle, BL_RTOS_PWM_CH, duty);
    dprintf(BL_RTOS_DEBUG_LEVEL, "backlight rtos pwm duty set: percent %d!\n", value);

    return 1;
}

void backlight_rtos_entry(const struct app_descriptor *app, void *args)
{
    backlight_rtos_init();
}

APP_START(backlight_rtos)
 .entry = (app_entry)backlight_rtos_entry,
APP_END

