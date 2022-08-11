/**
 * @file lv_port_disp_templ.h
 *
 */


#ifndef LV_PORT_DISP_TEMPL_H
#define LV_PORT_DISP_TEMPL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "sdm_display.h"
/*********************
 *      DEFINES
 *********************/
typedef struct {
    int disp_id;
    lv_disp_t *disp;
} lv_sdm_disp_t;

struct disp_data {
	sdm_display_t *sdm;
	void* g2d;
};

lv_disp_t *get_display(int display_id);

void lvgl_init(void);
void lvgl_mainloop(void);

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/

