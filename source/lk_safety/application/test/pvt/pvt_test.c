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

#include <lib/slt_module_test.h>

#include <pvt_hal.h>
#include <trace.h>

#define LOCAL_TRACE 1 //close local trace 1->0

#define PVT_PRINT_WAIT_TIME 3 // seconds

#define PVT_MODE_TYPE_TEMP 0
#define PVT_MODE_TYPE_VOL 1
#define PVT_MODE_TYPE_P 2

#define PVT_DEVICE_TYPE_ULVT 1
#define PVT_DEVICE_TYPE_LVT 2
#define PVT_DEVICE_TYPE_SVT 3

#define PVT_SLT_TEST_TEMP_MIN 5
#define PVT_SLT_TEST_TEMP_MAX 60

#define PVT_RES_ID_ALL (PVT_RES_ID_SEC + 1)

#define PVT_ALARM_TYPE_0 0
#define PVT_ALARM_INT_ENABLE 1
#define PVT_ALARM_INT_DISABLE 0

#define PVT_ALARM_INT_REGISTER 1
#define PVT_ALARM_INT_UNREGISTER 0

#define PVT_MONITOR_WAIT_TIME 3 // seconds
#define PVT_MONITOR_WAIT_TIME_VALUE (PVT_MONITOR_WAIT_TIME*1000)

#if PVT_IN_SAFETY_DOMAIN
#define PVT_RES_ID_ PVT_RES_ID_SAF
#else
#define PVT_RES_ID_ PVT_RES_ID_SEC
#endif

typedef enum pvt_test_result_offset {
    PVT_TEST_RESULT_OFFSET_VALUE_TEST  = 0x0,
    PVT_TEST_RESULT_OFFSET_INT_TEST,
} pvt_test_result_offset_t;

typedef struct pvt_ctrl_data {
    int pvt_out_print;
    int pvt_mode_type;
    int pvt_device_type;
    int domain_type;
} pvt_ctrl_data_t;

enum handler_return pvt_test_irq_handle(void* arg)
{
    pvt_instance_t * pvt_info;

    if(arg != NULL){
        pvt_info = (pvt_instance_t *)arg;
            //pvt_test_irq_happen = 1
        if(pvt_info->arg != NULL){
            *((uint32_t *)(pvt_info->arg)) = 1;
        }

    }

    //disable pvt int
    hal_pvt_int_en(PVT_RES_ID_SAF, PVT_INT_TYPE_HYST_HIGH, PVT_ALARM_INT_DISABLE, PVT_ALARM_TYPE_0);
    return INT_NO_RESCHEDULE;
}

//return 0 pass, other fail
int pvt_test_monitor_hyst_h_test(pvt_res_id_t res_id, float hyst_h_thresh_l, float hyst_h_thresh_h)
{
    int ret;
    uint32_t int_status = 0;
    float out_t_data;
    uint32_t index = PVT_ALARM_TYPE_0;
    uint32_t check_times = 30;
    uint32_t pvt_test_irq_happen = 0;

    LTRACEF("pvt_monitor_hyst_h_test enter\n");

    ret = 1;

    //pvt setting hyst h value
    hal_pvt_set_hyst_h(res_id, hyst_h_thresh_h, hyst_h_thresh_l, index);

    //enable pvt int
    hal_pvt_int_en(res_id, PVT_INT_TYPE_HYST_HIGH, PVT_ALARM_INT_ENABLE, index);

    //register int
    hal_pvt_int_register(res_id, PVT_ALARM_INT_REGISTER, pvt_test_irq_handle, index, (void *)&pvt_test_irq_happen);

    while (check_times != 0) {
        if (pvt_test_irq_happen == 1) {

            //get int status, pvt has four int type, this status is int type

            hal_pvt_get_int_status(res_id, index, &int_status);

            LTRACEF("pvt_monitor_hyst_h_test get ini status int_status=%d\n", int_status);

            //get pvt t value, if t is larger than hyst_h_thresh_l, can not clear hyst_h int
            hal_pvt_get_t(res_id, &out_t_data);
            LTRACEF("pvt_monitor_hyst_h_test get temperature value = %f\n", out_t_data);

            if (out_t_data < hyst_h_thresh_l) {
                LTRACEF("pvt_monitor_hyst_h_test clear int\n");
                //clear int
                hal_pvt_clear_int(res_id, PVT_INT_TYPE_HYST_HIGH, index);

                //enable pvt_h int, wait for next int happen
                //hal_pvt_int_en(res_id, PVT_INT_TYPE_HYST_HIGH, PVT_ALARM_INT_ENABLE, index);
                pvt_test_irq_happen = 0;
            }

            ret = 0;

            break;
        }else{
            hal_pvt_get_t(res_id, &out_t_data);
            LTRACEF("no irq get temperature value = %f, hyst_h_thresh_h =%f\n", out_t_data, hyst_h_thresh_h);
        }

        thread_sleep(PVT_MONITOR_WAIT_TIME_VALUE);
        check_times--;
    }

    return ret;

}

int slt_module_test_pvt_test(uint times, uint timeout, char* result_string)
{
    int ret;
    int result_value;

    pvt_ctrl_data_t g_out_data = {0};
    pvt_out_data_t out_data = {0};

    g_out_data.pvt_device_type = PVT_DEVICE_TYPE_ULVT;

    result_value = 1;

    //init pvt value
    while(out_data.temp_data <= 0){
        hal_pvt_get_pvt(PVT_RES_ID_, g_out_data.pvt_device_type, &out_data);
    }

    LTRACEF("temp_data =%f\n", out_data.temp_data);

    if ((out_data.temp_data == 0) || (out_data.voltage_data == 0) || (out_data.process_data == 0)) {

    }
    else {

        if ((out_data.temp_data > PVT_SLT_TEST_TEMP_MIN) && (out_data.temp_data < PVT_SLT_TEST_TEMP_MAX)) {
            // test pass
            result_value = 0;
        }

    }

    ret = (result_value << PVT_TEST_RESULT_OFFSET_VALUE_TEST);

    result_value = pvt_test_monitor_hyst_h_test(PVT_RES_ID_, (out_data.temp_data - 1), (out_data.temp_data + 1));

    ret |= (result_value << PVT_TEST_RESULT_OFFSET_INT_TEST);

    if (result_string != NULL) {
        if (ret  != 0) {
            strcpy(result_string, "pvt test fail, detail cause see error code");
        }
        else {
            strcpy(result_string, "pvt test pass");
        }
    }

    return ret;
}


uint32_t pvt_test_start(void)
{
    char out_print[64];
    LTRACEF("pvt_test_start start\n");
    //pvt setting
    slt_module_test_pvt_test(0, 0, (char*)&out_print);

    return 0;

}

#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("pvt_test", "pvt_test;\n", (console_cmd)&pvt_test_start)
STATIC_COMMAND_END(pvt_test);

#endif

SLT_MODULE_TEST_HOOK(pvt_test, slt_module_test_pvt_test);

APP_START(pvt_test)
.flags = 0,
APP_END
