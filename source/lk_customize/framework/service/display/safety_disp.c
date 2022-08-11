/*
 * Copyright (c) 2020 Semidrive Semiconductor Inc.
 * All rights reserved.
 *
 * Description: display service implementation
 *
 */

#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <dcf.h>
#include <app.h>
#include <thread.h>
#include <event.h>
#include <rp_ioctl.h>

#include "sdm_display.h"
#include <res.h>
#include "disp_data_type.h"
#include "display_service.h"
#include "display_share.h"


static struct dc_share g_dc_share = {0};

static int vsync_callback(int display_id, int64_t timestamp);

/* display ioc callback entry */
static rpc_call_result_t disp_svc_ioctl_cb(rpc_server_handle_t hserver, rpc_call_request_t *request)
{
    rpc_call_result_t result = {0,};
    struct disp_ioctl_cmd *ctl = (struct disp_ioctl_cmd *) &request->param[0];
    struct display_server *disp;
    unsigned int size = RP_IOC_SIZE(ctl->op);
    int ret = 0;
    static bool first_flag = true;

    result.ack = request->cmd;
    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        return result;
    }

    disp = &g_dc_share.display_server;

    mutex_acquire(&disp->dev_lock);

    if(first_flag) {
        first_flag = false;
        thread_detach_and_resume(disp->disp_thread);
    }

    switch(ctl->op) {
        case DISP_CMD_START:
        {
#if CONFIG_VSYNC_THREAD
            display_handle *handle = hal_get_display_handle(0);
            dprintf(CRITICAL, "=============== vsync_callback registered: %d ==============\n", handle->display_id);
            sdm_callback_vsync_register(handle, vsync_callback);
#endif
        }
        break;
        case DISP_CMD_SET_FRAMEINFO:
            if (size != sizeof(struct disp_frame_info)) {
                result.retcode = ERR_NOT_VALID;
                return result;
            }

            ret = disp_ioctl_set_frameinfo(&g_dc_share, RP_IOC_DATA(ctl));
#if CONFIG_DISP_THREAD
            event_signal(&disp->disp_event, false);
#else
        {
            display_handle *handle = hal_get_display_handle(0);
            struct sdm_post_config *post = &g_dc_share.post_configs[disp->on_screen];
            sdm_post(handle, post);
        }
#endif
            break;
        case DISP_CMD_SHARING_WITH_MASK:
        {
            if (size != sizeof(struct disp_frame_info)) {
                result.retcode = ERR_NOT_VALID;
                return result;
            }
            /* blending sharing frame and mask frame with porter-duff*/
            ret = disp_ioctl_set_frameinfo(&g_dc_share, RP_IOC_DATA(ctl));

#if CONFIG_DISP_THREAD
            event_signal(&disp->disp_event, false);
#else
            display_handle *handle = hal_get_display_handle(0);
            struct sdm_post_config *post = &g_dc_share.post_configs[disp->on_screen];
            sdm_post(handle, post);
#endif
            break;
        }
        case DISP_CMD_CLEAR:
        {
            display_handle *handle = hal_get_display_handle(0);
            sdm_clear_display(handle);
            break;
        }
        default:
            break;
    }

    result.retcode = 0;
    mutex_release(&disp->dev_lock);

    //dprintf(0, "%s ioctl cmd=0x%x, disp->binitialized = %d\n", __func__, ctl->op, disp->binitialized);

    return result;
}


#if CONFIG_DISP_THREAD
static int safe_disp_notify_agent(struct display_server *disp, u8 *data, u16 len);
static int mbox_registered = 0;
static int display_vsync_id;

#if CONFIG_VSYNC_THREAD
static int vsync_callback(int display_id, int64_t timestamp) {
    struct display_server *disp = &display_server;

    if (!disp->binitialized || !mbox_registered)
        return 0;

    if (display_id == 0) {
        display_vsync_id = display_id;
        event_signal(&disp->disp_vsync_event, false);
    }
    return 0;
}
#endif

static int safe_disp_notify_agent(struct display_server *disp, u8 *data, u16 len)
{
    return hal_mb_send_data(disp->mchan, data, len, 1000);
}

int fps(void) {
    static lk_time_t last_time;
    static lk_time_t now;
    static int fps_value = 0;
    static int frame_count = 0;

    now = current_time();
    frame_count++;
    if (now - last_time > 1000) {
        fps_value = frame_count;
        frame_count = 0;
        last_time = now;
        // dprintf(0, "disp[%d] fps = %d\n", display_vsync_id,  fps_value);
    }
    return fps_value;
}

#if CONFIG_VSYNC_THREAD
static int display_vsync_thread(void *arg)
{
    int ret;
    struct display_server *disp = arg;

    while(1) {
        ret = event_wait_timeout(&disp->disp_vsync_event, 4000);
        if (ret == 0) {
            fps();
            safe_disp_notify_agent(disp, (u8*) &display_vsync_id, sizeof(int));
        }
        // thread_sleep(10);
    }

    return 0;
}

static int safe_display_add_notifier(struct display_server *disp)
{
    int ret;

    if (mbox_registered)
        return 0;

    disp->mbox_addr = DISP_MAILBOX_ADDR;
    disp->rproc = 3; // IPCC_RRPOC_AP1
    disp->client = hal_mb_get_client_with_addr(disp->mbox_addr);
    if (!disp->client) {
        dprintf(CRITICAL,"get cl failed failed\n");
        return -1;
    }

    disp->mchan = hal_mb_request_channel_with_addr(disp->client, true,
                                  NULL, disp->rproc, disp->mbox_addr);
    if (!disp->mchan) {
        dprintf(CRITICAL,"request channel failed\n");
        return -1;
    }

    event_init(&disp->disp_vsync_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    disp->disp_vsync_thread = thread_create("vsyncd", display_vsync_thread, disp, THREAD_PRI_DISP_VSYN, 2048);
    thread_detach_and_resume(disp->disp_vsync_thread);

    thread_sleep(500);
    mbox_registered = 1;

    return 0;
}
#endif

static int display_main_thread(void *arg)
{
    int ret;
    struct display_server *disp = arg;
    display_handle *handle = hal_get_display_handle(DISPLAY_ID_RPC);

    ret = g2d_handle_init(&g_dc_share);
    if (ret < 0)
       dprintf(CRITICAL, "get g2d handle err!\n");

    while(1) {
        static int vsync_registered = 0;
        ret = event_wait_timeout(&disp->disp_event, 4000);
        if (ret == 0) {
            struct sdm_post_config *post = &g_dc_share.post_configs[disp->on_screen];

            g2d_mask_blending(&g_dc_share);

            int r = sdm_post(handle, post);
            if (r) {
                dprintf(CRITICAL, "sdm_post failed: %d\n", r);
            }
        }
        // thread_sleep(10);
    }

    return 0;
}
#endif

void display_service_init(void)
{
    int ret;
    struct display_server *disp;

    rpc_server_impl_t devfuncs[] = {
        {MOD_RPC_REQ_DC_IOCTL, disp_svc_ioctl_cb, IPCC_RPC_NO_FLAGS},
    };

    dprintf(CRITICAL, "#################\n");

    disp = &g_dc_share.display_server;

    if (disp->binitialized)
        return;

    mutex_init(&disp->dev_lock);

    mutex_acquire(&disp->dev_lock);

#if CONFIG_DISP_THREAD
    event_init(&disp->disp_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    sdm_post_init(&g_dc_share);
#if CONFIG_VSYNC_THREAD
    safe_display_add_notifier(disp);
#endif

    disp->disp_thread = thread_create("displayd", display_main_thread, disp, THREAD_PRI_DISP_MAIN, 4096);
#endif

    mutex_release(&disp->dev_lock);

    start_ipcc_rpc_service(devfuncs, ARRAY_SIZE(devfuncs));

    disp->binitialized = true;
}

void disp_service_entry(const struct app_descriptor *app, void *args)
{
    display_service_init();
}

APP_START(displayd)
 .entry = (app_entry)disp_service_entry,
APP_END

