/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <string.h>
#include "touch_driver.h"

void __register_touch_driver(struct touch_driver *driver)
{
    struct ts_board_config *tsc = NULL;
    int tsc_num = 0;
    get_touch_device(&tsc, &tsc_num);

    for (int i = 0; i < tsc_num; i++) {
        if (tsc[i].enable
                && driver->probe
                && !strcmp(tsc[i].device_name, driver->driver_name))
            driver->probe(&tsc[i]);
    }
}

