//*****************************************************************************
//
// disp_hal.h - Prototypes for the disp hal
//
// Copyright (c) 2019-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
// SDM  (semidrive display module)
//
//
//*****************************************************************************

#ifndef __DISP_HAL_H__
#define __DISP_HAL_H__
//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
// semidrive display module
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif
#include "res.h"
#include "chip_res.h"

#include "disp_common.h"
#include "sdm_panel.h"
#include <disp_link_types.h>

#define PANEL_NOT_NEED_INIT 0

enum DISPLAY_SCREEN {
    NOT_FOUND = -1,
    CLUSTER,
    INFOTAINMENT,
    ENTERTAINMENT,
    CONTROLPANEL,
    HUD,
    SCREEN_MAX
};

enum DISPLAY_TYPE {
    DISPLAY_TYPE_INVALID = -1,
    DISPLAY_TYPE_MAX = 10,
};

enum {
    CLEAR_SPIPE_LAYER = 0x1,
    CLEAR_GPIPE_LAYER = 0x2,
    CLEAR_ALL_LAYERS =  0x3
};

struct sdm_event {
    uint32_t type; //vsync, hotplug..
    uint32_t data;
    int64_t timestamp;
};

typedef int(*CALLBACK_EVENT_T)(int display_id, int64_t timestamp);
struct sdm_modeinfo {
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t vrefresh;
    struct sis_info rtos_screen;
};

typedef struct mod_res_t {
    uint32_t res_id;
    paddr_t base;
    int index;
    int irqs[4]; //reset 0
} mod_res_t;

struct sdm_domain_res
{
    int num_dp;
    mod_res_t *dp;
    int num_dc;
    mod_res_t *dc;
    int num_intf;
    mod_res_t *intf;
};

typedef struct display_instance {
    int display_id;
    const char *name;
    struct display_resource *res;
    struct sdm_domain_res *dm;
    int panel_num;
    struct sdm_panel *panels[4];
    int found_panel;
    struct sdm_modeinfo info;
    CALLBACK_EVENT_T vsync;
    CALLBACK_EVENT_T hotplug;
    bool inited;
    bool panel_post_end_done;
    bool is_need_register_irq;
} display_handle;


int hal_display_create_handle(display_handle **handle, enum DISPLAY_TYPE display);
void hal_display_release_handle(display_handle *handle);

// fill display_resource into handle res filed.
int hal_display_handle_install_resource(struct display_resource *res);
int hal_display_connect_all(void);
int hal_display_num(void);

int hal_sdm_callback_vsync_register(display_handle *handle, CALLBACK_EVENT_T func);
display_handle *hal_get_display_handle(enum DISPLAY_TYPE display);
display_handle *hal_get_display_handle_by_index(uint32_t index);

int hal_sdm_init(display_handle *handle);
int hal_sdm_deinit(display_handle *handle);

int hal_sdm_clear_layers(display_handle * handle, u8 mask, u8 zorder);
int hal_sdm_post_config(display_handle *handle, struct sdm_post_config *post);
int hal_sdm_buffer_alloc(void);
int hal_sdm_buffer_free(void);
int hal_sdm_event_callback(display_handle *handle, int type, void *callback);
int hal_sdm_panel_connect(display_handle *handle, int sub_id, struct sdm_panel *panel);
int hal_sdm_panel_disconnect(display_handle *handle, int sub_id, struct sdm_panel *panel);
void hal_sdm_set_pll_clk(display_handle *handle, int clk);
int hal_sdm_pq_config(display_handle *handle, struct sdm_pq_params *pq);
//######################

//######################


#ifdef __cplusplus
}
#endif
#endif // __DISP_HAL_H__

