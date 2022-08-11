#include <app.h>
#include <lk_wrapper.h>
#include "sdm_display.h"
#include <disp_data_type.h>
//#include "data_structure_def.h"
#include "container.h"

#include "dcf.h"
#include "early_app_common.h"

#include "lv_demo_6btn.h"

#include "lvgl_gui.h"

#include <debug.h>
#include <stdio.h>
#include <stdlib.h>

#include "lv_examples.h"


void tlvgl_entry(token_handle_t token)
{
    lvgl_init();

    lv_disp_t * disp = get_display(CONTROLPANEL);

    lv_obj_t* scr = lv_disp_get_scr_act(disp);
    lv_disp_set_bg_opa(disp,LV_OPA_TRANSP);
    lv_obj_set_style_local_bg_opa(scr,LV_OBJ_PART_MAIN,LV_STATE_DEFAULT,LV_OPA_TRANSP);
    lv_disp_set_default(disp);

    lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    token_handle_t tlvgl_token = token_create("TLVGL", NULL, 400, NULL,NULL);

    token_serialization(1, tlvgl_token);
    thread_t * tlvgl = thread_create("tlvgl", (thread_start_routine) lv_demo_6btn, tlvgl_token, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE*6);
    thread_detach_and_resume(tlvgl);

    token_destroy(tlvgl_token);
    lvgl_mainloop();
}