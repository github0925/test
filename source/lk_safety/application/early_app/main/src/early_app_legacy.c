#include "res_loader.h"
#include <app.h>
#include <thread.h>
#include <event.h>
#include <container.h>
#include <res.h>
#include <sdrpc.h>
#include <dcf.h>
#include <dc_status.h>
#include <early_app_cfg.h>

#define MAX_DISPLAY_NUM    3

status_t mask_interrupt(unsigned int vector);
bool sdm_display_is_inited(void);


void animation_entry(token_handle_t token);
void avm_start(token_handle_t token);
void tlvgl_entry(token_handle_t token);

void* splash_load_pic(void);
void splash_eliminate_pic(void* pic);
void splash_screen(void* pic, bool on);
void cluster_entry(token_handle_t token);
event_t animation_kick,avm_kick;
event_t cluster_kick;

event_t tlvgl_kick;


void animation_over_callback(token_handle_t token)
{
    printf("Animation over.\n");
    event_signal(&animation_kick,false);
}


void early_app(void)
{
    uint32_t i = 0;
#if CONFIG_USE_SYS_PROPERTY
    disp_sync_args_t disp_args;
    disp_args.val = 0;
    for(i = 0;i<MAX_DISPLAY_NUM; i++){
        disp_args.args[i].state = DC_STAT_INITING;
    }
    system_property_set(DMP_ID_DC_STATUS, disp_args.val);
#endif


    while(!sdm_display_is_inited());



#if CONFIG_USE_SYS_PROPERTY
    for(i = 0;i<MAX_DISPLAY_NUM; i++)
        disp_args.args[i].state = DC_STAT_INITED;
    system_property_set(DMP_ID_DC_STATUS, disp_args.val);
#endif

#ifdef ENABLE_BOOTANIMATION

#if CONFIG_USE_SYS_PROPERTY
    for(i = 0; i< MAX_DISPLAY_NUM; i++)
        disp_args.args[i].state = DC_STAT_BOOTING;
    system_property_set(DMP_ID_DC_STATUS, disp_args.val);
#endif

    event_init(&animation_kick,0,EVENT_FLAG_AUTOUNSIGNAL);
    token_handle_t animation_token = token_create("animation",NULL,3500,animation_over_callback,NULL);
    thread_t* animation = thread_create("animation",(thread_start_routine)animation_entry,animation_token,DEFAULT_PRIORITY,1024);
    thread_detach_and_resume(animation);

    event_wait(&animation_kick);

    event_destroy(&animation_kick);
    token_destroy(animation_token);

#endif
#ifdef ENABLE_TLVGL

    event_init(&tlvgl_kick,0,EVENT_FLAG_AUTOUNSIGNAL);
    token_handle_t tlvgl_token = token_create("TLVGLA",NULL,3500,NULL,NULL);
    thread_t* tlvgla = thread_create("tlvgla",(thread_start_routine)tlvgl_entry,tlvgl_token,DEFAULT_PRIORITY,1024);
    thread_detach_and_resume(tlvgla);

    event_wait(&tlvgl_kick);

    event_destroy(&tlvgl_kick);
    token_destroy(tlvgl_token);

#endif


#ifdef ENABLE_FASMAVM

    void* pic = splash_load_pic();
    splash_screen(pic,true);

    event_init(&avm_kick,0,EVENT_FLAG_AUTOUNSIGNAL);
    token_handle_t avm_token = token_create("avm",NULL,0,NULL,NULL);
    thread_t* avm = thread_create("avm",(thread_start_routine)avm_start,avm_token,DEFAULT_PRIORITY,2*DEFAULT_STACK_SIZE);

    thread_detach_and_resume(avm);
    event_wait(&avm_kick);
    token_destroy(avm_token);
    event_destroy(&avm_kick);

    splash_screen(pic,false);
    splash_eliminate_pic(pic);

#endif


#ifdef ENABLE_CLUSTER

    event_init(&cluster_kick,0,EVENT_FLAG_AUTOUNSIGNAL);
    token_handle_t cluster_token = token_create("cluster",NULL,0,NULL,NULL);
    thread_t* cluster = thread_create("cluster",(thread_start_routine)cluster_entry,cluster_token,DEFAULT_PRIORITY,DEFAULT_STACK_SIZE);
    thread_detach_and_resume(cluster);

    event_wait(&cluster_kick);

    event_destroy(&cluster_kick);
    token_destroy(cluster_token);

#endif

    mask_interrupt(IRQ_GIC1_DC1_DC_IRQ1_NUM);
    mask_interrupt(IRQ_GIC1_DC3_DC_IRQ1_NUM);
    sdrpc_notify_msg(NULL,COM_DC_STATUS,NULL);

#if CONFIG_USE_SYS_PROPERTY
    for(i = 0; i< MAX_DISPLAY_NUM; i++)
        disp_args.args[i].state = DC_STAT_CLOSED;
    system_property_set(DMP_ID_DC_STATUS, disp_args.val);
#endif

}

APP_START(early_app)
 .entry = (app_entry)early_app,
 .stack_size = 512,
APP_END

