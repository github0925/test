/**
 * @file lv_demo_cluster.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <err.h>
#include <platform.h>
#include "lv_demo_cluster.h"
#include "lvgl_gui.h"
#include <surface_info.h>
#include "lv_gpu_sdrv_dma2d.h"
#include "sd_graphics.h"
#include <stdio.h>
#include <early_app_common.h>
#include "early_app_cfg.h"
#include <dcf_common.h>
/*********************
 *      DEFINES
 *********************/
#define CLUSTER_BG_PATH     "S:/early_app/cluster/cluster_bg.png"
#define RPM_BG_PATH         "S:/early_app/cluster/rpm_bg_number.png"
#define SPEED_BG_PATH       "S:/early_app/cluster/speed_bg_number.png"
#define NEEDLE_PATH         "S:/early_app/cluster/needle.png"
#define LEFT_TURN_PATH         "S:/early_app/cluster/LeftTurn.png"
#define RIGHT_TURN_PATH         "S:/early_app/cluster/rightTurn.png"
#define RES_PATH         "S:/early_app/cluster/res.bin"
#define RPM_SMALL_PATH         "S:/early_app/cluster/rpm_bg_small.png"
#define SPEED_SMALL_PATH         "S:/early_app/cluster/speed_bg_small.png"
#define TOP_BAR_PATH     "S:/early_app/cluster/top_bar.png"
#define BOTTOM_BAR_PATH     "S:/early_app/cluster/botom_bar.png"

#define number_size 32
#define RES_DATA_SIZE number_size*number_size*4*11
/**********************
 *      TYPEDEFS
 **********************/
enum {
    LV_METER_STATE_LARGE = 0,
    LV_METER_STATE_SMALL = 100,
    LV_METER_STATE_LARGE_TO_SMALL,
    LV_METER_STATE_SMALL_TO_LARGE,
};

/**********************
 *  STATIC PROTOTYPES
 **********************/
int load_file(const void *src, uint8_t *data, uint32_t read_len);
void fg_proc_task(token_handle_t token);

/**********************
 *  STATIC VARIABLES
 **********************/
static bool has_init = false;
static sdm_display_t * m_sdm;
static int CLUSTER_DISPLAY_ID = CLUSTER;
static int disp_width = 1920;
static int disp_height = 720;
static int rpm_x = 104;
static int rpm_y = 76;
static int rpm_width = 600;
static int rpm_height = 600;
static int rpm_cx = 404;
static int rpm_cy = 376;
static int speed_x = 1216;
static int speed_y = 76;
static int speed_width = 600;
static int speed_height = 600;
static int speed_cx = 1516;
static int speed_cy = 376;
static int anim_move_x_rate = 0;
static int anim_move_y_rate = 0;
static float anim_scale_rate = 0.0f;
static int needle_x = 288;
static int needle_y = 64;
static int needle_width = 24;
static int needle_height = 256;
static int leftturn_x = 440;
static int rightturn_x = 1436;
static int turnlight_y = 10;
static int leftturn_width = 44;
static int leftturn_height = 37;
static int rightturn_width = 44;
static int rightturn_height = 37;
static int meter_small_width = 452;
static int meter_small_height = 452;
static int topbar_x = 380;
static int topbar_y = 0;
static int topbar_width = 1160;
static int topbar_height = 110;
static int bottombar_x = 480;
static int bottombar_y = 601;// move down 1 px
static int bottombar_width = 960;
static int bottombar_height = 119;// move down 1 px
static bool is_displaysharing = false;
static int animstate = LV_METER_STATE_LARGE;
static int anim_frame = 50;
static int cur_anim_frame = 0;
static int start_needle_angle = -1350;
static int end_needle_angle = 1350;
static int cur_needle_angle = -1350;
static bool up = true;
static int frame_index = 0;
static float fps = 60.0f;
static int fps_x = 30;
static bool should_show_turnlight = false;
static struct surface_buff* surfacebuff = NULL;
static status_t receive_status = NO_ERROR;
static lv_area_t full_area;
static lv_area_t bg_area;
static lv_area_t rpm_area;
static lv_area_t rpm_draw_area;
static lv_area_t speed_area;
static lv_area_t speed_draw_area;
static lv_area_t meter_small_area;
static lv_area_t rpm_small_draw_area;
static lv_area_t speed_small_draw_area;
lv_img_decoder_dsc_t bg_dsc;
lv_img_decoder_dsc_t rpm_dsc;
lv_img_decoder_dsc_t speed_dsc;
lv_img_decoder_dsc_t needle_dsc;
lv_img_decoder_dsc_t leftturn_dsc;
lv_img_decoder_dsc_t rightturn_dsc;
lv_img_decoder_dsc_t rpm_small_dsc;
lv_img_decoder_dsc_t speed_small_dsc;
//lv_img_decoder_dsc_t top_bar_dsc;
lv_img_decoder_dsc_t bottom_bar_dsc;

static uint8_t* res_data = NULL;
static bool cluster_start = false;

uint8_t* bg_fb1 = NULL;
uint8_t* bg_fb2 = NULL;
uint8_t* fg_fb1 = NULL;
uint8_t* fg_fb2 = NULL;
uint8_t* bg_buf_act = NULL;
uint8_t* fg_buf_act = NULL;
uint8_t* bg_src_buf = NULL;
uint8_t* fg_blank_buf = NULL;
SD1_POINT_T* Point_Add = NULL;

event_t fg_start_event;
event_t fg_end_event;
uint32_t fg_thread_time = 0;
/**********************
 *      MACROS
 **********************/
extern bool cluster_should_show_fps;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void lv_cluster_init(void* token)
{
    unified_service_publish(token,ussInit);
    printf(" lv_demo_cluster !!!!!!\n");
    //start lvgl
    lvgl_init();

    /* cluster on the 3rd display */
    struct list_node *head = sdm_get_display_list();
    list_for_every_entry(head, m_sdm, sdm_display_t, node) {
        if (m_sdm->handle->display_id == CLUSTER_DISPLAY_ID)
            break;
    }
    printf("cluster display_id %d\n", m_sdm->handle->display_id);

    //start decoder
    uint32_t app_start_time = current_time();

    lv_res_t ret = lv_img_decoder_open(&bg_dsc, CLUSTER_BG_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || bg_dsc.img_data == NULL)
    {
        printf("open bg failed \n");
        return;
    }

    // decode meter img
    ret = lv_img_decoder_open(&rpm_dsc, RPM_BG_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || rpm_dsc.img_data == NULL)
    {
        printf("open rpm failed \n");
        return;
    }

    ret = lv_img_decoder_open(&speed_dsc, SPEED_BG_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || speed_dsc.img_data == NULL)
    {
        printf("open speed failed \n");
        return;
    }

    //decode needle img
    ret = lv_img_decoder_open(&needle_dsc, NEEDLE_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || needle_dsc.img_data == NULL)
    {
        printf("open needle failed \n");
        return;
    }
    //decode turn light img
    ret = lv_img_decoder_open(&leftturn_dsc, LEFT_TURN_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || leftturn_dsc.img_data == NULL)
    {
        printf("open leftturn failed \n");
        return;
    }
    ret = lv_img_decoder_open(&rightturn_dsc, RIGHT_TURN_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || rightturn_dsc.img_data == NULL)
    {
        printf("open rightturn failed \n");
        return;
    }

    ret = lv_img_decoder_open(&rpm_small_dsc, RPM_SMALL_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || rpm_small_dsc.img_data == NULL)
    {
        printf("open rpm small failed \n");
        return;
    }
    ret = lv_img_decoder_open(&speed_small_dsc, SPEED_SMALL_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || speed_small_dsc.img_data == NULL)
    {
        printf("open speed small failed \n");
        return;
    }
    // ret = lv_img_decoder_open(&top_bar_dsc, TOP_BAR_PATH, LV_COLOR_RED);
    // if (ret != LV_RES_OK || top_bar_dsc.img_data == NULL)
    // {
    //     printf("open top bar failed \n");
    //     return;
    // }
    ret = lv_img_decoder_open(&bottom_bar_dsc, BOTTOM_BAR_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || bottom_bar_dsc.img_data == NULL)
    {
        printf("open bottom bar failed \n");
        return;
    }
    uint32_t decode_time = current_time() - app_start_time;

    rpm_width = rpm_dsc.header.w;
    rpm_height = rpm_dsc.header.h;
    speed_width = speed_dsc.header.w;
    speed_height = speed_dsc.header.h;
    needle_width = needle_dsc.header.w;
    needle_height = needle_dsc.header.h;
    leftturn_width = leftturn_dsc.header.w;
    leftturn_height = leftturn_dsc.header.h;
    rightturn_width = rightturn_dsc.header.w;
    rightturn_height = rightturn_dsc.header.h;

    full_area.x1 = 0;
    full_area.y1 = 0;
    full_area.x2 = disp_width - 1;
    full_area.y2 = disp_height - 1;

    bg_area.x1 = 0;
    bg_area.y1 = 0;
    bg_area.x2 = disp_width - 1;
    bg_area.y2 = disp_height - 1;

    rpm_area.x1 = 0;
    rpm_area.y1 = 0;
    rpm_area.x2 = rpm_width - 1;
    rpm_area.y2 = rpm_height - 1;

    rpm_draw_area.x1 = rpm_x;
    rpm_draw_area.y1 = rpm_y;
    rpm_draw_area.x2 = rpm_x + rpm_width - 1;
    rpm_draw_area.y2 = rpm_y + rpm_height - 1;


    speed_area.x1 = 0;
    speed_area.y1 = 0;
    speed_area.x2 = speed_width - 1;
    speed_area.y2 = speed_height - 1;

    speed_draw_area.x1 = speed_x;
    speed_draw_area.y1 = speed_y;
    speed_draw_area.x2 = speed_x + speed_width - 1;
    speed_draw_area.y2 = speed_y + speed_height - 1;

    meter_small_area.x1 = 0;
    meter_small_area.y1 = 0;
    meter_small_area.x2 = meter_small_width - 1;
    meter_small_area.y2 = meter_small_height - 1;

    rpm_small_draw_area.x1 = 28;
    rpm_small_draw_area.y1 = 250;
    rpm_small_draw_area.x2 = 28 + meter_small_width - 1;
    rpm_small_draw_area.y2 = 250 + meter_small_height - 1;

    speed_small_draw_area.x1 = 1440;
    speed_small_draw_area.y1 = 250;
    speed_small_draw_area.x2 = 1440 + meter_small_width - 1;
    speed_small_draw_area.y2 = 250 + meter_small_height - 1;

    res_data = memalign(0x1000, RES_DATA_SIZE);
    load_file(RES_PATH, res_data, RES_DATA_SIZE);

    event_init(&fg_start_event, false, EVENT_FLAG_AUTOUNSIGNAL);
    event_init(&fg_end_event, false, EVENT_FLAG_AUTOUNSIGNAL);

    printf("decode and show bg time: %d\n", decode_time);
    has_init = true;

    unified_service_publish(token,ussReady);
}

void lv_demo_cluster_property_update(bool remote, uint16_t val)
{
    int v;
    system_property_get(DMP_ID_CLUSTER_STATUS,&v);
    if(remote)
    {
        v &= 0xFFFF0000;
        v |= (val << 16);
    }
    else
    {
        v &= 0x0000FFFF;
        v |= val;
    }

    system_property_set(DMP_ID_CLUSTER_STATUS,v);
}

void lv_cluster_start(void* token) {
    if (!has_init) {
        printf("lv_cluster_start no init\n");
        return;
    }
    unified_service_publish(token,ussRun);

    bg_fb1 = memalign(0x1000, disp_width*disp_height*4);
    bg_fb2 = memalign(0x1000, disp_width*disp_height*4);
    fg_fb1 = memalign(0x1000, disp_width*disp_height*4);
    fg_fb2 = memalign(0x1000, disp_width*disp_height*4);

    bg_src_buf = memalign(0x1000, disp_width*disp_height*4);
    lv_gpu_sdrv_g2d_copy((lv_color_t *)(bg_src_buf), disp_width, (lv_color_t *)(bg_dsc.img_data), disp_width, disp_width, disp_height);

    fg_blank_buf = memalign(0x1000, disp_width*disp_height*4);
    memset(fg_blank_buf, 0x0, disp_width * disp_height * 4);
    arch_clean_cache_range((addr_t)fg_blank_buf, disp_width*disp_height*4);
    //lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_blank_buf) + topbar_x, disp_width, (lv_color_t *)(top_bar_dsc.img_data), topbar_width, topbar_width, topbar_height);
    lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_blank_buf) + bottombar_x + bottombar_y*disp_width, disp_width, (lv_color_t *)(bottom_bar_dsc.img_data), bottombar_width, bottombar_width, bottombar_height);

    Point_Add = memalign(0x1000, 10*1024*sizeof(SD1_POINT_T));

    struct sdm_buffer sdm_buf[2] = DISPLAY_CLUSTER_TEMPLATE;
    struct sdm_post_config post_data;

    cluster_start = true;

    surface_receiver_init();
    token_handle_t surface_recv_token = token_create("surface_recv",NULL,400,NULL,NULL);
    thread_t* surfacerecvthread = thread_create("surface_recv",(thread_start_routine)surface_receiver_task,surface_recv_token,DEFAULT_PRIORITY,1024);
    thread_detach_and_resume(surfacerecvthread);

    token_handle_t fg_proc_token = token_create("cluster_fg_proc",NULL,400,NULL,NULL);
    thread_t* fgprocthread = thread_create("cluster_fg_proc",(thread_start_routine)fg_proc_task,fg_proc_token,DEFAULT_PRIORITY,2048);
    thread_detach_and_resume(fgprocthread);

    uint32_t stage1_time = 0;
    uint32_t stage2_time = 0;
    uint32_t stage3_time = 0;
    uint32_t all_stage_time = 0;
    uint32_t frame_start_time = 0;
    uint32_t cur_time = current_time();
    uint32_t pre_time = current_time();
    float fps = 59.9;
    uint32_t fps_cur = (uint32_t)(fps*10);
    uint32_t fps_start_time = current_time();
    uint32_t fps_gap_time = 0;
    bg_buf_act = bg_fb1;
    fg_buf_act = fg_fb1;
    int bg_changed = 2;//2-1-0
    bool should_change_fmt = false;
    bool animal_flag = false;
    while(cluster_start) {
        frame_start_time = cur_time = current_time();
        pre_time = cur_time;

        //swap buffer
        if(bg_fb1 && bg_fb2) {
            if(bg_buf_act == bg_fb1)
                bg_buf_act = bg_fb2;
            else
                bg_buf_act = bg_fb1;
        }

        if(fg_fb1 && fg_fb2) {
            if(fg_buf_act == fg_fb1)
                fg_buf_act = fg_fb2;
            else
                fg_buf_act = fg_fb1;
        }

        // display sharing (1ms)
        receive_status = surface_receiver_dequeue(&surfacebuff, 1);

        if (receive_status == NO_ERROR)
        {
            if (surfacebuff->surface.cmd == surface_info && is_displaysharing) {
                if(surfacebuff->surface.phy_addr != 0) {
                    uint8_t* data = (uint8_t *)((uint32_t)(surfacebuff->surface.phy_addr-0x10000000));
                    int start_x = (disp_width-surfacebuff->surface.width)/2;
                    int start_y = (disp_height-surfacebuff->surface.height)/2;
                    lv_gpu_sdrv_g2d_copy((lv_color_t *)(bg_src_buf) + start_y*disp_width + start_x, disp_width, (lv_color_t *)(data), surfacebuff->surface.width, surfacebuff->surface.width, surfacebuff->surface.height);
                    bg_changed = 2;
                    should_change_fmt = true;
                }
            }
            else if (surfacebuff->surface.cmd == surface_start) {
                LV_LOG_WARN("surface start \n");
                is_displaysharing = true;
                lv_gpu_sdrv_g2d_copy((lv_color_t *)(bg_src_buf), disp_width, (lv_color_t *)(fg_blank_buf), disp_width, disp_width, disp_height);
            }
            else if (surfacebuff->surface.cmd == surface_end) {
                LV_LOG_WARN("surface end \n");
                is_displaysharing = false;
                lv_gpu_sdrv_g2d_copy((lv_color_t *)(bg_src_buf), disp_width, (lv_color_t *)(bg_dsc.img_data), disp_width, disp_width, disp_height);
                bg_changed = 2;
                should_change_fmt = false;
            }

            surface_receiver_enqueue();
        }

        if (is_displaysharing) {
            if (cur_anim_frame < anim_frame) {
                cur_anim_frame++;
                bg_changed = 2;
            }

            if (cur_anim_frame >= anim_frame) {
                cur_anim_frame = anim_frame;
                animstate = LV_METER_STATE_SMALL;
            }
        } else {
            if (cur_anim_frame > 0) {
                cur_anim_frame--;
                bg_changed = 2;
            }
            if (cur_anim_frame <= 0) {
                cur_anim_frame = 0;
                animstate = LV_METER_STATE_LARGE;
            }
        }

        anim_move_x_rate = cur_anim_frame*3;
        anim_move_y_rate = cur_anim_frame*2;
        anim_scale_rate = 1.0f - ((float)cur_anim_frame)/200.0f;

        //clear fg framebuffer
        lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act), disp_width, (lv_color_t *)(fg_blank_buf), disp_width, disp_width, disp_height);

        //draw fps to fg
        if (cluster_should_show_fps) {
            fps_cur = (uint32_t)(fps*10);
            lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + 10*disp_width + fps_x, disp_width, (lv_color_t *)(res_data + number_size*number_size*(fps_cur/100)*4), number_size, number_size, number_size);
            lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + 10*disp_width + fps_x + number_size, disp_width, (lv_color_t *)(res_data + number_size*number_size*((fps_cur%100)/10)*4), number_size, number_size, number_size);
            lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + 10*disp_width + fps_x + 2*number_size, disp_width, (lv_color_t *)(res_data + number_size*number_size*10*4), number_size, number_size, number_size);
            lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + 10*disp_width + fps_x + 3*number_size, disp_width, (lv_color_t *)(res_data + number_size*number_size*(fps_cur%10)*4), number_size, number_size, number_size);
        }

        //draw turn light to fg
        if (should_show_turnlight) {
            lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + turnlight_y*disp_width + leftturn_x, disp_width, (lv_color_t *)(leftturn_dsc.img_data), leftturn_width, leftturn_width, leftturn_height);
            lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + turnlight_y*disp_width + rightturn_x, disp_width, (lv_color_t *)(rightturn_dsc.img_data), rightturn_width, rightturn_width, rightturn_height);
        }

        cur_time = current_time();
        stage1_time = cur_time - pre_time;
        pre_time = cur_time;

        //start fg thread draw needle
        event_signal(&fg_start_event,true);
        if (bg_changed > 0) {
            bg_changed--;

            if (cur_anim_frame < anim_frame-1) {
                rpm_draw_area.x1 = rpm_cx - anim_move_x_rate - (int)(anim_scale_rate*rpm_width/2);
                rpm_draw_area.y1 = rpm_cy + anim_move_y_rate - (int)(anim_scale_rate*rpm_height/2);
                rpm_draw_area.x2 = rpm_cx - anim_move_x_rate + (int)(anim_scale_rate*rpm_width/2) - 1;
                rpm_draw_area.y2 = rpm_cy + anim_move_y_rate + (int)(anim_scale_rate*rpm_height/2) - 1;
                //blend bg_src and rpm to bg_act
                if (should_change_fmt) {
                    lv_gpu_sdrv_g2d_blend_sharing_scale((lv_color_t *)bg_buf_act, disp_width*4,
                                                (lv_color_t *)rpm_dsc.img_data, rpm_width*4,
                                                (lv_color_t *)bg_src_buf, disp_width*4,
                                                &full_area,
                                                &rpm_area, &rpm_draw_area,
                                                &bg_area, &full_area);
                } else {
                    lv_gpu_sdrv_g2d_blend_scale((lv_color_t *)bg_buf_act, disp_width*4,
                                                (lv_color_t *)rpm_dsc.img_data, rpm_width*4,
                                                (lv_color_t *)bg_src_buf, disp_width*4,
                                                &full_area,
                                                &rpm_area, &rpm_draw_area,
                                                &bg_area, &full_area);
                }


                speed_draw_area.x1 = speed_cx + anim_move_x_rate - (int)(anim_scale_rate*speed_width/2);
                speed_draw_area.y1 = speed_cy + anim_move_y_rate - (int)(anim_scale_rate*speed_height/2);
                speed_draw_area.x2 = speed_cx + anim_move_x_rate + (int)(anim_scale_rate*speed_width/2) - 1;
                speed_draw_area.y2 = speed_cy + anim_move_y_rate + (int)(anim_scale_rate*speed_height/2) - 1;
                //blend bg_act and speed to bg_act
                lv_gpu_sdrv_g2d_blend_scale((lv_color_t *)bg_buf_act, disp_width*4,
                                            (lv_color_t *)speed_dsc.img_data, speed_width*4,
                                            (lv_color_t *)bg_buf_act, disp_width*4,
                                            &full_area,
                                            &speed_area, &speed_draw_area,
                                            &bg_area, &full_area);
            } else if(cur_anim_frame >= anim_frame-1) {

                lv_gpu_sdrv_g2d_blend_sharing((lv_color_t *)bg_buf_act, disp_width*4,
                                  (lv_color_t *)rpm_small_dsc.img_data, meter_small_width*4,
                                  (lv_color_t *)bg_src_buf, disp_width*4,
                                  &full_area,
                                  &meter_small_area, &rpm_small_draw_area,
                                  &bg_area, &full_area);

                // lv_gpu_sdrv_g2d_blend_nocache((lv_color_t *)bg_buf_act, disp_width*4,
                //                   (lv_color_t *)speed_small_dsc.img_data, meter_small_width*4,
                //                   (lv_color_t *)bg_buf_act, disp_width*4,
                //                   &full_area,
                //                   &meter_small_area, &speed_small_draw_area,
                //                   &bg_area, &full_area);

                lv_gpu_sdrv_g2d_blend_nocache((lv_color_t *)bg_buf_act + speed_small_draw_area.x1 + speed_small_draw_area.y1*disp_width, disp_width*4,
                                  (lv_color_t *)speed_small_dsc.img_data, meter_small_width*4,
                                  (lv_color_t *)bg_buf_act, disp_width*4,
                                  &meter_small_area,
                                  &meter_small_area, &meter_small_area,
                                  &speed_small_draw_area, &meter_small_area);
            }
        }

        cur_time = current_time();
        stage2_time = cur_time - pre_time;
        pre_time = cur_time;
        event_wait(&fg_end_event);

        cur_time = current_time();
        all_stage_time = cur_time - frame_start_time;
        pre_time = cur_time;

        //post
        sdm_buf[1].addr[0] = (unsigned long)fg_buf_act;
        sdm_buf[0].addr[0] = (unsigned long)bg_buf_act;

        sdm_buf[1].src_stride[0] = disp_width*4;
        sdm_buf[0].src_stride[1] = disp_width*4;

        post_data.bufs             = sdm_buf;
        post_data.n_bufs           = 2;
        if (!animal_flag) {
            lv_demo_cluster_property_update(1,1);
            animal_flag = true;
        }
        sdm_post(m_sdm->handle, &post_data);

        //angle
        if (cur_needle_angle >= end_needle_angle){
            up = false;
        } else if (cur_needle_angle <= start_needle_angle){
            up = true;
        }

        if (up) {
            cur_needle_angle += 10;
        } else {
            cur_needle_angle -= 10;
        }

        frame_index++;
        if (frame_index >= 3600) {
            frame_index = 0;
        }

        if (frame_index <= 600  && frame_index%30 == 0 ) {
            should_show_turnlight = !should_show_turnlight;
        } else if (frame_index > 600) {
            should_show_turnlight = false;
        }

        if (frame_index%120 == 0) {
            fps_gap_time = current_time() - fps_start_time;
            if (fps_gap_time != 0) {
                fps = 120*1000.0f/fps_gap_time;
                if (fps > 99) {
                    fps = 99.9f;
                }
            }
            fps_start_time = current_time();
            //printf("fps:%f, stage1_time:%d, stage2_time:%d, fg_thread_time:%d, all_stage_time:%d\n", fps, stage1_time, stage2_time, fg_thread_time, all_stage_time);
        }
    }
}

void lv_cluster_stop(void* token) {

    cluster_start = false;
}

void lv_cluster_deinit(void* token) {
    has_init = false;
    if (bg_fb1 != NULL) {
        free(bg_fb1);
        bg_fb1 = NULL;
    }
    if (bg_fb2 != NULL) {
        free(bg_fb2);
        bg_fb2 = NULL;
    }
    if (fg_fb1 != NULL) {
        free(fg_fb1);
        fg_fb1 = NULL;
    }
    if (fg_fb2 != NULL) {
        free(fg_fb2);
        fg_fb2 = NULL;
    }
    if (bg_src_buf != NULL) {
        free(bg_src_buf);
        bg_src_buf = NULL;
    }
    if (fg_blank_buf != NULL) {
        free(fg_blank_buf);
        fg_blank_buf = NULL;
    }
    if (Point_Add != NULL) {
        free(Point_Add);
        Point_Add = NULL;
    }
    if (res_data != NULL) {
        free(res_data);
        res_data = NULL;
    }

    lv_img_decoder_close(&bg_dsc);
    lv_img_decoder_close(&rpm_dsc);
    lv_img_decoder_close(&speed_dsc);
    lv_img_decoder_close(&needle_dsc);
    lv_img_decoder_close(&leftturn_dsc);
    lv_img_decoder_close(&rightturn_dsc);
    lv_img_decoder_close(&rpm_small_dsc);
    lv_img_decoder_close(&speed_small_dsc);
    //lv_img_decoder_close(&top_bar_dsc);
    lv_img_decoder_close(&bottom_bar_dsc);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
int load_file(const void *src, uint8_t *data, uint32_t read_len) {
    lv_fs_file_t file;
    lv_fs_res_t res;
    res = lv_fs_open(&file, src, LV_FS_MODE_RD);
    if (res != LV_FS_RES_OK) {
        printf("open failed\n");
        return res;
    }

    uint32_t read_num;
    res = lv_fs_read(&file, data, read_len, &read_num);
    lv_fs_close(&file);

    if(res != LV_FS_RES_OK || read_num != read_len) {
        printf("read failed read_num: %d, read_len: %d\n", read_num, read_len);
    }

    return res;
}

void fg_proc_task(token_handle_t token)
{
    SD_SIZE_T window = {disp_width, disp_height};
    SD_SIZE_T img = {needle_width, needle_height};
    SD_POINT_T img_mid = {needle_width/2, needle_height - 14};
    uint32_t pretime = 0;
    while(cluster_start) {
        //wait event
        event_wait(&fg_start_event);
        pretime = current_time();

        int32_t z_param = (int)(4096*anim_scale_rate);
        //draw needle
        memset(Point_Add, 0 , 10*1024*sizeof(SD1_POINT_T));
        SD_POINT_T rpm_window_mid = {rpm_cx - anim_move_x_rate,  rpm_cy + anim_move_y_rate};
        SD_rotation(window,img,rpm_window_mid,img_mid,cur_needle_angle,(uint32_t*)needle_dsc.img_data,(uint32_t*)fg_buf_act,z_param,Point_Add);

        memset(Point_Add, 0 , 10*1024*sizeof(SD1_POINT_T));
        SD_POINT_T speed_window_mid = {speed_cx + anim_move_x_rate,  speed_cy + anim_move_y_rate};
        SD_rotation(window,img,speed_window_mid,img_mid,-cur_needle_angle,(uint32_t*)needle_dsc.img_data,(uint32_t*)fg_buf_act,z_param,Point_Add);

        //clean cache
        arch_clean_cache_range((addr_t)(fg_buf_act + 130*disp_width*4), disp_width*(disp_height - 260)*4);

        fg_thread_time = current_time() - pretime;
        //send finish event
        event_signal(&fg_end_event,true);
    }
}