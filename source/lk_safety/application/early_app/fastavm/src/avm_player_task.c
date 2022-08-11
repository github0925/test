#include <app.h>
#include <lk_wrapper.h>
#include <sdm_display.h>
#include <disp_data_type.h>
#include <v4l2.h>
#include <csi_hal.h>

#include "avm_app_csi.h"
#include "avm_data_structure_def.h"
#include "vstreamer.h"
#include "avm_fastavm_config.h"
#include "avm_player_utility.h"
#include <dcf.h>
#include <dc_status.h>

#ifdef PLAYER_USE_FPS_TIMER
static enum handler_return fps_tmr_cb(struct timer *t, lk_time_t now, void *arg)
{
    event_t* evt = arg;
    event_signal(evt,true);

    return 0;
}
#endif

void player_task(vstreamer_t* strm)
{
    struct list_node * disp_node = sdm_get_display_list();
    disp_req_t disp_req;
#ifdef PLAYER_USE_FPS_TIMER
    /* use timer to control */
    timer_t fps_timer;
    event_t fps_tmr_cb_signal;
#else
    event_t *evt_done = strm->evt_done;
#endif
    uint32_t posx_left, posx_middle, posx_right; // which screen to use is unknown for now

    /* display needs to be changed */
    sdm_display_t* display0 = containerof(disp_node->next, sdm_display_t, node);
    disp_node = disp_node->next;
    sdm_display_t* display1 = containerof(disp_node->next, sdm_display_t, node);

#ifdef PLAYER_USE_FPS_TIMER
    event_init(&fps_tmr_cb_signal, false, EVENT_FLAG_AUTOUNSIGNAL);
    timer_initialize(&fps_timer);
    timer_set_periodic(&fps_timer, 1000/ANIMATION_FPS, fps_tmr_cb,&fps_tmr_cb_signal);
#endif

    APP_DBG("display0:%p,%s\n", display0, display0->handle->name);
    APP_DBG("display1:%p,%s\n", display1, display1->handle->name);

    struct sdm_post_config post_data;
    memset(&post_data, 0, sizeof(struct sdm_post_config));

#if (PLAYER_LAYERS == 1)
    /* 1 layer for avm, 1 layer for single image view */
    struct sdm_buffer sdm_buf  = DISPLAY_AVM_TEMPLATE;
    post_data.bufs             = &sdm_buf;
    post_data.n_bufs           = 1;
    post_data.custom_data      = NULL;
    post_data.custom_data_size = 0;
#elif (PLAYER_LAYERS == 2)
    struct sdm_buffer sdm_bufs[2] = {DISPLAY_AVM_TEMPLATE, DISPLAY_SINGLE_TEMPLATE};
#else
    printf("only 1 or 2 layers is supported\n");
#endif
    APP_DBG("go into player task\n");

    while (1) {
        APP_DBG("--pop  start@%lu\n", current_time());
        strm->pop(strm, &disp_req);
        /* received producer data */
        APP_DBG("--pop  end@%lu\n", current_time());

#if (PLAYER_LAYERS == 1)
        fill_sdm_buf(&sdm_buf, disp_req.bufY, disp_req.strideY);
        posx_left = 0 + (disp_req.strideY / 3 * 0);
        crop_sdm_buf(&sdm_buf, posx_left, 0, AVM_WIDTH, AVM_HEIGHT);
        map_sdm_buf(&sdm_buf, 0, 0, AVM_WIDTH, AVM_HEIGHT);
        post_data.bufs             = &sdm_buf;
        post_data.n_bufs           = 1;
        post_data.custom_data      = NULL;
        post_data.custom_data_size = 0;
        APP_DBG("sdm buffer:");
        APP_DBG("src - Y - CB - CR - Stride | %x - %x - %x - %d\n",
                sdm_buf.addr[0], sdm_buf.addr[1], sdm_buf.addr[2], sdm_buf.src_stride[0]);
        APP_DBG("crop - x - y - w - h | (%d,%d) %dx%d\n",
                sdm_buf.src.x, sdm_buf.src.y, sdm_buf.src.w, sdm_buf.src.h);
        APP_DBG("start - x - y - w - h | (%d,%d) %dx%d\n",
                sdm_buf.start.x, sdm_buf.start.y, sdm_buf.start.w, sdm_buf.start.h);
        APP_DBG("mapto - x - y - w - h | (%d,%d) %dx%d\n",
                sdm_buf.dst.x, sdm_buf.dst.y, sdm_buf.dst.w, sdm_buf.dst.h);

        APP_DBG("layer:%d - en:%d alpha:%d - en:%d z-order:%d fmt:0x%x\n",
                sdm_buf.layer, sdm_buf.layer_en, sdm_buf.alpha, sdm_buf.alpha_en, sdm_buf.z_order, sdm_buf.fmt);
#elif (PLAYER_LAYERS == 2)
        fill_sdm_buf(&sdm_bufs[0], disp_req.bufY, disp_req.strideY);
        crop_sdm_buf(&sdm_bufs[0], posx_left, 0, AVM_WIDTH, AVM_HEIGHT);
        map_sdm_buf(&sdm_bufs[0], 0, 0, AVM_WIDTH, AVM_HEIGHT);
        sdm_bufs[1].addr[0]       = disp_req.bufCb; // bufCb is used for uyvy single image
        sdm_bufs[1].src_stride[0] = disp_req.bufCr; // bufCr is used for uyvy stride
        posx_left = 0 + (disp_req.strideY / 2 * 0);
        crop_sdm_buf(&sdm_bufs[1], posx_left, 0, IMG_WIDTH, IMG_HEIGHT);
        map_sdm_buf(&sdm_bufs[1], SINGLE_IMG_X, SINGLE_IMG_Y, SINGLE_IMG_WIDTH, SINGLE_IMG_HEIGHT); // will be handled by G2DLITE
        post_data.bufs             = sdm_bufs;
        post_data.n_bufs           = 2;
        post_data.custom_data      = NULL;
        post_data.custom_data_size = 0;

        APP_DBG("src1 - Y - CB - CR - Stride | %x - %x - %x - %d\n",
                sdm_bufs[0].addr[0], sdm_bufs[0].addr[1], sdm_bufs[0].addr[2], sdm_bufs[0].src_stride[0]);
        APP_DBG("src2 - Y - CB - CR - Stride | %x - %x - %x - %d\n",
                sdm_bufs[1].addr[0], sdm_bufs[1].addr[1], sdm_bufs[1].addr[2], sdm_bufs[1].src_stride[0]);
#endif
        APP_DBG("--wait start@%lu\n", current_time());
#ifdef PLAYER_USE_FPS_TIMER
        event_wait(&fps_tmr_cb_signal);
#else
        event_wait(evt_done);
#endif

        APP_DBG("--post start@%lu\n", current_time());
#if (PLAYER_USE_DISPLAY == 0)
        if (sdm_post(display0->handle, &post_data) != 0) {
            printf("sdm_post error\n");
        }
#elif (PLAYER_USE_DISPLAY == 1)
        if (sdm_post(display1->handle, &post_data) != 0) {
            printf("sdm_post error\n");
        }
#endif

        if(disp_req.end) {
            APP_DBG("last frame to disp.\n");
            break;
        }
    }

    vstream_destroy(strm);
}
