/*
 * power_down.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include <rtc_drv.h>
#include <pmu_hal.h>
#include "board_start.h"
/*chip power down*/
int set_system_powerdown(void)
{
    int ret = 0;
    void *handle = NULL;
    ret = hal_pmu_creat_handle(&handle, RES_PMU_PMU);

    if (ret != 0) {
        return ret;
    }

    ret = hal_pmu_init(handle);

    if (ret != 0) {
        return ret;
    }

    if (1 != 0xff) {
        ret = hal_pmu_set_powerup_delay(handle, 1);

        if (ret != 0) {
            return ret;
        }
    }

    if (1 != 0xff) {
        ret = hal_pmu_set_powerdown_delay(handle, 1);

        if (ret != 0) {
            return ret;
        }
    }

    ret = hal_pmu_powerdown(handle);

    if (ret != 0) {
        return ret;
    }

    ret = hal_pmu_exit(handle);

    if (ret != 0) {
        return ret;
    }

    ret = hal_pmu_release_handle(handle);

    if (ret != 0) {
        return ret;
    }

    return ret;
}
