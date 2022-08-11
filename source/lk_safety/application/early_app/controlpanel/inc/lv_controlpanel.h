/**
 * @file lv_controlpanel.h
 *
 */

#ifndef LV_CONTROLPANEL_H
#define LV_CONTROLPANEL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <sdm_display.h>
#include <disp_data_type.h>
/*********************
 *      DEFINES
 *********************/
#define DISPLAY_BG_TEMPLATE { \
    0,/*layer*/\
    0,/*layer_dirty*/\
    1,/*layer_en*/\
    COLOR_ARGB8888,/*fmt*/\
    {0,0,1920,720},/*x,y,w,h src */ \
    {0x0,0x0,0x0,0x0},/*y,u,v,a*/ \
    {0,0,0,0},/*stride*/ \
    {0,0,1920,720},/*start*/ \
    {0,0,1920,720},/*dst*/ \
    0,/*ckey_en*/\
    0,/*ckey*/\
    1,/*alpha_en*/\
    0xff,/*alpha*/\
    0,/*z-order*/ \
    0/*security*/\
}

/**********************
 *      TYPEDEFS
 **********************/
#define LV_PNG_USE_LV_FILESYSTEM 1

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_controlpanel(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_CONTROLPANEL_H*/
