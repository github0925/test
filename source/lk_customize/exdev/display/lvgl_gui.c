/**
 * @file lv_port_disp_templ.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl_gui.h"
#include "lvgl_disp_config.h"
#include "disp_panels.h"
#include "sdm_display.h"
#include "lvgl_disp_config.h"
#include "lv_fs_sdrv.h"
#include "lv_port_indev.h"
#include <trace.h>
#include <string.h>
// #include <kernel/timer.h>
// #include <kernel/thread.h>
#include <lk_wrapper.h>
#ifdef SUPPORT_LODE_PNG
#include "lv_png.h"
#endif
#ifdef SUPPORT_LODE_SJPG
#include "lv_sjpg.h"
#endif
static lk_time_t last_tick = 0;

static enum handler_return lv_tick_timercb(struct timer *t,
                                           lk_time_t now, void *arg)
{
    lv_tick_inc(now - last_tick);

    last_tick = now;
    return INT_NO_RESCHEDULE;
}

static enum handler_return lv_task_timercb(struct timer *t,
                                           lk_time_t now, void *arg)
{
    lv_task_handler();
    return INT_NO_RESCHEDULE;
}


void lvgl_refresh_task(lv_task_t * task)
{
    lv_task_handler();
}

void lvgl_mainloop(void)
{
#if 1
    while(1)
    {
        lv_task_handler();
        thread_sleep(10);
    }
#else
    static int lvgl_looped = 0;

    if (lvgl_looped)
    {
        LOGD("lvgl_looped already！！\n");
        return;
    }
    lvgl_looped = 1;
    while (0)
    {
        thread_sleep(LV_DISP_DEF_REFR_PERIOD);
        lv_task_handler();
    }

    static timer_t lv_task_timer;
    static lk_time_t lv_task_timeout = LV_DISP_DEF_REFR_PERIOD;
    timer_initialize(&lv_task_timer);
    timer_set_periodic(&lv_task_timer, lv_task_timeout,
                       lv_task_timercb, NULL);
    while(0) {
        thread_sleep(1000);
    }
#endif
}

lv_disp_t *g_registered_display[8] = {0};

lv_disp_t *get_display(int display_id)
{
    return g_registered_display[display_id];
}

display_handle *get_handle_from_display(lv_disp_t *disp)
{
    sdm_display_t *sdm = (sdm_display_t *)disp->driver.user_data;
    return sdm->handle;
}

void lvgl_init(void)
{
    static timer_t lv_tick_timer;
    static lk_time_t lv_tick_timeout = 5;
    static int lvgl_inited = 0;

    if (lvgl_inited)
    {
        LOGD("lvgl_init already！！\n");
        return;
    }

    lv_init();
    lv_port_fs_init();

    struct list_node *head = sdm_get_display_list();
    sdm_display_t *sdm;
    list_for_every_entry(head, sdm, sdm_display_t, node) {
        LOGD("disp->id, disp->handle->display_id (%d, %d)\n",
            sdm->id, sdm->handle->display_id);

        if (sdm->handle->display_id == CONTROLPANEL || sdm->handle->display_id == INFOTAINMENT) {
            LOGD("init lvgl for disp %d\n", sdm->handle->display_id);
            lv_disp_t *disp = lvgl_lcd_display_init(sdm);
            g_registered_display[sdm->id] = disp;
            lv_port_indev_init(get_display(sdm->id));
        }
    }

#ifdef SUPPORT_LODE_PNG
    lv_png_init();
#endif
#ifdef SUPPORT_LODE_SJPG
    lv_split_jpeg_init();
#endif

    timer_initialize(&lv_tick_timer);
    timer_set_periodic(&lv_tick_timer, lv_tick_timeout,
                       lv_tick_timercb, NULL);

    lvgl_inited = 1;
}

