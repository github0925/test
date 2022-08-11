/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <debug.h>
#include <string.h>
#include <app.h>
#include <lib/console.h>
#include <bl_conf.h>

#define APP_BL_DEBUG_LEVEL 0

static int app_backlight_duty_set(int argc, const cmd_args *argv)
{
    uint8_t duty;
    int displayid = 3;

    if(argc != 3) {
        dprintf(APP_BL_DEBUG_LEVEL, "Input parameter number must be 3!\n");
        return -1;
    }

    displayid = argv[1].u;
    duty = argv[2].u;
    backlight_duty_set(displayid, duty);
    dprintf(APP_BL_DEBUG_LEVEL, "app backlight pwm duty set: displayid %d percent %d!\n", displayid, duty);

    return 1;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("bl_duty_set", "bl_duty_set [screenid] [duty]",  (console_cmd)&app_backlight_duty_set)
STATIC_COMMAND_END(app_backlight);
#endif

