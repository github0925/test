#include <app.h>
#include <err.h>
#include <lib/console.h>
#include <thread.h>
#include <event.h>
#include "container.h"
#include <sdrpc.h>
#include <sdm_display.h>
#include <disp_data_type.h>
#include <string.h>
#if defined(__GNUC__)
    #include <malloc.h>
#elif defined (__ICCARM__)
    #include "heap.h"
#else
    #error Unknown Compiler!
#endif
#include <heap.h>
#include <spi_nor_hal.h>
#include <dcf_common.h>
#include "lv_controlpanel.h"
#include "lvgl_gui.h"
#include "storage_device.h"
#include <lk_wrapper.h>
#include <early_app_common.h>
#include "early_app_cfg.h"

void controlpanel_property_update(uint8_t val);

// void bootanim_observer_cb(void* pargs, int uargs,int singal)
// {
//     printf("bootanim_observer_cb\n");
//     void* token = (void*)pargs;

//     //lv_set_flush_enabled(true);
// }

void controlpanel_init(void* token)
{
    printf("controlpanel_init\n");
    unified_service_publish(token,ussInit);

    lv_controlpanel();

    // lv_set_flush_enabled(false);
    // unified_service_subscribe(DMP_ID_BA_STATUS,ussTerminated,bootanim_observer_cb,NULL,0);

    unified_service_publish(token,ussReady);
}

void controlpanel_entry(void* token)
{
    printf("controlpanel_entry\n");
    unified_service_publish(token,ussRun);

    lvgl_mainloop(/*disp*/);
}

#include <sys_diagnosis.h>

void controlpanel_property_update(uint8_t val)
{
    int v;
    system_property_get(DMP_ID_DC_STATUS,&v);
    v &= 0xFFFFFF00;
    v |= val;
    system_property_set(DMP_ID_DC_STATUS,v);
}

int controlpanel_backup_callback(int signal,void* args)
{
    controlpanel_property_update(0xAE);//will be set to 0x4 by ap2
    return 0;
}

void start_controlpanel(void)
{
    static bool flag = 0;
    if(flag == 0)
    {
        controlpanel_property_update(0xAE);
        flag = 1;
    }
    else
    {
        controlpanel_property_update(0x4);
        flag = 0;
    }

}

SYSD_CALL(controlpanel)
{
    sysd_register_handler(controlpanel_backup_callback,NULL,1,WDT6____ovflow_int);
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("controlpanel", "start controlpanel", (console_cmd)&start_controlpanel)
STATIC_COMMAND_END(controlpanel_sets);
#endif
