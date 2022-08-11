/*
 * sw_timer.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ii4 Test App.
 *
 * Revision History:
 * -----------------
 */
#include <lk_wrapper.h>
#include "board_init.h"
#include "board_start.h"
#include "board_cfg.h"
#include "func_can.h"
#include "func_dio.h"
#include "func_i2c.h"
#include "func_power.h"
#include "func_eth.h"

TimerHandle_t xAutoReloadTimer;

typedef struct {
#define MONITOR_MODE_TIMERS     70
#define DECT_ONLINE_MAX_TIMERS  10
#define DECT_OFFLINE_MAX_TIMERS  5
    bool state;
    uint8_t dect_online_timers;
    uint8_t dect_offline_timers;
} system_key_state_t;

system_key_state_t system_key_state;
/*get kl15 pin state*/
bool get_system_key_state(void)
{
    return system_key_state.state;
}

/*KL15 filtering processing */
void key_on_off_cyclic_check(system_key_state_t *system_key_state)
{
    if (get_kl15_gpio_state() == ON_LINE) {

        if (system_key_state->dect_online_timers < DECT_ONLINE_MAX_TIMERS) {

            system_key_state->dect_online_timers++;
        }
        else {
            system_key_state->state = ON_LINE;
            dprintf(debug_show_null, "key_state is %d\n", get_system_key_state());
        }

        system_key_state->dect_offline_timers = 0;
    }
    else {
        if (system_key_state->dect_offline_timers < DECT_OFFLINE_MAX_TIMERS) {

            system_key_state->dect_offline_timers++;
        }
        else {
            system_key_state->state = OFF_LINE;
            dprintf(debug_show_null, "key_state is %d\n", get_system_key_state());
        }

        system_key_state->dect_online_timers = 0;
    }
}

static void eth_filtering_error_time(void)
{
    for (uint8_t nr = 0; nr < ETH_MAX_CHN; nr++) {
        if (switch_error_arg[nr].time_val > 0) {
            switch_error_arg[nr].time_val--;
        }
    }
}

/*If the time is up, the cycle handles the event*/
static void sw_cb(TimerHandle_t xTimer)
{
    key_on_off_cyclic_check(&system_key_state);

    /*is the device determined the role in master or slave ? */
    if (get_dev_master_slave_work_role() !=
            true) {
        monitor_master_slave_mode(MONITOR_MODE_TIMERS);
    }

    /*periodic interval test delay*/
    monitor_periodic_interval();
    eth_self_check_for_startingup();
    eth_filtering_error_time();
    spread_spectrum_ops();
}
/*timer init, usually for checking con1_19 pin state or kl15 and so on*/
void sw_time_init(void)
{
    if ((xAutoReloadTimer = xTimerCreate("autoReload", 200, pdTRUE, NULL,
                                         sw_cb)) != NULL) {
        if (xTimerStart(xAutoReloadTimer, 0) != pdPASS )
            dprintf(0, "xTimerStart fail\n");
    }
}
