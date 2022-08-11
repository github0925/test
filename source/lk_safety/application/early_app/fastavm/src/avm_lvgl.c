
#include <disp_data_type.h>

#include "lvgl_gui.h"
#include <g2dlite_api.h>
#include <macros.h>
#include "res_loader.h"
#include "early_app_cfg.h"
#include "avm_fastavm_config.h"

#ifdef SUPPORT_INPUT_SVC
#include "avm_touch.h"
#endif
#include "avm_lvgl.h"

#define CARIMG_PATH    "early_app/fastavm/car.bin"

event_t avm_get_finevent(void);

bool lvgl_stop = false;
lv_img_dsc_t g_cardsc;
lv_obj_t * parent_scr = NULL;

/*0:F; 1:B; 2:L; 3:R; 4:Close*/
const char* clabel[] = {"Front","Right","Back","Left","Close"};
uint32_t idBtn = 0;

static lv_style_t style_btn;
static lv_style_t style_label;

static lv_img_dsc_t _getCarImg(void)
{
    //1. get car from res
    int carimg_size = ROUNDUP(res_size(CARIMG_PATH),32);
    void* pcarimgdata = memalign(32,carimg_size);
    if(NULL == pcarimgdata){
        USDBG("ERROR:pcarimgdata is NULL \r\n");
    }
    res_load(CARIMG_PATH,pcarimgdata,carimg_size,0);

    //2. create car object
    lv_img_dsc_t carimg_dsc;
    carimg_dsc.header = *((lv_img_header_t*)pcarimgdata);
    carimg_dsc.data = (uint8_t*)pcarimgdata+4;
    carimg_dsc.data_size = carimg_dsc.header.w*carimg_dsc.header.h*LV_IMG_PX_SIZE_ALPHA_BYTE;
    return carimg_dsc;
}
void setCarImg(void)
{
    g_cardsc = _getCarImg();
}

void getScrSize(lv_obj_t * scr,int16_t* width, int16_t* height)
{
    *width = scr->coords.x2 - scr->coords.x1 + 1;
    *height = scr->coords.y2 - scr->coords.y1 + 1;
}

void setImgPosition(lv_obj_t* obj,int16_t width,int16_t height,int16_t xpos)
{
    lv_img_set_src(obj,&g_cardsc);
    lv_obj_set_pos(obj, (width-g_cardsc.header.w)/2 + xpos, (height-g_cardsc.header.h)/2);
    lv_obj_set_size(obj, g_cardsc.header.w, g_cardsc.header.h);
}

void lvglkickmainloop(void)
{
    while(!lvgl_stop)
    {
        lv_task_handler();
        thread_sleep(50);
    }
}

void initG2dInput(struct g2dlite_input* input,int16_t width, uint16_t height)
{
    input->layer_num = 2;
    input->layer[0].layer = 0;
    input->layer[0].layer_en = 1;
    input->layer[0].fmt = COLOR_YUYV;
    input->layer[0].zorder = 0;
    input->layer[0].src.x = 0;
    input->layer[0].src.y = MAX(0,(height-CAMERA_HEIGHT)/2);
    input->layer[0].src.w = MIN(CAMERA_WIDTH,(width-AVM_WIDTH));
    input->layer[0].src.h = MIN(height,CAMERA_HEIGHT);
    input->layer[0].src_stride[0] = CAMERA_WIDTH * 2;
    input->layer[0].dst.x = AVM_WIDTH+MAX(0,(width-AVM_WIDTH-CAMERA_WIDTH)/2);
    input->layer[0].dst.y = MAX(0,(height-CAMERA_HEIGHT)/2);
    input->layer[0].dst.w = MIN(CAMERA_WIDTH,width-AVM_WIDTH);
    input->layer[0].dst.h = MIN(height,CAMERA_HEIGHT);
    input->layer[0].ckey.en = 0;
    input->layer[0].blend = BLEND_PIXEL_NONE;
    input->layer[0].alpha = 0xFF;

    input->layer[1].layer = 1;
    input->layer[1].layer_en = 1;
    input->layer[1].fmt = COLOR_BGR888;
    input->layer[1].zorder = 1;
    input->layer[1].src.x = 0;
    input->layer[1].src.y = MAX(0,(height-AVM_HEIGHT)/2);
    input->layer[1].src.w = AVM_WIDTH;
    input->layer[1].src.h = MIN(height,AVM_HEIGHT);
    input->layer[1].src_stride[0] = AVM_WIDTH * 3;
    input->layer[1].dst.x = MAX(0,(width-AVM_WIDTH-CAMERA_WIDTH)/2);;
    input->layer[1].dst.y = MAX(0,(height-AVM_HEIGHT)/2);
    input->layer[1].dst.w = AVM_WIDTH;
    input->layer[1].dst.h = MIN(height,AVM_HEIGHT);
    input->layer[1].ckey.en = 0;
    input->layer[1].blend = BLEND_PIXEL_NONE;
    input->layer[1].alpha = 0xFF;

    input->output.width = width;
    input->output.height = height;
    input->output.fmt = COLOR_RGB888;
    input->output.stride[0] = width* 3;
    input->output.rotation = 0;

}

void SetAvmFlag(uint8_t bavm)
{
#ifdef SUPPORT_INPUT_SVC
    avm_enable_flag = bavm;
#endif
}

void _set_style(void)
{
    lv_style_init(&style_btn);
    lv_style_set_radius(&style_btn,LV_STATE_DEFAULT,10);
    lv_style_set_bg_opa(&style_btn,LV_STATE_DEFAULT,LV_OPA_COVER);
    lv_style_set_bg_color(&style_btn,LV_STATE_DEFAULT,LV_COLOR_SILVER);
    //lv_style_set_bg_grad_color(&style_btn,LV_STATE_DEFAULT,LV_COLOR_GRAY);
    lv_style_set_bg_grad_color(&style_btn,LV_STATE_DEFAULT,LV_COLOR_GREEN);
    lv_style_set_bg_grad_dir(&style_btn,LV_STATE_DEFAULT,LV_GRAD_DIR_VER);

    lv_style_set_border_color(&style_btn,LV_STATE_DEFAULT,LV_COLOR_WHITE);
    lv_style_set_border_opa(&style_btn,LV_STATE_DEFAULT,LV_OPA_70);
    lv_style_set_border_width(&style_btn,LV_STATE_DEFAULT,5);

    lv_style_init(&style_label);
    lv_style_set_text_font(&style_label,LV_STATE_DEFAULT,&lv_font_montserrat_48);
}

static void _createBtn(lv_obj_avm* avmBtn)
{
    lv_obj_t* label;
    lv_obj_t* parent = avmBtn->obj;
    lv_obj_t* btn = lv_btn_create(parent,NULL);
    lv_obj_add_style(btn,LV_BTN_PART_MAIN,&style_btn);

    lv_obj_set_pos(btn,avmBtn->refPos.x,avmBtn->refPos.y);
    lv_obj_set_size(btn,avmBtn->width,avmBtn->height);
    lv_obj_set_event_cb(btn,avmBtn->event_cb);

    label = lv_label_create(btn,NULL);
    lv_obj_add_style(label,LV_CONT_PART_MAIN, &style_label);
    lv_label_set_text(label,avmBtn->label);
}

static void _avm_front_cb(lv_obj_t* obj, lv_event_t event)
{
    if(LV_EVENT_PRESSED != event)
        return;
    idBtn = 0;
}
static void _avm_back_cb(lv_obj_t* obj, lv_event_t event)
{
    if(LV_EVENT_PRESSED != event)
        return;
    idBtn = 1;
}
static void _avm_left_cb(lv_obj_t* obj, lv_event_t event)
{
    if(LV_EVENT_PRESSED != event)
        return;
    idBtn = 2;
}
static void _avm_right_cb(lv_obj_t* obj, lv_event_t event)
{
    if(LV_EVENT_PRESSED != event)
        return;
    idBtn = 3;
}
static void _avm_close_cb(lv_obj_t * obj, lv_event_t event)
{

    if(LV_EVENT_PRESSED != event)
        return;
    lv_obj_set_event_cb(obj,NULL);
    //del lvgl display
    if(parent_scr)
        lv_obj_del(parent_scr);
    event_t avmfin = avm_get_finevent();

    event_signal( &avmfin,false);
}

uint32_t getCameraId(void)
{
    return idBtn;
}

void createSwitch(lv_obj_avm* avmobj,uint32_t num,lv_coord_t height,lv_coord_t xpos)
{
    memset(avmobj,0, sizeof(lv_obj_avm));
    parent_scr = lv_disp_get_scr_act(get_display(INFOTAINMENT));
    _set_style();

    uint32_t width = IMG_WIDTH/ num;
    lv_event_cb_t eHandler[] = {_avm_front_cb,_avm_right_cb,_avm_back_cb,_avm_left_cb,_avm_close_cb};

    for(int i = 0; i<num; i++)
    {
        avmobj->obj = parent_scr;
        avmobj->refPos.x = xpos + i * width;
        avmobj->refPos.y = IMG_HEIGHT-height;
        avmobj->width = width;
        avmobj->height = height;
        avmobj->label = (char*)clabel[i];
        avmobj->event_cb = eHandler[i];
        _createBtn(avmobj);
        avmobj++;
    }
}

void stopLvgl(void)
{
    lvgl_stop = true;
}
