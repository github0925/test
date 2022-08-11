/*
* disp_drv.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 06/28/2019 BI create this file
*/
#ifndef __DISP_DRV_H__
#define __DISP_DRV_H__

#include <disp_hal.h>

extern int disp_init(display_handle *handle);
extern int disp_uninit(display_handle *handle);
extern int disp_clear_layers(display_handle *handle, u8 mask, u8 z_order);
extern int disp_post_config(display_handle *handle,
    struct sdm_post_config *bufs);
extern bool disp_panel_connect(display_handle *handle, int sub_id,
            struct sdm_panel *panel);
extern int disp_panel_disconnect(display_handle *handle, int sub_id,
            struct sdm_panel *panel);
extern int disp_pq_config(display_handle *handle, struct sdm_pq_params *pq);
#endif
