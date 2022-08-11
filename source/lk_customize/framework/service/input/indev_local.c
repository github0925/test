/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <lib/bytes.h>
#include <lib/reg.h>

#include "indev_local.h"
#include "safe_ts.h"

#define MAX_DISPLAY_TOUCH_NUM 5

static int indev_dbg_flag = 0;

struct indev_local_dev {
    int abs_x_max;
    int abs_y_max;
    int x_offset;
    int y_offset;
    int x;
    int y;
    bool pressed;
    int screen_id;
};

struct indev_local_dev indev[MAX_DISPLAY_TOUCH_NUM];

int indev_local_init(int x_off, int y_off, int x_max, int y_max)
{
    //TODO: support multidisplay, Assuming all screens are the same
    for (int i=0;i<MAX_DISPLAY_TOUCH_NUM;i++) {
        indev[i].abs_x_max = x_max;
        indev[i].abs_y_max = y_max;
        indev[i].x_offset = x_off;
        indev[i].y_offset = y_off;
    }

    dprintf(0, "%s: (%d, %d), (%d, %d)\n", __func__, x_max, y_max, x_off,
            y_off);
    return 0;
}

int indev_local_process_touch_report(u8 *data, u16 len, int screen_id)
{
    int i;
    int x, y;
    struct touch_report_data *report_data = (struct touch_report_data *)data;

    if (len <= 0)
        return 0;
    //dprintf(0, "report touch screen id %d\n", screen_id);
    if (screen_id < 0 || screen_id >= MAX_DISPLAY_TOUCH_NUM) {
        dprintf(0, "screen id is invalid %d\n", screen_id);
        return 0;
    }
    //TODO: support multi-touchpoint
    for (i = 0; i < 1; i++) {
        //id = report_data->coord_data[i].id;
        //w = report_data->coord_data[i].w;
        x = report_data->coord_data[i].x - indev[screen_id].x_offset;
        y = report_data->coord_data[i].y - indev[screen_id].y_offset;

        if ((x < 0) || (x > indev[screen_id].abs_x_max) || (y < 0) || (y > indev[screen_id].abs_y_max))
            return 0;

        if (report_data->touch_num > 0)
            indev[screen_id].pressed = true;
        else
            indev[screen_id].pressed = false;

        indev[screen_id].x = report_data->coord_data[i].x;
        indev[screen_id].y = report_data->coord_data[i].y;
        indev[screen_id].screen_id = screen_id;
    }

    if (indev_dbg_flag)
        dprintf(0, "%s(): (%d, %d), press=%d\n", __func__, indev[screen_id].x, indev[screen_id].y,
                indev[screen_id].pressed);

    return 0;
}

bool indev_local_is_pressed(int screen_id)
{
    if (indev[screen_id].pressed)
        return true;
    else
        return false;
}

void indev_local_get_xy(int *x, int *y, int screen_id)
{
    (*x) = indev[screen_id].x;
    (*y) = indev[screen_id].y;
}

#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
static int indev_dbg_en(int argc, const cmd_args *argv)
{
    indev_dbg_flag = atoui(argv[1].str);
    dprintf(ALWAYS, "indev_dbg_flag=%d\n", indev_dbg_flag);
    return 0;
}

STATIC_COMMAND_START STATIC_COMMAND("indev_dbg_en", "indev_dbg_en [0/1]",
                                    (console_cmd)&indev_dbg_en)
STATIC_COMMAND_END(indev_touch);
#endif

