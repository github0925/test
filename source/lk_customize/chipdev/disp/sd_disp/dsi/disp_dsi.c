/*
* disp_dsi.c
*
* Copyright (c) 2018-2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 12/23/2019 BI create this file
*/

#include <irq.h>
#include <lib/reg.h>
#include <string.h>
#include <disp_general_header.h>
#include <disp_dsi.h>
#include <dsi_reg.h>
#include <sdm_panel.h>
#include <disp_drv_log.h>
#include <disp_interface.h>
#include <disp_dsi_api.h>

#define MAX_TIME_HS2LP	120 /* unit: ns */
#define MAX_TIME_LP2HS	500 /* unit: ns */

extern struct dsi_core_ops dwc_mipi_dsi_host_ops;
struct dsi_context dsi_ctx[2];

static int dsi_get_index(int if_res)
{
    switch (if_res) {
    case IF_RES_MIPI_DSI1:
        return 0;
    case IF_RES_MIPI_DSI2:
        return 1;
    default:
        DISPERR("dsi get index input is not correct\n");
        return -1;
    }
}
static int dsi_get_reg_base(struct dsi_context *ctx)
{
    if (ctx->base) {
        DISPMSG("dsi ctx base already inited before\n");
        return 0;
    }

    switch (ctx->id) {
    case 0:
        ctx->base = (addr_t) _ioaddr(DSI1_BASE);
        break;
    case 1:
        ctx->base = (addr_t) _ioaddr(DSI2_BASE);
        break;
    }

    return 0;
}

static void dsi_get_irq_num(struct dsi_context *ctx)
{
    switch (ctx->id) {
    case 0:
        ctx->irq = MIPI_DSI1_INTERRUPT_NUM;
        break;
    case 1:
        ctx->irq = MIPI_DSI2_INTERRUPT_NUM;
        break;
    }
}

static int dsi_init(struct intf_device *dev)
{
    return 0;
}

static int dsi_deinit(struct intf_device *dev)
{
    return 0;
}

static void dsi_config(struct intf_device *dev, struct sdm_panel *panel)
{
    int id, ret;
    uint32_t hs2lp, lp2hs;
    struct dsi_context *ctx;
    struct dsih_dpi_video_t *dpi_video;

    id = dsi_get_index(dev->intf_res);
    if (id < 0) {
        return;
    }

    dev->dsi_index = id;
    ctx = &dsi_ctx[id];
    dpi_video = &ctx->dpi_video;
    memset(ctx, 0, sizeof(struct dsi_context));
    ctx->id = id;
    /*dsi ctx set base address*/
    dsi_get_reg_base(ctx);
    /*dsi ctx set irq number*/
    dsi_get_irq_num(ctx);

    ctx->ops = &dwc_mipi_dsi_host_ops;

    ret = mipi_dsih_open(ctx);
    if (ret < 0) {
        DISPERR("mipi dsih open failed\n");
        return;
    }

    /*init dpi video params*/
    dpi_video->n_lanes = panel->mipi->lane_number;
    dpi_video->burst_mode = panel->mipi->burst_mode;
    dpi_video->phy_freq = panel->mipi->phy_freq;

    dpi_video->color_coding = COLOR_CODE_24BIT;

    hs2lp = MAX_TIME_HS2LP * panel->mipi->phy_freq / 8000000;
    hs2lp += (MAX_TIME_HS2LP * panel->mipi->phy_freq % 8000000) < 4000000 ? 0 : 1;
    dpi_video->data_hs2lp = hs2lp;

    lp2hs = MAX_TIME_LP2HS * panel->mipi->phy_freq / 8000000;
    lp2hs += (MAX_TIME_LP2HS * panel->mipi->phy_freq % 8000000) < 4000000 ? 0 : 1;
    dpi_video->data_lp2hs = lp2hs;

    dpi_video->clk_hs2lp = 4;
    dpi_video->clk_lp2hs = 15;
    dpi_video->nc_clk_en = 0;
    dpi_video->frame_ack_en = 0;
    dpi_video->is_18_loosely = 0;
    dpi_video->eotp_rx_en = 0;
    dpi_video->eotp_tx_en = 1;
    dpi_video->dpi_lp_cmd_en = 0;

    dpi_video->virtual_channel= 0;

    dpi_video->h_total_pixels = panel->display_timing->hactive +
        panel->display_timing->hback_porch + panel->display_timing->hsync_len +
        panel->display_timing->hfront_porch;
    dpi_video->h_active_pixels = panel->display_timing->hactive;
    dpi_video->h_sync_pixels = panel->display_timing->hsync_len;
    dpi_video->h_back_porch_pixels = panel->display_timing->hback_porch;

    dpi_video->v_total_lines = panel->display_timing->vactive +
        panel->display_timing->vback_porch + panel->display_timing->vsync_len +
        panel->display_timing->vfront_porch;
    dpi_video->v_active_lines = panel->display_timing->vactive;
    dpi_video->v_sync_lines = panel->display_timing->vsync_len;
    dpi_video->v_back_porch_lines = panel->display_timing->vback_porch;
    dpi_video->v_front_porch_lines = panel->display_timing->vfront_porch;

    dpi_video->data_en_polarity = panel->display_timing->de_pol;
    dpi_video->h_polarity = panel->display_timing->hsync_pol;
    dpi_video->v_polarity = panel->display_timing->vsync_pol;

    dpi_video->pixel_clock = dev->tcon_clk / 1000;
    //dpi_video->pixel_clock = panel->display_timing->tcon_clk / 1000;
    dpi_video->byte_clock = dpi_video->phy_freq / 8;

    ret = mipi_dsih_dpi_video(ctx);
    if (ret < 0) {
        DISPERR("dsi: mipi_dsih_dpi_video failed\n");
    }
}

DISP_MODULE_DRIVER disp_driver_dsi = {
    .module = DISP_IF_MODULE_DSI,
    .init   = dsi_init,
    .deinit = dsi_deinit,
    .config = dsi_config,
};

