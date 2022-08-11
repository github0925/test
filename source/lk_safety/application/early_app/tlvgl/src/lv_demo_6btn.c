/**
 * @file lv_demo_6btn.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_examples.h"
#include "lv_demo_6btn.h"
#include "container.h"
#include <stdlib.h>
#include "stb_decoder.h"
#include "heap.h"

#include <lk_wrapper.h>
#include "radio.h"

#define LV_USE_DEMO_6BTN 1

#define IMG_W 212
#define OFFSET_X(i) (640 + (i) * IMG_W)

#ifdef S3SIN1_1920X1080
#define PANEL_X 960
#define OFFSET_Y (PANEL_X+2)
#else
#define PANEL_X 600
#define OFFSET_Y (PANEL_X+2)
#endif

#define PRESSABLE_NUM 7


extern lv_img_dsc_t decoderdimg_dsc[PNGNUM];
extern void tflashing(void);


bool bPressed[PRESSABLE_NUM];
bool bFMPressed[PRESSABLE_NUM];



lv_obj_t fmlable;
lv_obj_t* fmFavorite;

lv_img_dsc_t ic_button;
lv_img_dsc_t ic_button_ck;


void createBtns(bool bfm);

static lv_style_t style_tmp;
static lv_style_t style_fm;

static uint8_t _get_img_index(lv_obj_t * obj)
{
    lv_area_t area;
    lv_obj_t * objbk = lv_obj_get_parent(obj);
    lv_obj_get_coords(objbk, &area);

    //offset_0----hvac/fm----imgbtn0
    if (area.x1 == OFFSET_X(0))
        return 0;

    //bPressed[0]:true--show fm, false -- show hvac
    if(bPressed[0]){
        for (uint8_t i = 1; i < sizeof(bPressed) - 1; i++)
            if (area.x1 == OFFSET_X(i))
                return i ;
    }
    if (area.x1 == OFFSET_X(1) ) {
        lv_area_t arealr;
        lv_obj_get_coords(obj, &arealr);
        if (arealr.x1 < (OFFSET_X(1) + IMG_W / 2)) //left pos < mid pos
            return 1; //offset_1 ----imgbtn1----left
        else
            return 2; //offset_1 ----imgbtn2----right
    }
    //7 imgbtn, 6 offset_x
    //offset2----auto----imgbtn3,offset3----cycle----imgbtn4----------
    for (uint8_t i = 2; i < sizeof(bPressed) - 1; i++) {
        if (area.x1 == OFFSET_X(i))
            return i + 1;
    }
    return 0;
}

static uint8_t _get_pos_index(uint32_t posx)
{
    //offset_0----hvac/fm----imgbtn0
    if (posx == OFFSET_X(0))
        return 0;

    //bPressed[0]:true--show fm, false -- show hvac
    if(bPressed[0]){
        for (uint8_t i = 1; i < sizeof(bPressed) - 1; i++)
            if (posx == OFFSET_X(i))
                return i ;
    }
    else{
            if (posx == OFFSET_X(1) ) {
                if (posx < (OFFSET_X(1) + IMG_W / 2)) //left pos < mid pos
                    return 1; //offset_1 ----imgbtn1----left
            else
                return 2; //offset_1 ----imgbtn2----right
            }
    //7 imgbtn, 6 offset_x
    //offset2----auto----imgbtn3,offset3----cycle----imgbtn4----------
            for (uint8_t i = 2; i < sizeof(bPressed) - 1; i++) {
                if (posx == OFFSET_X(i))
                    return i + 1;
            }
    }
    return 0;
}

/* val : 0x120
 1: fm or not; 2: 2nd button ,play/auto , 1st is hvac/fm; 0: clicked or not
 1st button is fm=0x100, hvac = 0x000
 0x2**:common png
*/
uint8_t _getIconId(uint32_t val)
{
    char chIconName[40] = "";

    switch (val){
        case 0x0:   strcpy(chIconName,"ic-fm.");
        break;
        case 0x1:  strcpy(chIconName,"ic-hvac.");
        break;
        case 0x010: strcpy(chIconName,"ic-temperature-left.");
        break;
        case 0x011: strcpy(chIconName,"ic-temperature-left-click.");
        break;
        case 0x012: strcpy(chIconName,"ic-temperature-right.");
        break;
        case 0x013: strcpy(chIconName,"ic-temperature-right-click.");
        break;
        case 0x014: strcpy(chIconName,"ic-button-temperature-left-click.");
        break;
        case 0x015: strcpy(chIconName,"ic-button-temperature-right-click.");
        break;
        case 0x020: strcpy(chIconName,"ic-auto.");
        break;
        case 0x021: strcpy(chIconName,"ic-auto-click.");
        break;
        case 0x030: strcpy(chIconName,"ic-cycle-n.");
        break;
        case 0x031: strcpy(chIconName,"ic-cycle-n-click.");
        break;
        case 0x040: strcpy(chIconName,"ic-front-glass.");
        break;
        case 0x041: strcpy(chIconName,"ic-front-glass-click.");
        break;
        case 0x050: strcpy(chIconName,"ic-after-glass.");
        break;
        case 0x051: strcpy(chIconName,"ic-after-glass-click.");
        break;
        case 0x100:   strcpy(chIconName,"ic-hvac.");
        break;
        case 0x101:   strcpy(chIconName,"ic-fm.");
        break;
        case 0x110:   strcpy(chIconName,"ic-shang.");
        break;
        case 0x111:   strcpy(chIconName,"ic-shang-click.");
        break;
        case 0x120:   strcpy(chIconName,"ic-play.");
        break;
        case 0x121:   strcpy(chIconName,"ic-suspend.");
        break;
        case 0x130:   strcpy(chIconName,"ic-xia.");
        break;
        case 0x131:   strcpy(chIconName,"ic-xia-click.");
        break;
        case 0x150:   strcpy(chIconName,"ic-sc.");
        break;
        case 0x151:   strcpy(chIconName,"ic-sc-click.");
        break;
        case 0x200:    strcpy(chIconName,"ic-mileage.");
        break;
        case 0x201:    strcpy(chIconName,"ic-line.");
        break;
        case 0x202:    strcpy(chIconName,"ic-button.");
        break;
        case 0x203:    strcpy(chIconName,"ic-button-click.");
        break;
        case 0x204:    strcpy(chIconName,"LeftTurn.");
        break;
        case 0x205:    strcpy(chIconName,"rightTurn.");
        break;
        case 0x206:    strcpy(chIconName,"ops.");
        break;
        default:
           return 1;
    }
    uint8_t i = 0;
    for(i = 0; i < PNGNUM; i++){
        if( strstr(png_path[i],chIconName) ){
                return i;
        }

    }
    return 1;
}
static void _handleTuning(lv_obj_t* objfav,lv_obj_t* lable,uint8_t buttonid,uint8_t fmselected)
{
    uint8_t bSel = 0;
    uint8_t selected = fmselected;
    if(buttonid == 1){
         bSel = (fmselected == 0) ? (FM_NUM-1) : (fmselected-1);
    }else
    {
        bSel = (fmselected == (FM_NUM-1)) ? 0 : (fmselected+1);
    }
    fmselected = bSel;
    setsel(fmselected);

    lv_obj_add_style(lable,LV_CONT_PART_MAIN, &style_fm);
    lv_label_set_text(lable, get_lable(fmselected));
    bFMPressed[buttonid] = !bFMPressed[buttonid];
    if(get_fav(fmselected)){
        lv_img_set_src(objfav, &ic_button_ck);
    }
    else
    {
        lv_img_set_src(objfav, &ic_button);
    }
}

//callback for fm
static void img_event_cbfm(lv_obj_t * obj, lv_event_t event)
{
     lv_area_t area;
    lv_obj_get_coords(lv_scr_act(), &area);
    lv_obj_t * objbk = lv_obj_get_parent(obj);
    uint8_t i = _get_img_index(obj);
    if(i == 0)
        return;
    if ((LV_EVENT_PRESSED == event)) {
        if (bFMPressed[i] == false) {
            switch (i) {
                case 2:
                    lv_btn_set_state(obj,LV_BTN_STATE_RELEASED);
                    lv_imgbtn_set_src(obj,LV_BTN_STATE_RELEASED,&decoderdimg_dsc[_getIconId(0x121)]);
                    lv_img_set_src(objbk, &ic_button_ck);
                break;
                case 5:
                    lv_img_set_src(objbk, &ic_button_ck);
                    set_fav(!bFMPressed[i],getsel());
                break;
                case 1:
                    _handleTuning(fmFavorite,&fmlable,1,getsel());
                break;
                case 3:
                    _handleTuning(fmFavorite,&fmlable,3,getsel());
                break;
            }
        }else
        {
            if(i == 2){
                lv_btn_set_state(obj,LV_BTN_STATE_RELEASED);
                lv_imgbtn_set_src(obj,LV_BTN_STATE_RELEASED,&decoderdimg_dsc[_getIconId(0x120)]);
            }
            if(i == 5)
                set_fav(!bFMPressed[i],getsel());
            lv_img_set_src(objbk, &ic_button);
            }

        lv_obj_invalidate_area(lv_scr_act(), &area);
        bFMPressed[i] = !bFMPressed[i];
    }
    lv_obj_invalidate_area(lv_scr_act(), &area);
}

//callback for hvac
static void img_event_cb(lv_obj_t * obj, lv_event_t event)
{
    lv_area_t area;
    lv_obj_get_coords(lv_scr_act(), &area);
    lv_obj_t * objbk = lv_obj_get_parent(obj);
    uint8_t i = _get_img_index(obj);
    if ((LV_EVENT_PRESSED == event)) {
        if (bPressed[i] == false) { //hvac
            if (i == 1)
                lv_img_set_src(objbk, &decoderdimg_dsc[_getIconId(0x014)]);
            else if (i == 2)
                lv_img_set_src(objbk, &decoderdimg_dsc[_getIconId(0x015)]);
            else
                lv_img_set_src(objbk, &ic_button_ck);
        }else
            lv_img_set_src(objbk, &ic_button);

        lv_obj_invalidate_area(lv_scr_act(), &area);
        bPressed[i] = !bPressed[i];
    }
    lv_obj_invalidate_area(lv_scr_act(), &area);

}
static void img_event_cb0(lv_obj_t * obj, lv_event_t event)
{
    lv_area_t area;
    lv_obj_get_coords(lv_scr_act(), &area);
    lv_obj_t * objbk = lv_obj_get_parent(obj);
    uint8_t i = _get_img_index(obj);
    if ((LV_EVENT_PRESSED == event)) {
        if (bPressed[i] == false) {
                lv_img_set_src(objbk, &ic_button_ck);
                lv_imgbtn_set_state(obj,LV_BTN_STATE_RELEASED);
                lv_imgbtn_set_src(obj,LV_BTN_STATE_RELEASED,&decoderdimg_dsc[_getIconId(0x01)]);
        }else{
             lv_img_set_src(objbk, &ic_button);
             lv_imgbtn_set_state(obj,LV_BTN_STATE_RELEASED);
             lv_imgbtn_set_src(obj,LV_BTN_STATE_RELEASED,&decoderdimg_dsc[_getIconId(0x00)]);
        }
        lv_obj_invalidate_area(lv_scr_act(), &area);
        bPressed[0] = !bPressed[0];
        createBtns(bPressed[0]);
    }
    lv_obj_invalidate_area(lv_scr_act(), &area);

}
static void create_ic_button0(void)
{
    lv_obj_t * imgbk = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(imgbk, &ic_button);
    lv_obj_set_pos(imgbk, OFFSET_X(0), OFFSET_Y);
    lv_obj_t * img = lv_imgbtn_create(imgbk, NULL);
    lv_imgbtn_set_src(img, LV_BTN_STATE_RELEASED, &decoderdimg_dsc[_getIconId(0)]);

    lv_obj_set_event_cb(img, img_event_cb0);
    lv_obj_align(img, imgbk, LV_ALIGN_CENTER, 0, 0);

}
static lv_obj_t* _create_imgbk(lv_coord_t posx){
    lv_obj_t * imgbk = lv_img_create(lv_scr_act(), NULL);
    uint8_t i = _get_pos_index(posx);

    if(bPressed[0]){
        if(bFMPressed[i] && i != 2)
            lv_img_set_src(imgbk, &ic_button_ck);
         else
             lv_img_set_src(imgbk, &ic_button);
    }
    else{
            if(i == 1){
                if(bPressed[1])
                    lv_img_set_src(imgbk, &decoderdimg_dsc[_getIconId(0x015)]);
                else
                    lv_img_set_src(imgbk, &ic_button);
            }
            else if(i == 2){
                if(bPressed[2])
                    lv_img_set_src(imgbk, &decoderdimg_dsc[_getIconId(0x015)]);
                else
                    lv_img_set_src(imgbk, &ic_button);
            }
            else{
                if(bPressed[i])
                    lv_img_set_src(imgbk, &ic_button_ck);
                else
                    lv_img_set_src(imgbk, &ic_button);
                }
        }
    return imgbk;

}

lv_obj_t* create_ic_button(const void * source_img,const void * source_imgck, lv_coord_t posx, lv_coord_t posy)
{
    lv_obj_t * imgbk=_create_imgbk(posx);
    lv_obj_set_pos(imgbk, posx, posy);

    if (source_img != NULL) {
        lv_obj_t * img = lv_imgbtn_create(imgbk, NULL);
        lv_imgbtn_set_src(img, LV_BTN_STATE_RELEASED, source_img);
        lv_imgbtn_set_src(img, LV_BTN_STATE_PRESSED, source_imgck);
        if(bPressed[0] == false)
            lv_obj_set_event_cb(img, img_event_cb);
        else
            lv_obj_set_event_cb(img, img_event_cbfm);

        lv_obj_align(img, imgbk, LV_ALIGN_CENTER, 0, 0);

    } else {
        if(bPressed[0]){
            lv_obj_t *lable = lv_label_create(imgbk, NULL);
            lv_obj_add_style(lable,LV_CONT_PART_MAIN, &style_fm);
            fmlable = *lable;
            lv_label_set_text(lable,get_lable(getsel()));
            lv_obj_align(lable, imgbk, LV_ALIGN_CENTER, 0, 0);
        }
        else{
            lv_obj_t * imgleft = lv_imgbtn_create(imgbk, NULL);
            lv_obj_t * lable = lv_label_create(imgbk, NULL);
            lv_obj_add_style(lable,LV_CONT_PART_MAIN, &style_tmp);

            lv_obj_t * imgright = lv_imgbtn_create(imgbk, NULL);
            lv_obj_align(imgleft, imgbk, LV_ALIGN_IN_LEFT_MID, 30, -8);
            lv_imgbtn_set_src(imgleft, LV_BTN_STATE_RELEASED, &decoderdimg_dsc[_getIconId(0x010)]);
            lv_imgbtn_set_src(imgleft, LV_BTN_STATE_PRESSED, &decoderdimg_dsc[_getIconId(0x011)]);
            lv_obj_set_event_cb(imgleft, img_event_cb);
            lv_label_set_text(lable, "22");
            lv_obj_align(lable, imgbk, LV_ALIGN_CENTER, 0, 0);

            lv_imgbtn_set_src(imgright, LV_BTN_STATE_RELEASED, &decoderdimg_dsc[_getIconId(0x012)]);
            lv_imgbtn_set_src(imgright, LV_BTN_STATE_PRESSED, &decoderdimg_dsc[_getIconId(0x013)]);
            lv_obj_set_event_cb(imgright, img_event_cb);
            lv_obj_align(imgright, imgbk, LV_ALIGN_IN_RIGHT_MID, -30, 0);
        }
    }
    return imgbk;
}

void createBtns(bool bfm)
{
    uint8_t i = 1;
    uint32_t val = 0;
    if (true == bfm){
        for(i = 1; i<5; i++){
            if(i == 4 ){
                create_ic_button(NULL, NULL, OFFSET_X(i), OFFSET_Y);
                continue;
            }
            val = 0x100+ (i * 0x10);

            create_ic_button(&decoderdimg_dsc[_getIconId(val)], &decoderdimg_dsc[_getIconId(val+1)], OFFSET_X(i), OFFSET_Y);
        }
        val = 0x100+ (5 * 0x10);
        fmFavorite = create_ic_button(&decoderdimg_dsc[_getIconId(val)], &decoderdimg_dsc[_getIconId(val+1)], OFFSET_X(5), OFFSET_Y);
    }
    else{
        create_ic_button(NULL, NULL, OFFSET_X(1), OFFSET_Y);
         for(i = 2; i<6; i++){
            val = i * 0x10;
            create_ic_button(&decoderdimg_dsc[_getIconId(val)], &decoderdimg_dsc[_getIconId(val+1)], OFFSET_X(i), OFFSET_Y);
        }
    }
}

void lv_demo_6btn(token_handle_t token)
{
    stb_decoder();
    lv_style_init(&style_tmp);
    lv_style_set_text_font(&style_tmp,LV_STATE_DEFAULT,&lv_font_montserrat_48);
    lv_style_init(&style_fm);
    lv_style_set_text_font(&style_fm,LV_STATE_DEFAULT,&lv_font_montserrat_48);


    ic_button = decoderdimg_dsc[_getIconId(0x202)];
    ic_button_ck = decoderdimg_dsc[_getIconId(0x203)];
    init_fm();
    lv_obj_t * imgmile = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(imgmile, &decoderdimg_dsc[_getIconId(0x200)]);
    lv_obj_set_pos(imgmile, 0, PANEL_X);


    lv_obj_t * imgline = lv_img_create(lv_scr_act(), NULL);
    lv_img_set_src(imgline, &decoderdimg_dsc[_getIconId(0x201)]);
    lv_obj_set_pos(imgline, OFFSET_X(0), PANEL_X);

    for (uint8_t i = 0; i < 7; i++){
        bPressed[i] = false;
        bFMPressed[i] = false;
    }

    create_ic_button0();
    createBtns(false);
    tflashing();

}
