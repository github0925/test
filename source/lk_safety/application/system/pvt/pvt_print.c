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

#define LOCAL_TRACE 0 //close local trace 1->0

#define PVT_PRINT_WAIT_TIME 3 // seconds

#define PVT_PRINT_WAIT_TIME_VALUE (PVT_PRINT_WAIT_TIME*1000)

#define PVT_MODE_TYPE_TEMP 0
#define PVT_MODE_TYPE_VOL 1
#define PVT_MODE_TYPE_P 2

#define PVT_DEVICE_TYPE_ULVT 1
#define PVT_DEVICE_TYPE_LVT 2
#define PVT_DEVICE_TYPE_SVT 3

#define PVT_OUT_TYPE_OFF 0
#define PVT_OUT_TYPE_ON_ALL 1
#define PVT_OUT_TYPE_ON_ONE_TYPE 2

#define PVT_RES_ID_ALL (PVT_RES_ID_SEC + 1)

#if PVT_IN_SAFETY_DOMAIN
#define PVT_RES_ID_ PVT_RES_ID_SAF
#else
#define PVT_RES_ID_ PVT_RES_ID_SEC
#endif

typedef struct pvt_ctrl_data
{
    int pvt_out_print;
    int pvt_mode_type;
    int pvt_device_type;
    int domain_type;
}pvt_ctrl_data_t;

pvt_ctrl_data_t g_out_data = {0};

int pvt_ctrl(void)
{

    int ret = 0;
    int i = 0;
    pvt_out_data_t out_data;
    uint32_t out_p_data = 0;
    float out_v_data;
    float out_t_data;
    char domain_name[PVT_RES_ID_ALL][8]={{"safety"},{"secure"}};
#ifdef PVT_AUTO_OUT_PRT
    g_out_data.pvt_out_print = PVT_OUT_TYPE_ON_ALL;
    g_out_data.pvt_device_type = PVT_DEVICE_TYPE_ULVT;
    g_out_data.domain_type = PVT_RES_ID_;
#endif
    //read pvt value
    while (1) {
        if(g_out_data.pvt_out_print == PVT_OUT_TYPE_ON_ALL){

            if(g_out_data.domain_type == PVT_RES_ID_ALL){
                for(i = PVT_RES_ID_SAF; i < PVT_RES_ID_ALL; i++){

                    hal_pvt_get_pvt(i,g_out_data.pvt_device_type,&out_data);

                    if(g_out_data.pvt_device_type == PVT_DEVICE_TYPE_ULVT){
                        dprintf(CRITICAL, "%s temperature value = %f, voltage value = %f, ULVT process value = %d\n", domain_name[i], out_data.temp_data,out_data.voltage_data,out_data.process_data);
                    }else if(g_out_data.pvt_device_type == PVT_DEVICE_TYPE_LVT){
                        dprintf(CRITICAL, "%s temperature value = %f, voltage value = %f, LVT process value = %d\n", domain_name[i], out_data.temp_data,out_data.voltage_data,out_data.process_data);
                    }else if(g_out_data.pvt_device_type == PVT_DEVICE_TYPE_SVT){
                        dprintf(CRITICAL, "%s temperature value = %f, voltage value = %f, SVT process value = %d\n", domain_name[i], out_data.temp_data,out_data.voltage_data,out_data.process_data);
                    }else{
                        dprintf(CRITICAL, "%s temperature value = %f, voltage value = %f, device %d process value = %d\n", domain_name[i], out_data.temp_data,out_data.voltage_data, g_out_data.pvt_device_type,out_data.process_data);
                    }
                }
            }else{
                hal_pvt_get_pvt(g_out_data.domain_type,g_out_data.pvt_device_type,&out_data);

                if(g_out_data.pvt_device_type == PVT_DEVICE_TYPE_ULVT){
                    dprintf(CRITICAL, "%s temperature value = %f, voltage value = %f, ULVT process value = %d\n", domain_name[g_out_data.domain_type], out_data.temp_data,out_data.voltage_data,out_data.process_data);
                }else if(g_out_data.pvt_device_type == PVT_DEVICE_TYPE_LVT){
                    dprintf(CRITICAL, "%s temperature value = %f, voltage value = %f, LVT process value = %d\n", domain_name[g_out_data.domain_type], out_data.temp_data,out_data.voltage_data,out_data.process_data);
                }else if(g_out_data.pvt_device_type == PVT_DEVICE_TYPE_SVT){
                    dprintf(CRITICAL, "%s temperature value = %f, voltage value = %f, SVT process value = %d\n", domain_name[g_out_data.domain_type], out_data.temp_data,out_data.voltage_data,out_data.process_data);
                }else{
                    dprintf(CRITICAL, "%s temperature value = %f, voltage value = %f, device %d process value = %d\n", domain_name[g_out_data.domain_type], out_data.temp_data,out_data.voltage_data, g_out_data.pvt_device_type,out_data.process_data);
                }
            }

            thread_sleep(PVT_PRINT_WAIT_TIME_VALUE);

        }else if(g_out_data.pvt_out_print == PVT_OUT_TYPE_ON_ONE_TYPE){

            if(g_out_data.domain_type == PVT_RES_ID_ALL){
                for(i = PVT_RES_ID_SAF; i < PVT_RES_ID_ALL; i++){
                    if(g_out_data.pvt_mode_type == PVT_MODE_TYPE_TEMP){

                        hal_pvt_get_t(i, &out_t_data);
                        dprintf(CRITICAL, "%s temperature value = %f\n", domain_name[i], out_t_data);

                    }else if(g_out_data.pvt_mode_type == PVT_MODE_TYPE_VOL){

                        hal_pvt_get_v(i, &out_v_data);
                        dprintf(CRITICAL, "%s voltage value = %f\n", domain_name[i], out_v_data);

                    }else if(g_out_data.pvt_mode_type == PVT_MODE_TYPE_P){

                        hal_pvt_get_p(i, g_out_data.pvt_device_type, &out_p_data);

                        if(g_out_data.pvt_device_type == PVT_DEVICE_TYPE_ULVT){
                            dprintf(CRITICAL, "%s ULVT process value = %d\n", domain_name[i], out_p_data);
                        }else if(g_out_data.pvt_device_type == PVT_DEVICE_TYPE_LVT){
                            dprintf(CRITICAL, "%s LVT process value = %d\n", domain_name[i], out_p_data);
                        }else if(g_out_data.pvt_device_type == PVT_DEVICE_TYPE_SVT){
                            dprintf(CRITICAL, "%s SVT process value = %d\n", domain_name[i], out_p_data);
                        }else{
                            dprintf(CRITICAL, "%s device %d process value = %d\n", domain_name[i], g_out_data.pvt_device_type, out_p_data);
                        }

                    }else{

                    }
                }
            }else{
                if(g_out_data.pvt_mode_type == PVT_MODE_TYPE_TEMP){

                    hal_pvt_get_t(g_out_data.domain_type, &out_t_data);
                    dprintf(CRITICAL, "%s temperature value = %f\n", domain_name[g_out_data.domain_type], out_t_data);

                }else if(g_out_data.pvt_mode_type == PVT_MODE_TYPE_VOL){

                    hal_pvt_get_v(g_out_data.domain_type, &out_v_data);
                    dprintf(CRITICAL, "%s voltage value = %f\n", domain_name[g_out_data.domain_type], out_v_data);

                }else if(g_out_data.pvt_mode_type == PVT_MODE_TYPE_P){

                    hal_pvt_get_p(g_out_data.domain_type, g_out_data.pvt_device_type, &out_p_data);

                    if(g_out_data.pvt_device_type == PVT_DEVICE_TYPE_ULVT){
                        dprintf(CRITICAL, "%s ULVT process value = %d\n", domain_name[g_out_data.domain_type], out_p_data);
                    }else if(g_out_data.pvt_device_type == PVT_DEVICE_TYPE_LVT){
                        dprintf(CRITICAL, "%s LVT process value = %d\n", domain_name[g_out_data.domain_type], out_p_data);
                    }else if(g_out_data.pvt_device_type == PVT_DEVICE_TYPE_SVT){
                        dprintf(CRITICAL, "%s SVT process value = %d\n", domain_name[g_out_data.domain_type], out_p_data);
                    }else{
                        dprintf(CRITICAL, "%s device %d process value = %d\n", domain_name[g_out_data.domain_type], g_out_data.pvt_device_type, out_p_data);
                    }

                }else{

                }
            }

            thread_sleep(PVT_PRINT_WAIT_TIME_VALUE);
        }else{
            thread_sleep(PVT_PRINT_WAIT_TIME_VALUE);
        }
    }

    return ret;
}


int pvt_set_print(int argc, const cmd_args *argv)
{
    unsigned long is_out;

    if (argc == 2)    {
        is_out = argv[1].u;
        if(is_out == 0){
            g_out_data.pvt_out_print = PVT_OUT_TYPE_OFF;
        }else if(is_out == 1){
            g_out_data.pvt_out_print = PVT_OUT_TYPE_ON_ALL;
            g_out_data.pvt_device_type = PVT_DEVICE_TYPE_ULVT;
            g_out_data.domain_type = PVT_RES_ID_;
        }
        else{
            dprintf(CRITICAL, "input argv error\n");
            return -1;
        }
    }else if(argc == 3) {
        is_out = argv[1].u;
        g_out_data.domain_type = argv[2].u;

        if((is_out == PVT_DEVICE_TYPE_ULVT)||(is_out == PVT_DEVICE_TYPE_LVT)||(is_out == PVT_DEVICE_TYPE_SVT)){
            if((g_out_data.domain_type < PVT_RES_ID_SAF)||(g_out_data.domain_type > PVT_RES_ID_ALL)){
                dprintf(CRITICAL, "input argv error\n");
                return -1;
            }
            g_out_data.pvt_out_print = PVT_OUT_TYPE_ON_ALL;
            g_out_data.pvt_device_type = is_out;
        }else if(is_out == 0){
            g_out_data.pvt_out_print = PVT_OUT_TYPE_OFF;
        }else{
            dprintf(CRITICAL, "input argv error\n");
            return -1;
        }

    }else if(argc == 4){
        g_out_data.pvt_mode_type = argv[1].u;
        g_out_data.pvt_device_type = argv[2].u;
        g_out_data.domain_type = argv[3].u;
        if(((g_out_data.domain_type < PVT_RES_ID_SAF)||(g_out_data.domain_type > PVT_RES_ID_ALL))
           ||((g_out_data.pvt_mode_type < PVT_MODE_TYPE_TEMP)||(g_out_data.pvt_mode_type > PVT_MODE_TYPE_P))
           ||((g_out_data.pvt_mode_type == PVT_MODE_TYPE_P)&&((g_out_data.pvt_device_type < PVT_DEVICE_TYPE_ULVT)||(g_out_data.pvt_device_type > PVT_DEVICE_TYPE_SVT)))){
            dprintf(CRITICAL, "input argv error\n");
            return -1;
        }
        g_out_data.pvt_out_print = PVT_OUT_TYPE_ON_ONE_TYPE;

    }else{
        dprintf(CRITICAL, "input argv error\n");
        return -1;
    }
    return 0;
}

void pvt_entry(const struct app_descriptor * app_desp, void *args)
{
    LTRACEF("pvt_entry start\n");
    //pvt setting
    pvt_ctrl();

}

#if defined(WITH_LIB_CONSOLE)

STATIC_COMMAND_START
STATIC_COMMAND("pvt_on", "pvt_on (0/1/2/3) (0/1/2): print p v t, (off/ulvt/lvt/svt) (saf sec all);\n"
                         "pvt_on (0/1/2) (1/2/3) (0/1/2): print p/v/t, (t/v/p) (ulvt/lvt/svt) (saf sec all)", (console_cmd)&pvt_set_print)
STATIC_COMMAND_END(pvt_info);

#endif

APP_START(pvt_entry)
.flags = 0,
.entry = pvt_entry,
.stack_size = 1024,
.flags = APP_FLAG_CUSTOM_STACK_SIZE,
APP_END
