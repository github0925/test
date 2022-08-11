/*
* disp_path.h
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

#include "sdm_panel.h"
/* DISP MODULE */
typedef enum
{
    DISP_IF_MODULE_UNKNOWN = -1,
    DISP_IF_MODULE_LVDS,
    DISP_IF_MODULE_DSI,
    DISP_IF_MODULE_PARALLEL,
    DISP_IF_MODULE_NUM
} DISP_IF_MODULE_ENUM;

struct intf_device {
    int display_id;
    int intf_res;
    int lvds_cmb;
    int lvds_mode;
    int pixel_bpp;
    int dsi_index;
    int tcon_clk;
};

typedef struct {
    DISP_IF_MODULE_ENUM module;
    int (*init)(struct intf_device *dev);
    int (*deinit)(struct intf_device *dev);
    void (*config)(struct intf_device *dev, struct sdm_panel *panel);
    int (*reset)(struct intf_device *dev);
    int (*power_on)(struct intf_device *dev);
    int (*power_off)(struct intf_device *dev);
    int (*is_idle)(struct intf_device *dev);
    int (*is_busy)(struct intf_device *dev);
} DISP_MODULE_DRIVER;

int disp_interface_init(DISP_IF_MODULE_ENUM module,
        struct intf_device *dev);
int disp_interface_deinit(DISP_IF_MODULE_ENUM module,
        struct intf_device *dev);
int disp_interface_reset(DISP_IF_MODULE_ENUM module,
        struct intf_device *dev);
int disp_interface_config(DISP_IF_MODULE_ENUM module,
        struct intf_device *dev, struct sdm_panel *panel);
int disp_interface_power_on(DISP_IF_MODULE_ENUM module,
        struct intf_device *dev);
int disp_interface_power_off(DISP_IF_MODULE_ENUM module,
        struct intf_device *dev);
#endif
