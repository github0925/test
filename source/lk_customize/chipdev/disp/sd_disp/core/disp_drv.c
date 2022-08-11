/*
* disp_drv.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 11/02/2019 BI create this file
*/
#include <chip_res.h>
#include <disp_ctrl.h>
#include <disp_general_header.h>
#include <disp_hal.h>
#include <disp_interface.h>
#include <disp_drv_log.h>
#include <disp_panel.h>
#include <lib/reg.h>
#include <irq.h>
#include <platform/interrupts.h>
#include <platform.h>
#include <res.h>
#include <err.h>
#include <event.h>
extern const struct dc_operations dc_ops;
extern bool vsync_r;
struct dc_device dc_dev[MAX_DISP_NUM] = {0};
struct intf_device intf_dev[MAX_DISP_NUM] = {0};

static int get_if_type_by_res(int if_res)
{
    switch(if_res) {
    case IF_RES_MIPI_DSI1:
    case IF_RES_MIPI_DSI2:
    case IF_RES_DSI1_DSI2:
        return IF_TYPE_DSI;
    case IF_RES_LVDS1:
    case IF_RES_LVDS2:
    case IF_RES_LVDS3:
    case IF_RES_LVDS4:
    case IF_RES_LVDS1_LVDS2:
    case IF_RES_LVDS3_LVDS4:
    case IF_RES_LVDS1_4:
        return IF_TYPE_LVDS;
    case IF_RES_PARALLEL:
        return IF_TYPE_PARALLEL;
    default:
         DISPERR("not support this if res [%d]\n", if_res);
         return IF_TYPE_INVALID;
    }
}

static int get_if_module_by_panel(struct sdm_panel *panel)
{
    switch(panel->if_type) {
    case IF_TYPE_DSI:
        return DISP_IF_MODULE_DSI;
    case IF_TYPE_LVDS:
        return DISP_IF_MODULE_LVDS;
    case IF_TYPE_PARALLEL:
        return DISP_IF_MODULE_PARALLEL;
    default:
         DISPERR("not support this if type [%d]\n", panel->if_type);
         return DISP_IF_MODULE_UNKNOWN;
    }
}

static int display_if_init(struct intf_device *dev, struct sdm_panel *panel)
{
    int if_module = DISP_IF_MODULE_UNKNOWN;

    DISPFUNC();

    if_module = get_if_module_by_panel(panel);
    disp_interface_init(if_module, dev);
    disp_interface_config(if_module, dev, panel);

    return if_module;
}

static int display_if_deinit(struct intf_device *dev, struct sdm_panel *panel)
{
    int if_module = DISP_IF_MODULE_UNKNOWN;

    DISPFUNC();

    if_module = get_if_module_by_panel(panel);
    disp_interface_deinit(if_module, dev);

    return 0;
}

static int panel_init(struct intf_device *dev, struct sdm_panel *panel)
{
    switch (panel->if_type) {
    case IF_TYPE_DSI:
        mipi_lcm_init(dev->dsi_index, panel);
        break;
    case IF_TYPE_LVDS:
        /*billy*/;
        break;
    case IF_TYPE_PARALLEL:
        // TODO: parallel init
        break;
    case IF_TYPE_HDMI:
    case IF_TYPE_I2C:
    case IF_TYPE_SPI:
        DISPERR("External interface not init here\n");
        break;
    default:
        DISPERR("not support this if type [%d]\n", panel->if_type);
        return -1;
    }
    return 0;
}

static int get_tcon_irq(mod_res_t *res_info)
{
    switch (res_info->res_id) {
#if RES_DC_DC1
    case RES_DC_DC1:
        return DC1_DC_IRQ1_NUM;
#endif
#if RES_DC_DC2
    case RES_DC_DC2:
        return DC2_DC_IRQ1_NUM;
#endif
#if RES_DC_DC3
    case RES_DC_DC3:
        return DC3_DC_IRQ1_NUM;
#endif
#if RES_DC_DC4
    case RES_DC_DC4:
        return DC4_DC_IRQ1_NUM;
#endif
#if RES_DC_DC5
    case RES_DC_DC5:
        return DC5_DC_IRQ1_NUM;
#endif
    default:
        DISPERR("irq:not support this res [%d]\n", res_info->res_id);
        return -1;
    }
}

static int get_dc_index(mod_res_t *res_info)
{
    switch (res_info->res_id) {
#if RES_DC_DC1
    case RES_DC_DC1:
        return 0;
#endif
#if RES_DC_DC2
    case RES_DC_DC2:
        return 1;
#endif
#if RES_DC_DC3
    case RES_DC_DC3:
        return 2;
#endif
#if RES_DC_DC4
    case RES_DC_DC4:
        return 3;
#endif
#if RES_DC_DC5
    case RES_DC_DC5:
        return 4;
#endif
    default:
        DISPERR("index:not support this res [%d]\n", res_info->res_id);
        return -1;
    }
}

static int get_mlc_select(int dc_num, int dc_id, int dp)
{
    switch (dp) {
        case DP_RES_DP1:
        case DP_RES_DP3:
            return 0x1;
        case DP_RES_DP2:
            return 0x2;
        case DP_RES_DP1_DP2:
            if (dc_num == 1)
                return 0x3;
            else if (dc_id)
                return 0x2;
            else
                return 0x1;
        default:
            return 0;
    }
}

static int get_dsp_clk(int clk, int lvds_mode)
{
    if (lvds_mode == LVDS_MODE_DUAL)
        return clk / 3.5;

    return clk / 7.0;
}

static struct intf_device* handle_to_intf(display_handle *handle)
{
    int dc_id;
    mod_res_t *dc_res;

    dc_res = &handle->dm->dc[0];
    dc_id = get_dc_index(dc_res);
    intf_dev[dc_id].display_id = handle->display_id;
    intf_dev[dc_id].intf_res = handle->res->if_res;
    intf_dev[dc_id].lvds_cmb = handle->res->lvds_cmb;
    intf_dev[dc_id].lvds_mode = handle->res->lvds_mode;
    intf_dev[dc_id].pixel_bpp = handle->res->pixel_bpp;
    intf_dev[dc_id].tcon_clk = get_dsp_clk(handle->res->clk, handle->res->lvds_mode);

    return &intf_dev[dc_id];
}

/**********/
int disp_vsync_callback(int display_id) {
    display_handle *handle = hal_get_display_handle(display_id);

    if (handle->vsync)
        return handle->vsync(display_id, 0);
    return 0;
}

int disp_init(display_handle *handle)
{
    int tcon_irq, dc_id;
    int mlc_select;
    double ratio, dsp_clk;
    mod_res_t *dc_res;
    struct sdm_panel *panel;
    struct dc_device *dev = NULL;
    const double pix_clk = 415800000;//415.8MHz

#ifdef DISP_VERSION
    DISPMSG("sd_disp build safety version: %s\n", DISP_VERSION);
#endif
    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        tcon_irq = get_tcon_irq(dc_res);
        panel = handle->panels[0];
        mlc_select = get_mlc_select(handle->dm->num_dc, dc_id, handle->res->dp_res);
        dsp_clk = get_dsp_clk(handle->res->clk, handle->res->lvds_mode);
        ratio = pix_clk / dsp_clk;
        dev = &dc_dev[dc_id];

        dev->display_id = handle->display_id;
        dev->dc_idx = dc_id;

        if (handle->dm->num_dc == 1) {
            dev->ms_info.ms_en = 0;
        } else {
            dev->ms_info.ms_en = 1;
            dev->ms_info.start = 0;
            dev->ms_info.end = panel->display_timing->hactive / 2;
        }
        dev->ms_info.ms_mode = MASTER_MODE;

        dev->irq = tcon_irq;
        dev->base = (unsigned long)_ioaddr(dc_res->base);
        dev->mlc_select = mlc_select;
        dev->dsp_clk = dsp_clk;
        dev->ratio = ratio;
        dev->vsync_wait = 0;
        mutex_init(&dev->mutex_refresh);
        // event_init(&dev->vsync_kick, 0, EVENT_FLAG_AUTOUNSIGNAL);
        dc_ops.init(dev, panel);

        if (handle->is_need_register_irq) {
            register_int_handler(tcon_irq, dc_ops.irq_handler, dev);
        }
    }

    if (handle->dm->num_dc > 1) {
        dc_res = &handle->dm->dc[1];
        dc_id = get_dc_index(dc_res);
        tcon_irq = get_tcon_irq(dc_res);
        mlc_select = get_mlc_select(handle->dm->num_dc, dc_id, handle->res->dp_res);
        dev = &dc_dev[dc_id];

        dev->display_id = handle->display_id;
        dev->dc_idx = dc_id;
        dev->ms_info.ms_en = 1;
        dev->ms_info.ms_mode = SLAVE_MODE;
        dev->ms_info.start = panel->display_timing->hactive / 2;
        dev->ms_info.end = panel->display_timing->hactive;
        dev->irq = tcon_irq;
        dev->base = (unsigned long)_ioaddr(dc_res->base);
        dev->mlc_select = mlc_select;
        dev->dsp_clk = dsp_clk;
        dev->ratio = ratio;

        dc_ops.init(dev, panel);
    }

    return 0;
}

int disp_uninit(display_handle *handle)
{
    return 0;
}

int disp_clear_layers(display_handle *handle, u8 mask, u8 z_order)
{
    int dc_id;
    mod_res_t *dc_res;
    struct dc_device *dev = NULL;

    if (handle->dm->num_dc > 1) {
        dc_res = &handle->dm->dc[1];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        dc_ops.clear_layers(dev, mask, z_order);

        dc_ops.triggle(dev);
    }

    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        dc_ops.clear_layers(dev, mask, z_order);

        dc_ops.triggle(dev);

    }

    return 0;
}

int disp_post_config(display_handle *handle, struct sdm_post_config *post)
{
    static bool run_status[MAX_DISP_NUM] = {false};
    int dc_id;
    int res;
    mod_res_t *dc_res;
    struct dc_device *dev = NULL;


    lk_time_t post_time = current_time();
    if (handle->dm->num_dc > 1) {
        dc_res = &handle->dm->dc[1];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        res = mutex_acquire_timeout(&dev->mutex_refresh, 1000);
        if (res) {
            DISPMSG("display[%d] got mutex timeout: %d\n", handle->display_id, res);
        }
        // event_t *vsync = &dc_dev[get_dc_index(&handle->dm->dc[0])].vsync_kick;
        // while(dc_ops.check_triggle_status(dev)){
        //     event_wait(vsync);
        // }

        dc_ops.update(dev, post);

        dc_ops.triggle(dev);

        if (!run_status[dc_id]) {
            dc_ops.enable(dev);
            run_status[dc_id] = true;
        }
        if (!res)
            mutex_release(&dev->mutex_refresh);
    }

    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        res = mutex_acquire_timeout(&dev->mutex_refresh, 1000);
        if (res) {
            DISPMSG("display[%d] got mutex timeout: %d\n", handle->display_id, res);
        }
        dc_ops.update(dev, post);

        if (!run_status[dc_id]) {
            dc_ops.vsync_enable(dev, true);
        }

        dc_ops.triggle(dev);

        if (!run_status[dc_id]) {
            if (handle->is_need_register_irq) {
                unmask_interrupt(dev->irq);
            }
            dc_ops.enable(dev);
            run_status[dc_id] = true;
        }

        if (!res)
            mutex_release(&dev->mutex_refresh);
    }

    while((handle->dm->num_dc <= 1) && dc_ops.check_triggle_status(dev)){
        while(post_time > (lk_time_t)dev->vsync_wait) {
            thread_sleep(1);
        }
    }

    return 0;
}

bool disp_panel_connect(display_handle *handle, int sub_id,
         struct sdm_panel *panel)
{
    int ret;
    int if_type;
    struct intf_device *dev = NULL;

    dprintf(0, "[disp] We will check lcm: %s\n", panel->panel_name);

    dev = handle_to_intf(handle);
#ifdef PROJECT_BF200
    if (dev->display_id == 2)
        dev->intf_res = IF_RES_PARALLEL;
#endif
    /*check if type*/
    if_type = get_if_type_by_res(dev->intf_res);
    if (panel->if_type != if_type) {
        DISPERR("Interface is not compatible\n");
        return false;
    }

    /*interface init*/
    ret = display_if_init(dev, panel);
    if (ret < 0) {
        DISPERR("Interface init failed\n");
        return false;
    }

    /*Panel init power*/
    //panel_init_power(); //billy

    if (0)/*(panel_compare_id()) billy*/ {
        ret = display_if_deinit(dev, panel);
        if (ret < 0) {
            DISPERR("Interface deinit failed\n");
        }
        return false;
    }

    ret = panel_init(dev, panel);
    if (ret < 0) {
        DISPERR("panel init failed\n");
        return false;
    }

    return true;
}

int disp_panel_disconnect(display_handle *handle, int sub_id,
        struct sdm_panel *panel)
{
    return 0;
}

int disp_pq_config(display_handle *handle, struct sdm_pq_params *pq)
{
    int dc_id;
    mod_res_t *dc_res;
    struct dc_device *dev = NULL;

    if (handle->dm->num_dc > 1) {
        dc_res = &handle->dm->dc[1];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        dc_ops.csc_set(dev, pq);
        dc_ops.triggle(dev);
    }

    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        dc_ops.csc_set(dev, pq);
        dc_ops.triggle(dev);

    }

    return 0;
}
