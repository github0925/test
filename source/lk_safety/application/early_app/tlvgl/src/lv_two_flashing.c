#include "stb_decoder.h"
#include <lk_wrapper.h>
#include "lv_examples.h"
#include "lvgl_gui.h"

bool bHidden = false ;
volatile uint8_t iCount = 0;

#define TFINTERVAL  800
#define FLASHING_Y  130
#define LFLASHING_X 100
#define RFLASHING_X 540

lv_obj_t * imglbg = NULL;
lv_obj_t * imgrbg = NULL;
lv_obj_t * tmplbg = NULL;
lv_obj_t*  tmprbg = NULL;

LV_IMG_DECLARE(LeftTurn);
LV_IMG_DECLARE(rightTurn);

extern lv_img_dsc_t decoderdimg_dsc[PNGNUM];
extern uint8_t _getIconId(uint32_t val);
static lv_img_dsc_t ops_dsc;

static void  _getDsc(void)
{
    ops_dsc = decoderdimg_dsc[_getIconId(0x206)];
    ops_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;

}

static enum handler_return tf_tmr_cb(struct timer* t,lk_time_t now, void* arg)
{
    iCount ++;
    if (iCount < 25){
        bHidden = !bHidden;
    }
    else{
         bHidden = true;
    }
    lv_obj_set_hidden(imglbg,bHidden);
    lv_obj_set_hidden(imgrbg,bHidden);
    return 0;
}

static lv_obj_t* _createImg(const void* scr,lv_coord_t posx, lv_coord_t posy)
{
    lv_obj_t *tmpbg = lv_img_create(lv_scr_act(), NULL);

    lv_img_set_src(tmpbg, scr);

    lv_obj_set_pos(tmpbg, posx,posy);

    return tmpbg;
}

void tflashing(void)
{
    _getDsc();

    /*Create Img Object
    *tmplbg,tmprbg: opa background for hidden left/right*
    *imglbg,imgrbg: load flashing left/right*
    */
    tmplbg = _createImg(&ops_dsc,LFLASHING_X,FLASHING_Y);
    imglbg = _createImg(&LeftTurn,LFLASHING_X,FLASHING_Y);
    tmprbg = _createImg(&ops_dsc,RFLASHING_X,FLASHING_Y);
    imgrbg = _createImg(&rightTurn,RFLASHING_X,FLASHING_Y);

    timer_t tf_timer;
    timer_initialize(&tf_timer);
    timer_set_periodic(&tf_timer, TFINTERVAL, tf_tmr_cb,NULL);
    while(1){
        if(iCount >= 25){
            timer_cancel(&tf_timer);
            break;
        }
    }
}

