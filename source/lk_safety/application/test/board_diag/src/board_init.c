/*
 * board_init.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include "board_init.h"
#include "dqueue.h"

static void vcan_rx_init(bool *falg)
{
    *falg = true;
}

bool board_diag_module_init(bool *init_flag)
{
    bool ret = true;
    comx_gpio_init();
    i2c_channel_init();
    vcan_rx_init(init_flag);
    adc_channel_init();
    sw_time_init();
    rtc_clk_init();
    can_bus_wakeup_init();
    remote_test_init();
    return ret;
}
