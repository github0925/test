/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#ifndef __TOUCH_DRIVER_H__
#define __TOUCH_DRIVER_H__

#include <lk/init.h>
#include "touch_device.h"

struct touch_driver {
    const char *driver_name;
    int (*probe)(const struct ts_board_config *_tsc);
};

#define register_touch_driver(drv) \
static void register_touch_driver_entry(uint level) \
{ \
    __register_touch_driver(&drv); \
} \
LK_INIT_HOOK(touch_driver##drv, register_touch_driver_entry, LK_INIT_LEVEL_TARGET)

extern void __register_touch_driver(struct touch_driver *drv);

#endif
