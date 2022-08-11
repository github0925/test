/**
 * @file lv_demo_cluster.h
 *
 */

#ifndef LV_DEMO_CLUSTER_H
#define LV_DEMO_CLUSTER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <sdm_display.h>
#include <disp_data_type.h>
#include <cluster_ui_parameter.h>

/*********************
 *      DEFINES
 *********************/
#define DISPLAY_NAV_TEMPLATE { \
    0,/*layer*/\
    0,/*layer_dirty*/\
    1,/*layer_en*/\
    COLOR_ABGR8888,/*fmt*/\
    {0,0,1120,720},/*x,y,w,h src */ \
    {0x0,0x0,0x0,0x0},/*y,u,v,a*/ \
    {4480,0,0,0},/*stride*/ \
    {0,0,1120,720},/*start*/ \
    {400,0,1120,720},/*dst*/ \
    0,/*ckey_en*/\
    0,/*ckey*/\
    1,/*alpha_en*/\
    0xff,/*alpha*/\
    0,/*z-order*/ \
    0/*security*/\
}

#define DISPLAY_BG_TEMPLATE { \
    0,/*layer*/\
    0,/*layer_dirty*/\
    1,/*layer_en*/\
    COLOR_ARGB8888,/*fmt*/\
    {0,0,1920,720},/*x,y,w,h src */ \
    {0x0,0x0,0x0,0x0},/*y,u,v,a*/ \
    {7680,0,0,0},/*stride*/ \
    {0,0,1920,720},/*start*/ \
    {0,0,1920,720},/*dst*/ \
    0,/*ckey_en*/\
    0,/*ckey*/\
    1,/*alpha_en*/\
    0xff,/*alpha*/\
    0,/*z-order*/ \
    0/*security*/\
}

//0:fg, 1:bg
#define DISPLAY_CLUSTER_TEMPLATE {\
{\
    0,/*layer*/\
    0,/*layer_dirty*/\
    1,/*layer_en*/\
    COLOR_ARGB8888,/*fmt*/\
    {0,0,1920,720},/*x,y,w,h src */ \
    {0x0,0x0,0x0,0x0},/*y,u,v,a*/ \
    {7680,0,0,0},/*stride*/ \
    {0,0,1920,720},/*start*/ \
    {0,0,1920,720},/*dst*/ \
    0,/*ckey_en*/\
    0,/*ckey*/\
    1,/*alpha_en*/\
    0xff,/*alpha*/\
    0,/*z-order*/ \
    0/*security*/\
},\
{\
    1,/*layer*/\
    0,/*layer_dirty*/\
    1,/*layer_en*/\
    COLOR_ARGB8888,/*fmt*/\
    {0,0,1920,720},/*x,y,w,h src */ \
    {0x0,0x0,0x0,0x0},/*y,u,v,a*/ \
    {7680,0,0,0},/*stride*/ \
    {0,0,1920,720},/*start*/ \
    {0,0,1920,720},/*dst*/ \
    0,/*ckey_en*/\
    0,/*ckey*/\
    0,/*alpha_en*/\
    0x0,/*alpha*/\
    1,/*z-order*/ \
    0/*security*/\
}\
}

/**********************
 *      TYPEDEFS
 **********************/
#define LV_PNG_USE_LV_FILESYSTEM 1
/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_cluster_init(void* token);
void lv_cluster_start(void* token);
void lv_cluster_stop(void* token);
void lv_cluster_deinit(void* token);
void lv_demo_cluster_property_update(bool remote, uint16_t val);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_CLUSTER_H*/
