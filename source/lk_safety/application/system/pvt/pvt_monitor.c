/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/

#include <debug.h>
#include <string.h>
#include <app.h>
#include <lib/console.h>
#include <kernel/thread.h>

#include <pvt_hal.h>
#include <trace.h>

#define LOCAL_TRACE 1 //close local trace 1->0

#define PVT_MONITOR_WAIT_TIME 3 // seconds

#define PVT_MONITOR_WAIT_TIME_VALUE (PVT_MONITOR_WAIT_TIME*1000)

#define PVT_ALARM_TYPE_0 0
#define PVT_ALARM_TYPE_1 1

#define PVT_ALARM_INT_ENABLE 1
#define PVT_ALARM_INT_DISABLE 0

#define PVT_ALARM_INT_REGISTER 1
#define PVT_ALARM_INT_UNREGISTER 0

int pvt_monitor_test(int argc, const cmd_args *argv)
{
    dprintf(CRITICAL, "pvt_monitor_test empty\n");

    return 0;
}

enum handler_return pvt_irq_handle(void* arg)
{
    pvt_instance_t * pvt_info;
    //DEBUG_ASSERT(false);
    if(arg != NULL){
        pvt_info = (pvt_instance_t *)arg;
            //irq_happen = 1
        if(pvt_info->arg != NULL){
            *((uint32_t *)(pvt_info->arg)) = 1;
        }

    }
    //disable pvt int
    hal_pvt_int_en(PVT_RES_ID_SAF, PVT_INT_TYPE_HYST_HIGH, PVT_ALARM_INT_DISABLE, PVT_ALARM_TYPE_0);
    return INT_NO_RESCHEDULE;
}

int pvt_monitor_hyst_h_test(void)
{
    uint32_t int_status = 0;
    float hyst_h_thresh_h = 75.5;
    float hyst_h_thresh_l = 65.5;
    float out_t_data;
    uint32_t index = PVT_ALARM_TYPE_0;
    uint32_t irq_happen = 0;

    LTRACEF("pvt_monitor_hyst_h_test enter\n");

    //pvt setting hyst h value
    hal_pvt_set_hyst_h(PVT_RES_ID_SAF, hyst_h_thresh_h, hyst_h_thresh_l, index);

    //enable pvt int
    hal_pvt_int_en(PVT_RES_ID_SAF, PVT_INT_TYPE_HYST_HIGH, PVT_ALARM_INT_ENABLE, index);

    //register int
    hal_pvt_int_register(PVT_RES_ID_SAF, PVT_ALARM_INT_REGISTER, pvt_irq_handle, index, (void *)&irq_happen);

    while (1) {
        if(irq_happen == 1){

            //get int status, pvt has four int type, this status is int type

            hal_pvt_get_int_status(PVT_RES_ID_SAF, index, &int_status);

            LTRACEF("pvt_monitor_hyst_h_test get ini status int_status=%d\n",int_status);

            //get pvt t value, if t is larger than hyst_h_thresh_l, can not clear hyst_h int
            hal_pvt_get_t(PVT_RES_ID_SAF, &out_t_data);
            LTRACEF("pvt_monitor_hyst_h_test get temperature value = %f\n", out_t_data);

            if(out_t_data<hyst_h_thresh_l){
                LTRACEF("pvt_monitor_hyst_h_test clear int\n");
                //clear int
                hal_pvt_clear_int(PVT_RES_ID_SAF, PVT_INT_TYPE_HYST_HIGH, index);

                //enable pvt_h int, wait for next int happen
                hal_pvt_int_en(PVT_RES_ID_SAF, PVT_INT_TYPE_HYST_HIGH, PVT_ALARM_INT_ENABLE, index);
                irq_happen = 0;
            }
        }
        thread_sleep(PVT_MONITOR_WAIT_TIME_VALUE);
    }

}

void pvt_monitor(const struct app_descriptor * app_desp, void *args)
{
    LTRACEF("pvt_monitor enter\n");
    pvt_monitor_hyst_h_test();
}
#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("pvt_mt", "pvt_monitor test\n", (console_cmd)&pvt_monitor_test)
STATIC_COMMAND_END(pvt_mt);

#endif

APP_START(pvt_monitor)
.flags = 0,
.entry = pvt_monitor,
.stack_size = 1024,
.flags = APP_FLAG_CUSTOM_STACK_SIZE,
APP_END
