#ifndef _AVM_LVGL_H
#define _AVM_LVGL_H
#include <g2dlite_api.h>
#include "lvgl_gui.h"

typedef struct _lv_obj_avm{
    lv_obj_t* obj;
    lv_point_t refPos;
    lv_coord_t width;
    lv_coord_t height;
    lv_event_cb_t event_cb;
    char* label;
}lv_obj_avm;

void getScrSize(lv_obj_t * scr,int16_t* width, int16_t* height);
void initG2dInput(struct g2dlite_input* input,int16_t width, uint16_t height);
void lvglkickmainloop(void);
void setCarImg(void);
void setImgPosition(lv_obj_t* obj,int16_t width,int16_t height,int16_t xpos);
void createSwitch(lv_obj_avm* avmobj,uint32_t num,lv_coord_t height,lv_coord_t xpos);
uint32_t getCameraId(void);
void SetAvmFlag(uint8_t bavm);
void stopLvgl(void);
#endif