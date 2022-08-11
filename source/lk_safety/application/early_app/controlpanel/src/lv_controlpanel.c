/**
 * @file lv_controlpanel.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <err.h>
#include <platform.h>
#include "lv_controlpanel.h"
#include "lvgl_gui.h"
#include <lk_wrapper.h>
#include <sdm_display.h>

#if 1
/*********************
 *      DEFINES
 *********************/
#define TMP_VALUE "16.0\n16.5\n17.0\n18.0\n18.5\n19.0\n19.5\n20.0\n20.5\n21.0\n21.5\n22.0\n22.5\n23.0\n23.5\n24.0\n24.5\n25.0\n25.5\n26.0\n26.5\n27.0\n27.5\n28.0"
#define BTN_SIZE 112

#define PNG_RES_PATH(x) "S:/early_app/controlpanel/"x".png"
/**********************
 *      TYPEDEFS
 **********************/
extern bool cluster_should_show_fps;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void init_style(void);
static void content_create(lv_obj_t * parent);
static lv_obj_t * control_btns_create(lv_obj_t * parent);
static lv_obj_t * control_fans_create(lv_obj_t * parent);
static lv_obj_t * control_tmp_create(lv_obj_t * parent);

static lv_obj_t * control_chair_create(lv_obj_t * parent);

static void slider_event_handler(lv_obj_t * obj, lv_event_t event);
#if LV_USE_THEME_MATERIAL
static void color_chg_event_cb(lv_obj_t * sw, lv_event_t e);
#endif

static void imgbtn_event_handler(lv_obj_t *obj, lv_event_t e);

static void fan_animation(lv_obj_t * slider, lv_anim_value_t value);
static void tmp_animation(lv_obj_t * slider, lv_anim_value_t value);
static void gif_animation(lv_obj_t * img, lv_anim_value_t value);
static void rotation_animation(lv_obj_t * img, lv_anim_value_t value);

static void on_fan_value_changed(lv_obj_t * img, lv_obj_t * label, int16_t value);

static void run_test(void);

static void show_bg(lv_obj_t * parent, const void * bg_img);
static void show_bg_hw_layer(lv_obj_t * parent, const void * bg_img);
/**********************
 *  STATIC VARIABLES
 **********************/
static lv_style_t style_roller;
static lv_style_t style_roller_seleted;
static lv_style_t style_btn;
static lv_style_t style_obj;
static lv_style_t style_slider;

static lv_obj_t * front_tmp_roller;
static lv_obj_t * behind_tmp_roller;
static lv_obj_t * all_ac_btn;
static lv_obj_t * behind_ac_btn;
static lv_obj_t * sync_btn;
static lv_obj_t * auto_btn;
static lv_obj_t * loop_btn;
static lv_obj_t * front_defog_btn;
static lv_obj_t * behind_defog_btn;

static lv_obj_t * left_chair_heart_state_img;
static lv_obj_t * right_chair_heart_state_img;

static lv_obj_t * front_ac_switch_ib;
static lv_obj_t * back_ac_switch_ib;

static lv_obj_t * front_fan_state_ib;
static lv_obj_t * back_fan_state_ib;
static lv_obj_t * front_fan_state_l;
static lv_obj_t * back_fan_state_l;

static lv_obj_t * front_fan_slider;
static lv_obj_t * back_fan_slider;

static lv_obj_t * center_gif_img;
static lv_obj_t * center_gif_img2;
static lv_obj_t * center_gif_img3;

static lv_obj_t * front_fan_icon;
static lv_obj_t * back_fan_icon;

static lv_anim_t anim_gif;
static lv_anim_t anim_rotation_front;
static lv_anim_t anim_rotation_back;

//static lv_obj_t *


bool all_ac_checked = false;
bool behind_ac_checked = false;
bool sync_checked = false;
bool auto_checked = false;
bool is_inner_loop = true;
bool front_defog_checked = false;
bool behind_defog_checked = false;
bool front_off = false;
bool back_off = false;

int16_t front_fan_state = 0;
int16_t back_fan_state = 1;
int sp = 0;

static int CONTROLPANEL_DISPLAY_ID = CONTROLPANEL;

static sdm_display_t * m_sdm;
/**********************
 *      MACROS
 **********************/
#define BG_USE_HW_LAYER 0

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_controlpanel(void)
{
    //start lvgl, Reentrant
    lvgl_init();

    /* cluster on the 3rd display */
    struct list_node *head = sdm_get_display_list();
    list_for_every_entry(head, m_sdm, sdm_display_t, node) {
        if (m_sdm->handle->display_id == CONTROLPANEL_DISPLAY_ID)
            break;
    }
    LV_LOG_WARN("controlpanel display_id %d\n", m_sdm->handle->display_id);

    lv_disp_t * disp = get_display(CONTROLPANEL_DISPLAY_ID);//lv_disp_get_next(NULL);

    if (lv_disp_get_hor_res(disp) == 1280) {
        sp = 1;
    }
    lv_obj_t * scr = lv_disp_get_scr_act(disp);

    init_style();

    content_create(scr);

    //run_test();
}

/**********************
 *   INIT STYLE
 **********************/
static void init_style(void)
{
    lv_style_init(&style_roller);
    lv_style_set_text_font(&style_roller,LV_STATE_DEFAULT, &lv_font_montserrat_40);
    lv_style_set_bg_grad_color(&style_roller,LV_STATE_DEFAULT, LV_COLOR_TRANSP);
    lv_style_set_image_opa(&style_roller,LV_STATE_DEFAULT, 0);
    lv_style_set_border_opa(&style_roller,LV_STATE_DEFAULT,0);
    lv_style_set_border_opa(&style_roller,LV_STATE_FOCUSED,0);
    lv_style_set_bg_opa(&style_roller,LV_STATE_DEFAULT,0);
    lv_style_set_value_opa(&style_roller,LV_STATE_DEFAULT,0);

    lv_style_init(&style_roller_seleted);
    lv_style_set_text_font(&style_roller_seleted,LV_STATE_DEFAULT, &lv_font_montserrat_46);
    lv_style_set_bg_grad_color(&style_roller_seleted,LV_STATE_DEFAULT, LV_COLOR_TRANSP);
    lv_style_set_image_opa(&style_roller_seleted,LV_STATE_DEFAULT, 0);
    lv_style_set_border_opa(&style_roller_seleted,LV_STATE_DEFAULT,0);
    lv_style_set_border_opa(&style_roller_seleted,LV_STATE_FOCUSED,0);
    lv_style_set_bg_opa(&style_roller_seleted,LV_STATE_DEFAULT,0);
    lv_style_set_value_opa(&style_roller_seleted,LV_STATE_DEFAULT,0);

    lv_style_init(&style_btn);
    lv_style_set_text_font(&style_btn,LV_STATE_DEFAULT, &lv_font_montserrat_30);
    lv_style_set_text_color(&style_btn,LV_STATE_DEFAULT,LV_COLOR_WHITE);

    lv_style_init(&style_obj);
    lv_style_set_border_opa(&style_obj,LV_STATE_DEFAULT,0);
    lv_style_set_border_opa(&style_obj,LV_STATE_FOCUSED,0);
    lv_style_set_bg_opa(&style_obj,LV_STATE_DEFAULT,0);
    lv_style_set_bg_opa(&style_obj,LV_STATE_FOCUSED,0);
    lv_style_set_text_font(&style_obj, LV_STATE_DEFAULT, &lv_font_montserrat_20);
    lv_style_set_text_color(&style_obj,LV_STATE_DEFAULT,LV_COLOR_WHITE);

    lv_style_init(&style_slider);
    lv_style_set_bg_opa(&style_slider,LV_STATE_DEFAULT,0);
    lv_style_set_opa_scale(&style_slider,LV_STATE_DEFAULT,0);
    lv_style_set_opa_scale(&style_slider,LV_STATE_DISABLED,0);
    lv_style_set_value_opa(&style_slider,LV_STATE_DEFAULT,0);
}


/**********************
 *   STATIC FUNCTIONS
 **********************/


static void content_create(lv_obj_t * parent)
{
    show_bg(parent, PNG_RES_PATH("img_background"));

    lv_obj_t * chair_layout = control_chair_create(parent);
    lv_obj_set_pos(chair_layout,660-320*sp,80+10*sp);

    lv_obj_t * tmps_layout = control_tmp_create(parent);
    lv_obj_set_pos(tmps_layout,250-160*sp,100+20*sp);

    lv_obj_t * fan_layout = control_fans_create(parent);
    lv_obj_set_pos(fan_layout,150-120*sp,400+50*sp);

    lv_obj_t * bottom_layout = control_btns_create(parent);
    lv_obj_set_pos(bottom_layout,100-38*sp,570+60*sp);

}

static void show_bg(lv_obj_t * parent, const void * bg_img)
{
#if BG_USE_HW_LAYER
    show_bg_hw_layer(parent, bg_img);
#else
    lv_obj_t * bg = lv_img_create(parent, NULL);
    lv_img_set_src(bg, bg_img);
    lv_obj_set_size(bg, lv_obj_get_width(parent),lv_obj_get_height(parent));
#endif
}

static lv_obj_t * control_chair_create(lv_obj_t * parent)
{
    lv_obj_t * chair_layout = lv_obj_create(parent,NULL);
    lv_obj_set_size(chair_layout,600,400);
    lv_obj_add_style(chair_layout, LV_CONT_PART_MAIN, &style_obj);

    lv_obj_t * chair_img = lv_img_create(chair_layout, NULL);
    lv_obj_set_pos(chair_img,143,20);
    lv_img_set_src(chair_img, PNG_RES_PATH("chair"));

    center_gif_img = lv_img_create(chair_layout,NULL);
    lv_obj_set_pos(center_gif_img,143,0);
    lv_img_set_src(center_gif_img, PNG_RES_PATH("gif0"));

    center_gif_img2 = lv_img_create(chair_layout,center_gif_img);
    lv_obj_set_pos(center_gif_img2,100,70);

    center_gif_img3 = lv_img_create(chair_layout,center_gif_img);
    lv_obj_set_pos(center_gif_img3,20,130);


    lv_anim_init(&anim_gif);
    lv_anim_set_var(&anim_gif,center_gif_img);
    lv_anim_set_time(&anim_gif, 2000);
    lv_anim_set_values(&anim_gif, 0, 12);
    //lv_anim_set_playback_time(anim_gif, 1000);
    lv_anim_set_exec_cb(&anim_gif,(lv_anim_exec_xcb_t)gif_animation);
    lv_anim_set_repeat_count(&anim_gif, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&anim_gif);

    lv_obj_t * chair_heat_img = lv_img_create(chair_layout, NULL);
    lv_obj_set_pos(chair_heat_img,187,322);
    lv_img_set_src(chair_heat_img, PNG_RES_PATH("chair_heat"));

    left_chair_heart_state_img = lv_img_create(chair_layout, NULL);
    lv_obj_set_pos(left_chair_heart_state_img,210,390);
    lv_img_set_src(left_chair_heart_state_img, PNG_RES_PATH("left_heat_1"));

    right_chair_heart_state_img = lv_img_create(chair_layout, NULL);
    lv_obj_set_pos(right_chair_heart_state_img,320,390);
    lv_img_set_src(right_chair_heart_state_img, PNG_RES_PATH("right_heat_1"));
    return chair_layout;
}

static lv_obj_t * control_tmp_create(lv_obj_t * parent)
{
    lv_obj_t * tmps_layout = lv_obj_create(parent,NULL);
    lv_obj_set_size(tmps_layout,1420-320*sp,300);
    lv_obj_add_style(tmps_layout, LV_CONT_PART_MAIN, &style_obj);

    front_tmp_roller = lv_roller_create(tmps_layout, NULL);

    lv_obj_set_size(front_tmp_roller,300,300);
    lv_obj_add_style(front_tmp_roller, LV_CONT_PART_MAIN, &style_roller);
    lv_obj_add_style(front_tmp_roller, LV_ROLLER_PART_SELECTED, &style_roller_seleted);
    lv_roller_set_auto_fit(front_tmp_roller, false);
    lv_roller_set_align(front_tmp_roller, LV_LABEL_ALIGN_CENTER);
    lv_roller_set_options(front_tmp_roller,TMP_VALUE
                          ,LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(front_tmp_roller,2,LV_ANIM_ON);
    lv_roller_set_visible_row_count(front_tmp_roller, 3);
    lv_obj_set_pos(front_tmp_roller,0,0);

    lv_obj_t *roller_select_bg1 = lv_img_create(tmps_layout,NULL);
    lv_img_set_src(roller_select_bg1, PNG_RES_PATH("background"));
    lv_obj_set_pos(roller_select_bg1,20,50);

    behind_tmp_roller = lv_roller_create(tmps_layout, NULL);
    lv_obj_add_style(behind_tmp_roller, LV_CONT_PART_MAIN, &style_roller);
    lv_obj_add_style(behind_tmp_roller, LV_ROLLER_PART_SELECTED, &style_roller_seleted);
    lv_roller_set_auto_fit(behind_tmp_roller, false);
    lv_roller_set_align(behind_tmp_roller, LV_LABEL_ALIGN_CENTER);
    lv_roller_set_options(behind_tmp_roller,TMP_VALUE
                          ,LV_ROLLER_MODE_NORMAL);
    lv_roller_set_selected(behind_tmp_roller,2,LV_ANIM_ON);
    lv_roller_set_visible_row_count(behind_tmp_roller, 3);
    lv_obj_set_pos(behind_tmp_roller,1120-320*sp,0);

    lv_obj_t *roller_select_bg2 = lv_img_create(tmps_layout,NULL);
    lv_img_set_src(roller_select_bg2, PNG_RES_PATH("background"));
    lv_obj_set_pos(roller_select_bg2,1140-320*sp,50);

    lv_obj_t *tmp_label = lv_label_create(tmps_layout,NULL);
    lv_label_set_text(tmp_label,"C");
    lv_obj_add_style(tmp_label,LV_CONT_PART_MAIN, &style_roller_seleted);
    lv_obj_set_pos(tmp_label,190,65);

    lv_obj_t *o_label = lv_label_create(tmps_layout,NULL);
    lv_label_set_text(o_label,"o");
    lv_obj_set_pos(o_label,180,65);

    lv_obj_t *tmp_label2 = lv_label_create(tmps_layout,NULL);
    lv_label_set_text(tmp_label2,"C");
    lv_obj_add_style(tmp_label2,LV_CONT_PART_MAIN, &style_roller_seleted);
    lv_obj_set_pos(tmp_label2,1310-320*sp,65);

    lv_obj_t *o2_label = lv_label_create(tmps_layout,NULL);
    lv_label_set_text(o2_label,"o");
    lv_obj_set_pos(o2_label,1300-320*sp,65);

    return tmps_layout;
}

static void slider_event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_VALUE_CHANGED) {
        printf("Value: %d\n", lv_slider_get_value(obj));
        lv_obj_t * tmp = front_fan_state_ib;
        lv_obj_t * tmp_lable = front_fan_state_l;
        if(obj == back_fan_slider) {
            tmp = back_fan_state_ib;
            tmp_lable = back_fan_state_l;
        }

        on_fan_value_changed(tmp,tmp_lable,lv_slider_get_value(obj));
    }
}

static lv_obj_t * control_fans_create(lv_obj_t * parent)
{
    lv_obj_t * fans_layout = lv_obj_create(parent,NULL);
    lv_obj_set_size(fans_layout,1620-400*sp,112);
    lv_obj_add_style(fans_layout, LV_CONT_PART_MAIN, &style_obj);

    //    lv_obj_t * front_label = lv_label_create(fans_layout,NULL);
    //    lv_obj_set_pos(front_label,180,10);
    //    lv_label_set_text(front_label,"Front");

    // front_ac_switch_ib = lv_imgbtn_create(fans_layout,NULL);
    // //lv_obj_set_event_cb(front_ac_switch_ib, imgbtn_event_handler);
    // lv_obj_set_pos(front_ac_switch_ib,220,0);
    // lv_imgbtn_set_src(front_ac_switch_ib,LV_STATE_DEFAULT, PNG_RES_PATH("switch_normal"));
    // lv_imgbtn_set_src(front_ac_switch_ib,LV_BTN_STATE_PRESSED, PNG_RES_PATH("switch_normal"));

    front_fan_icon = lv_img_create(fans_layout,NULL);
    lv_obj_set_pos(front_fan_icon,0,40);
    lv_img_set_src(front_fan_icon, PNG_RES_PATH("fan_normal"));

    lv_anim_init(&anim_rotation_front);
    lv_anim_set_var(&anim_rotation_front,front_fan_icon);
    lv_anim_set_time(&anim_rotation_front, 2000);
    lv_anim_set_values(&anim_rotation_front, 0, 3600);
    //    lv_anim_set_playback_time(&rotation, 1000);
    lv_anim_set_exec_cb(&anim_rotation_front,(lv_anim_exec_xcb_t)rotation_animation);
    lv_anim_set_repeat_count(&anim_rotation_front, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&anim_rotation_front);

    front_fan_state_ib = lv_img_create(fans_layout,NULL);
    lv_obj_set_pos(front_fan_state_ib,70,50);
    lv_img_set_src(front_fan_state_ib, PNG_RES_PATH("fan0"));

    front_fan_slider = lv_slider_create(front_fan_state_ib,NULL);
    lv_obj_add_style(front_fan_slider,LV_SLIDER_PART_BG,&style_slider);
    lv_obj_add_style(front_fan_slider,LV_SLIDER_PART_INDIC,&style_slider);
    lv_obj_add_style(front_fan_slider,LV_SLIDER_PART_KNOB,&style_slider);
    lv_obj_set_event_cb(front_fan_slider, slider_event_handler);
    lv_slider_set_range(front_fan_slider,0,7);
    lv_obj_set_pos(front_fan_slider,0,0);
    lv_obj_set_size(front_fan_slider,342,40);
    lv_slider_set_value(front_fan_slider,front_fan_state,true);

    front_fan_state_l = lv_label_create(fans_layout,NULL);
    lv_obj_add_style(front_fan_state_l,LV_CONT_PART_MAIN,&style_btn);
    lv_obj_set_pos(front_fan_state_l,420,50);
    lv_label_set_text(front_fan_state_l,"0");

    //    lv_obj_t * back_label = lv_label_create(fans_layout,NULL);
    //    lv_obj_set_pos(back_label,1100,10);
    //    lv_label_set_text(back_label,"Back");

    // back_ac_switch_ib = lv_imgbtn_create(fans_layout,NULL);
    // lv_obj_set_event_cb(back_ac_switch_ib, imgbtn_event_handler);
    // lv_obj_set_pos(back_ac_switch_ib,1370-400*sp,0);
    // lv_imgbtn_set_src(back_ac_switch_ib,LV_STATE_DEFAULT, PNG_RES_PATH("switch_normal"));
    // lv_imgbtn_set_src(back_ac_switch_ib,LV_BTN_STATE_PRESSED, PNG_RES_PATH("switch_normal"));

    back_fan_icon = lv_img_create(fans_layout,NULL);
    lv_obj_set_pos(back_fan_icon,1150-400*sp,40);
    lv_img_set_src(back_fan_icon, PNG_RES_PATH("fan_normal"));

    lv_anim_init(&anim_rotation_back);
    lv_anim_set_var(&anim_rotation_back,back_fan_icon);
    lv_anim_set_time(&anim_rotation_back, 2000);
    lv_anim_set_values(&anim_rotation_back, 0, 3600);
    //    lv_anim_set_playback_time(&rotation, 1000);
    lv_anim_set_exec_cb(&anim_rotation_back,(lv_anim_exec_xcb_t)rotation_animation);
    lv_anim_set_repeat_count(&anim_rotation_back, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&anim_rotation_back);

    back_fan_state_ib = lv_img_create(fans_layout,NULL);
    lv_obj_set_pos(back_fan_state_ib,1219-400*sp,50);
    lv_img_set_src(back_fan_state_ib, PNG_RES_PATH("fan0"));

    back_fan_slider = lv_slider_create(back_fan_state_ib,NULL);
    lv_obj_add_style(back_fan_slider,LV_SLIDER_PART_BG,&style_slider);
    lv_obj_add_style(back_fan_slider,LV_SLIDER_PART_INDIC,&style_slider);
    lv_obj_add_style(back_fan_slider,LV_SLIDER_PART_KNOB,&style_slider);
    lv_obj_set_event_cb(back_fan_slider, slider_event_handler);
    lv_obj_set_pos(back_fan_slider,0,0);
    lv_obj_set_size(back_fan_slider,342,40);
    lv_slider_set_range(back_fan_slider,0,7);

    lv_obj_add_style(back_fan_slider,LV_CONT_PART_MAIN,&style_slider);
    lv_obj_set_event_cb(back_fan_slider, slider_event_handler);
    lv_slider_set_value(back_fan_slider,back_fan_state,true);

    back_fan_state_l = lv_label_create(fans_layout,NULL);
    lv_obj_set_pos(back_fan_state_l,1580-400*sp,50);
    lv_obj_add_style(back_fan_state_l,LV_CONT_PART_MAIN,&style_btn);
    lv_label_set_text(back_fan_state_l,"0");
    return fans_layout;
}

static void on_fan_value_changed(lv_obj_t * img, lv_obj_t * label, int16_t value)
{
    if((img == front_fan_state_ib)&&(front_off)) {
        lv_img_set_src(img, PNG_RES_PATH("fan0"));
        return;
    }else if((img == back_fan_state_ib)&&(back_off)) {
        lv_img_set_src(img, PNG_RES_PATH("fan0"));
        return;
    }
    value = value+1;
    switch (value) {
    case 0:
        lv_img_set_src(img, PNG_RES_PATH("fan0"));
        lv_label_set_text(label,"0");
        break;
    case 1:
        lv_img_set_src(img, PNG_RES_PATH("fan1"));
        lv_label_set_text(label,"1");
        break;
    case 2:
        lv_img_set_src(img, PNG_RES_PATH("fan2"));
        lv_label_set_text(label,"2");
        break;
    case 3:
        lv_img_set_src(img, PNG_RES_PATH("fan3"));
        lv_label_set_text(label,"3");
        break;
    case 4:
        lv_img_set_src(img, PNG_RES_PATH("fan4"));
        lv_label_set_text(label,"4");
        break;
    case 5:
        lv_img_set_src(img, PNG_RES_PATH("fan5"));
        lv_label_set_text(label,"5");
        break;
    case 6:
        lv_img_set_src(img, PNG_RES_PATH("fan6"));
        lv_label_set_text(label,"6");
        break;
    case 7:
        lv_img_set_src(img, PNG_RES_PATH("fan7"));
        lv_label_set_text(label,"7");
        break;
    }
}

static lv_obj_t * control_btns_create(lv_obj_t * parent)
{
    lv_obj_t * btns_layout = lv_obj_create(parent,NULL);
    lv_obj_set_size(btns_layout,1720-516*sp,112);
    lv_obj_add_style(btns_layout, LV_CONT_PART_MAIN, &style_obj);

    all_ac_btn = lv_imgbtn_create(btns_layout,NULL);

    lv_obj_set_event_cb(all_ac_btn, imgbtn_event_handler);

    lv_imgbtn_set_src(all_ac_btn,LV_STATE_DEFAULT, PNG_RES_PATH("ac_normal"));
    lv_imgbtn_set_src(all_ac_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("ac_normal_click"));
    lv_imgbtn_set_src(all_ac_btn,LV_BTN_STATE_CHECKED_PRESSED, PNG_RES_PATH("ac_select_click"));
    lv_imgbtn_set_src(all_ac_btn,LV_BTN_STATE_CHECKED_RELEASED, PNG_RES_PATH("ac_select"));
    lv_obj_set_pos(all_ac_btn,54-54*sp,0);

    lv_obj_t* front2_label = lv_label_create(btns_layout, NULL);
    lv_label_set_text(front2_label,"Front");
    lv_obj_set_pos(front2_label,85-54*sp,20);

    behind_ac_btn = lv_imgbtn_create(btns_layout, NULL);
    lv_obj_set_event_cb(behind_ac_btn, imgbtn_event_handler);
    lv_imgbtn_set_src(behind_ac_btn,LV_STATE_DEFAULT, PNG_RES_PATH("ac_normal"));
    lv_imgbtn_set_src(behind_ac_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("ac_normal_click"));
    lv_obj_set_pos(behind_ac_btn,304-130*sp,0);

    lv_obj_t* back2_label = lv_label_create(btns_layout, NULL);
    lv_label_set_text(back2_label,"Back");
    lv_obj_set_pos(back2_label,334-130*sp,20);


    sync_btn = lv_imgbtn_create(btns_layout, NULL);
    lv_obj_set_event_cb(sync_btn, imgbtn_event_handler);
    lv_imgbtn_set_src(sync_btn,LV_STATE_DEFAULT, PNG_RES_PATH("sync_normal"));
    lv_imgbtn_set_src(sync_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("sync_normal_click"));
    lv_obj_set_pos(sync_btn,554-206*sp,0);

    auto_btn = lv_imgbtn_create(btns_layout, NULL);
    lv_obj_set_event_cb(auto_btn, imgbtn_event_handler);
    lv_imgbtn_set_src(auto_btn,LV_STATE_DEFAULT, PNG_RES_PATH("auto_normal"));
    lv_imgbtn_set_src(auto_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("auto_normal_click"));
    lv_obj_set_pos(auto_btn,804-292*sp,0);

    loop_btn = lv_imgbtn_create(btns_layout, NULL);
    lv_obj_set_event_cb(loop_btn, imgbtn_event_handler);
    lv_imgbtn_set_src(loop_btn,LV_STATE_DEFAULT, PNG_RES_PATH("inner_circulate_select"));
    lv_imgbtn_set_src(loop_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("inner_circulate_select_click"));
    lv_obj_set_pos(loop_btn,1054-358*sp,0);

    front_defog_btn = lv_imgbtn_create(btns_layout, NULL);
    lv_obj_set_event_cb(front_defog_btn, imgbtn_event_handler);
    lv_imgbtn_set_src(front_defog_btn,LV_STATE_DEFAULT, PNG_RES_PATH("front_defrost_normal"));
    lv_imgbtn_set_src(front_defog_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("front_defrost_normal_click"));
    lv_obj_set_pos(front_defog_btn,1304-434*sp,0);

    behind_defog_btn = lv_imgbtn_create(btns_layout, NULL);
    lv_obj_set_event_cb(behind_defog_btn, imgbtn_event_handler);
    lv_imgbtn_set_src(behind_defog_btn,LV_STATE_DEFAULT, PNG_RES_PATH("back_defrost_normal"));
    lv_imgbtn_set_src(behind_defog_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("back_defrost_normal_click"));
    lv_obj_set_pos(behind_defog_btn,1554-510*sp,0);
    return btns_layout;
}

static void imgbtn_event_handler(lv_obj_t *obj, lv_event_t e)
{
    if(e == LV_EVENT_CLICKED) {
        if(obj == all_ac_btn) {
            printf("--imgbtn_event_handler---click all_ac_btn-ac_select=%d\n",all_ac_checked);
            if(all_ac_checked) {
                all_ac_checked = false;
                cluster_should_show_fps = false;
                lv_imgbtn_set_src(all_ac_btn,LV_STATE_DEFAULT, PNG_RES_PATH("ac_normal"));
                lv_imgbtn_set_src(all_ac_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("ac_normal_click"));
            }else {
                all_ac_checked = true;
                cluster_should_show_fps = true;
                lv_imgbtn_set_src(all_ac_btn,LV_STATE_DEFAULT, PNG_RES_PATH("ac_select"));
                lv_imgbtn_set_src(all_ac_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("ac_select_click"));
            }

        }else if(obj == behind_ac_btn){
            printf("--imgbtn_event_handler---click behind_ac_btn -\n");

            if(behind_ac_checked) {
                behind_ac_checked = false;
                lv_imgbtn_set_src(behind_ac_btn,LV_STATE_DEFAULT, PNG_RES_PATH("ac_normal"));
                lv_imgbtn_set_src(behind_ac_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("ac_normal_click"));
            }else {
                behind_ac_checked = true;
                lv_imgbtn_set_src(behind_ac_btn,LV_STATE_DEFAULT, PNG_RES_PATH("ac_select"));
                lv_imgbtn_set_src(behind_ac_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("ac_select_click"));
            }

        }else if(obj == sync_btn) {
            if(sync_checked) {
                sync_checked = false;
                lv_imgbtn_set_src(sync_btn,LV_STATE_DEFAULT, PNG_RES_PATH("sync_normal"));
                lv_imgbtn_set_src(sync_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("sync_normal_click"));
            }else {
                sync_checked = true;
                lv_imgbtn_set_src(sync_btn,LV_STATE_DEFAULT, PNG_RES_PATH("sync_select"));
                lv_imgbtn_set_src(sync_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("sync_select_click"));
            }
        }else if(obj == auto_btn) {
            if(auto_checked) {
                auto_checked = false;
                lv_imgbtn_set_src(auto_btn,LV_STATE_DEFAULT, PNG_RES_PATH("auto_normal"));
                lv_imgbtn_set_src(auto_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("auto_normal_click"));
            }else {
                auto_checked = true;
                lv_imgbtn_set_src(auto_btn,LV_STATE_DEFAULT, PNG_RES_PATH("auto_select"));
                lv_imgbtn_set_src(auto_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("auto_select_click"));
            }
        }else if(obj == loop_btn) {
            if(is_inner_loop) {
                is_inner_loop = false;
                lv_imgbtn_set_src(loop_btn,LV_STATE_DEFAULT, PNG_RES_PATH("output_circulate_select"));
                lv_imgbtn_set_src(loop_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("output_circulate_select_click"));
            }else {
                is_inner_loop = true;
                lv_imgbtn_set_src(loop_btn,LV_STATE_DEFAULT, PNG_RES_PATH("inner_circulate_select"));
                lv_imgbtn_set_src(loop_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("inner_circulate_select_click"));
            }
        }else if(obj == front_defog_btn) {
            if(front_defog_checked) {
                front_defog_checked = false;
                lv_imgbtn_set_src(front_defog_btn,LV_STATE_DEFAULT, PNG_RES_PATH("front_defrost_normal"));
                lv_imgbtn_set_src(front_defog_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("front_defrost_normal_click"));
            }else {
                front_defog_checked = true;
                lv_imgbtn_set_src(front_defog_btn,LV_STATE_DEFAULT, PNG_RES_PATH("front_defrost_select"));
                lv_imgbtn_set_src(front_defog_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("front_defrost_select_click"));
            }
        }else if(obj == behind_defog_btn) {
            if(behind_defog_checked) {
                behind_defog_checked = false;
                lv_imgbtn_set_src(behind_defog_btn,LV_STATE_DEFAULT, PNG_RES_PATH("back_defrost_normal"));
                lv_imgbtn_set_src(behind_defog_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("back_defrost_normal_click"));
            }else {
                behind_defog_checked = true;
                lv_imgbtn_set_src(behind_defog_btn,LV_STATE_DEFAULT, PNG_RES_PATH("back_defrost_select"));
                lv_imgbtn_set_src(behind_defog_btn,LV_BTN_STATE_PRESSED, PNG_RES_PATH("back_defrost_select_click"));
            }
        }else if(obj == front_ac_switch_ib) {
            if(front_off) {
                lv_imgbtn_set_src(front_ac_switch_ib,LV_STATE_DEFAULT, PNG_RES_PATH("switch_select"));
                lv_img_set_src(front_fan_icon, PNG_RES_PATH("fan_select"));
                lv_anim_start(&anim_rotation_front);
            }else {
                lv_imgbtn_set_src(front_ac_switch_ib,LV_STATE_DEFAULT, PNG_RES_PATH("switch_normal"));
                lv_img_set_src(front_fan_icon, PNG_RES_PATH("fan_normal"));
                lv_anim_custom_del(&anim_rotation_front,NULL);
            }
            front_off = !front_off;
            on_fan_value_changed(front_fan_state_ib,front_fan_state_l,front_fan_state);
        }else if(obj == back_ac_switch_ib) {
            if(back_off) {
                lv_imgbtn_set_src(back_ac_switch_ib,LV_STATE_DEFAULT, PNG_RES_PATH("switch_select"));
                lv_img_set_src(back_fan_icon, PNG_RES_PATH("fan_select"));
                lv_anim_start(&anim_rotation_back);
            }else {
                lv_imgbtn_set_src(back_ac_switch_ib,LV_STATE_DEFAULT, PNG_RES_PATH("switch_normal"));
                lv_img_set_src(back_fan_icon, PNG_RES_PATH("fan_normal"));
                lv_anim_custom_del(&anim_rotation_back,NULL);
            }
            back_off = !back_off;
            on_fan_value_changed(back_fan_state_ib,back_fan_state_l,back_fan_state);
        }
    }
}

static void run_test(void)
{
    lv_anim_t slider;
    lv_anim_init(&slider);
    lv_anim_set_var(&slider,front_fan_slider);
    lv_anim_set_time(&slider, 8000);
    lv_anim_set_values(&slider, 0, 7);
    lv_anim_set_playback_time(&slider, 1000);
    lv_anim_set_exec_cb(&slider,(lv_anim_exec_xcb_t)fan_animation);
    lv_anim_set_repeat_count(&slider, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&slider);


    lv_anim_t roller;
    lv_anim_init(&roller);
    lv_anim_set_var(&roller,front_tmp_roller);
    lv_anim_set_time(&roller, 28000);
    lv_anim_set_values(&roller, 0, 23);
    lv_anim_set_playback_time(&roller, 11000);
    lv_anim_set_exec_cb(&roller,(lv_anim_exec_xcb_t)tmp_animation);
    lv_anim_set_repeat_count(&roller, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&roller);

    lv_anim_t roller2;
    lv_anim_init(&roller2);
    lv_anim_set_var(&roller2,behind_tmp_roller);
    lv_anim_set_time(&roller2, 28000);
    lv_anim_set_values(&roller2, 0, 23);
    lv_anim_set_playback_time(&roller2, 11000);
    lv_anim_set_exec_cb(&roller2,(lv_anim_exec_xcb_t)tmp_animation);
    lv_anim_set_repeat_count(&roller2, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&roller2);

}

static void fan_animation(lv_obj_t * slider, lv_anim_value_t value)
{
    //    char tmp_value = (char)value;
    //    lv_label_set_text(front_fan_state_l,&tmp_value);
    //    lv_label_set_text(front_fan_state_l,&tmp_value);

    on_fan_value_changed(front_fan_state_ib,front_fan_state_l,value);
    on_fan_value_changed(back_fan_state_ib,back_fan_state_l,value);
}

static void tmp_animation(lv_obj_t * roller, lv_anim_value_t value)
{
    lv_roller_set_selected(roller,value,true);
}

static void gif_animation(lv_obj_t * img, lv_anim_value_t value)
{
    switch (value) {
    case 0:
        lv_img_set_src(img, PNG_RES_PATH("gif0"));
        lv_img_set_src(center_gif_img2, PNG_RES_PATH("gif0"));
        lv_img_set_src(center_gif_img3, PNG_RES_PATH("gif0"));
        break;
    case 1:
        lv_img_set_src(img, PNG_RES_PATH("gif1"));
        lv_img_set_src(center_gif_img2, PNG_RES_PATH("gif1"));
        lv_img_set_src(center_gif_img3, PNG_RES_PATH("gif1"));
        break;
    case 2:
        lv_img_set_src(img, PNG_RES_PATH("gif2"));
        lv_img_set_src(center_gif_img2, PNG_RES_PATH("gif2"));
        lv_img_set_src(center_gif_img3, PNG_RES_PATH("gif2"));
        break;
    case 3:
        lv_img_set_src(img, PNG_RES_PATH("gif3"));
        lv_img_set_src(center_gif_img2, PNG_RES_PATH("gif3"));
        lv_img_set_src(center_gif_img3, PNG_RES_PATH("gif3"));
        break;
    case 4:
        lv_img_set_src(img, PNG_RES_PATH("gif4"));
        lv_img_set_src(center_gif_img2, PNG_RES_PATH("gif4"));
        lv_img_set_src(center_gif_img3, PNG_RES_PATH("gif4"));
        break;
    case 5:
        lv_img_set_src(img, PNG_RES_PATH("gif5"));
        lv_img_set_src(center_gif_img2, PNG_RES_PATH("gif5"));
        lv_img_set_src(center_gif_img3, PNG_RES_PATH("gif5"));
        break;
    case 6:
        lv_img_set_src(img, PNG_RES_PATH("gif6"));
        lv_img_set_src(center_gif_img2, PNG_RES_PATH("gif6"));
        lv_img_set_src(center_gif_img3, PNG_RES_PATH("gif6"));
        break;
    case 7:
        lv_img_set_src(img, PNG_RES_PATH("gif7"));
        lv_img_set_src(center_gif_img2, PNG_RES_PATH("gif7"));
        lv_img_set_src(center_gif_img3, PNG_RES_PATH("gif7"));
        break;
    case 8:
        lv_img_set_src(img, PNG_RES_PATH("gif8"));
        lv_img_set_src(center_gif_img2, PNG_RES_PATH("gif8"));
        lv_img_set_src(center_gif_img3, PNG_RES_PATH("gif8"));
        break;
    case 9:
        lv_img_set_src(img, PNG_RES_PATH("gif9"));
        lv_img_set_src(center_gif_img2, PNG_RES_PATH("gif9"));
        lv_img_set_src(center_gif_img3, PNG_RES_PATH("gif9"));
        break;
    case 10:
        lv_img_set_src(img, PNG_RES_PATH("gif10"));
        lv_img_set_src(center_gif_img2, PNG_RES_PATH("gif10"));
        lv_img_set_src(center_gif_img3, PNG_RES_PATH("gif10"));
        break;
    case 11:
        lv_img_set_src(img, PNG_RES_PATH("gif11"));
        lv_img_set_src(center_gif_img2, PNG_RES_PATH("gif11"));
        lv_img_set_src(center_gif_img3, PNG_RES_PATH("gif11"));
        break;
    case 12:
        lv_img_set_src(img, PNG_RES_PATH("gif12"));
        lv_img_set_src(center_gif_img2, PNG_RES_PATH("gif12"));
        lv_img_set_src(center_gif_img3, PNG_RES_PATH("gif12"));
        break;
    }
}

static void rotation_animation(lv_obj_t * img, lv_anim_value_t value)
{
    lv_img_set_angle(img,value);
}

static void show_bg_hw_layer(lv_obj_t * parent, const void * bg_img)
{
    lv_img_decoder_dsc_t dsc;
    lv_disp_set_bg_opa(get_display(CONTROLPANEL_DISPLAY_ID), LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(parent, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

    lv_res_t ret = lv_img_decoder_open(&dsc, bg_img, LV_COLOR_RED);
    if (ret != LV_RES_OK || dsc.img_data == NULL)
    {
        LV_LOG_WARN("open bg failed \n");
        return;
    }

    struct sdm_buffer sdm_buf = DISPLAY_BG_TEMPLATE;
    struct sdm_post_config post_data;

    sdm_buf.addr[0] = (unsigned long)dsc.img_data;

    sdm_buf.src_stride[0] = 1920*4;

    post_data.bufs             = &sdm_buf;
    post_data.n_bufs           = 1;

    sdm_post(m_sdm->handle, &post_data);

    LV_LOG_WARN("show bg  end  \n");
}
#endif